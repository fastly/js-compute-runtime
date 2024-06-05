/* eslint-env serviceworker */
import { pass, assert, assertDoesNotThrow, assertThrows } from "./assertions.js";
import { routes } from "./routes.js";

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
    routes.set("/setTimeout/200-ms", async () => {
        let controller, start
        setTimeout(() => {
            const end = Date.now()
            controller.enqueue(new TextEncoder().encode(`END\n`))
            if (end - start < 200) {
                controller.enqueue(new TextEncoder().encode(`ERROR: Timer took ${end - start} instead of 200ms`))
            }
            controller.close()
        }, 200);
        return new Response(new ReadableStream({
            start(_controller) {
                controller = _controller
                start = Date.now()
                controller.enqueue(new TextEncoder().encode(`START\n`))
            }
        }))
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
