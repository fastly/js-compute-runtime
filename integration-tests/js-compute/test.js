#!/usr/bin/env node

import { fileURLToPath } from 'node:url';
import { dirname, join } from 'node:path';
import { cd, $ as zx, } from 'zx'
import { request } from 'undici'
import { retry, expBackoff } from 'zx/experimental'
import { compareDownstreamResponse } from "./compare-downstream-response.js";
import { argv } from "node:process";
import { existsSync } from "node:fs";
import { copyFile, readFile, writeFile } from "node:fs/promises";
import core from '@actions/core';
import TOML from '@iarna/toml'

async function killPortProcess(port) {
    zx.verbose = false;
    const pids = (await zx`lsof -ti:${port}`).stdout
    if (pids) {
        for (const pid of pids.split('\n').reverse()) {
            if (pid && pid != process.pid) {
                await zx`kill -15 ${pid}`
            }
        }
    }
}

const startTime = Date.now();
const __dirname = dirname(fileURLToPath(import.meta.url));

async function sleep(seconds) {
    return new Promise(resolve => {
        setTimeout(resolve, 1_000 * seconds)
    })
}

let args = argv.slice(2);

const local = args.includes('--local');
const starlingmonkey = args.includes('--starlingmonkey');

async function $(...args) {
    return await retry(10, () => zx(...args))
}

if (!local && process.env.FASTLY_API_TOKEN === undefined) {
    try {
        zx.verbose = false;
        process.env.FASTLY_API_TOKEN = String(await zx`fastly profile token --quiet`).trim()
    } catch {
        console.error('No environment variable named FASTLY_API_TOKEN has been set and no default fastly profile exists.');
        console.error('In order to run the tests, either create a fastly profile using `fastly profile create` or export a fastly token under the name FASTLY_API_TOKEN');
        process.exit(1)
    }
}

const FASTLY_API_TOKEN = process.env.FASTLY_API_TOKEN;
zx.verbose = true;
const branchName = (await zx`git branch --show-current`).stdout.trim().replace(/[^a-zA-Z0-9_-]/g, '_')

const fixture = 'app';
const serviceName = `${fixture}--${branchName}${starlingmonkey ? '--sm' : ''}`
let domain;
const fixturePath = join(__dirname, 'fixtures', fixture)
let localServer;

await cd(fixturePath);
await copyFile(join(fixturePath, 'fastly.toml.in'), join(fixturePath, 'fastly.toml'))
const config = TOML.parse(await readFile(join(fixturePath, 'fastly.toml'), 'utf-8'))
config.name = serviceName;
if (starlingmonkey) {
    const buildArgs = config.scripts.build.split(' ')
    buildArgs.splice(-1, null, '--starlingmonkey')
    config.scripts.build = buildArgs.join(' ')
}
await writeFile(join(fixturePath, 'fastly.toml'), TOML.stringify(config), 'utf-8')
if (!local) {
    core.startGroup('Delete service if already exists')
    try {
        await zx`fastly service delete --quiet --service-name "${serviceName}" --force --token $FASTLY_API_TOKEN`
    } catch { }
    core.endGroup()
    core.startGroup('Build and deploy service')
    await zx`npm i`
    await $`fastly compute publish -i --quiet --token $FASTLY_API_TOKEN --status-check-off`
    core.endGroup()

    // get the public domain of the deployed application
    domain = 'https://' + JSON.parse(await $`fastly domain list --quiet --version latest --json`)[0].Name
    core.notice(`Service is running on ${domain}`)

    const setupPath = join(fixturePath, 'setup.js')
    if (existsSync(setupPath)) {
        core.startGroup('Extra set-up steps for the service')
        await zx`node ${setupPath} ${serviceName} ${starlingmonkey ? '--starlingmonkey' : ''}`
        await sleep(60)
        core.endGroup()
    }
} else {
    localServer = zx`fastly compute serve --verbose`
    domain = "http://127.0.0.1:7676"
}

core.startGroup('Check service is up and running')
await retry(10, expBackoff('60s', '30s'), async () => {
    const response = await request(domain)
    if (response.statusCode !== 200) {
        throw new Error(`Application "${fixture}" :: Not yet available on domain: ${domain}`)
    }
})
core.endGroup()

let { default: tests } = await import(join(fixturePath, 'tests.json'), { with: { type: 'json' } });

