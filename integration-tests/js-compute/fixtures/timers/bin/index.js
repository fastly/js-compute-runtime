/* eslint-env serviceworker */

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
        console.log(`FASTLY_SERVICE_VERSION: ${fastly.env.get('FASTLY_SERVICE_VERSION')}`)
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
// setInterval
{
    routes.set("/setInterval/exposed-as-global", async () => {
        let error = assert(typeof setInterval, 'function', `typeof setInterval`)
        if (error) { return error }
        return pass()
    });
    routes.set("/setInterval/interface", async () => {
        let actual = Reflect.getOwnPropertyDescriptor(globalThis, 'setInterval')
        let expected = {
            writable: true,
            enumerable: true,
            configurable: true,
            value: globalThis.setInterval
        }
        let error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(globalThis, 'setInterval)`)
        if (error) { return error }

        error = assert(typeof globalThis.setInterval, 'function', `typeof globalThis.setInterval`)
        if (error) { return error }

        actual = Reflect.getOwnPropertyDescriptor(globalThis.setInterval, 'length')
        expected = {
            value: 1,
            writable: false,
            enumerable: false,
            configurable: true
        }
        error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(globalThis.setInterval, 'length')`)
        if (error) { return error }

        actual = Reflect.getOwnPropertyDescriptor(globalThis.setInterval, 'name')
        expected = {
            value: "setInterval",
            writable: false,
            enumerable: false,
            configurable: true
        }
        error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(globalThis.setInterval, 'name')`)
        if (error) { return error }

        return pass()
    });
    routes.set("/setInterval/called-as-constructor-function", async () => {
        let error = assertThrows(() => {
            new setInterval
        }, TypeError, `setInterval is not a constructor`)
        if (error) { return error }
        return pass()
    });
    routes.set("/setInterval/empty-parameter", async () => {
        let error = assertThrows(() => {
            setInterval()
        }, TypeError, `setInterval: At least 1 argument required, but only 0 passed`)
        if (error) { return error }
        return pass()
    });
    routes.set("/setInterval/handler-parameter-not-supplied", async () => {
        let error = assertThrows(() => {
            setInterval()
        }, TypeError, `setInterval: At least 1 argument required, but only 0 passed`)
        if (error) { return error }
        return pass()
    });
    routes.set("/setInterval/handler-parameter-not-callable", async () => {
        let non_callable_types = [
            // Primitive types
            null,
            undefined,
            true,
            1,
            1n,
            'hello',
            Symbol(),
            // After primitive types, the only remaining types are Objects and Functions
            {},
        ];
        for (const type of non_callable_types) {
            let error = assertThrows(() => {
                setInterval(type)
                // TODO: Make a TypeError
            }, Error, `First argument to setInterval must be a function`)
            if (error) { return error }
        }
        return pass()
    });
    routes.set("/setInterval/timeout-parameter-not-supplied", async () => {
        let error = assertDoesNotThrow(() => {
            setInterval(function () { })
        })
        if (error) { return error }
        return pass()
    });
    // https://tc39.es/ecma262/#sec-tonumber
    routes.set("/setInterval/timeout-parameter-calls-7.1.4-ToNumber", async () => {
        let sentinel;
        let requestedType;
        const test = () => {
            sentinel = Symbol();
            const timeout = {
                [Symbol.toPrimitive](type) {
                    requestedType = type;
                    throw sentinel;
                }
            }
            setInterval(function () { }, timeout)
        }
        let error = assertThrows(test)
        if (error) { return error }
        try {
            test()
        } catch (thrownError) {
            let error = assert(thrownError, sentinel, 'thrownError === sentinel')
            if (error) { return error }
            error = assert(requestedType, 'number', 'requestedType === "number"')
            if (error) { return error }
        }
        error = assertThrows(() => setInterval(function () { }, Symbol()), TypeError, `can't convert symbol to number`)
        if (error) { return error }
        return pass()
    });

    routes.set("/setInterval/timeout-parameter-negative", async () => {
        let error = assertDoesNotThrow(() => setInterval(() => { }, -1))
        if (error) { return error }
        error = assertDoesNotThrow(() => setInterval(() => { }, -1.1))
        if (error) { return error }
        error = assertDoesNotThrow(() => setInterval(() => { }, Number.MIN_SAFE_INTEGER))
        if (error) { return error }
        error = assertDoesNotThrow(() => setInterval(() => { }, Number.MIN_VALUE))
        if (error) { return error }
        error = assertDoesNotThrow(() => setInterval(() => { }, -Infinity))
        if (error) { return error }
        return pass()
    });
    routes.set("/setInterval/timeout-parameter-positive", async () => {
        let error = assertDoesNotThrow(() => setInterval(() => { }, 1))
        if (error) { return error }
        error = assertDoesNotThrow(() => setInterval(() => { }, 1.1))
        if (error) { return error }
        error = assertDoesNotThrow(() => setInterval(() => { }, Number.MAX_SAFE_INTEGER))
        if (error) { return error }
        error = assertDoesNotThrow(() => setInterval(() => { }, Number.MAX_VALUE))
        if (error) { return error }
        error = assertDoesNotThrow(() => setInterval(() => { }, Infinity))
        if (error) { return error }
        return pass()
    });
    routes.set("/setInterval/returns-integer", async () => {
        let id = setInterval(() => { }, 1)
        let error = assert(typeof id, "number", `typeof id === "number"`)
        if (error) { return error }
        return pass()
    });
    routes.set("/setInterval/called-unbound", async () => {
        let error = assertDoesNotThrow(() => {
            setInterval.call(undefined, () => { }, 1)
        })
        if (error) { return error }
        return pass()
    });
}

