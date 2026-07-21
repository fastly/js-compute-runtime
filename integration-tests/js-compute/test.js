#!/usr/bin/env node

import { fileURLToPath } from 'node:url';
import { dirname, join } from 'node:path';
import { cd, $ as zx, retry, expBackoff } from 'zx';
import { request } from 'undici';
import { compareDownstreamResponse } from './compare-downstream-response.js';
import { argv } from 'node:process';
import { existsSync } from 'node:fs';
import { copyFile, readFile, writeFile } from 'node:fs/promises';
import * as core from '@actions/core';
import TOML from '@iarna/toml';
import { getEnv, GLOBAL_PREFIX } from './env.js';
import { $ } from './util.js';

// test environment variable handling
process.env.LOCAL_TEST = 'local val';

async function killPortProcess(port) {
  zx.verbose = false;
  const pids = (await zx`lsof -ti:${port}`).stdout;
  if (pids) {
    for (const pid of pids.split('\n').reverse()) {
      if (pid && pid != process.pid) {
        await zx`kill -15 ${pid}`;
      }
    }
  }
}

const startTime = Date.now();
const __dirname = dirname(fileURLToPath(import.meta.url));

const green = '\u001b[32m';
const red = '\u001b[31m';
const reset = '\u001b[0m';
const white = '\u001b[39m';
const info = '\u2139';
const tick = '\u2714';
const cross = '\u2716';

function appendBuildFlag(config, flag) {
  const buildArgs = config.scripts.build.split(' ');
  buildArgs.splice(-1, null, flag);
  config.scripts.build = buildArgs.join(' ');
}

async function resolveFastlyApiToken() {
  try {
    zx.verbose = false;
    process.env.FASTLY_API_TOKEN = String(
      await $`fastly auth show --reveal | grep 'Token:' | cut -d ' ' -f2-`,
    ).trim();
  } catch {
    console.error(
      'No environment variable named FASTLY_API_TOKEN has been set and no default fastly profile exists.',
    );
    console.error(
      'In order to run the tests, either create a fastly profile using `fastly profile create` or export a fastly token under the name FASTLY_API_TOKEN',
    );
    process.exit(1);
  }
}

let args = argv.slice(2);

const local = args.includes('--local');
const verbose = args.includes('--verbose');
const serial = args.includes('--serial');
const fixtureArg = args.find((arg) => arg.startsWith('--fixture='));
const httpCache = args.includes('--http-cache');
const aot = args.includes('--aot');
const debugBuild = args.includes('--debug-build');
const debugLog = args.includes('--debug-log');
const skipSetup = args.includes('--skip-setup');
const skipTeardown = args.includes('--skip-teardown');
const filter = args.filter((arg) => !arg.startsWith('--'));
const bail = args.includes('--bail');
const ci = args.includes('--ci');

if (!local && process.env.FASTLY_API_TOKEN === undefined) {
  await resolveFastlyApiToken();
}

const FASTLY_API_TOKEN = process.env.FASTLY_API_TOKEN;
zx.verbose = true;
const branchName = (await zx`git branch --show-current`).stdout
  .trim()
  .replace(/[^a-zA-Z0-9_-]/g, '_');

const fixture = fixtureArg !== undefined ? fixtureArg.split('=')[1] : 'app';

// Service names are carefully unique to support parallel runs
const serviceName = `${GLOBAL_PREFIX}app-${branchName}${aot ? '--aot' : ''}${httpCache ? '--http' : ''}${process.env.SUFFIX_STRING ? '--' + process.env.SUFFIX_STRING : ''}`;
let domain, serviceId;
const fixturePath = join(__dirname, 'fixtures', fixture);
let localServer;

const env = getEnv(ci && !local ? serviceName : null);
Object.assign(process.env, env);
if (debugLog) {
  process.env.FASTLY_DEBUG_LOGGING = '1';
}

await cd(fixturePath);
await copyFile(
  join(fixturePath, 'fastly.toml.in'),
  join(fixturePath, 'fastly.toml'),
);
const envSeen = new Set();
const config = TOML.parse(
  (await readFile(join(fixturePath, 'fastly.toml'), 'utf-8')).replace(
    /DICTIONARY_NAME|CONFIG_STORE_NAME|KV_STORE_NAME|SECRET_STORE_NAME|ACL_NAME/g,
    (match) => {
      // we only replace the second instance, because the first is the --env flag itself
      if (!envSeen.has(match)) {
        envSeen.add(match);
        return match;
      }
      return env[match] || match;
    },
  ),
);
config.name = serviceName;
if (aot) appendBuildFlag(config, '--enable-aot');
if (debugBuild) appendBuildFlag(config, '--debug-build');
if (httpCache) appendBuildFlag(config, '--enable-http-cache');
await writeFile(
  join(fixturePath, 'fastly.toml'),
  TOML.stringify(config),
  'utf-8',
);

