#!/usr/bin/env node

import { fileURLToPath } from 'node:url';
import { dirname, join } from 'node:path';
import { cd, $ as zx, retry, expBackoff } from 'zx';
import { request } from 'undici';
import { compareDownstreamResponse } from './compare-downstream-response.js';
import { argv } from 'node:process';
import { existsSync } from 'node:fs';
import { copyFile, readFile, writeFile } from 'node:fs/promises';
import core from '@actions/core';
import TOML from '@iarna/toml';
import { getEnv } from './env.js';

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

async function sleep(seconds) {
  return new Promise((resolve) => {
    setTimeout(resolve, 1_000 * seconds);
  });
}

let args = argv.slice(2);

const local = args.includes('--local');
const verbose = args.includes('--verbose');
const moduleMode = args.includes('--module-mode');
const httpCache = args.includes('--http-cache');
const aot = args.includes('--aot');
const debugBuild = args.includes('--debug-build');
const filter = args.filter((arg) => !arg.startsWith('--'));
const bail = args.includes('--bail');
const ci = args.includes('--ci');

async function $(...args) {
  return await retry(10, () => zx(...args));
}

if (!local && process.env.FASTLY_API_TOKEN === undefined) {
  try {
    zx.verbose = false;
    process.env.FASTLY_API_TOKEN = String(
      await zx`fastly profile token --quiet`,
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

const FASTLY_API_TOKEN = process.env.FASTLY_API_TOKEN;
zx.verbose = true;
const branchName = (await zx`git branch --show-current`).stdout
  .trim()
  .replace(/[^a-zA-Z0-9_-]/g, '_');

const fixture = !moduleMode ? 'app' : 'module-mode';

// Service names are carefully unique to support parallel runs
const serviceName = `app-${branchName}${aot ? '--aot' : ''}${httpCache ? '--http' : ''}${process.env.SUFFIX_STRING ? '--' + process.env.SUFFIX_STRING : ''}`;
let domain, serviceId;
const fixturePath = join(__dirname, 'fixtures', fixture);
let localServer;

const env = getEnv(ci && !local ? serviceName : null);
Object.assign(process.env, env);

await cd(fixturePath);
await copyFile(
  join(fixturePath, 'fastly.toml.in'),
  join(fixturePath, 'fastly.toml'),
);
const config = TOML.parse(
  (await readFile(join(fixturePath, 'fastly.toml'), 'utf-8')).replace(
    /DICTIONARY_NAME|CONFIG_STORE_NAME|KV_STORE_NAME|SECRET_STORE_NAME/g,
    (match) => env[match] || match,
  ),
);
config.name = serviceName;
if (aot) {
  const buildArgs = config.scripts.build.split(' ');
  buildArgs.splice(-1, null, '--enable-aot');
  config.scripts.build = buildArgs.join(' ');
}
if (debugBuild) {
  const buildArgs = config.scripts.build.split(' ');
  buildArgs.splice(-1, null, '--debug-build');
  config.scripts.build = buildArgs.join(' ');
}
if (httpCache) {
  const buildArgs = config.scripts.build.split(' ');
  buildArgs.splice(-1, null, '--enable-http-cache');
  config.scripts.build = buildArgs.join(' ');
}
await writeFile(
  join(fixturePath, 'fastly.toml'),
  TOML.stringify(config),
  'utf-8',
);
if (!local) {
  core.startGroup('Delete service if already exists');
  try {
    await zx`fastly service delete --quiet --service-name "${serviceName}" --force --token $FASTLY_API_TOKEN`;
  } catch {}
  core.endGroup();
  core.startGroup('Build and deploy service');
  await zx`npm i`;
  await $`fastly compute publish -i --quiet --token $FASTLY_API_TOKEN --status-check-off`;
  core.endGroup();

  // get the public domain of the deployed application
  const domainListing = JSON.parse(
    await $`fastly domain list --quiet --version latest --json`,
  )[0];
  domain = `https://${domainListing.Name}`;
  serviceId = domainListing.ServiceID;
  core.notice(`Service is running on ${domain}`);
} else {
  localServer = zx`fastly compute serve --verbose --viceroy-args="${verbose ? '-vv' : ''}"`;
  domain = 'http://127.0.0.1:7676';
}

core.startGroup(`Check service is up and running on ${domain}`);
await retry(
  27,
  local
    ? [
        // we expect it to take ~10 seconds to deploy, so focus on that time
        6000, 3000, 1500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500, 500,
        500, 500, 500, 500, 500, 500, 500, 500,
        // after more than 20 seconds, means we have an unusually slow build, start backoff before timeout
        1500,
        3000, 6000, 12000, 24000,
      ].values()
    : expBackoff('60s', '10s'),
  async () => {
    const response = await request(domain);
    if (response.statusCode !== 200) {
      throw new Error(
        `Application "${fixture}" :: Not yet available on domain: ${domain}`,
      );
    }
  },
);
core.endGroup();

if (!local) {
  const setupPath = join(__dirname, 'setup.js');
  if (existsSync(setupPath)) {
    core.startGroup('Extra set-up steps for the service');
    await zx`node ${setupPath} ${serviceId} ${ci ? serviceName : ''}`;
    await sleep(15);
    core.endGroup();
  }
}

let { default: tests } = await import(join(fixturePath, 'tests.json'), {
  with: { type: 'json' },
});

core.startGroup('Running tests');
function chunks(arr, size) {
  const output = [];
  for (let i = 0; i < arr.length; i += size) {
    output.push(arr.slice(i, i + size));
  }
  return output;
}

let results = [];
for (const chunk of chunks(Object.entries(tests), 100)) {
  results.push(
    ...(await (
      bail ? Promise.all.bind(Promise) : Promise.allSettled.bind(Promise)
    )(
      chunk.map(async ([title, test]) => {
        // test defaults
        if (!test.downstream_request) {
          const [method, pathname, extra] = title.split(' ');
          if (typeof extra === 'string')
            throw new Error('Cannot infer downstream_request from title');
          test.downstream_request = { method, pathname };
        }
        if (!test.downstream_response) {
          test.downstream_response = {
            status: 200,
          };
        }
        if (!test.environments) {
          test.environments = ['viceroy', 'compute'];
        }

        // basic test filtering
        if (
          test.skip ||
          (filter.length > 0 && filter.every((f) => !title.includes(f)))
        ) {
          return {
            title,
            test,
            skipped: true,
          };
        }
        // feature based test filtering
        if (
          (!httpCache &&
            test.features &&
            test.features.includes('http-cache')) ||
          (httpCache &&
            test.features &&
            test.features.includes('skip-http-cache'))
        ) {
          return {
            title,
            test,
            skipped: true,
          };
        }
        async function getBodyChunks(response) {
          const bodyChunks = [];
          let downstreamTimeout;
          await Promise.race([
            (async () => {
              // This body_streaming property allows us to test different cases
              // of consumer streamining behaviours.
              switch (test.body_streaming) {
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
            })(),
            new Promise((_, reject) => {
              downstreamTimeout = setTimeout(() => {
                reject(
                  new Error(`Test downstream response body chunk timeout`),
                );
              }, 30_000);
            }),
          ]);
          clearTimeout(downstreamTimeout);
          return bodyChunks;
        }
        if (local) {
          if (test.environments.includes('viceroy')) {
            return (bail || !test.flake ? (_, __, fn) => fn() : retry)(
              5,
              expBackoff('10s', '1s'),
              async () => {
                let path = test.downstream_request.pathname;
                let url = `${domain}${path}`;
                try {
                  const response = await request(url, {
                    method: test.downstream_request.method || 'GET',
                    headers: test.downstream_request.headers || undefined,
                    body: test.downstream_request.body || undefined,
                  });
                  const bodyChunks = await getBodyChunks(response);
                  await compareDownstreamResponse(
                    test.downstream_response,
                    response,
                    bodyChunks,
                  );
                  return {
                    title,
                    test,
                    skipped: false,
                  };
                } catch (error) {
                  console.error('\n' + test.downstream_request.pathname);
                  throw new Error(`${title} ${error.message}`, {
                    cause: error,
                  });
                }
              },
            );
          } else {
            return {
              title,
              test,
              skipped: true,
            };
          }
        } else {
          if (test.environments.includes('compute')) {
            return retry(
              test.flake ? 10 : bail ? 1 : 4,
              expBackoff(test.flake ? '60s' : '30s', test.flake ? '10s' : '1s'),
              async () => {
                let path = test.downstream_request.pathname;
                let url = `${domain}${path}`;
                try {
                  const response = await request(url, {
                    method: test.downstream_request.method || 'GET',
                    headers: test.downstream_request.headers || undefined,
                    body: test.downstream_request.body || undefined,
                  });
                  const bodyChunks = await getBodyChunks(response);
                  await compareDownstreamResponse(
                    test.downstream_response,
                    response,
                    bodyChunks,
                  );
                  return {
                    title,
                    test,
                    skipped: false,
                  };
                } catch (error) {
                  console.error('\n' + test.downstream_request.pathname);
                  throw new Error(`${title} ${error.message}`);
                }
              },
            );
          } else {
            return {
              title,
              test,
              skipped: true,
            };
          }
        }
      }),
    )),
  );
}
core.endGroup();

console.log('Test results');
core.startGroup('Test results');
let passed = 0;
const failed = [];
const green = '\u001b[32m';
const red = '\u001b[31m';
const reset = '\u001b[0m';
const white = '\u001b[39m';
const info = '\u2139';
const tick = '\u2714';
const cross = '\u2716';
for (const result of results) {
  if (result.status === 'fulfilled') {
    passed += 1;
    if (result.value.skipped) {
      if (
        filter.length > 0 &&
        filter.every((f) => !result.value.title.includes(f))
      ) {
        // console.log(white, info, `Skipped by test filter: ${result.value.title}`, reset);
      } else if (local && !result.value.test.environments.includes('viceroy')) {
        console.log(
          white,
          info,
          `Skipped as test marked to only run on Fastly Compute: ${result.value.title}`,
          reset,
        );
      } else if (
        !local &&
        !result.value.test.environments.includes('compute')
      ) {
        console.log(
          white,
          info,
          `Skipped as test marked to only run on local server: ${result.value.title}`,
          reset,
        );
      } else {
        console.log(
          white,
          info,
          `Skipped due to no environments set: ${result.value.title}`,
          reset,
        );
      }
    } else {
      console.log(green, tick, result.value.title, reset);
    }
  } else {
    console.log(red, cross, result.reason, reset);
    failed.push(result.reason);
  }
}
core.endGroup();

if (failed.length) {
  process.exitCode = 1;
  core.startGroup('Failed tests');

  for (const result of failed) {
    console.log(red, cross, result, reset);
  }

  core.endGroup();
}

if (!local && failed.length) {
  core.notice(`Tests failed, the service is named "${serviceName}"`);
  if (domain) {
    core.notice(`You can debug the service on ${domain}`);
  }
}

if (!local && !failed.length) {
  const teardownPath = join(fixturePath, 'teardown.js');
  if (existsSync(teardownPath)) {
    core.startGroup('Tear down the extra set-up for the service');
    await zx`${teardownPath} ${serviceId} ${ci ? serviceName : ''}`;
    core.endGroup();
  }

  core.startGroup('Delete service');
  // Delete the service now the tests have finished
  await zx`fastly service delete --quiet --service-name "${serviceName}" --force --token $FASTLY_API_TOKEN`;
  core.endGroup();
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