// setTimeout
{
    routes.set("/setTimeout/exposed-as-global", async () => {
        let error = assert(typeof setTimeout, 'function', `typeof setTimeout`)
        if (error) { return error }
        return pass()
    });
    routes.set("/setTimeout/interface", async () => {
        let actual = Reflect.getOwnPropertyDescriptor(globalThis, 'setTimeout')
        let expected = {
            writable: true,
            enumerable: true,
            configurable: true,
            value: globalThis.setTimeout
        }
        let error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(globalThis, 'setTimeout)`)
        if (error) { return error }

        error = assert(typeof globalThis.setTimeout, 'function', `typeof globalThis.setTimeout`)
        if (error) { return error }

        actual = Reflect.getOwnPropertyDescriptor(globalThis.setTimeout, 'length')
        expected = {
            value: 1,
            writable: false,
            enumerable: false,
            configurable: true
        }
        error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(globalThis.setTimeout, 'length')`)
        if (error) { return error }

        actual = Reflect.getOwnPropertyDescriptor(globalThis.setTimeout, 'name')
        expected = {
            value: "setTimeout",
            writable: false,
            enumerable: false,
            configurable: true
        }
        error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(globalThis.setTimeout, 'name')`)
        if (error) { return error }

        return pass()
    });
    routes.set("/setTimeout/called-as-constructor-function", async () => {
        let error = assertThrows(() => {
            new setTimeout
        }, TypeError, `setTimeout is not a constructor`)
        if (error) { return error }
        return pass()
    });
    routes.set("/setTimeout/empty-parameter", async () => {
        let error = assertThrows(() => {
            setTimeout()
        }, TypeError, `setTimeout: At least 1 argument required, but only 0 passed`)
        if (error) { return error }
        return pass()
    });
    routes.set("/setTimeout/handler-parameter-not-supplied", async () => {
        let error = assertThrows(() => {
            setTimeout()
        }, TypeError, `setTimeout: At least 1 argument required, but only 0 passed`)
        if (error) { return error }
        return pass()
    });
    routes.set("/setTimeout/handler-parameter-not-callable", async () => {
        let non_callable_types = [
            // Primitive types
            null,
            undefined,
            true,
            1,
            1n,
            'hello',
            Symbol(),
            // After primitive types, the only remaining types are Objects and Functions
            {},
        ];
        for (const type of non_callable_types) {
            let error = assertThrows(() => {
                setTimeout(type)
                // TODO: Make a TypeError
            }, Error, `First argument to setTimeout must be a function`)
            if (error) { return error }
        }
        return pass()
    });
    routes.set("/setTimeout/timeout-parameter-not-supplied", async () => {
        let error = assertDoesNotThrow(() => {
            setTimeout(function () { })
        })
        if (error) { return error }
        return pass()
    });
    // https://tc39.es/ecma262/#sec-tonumber
    routes.set("/setTimeout/timeout-parameter-calls-7.1.4-ToNumber", async () => {
        let sentinel;
        let requestedType;
        const test = () => {
            sentinel = Symbol();
            const timeout = {
                [Symbol.toPrimitive](type) {
                    requestedType = type;
                    throw sentinel;
                }
            }
            setTimeout(function () { }, timeout)
        }
        let error = assertThrows(test)
        if (error) { return error }
        try {
            test()
        } catch (thrownError) {
            let error = assert(thrownError, sentinel, 'thrownError === sentinel')
            if (error) { return error }
            error = assert(requestedType, 'number', 'requestedType === "number"')
            if (error) { return error }
        }
        error = assertThrows(() => setTimeout(function () { }, Symbol()), TypeError, `can't convert symbol to number`)
        if (error) { return error }
        return pass()
    });

    routes.set("/setTimeout/timeout-parameter-negative", async () => {
        let error = assertDoesNotThrow(() => setTimeout(() => { }, -1))
        if (error) { return error }
        error = assertDoesNotThrow(() => setTimeout(() => { }, -1.1))
        if (error) { return error }
        error = assertDoesNotThrow(() => setTimeout(() => { }, Number.MIN_SAFE_INTEGER))
        if (error) { return error }
        error = assertDoesNotThrow(() => setTimeout(() => { }, Number.MIN_VALUE))
        if (error) { return error }
        error = assertDoesNotThrow(() => setTimeout(() => { }, -Infinity))
        if (error) { return error }
        return pass()
    });
    routes.set("/setTimeout/timeout-parameter-positive", async () => {
        let error = assertDoesNotThrow(() => setTimeout(() => { }, 1))
        if (error) { return error }
        error = assertDoesNotThrow(() => setTimeout(() => { }, 1.1))
        if (error) { return error }
        error = assertDoesNotThrow(() => setTimeout(() => { }, Number.MAX_SAFE_INTEGER))
        if (error) { return error }
        error = assertDoesNotThrow(() => setTimeout(() => { }, Number.MAX_VALUE))
        if (error) { return error }
        error = assertDoesNotThrow(() => setTimeout(() => { }, Infinity))
        if (error) { return error }
        return pass()
    });
    routes.set("/setTimeout/returns-integer", async () => {
        let id = setTimeout(() => { }, 1)
        let error = assert(typeof id, "number", `typeof id === "number"`)
        if (error) { return error }
        return pass()
    });
    routes.set("/setTimeout/called-unbound", async () => {
        let error = assertDoesNotThrow(() => {
            setTimeout.call(undefined, () => { }, 1)
        })
        if (error) { return error }
        return pass()
    });
}

