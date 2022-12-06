// Node & 3P Modules
import fsPromises from 'fs/promises';
import childProcess from 'node:child_process';
import { request, setGlobalDispatcher, Agent, Pool } from 'undici'
import chalk from 'chalk';
import betterLogging from 'better-logging';
import core from '@actions/core';

const fixture = core.getInput('fixture') || process.argv[2];

setGlobalDispatcher(new Agent({
  factory: (origin) => new Pool(origin, {
    connections: 128, 
    connect: { timeout: 120_000 },
    bodyTimeout: 0,
    headersTimeout: 120_000,
    keepAliveTimeout: 120_000,
    keepAliveTimeoutThreshold: 120_000,
  }),
  connect: { timeout: 120_000 },
  bodyTimeout: 0,
  headersTimeout: 120_000,
  keepAliveTimeout: 120_000,
  keepAliveTimeoutThreshold: 120_000,
}))

betterLogging(console, {
  format: ctx => {

    const tag = chalk.bold(`[compute-sdk-test]`);

    if (ctx.type.includes("debug")) {
      return chalk.blue(`${tag} ${chalk.bold(ctx.type)} ${ctx.msg}`);
    } else if (ctx.type.includes("info")) {
      return chalk.green(`${tag} ${chalk.bold(ctx.type)} ${ctx.msg}`);
    } else if (ctx.type.includes("warn")) {
      return chalk.yellow(`${tag} ${chalk.bold(ctx.type)} ${ctx.msg}`);
    } else if (ctx.type.includes("error")) {
      return chalk.red(`${tag} ${chalk.bold(ctx.type)} ${ctx.msg}`);
    }

    return `${tag} ${chalk.white.bold(ctx.type)} ${ctx.msg}`;
  }
});

// Utility modules
import Viceroy from './src/viceroy.js';
import UpstreamServer from './src/upstream-server.js';
import compareUpstreamRequest from './src/compare-upstream-request.js';
import compareDownstreamResponse from './src/compare-downstream-response.js';
import tests from './src/test-suite.js';


async function spawnViceroy(config, testName, viceroyAddr) {
  console.info(`Spawning a viceroy instance for ${testName} on ${viceroyAddr}`);

  let viceroy = new Viceroy();

  await viceroy.spawn(config.wasmPath(testName), {
    config: config.fastlyTomlPath(testName),
    addr: viceroyAddr
  });

  return viceroy;
}

function buildTest(config, testName, backendAddr) {
  console.info(`Compiling the fixture for: ${testName} ...`);

  try {
    childProcess.execSync(`${config.replaceHostScript} ${testName} http://${backendAddr}`);
    childProcess.execSync(`${config.buildScript} ${testName}`);
  } catch (e) {
    console.error(`Failed to compile ${testName}`);
    console.info(e.stdout.toString("utf-8"));
    process.exit(1);
  }

}

async function discoverTests(config) {
  console.info(`Looking for tests in ${config.fixtureBase}`);

  let tests = {};

  // discover all of our test cases
  let fixtures = await fsPromises.readdir(config.fixtureBase, { withFileTypes: true });
  for (const ent of fixtures) {
    if (!ent.isDirectory()) {
      continue;
    }

    let jsonText;
    try {
      jsonText = await fsPromises.readFile(config.testJsonPath(ent.name));
    } catch (err) {
      continue;
    }

    tests[ent.name] = JSON.parse(jsonText);
  }

  return tests;
}


