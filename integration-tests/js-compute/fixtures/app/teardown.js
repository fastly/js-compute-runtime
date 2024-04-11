#!/usr/bin/env node

import { $ as zx } from 'zx'

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

async function removeConfigStore() {
    let stores = await (async function () {
        try {
            return JSON.parse(await zx`fastly config-store list --quiet --json --token $FASTLY_API_TOKEN`)
        } catch {
            return []
        }
    }())
    let links = await (async function () {
        try {
            return JSON.parse(await zx`fastly resource-link list --quiet --json --version latest --token $FASTLY_API_TOKEN`)
        } catch {
            return []
        }
    }())

    const STORE_ID = stores.find(({ name }) => name === 'aZ1 __ 2')?.id
    if (STORE_ID) {
        process.env.STORE_ID = STORE_ID;
        let LINK_ID = links.find(({ resource_id }) => resource_id == STORE_ID)?.id;
        if (LINK_ID) {
            process.env.LINK_ID = LINK_ID;
            try {
                await zx`fastly resource-link delete --version latest --autoclone --id=$LINK_ID  --token $FASTLY_API_TOKEN`
                await zx`fastly service-version activate --version latest --token $FASTLY_API_TOKEN`
            } catch { }
        }
        try {
            await zx`fastly config-store delete --store-id=$STORE_ID  --token $FASTLY_API_TOKEN`
        } catch { }
    }
}

async function removeKVStore() {
    let stores = await fetch("https://api.fastly.com/resources/stores/object", {
        method: 'GET',
        headers: {
            "Content-Type": "application/json",
            Accept: "application/json",
            "Fastly-Key": FASTLY_API_TOKEN
        }
    }).then(res => res.json())


    let STORE_ID = stores.data.find(({ name }) => name === 'example-test-kv-store')?.id
    if (STORE_ID) {
        await fetch(`https://api.fastly.com/resources/stores/object/${STORE_ID}`, {
            method: 'DELETE',
            headers: {
                "Fastly-Key": FASTLY_API_TOKEN
            }
        })
    }
}

async function removeSecretStore() {
    let stores = await (async function() {
        try {
            return JSON.parse(await zx`fastly secret-store list --quiet --json --token $FASTLY_API_TOKEN`)
        } catch {
            return []
        }
    }())
    let links = await (async function() {
        try {
            return JSON.parse(await zx`fastly resource-link list --quiet --json --version latest --token $FASTLY_API_TOKEN`)
        } catch {
            return []
        }
    }())
    
    const STORE_ID = stores.find(({ name }) => name === 'example-test-secret-store')?.id
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
}

await removeConfigStore()
await removeKVStore()
await removeSecretStore()

console.log(`Tear down has finished! Took ${(Date.now() - startTime) / 1000} seconds to complete`);