// clearInterval
{
    routes.set("/clearInterval/exposed-as-global", async () => {
        let error = assert(typeof clearInterval, 'function', `typeof clearInterval`)
        if (error) { return error }
        return pass()
    });
    routes.set("/clearInterval/interface", async () => {
        let actual = Reflect.getOwnPropertyDescriptor(globalThis, 'clearInterval')
        let expected = {
            writable: true,
            enumerable: true,
            configurable: true,
            value: globalThis.clearInterval
        }
        let error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(globalThis, 'clearInterval)`)
        if (error) { return error }

        error = assert(typeof globalThis.clearInterval, 'function', `typeof globalThis.clearInterval`)
        if (error) { return error }

        actual = Reflect.getOwnPropertyDescriptor(globalThis.clearInterval, 'length')
        expected = {
            value: 1,
            writable: false,
            enumerable: false,
            configurable: true
        }
        error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(globalThis.clearInterval, 'length')`)
        if (error) { return error }

        actual = Reflect.getOwnPropertyDescriptor(globalThis.clearInterval, 'name')
        expected = {
            value: "clearInterval",
            writable: false,
            enumerable: false,
            configurable: true
        }
        error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(globalThis.clearInterval, 'name')`)
        if (error) { return error }

        return pass()
    });
    routes.set("/clearInterval/called-as-constructor-function", async () => {
        let error = assertThrows(() => {
            new clearInterval
        }, TypeError, `clearInterval is not a constructor`)
        if (error) { return error }
        return pass()
    });
    routes.set("/clearInterval/id-parameter-not-supplied", async () => {
        let error = assertThrows(() => {
            clearInterval()
        }, TypeError, `clearInterval: At least 1 argument required, but only 0 passed`)
        if (error) { return error }
        return pass()
    });
    // https://tc39.es/ecma262/#sec-tonumber
    routes.set("/clearInterval/id-parameter-calls-7.1.4-ToNumber", async () => {
        let sentinel;
        let requestedType;
        const test = () => {
            sentinel = Symbol();
            const timeout = {
                [Symbol.toPrimitive](type) {
                    requestedType = type;
                    throw sentinel;
                }
            }
            clearInterval(timeout)
        }
        let error = assertThrows(test)
        if (error) { return error }
        try {
            test()
        } catch (thrownError) {
            let error = assert(thrownError, sentinel, 'thrownError === sentinel')
            if (error) { return error }
            error = assert(requestedType, 'number', 'requestedType === "number"')
            if (error) { return error }
        }
        error = assertThrows(() => clearInterval(Symbol()), TypeError, `can't convert symbol to number`)
        if (error) { return error }
        return pass()
    });

    routes.set("/clearInterval/id-parameter-negative", async () => {
        let error = assertDoesNotThrow(() => clearInterval(-1))
        if (error) { return error }
        error = assertDoesNotThrow(() => clearInterval(-1.1))
        if (error) { return error }
        error = assertDoesNotThrow(() => clearInterval(Number.MIN_SAFE_INTEGER))
        if (error) { return error }
        error = assertDoesNotThrow(() => clearInterval(Number.MIN_VALUE))
        if (error) { return error }
        error = assertDoesNotThrow(() => clearInterval(-Infinity))
        if (error) { return error }
        return pass()
    });
    routes.set("/clearInterval/id-parameter-positive", async () => {
        let error = assertDoesNotThrow(() => clearInterval(1))
        if (error) { return error }
        error = assertDoesNotThrow(() => clearInterval(1.1))
        if (error) { return error }
        error = assertDoesNotThrow(() => clearInterval(Number.MAX_SAFE_INTEGER))
        if (error) { return error }
        error = assertDoesNotThrow(() => clearInterval(Number.MAX_VALUE))
        if (error) { return error }
        error = assertDoesNotThrow(() => clearInterval(Infinity))
        if (error) { return error }
        return pass()
    });
    routes.set("/clearInterval/returns-undefined", async () => {
        let result = clearInterval(1)
        let error = assert(typeof result, "undefined", `typeof result === "undefined"`)
        if (error) { return error }
        return pass()
    });
    routes.set("/clearInterval/called-unbound", async () => {
        let error = assertDoesNotThrow(() => {
            clearInterval.call(undefined, 1)
        })
        if (error) { return error }
        return pass()
    });
}

