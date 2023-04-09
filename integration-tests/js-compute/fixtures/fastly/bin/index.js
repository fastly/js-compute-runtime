/* eslint-env serviceworker */

import { env } from 'fastly:env';
import { pass, fail, assert } from "../../../assertions.js";

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
// fastly.now
{
    routes.set("/fastly/now", function () {
        let error = assert(typeof fastly.now, 'function', 'typeof fastly.now')
        if (error) { return error }

        error = assert(fastly.now.name, 'now', 'fastly.now.name')
        if (error) { return error }

        error = assert(fastly.now.length, 0, 'fastly.now.length')
        if (error) { return error }

        error = assert(typeof fastly.now(), 'number', `typeof fastly.now()`)
        if (error) { return error }

        error = assert(fastly.now() > Date.now(), true, `fastly.now() > Date.now()`)
        if (error) { return error }

        console.log(fastly.now())

        return pass()
    })
}