// Our main task, in which we compile and run tests
const mainAsyncTask = async () => {

  // Get our config from the Github Action
  const config = new tests.TestConfig('./integration-tests/js-compute');

  const backendAddr = '127.0.0.1:8082';

  const testCases = await discoverTests(config);
  const test = testCases[fixture];

  if (!test) {
    throw new Error(`No fixture named "${fixture}". The available fixtures are ${Object.keys(testCases)}`)
  }

  // build the test
  buildTest(config, fixture, backendAddr);
  buildTest(config, 'backend', backendAddr);

  // Start up the local backend
  console.info(`Starting the generic backend on ${backendAddr}`);
  let backend = await spawnViceroy(config, 'backend', backendAddr);

  console.info(`Running the Viceroy environment tests ...`);

  // Define our viceroy here so we can kill it on errors
  let viceroy;

  // Check if we are validating any local upstream requests (For example, like telemetry being sent)
  // If so, we will need an upstream server to compare the request that was sent
  let isDownstreamResponseHandled = false;
  let upstreamServer = new UpstreamServer();
  let upstreamServerTest;
  await upstreamServer.listen(8081, async (localUpstreamRequestNumber, req, res) => {
    // Let's do the verifications on the request
    const configRequest = upstreamServerTest.local_upstream_requests[localUpstreamRequestNumber];

    try {
      await compareUpstreamRequest(configRequest, req, isDownstreamResponseHandled);
    } catch (err) {
      await backend.kill();
      await viceroy.kill();
      console.error(`[LocalUpstreamRequest (${localUpstreamRequestNumber})] ${err.message}`);
      process.exit(1);
    }
  });

  // Spawn a new viceroy instance for the module
  const viceroyAddr = '127.0.0.1:8080';
  viceroy = await spawnViceroy(config, fixture, viceroyAddr);
  await new Promise((resolve) => {
    setTimeout(resolve, 5_000)
  })
  // Run the Viceroy tests
  const moduleTestKeys = Object.keys(test);
  console.info(`Running tests for the module: ${fixture} ...`);


  for (const testKey of moduleTestKeys) {
    const test = testCases[fixture][testKey];

    // Check if this case should be tested in viceroy
    if (!test.environments.includes("viceroy")) {
      continue;
    }

    console.info(`Running the test ${testKey} ...`);

    // Prepare our upstream server for this specific test
    isDownstreamResponseHandled = false;
    if (test.local_upstream_requests) {
      upstreamServerTest = test;
      upstreamServer.setExpectedNumberOfRequests(test.local_upstream_requests.length);
    } else {
      upstreamServerTest = null;
      upstreamServer.setExpectedNumberOfRequests(0);
    }

    // Make the downstream request to the server (Viceroy)
    const downstreamRequest = test.downstream_request;
    let downstreamResponse;
    let url = `http://${viceroyAddr}${downstreamRequest.pathname || ''}`
    try {
      downstreamResponse = await request(url,
        {
          method: downstreamRequest.method || 'GET',
          headers: downstreamRequest.headers || undefined,
          body: downstreamRequest.body || undefined,
        });
    } catch (error) {
      console.error(url);
      await upstreamServer.close();
      await backend.kill();
      await viceroy.kill();
      console.error(error);
      console.error(error.stack);
      process.exit(1);
    }

    // Now that we have gotten our downstream response, we can flip our boolean
    // that our local_upstream_request will check
    isDownstreamResponseHandled = true;

    // Check the Logs to see if they match expected logs in the config
    if (test.logs) {
      for (let i = 0; i < test.logs.length; i++) {
        let log = test.logs[i];

        if (!viceroy.logs.includes(log)) {
          console.error(`[Logs: log not found] Expected: ${log}`);
          await upstreamServer.close();
          await backend.kill();
          await viceroy.kill();
          process.exit(1);
        }
      }
    }

    // Do our confirmations about the downstream response
    const configResponse = test.downstream_response;
    try {
      await compareDownstreamResponse(configResponse, downstreamResponse);
    } catch (err) {
      console.error(err.message);
      await upstreamServer.close();
      await backend.kill();
      await viceroy.kill();
      process.exit(1);
    }

    console.log(`The test ${testKey} Passed!`);

    // Done! Kill the process, and go to the next test
    try {
      await upstreamServer.waitForExpectedNumberOfRequests();
      upstreamServerTest = null;
      upstreamServer.setExpectedNumberOfRequests(0);
    } catch (e) {
      console.error('Could not cleanup the upstream server. Error Below:');
      console.error(e);
      process.exit(1);
    }
  }

  // Kill Viceroy and continue onto the next module
  try {
    await viceroy.kill();
  } catch (e) {
    console.error('Could not kill test Viceroy instance. Error Below:');
    console.error(e);
    process.exit(1);
  }

  // Viceroy is done! Close our upstream server and things
  await upstreamServer.close();
  try {
    await backend.kill();
  } catch (e) {
    console.error('Could not kill backend Viceroy instance. Error Below:');
    console.error(e);
    process.exit(1);
  }

  console.info('We are finished, all tests passed! :)');
};
mainAsyncTask().then(() => {
  process.exit(0);
}).catch((error) => {
  console.error(error.message);
  console.dir(error);
  process.exit(1);
});