async function waitUntilServiceReady() {
  // Local uses a hand-tuned backoff since we expect the build to take ~10s.
  const localReadyBackoff = [
    6000, 3000, 1500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500,
    500, 500, 500, 500, 500, 500, 500, 500,
    // after >20s the build is unusually slow; start backing off before timeout
    1500, 3000, 6000, 12000, 24000,
  ].values();

  await Promise.all([
    retry(
      27,
      local ? localReadyBackoff : expBackoff('60s', '10s'),
      async () => {
        const response = await request(domain);
        if (response.statusCode !== 200) {
          throw new Error(
            `Application "${fixture}" :: Not yet available on domain: ${domain}`,
          );
        }
      },
    ),
    // we need to wait for the service resource links to all activate,
    // and we don't currently have a reliable way to poll on that
    local ? null : new Promise((resolve) => setTimeout(resolve, 60_000)),
  ]);
}

async function teardownRemoteService() {
  const teardownPath = join(__dirname, 'teardown.js');
  if (existsSync(teardownPath)) {
    core.startGroup('Tear down the extra set-up for the service');
    await zx`${teardownPath} ${serviceId} ${ci ? serviceName : ''}`;
    core.endGroup();
  }
  core.startGroup('Delete service');
  try {
    await $`fastly service delete --quiet --service-name "${serviceName}" --force --token $FASTLY_API_TOKEN`;
  } catch (e) {
    console.log('Failed to delete service:', e.message);
  }
  core.endGroup();
}

function chunks(arr, size) {
  const output = [];
  for (let i = 0; i < arr.length; i += size) {
    output.push(arr.slice(i, i + size));
  }
  return output;
}

async function getBodyChunks(response, bodyStreaming) {
  const bodyChunks = [];
  let readChunks = async () => {
    switch (bodyStreaming) {
      case 'first-chunk-only':
        for await (const chunk of response.body) {
          bodyChunks.push(chunk);
          response.body.on('error', () => {});
          break;
        }
        break;
      case 'none':
        response.body.on('error', () => {});
        break;
      case 'full':
      default:
        for await (const chunk of response.body) {
          bodyChunks.push(chunk);
        }
    }
  };
  let downstreamTimeout;
  let timeoutPromise = new Promise((_, reject) => {
    downstreamTimeout = setTimeout(() => {
      reject(new Error(`Test downstream response body chunk timeout`));
    }, 30_000);
  });
  await Promise.race([readChunks(), timeoutPromise]);
  clearTimeout(downstreamTimeout);
  return bodyChunks;
}

async function runLocalTest(title, test) {
  const url = `${domain}${test.downstream_request.pathname}`;
  return (bail || !test.flake ? (_, __, fn) => fn() : retry)(
    5,
    expBackoff('10s', '1s'),
    async () => {
      try {
        const response = await request(url, {
          method: test.downstream_request.method || 'GET',
          headers: test.downstream_request.headers || undefined,
          body: test.downstream_request.body || undefined,
        });
        const bodyChunks = await getBodyChunks(response, test.body_streaming);
        await compareDownstreamResponse(
          test.downstream_response,
          response,
          bodyChunks,
        );
        return { title, test, skipped: false };
      } catch (error) {
        console.error('\n' + test.downstream_request.pathname);
        throw new Error(`${title} ${error.message}`, { cause: error });
      }
    },
  );
}

async function runComputeTest(title, test) {
  const url = `${domain}${test.downstream_request.pathname}`;
  const onInfo = test.downstream_info
    ? async (status, headers) => {
        if (
          test.downstream_info.status !== undefined &&
          test.downstream_info.status != status
        ) {
          throw new Error(
            `[DownstreamInfo: Status mismatch] Expected: ${test.downstream_info.status} - Got: ${status}`,
          );
        }
        if (headers) {
          compareHeaders(
            test.downstream_info.headers,
            headers,
            test.downstream_info.headersExhaustive,
          );
        }
      }
    : undefined;

  return retry(
    test.flake ? 15 : bail ? 1 : 4,
    expBackoff(test.flake ? '60s' : '30s', test.flake ? '30s' : '1s'),
    async () => {
      try {
        const response = await request(url, {
          method: test.downstream_request.method || 'GET',
          headers: test.downstream_request.headers || undefined,
          body: test.downstream_request.body || undefined,
          onInfo,
        });
        const bodyChunks = await getBodyChunks(response, test.body_streaming);
        await compareDownstreamResponse(
          test.downstream_response,
          response,
          bodyChunks,
        );
        return { title, test, skipped: false };
      } catch (error) {
        console.error('\n' + test.downstream_request.pathname);
        throw new Error(`${title} ${error.message}`);
      }
    },
  );
}

