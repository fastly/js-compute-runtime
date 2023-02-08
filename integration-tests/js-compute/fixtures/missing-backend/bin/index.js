/* eslint-env serviceworker */
/* global ReadableStream ObjectStore ObjectStoreEntry */
import { env } from 'fastly:env';
import { pass, fail, assert, assertThrows, assertRejects, assertResolves } from "../../../assertions.js";

addEventListener("fetch", event => {
    event.respondWith(app(event))
})
/**
 * @param {FetchEvent} event
 * @returns {Response}
 */
async function app(event) {
    try {
        const path = (new URL(event.request.url)).pathname;
        console.log(`path: ${path}`)
        console.log(`FASTLY_SERVICE_VERSION: ${env('FASTLY_SERVICE_VERSION')}`)
        if (routes.has(path)) {
            const routeHandler = routes.get(path);
            return await routeHandler()
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

routes.set("/test", async () => {
  let error = await assertRejects(async () => fetch('https://example.com', {backend: 'missing'}), TypeError, `Requested backend named 'missing' does not exist`)
if (error) { return error }
return pass()
});