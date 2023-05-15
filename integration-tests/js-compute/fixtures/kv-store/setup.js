#!/usr/bin/env node

import { $ as zx, fetch } from 'zx'
import { retry } from 'zx/experimental'

const startTime = Date.now();

async function $(...args) {
    return await retry(10, () => zx(...args))
}
zx.verbose = false;
if (process.env.FASTLY_API_TOKEN === undefined) {
    try {
        process.env.FASTLY_API_TOKEN = String(await zx`fastly profile token --quiet`).trim()
    } catch {
        console.error('No environment variable named FASTLY_API_TOKEN has been set and no default fastly profile exists.');
        console.error('In order to run the tests, either create a fastly profile using `fastly profile create` or export a fastly token under the name FASTLY_API_TOKEN');
        process.exit(1)
    }
}
const FASTLY_API_TOKEN = process.env.FASTLY_API_TOKEN;

zx.verbose = true;

let stores = await (async function() {
    try {
        // TODO: update this to the kv api when it is ready
        let response = await fetch("https://api.fastly.com/resources/stores/object", {
            method: 'GET',
            headers: {
                "Content-Type": "application/json",
                Accept: "application/json",
                "Fastly-Key": FASTLY_API_TOKEN
            }
        });
        return await response.json();
    } catch {
        return {data:[]}
    }
}())

let STORE_ID = stores.data.find(({ name }) => name === 'example-test-kv-store')?.id
if (!STORE_ID) {
    STORE_ID = await fetch("https://api.fastly.com/resources/stores/object", {
        method: 'POST',
        headers: {
            "Content-Type": "application/json",
            Accept: "application/json",
            "Fastly-Key": FASTLY_API_TOKEN
        },
        body: '{"name":"example-test-kv-store"}'
    })
    STORE_ID = (await STORE_ID.json()).id
}

let VERSION = String(await $`fastly service-version clone --quiet --version=latest --token $FASTLY_API_TOKEN`).trim()
VERSION = VERSION.match(/\d+$/)?.[0]

let SERVICE_ID = await $`fastly service describe --json --quiet --token $FASTLY_API_TOKEN`
SERVICE_ID = JSON.parse(SERVICE_ID).ID
await fetch(`https://api.fastly.com/service/${SERVICE_ID}/version/${VERSION}/resource`, {
    method: 'POST',
    headers: {
        "Content-Type": "application/x-www-form-urlencoded",
        Accept: "application/json",
        "Fastly-Key": FASTLY_API_TOKEN
    },
    body: `name=example-test-kv-store&resource_id=${STORE_ID}`
})
await $`fastly service-version activate --version=${VERSION} --quiet --token $FASTLY_API_TOKEN`

console.log(`Set up has finished! Took ${(Date.now() - startTime) / 1000} seconds to complete`);
