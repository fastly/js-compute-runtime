/* global Request, RequestEntry, fastly */
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
        console.log(`FASTLY_SERVICE_VERSION: ${fastly.env.get('FASTLY_SERVICE_VERSION')}`)
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
        let error = assert(request.headers.get('fastly-xqd-cache-key'), '404cdd7bc109c432f8cc2443b45bcfe95980f5107215c645236e577929ac3e52', `request.headers.get('fastly-xqd-cache-key'`)
        if (error) { return error }
        return pass()
    });
    routes.set("/request/constructor/cacheKey", () => {
        const request = new Request('https://www.fastly.com', {cacheKey: 'meow'})
        request.setCacheKey('meow')
        let error = assert(request.headers.get('fastly-xqd-cache-key'), '404cdd7bc109c432f8cc2443b45bcfe95980f5107215c645236e577929ac3e52', `request.headers.get('fastly-xqd-cache-key'`)
        if (error) { return error }
        return pass()
    });
}

// Testing/Assertion functions //

function pass(message = '') {
    return new Response(message)
}

function fail(message = '') {
    return new Response(message, { status: 500 })
}

function assert(actual, expected, code) {
    if (!deepEqual(actual, expected)) {
        return fail(`Expected \`${code}\` to equal \`${JSON.stringify(expected)}\` - Found \`${JSON.stringify(actual)}\``)
    }
}

function assertResolves(func) {
    try {
        func()
    } catch (error) {
        return fail(`Expected \`${func.toString()}\` to resolve - Found it rejected: ${error.name}: ${error.message}`)
    }
}

function assertThrows(func, errorClass, errorMessage) {
    try {
        func()
        return fail(`Expected \`${func.toString()}\` to reject - Found it did not reject`)
    } catch (error) {
        if (errorClass) {
            if ((error instanceof errorClass) === false) {
                return fail(`Expected \`${func.toString()}\` to reject instance of \`${errorClass.name}\` - Found instance of \`${error.name}\``)
            }
        }

        if (errorMessage) {
            if (error.message !== errorMessage) {
                return fail(`Expected \`${func.toString()}\` to reject error message of \`${errorMessage}\` - Found \`${error.message}\``)
            }
        }
    }
}

function assertThrows(func, errorClass, errorMessage) {
    try {
        func()
        return fail(`Expected \`${func.toString()}\` to throw - Found it did not throw`)
    } catch (error) {
        if (errorClass) {
            if ((error instanceof errorClass) === false) {
                return fail(`Expected \`${func.toString()}\` to throw instance of \`${errorClass.name}\` - Found instance of \`${error.name}\``)
            }
        }

        if (errorMessage) {
            if (error.message !== errorMessage) {
                return fail(`Expected \`${func.toString()}\` to throw error message of \`${errorMessage}\` - Found \`${error.message}\``)
            }
        }
    }
}

// eslint-disable-next-line no-unused-vars
function assertDoesNotThrow(func) {
    try {
        func()
    } catch (error) {
        return fail(`Expected \`${func.toString()}\` to not throw - Found it did throw: ${error.name}: ${error.message}`)
    }
}

/**
* Tests for deep equality between two values.
*
* @param {*} a - first comparison value
* @param {*} b - second comparison value
* @returns {boolean} boolean indicating if `a` is deep equal to `b`
*
* @example
* var bool = deepEqual( [ 1, 2, 3 ], [ 1, 2, 3 ] );
* // returns true
*
* @example
* var bool = deepEqual( [ 1, 2, 3 ], [ 1, 2, '3' ] );
* // returns false
*
* @example
* var bool = deepEqual( { 'a': 2 }, { 'a': [ 2 ] } );
* // returns false
*
* @example
* var bool = deepEqual( [], {} );
* // returns false
*
* @example
* var bool = deepEqual( null, null );
* // returns true
*/
function deepEqual(a, b) {
    var aKeys;
    var bKeys;
    var typeA;
    var typeB;
    var key;
    var i;

    typeA = typeof a;
    typeB = typeof b;
    if (a === null || typeA !== 'object') {
        if (b === null || typeB !== 'object') {
            return a === b;
        }
        return false;
    }
    // Case: `a` is of type 'object'
    if (typeB !== 'object') {
        return false;
    }
    if (Object.getPrototypeOf(a) !== Object.getPrototypeOf(b)) {
        return false;
    }
    if (a instanceof Date) {
        return a.getTime() === b.getTime();
    }
    if (a instanceof RegExp) {
        return a.source === b.source && a.flags === b.flags;
    }
    if (a instanceof Error) {
        if (a.message !== b.message || a.name !== b.name) {
            return false;
        }
    }

    aKeys = Object.keys(a);
    bKeys = Object.keys(b);
    if (aKeys.length !== bKeys.length) {
        return false;
    }
    aKeys.sort();
    bKeys.sort();

    // Cheap key test:
    for (i = 0; i < aKeys.length; i++) {
        if (aKeys[i] !== bKeys[i]) {
            return false;
        }
    }
    // Possibly expensive deep equality test for each corresponding key:
    for (i = 0; i < aKeys.length; i++) {
        key = aKeys[i];
        if (!deepEqual(a[key], b[key])) {
            return false;
        }
    }
    return typeA === typeB;
}
