/* eslint-env serviceworker */
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
// Request setCacheKey method
{
    routes.set("/request/setCacheKey/called-as-constructor", () => {
        let error = assertThrows(() => {
            new Request.prototype.setCacheKey('1', '1')
        }, TypeError, `Request.prototype.setCacheKey is not a constructor`)
        if (error) { return error }
        return pass()
    });
    routes.set("/request/setCacheKey/called-unbound", () => {
        let error = assertThrows(() => {
            Request.prototype.setCacheKey.call(undefined, '1', '2')
        }, TypeError, "Method setCacheKey called on receiver that's not an instance of Request")
        if (error) { return error }
        return pass()
    });
    // https://tc39.es/ecma262/#sec-tostring
    routes.set("/request/setCacheKey/key-parameter-calls-7.1.17-ToString", () => {
        let sentinel;
        const test = () => {
            sentinel = Symbol();
            const key = {
                toString() {
                    throw sentinel;
                }
            }
            const request = new Request('https://www.fastly.com')
            request.setCacheKey(key)
        }
        let error = assertThrows(test)
        if (error) { return error }
        try {
            test()
        } catch (thrownError) {
            let error = assert(thrownError, sentinel, 'thrownError === sentinel')
            if (error) { return error }
        }
        error = assertThrows(() => {
            const request = new Request('https://www.fastly.com')
            request.setCacheKey(Symbol())
        }, Error, `can't convert symbol to string`)
        if (error) { return error }
        return pass()
    });
    routes.set("/request/setCacheKey/key-parameter-not-supplied", () => {
        let error = assertThrows(() => {
            const request = new Request('https://www.fastly.com')
            request.setCacheKey()
        }, TypeError, `setCacheKey: At least 1 argument required, but only 0 passed`)
        if (error) { return error }
        return pass()
    });
    routes.set("/request/setCacheKey/key-valid", () => {
        const request = new Request('https://www.fastly.com')
        request.setCacheKey('meow')
        let error = assert(request.headers.get('fastly-xqd-cache-key'), '404CDD7BC109C432F8CC2443B45BCFE95980F5107215C645236E577929AC3E52', `request.headers.get('fastly-xqd-cache-key'`)
        if (error) { return error }
        return pass()
    });
    routes.set("/request/constructor/cacheKey", () => {
        const request = new Request('https://www.fastly.com', {cacheKey: 'meow'})
        request.setCacheKey('meow')
        let error = assert(request.headers.get('fastly-xqd-cache-key'), '404CDD7BC109C432F8CC2443B45BCFE95980F5107215C645236E577929AC3E52', `request.headers.get('fastly-xqd-cache-key'`)
        if (error) { return error }
        return pass()
    });
}
