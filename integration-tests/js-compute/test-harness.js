/* eslint-env serviceworker */
import { env } from 'fastly:env';
import { fail } from "./assertions.js";

addEventListener("fetch", event => {
    event.respondWith(app(event))
})

export let FASTLY_SERVICE_VERSION;

/**
 * @param {FetchEvent} event
 * @returns {Response}
 */
async function app(event) {
    let res = new Response('Internal Server Error', { status: 500 });
    const path = (new URL(event.request.url)).pathname
    console.log(`path: ${path}`)
    try {
        FASTLY_SERVICE_VERSION = env('FASTLY_SERVICE_VERSION') || 'local'
        console.log(`FASTLY_SERVICE_VERSION: ${FASTLY_SERVICE_VERSION}`)
        if (routes.has(path)) {
            const routeHandler = routes.get(path)
            res = await routeHandler(event)
        } else {
            res = fail(`${path} endpoint does not exist`)
        }
    } catch (error) {
        res = fail(`The routeHandler for ${path} threw an error: ${error.message || error}` + '\n' + error.stack)
    } finally {
        res.headers.set('fastly_service_version', FASTLY_SERVICE_VERSION);
    }
    return res;
}

export const routes = new Map()
routes.set('/', () => {
    routes.delete('/')
    let test_routes = Array.from(routes.keys())
    return new Response(JSON.stringify(test_routes), { 'headers': { 'content-type': 'application/json' } })
})
