#!/usr/bin/env node

import { $ as zx } from 'zx'
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

zx.verbose = true;

let stores = await (async function() {
    try {
        return JSON.parse(await zx`fastly secret-store list --json --token $FASTLY_API_TOKEN`)
    } catch {
        return {data:[]}
    }
}())

process.env.STORE_ID = stores.data.find(({ name }) => name === 'example-test-secret-store')?.id
if (!process.env.STORE_ID) {
    process.env.STORE_ID = JSON.parse(await zx`fastly secret-store create --name=example-test-secret-store --json --token $FASTLY_API_TOKEN`).id
}

try {
    await zx`echo -n 'This is also some secret data' | fastly secret-store-entry create --name first --store-id=$STORE_ID --stdin --token $FASTLY_API_TOKEN`
} catch {}
try {
let key = 'aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa'
    await zx`echo -n 'This is some secret data' | fastly secret-store-entry create --name ${key} --store-id=$STORE_ID --stdin --token $FASTLY_API_TOKEN`
} catch {}

await zx`fastly resource-link create --version latest --resource-id $STORE_ID --token $FASTLY_API_TOKEN --autoclone`

console.log(`Set up has finished! Took ${(Date.now() - startTime) / 1000} seconds to complete`);