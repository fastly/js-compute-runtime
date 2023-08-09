/* eslint-env serviceworker */
import { env } from 'fastly:env';
import { pass, fail, assert, assertRejects } from "../../../assertions.js";

addEventListener("fetch", event => {
    event.respondWith(app(event))
})
/**
 * @param {FetchEvent} event
 * @returns {Response}
 */
function app(event) {
    try {
        const path = (new URL(event.request.url)).pathname;
        console.log(`path: ${path}`)
        console.log(`FASTLY_SERVICE_VERSION: ${env('FASTLY_SERVICE_VERSION')}`)
        if (routes.has(path)) {
            const routeHandler = routes.get(path);
            return routeHandler()
        }
        return fail(`${path} endpoint does not exist`)
    } catch (error) {
        return fail(`The routeHandler threw an error: ${error.message}` + '\n' + error.stack)
    }
}

const routes = new Map();
routes.set('/', () => {
    routes.delete('/');
    let test_routes = Array.from(routes.keys())
    return new Response(JSON.stringify(test_routes), { 'headers': { 'content-type': 'application/json' } });
});

// Request.fastly.decompressGzip option -- automatic gzip decompression of responses
{
    routes.set("/request/constructor/fastly/decompressGzip/true", async () => {
        const request = new Request('https://httpbin.org/gzip', {
            headers: {
                accept: 'application/json'
            },
            backend: "httpbin",
            fastly: {
                decompressGzip: true
            }
        });
        const response = await fetch(request);
        // This should work because the response will be decompressed and valid json.
        const body = await response.json();
        let error = assert(body.gzipped, true, `body.gzipped`)
        if (error) { return error }
        return pass()
    });
    routes.set("/request/constructor/fastly/decompressGzip/false", async () => {
        const request = new Request('https://httpbin.org/gzip', {
            headers: {
                accept: 'application/json'
            },
            backend: "httpbin",
            fastly: {
                decompressGzip: false
            }
        });
        const response = await fetch(request);
        let error = await assertRejects(async function() {
            // This should throw because the response will be gzipped compressed, which we can not parse as json.
            await response.json();
        });
        if (error) { return error }
        return pass()
    });

    routes.set("/fetch/requestinit/fastly/decompressGzip/true", async () => {
        const response = await fetch('https://httpbin.org/gzip', {
            headers: {
                accept: 'application/json'
            },
            backend: "httpbin",
            fastly: {
                decompressGzip: true
            }
        });
        // This should work because the response will be decompressed and valid json.
        const body = await response.json();
        let error = assert(body.gzipped, true, `body.gzipped`)
        if (error) { return error }
        return pass()
    });
    routes.set("/fetch/requestinit/fastly/decompressGzip/false", async () => {
        const response = await fetch('https://httpbin.org/gzip', {
            headers: {
                accept: 'application/json'
            },
            backend: "httpbin",
            fastly: {
                decompressGzip: false
            }
        });
        let error = await assertRejects(async function() {
            // This should throw because the response will be gzipped compressed, which we can not parse as json.
            await response.json();
        });
        if (error) { return error }
        return pass()
    });
}
