/* eslint-env serviceworker */
/// <reference path="../../../../../types/index.d.ts" />

import { env } from 'fastly:env';
import { CacheOverride } from 'fastly:cache-override';
import { pass, fail, assert, assertThrows, assertDoesNotThrow } from "../../../assertions.js";

addEventListener("fetch", event => event.respondWith(app(event)));

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

// CacheOverride
{
    // CacheOverride constructor
    {

        routes.set("/cache-override/constructor/called-as-regular-function", async () => {
            let error = assertThrows(() => {
                CacheOverride()
            }, TypeError, `calling a builtin CacheOverride constructor without new is forbidden`)
            if (error) { return error }
            return pass()
        });
        // https://tc39.es/ecma262/#sec-tostring
        routes.set("/cache-override/constructor/parameter-calls-7.1.17-ToString", async () => {
            let sentinel;
            const test = () => {
                sentinel = Symbol();
                const name = {
                    toString() {
                        throw sentinel;
                    }
                }
                new CacheOverride(name)
            }
            let error = assertThrows(test)
            if (error) { return error }
            try {
                test()
            } catch (thrownError) {
                let error = assert(thrownError, sentinel, 'thrownError === sentinel')
                if (error) { return error }
            }
            error = assertThrows(() => new CacheOverride(Symbol()), TypeError, `can't convert symbol to string`)
            if (error) { return error }
            return pass()
        });
        routes.set("/cache-override/constructor/empty-parameter", async () => {
            let error = assertThrows(() => {
                new CacheOverride()
            }, TypeError, `CacheOverride constructor: At least 1 argument required, but only 0 passed`)
            if (error) { return error }
            return pass()
        });
        routes.set("/cache-override/constructor/invalid-mode", async () => {
            // empty string not allowed
            let error = assertThrows(() => {
                new CacheOverride('')
            }, TypeError, `CacheOverride constructor: 'mode' has to be "none", "pass", or "override", but got ""`)
            if (error) { return error }

            error = assertThrows(() => {
                new CacheOverride('be nice to the cache')
            }, TypeError, `CacheOverride constructor: 'mode' has to be "none", "pass", or "override", but got "be nice to the cache"`)
            if (error) { return error }
            return pass()
        });
        routes.set("/cache-override/constructor/valid-mode", async () => {
            let error = assertDoesNotThrow(() => {
                new CacheOverride('none')
            })
            if (error) { return error }
            error = assertDoesNotThrow(() => {
                new CacheOverride('pass')
            })
            if (error) { return error }
            error = assertDoesNotThrow(() => {
                new CacheOverride('override', {})
            })
            if (error) { return error }
            return pass()
        });
    }
    // Using CacheOverride
    {
        routes.set("/cache-override/fetch/mode-none", async () => {
            const response = await fetch('https://httpbin.org/status/200', {
                backend: 'httpbin',
                cacheOverride: new CacheOverride('none')
            })
            let error = assert(response.headers.has('x-cache'), true, `CacheOveride('none'); response.headers.has('x-cache') === true`)
            if (error) { return error }
            return pass()
        });
        routes.set("/cache-override/fetch/mode-pass", async () => {
            const response = await fetch('https://httpbin.org/status/200', {
                backend: 'httpbin',
                cacheOverride: new CacheOverride('pass')
            })
            let error = assert(response.headers.has('x-cache'), false, `CacheOveride('pass'); response.headers.has('x-cache') === false`)
            if (error) { return error }
            return pass()
        });
    }
}
