/// <reference path="../../../../../types/index.d.ts" />
/* eslint-env serviceworker */
/* global ReadableStream */
import { pass, assert, assertDoesNotThrow, assertThrows, assertRejects, iteratableToStream, streamToString, assertResolves } from "../../../assertions.js";
import { routes, FASTLY_SERVICE_VERSION } from "../../../test-harness.js";
import { SimpleCache, SimpleCacheEntry } from 'fastly:cache';

let error;
routes.set("/simple-cache/interface", () => {
    let actual = Reflect.ownKeys(SimpleCache)
    let expected = ["prototype", "purge", "get", "getOrSet", "set", "length", "name"]
    error = assert(actual, expected, `Reflect.ownKeys(SimpleCache)`)
    if (error) { return error }

    // Check the prototype descriptors are correct
    {
        actual = Reflect.getOwnPropertyDescriptor(SimpleCache, 'prototype')
        expected = {
            "value": SimpleCache.prototype,
            "writable": false,
            "enumerable": false,
            "configurable": false
        }
        error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(SimpleCache, 'prototype')`)
        if (error) { return error }
    }

    // Check the constructor function's defined parameter length is correct
    {
        actual = Reflect.getOwnPropertyDescriptor(SimpleCache, 'length')
        expected = {
            "value": 0,
            "writable": false,
            "enumerable": false,
            "configurable": true
        }
        error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(SimpleCache, 'length')`)
        if (error) { return error }
    }

    // Check the constructor function's name is correct
    {
        actual = Reflect.getOwnPropertyDescriptor(SimpleCache, 'name')
        expected = {
            "value": "SimpleCache",
            "writable": false,
            "enumerable": false,
            "configurable": true
        }
        error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(SimpleCache, 'name')`)
        if (error) { return error }
    }

    // Check the prototype has the correct keys
    {
        actual = Reflect.ownKeys(SimpleCache.prototype)
        expected = ["constructor", Symbol.toStringTag]
        error = assert(actual, expected, `Reflect.ownKeys(SimpleCache.prototype)`)
        if (error) { return error }
    }

    // Check the constructor on the prototype is correct
    {
        actual = Reflect.getOwnPropertyDescriptor(SimpleCache.prototype, 'constructor')
        expected = { "writable": true, "enumerable": false, "configurable": true, value: SimpleCache.prototype.constructor }
        error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(SimpleCache.prototype, 'constructor')`)
        if (error) { return error }

        error = assert(typeof SimpleCache.prototype.constructor, 'function', `typeof SimpleCache.prototype.constructor`)
        if (error) { return error }

        actual = Reflect.getOwnPropertyDescriptor(SimpleCache.prototype.constructor, 'length')
        expected = {
            "value": 0,
            "writable": false,
            "enumerable": false,
            "configurable": true
        }
        error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(SimpleCache.prototype.constructor, 'length')`)
        if (error) { return error }

        actual = Reflect.getOwnPropertyDescriptor(SimpleCache.prototype.constructor, 'name')
        expected = {
            "value": "SimpleCache",
            "writable": false,
            "enumerable": false,
            "configurable": true
        }
        error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(SimpleCache.prototype.constructor, 'name')`)
        if (error) { return error }
    }

    // Check the Symbol.toStringTag on the prototype is correct
    {
        actual = Reflect.getOwnPropertyDescriptor(SimpleCache.prototype, Symbol.toStringTag)
        expected = { "writable": false, "enumerable": false, "configurable": true, value: "SimpleCache" }
        error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(SimpleCache.prototype, [Symbol.toStringTag])`)
        if (error) { return error }

        error = assert(typeof SimpleCache.prototype[Symbol.toStringTag], 'string', `typeof SimpleCache.prototype[Symbol.toStringTag]`)
        if (error) { return error }
    }

    // Check the get static method has correct descriptors, length and name
    {
        actual = Reflect.getOwnPropertyDescriptor(SimpleCache, 'get')
        expected = { "writable": true, "enumerable": true, "configurable": true, value: SimpleCache.get }
        error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(SimpleCache, 'get')`)
        if (error) { return error }

        error = assert(typeof SimpleCache.get, 'function', `typeof SimpleCache.get`)
        if (error) { return error }

        actual = Reflect.getOwnPropertyDescriptor(SimpleCache.get, 'length')
        expected = {
            "value": 1,
            "writable": false,
            "enumerable": false,
            "configurable": true
        }
        error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(SimpleCache.get, 'length')`)
        if (error) { return error }

        actual = Reflect.getOwnPropertyDescriptor(SimpleCache.get, 'name')
        expected = {
            "value": "get",
            "writable": false,
            "enumerable": false,
            "configurable": true
        }
        error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(SimpleCache.get, 'name')`)
        if (error) { return error }
    }

    // Check the set static method has correct descriptors, length and name
    {
        actual = Reflect.getOwnPropertyDescriptor(SimpleCache, 'set')
        expected = { "writable": true, "enumerable": true, "configurable": true, value: SimpleCache.set }
        error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(SimpleCache, 'set')`)
        if (error) { return error }

        error = assert(typeof SimpleCache.set, 'function', `typeof SimpleCache.set`)
        if (error) { return error }

        actual = Reflect.getOwnPropertyDescriptor(SimpleCache.set, 'length')
        expected = {
            "value": 3,
            "writable": false,
            "enumerable": false,
            "configurable": true
        }
        error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(SimpleCache.set, 'length')`)
        if (error) { return error }

        actual = Reflect.getOwnPropertyDescriptor(SimpleCache.set, 'name')
        expected = {
            "value": "set",
            "writable": false,
            "enumerable": false,
            "configurable": true
        }
        error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(SimpleCache.set, 'name')`)
        if (error) { return error }
    }

    // Check the purge static method has correct descriptors, length and name
    {
        actual = Reflect.getOwnPropertyDescriptor(SimpleCache, 'purge')
        expected = { "writable": true, "enumerable": true, "configurable": true, value: SimpleCache.purge }
        error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(SimpleCache, 'purge')`)
        if (error) { return error }

        error = assert(typeof SimpleCache.purge, 'function', `typeof SimpleCache.purge`)
        if (error) { return error }

        actual = Reflect.getOwnPropertyDescriptor(SimpleCache.purge, 'length')
        expected = {
            "value": 2,
            "writable": false,
            "enumerable": false,
            "configurable": true
        }
        error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(SimpleCache.purge, 'length')`)
        if (error) { return error }

        actual = Reflect.getOwnPropertyDescriptor(SimpleCache.purge, 'name')
        expected = {
            "value": "purge",
            "writable": false,
            "enumerable": false,
            "configurable": true
        }
        error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(SimpleCache.purge, 'name')`)
        if (error) { return error }
    }

    // Check the getOrSet static method has correct descriptors, length and name
    {
        actual = Reflect.getOwnPropertyDescriptor(SimpleCache, 'getOrSet')
        expected = { "writable": true, "enumerable": true, "configurable": true, value: SimpleCache.getOrSet }
        error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(SimpleCache, 'getOrSet')`)
        if (error) { return error }

        error = assert(typeof SimpleCache.getOrSet, 'function', `typeof SimpleCache.getOrSet`)
        if (error) { return error }

        actual = Reflect.getOwnPropertyDescriptor(SimpleCache.getOrSet, 'length')
        expected = {
            "value": 2,
            "writable": false,
            "enumerable": false,
            "configurable": true
        }
        error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(SimpleCache.getOrSet, 'length')`)
        if (error) { return error }

        actual = Reflect.getOwnPropertyDescriptor(SimpleCache.getOrSet, 'name')
        expected = {
            "value": "getOrSet",
            "writable": false,
            "enumerable": false,
            "configurable": true
        }
        error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(SimpleCache.getOrSet, 'name')`)
        if (error) { return error }
    }

    return pass()
});

// SimpleCache constructor
{

    routes.set("/simple-store/constructor/called-as-regular-function", () => {
        error = assertThrows(() => {
            SimpleCache()
        }, TypeError, `Illegal constructor`)
        if (error) { return error }
        return pass()
    });
    routes.set("/simple-cache/constructor/throws", () => {
        error = assertThrows(() => new SimpleCache(), TypeError, `Illegal constructor`)
        if (error) { return error }
        return pass()
    });
}

// SimpleCache purge static method
// static purge(key: string, options: PurgeOptions): undefined;
{
    routes.set("/simple-cache/purge/called-as-constructor", () => {
        error = assertThrows(() => {
            new SimpleCache.purge('1', {scope:"global"})
        }, TypeError, `SimpleCache.purge is not a constructor`)
        if (error) { return error }
        return pass()
    });
    // Ensure we correctly coerce the parameter to a string as according to
    // https://tc39.es/ecma262/#sec-tostring
    routes.set("/simple-cache/purge/key-parameter-calls-7.1.17-ToString", () => {
        let sentinel;
        const test = () => {
            sentinel = Symbol('sentinel');
            const key = {
                toString() {
                    throw sentinel;
                }
            }
            SimpleCache.purge(key, {scope:"global"})
        }
        error = assertThrows(test)
        if (error) { return error }
        try {
            test()
        } catch (thrownError) {
            error = assert(thrownError, sentinel, 'thrownError === sentinel')
            if (error) { return error }
        }
        error = assertThrows(() => {
            SimpleCache.purge(Symbol(),{scope:"global"})
        }, TypeError, `can't convert symbol to string`)
        if (error) { return error }
        return pass()
    });
    routes.set("/simple-cache/purge/key-parameter-not-supplied", () => {
        error = assertThrows(() => {
            SimpleCache.purge()
        }, TypeError, `SimpleCache.purge: At least 2 arguments required, but only 0 passed`)
        if (error) { return error }
        return pass()
    });
    routes.set("/simple-cache/purge/key-parameter-empty-string", () => {
        error = assertThrows(() => {
            SimpleCache.purge('',{scope:"global"})
        }, Error, `SimpleCache.purge: key can not be an empty string`)
        if (error) { return error }
        return pass()
    });
    routes.set("/simple-cache/purge/key-parameter-8135-character-string", () => {
        error = assertDoesNotThrow(() => {
            const key = 'a'.repeat(8135)
            SimpleCache.purge(key,{scope:"global"})
        })
        if (error) { return error }
        return pass()
    });
    routes.set("/simple-cache/purge/key-parameter-8136-character-string", () => {
        error = assertThrows(() => {
            const key = 'a'.repeat(8136)
            SimpleCache.purge(key,{scope:"global"})
        }, Error, `SimpleCache.purge: key is too long, the maximum allowed length is 8135.`)
        if (error) { return error }
        return pass()
    });
    routes.set("/simple-cache/purge/options-parameter", () => {
        error = assertThrows(() => {
            const key = 'a'
            SimpleCache.purge(key, "hello")
        }, Error, `SimpleCache.purge: options parameter is not an object.`)
        if (error) { return error }
        error = assertThrows(() => {
            const key = 'a'
            SimpleCache.purge(key, {scope: Symbol()})
        }, Error, `can't convert symbol to string`)
        if (error) { return error }
        error = assertThrows(() => {
            const key = 'a'
            SimpleCache.purge(key, {scope: ""})
        }, Error, `SimpleCache.purge: scope field of options parameter must be either 'pop', or 'global'.`)
        if (error) { return error }
        error = assertDoesNotThrow(() => {
            const key = 'a'
            SimpleCache.purge(key, {scope: "pop"})
        })
        if (error) { return error }
        error = assertDoesNotThrow(() => {
            const key = 'a'
            SimpleCache.purge(key, {scope: "global"})
        })
        if (error) { return error }
        return pass()
    });
    routes.set("/simple-cache/purge/returns-undefined", () => {
        error = assert(SimpleCache.purge('meow', {scope:"global"}), undefined, "SimpleCache.purge('meow', {scope'global'})")
        if (error) { return error }
        return pass()
    });
}

