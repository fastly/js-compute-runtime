import { $ as zx, fetch } from 'zx'
import { retry } from 'zx/experimental'

zx.verbose = false
async function $(...args) {
    return await retry(10, () => zx(...args))
}

const startTime = Date.now();
let domain = await $`fastly domain list --version latest --json`
domain = JSON.parse(domain)[0].Name

let FASTLY_KEY = String(await $`fastly profile token `).trim()

let stores = await fetch("https://api.fastly.com/resources/stores/object", {
    method: 'GET',
    headers: {
        "Content-Type": "application/json",
        Accept: "application/json",
        "Fastly-Key": FASTLY_KEY
    }
})
let STORE_ID = (await stores.json()).data.find(({ name }) => name === 'test-store')?.id
if (!STORE_ID) {
    STORE_ID = await fetch("https://api.fastly.com/resources/stores/object", {
        method: 'POST',
        headers: {
            "Content-Type": "application/json",
            Accept: "application/json",
            "Fastly-Key": FASTLY_KEY
        },
        body: '{"name":"test-store"}'
    })
    STORE_ID = (await STORE_ID.json()).id

    let VERSION = String(await $`fastly service-version clone --version=latest`).trim()
    VERSION = VERSION.match(/\d+$/)?.[0]

    let SERVICE_ID = await $`fastly service describe --json`
    SERVICE_ID = JSON.parse(SERVICE_ID).ID
    await fetch(`https://api.fastly.com/service/${SERVICE_ID}/version/${VERSION}/resource`, {
        method: 'POST',
        headers: {
            "Content-Type": "application/x-www-form-urlencoded",
            Accept: "application/json",
            "Fastly-Key": FASTLY_KEY
        },
        body: `name=test-store&resource_id=${STORE_ID}`
    })
    await $`fastly service-version activate --version=${VERSION}`
}


let test_paths = await (await fetch(`https://${domain}`)).json()
let counter = 0;
try {
    await Promise.all(test_paths.map(async path => {
        return retry(10, async () => {
            let url = `https://${domain}${path}`
            let response = await fetch(url);
            let body = await response.text()
            if (response.ok) {
                console.log(`Passed ${url}`)
                counter += 1;
            } else {
                console.log()
                console.log(`Failed ${url}`)
                console.log()
                console.error(body)
                process.exit()
            }
        })
    }))
    console.log(`All ${counter} tests passed! Took ${(Date.now() - startTime) / 1000} seconds to complete`)
} catch (error) {
    console.error(error)
    process.exitCode = 1;
}

// await fetch(`https://api.fastly.com/resources/stores/object/${STORE_ID}`, {
//     method: 'DELETE',
//     headers: {
//         "Fastly-Key": FASTLY_KEY
//     }
// })
