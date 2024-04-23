import * as cacheOverride from 'fastly:cache-override';
import { pass, assert, assertThrows, assertDoesNotThrow } from "./assertions.js";
import { isRunningLocally, routes } from "./routes.js";

const { CacheOverride } = cacheOverride;

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
        if (!isRunningLocally()) {
            const response = await fetch('https://http-me.glitch.me/now?status=200', {
                backend: 'httpme',
                cacheOverride: new CacheOverride('none')
            })
            let error = assert(response.headers.has('x-cache'), true, `CacheOveride('none'); response.headers.has('x-cache') === true`)
            if (error) { return error }
        }
        return pass()
    });
    routes.set("/cache-override/fetch/mode-pass", async () => {
        if (!isRunningLocally()) {
            const response = await fetch('https://http-me.glitch.me/now?status=200', {
                backend: 'httpme',
                cacheOverride: new CacheOverride('pass')
            })
            let error = assert(response.headers.has('x-cache'), false, `CacheOveride('pass'); response.headers.has('x-cache') === false`)
            if (error) { return error }
        }
        return pass()
    });
}