// SimpleCache set static method
// static set(key: string, value: BodyInit, ttl: number): undefined;
{
    routes.set("/simple-cache/set/called-as-constructor", () => {
        error = assertThrows(() => {
            new SimpleCache.set('1', 'meow', 1)
        }, TypeError, `SimpleCache.set is not a constructor`)
        if (error) { return error }
        return pass()
    });
    // Ensure we correctly coerce the key parameter to a string as according to
    // https://tc39.es/ecma262/#sec-tostring
    routes.set("/simple-cache/set/key-parameter-calls-7.1.17-ToString", () => {
        let sentinel;
        const test = () => {
            sentinel = Symbol('sentinel');
            const key = {
                toString() {
                    throw sentinel;
                }
            }
            SimpleCache.set(key, 'meow', 1)
        }
        error = assertThrows(test)
        if (error) { return error }
        try {
            test()
        } catch (thrownError) {
            error = assert(thrownError, sentinel, 'thrownError === sentinel')
            if (error) { return error }
        }
        error = assertThrows(() => {
            SimpleCache.set(Symbol(), 'meow', 1)
        }, TypeError, `can't convert symbol to string`)
        if (error) { return error }
        return pass()
    });
    // Ensure we correctly coerce the tll parameter to a number as according to
    // https://tc39.es/ecma262/#sec-tonumber
    routes.set("/simple-cache/set/tll-parameter-7.1.4-ToNumber", () => {
        let sentinel;
        let requestedType;
        const test = () => {
            sentinel = Symbol('sentinel');
            const ttl = {
                [Symbol.toPrimitive](type) {
                    requestedType = type;
                    throw sentinel;
                }
            }
            SimpleCache.set('1', 'meow', ttl)
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
        error = assertThrows(() => SimpleCache.set('1', 'meow', Symbol()), TypeError, `can't convert symbol to number`)
        if (error) { return error }
        return pass()
    });
    routes.set("/simple-cache/set/no-parameters-supplied", () => {
        error = assertThrows(() => {
            SimpleCache.set()
        }, TypeError, `SimpleCache.set: At least 3 arguments required, but only 0 passed`)
        if (error) { return error }
        return pass()
    });
    routes.set("/simple-cache/set/key-parameter-empty-string", () => {
        error = assertThrows(() => {
            SimpleCache.set('', 'meow', 1)
        }, Error, `SimpleCache.set: key can not be an empty string`)
        if (error) { return error }
        return pass()
    });
    routes.set("/simple-cache/set/key-parameter-8135-character-string", () => {
        error = assertDoesNotThrow(() => {
            const key = 'a'.repeat(8135)
            SimpleCache.set(key, 'meow', 1)
        })
        if (error) { return error }
        return pass()
    });
    routes.set("/simple-cache/set/key-parameter-8136-character-string", () => {
        error = assertThrows(() => {
            const key = 'a'.repeat(8136)
            SimpleCache.set(key, 'meow', 1)
        }, Error, `SimpleCache.set: key is too long, the maximum allowed length is 8135.`)
        if (error) { return error }
        return pass()
    });
    routes.set("/simple-cache/set/ttl-parameter-negative-number", () => {
        error = assertThrows(() => {
            SimpleCache.set('cat', 'meow', -1)
        }, Error, `SimpleCache.set: TTL parameter is an invalid value, only positive numbers can be used for TTL values.`)
        if (error) { return error }
        return pass()
    });
    routes.set("/simple-cache/set/ttl-parameter-NaN", () => {
        error = assertThrows(() => {
            SimpleCache.set('cat', 'meow', NaN)
        }, Error, `SimpleCache.set: TTL parameter is an invalid value, only positive numbers can be used for TTL values.`)
        if (error) { return error }
        return pass()
    });
    routes.set("/simple-cache/set/ttl-parameter-Infinity", () => {
        error = assertThrows(() => {
            SimpleCache.set('cat', 'meow', Number.POSITIVE_INFINITY)
        }, Error, `SimpleCache.set: TTL parameter is an invalid value, only positive numbers can be used for TTL values.`)
        if (error) { return error }
        error = assertThrows(() => {
            SimpleCache.set('cat', 'meow', Number.NEGATIVE_INFINITY)
        }, Error, `SimpleCache.set: TTL parameter is an invalid value, only positive numbers can be used for TTL values.`)
        if (error) { return error }
        return pass()
    });
    routes.set("/simple-cache/set/value-parameter-as-undefined", () => {
        error = assert(SimpleCache.set("meow", undefined, 1), undefined, 'SimpleCache.set("meow", undefined, 1) === undefined')
        if (error) { return error }
        return pass()
    });
    // - ReadableStream
    routes.set("/simple-cache/set/value-parameter-readablestream-missing-length-parameter", () => {
        // TODO: remove this when streams are supported
        let error = assertThrows(() => {
            const stream = iteratableToStream([])
            SimpleCache.set("meow", stream, 1)
        }, TypeError, `Content-provided streams are not yet supported for streaming into SimpleCache`)
        if (error) { return error }
        return pass()
    });
    routes.set("/simple-cache/set/value-parameter-readablestream-negative-length-parameter", () => {
        // TODO: remove this when streams are supported
        let error = assertThrows(() => {
            const stream = iteratableToStream([])
            SimpleCache.set("meow", stream, 1, -1)
        }, TypeError, `Content-provided streams are not yet supported for streaming into SimpleCache`)
        if (error) { return error }
        return pass()
    });
    routes.set("/simple-cache/set/value-parameter-readablestream-nan-length-parameter", () => {
        // TODO: remove this when streams are supported
        let error = assertThrows(() => {
            const stream = iteratableToStream([])
            SimpleCache.set("meow", stream, 1, NaN)
        }, TypeError, `Content-provided streams are not yet supported for streaming into SimpleCache`)
        if (error) { return error }
        return pass()
    });
    routes.set("/simple-cache/set/value-parameter-readablestream-negative-infinity-length-parameter", () => {
        // TODO: remove this when streams are supported
        let error = assertThrows(() => {
            const stream = iteratableToStream([])
            SimpleCache.set("meow", stream, 1, Number.NEGATIVE_INFINITY)
        }, TypeError, `Content-provided streams are not yet supported for streaming into SimpleCache`)
        if (error) { return error }
        return pass()
    });
    routes.set("/simple-cache/set/value-parameter-readablestream-positive-infinity-length-parameter", () => {
        // TODO: remove this when streams are supported
        let error = assertThrows(() => {
            const stream = iteratableToStream([])
            SimpleCache.set("meow", stream, 1, Number.POSITIVE_INFINITY)
        }, TypeError, `Content-provided streams are not yet supported for streaming into SimpleCache`)
        if (error) { return error }
        return pass()
    });
    // Ensure we correctly coerce the tll parameter to a number as according to
    // https://tc39.es/ecma262/#sec-tonumber
    routes.set("/simple-cache/set/length-parameter-7.1.4-ToNumber", async () => {
        const res = await fetch('https://compute-sdk-test-backend.edgecompute.app/', {
            backend: "TheOrigin",
        })
        let sentinel;
        let requestedType;
        const test = () => {
            sentinel = Symbol('sentinel');
            const length = {
                [Symbol.toPrimitive](type) {
                    requestedType = type;
                    throw sentinel;
                }
            }
            SimpleCache.set('1', res.body, 1, length)
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
        error = assertThrows(() => SimpleCache.set('1', res.body, 1, Symbol()), TypeError, `can't convert symbol to number`)
        if (error) { return error }
        return pass()
    });
    routes.set("/simple-cache/set/value-parameter-readablestream-empty", () => {
        // TODO: remove this when streams are supported
        let error = assertThrows(() => {
            const stream = iteratableToStream([])
            SimpleCache.set("meow", stream, 1, 0)
        }, TypeError, `Content-provided streams are not yet supported for streaming into SimpleCache`)
        if (error) { return error }
        return pass()
    });
    routes.set("/simple-cache/set/value-parameter-readablestream-locked", () => {
        const stream = iteratableToStream([])
        // getReader() causes the stream to become locked
        stream.getReader()
        let error = assertThrows(() => {
            SimpleCache.set("meow", stream, 1, 0)
        }, TypeError, `Can't use a ReadableStream that's locked or has ever been read from or canceled`)
        if (error) { return error }
        return pass()
    });
    routes.set("/simple-cache/set/value-parameter-readablestream", async () => {
        const res = await fetch('https://compute-sdk-test-backend.edgecompute.app/', {
            backend: "TheOrigin",
        })
        let result = SimpleCache.set('readablestream', res.body, 100, res.headers.get('content-length'))
        error = assert(result, undefined, `SimpleCache.set('readablestream', res.body, 100)`)
        if (error) { return error }
        return pass()
    });

    // - URLSearchParams
    routes.set("/simple-cache/set/value-parameter-URLSearchParams", () => {
        const items = [
            new URLSearchParams,
            new URLSearchParams({ a: 'b', c: 'd' }),
        ];
        for (const searchParams of items) {
            let result = SimpleCache.set("meow", searchParams, 1)
            error = assert(result, undefined, `SimpleCache.set("meow", searchParams, 1) === undefiend`)
            if (error) { return error }
        }
        return pass()
    });
    // - USV strings
    routes.set("/simple-cache/set/value-parameter-strings", () => {
        const strings = [
            // empty
            '',
            // lone surrogate
            '\uD800',
            // surrogate pair
            '𠈓',
            String('carrot'),
        ];
        for (const string of strings) {
            let result = SimpleCache.set("meow", string, 1)
            error = assert(result, undefined, `SimpleCache.set("meow", string, 1) === undefined`)
            if (error) { return error }
        }
        return pass()
    });

    // https://tc39.es/ecma262/#sec-tostring
    routes.set("/simple-cache/set/value-parameter-calls-7.1.17-ToString", () => {
        let sentinel;
        const test = () => {
            sentinel = Symbol('sentinel');
            const value = {
                toString() {
                    throw sentinel;
                }
            }
            SimpleCache.set("meow", value, 1)
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
            SimpleCache.set("meow", Symbol(), 1)
        }, TypeError, `can't convert symbol to string`)
        if (error) { return error }
        return pass()
    });

    // - buffer source
    routes.set("/simple-cache/set/value-parameter-buffer", () => {
        const typedArrayConstructors = [
            Int8Array,
            Int16Array,
            Int32Array,
            Float32Array,
            Float64Array,
            BigInt64Array,
            Uint8Array,
            Uint8ClampedArray,
            Uint16Array,
            Uint32Array,
            BigUint64Array,
        ];
        for (const constructor of typedArrayConstructors) {
            const typedArray = new constructor(8);
            let result = SimpleCache.set("meow", typedArray.buffer, 1)
            error = assert(result, undefined, `SimpleCache.set("meow", typedArray.buffer, 1) === undefined`)
            if (error) { return error }
        }
        return pass()
    });
    routes.set("/simple-cache/set/value-parameter-arraybuffer", () => {
        const typedArrayConstructors = [
            Int8Array,
            Int16Array,
            Int32Array,
            Float32Array,
            Float64Array,
            BigInt64Array,
            Uint8Array,
            Uint8ClampedArray,
            Uint16Array,
            Uint32Array,
            BigUint64Array,
        ];
        for (const constructor of typedArrayConstructors) {
            const typedArray = new constructor(8);
            let result = SimpleCache.set("meow", typedArray.buffer, 1)
            error = assert(result, undefined, `SimpleCache.set("meow", typedArray.buffer, 1) === undefined`)
            if (error) { return error }
        }
        return pass()
    });
    routes.set("/simple-cache/set/value-parameter-typed-arrays", () => {
        const typedArrayConstructors = [
            Int8Array,
            Int16Array,
            Int32Array,
            Float32Array,
            Float64Array,
            BigInt64Array,
            Uint8Array,
            Uint8ClampedArray,
            Uint16Array,
            Uint32Array,
            BigUint64Array,
        ];
        for (const constructor of typedArrayConstructors) {
            const typedArray = new constructor(8);
            let result = SimpleCache.set("meow", typedArray, 1)
            error = assert(result, undefined, `SimpleCache.set("meow", typedArray, 1) === undefined`)
            if (error) { return error }
        }
        return pass()
    });
    routes.set("/simple-cache/set/value-parameter-dataview", () => {
        const typedArrayConstructors = [
            Int8Array,
            Uint8Array,
            Uint8ClampedArray,
            Int16Array,
            Uint16Array,
            Int32Array,
            Uint32Array,
            Float32Array,
            Float64Array,
            BigInt64Array,
            BigUint64Array,
        ];
        for (const constructor of typedArrayConstructors) {
            const typedArray = new constructor(8);
            const view = new DataView(typedArray.buffer);
            let result = SimpleCache.set("meow", view, 1)
            error = assert(result, undefined, `SimpleCache.set("meow", view, 1) === undefined`)
            if (error) { return error }
        }
        return pass()
    });
    routes.set("/simple-cache/set/returns-undefined", () => {
        error = assert(SimpleCache.set('1', 'meow', 1), undefined, "SimpleCache.set('1', 'meow', 1) === undefined")
        if (error) { return error }
        return pass()
    });
}

// SimpleCache get static method
// static get(key: string): SimpleCacheEntry | null;
{
    routes.set("/simple-cache/get/called-as-constructor", () => {
        let error = assertThrows(() => {
            new SimpleCache.get('1')
        }, TypeError, `SimpleCache.get is not a constructor`)
        if (error) { return error }
        return pass()
    });
    // https://tc39.es/ecma262/#sec-tostring
    routes.set("/simple-cache/get/key-parameter-calls-7.1.17-ToString", () => {
        let sentinel;
        const test = () => {
            sentinel = Symbol('sentinel');
            const key = {
                toString() {
                    throw sentinel;
                }
            }
            SimpleCache.get(key)
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
            SimpleCache.get(Symbol())
        }, TypeError, `can't convert symbol to string`)
        if (error) { return error }
        return pass()
    });
    routes.set("/simple-cache/get/key-parameter-not-supplied", () => {
        let error = assertThrows(() => {
            SimpleCache.get()
        }, TypeError, `SimpleCache.get: At least 1 argument required, but only 0 passed`)
        if (error) { return error }
        return pass()
    });
    routes.set("/simple-cache/get/key-parameter-empty-string", () => {
        let error = assertThrows(() => {
            SimpleCache.get('')
        }, Error, `SimpleCache.get: key can not be an empty string`)
        if (error) { return error }
        return pass()
    });
    routes.set("/simple-cache/get/key-parameter-8135-character-string", () => {
        let error = assertDoesNotThrow(() => {
            const key = 'a'.repeat(8135)
            SimpleCache.get(key)
        })
        if (error) { return error }
        return pass()
    });
    routes.set("/simple-cache/get/key-parameter-8136-character-string", () => {
        let error = assertThrows(() => {
            const key = 'a'.repeat(8136)
            SimpleCache.get(key)
        }, Error, `SimpleCache.get: key is too long, the maximum allowed length is 8135.`)
        if (error) { return error }
        return pass()
    });
    routes.set("/simple-cache/get/key-does-not-exist-returns-null", () => {
        let result = SimpleCache.get(Math.random())
        error = assert(result, null, `SimpleCache.get(Math.random()) === null`)
        if (error) { return error }
        return pass()
    });
    routes.set("/simple-cache/get/key-exists", () => {
        SimpleCache.set('cat', 'meow', 100);
        let result = SimpleCache.get('cat');
        error = assert(result instanceof SimpleCacheEntry, true, `SimpleCache.get('cat') instanceof SimpleCacheEntry`)
        if (error) { return error }
        return pass()
    });
}