// clearTimeout
{
    routes.set("/clearTimeout/exposed-as-global", async () => {
        let error = assert(typeof clearTimeout, 'function', `typeof clearTimeout`)
        if (error) { return error }
        return pass()
    });
    routes.set("/clearTimeout/interface", async () => {
        let actual = Reflect.getOwnPropertyDescriptor(globalThis, 'clearTimeout')
        let expected = {
            writable: true,
            enumerable: true,
            configurable: true,
            value: globalThis.clearTimeout
        }
        let error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(globalThis, 'clearTimeout)`)
        if (error) { return error }

        error = assert(typeof globalThis.clearTimeout, 'function', `typeof globalThis.clearTimeout`)
        if (error) { return error }

        actual = Reflect.getOwnPropertyDescriptor(globalThis.clearTimeout, 'length')
        expected = {
            value: 1,
            writable: false,
            enumerable: false,
            configurable: true
        }
        error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(globalThis.clearTimeout, 'length')`)
        if (error) { return error }

        actual = Reflect.getOwnPropertyDescriptor(globalThis.clearTimeout, 'name')
        expected = {
            value: "clearTimeout",
            writable: false,
            enumerable: false,
            configurable: true
        }
        error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(globalThis.clearTimeout, 'name')`)
        if (error) { return error }

        return pass()
    });
    routes.set("/clearTimeout/called-as-constructor-function", async () => {
        let error = assertThrows(() => {
            new clearTimeout
        }, TypeError, `clearTimeout is not a constructor`)
        if (error) { return error }
        return pass()
    });
    routes.set("/clearTimeout/id-parameter-not-supplied", async () => {
        let error = assertThrows(() => {
            clearTimeout()
        }, TypeError, `clearTimeout: At least 1 argument required, but only 0 passed`)
        if (error) { return error }
        return pass()
    });
    // https://tc39.es/ecma262/#sec-tonumber
    routes.set("/clearTimeout/id-parameter-calls-7.1.4-ToNumber", async () => {
        let sentinel;
        let requestedType;
        const test = () => {
            sentinel = Symbol();
            const timeout = {
                [Symbol.toPrimitive](type) {
                    requestedType = type;
                    throw sentinel;
                }
            }
            clearTimeout(timeout)
        }
        let error = assertThrows(test)
        if (error) { return error }
        try {
            test()
        } catch (thrownError) {
            let error = assert(thrownError, sentinel, 'thrownError === sentinel')
            if (error) { return error }
            error = assert(requestedType, 'number', 'requestedType === "number"')
            if (error) { return error }
        }
        error = assertThrows(() => clearTimeout(Symbol()), TypeError, `can't convert symbol to number`)
        if (error) { return error }
        return pass()
    });

    routes.set("/clearTimeout/id-parameter-negative", async () => {
        let error = assertDoesNotThrow(() => clearTimeout(-1))
        if (error) { return error }
        error = assertDoesNotThrow(() => clearTimeout(-1.1))
        if (error) { return error }
        error = assertDoesNotThrow(() => clearTimeout(Number.MIN_SAFE_INTEGER))
        if (error) { return error }
        error = assertDoesNotThrow(() => clearTimeout(Number.MIN_VALUE))
        if (error) { return error }
        error = assertDoesNotThrow(() => clearTimeout(-Infinity))
        if (error) { return error }
        return pass()
    });
    routes.set("/clearTimeout/id-parameter-positive", async () => {
        let error = assertDoesNotThrow(() => clearTimeout(1))
        if (error) { return error }
        error = assertDoesNotThrow(() => clearTimeout(1.1))
        if (error) { return error }
        error = assertDoesNotThrow(() => clearTimeout(Number.MAX_SAFE_INTEGER))
        if (error) { return error }
        error = assertDoesNotThrow(() => clearTimeout(Number.MAX_VALUE))
        if (error) { return error }
        error = assertDoesNotThrow(() => clearTimeout(Infinity))
        if (error) { return error }
        return pass()
    });
    routes.set("/clearTimeout/returns-undefined", async () => {
        let result = clearTimeout(1)
        let error = assert(typeof result, "undefined", `typeof result === "undefined"`)
        if (error) { return error }
        return pass()
    });
    routes.set("/clearTimeout/called-unbound", async () => {
        let error = assertDoesNotThrow(() => {
            clearTimeout.call(undefined, 1)
        })
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