if (starlingmonkey) {
    const { default: testsStarlingMonkey } = await import(join(fixturePath, 'tests-starlingmonkey.json'), { with: { type: 'json' } });
    const testCnt = Object.keys(tests).length;
    const starlingTestCnt = testsStarlingMonkey.length;
    if (process.env.GITHUB_ACTIONS) {
        await core.summary
        .addHeading('StarlingMonkey Progress')
        .addRaw(`
\`\`\`mermaid
pie showData
    title 🐦🐵 Test Progress 🚀🚀
    "Remaining Tests" : ${testCnt - starlingTestCnt}
    "StarlingMonkey Tests" : ${starlingTestCnt}
\`\`\``, true).write();
    }
    tests = Object.fromEntries(Object.entries(tests).filter(([key]) => testsStarlingMonkey.includes(key)));
}

core.startGroup('Running tests')
function chunks(arr, size) {
    const output = [];
    for (let i = 0; i < arr.length; i += size) {
        output.push(arr.slice(i, i + size));
    }
    return output;
}

let results = [];
for (const chunk of chunks(Object.entries(tests), 100)) {
    results.push(...await Promise.allSettled(chunk.map(async ([title, test]) => {
        if (local) {
            if (test.environments.includes("viceroy")) {
                let path = test.downstream_request.pathname;
                let url = `${domain}${path}`
                try {
                    let response = await request(url, {
                        method: test.downstream_request.method || 'GET',
                        headers: test.downstream_request.headers || undefined,
                        body: test.downstream_request.body || undefined,
                    });
    
                    await compareDownstreamResponse(test.downstream_response, response);
                    return {
                        title,
                        skipped: false
                    };
                } catch (error) {
                    throw new Error(`${title} ${error.message}`);
                }
            } else {
                return {
                    title,
                    skipped: true
                }
            }
        } else  {
            if (test.environments.includes("compute")) {
                return retry(10, expBackoff('60s', '10s'), async () => {
                    let path = test.downstream_request.pathname;
                    let url = `${domain}${path}`
                    try {
                        let response = await request(url, {
                            method: test.downstream_request.method || 'GET',
                            headers: test.downstream_request.headers || undefined,
                            body: test.downstream_request.body || undefined,
                        });
    
                        await compareDownstreamResponse(test.downstream_response, response);
                        return {
                            title,
                            skipped: false
                        };
                    } catch (error) {
                        throw new Error(`${title} ${error.message}`);
                    }
                })
            } else {
                return {
                    title,
                    skipped: true
                }
            }
        }
    })))
}
core.endGroup()

console.log('Test results')
core.startGroup('Test results')
let passed = 0;
const failed = [];
const green = '\u001b[32m';
const red = '\u001b[31m';
const white = '\u001b[39m';
const info = '\u2139';
const tick = '\u2714';
const cross = '\u2716';
for (const result of results) {
    if (result.status === 'fulfilled') {
        passed += 1;
        if (result.value.skipped) {
            if (local) {
                console.log(white, info, `Skipped as test marked to only run on Fastly Compute: ${result.value.title}`, white);
            } else {
                console.log(white, info, `Skipped as test marked to only run on local server: ${result.value.title}`, white);
            }
        } else {
            console.log(green, tick, result.value.title, white);
        }
    } else {
        console.log(red, cross, result.reason, white);
        failed.push(result.reason)
    }
}
core.endGroup()

if (failed.length) {
    process.exitCode = 1;
    core.startGroup('Failed tests')

    for (const result of failed) {
        console.log(red, cross, result, white);
    }

    core.endGroup()
}


if (!local && failed.length) {
    core.notice(`Tests failed, the service is named "${serviceName}"`)
    if (domain) {
        core.notice(`You can debug the service on ${domain}`)
    }
}

if (!local && !failed.length) {
    const teardownPath = join(fixturePath, 'teardown.js')
    if (existsSync(teardownPath)) {
        core.startGroup('Tear down the extra set-up for the service')
        await zx`${teardownPath}`
        core.endGroup()
    }

    core.startGroup('Delete service')
    // Delete the service now the tests have finished
    await zx`fastly service delete --quiet --service-name "${serviceName}" --force --token $FASTLY_API_TOKEN`
    core.endGroup()
}
if (process.exitCode == undefined || process.exitCode == 0) {
    console.log(`All tests passed! Took ${(Date.now() - startTime) / 1000} seconds to complete`);
} else {
    console.log(`Tests failed!`);
}
if (localServer) {
    await killPortProcess(7676)
}
process.exit()