// SimpleCacheEntry
{
    routes.set("/simple-cache-entry/interface", async () => {
        return simpleCacheEntryInterfaceTests()
    });
    routes.set("/simple-cache-entry/text/valid", async () => {
        let key = `entry-text-valid-${FASTLY_SERVICE_VERSION}`;
        SimpleCache.set(key, 'hello', 100)
        let entry = SimpleCache.get(key)
        let result = entry.text()
        let error = assert(result instanceof Promise, true, `entry.text() instanceof Promise`)
        if (error) { return error }
        result = await result
        error = assert(result, 'hello', `await entry.text()`)
        if (error) { return error }
        return pass()
    });
    routes.set("/simple-cache-entry/json/valid", async () => {
        let key = `entry-json-valid-${FASTLY_SERVICE_VERSION}`;
        const obj = { a: 1, b: 2, c: 3 }
        SimpleCache.set(key, JSON.stringify(obj), 100)
        let entry = SimpleCache.get(key)
        let result = entry.json()
        let error = assert(result instanceof Promise, true, `entry.json() instanceof Promise`)
        if (error) { return error }
        result = await result
        error = assert(result, obj, `await entry.json()`)
        if (error) { return error }
        return pass()
    });
    routes.set("/simple-cache-entry/json/invalid", async () => {
        let key = `entry-json-invalid-${FASTLY_SERVICE_VERSION}`;
        SimpleCache.set(key, "132abc;['-=9", 100)
        let entry = SimpleCache.get(key)
        let error = await assertRejects(() => entry.json(), SyntaxError, `JSON.parse: unexpected non-whitespace character after JSON data at line 1 column 4 of the JSON data`)
        if (error) { return error }
        return pass()
    });
    routes.set("/simple-cache-entry/arrayBuffer/valid", async () => {
        let key = `entry-arraybuffer-valid-${FASTLY_SERVICE_VERSION}`;
        SimpleCache.set(key, new Int8Array([0, 1, 2, 3]), 100)
        let entry = SimpleCache.get(key)
        let result = entry.arrayBuffer()
        let error = assert(result instanceof Promise, true, `entry.arrayBuffer() instanceof Promise`)
        if (error) { return error }
        result = await result
        error = assert(result instanceof ArrayBuffer, true, `(await entry.arrayBuffer()) instanceof ArrayBuffer`)
        if (error) { return error }
        return pass()
    });
    routes.set("/simple-cache-entry/body", async () => {
        let key = `entry-body-${FASTLY_SERVICE_VERSION}`;
        SimpleCache.set(key, 'body body body', 100)
        let entry = SimpleCache.get(key)
        let result = entry.body;
        let error = assert(result instanceof ReadableStream, true, `entry.body instanceof ReadableStream`)
        if (error) { return error }
        let text = await streamToString(result);
        error = assert(text, 'body body body', `entry.body contents as string`)
        if (error) { return error }
        return pass()
    });
    routes.set("/simple-cache-entry/bodyUsed", async () => {
        let key = `entry-bodyUsed-${FASTLY_SERVICE_VERSION}`;
        SimpleCache.set(key, 'body body body', 100)
        let entry = SimpleCache.get(key)
        let error = assert(entry.bodyUsed, false, `entry.bodyUsed`)
        if (error) { return error }
        await entry.text();
        error = assert(entry.bodyUsed, true, `entry.bodyUsed`)
        if (error) { return error }
        return pass()
    });
    routes.set("/simple-cache-entry/readablestream", async () => {
        const res = await fetch('https://compute-sdk-test-backend.edgecompute.app/', {
            backend: "TheOrigin",
        })
        let key = `readablestream-${FASTLY_SERVICE_VERSION}`;
        SimpleCache.set(key, res.body, 100, res.headers.get('content-length'))
        let entry = SimpleCache.get(key)
        error = assert(await entry.text(), 'Compute SDK Test Backend', `await entry.text()`)
        if (error) { return error }
        return pass()
    });
}
async function simpleCacheEntryInterfaceTests() {
    let actual = Reflect.ownKeys(SimpleCacheEntry)
    let expected = ["prototype", "length", "name"]
    let error = assert(actual, expected, `Reflect.ownKeys(SimpleCacheEntry)`)
    if (error) { return error }

    actual = Reflect.getOwnPropertyDescriptor(SimpleCacheEntry, 'prototype')
    expected = {
        "value": SimpleCacheEntry.prototype,
        "writable": false,
        "enumerable": false,
        "configurable": false
    }
    error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(SimpleCacheEntry, 'prototype')`)
    if (error) { return error }

    actual = Reflect.getOwnPropertyDescriptor(SimpleCacheEntry, 'length')
    expected = {
        "value": 0,
        "writable": false,
        "enumerable": false,
        "configurable": true
    }
    error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(SimpleCacheEntry, 'length')`)
    if (error) { return error }

    actual = Reflect.getOwnPropertyDescriptor(SimpleCacheEntry, 'name')
    expected = {
        "value": "SimpleCacheEntry",
        "writable": false,
        "enumerable": false,
        "configurable": true
    }
    error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(SimpleCacheEntry, 'name')`)
    if (error) { return error }

    actual = Reflect.ownKeys(SimpleCacheEntry.prototype)
    expected = ["constructor", "body", "bodyUsed", "arrayBuffer", "json", "text", Symbol.toStringTag]
    error = assert(actual, expected, `Reflect.ownKeys(SimpleCacheEntry.prototype)`)
    if (error) { return error }

    actual = Reflect.getOwnPropertyDescriptor(SimpleCacheEntry.prototype, 'constructor')
    expected = { "writable": true, "enumerable": false, "configurable": true, value: SimpleCacheEntry.prototype.constructor }
    error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(SimpleCacheEntry.prototype, 'constructor')`)
    if (error) { return error }

    actual = Reflect.getOwnPropertyDescriptor(SimpleCacheEntry.prototype, 'text')
    expected = { "writable": true, "enumerable": true, "configurable": true, value: SimpleCacheEntry.prototype.text }
    error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(SimpleCacheEntry.prototype, 'text')`)
    if (error) { return error }

    actual = Reflect.getOwnPropertyDescriptor(SimpleCacheEntry.prototype, 'json')
    expected = { "writable": true, "enumerable": true, "configurable": true, value: SimpleCacheEntry.prototype.json }
    error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(SimpleCacheEntry.prototype, 'json')`)
    if (error) { return error }

    actual = Reflect.getOwnPropertyDescriptor(SimpleCacheEntry.prototype, 'arrayBuffer')
    expected = { "writable": true, "enumerable": true, "configurable": true, value: SimpleCacheEntry.prototype.arrayBuffer }
    error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(SimpleCacheEntry.prototype, 'arrayBuffer')`)
    if (error) { return error }
    actual = Reflect.getOwnPropertyDescriptor(SimpleCacheEntry.prototype, 'body')
    error = assert(actual.enumerable, true, `Reflect.getOwnPropertyDescriptor(SimpleCacheEntry.prototype, 'body').enumerable`)
    error = assert(actual.configurable, true, `Reflect.getOwnPropertyDescriptor(SimpleCacheEntry.prototype, 'body').configurable`)
    error = assert('set' in actual, true, `'set' in Reflect.getOwnPropertyDescriptor(SimpleCacheEntry.prototype, 'body')`)
    error = assert(actual.set, undefined, `Reflect.getOwnPropertyDescriptor(SimpleCacheEntry.prototype, 'body').set`)
    error = assert(typeof actual.get, 'function', `typeof Reflect.getOwnPropertyDescriptor(SimpleCacheEntry.prototype, 'body').get`)
    if (error) { return error }
    actual = Reflect.getOwnPropertyDescriptor(SimpleCacheEntry.prototype, 'bodyUsed')
    error = assert(actual.enumerable, true, `Reflect.getOwnPropertyDescriptor(SimpleCacheEntry.prototype, 'bodyUsed').enumerable`)
    error = assert(actual.configurable, true, `Reflect.getOwnPropertyDescriptor(SimpleCacheEntry.prototype, 'bodyUsed').configurable`)
    error = assert('set' in actual, true, `'set' in Reflect.getOwnPropertyDescriptor(SimpleCacheEntry.prototype, 'bodyUsed')`)
    error = assert(actual.set, undefined, `Reflect.getOwnPropertyDescriptor(SimpleCacheEntry.prototype, 'bodyUsed').set`)
    error = assert(typeof actual.get, 'function', `typeof Reflect.getOwnPropertyDescriptor(SimpleCacheEntry.prototype, 'bodyUsed').get`)
    if (error) { return error }

    error = assert(typeof SimpleCacheEntry.prototype.constructor, 'function', `typeof SimpleCacheEntry.prototype.constructor`)
    if (error) { return error }
    error = assert(typeof SimpleCacheEntry.prototype.text, 'function', `typeof SimpleCacheEntry.prototype.text`)
    if (error) { return error }
    error = assert(typeof SimpleCacheEntry.prototype.json, 'function', `typeof SimpleCacheEntry.prototype.json`)
    if (error) { return error }
    error = assert(typeof SimpleCacheEntry.prototype.arrayBuffer, 'function', `typeof SimpleCacheEntry.prototype.arrayBuffer`)
    if (error) { return error }

    actual = Reflect.getOwnPropertyDescriptor(SimpleCacheEntry.prototype.constructor, 'length')
    expected = {
        "value": 0,
        "writable": false,
        "enumerable": false,
        "configurable": true
    }
    error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(SimpleCacheEntry.prototype.constructor, 'length')`)
    if (error) { return error }

    actual = Reflect.getOwnPropertyDescriptor(SimpleCacheEntry.prototype.constructor, 'name')
    expected = {
        "value": "SimpleCacheEntry",
        "writable": false,
        "enumerable": false,
        "configurable": true
    }
    error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(SimpleCacheEntry.prototype.constructor, 'name')`)
    if (error) { return error }

    actual = Reflect.getOwnPropertyDescriptor(SimpleCacheEntry.prototype.text, 'length')
    expected = {
        "value": 0,
        "writable": false,
        "enumerable": false,
        "configurable": true
    }
    error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(SimpleCacheEntry.prototype.text, 'length')`)
    if (error) { return error }

    actual = Reflect.getOwnPropertyDescriptor(SimpleCacheEntry.prototype.text, 'name')
    expected = {
        "value": "text",
        "writable": false,
        "enumerable": false,
        "configurable": true
    }
    error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(SimpleCacheEntry.prototype.text, 'name')`)
    if (error) { return error }

    actual = Reflect.getOwnPropertyDescriptor(SimpleCacheEntry.prototype.json, 'length')
    expected = {
        "value": 0,
        "writable": false,
        "enumerable": false,
        "configurable": true
    }
    error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(SimpleCacheEntry.prototype.json, 'length')`)
    if (error) { return error }

    actual = Reflect.getOwnPropertyDescriptor(SimpleCacheEntry.prototype.json, 'name')
    expected = {
        "value": "json",
        "writable": false,
        "enumerable": false,
        "configurable": true
    }
    error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(SimpleCacheEntry.prototype.json, 'name')`)
    if (error) { return error }

    actual = Reflect.getOwnPropertyDescriptor(SimpleCacheEntry.prototype.arrayBuffer, 'length')
    expected = {
        "value": 0,
        "writable": false,
        "enumerable": false,
        "configurable": true
    }
    error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(SimpleCacheEntry.prototype.arrayBuffer, 'length')`)
    if (error) { return error }

    actual = Reflect.getOwnPropertyDescriptor(SimpleCacheEntry.prototype.arrayBuffer, 'name')
    expected = {
        "value": "arrayBuffer",
        "writable": false,
        "enumerable": false,
        "configurable": true
    }
    error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(SimpleCacheEntry.prototype.arrayBuffer, 'name')`)
    if (error) { return error }

    return pass()
}

// SimpleCache getOrSet static method
// static getOrSet(key: string, set: () => Promise<{value: BodyInit,  ttl: number}>): Promise<SimpleCacheEntry>;
// static getOrSet(key: string, set: () => Promise<{value: ReadableStream, ttl: number, length: number}>): Promise<SimpleCacheEntry>;
{
    routes.set("/simple-cache/getOrSet/called-as-constructor", () => {
        let key = "/simple-cache/getOrSet/called-as-constructor"
        error = assertThrows(() => {
            new SimpleCache.getOrSet(key, async () => {
                return {
                    value: 'meow',
                    ttl: 10
                }
            });
        }, TypeError, `SimpleCache.getOrSet is not a constructor`)
        if (error) { return error }
        return pass()
    });
    routes.set("/simple-cache/getOrSet/no-parameters-supplied", async () => {
        error = await assertRejects(async () => {
            await SimpleCache.getOrSet()
        }, TypeError, `SimpleCache.getOrSet: At least 2 arguments required, but only 0 passed`)
        if (error) { return error }
        return pass()
    });
    // Ensure we correctly coerce the key parameter to a string as according to
    // https://tc39.es/ecma262/#sec-tostring
    routes.set("/simple-cache/getOrSet/key-parameter-calls-7.1.17-ToString", async () => {
        let sentinel = { sentinel: 'sentinel' };
        const test = async () => {
            const key = {
                toString() {
                    throw sentinel;
                }
            }
            await SimpleCache.getOrSet(key, async () => {
                return {
                    value: 'meow',
                    ttl: 10
                }
            });
        }
        error = await assertRejects(test)
        if (error) { return error }
        try {
            await test()
        } catch (thrownError) {
            error = assert(thrownError, sentinel, 'thrownError === sentinel')
            if (error) { return error }
        }
        error = await assertRejects(async () => {
            await SimpleCache.getOrSet(Symbol(), async () => {
                return {
                    value: 'meow',
                    ttl: 10
                }
            });
        }, TypeError, `can't convert symbol to string`)
        if (error) { return error }
        return pass()
    });

    // TODO: second parameter not a function

    routes.set("/simple-cache/getOrSet/key-parameter-empty-string", async () => {
        error = await assertRejects(async () => {
            await SimpleCache.getOrSet('', async () => {
                return {
                    value: 'meow',
                    ttl: 10
                }
            });
        }, Error, `SimpleCache.getOrSet: key can not be an empty string`)
        if (error) { return error }
        return pass()
    });
    routes.set("/simple-cache/getOrSet/key-parameter-8135-character-string", async () => {
        error = await assertDoesNotThrow(async () => {
            const key = 'a'.repeat(8135)
            await SimpleCache.getOrSet(key, async () => {
                return {
                    value: 'meow',
                    ttl: 10
                }
            });
        })
        if (error) { return error }
        return pass()
    });
    routes.set("/simple-cache/getOrSet/key-parameter-8136-character-string", async () => {
        error = await assertRejects(async () => {
            const key = 'a'.repeat(8136)
            await SimpleCache.getOrSet(key, async () => {
                return {
                    value: 'meow',
                    ttl: 10
                }
            });
        }, Error, `SimpleCache.getOrSet: key is too long, the maximum allowed length is 8135.`)
        if (error) { return error }
        return pass()
    });
    // Ensure we correctly coerce the ttl field to a number as according to
    // https://tc39.es/ecma262/#sec-tonumber
    routes.set("/simple-cache/getOrSet/ttl-field-7.1.4-ToNumber", async () => {
        let key = "/simple-cache/getOrSet/ttl-field-7.1.4-ToNumber";
        let sentinel = { sentinel: 'sentinel' };
        let requestedType;
        const test = async () => {
            const ttl = {
                [Symbol.toPrimitive](type) {
                    requestedType = type;
                    throw sentinel;
                }
            }
            await SimpleCache.getOrSet(key, async () => {
                return {
                    value: 'meow',
                    ttl
                }
            });
        }
        let error = await assertRejects(test)
        if (error) { return error }
        try {
            await test()
        } catch (thrownError) {
            let error = assert(thrownError, sentinel, 'thrownError === sentinel')
            if (error) { return error }
            error = assert(requestedType, 'number', 'requestedType === "number"')
            if (error) { return error }
        }
        error = await assertRejects(() => SimpleCache.getOrSet(key, async () => {
            return {
                value: 'meow',
                ttl: Symbol()
            }
        }), TypeError, `can't convert symbol to number`)
        if (error) { return error }
        return pass()
    });
    routes.set("/simple-cache/getOrSet/ttl-field-negative-number", async () => {
        let key = "/simple-cache/getOrSet/ttl-field-negative-number"
        error = await assertRejects(async () => {
            await SimpleCache.getOrSet(key, async () => {
                return {
                    value: 'meow',
                    ttl: -1
                }
            });
        }, Error, `SimpleCache.getOrSet: TTL field is an invalid value, only positive numbers can be used for TTL values.`)
        if (error) { return error }
        return pass()
    });
    routes.set("/simple-cache/getOrSet/ttl-field-NaN", async () => {
        let key = "/simple-cache/getOrSet/ttl-field-NaN"
        error = await assertRejects(async () => {
            await SimpleCache.getOrSet(key, async () => {
                return {
                    value: 'meow',
                    ttl: NaN
                }
            });
        }, Error, `SimpleCache.getOrSet: TTL field is an invalid value, only positive numbers can be used for TTL values.`)
        if (error) { return error }
        return pass()
    });
    routes.set("/simple-cache/getOrSet/ttl-field-Infinity", async () => {
        let key = "/simple-cache/getOrSet/ttl-field-Infinity";
        error = await assertRejects(async () => {
            await SimpleCache.getOrSet(key, async () => {
                return {
                    value: 'meow',
                    ttl: Number.POSITIVE_INFINITY
                }
            });
        }, Error, `SimpleCache.getOrSet: TTL field is an invalid value, only positive numbers can be used for TTL values.`)
        if (error) { return error }
        error = await assertRejects(async () => {
            await SimpleCache.getOrSet(key, async () => {
                return {
                    value: 'meow',
                    ttl: Number.NEGATIVE_INFINITY
                }
            });
        }, Error, `SimpleCache.getOrSet: TTL field is an invalid value, only positive numbers can be used for TTL values.`)
        if (error) { return error }
        return pass()
    });
    routes.set("/simple-cache/getOrSet/value-field-as-undefined", async () => {
        let key = "/simple-cache/getOrSet/value-field-as-undefined";
        error = await assertResolves(async () => {
            await SimpleCache.getOrSet(key, async () => { return { value: undefined, ttl: 1 } })
        });
        if (error) { return error }
        return pass()
    });
    // - ReadableStream
    routes.set("/simple-cache/getOrSet/value-field-readablestream-missing-length-field", async () => {
        let key = "/simple-cache/getOrSet/value-field-readablestream-missing-length-field";
        // TODO: remove this when streams are supported
        let error = await assertRejects(async () => {
            const stream = iteratableToStream([])
            await SimpleCache.getOrSet(key, async () => { return { value: stream, ttl: 1 } })
        }, TypeError, `Content-provided streams are not yet supported for streaming into SimpleCache`)
        if (error) { return error }
        return pass()
    });
    routes.set("/simple-cache/getOrSet/value-field-readablestream-negative-length-field", async () => {
        let key = "/simple-cache/getOrSet/value-field-readablestream-negative-length-field";
        // TODO: remove this when streams are supported
        let error = await assertRejects(async () => {
            const stream = iteratableToStream([])
            await SimpleCache.getOrSet(key, async () => { return { value: stream, ttl: 1, length: -1 } })
        }, TypeError, `Content-provided streams are not yet supported for streaming into SimpleCache`)
        if (error) { return error }
        return pass()
    });
    routes.set("/simple-cache/getOrSet/value-field-readablestream-nan-length-field", async () => {
        let key = "/simple-cache/getOrSet/value-field-readablestream-nan-length-field";
        // TODO: remove this when streams are supported
        let error = await assertRejects(async () => {
            const stream = iteratableToStream([])
            await SimpleCache.getOrSet(key, async () => { return { value: stream, ttl: 1, length: NaN } })
        }, TypeError, `Content-provided streams are not yet supported for streaming into SimpleCache`)
        if (error) { return error }
        return pass()
    });
    routes.set("/simple-cache/getOrSet/value-field-readablestream-negative-infinity-length-field", async () => {
        let key = "/simple-cache/getOrSet/value-field-readablestream-negative-infinity-length-field";
        // TODO: remove this when streams are supported
        let error = await assertRejects(async () => {
            const stream = iteratableToStream([])
            await SimpleCache.getOrSet(key, async () => { return { value: stream, ttl: 1, length: Number.NEGATIVE_INFINITY } })
        }, TypeError, `Content-provided streams are not yet supported for streaming into SimpleCache`)
        if (error) { return error }
        return pass()
    });
    routes.set("/simple-cache/getOrSet/value-field-readablestream-positive-infinity-length-field", async () => {
        let key = "/simple-cache/getOrSet/value-field-readablestream-positive-infinity-length-field";
        // TODO: remove this when streams are supported
        let error = await assertRejects(async () => {
            const stream = iteratableToStream([])
            await SimpleCache.getOrSet(key, async () => { return { value: stream, ttl: 1, length: Number.POSITIVE_INFINITY } })
        }, TypeError, `Content-provided streams are not yet supported for streaming into SimpleCache`)
        if (error) { return error }
        return pass()
    });
    // Ensure we correctly coerce the length field to a number as according to
    // https://tc39.es/ecma262/#sec-tonumber
    routes.set("/simple-cache/getOrSet/length-field-7.1.4-ToNumber", async () => {
        let key = "/simple-cache/getOrSet/length-field-7.1.4-ToNumber";
        const res = await fetch('https://compute-sdk-test-backend.edgecompute.app/', {
            backend: "TheOrigin",
        })
        let sentinel = { sentinel: 'sentinel' };
        let requestedType;
        const test = async () => {
            const length = {
                [Symbol.toPrimitive](type) {
                    requestedType = type;
                    throw sentinel;
                }
            }
            await SimpleCache.getOrSet(key, async () => { return { value: res.body, ttl: 1, length } })
        }
        let error = await assertRejects(test)
        if (error) { return error }
        try {
            await test()
        } catch (thrownError) {
            let error = assert(thrownError, sentinel, 'thrownError === sentinel')
            if (error) { return error }
            error = assert(requestedType, 'number', 'requestedType === "number"')
            if (error) { return error }
        }
        error = await assertRejects(async () =>
            await SimpleCache.getOrSet(key, async () => { return { value: res.body, ttl: 1, length: Symbol() } })
            , TypeError, `can't convert symbol to number`)
        if (error) { return error }
        return pass()
    });
    routes.set("/simple-cache/getOrSet/value-field-readablestream-empty", async () => {
        let key = "/simple-cache/getOrSet/value-field-readablestream-empty";
        // TODO: remove this when streams are supported
        let error = await assertRejects(async () => {
            const stream = iteratableToStream([])
            await SimpleCache.getOrSet(key, async () => { return { value: stream, ttl: 1, length: 0 } })
        }, TypeError, `Content-provided streams are not yet supported for streaming into SimpleCache`)
        if (error) { return error }
        return pass()
    });
    routes.set("/simple-cache/getOrSet/value-field-readablestream-locked", async () => {
        let key = "/simple-cache/getOrSet/value-field-readablestream-locked";
        const stream = iteratableToStream([])
        // getReader() causes the stream to become locked
        stream.getReader()
        let error = await assertRejects(async () => {
            await SimpleCache.getOrSet(key, async () => { return { value: stream, ttl: 1, length: 0 } })
        }, TypeError, `Can't use a ReadableStream that's locked or has ever been read from or canceled`)
        if (error) { return error }
        return pass()
    });
    routes.set("/simple-cache/getOrSet/value-field-readablestream", async () => {
        let key = "/simple-cache/getOrSet/value-field-readablestream";
        const res = await fetch('https://compute-sdk-test-backend.edgecompute.app/', {
            backend: "TheOrigin",
        });
        error = await assertResolves(async () => {
            await SimpleCache.getOrSet(key, async () => { return { value: res.body, ttl: 100, length: res.headers.get('content-length') } })
        });
        if (error) { return error }
        return pass()
    });

    // - URLSearchParams
    routes.set("/simple-cache/getOrSet/value-field-URLSearchParams", async () => {
        let key = "/simple-cache/getOrSet/value-field-URLSearchParams";
        const items = [
            new URLSearchParams,
            new URLSearchParams({ a: 'b', c: 'd' }),
        ];
        for (const searchParams of items) {
            error = await assertResolves(async () => {
                await SimpleCache.getOrSet(key, async () => { return { value: searchParams, ttl: 1 } })
            });
            if (error) { return error }
        }
        return pass()
    });
    // - USV strings
    routes.set("/simple-cache/getOrSet/value-field-strings", async () => {
        let key = "/simple-cache/getOrSet/value-field-strings";
        const strings = [
            // empty
            '',
            // lone surrogate
            '\uD800',
            // surrogate pair
            '𠈓',
            String('carrot'),
        ];
        for (const string of strings) {
            error = await assertResolves(async () => {
                await SimpleCache.getOrSet(key, async () => { return { value: string, ttl: 1 } })
            });
            if (error) { return error }
        }
        return pass()
    });

    // https://tc39.es/ecma262/#sec-tostring
    routes.set("/simple-cache/getOrSet/value-field-calls-7.1.17-ToString", async () => {
        let key = "/simple-cache/getOrSet/value-field-calls-7.1.17-ToString";
        let sentinel = { sentinel: 'sentinel' };
        const test = async () => {
            const value = {
                toString() {
                    throw sentinel;
                }
            }
            await SimpleCache.getOrSet(key, async () => { return { value, ttl: 1 } })
        }
        let error = await assertRejects(test)
        if (error) { return error }
        try {
            await test()
        } catch (thrownError) {
            let error = assert(thrownError, sentinel, 'thrownError === sentinel')
            if (error) { return error }
        }
        error = await assertRejects(async () => {
            await SimpleCache.getOrSet(key, async () => { return { value: Symbol(), ttl: 1 } })
        }, TypeError, `can't convert symbol to string`)
        if (error) { return error }
        return pass()
    });

    // - buffer source
    routes.set("/simple-cache/getOrSet/value-field-buffer", async () => {
        let key = "/simple-cache/getOrSet/value-field-buffer";
        const typedArrayConstructors = [
            Int8Array,
            Int16Array,
            Int32Array,
            Float32Array,
            Float64Array,
            BigInt64Array,
            Uint8Array,
            Uint8ClampedArray,
            Uint16Array,
            Uint32Array,
            BigUint64Array,
        ];
        for (const constructor of typedArrayConstructors) {
            const typedArray = new constructor(8);
            error = await assertResolves(async () => {
                await SimpleCache.getOrSet(key + constructor.name, async () => { return { value: typedArray.buffer, ttl: 1 } })
            });
            if (error) { return error }
        }
        return pass()
    });

    routes.set("/simple-cache/getOrSet/value-field-typed-arrays", async () => {
        let key = "/simple-cache/getOrSet/value-field-typed-arrays";
        const typedArrayConstructors = [
            Int8Array,
            Int16Array,
            Int32Array,
            Float32Array,
            Float64Array,
            BigInt64Array,
            Uint8Array,
            Uint8ClampedArray,
            Uint16Array,
            Uint32Array,
            BigUint64Array,
        ];
        for (const constructor of typedArrayConstructors) {
            const typedArray = new constructor(8);
            error = await assertResolves(async () => {
                await SimpleCache.getOrSet(key + constructor.name, async () => { return { value: typedArray, ttl: 1 } })
            });
            if (error) { return error }
        }
        return pass()
    });
    routes.set("/simple-cache/getOrSet/value-field-dataview", async () => {
        let key = "/simple-cache/getOrSet/value-field-dataview";
        const typedArrayConstructors = [
            Int8Array,
            Uint8Array,
            Uint8ClampedArray,
            Int16Array,
            Uint16Array,
            Int32Array,
            Uint32Array,
            Float32Array,
            Float64Array,
            BigInt64Array,
            BigUint64Array,
        ];
        for (const constructor of typedArrayConstructors) {
            const typedArray = new constructor(8);
            const view = new DataView(typedArray.buffer);
            error = await assertResolves(async () => {
                await SimpleCache.getOrSet(key + constructor.name, async () => { return { value: view, ttl: 1 } })
            });
            if (error) { return error }
        }
        return pass()
    });
    routes.set("/simple-cache/getOrSet/returns-SimpleCacheEntry", async () => {
        let key = "/simple-cache/getOrSet/returns-SimpleCacheEntry";
        let result = await SimpleCache.getOrSet(key, async () => {
            return {
                value: 'meow',
                ttl: 10
            }
        });
        error = assert(result instanceof SimpleCacheEntry, true, "result instanceof SimpleCacheEntry")
        if (error) { return error }
        return pass()
    });
    routes.set("/simple-cache/getOrSet/executes-the-set-method-when-key-not-in-cache", async () => {
        let called = false;
        await SimpleCache.getOrSet(Math.random(), async () => {
            called = true;
            return {
                value: 'meow',
                ttl: 10
            }
        });
        error = assert(called, true, "called === true")
        if (error) { return error }
        return pass()
    });
    routes.set("/simple-cache/getOrSet/does-not-execute-the-set-method-when-key-is-in-cache", async () => {
        let key = Math.random();
        SimpleCache.set(key, 'meow', 100);
        let called = false;
        await SimpleCache.getOrSet(key, async () => {
            called = true;
            return {
                value: 'meow',
                ttl: 10
            }
        });
        error = assert(called, false, "called === false")
        if (error) { return error }
        return pass()
    });
}


