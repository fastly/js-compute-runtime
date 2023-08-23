#!/usr/bin/env node

import { fileURLToPath } from 'node:url';
import { dirname, join } from 'node:path';
import { cd, $ as zx, within } from 'zx'
import { request } from 'undici'
import { retry, expBackoff } from 'zx/experimental'
import { compareDownstreamResponse } from "./compare-downstream-response.js";
import { argv, exit } from "node:process";
import { existsSync } from "node:fs";
import { copyFile, readdir, readFile, writeFile } from "node:fs/promises";
import core from '@actions/core';
import TOML from '@iarna/toml'

const startTime = Date.now();
const __dirname = dirname(fileURLToPath(import.meta.url));

async function sleep(seconds) {
    return new Promise(resolve => {
        setTimeout(resolve, 1_000 * seconds)
    })
}

let testFixtures = argv.slice(2);
const existingFixtures = await fixturesWithComputeTests();
if (testFixtures.length === 0) {
    console.error('\nUsage: $0 <test-fixture...>\n')
    console.error('No fixture names were given as arguments.')

    console.error(`\nThe available fixtures are: \n - ${existingFixtures.join('\n - ')}\n`)
    console.error(`Run all the fixtures by setting the flag --all`)
    exit(1)
}

if (testFixtures.length === 1 && testFixtures[0] === '--all') {
    testFixtures = existingFixtures;
}

for (const fixture of testFixtures) {
    if (existingFixtures.includes(fixture) === false) {
        console.error('\nUsage: $0 <test-fixture...>\n')
        console.error(`No fixture exists named ${fixture}\n`)

        console.error(`\nThe available fixtures are: \n - ${existingFixtures.join('\n - ')}`)
        exit(1)
    }
}

async function fixturesWithComputeTests() {
    const fixturesPath = await join(__dirname, 'fixtures');
    const fixtureNames = await readdir(fixturesPath);
    const fixtures = [];
    for (const fixtureName of fixtureNames) {
        const testManifestPath = join(fixturesPath, fixtureName, 'tests.json')
        if (existsSync(testManifestPath)) {
            const tests = JSON.parse(await readFile(testManifestPath, {encoding:'utf-8'}));
            for (const test of Object.values(tests)) {
                if (test.environments.includes("c@e")) {
                    fixtures.push(fixtureName)
                    break
                }
            }
        }
    }
    return fixtures
}

async function $(...args) {
    return await retry(10, () => zx(...args))
}

if (process.env.FASTLY_API_TOKEN === undefined) {
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
const branchName = (await $`git branch --show-current`).stdout.trim().replace(/[^a-zA-Z0-9_-]/g, '_')

for (const fixture of testFixtures) {
    const serviceName = `${fixture}--${branchName}`
    let domain;
    await within(async () => {
        const fixturePath = join(__dirname, 'fixtures', fixture)
        try {
            const startTime = Date.now();
            await cd(fixturePath);
            await copyFile(join(fixturePath, 'fastly.toml.in'), join(fixturePath, 'fastly.toml'))
            const config = TOML.parse(await readFile(join(fixturePath, 'fastly.toml'), 'utf-8'))
            config.name = serviceName;
            await writeFile(join(fixturePath, 'fastly.toml'), TOML.stringify(config), 'utf-8')
            core.startGroup('Delete service if already exists')
            try {
                await zx`fastly service delete --quiet --service-name "${serviceName}" --force --token $FASTLY_API_TOKEN`
            } catch {}
            core.endGroup()
            core.startGroup('Build and deploy service')
            await zx`npm i`
            await $`fastly compute publish -i --quiet --token $FASTLY_API_TOKEN --status-check-off`
            core.endGroup()

            // get the public domain of the deployed application
            domain = JSON.parse(await $`fastly domain list --quiet --version latest --json`)[0].Name
            core.notice(`Service is running on https://${domain}`)

            const setupPath = join(fixturePath, 'setup.js')
            if (existsSync(setupPath)) {
                core.startGroup('Extra set-up steps for the service')
                await $`${setupPath}`
                await sleep(60)
                core.endGroup()
            }

            core.startGroup('Check service is up and running')
            await retry(10, expBackoff('60s', '30s'), async () => {
                const response = await request(`https://${domain}`)
                if (response.statusCode !== 200) {
                    throw new Error(`Application "${fixture}" :: Not yet available on domain: ${domain}`)
                }
            })
            core.endGroup()

            const { default: tests } = await import(join(fixturePath, 'tests.json'), { assert: { type: 'json' } });

            core.startGroup('Running tests')
            let counter = 0;
            await Promise.all(Object.entries(tests).map(async ([title, test]) => {
                if (test.environments.includes("c@e")) {
                    return retry(10, expBackoff('60s', '10s'), async () => {
                        let path = test.downstream_request.pathname;
                        let url = `https://${domain}${path}`
                        let response = await request(url, {
                            method: test.downstream_request.method || 'GET',
                            headers: test.downstream_request.headers || undefined,
                            body: test.downstream_request.body || undefined,
                        });

                        await compareDownstreamResponse(test.downstream_response, response);
                        counter += 1;
                    })
                }
            }))
            core.endGroup()
            console.log(`Application "${fixture}" :: All ${counter} tests passed! Took ${(Date.now() - startTime) / 1000} seconds to complete`)
            const teardownPath = join(fixturePath, 'teardown.js')
            if (existsSync(teardownPath)) {
                core.startGroup('Tear down the extra set-up for the service')
                await $`${teardownPath}`
                core.endGroup()
            }

            core.startGroup('Delete service')
            // Delete the service now the tests have finished
            await zx`fastly service delete --quiet --service-name "${serviceName}" --force --token $FASTLY_API_TOKEN`
            core.endGroup()
        } catch (error) {
            console.error(`Application "${fixture}" :: ${error.message}`)
            core.notice(`Tests failed, the service is named "${serviceName}"`)
            if (domain) {
                core.notice(`You can debug the service on https://${domain}`)
            }
            process.exitCode = 1;
        }

    })
}

if (process.exitCode == undefined || process.exitCode == 0) {
    console.log(`All tests passed! Took ${(Date.now() - startTime) / 1000} seconds to complete`);
} else {
    console.log(`Tests failed!`);
    process.exit()
}
