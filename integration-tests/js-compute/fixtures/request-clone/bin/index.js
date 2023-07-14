/* eslint-env serviceworker */
/* global ReadableStream */
import { env } from 'fastly:env';
import { pass, fail, assert, assertThrows } from "../../../assertions.js";

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
routes.set("/request/clone/called-as-constructor", () => {
    let error = assertThrows(() => {
        new Request.prototype.clone()
    }, TypeError, `Request.prototype.clone is not a constructor`)
    if (error) { return error }
    return pass()
});
routes.set("/request/clone/called-unbound", () => {
    let error = assertThrows(() => {
        Request.prototype.clone.call(undefined)
    }, TypeError, "Method clone called on receiver that's not an instance of Request")
    if (error) { return error }
    return pass()
});
routes.set("/request/clone/valid", async () => {
    let request = new Request('https://www.fastly.com', {
        headers: {
            hello: 'world'
        },
        body: 'te',
        method: 'post'
    })
    let newRequest = request.clone();
    let error = assert(newRequest instanceof Request, true, 'newRequest instanceof Request')
    if (error) { return error }
    error = assert(newRequest.method, request.method, 'newRequest.method')
    if (error) { return error }
    error = assert(newRequest.url, request.url, 'newRequest.url')
    if (error) { return error }
    error = assert(newRequest.headers, request.headers, 'newRequest.headers')
    if (error) { return error }
    error = assert(request.bodyUsed, false, 'request.bodyUsed')
    if (error) { return error }
    error = assert(newRequest.bodyUsed, false, 'newRequest.bodyUsed')
    if (error) { return error }
    error = assert(newRequest.body instanceof ReadableStream, true, 'newRequest.body instanceof ReadableStream')
    if (error) { return error }

    request = new Request('https://www.fastly.com', {
        method: 'get'
    })
    newRequest = request.clone()

    error = assert(newRequest.bodyUsed, false, 'newRequest.bodyUsed')
    if (error) { return error }
    error = assert(newRequest.body, null, 'newRequest.body')
    if (error) { return error }
    return pass()
});
routes.set("/request/clone/invalid", async () => {
    const request = new Request('https://www.fastly.com', {
        headers: {
            hello: 'world'
        },
        body: 'te',
        method: 'post'
    })
    await request.text()
    let error = assertThrows(() => request.clone())
    if (error) { return error }
    return pass()
});