async function runTest(title, test) {
  // Apply defaults
  if (!test.downstream_request) {
    const [method, pathname, extra] = title.split(' ');
    if (typeof extra === 'string')
      throw new Error('Cannot infer downstream_request from title');
    test.downstream_request = { method, pathname };
  }
  if (!test.downstream_response) {
    test.downstream_response = { status: 200 };
  }
  if (!test.environments) {
    test.environments = ['viceroy', 'compute'];
  }

  // Basic test filtering
  if (
    test.skip ||
    (filter.length > 0 && filter.every((f) => !title.includes(f)))
  ) {
    return {
      title,
      test,
      skipped: true,
      skipReason: test.skip ? 'MARKED AS SKIPPED (pending further work)' : null,
    };
  }

  // Feature-based test filtering
  if (
    (!httpCache && test.features?.includes('http-cache')) ||
    (httpCache && test.features?.includes('skip-http-cache'))
  ) {
    return {
      title,
      test,
      skipped: true,
      skipReason: `feature "http-cache" ${httpCache ? '' : 'not '}"enabled`,
    };
  }

  if (local) {
    if (!test.environments.includes('viceroy')) {
      return { title, test, skipped: true, skipReason: 'no environments' };
    }
    return runLocalTest(title, test);
  } else {
    if (!test.environments.includes('compute')) {
      return { title, test, skipped: true, skipReason: 'no environments' };
    }
    return runComputeTest(title, test);
  }
}

let passed = 0;
const failed = [];
try {
  if (!local) {
    core.startGroup('Delete service if already exists');
    try {
      await zx`fastly service delete --quiet --service-name "${serviceName}" --force --token $FASTLY_API_TOKEN`;
    } catch {}
    core.endGroup();
    core.startGroup('Build and deploy service');
    await new Promise((resolve) => setTimeout(resolve, 5000));
    await zx`npm i`;
    await $`fastly compute publish -i ${verbose ? '--verbose' : '--quiet'} --token $FASTLY_API_TOKEN --status-check-off`;
    core.endGroup();

    // It can take time for the new domain to show up on the list.
    await Promise.all([
      retry(27, expBackoff('60s', '10s'), async () => {
        // get the public domain of the deployed application
        const domainListing = JSON.parse(
          await $`fastly service domain list --quiet --version latest --json`,
        )[0];
        domain = `https://${domainListing.Name}`;
        serviceId = domainListing.ServiceID;
        core.notice(`Service is running on ${domain}`);
      }),
      new Promise((resolve) => setTimeout(resolve, 60_000)),
    ]);
  } else {
    const pushpin = '--local-pushpin-proxy-port=0';
    const args = verbose ? '-vv ' + pushpin : pushpin;
    localServer = zx`fastly compute serve --verbose --viceroy-args="${args}"`;
    domain = 'http://127.0.0.1:7676';
  }

  core.startGroup(`Setting up service ${domain}`);

  if (!local && !skipSetup) {
    const setupPath = join(__dirname, 'setup.js');
    if (existsSync(setupPath)) {
      await zx`node ${setupPath} ${serviceId} ${ci ? serviceName : ''}`;
    }
  }

  await waitUntilServiceReady();

  core.endGroup();

  core.startGroup('Running tests');

  const { default: tests } = await import(join(fixturePath, 'tests.json'), {
    with: { type: 'json' },
  });

  const chunkSize = serial ? 1 : 100;
  const runChunk = bail
    ? Promise.all.bind(Promise)
    : Promise.allSettled.bind(Promise);
  let results = [];

  for (const chunk of chunks(Object.entries(tests), chunkSize)) {
    results.push(
      ...(await runChunk(chunk.map(([title, test]) => runTest(title, test)))),
    );
  }

  core.endGroup();

  console.log('Test results');
  core.startGroup('Test results');
  for (const result of results) {
    if (result.status === 'fulfilled' || bail) {
      const value = bail ? result : result.value;
      if (value.skipped) {
        if (value.skipReason)
          console.log(
            white,
            info,
            `Skipped ${value.title} due to ${value.skipReason}`,
            reset,
          );
      } else {
        passed += 1;
        console.log(green, tick, value.title, reset);
      }
    } else {
      console.log(red, cross, result.reason, reset);
      failed.push(`${value.title} - ${result.reason}`);
    }
  }
  core.endGroup();
} finally {
  if (failed.length || !passed) {
    process.exitCode = 1;
    core.startGroup('Failed tests');

    for (const result of failed) {
      console.log(red, cross, result, reset);
    }
    if (!passed) {
      console.log('No tests passed/ran.');
    }

    core.endGroup();
  }
  if (!local && (failed.length || !passed)) {
    core.notice(`Tests failed.`);
  }
  // No need to tear down the service if what failed was setting it up.
  if (!local && !skipTeardown && serviceId) {
    await teardownRemoteService();
  }
  if (process.exitCode == undefined || process.exitCode == 0) {
    console.log(
      `All tests passed! Took ${(Date.now() - startTime) / 1000} seconds to complete`,
    );
  } else {
    console.log(`Tests failed!`);
  }
  if (localServer) {
    await killPortProcess(7676);
  }
  process.exit();
}
