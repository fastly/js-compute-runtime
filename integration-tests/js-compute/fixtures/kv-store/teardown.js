#!/usr/bin/env node

import { $ as zx, fetch } from 'zx'

const startTime = Date.now();


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
// TODO: update this to the kv api when it is ready
let stores = await fetch("https://api.fastly.com/resources/stores/object", {
    method: 'GET',
    headers: {
        "Content-Type": "application/json",
        Accept: "application/json",
        "Fastly-Key": FASTLY_API_TOKEN
    }
})

let STORE_ID = (await stores.json()).data.find(({ name }) => name === 'example-test-kv-store')?.id
if (STORE_ID) {
    await fetch(`https://api.fastly.com/resources/stores/object/${STORE_ID}`, {
        method: 'DELETE',
        headers: {
            "Fastly-Key": FASTLY_API_TOKEN
        }
    })
}

console.log(`Tear down has finished! Took ${(Date.now() - startTime) / 1000} seconds to complete`);