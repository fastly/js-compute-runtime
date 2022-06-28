// Node & 3P Modules
import fs from 'fs';
import fsPromises from 'fs/promises';
import path from 'path';
import childProcess from 'node:child_process';
import fetch from 'node-fetch';
import chalk from 'chalk';
import betterLogging from 'better-logging';

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

// Get our config from the Github Action
const integrationTestBase = `./integration-tests/js-compute`;
const fixtureBase = `${integrationTestBase}/fixtures`;

async function spawnViceroy(testName, viceroyAddr) {
  const wasmPath = `${fixtureBase}/${testName}/${testName}.wasm`;
  const fastlyTomlPath = `${fixtureBase}/${testName}/fastly.toml`;

  let viceroy = new Viceroy();
  await viceroy.spawn(wasmPath, {
    config: fastlyTomlPath,
    addr: viceroyAddr
  });

  return viceroy;
}

function buildTest(testName, backendAddr) {
  console.info(`Compiling the fixture for: ${testName} ...`);

  try {
    childProcess.execSync(`./integration-tests/js-compute/build-one.sh ${testName}`);
  } catch(e) {
    console.error(`Failed to compile ${testName}`);
    console.log(e.stdout.toString("utf-8"));
    process.exit(1);
  }

  try {
    childProcess.execSync(`./integration-tests/js-compute/replace-host.sh ${testName} http://${backendAddr}`);
  } catch(e) {
    console.error(`Failed to compile ${testName}`);
    console.log(e.stdout.toString("utf-8"));
    process.exit(1);
  }
}

async function discoverTests(fixturesPath) {
  let tests = {};

  // discover all of our test cases
  let fixtures = await fsPromises.readdir(fixturesPath, { withFileTypes: true });
  for (const ent of fixtures) {
    if (!ent.isDirectory()) {
      continue;
    }

    let jsonText;
    try {
      jsonText = await fsPromises.readFile(`${fixturesPath}/${ent.name}/tests.json`);
    } catch(err) {
      continue;
    }

    tests[ent.name] = JSON.parse(jsonText);
  }

  return tests;
}


// Our main task, in which we compile and run tests
const mainAsyncTask = async () => {

  const backendAddr = '127.0.0.1:8082';

  const testCases = await discoverTests(fixtureBase);
  const testNames = Object.keys(testCases);

  // build all the tests
  testNames.forEach(testName => buildTest(testName, backendAddr));
  buildTest('backend', backendAddr);

  // Start up the local backend
  console.info(`Starting the generic backend on ${backendAddr}`);
  let backend = await spawnViceroy('backend', backendAddr);

  console.info(`Running the Viceroy environment tests ...`);

  // Define our viceroy here so we can kill it on errors
  let viceroy;

  // Check if we are validating any local upstream requests (For example, like telemetry being sent)
  // If so, we will need an upstream server to compare the request that was sent
  let isDownstreamResponseHandled = false;
  let upstreamServer = new UpstreamServer();
  let upstreamServerTest;
  upstreamServer.listen(8081, async (localUpstreamRequestNumber, req, res) => {
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

  // Iterate through the module tests, and run the Viceroy tests
  for (const testName of testNames) {
    const testBase = `${fixtureBase}/${testName}`;

    // created/used by ./integration-tests/js-compute/build-one.sh
    const fastlyTomlPath = `${testBase}/fastly.toml`;
    const wasmPath = `${testBase}/${testName}.wasm`;

    const tests = testCases[testName];
    const moduleTestKeys = Object.keys(tests);
    console.info(`Running tests for the module: ${testName} ...`);

    // Spawn a new viceroy instance for the module
    viceroy = new Viceroy();
    const viceroyAddr = '127.0.0.1:8080';
    await viceroy.spawn(wasmPath, {
      config: fastlyTomlPath,
      addr: viceroyAddr
    })

    for (const testKey of moduleTestKeys) {
      const test = tests[testKey];

      // Check if this case should be tested in viceroy
      if (!test.environments.includes("viceroy")) {
        continue;
      }

      console.log(`Running the test ${testKey} ...`);

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
      try {
        downstreamResponse = await fetch(`http://${viceroyAddr}${downstreamRequest.pathname || ''}`, {
          method: downstreamRequest.method || 'GET',
          headers: downstreamRequest.headers || undefined,
          body: downstreamRequest.body || undefined
        });
      } catch(error) {
        await upstreamServer.close();
        await backend.kill();
        await viceroy.kill();
        console.error(error);
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
      } catch(e) {
        console.error('Could not cleanup the upstream server. Error Below:');
        console.error(e);
        process.exit(1);
      }
    }

    // Kill Viceroy and continue onto the next module
    try {
      await viceroy.kill();
    } catch(e) {
      console.error('Could not kill test Viceroy instance. Error Below:');
      console.error(e);
      process.exit(1);
    }
  };

  // Viceroy is done! Close our upstream server and things
  await upstreamServer.close();
  try {
    await backend.kill();
  } catch(e) {
      console.error('Could not kill backend Viceroy instance. Error Below:');
      console.error(e);
      process.exit(1);
  }

  // Check if we have C@E Environement tests
  let shouldRunComputeTests = testNames.some(testName => {
    const tests = testCases[testName];
    const moduleTestKeys = Object.keys(tests);

    return moduleTestKeys.some(testKey => {
      const test = tests[testKey];

      // Check if this module should be tested in viceroy
      if (test.environments.includes("c@e")) {
        return true;
      }
      return false;
    });
  });

  if (!shouldRunComputeTests) {
    console.info('Viceroy environment tests are done, and no C@E environment tests!');
    console.info('We are finished, all tests passed! :)');
    return;
  }

  console.warn('C@E tests are currently unsupported');
};
mainAsyncTask().then(() => {
  process.exit(0);
}).catch((error) => {
  console.error(error.message);
  console.dir(error);
  process.exit(1);
});


