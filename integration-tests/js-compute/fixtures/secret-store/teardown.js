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
zx.verbose = true;
let stores = await (async function() {
    try {
        return JSON.parse(await zx`fastly secret-store list --json --token $FASTLY_API_TOKEN`)
    } catch {
        return {data:[]}
    }
}())
let links = await (async function() {
    try {
        return JSON.parse(await zx`fastly resource-link list --json --version latest --token $FASTLY_API_TOKEN`)
    } catch {
        return []
    }
}())

const STORE_ID = stores.data.find(({ name }) => name === 'example-test-secret-store')?.id
if (STORE_ID) {
    process.env.STORE_ID = STORE_ID;
    let LINK_ID = links.find(({resource_id}) => resource_id == STORE_ID)?.id;
    if (LINK_ID) {
        process.env.LINK_ID = LINK_ID;
        try {
            await zx`fastly resource-link delete --version latest --autoclone --id=$LINK_ID  --token $FASTLY_API_TOKEN`
            await zx`fastly service-version activate --version latest --token $FASTLY_API_TOKEN`
        } catch {}
    }
    try {
        await zx`fastly secret-store delete --store-id=$STORE_ID  --token $FASTLY_API_TOKEN`
    } catch {}
}

console.log(`Tear down has finished! Took ${(Date.now() - startTime) / 1000} seconds to complete`);