/// <reference path="../../../../../types/index.d.ts" />
/* eslint-env serviceworker */

import { pass, assert, assertDoesNotThrow, assertThrows, sleep, streamToString, assertResolves } from "./assertions.js";
import { routes } from "./routes.js";
import { CoreCache, CacheEntry, CacheState, TransactionCacheEntry } from 'fastly:cache';
import { FastlyBody } from "fastly:body";

function iteratableToStream(iterable) {
    return new ReadableStream({
        async pull(controller) {
            for await (const value of iterable) {
                controller.enqueue(value);
            }
            controller.close();
        }
    });
}

let error;

// FastlyBody
{
    routes.set("/FastlyBody/interface", () => {
        let actual = Reflect.ownKeys(FastlyBody)
        let expected = ["prototype", "length", "name"]
        error = assert(actual, expected, `Reflect.ownKeys(FastlyBody)`)
        if (error) { return error }

        // Check the prototype descriptors are correct
        {
            actual = Reflect.getOwnPropertyDescriptor(FastlyBody, 'prototype')
            expected = {
                "value": FastlyBody.prototype,
                "writable": false,
                "enumerable": false,
                "configurable": false
            }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(FastlyBody, 'prototype')`)
            if (error) { return error }
        }

        // Check the constructor function's defined parameter length is correct
        {
            actual = Reflect.getOwnPropertyDescriptor(FastlyBody, 'length')
            expected = {
                "value": 0,
                "writable": false,
                "enumerable": false,
                "configurable": true
            }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(FastlyBody, 'length')`)
            if (error) { return error }
        }

        // Check the constructor function's name is correct
        {
            actual = Reflect.getOwnPropertyDescriptor(FastlyBody, 'name')
            expected = {
                "value": "FastlyBody",
                "writable": false,
                "enumerable": false,
                "configurable": true
            }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(FastlyBody, 'name')`)
            if (error) { return error }
        }

        // Check the prototype has the correct keys
        {
            actual = Reflect.ownKeys(FastlyBody.prototype)
            expected = ["constructor", "concat", "read", "append", "prepend", "close", Symbol.toStringTag]
            error = assert(actual, expected, `Reflect.ownKeys(FastlyBody.prototype)`)
            if (error) { return error }
        }

        // Check the constructor on the prototype is correct
        {
            actual = Reflect.getOwnPropertyDescriptor(FastlyBody.prototype, 'constructor')
            expected = { "writable": true, "enumerable": false, "configurable": true, value: FastlyBody.prototype.constructor }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(FastlyBody.prototype, 'constructor')`)
            if (error) { return error }

            error = assert(typeof FastlyBody.prototype.constructor, 'function', `typeof FastlyBody.prototype.constructor`)
            if (error) { return error }

            actual = Reflect.getOwnPropertyDescriptor(FastlyBody.prototype.constructor, 'length')
            expected = {
                "value": 0,
                "writable": false,
                "enumerable": false,
                "configurable": true
            }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(FastlyBody.prototype.constructor, 'length')`)
            if (error) { return error }

            actual = Reflect.getOwnPropertyDescriptor(FastlyBody.prototype.constructor, 'name')
            expected = {
                "value": "FastlyBody",
                "writable": false,
                "enumerable": false,
                "configurable": true
            }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(FastlyBody.prototype.constructor, 'name')`)
            if (error) { return error }
        }

        // Check the Symbol.toStringTag on the prototype is correct
        {
            actual = Reflect.getOwnPropertyDescriptor(FastlyBody.prototype, Symbol.toStringTag)
            expected = { "writable": false, "enumerable": false, "configurable": true, value: "FastlyBody" }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(FastlyBody.prototype, [Symbol.toStringTag])`)
            if (error) { return error }

            error = assert(typeof FastlyBody.prototype[Symbol.toStringTag], 'string', `typeof FastlyBody.prototype[Symbol.toStringTag]`)
            if (error) { return error }
        }

        // Check the concat method has correct descriptors, length and name
        {
            actual = Reflect.getOwnPropertyDescriptor(FastlyBody.prototype, 'concat')
            expected = { "writable": true, "enumerable": true, "configurable": true, value: FastlyBody.prototype.concat }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(FastlyBody.prototype, 'concat')`)
            if (error) { return error }

            error = assert(typeof FastlyBody.prototype.concat, 'function', `typeof FastlyBody.prototype.concat`)
            if (error) { return error }

            actual = Reflect.getOwnPropertyDescriptor(FastlyBody.prototype.concat, 'length')
            expected = {
                "value": 1,
                "writable": false,
                "enumerable": false,
                "configurable": true
            }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(FastlyBody.prototype.concat, 'length')`)
            if (error) { return error }

            actual = Reflect.getOwnPropertyDescriptor(FastlyBody.prototype.concat, 'name')
            expected = {
                "value": "concat",
                "writable": false,
                "enumerable": false,
                "configurable": true
            }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(FastlyBody.prototype.concat, 'name')`)
            if (error) { return error }
        }

        // Check the read method has correct descriptors, length and name
        {
            actual = Reflect.getOwnPropertyDescriptor(FastlyBody.prototype, 'read')
            expected = { "writable": true, "enumerable": true, "configurable": true, value: FastlyBody.prototype.read }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(FastlyBody.prototype, 'read')`)
            if (error) { return error }

            error = assert(typeof FastlyBody.prototype.read, 'function', `typeof FastlyBody.prototype.read`)
            if (error) { return error }

            actual = Reflect.getOwnPropertyDescriptor(FastlyBody.prototype.read, 'length')
            expected = {
                "value": 1,
                "writable": false,
                "enumerable": false,
                "configurable": true
            }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(FastlyBody.prototype.read, 'length')`)
            if (error) { return error }

            actual = Reflect.getOwnPropertyDescriptor(FastlyBody.prototype.read, 'name')
            expected = {
                "value": "read",
                "writable": false,
                "enumerable": false,
                "configurable": true
            }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(FastlyBody.prototype.read, 'name')`)
            if (error) { return error }
        }

        // Check the append method has correct descriptors, length and name
        {
            actual = Reflect.getOwnPropertyDescriptor(FastlyBody.prototype, 'append')
            expected = { "writable": true, "enumerable": true, "configurable": true, value: FastlyBody.prototype.append }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(FastlyBody.prototype, 'append')`)
            if (error) { return error }

            error = assert(typeof FastlyBody.prototype.append, 'function', `typeof FastlyBody.prototype.append`)
            if (error) { return error }

            actual = Reflect.getOwnPropertyDescriptor(FastlyBody.prototype.append, 'length')
            expected = {
                "value": 1,
                "writable": false,
                "enumerable": false,
                "configurable": true
            }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(FastlyBody.prototype.append, 'length')`)
            if (error) { return error }

            actual = Reflect.getOwnPropertyDescriptor(FastlyBody.prototype.append, 'name')
            expected = {
                "value": "append",
                "writable": false,
                "enumerable": false,
                "configurable": true
            }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(FastlyBody.prototype.append, 'name')`)
            if (error) { return error }
        }

        // Check the prepend method has correct descriptors, length and name
        {
            actual = Reflect.getOwnPropertyDescriptor(FastlyBody.prototype, 'prepend')
            expected = { "writable": true, "enumerable": true, "configurable": true, value: FastlyBody.prototype.prepend }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(FastlyBody.prototype, 'prepend')`)
            if (error) { return error }

            error = assert(typeof FastlyBody.prototype.prepend, 'function', `typeof FastlyBody.prototype.prepend`)
            if (error) { return error }

            actual = Reflect.getOwnPropertyDescriptor(FastlyBody.prototype.prepend, 'length')
            expected = {
                "value": 1,
                "writable": false,
                "enumerable": false,
                "configurable": true
            }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(FastlyBody.prototype.prepend, 'length')`)
            if (error) { return error }

            actual = Reflect.getOwnPropertyDescriptor(FastlyBody.prototype.prepend, 'name')
            expected = {
                "value": "prepend",
                "writable": false,
                "enumerable": false,
                "configurable": true
            }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(FastlyBody.prototype.prepend, 'name')`)
            if (error) { return error }
        }

        // Check the close method has correct descriptors, length and name
        {
            actual = Reflect.getOwnPropertyDescriptor(FastlyBody.prototype, 'close')
            expected = { "writable": true, "enumerable": true, "configurable": true, value: FastlyBody.prototype.close }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(FastlyBody.prototype, 'close')`)
            if (error) { return error }

            error = assert(typeof FastlyBody.prototype.close, 'function', `typeof FastlyBody.prototype.close`)
            if (error) { return error }

            actual = Reflect.getOwnPropertyDescriptor(FastlyBody.prototype.close, 'length')
            expected = {
                "value": 0,
                "writable": false,
                "enumerable": false,
                "configurable": true
            }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(FastlyBody.prototype.close, 'length')`)
            if (error) { return error }

            actual = Reflect.getOwnPropertyDescriptor(FastlyBody.prototype.close, 'name')
            expected = {
                "value": "close",
                "writable": false,
                "enumerable": false,
                "configurable": true
            }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(FastlyBody.prototype.close, 'name')`)
            if (error) { return error }
        }


        return pass("ok")
    });

    // constructor
    {

        routes.set("/FastlyBody/constructor/called-as-regular-function", () => {
            error = assertThrows(() => {
                FastlyBody()
            }, TypeError)
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/FastlyBody/constructor/called-as-constructor", () => {
            error = assertDoesNotThrow(() => new FastlyBody())
            if (error) { return error }
            return pass("ok")
        });
    }
    // append(data: BodyInit): void;
    {
        routes.set("/FastlyBody/append/called-as-constructor", () => {
            let error = assertThrows(() => {
                new FastlyBody.append('1')
            }, TypeError)
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/FastlyBody/append/data-parameter-not-supplied", () => {
            let error = assertThrows(() => {
                const body = new FastlyBody()
                body.append()
            }, TypeError, `append: At least 1 argument required, but only 0 passed`)
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/FastlyBody/append/data-parameter-wrong-type", () => {
            let error = assertThrows(() => {
                const body = new FastlyBody()
                body.append(Symbol())
            })
            if (error) { return error }
            return pass("ok")
        });
        // - ReadableStream
        routes.set("/FastlyBody/append/data-parameter-readablestream-guest-backed", () => {
            // TODO: update this when streams are supported
            let error = assertThrows(() => {
                const stream = iteratableToStream([])
                const body = new FastlyBody()
                body.append(stream)
            }, TypeError, `Content-provided streams are not yet supported for appending onto a FastlyBody`)
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/FastlyBody/append/data-parameter-readablestream-host-backed", async () => {
            const res = await fetch('https://compute-sdk-test-backend.edgecompute.app/', {
                backend: "TheOrigin",
            })
            const body = new FastlyBody()
            let result = body.append(res.body)
            error = assert(result, undefined, `body.append(res.body)`)
            if (error) { return error }
            return pass("ok")
        });
        // - URLSearchParams
        routes.set("/FastlyBody/append/data-parameter-URLSearchParams", () => {
            const items = [
                new URLSearchParams,
                new URLSearchParams({ a: 'b', c: 'd' }),
            ];
            const body = new FastlyBody()
            for (const searchParams of items) {
                let result = body.append(searchParams)
                error = assert(result, undefined, `await body.append(searchParams)`)
                if (error) { return error }
            }
            return pass("ok")
        });
        // - USV strings
        routes.set("/FastlyBody/append/data-parameter-strings", () => {
            const strings = [
                // empty
                '',
                // lone surrogate
                '\uD800',
                // surrogate pair
                '𠈓',
                String('carrot'),
            ];
            const body = new FastlyBody()
            for (const string of strings) {
                let result = body.append(string)
                error = assert(result, undefined, `body.append(string)`)
                if (error) { return error }
            }
            return pass("ok")
        });
        // https://tc39.es/ecma262/#sec-tostring
        routes.set("/FastlyBody/append/data-parameter-calls-7.1.17-ToString", () => {
            let sentinel;
            const test = () => {
                sentinel = Symbol();
                const value = {
                    toString() {
                        throw sentinel;
                    }
                }
                const body = new FastlyBody()
                body.append(value)
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
                const body = new FastlyBody()
                body.append(Symbol())
            }, TypeError, `can't convert symbol to string`)
            if (error) { return error }
            return pass("ok")
        });

        // - buffer source
        routes.set("/FastlyBody/append/data-parameter-buffer", () => {
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
            const body = new FastlyBody()
            for (const constructor of typedArrayConstructors) {
                const typedArray = new constructor(8);
                let result = body.append(typedArray.buffer)
                error = assert(result, undefined, `body.append(typedArray.buffer)`)
                if (error) { return error }
            }
            return pass("ok")
        });
        routes.set("/FastlyBody/append/data-parameter-arraybuffer", () => {
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
            const body = new FastlyBody()
            for (const constructor of typedArrayConstructors) {
                const typedArray = new constructor(8);
                let result = body.append(typedArray.buffer)
                error = assert(result, undefined, `body.append(typedArray.buffer)`)
                if (error) { return error }
            }
            return pass("ok")
        });
        routes.set("/FastlyBody/append/data-parameter-typed-arrays", () => {
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
            const body = new FastlyBody()
            for (const constructor of typedArrayConstructors) {
                const typedArray = new constructor(8);
                let result = body.append(typedArray)
                error = assert(result, undefined, `body.append(typedArray)`)
                if (error) { return error }
            }
            return pass("ok")
        });
        routes.set("/FastlyBody/append/data-parameter-dataview", () => {
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
            const body = new FastlyBody()
            for (const constructor of typedArrayConstructors) {
                const typedArray = new constructor(8);
                const view = new DataView(typedArray.buffer);
                let result = body.append(view)
                error = assert(result, undefined, `body.append(typedArray)`)
                if (error) { return error }
            }
            return pass("ok")
        });
    }
    // prepend(data: BodyInit): void;
    {
        routes.set("/FastlyBody/prepend/called-as-constructor", () => {
            let error = assertThrows(() => {
                new FastlyBody.prepend('1')
            }, TypeError)
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/FastlyBody/prepend/data-parameter-not-supplied", () => {
            let error = assertThrows(() => {
                const body = new FastlyBody()
                body.prepend()
            }, TypeError, `prepend: At least 1 argument required, but only 0 passed`)
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/FastlyBody/prepend/data-parameter-wrong-type", () => {
            let error = assertThrows(() => {
                const body = new FastlyBody()
                body.prepend(Symbol())
            })
            if (error) { return error }
            return pass("ok")
        });
        // - ReadableStream
        routes.set("/FastlyBody/prepend/data-parameter-readablestream-guest-backed", () => {
            // TODO: update this when streams are supported
            let error = assertThrows(() => {
                const stream = iteratableToStream([])
                const body = new FastlyBody()
                body.prepend(stream)
            }, TypeError, `Content-provided streams are not yet supported for prepending onto a FastlyBody`)
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/FastlyBody/prepend/data-parameter-readablestream-host-backed", async () => {
            const res = await fetch('https://compute-sdk-test-backend.edgecompute.app/', {
                backend: "TheOrigin",
            })
            const body = new FastlyBody()
            let result = body.prepend(res.body)
            error = assert(result, undefined, `body.prepend(res.body)`)
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/FastlyBody/prepend/data-parameter-readablestream-locked", () => {
            const stream = iteratableToStream([])
            // getReader() causes the stream to become locked
            stream.getReader()
            const body = new FastlyBody()
            let error = assertThrows(() => {
                body.prepend(stream)
            }, TypeError, `Can't use a ReadableStream that's locked or has ever been read from or canceled`)
            if (error) { return error }
            return pass("ok")
        });

        // - URLSearchParams
        routes.set("/FastlyBody/prepend/data-parameter-URLSearchParams", () => {
            const items = [
                new URLSearchParams,
                new URLSearchParams({ a: 'b', c: 'd' }),
            ];
            const body = new FastlyBody()
            for (const searchParams of items) {
                let result = body.prepend(searchParams)
                error = assert(result, undefined, `await body.prepend(searchParams)`)
                if (error) { return error }
            }
            return pass("ok")
        });
        // - USV strings
        routes.set("/FastlyBody/prepend/data-parameter-strings", () => {
            const strings = [
                // empty
                '',
                // lone surrogate
                '\uD800',
                // surrogate pair
                '𠈓',
                String('carrot'),
            ];
            const body = new FastlyBody()
            for (const string of strings) {
                let result = body.prepend(string)
                error = assert(result, undefined, `body.prepend(string)`)
                if (error) { return error }
            }
            return pass("ok")
        });
        // https://tc39.es/ecma262/#sec-tostring
        routes.set("/FastlyBody/prepend/data-parameter-calls-7.1.17-ToString", () => {
            let sentinel;
            const test = () => {
                sentinel = Symbol();
                const value = {
                    toString() {
                        throw sentinel;
                    }
                }
                const body = new FastlyBody()
                body.prepend(value)
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
                const body = new FastlyBody()
                body.prepend(Symbol())
            }, TypeError, `can't convert symbol to string`)
            if (error) { return error }
            return pass("ok")
        });

        // - buffer source
        routes.set("/FastlyBody/prepend/data-parameter-buffer", () => {
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
            const body = new FastlyBody()
            for (const constructor of typedArrayConstructors) {
                const typedArray = new constructor(8);
                let result = body.prepend(typedArray.buffer)
                error = assert(result, undefined, `body.prepend(typedArray.buffer)`)
                if (error) { return error }
            }
            return pass("ok")
        });
        routes.set("/FastlyBody/prepend/data-parameter-arraybuffer", () => {
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
            const body = new FastlyBody()
            for (const constructor of typedArrayConstructors) {
                const typedArray = new constructor(8);
                let result = body.prepend(typedArray.buffer)
                error = assert(result, undefined, `body.prepend(typedArray.buffer)`)
                if (error) { return error }
            }
            return pass("ok")
        });
        routes.set("/FastlyBody/prepend/data-parameter-typed-arrays", () => {
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
            const body = new FastlyBody()
            for (const constructor of typedArrayConstructors) {
                const typedArray = new constructor(8);
                let result = body.prepend(typedArray)
                error = assert(result, undefined, `body.prepend(typedArray)`)
                if (error) { return error }
            }
            return pass("ok")
        });
        routes.set("/FastlyBody/prepend/data-parameter-dataview", () => {
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
            const body = new FastlyBody()
            for (const constructor of typedArrayConstructors) {
                const typedArray = new constructor(8);
                const view = new DataView(typedArray.buffer);
                let result = body.prepend(view)
                error = assert(result, undefined, `body.prepend(typedArray)`)
                if (error) { return error }
            }
            return pass("ok")
        });
    }
    // concat(dest: FastlyBody): void;
    {
        routes.set("/FastlyBody/concat/called-as-constructor", () => {
            let error = assertThrows(() => {
                new FastlyBody.concat(new FastlyBody())
            }, TypeError)
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/FastlyBody/concat/dest-parameter-not-supplied", () => {
            let error = assertThrows(() => {
                const body = new FastlyBody()
                body.concat()
            }, TypeError, `concat: At least 1 argument required, but only 0 passed`)
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/FastlyBody/concat/dest-parameter-wrong-type", () => {
            let error = assertThrows(() => {
                const body = new FastlyBody()
                body.concat('hello')
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/FastlyBody/concat/concat-same-fastlybody-twice", () => {
            let error = assertThrows(() => {
                const body = new FastlyBody()
                const body2 = new FastlyBody()
                body.concat(body2)
                body.concat(body2)
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/FastlyBody/concat/happy-path", () => {
            let error = assertDoesNotThrow(() => {
                const body = new FastlyBody()
                body.concat(new FastlyBody())
            })
            if (error) { return error }
            return pass("ok")
        });
    }
    // read(chunkSize: number): ArrayBuffer;
    {
        routes.set("/FastlyBody/read/called-as-constructor", () => {
            let error = assertThrows(() => {
                new FastlyBody.read(1)
            }, TypeError)
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/FastlyBody/read/chunkSize-parameter-not-supplied", () => {
            let error = assertThrows(() => {
                const body = new FastlyBody()
                body.read()
            }, TypeError, `read: At least 1 argument required, but only 0 passed`)
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/FastlyBody/read/chunkSize-parameter-wrong-type", () => {
            let error = assertThrows(() => {
                const body = new FastlyBody()
                body.read(Symbol())
            })
            if (error) { return error }
            return pass("ok")
        });
        // negative
        routes.set("/FastlyBody/read/chunkSize-parameter-negative", () => {
            let error = assertThrows(() => {
                const body = new FastlyBody()
                body.append('hello world')
                body.read(-1)
            })
            if (error) { return error }
            return pass("ok")
        });
        // infinity
        routes.set("/FastlyBody/read/chunkSize-parameter-infinity", () => {
            let error = assertThrows(() => {
                const body = new FastlyBody()
                body.append('hello world')
                body.read(Infinity)
            })
            if (error) { return error }
            return pass("ok")
        });
        // NaN
        routes.set("/FastlyBody/read/chunkSize-parameter-NaN", () => {
            let error = assertThrows(() => {
                const body = new FastlyBody()
                body.append('hello world')
                body.read(NaN)
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/FastlyBody/read/happy-path", () => {
            const body = new FastlyBody()
            body.append('world')
            body.prepend('hello ')
            const body2 = new FastlyBody()
            body2.append('!')
            body.concat(body2)
            const decoder = new TextDecoder()
            let result = decoder.decode(body.read(1))
            let error = assert(result, 'h', `body.read(1)`)
            if (error) { return error }
            result = decoder.decode(body.read(1));
            error = assert(result, 'e', `body.read(1)`)
            if (error) { return error }
            return pass("ok")
        });
    }
    // close(): void;
    {
        routes.set("/FastlyBody/close/called-as-constructor", () => {
            let error = assertThrows(() => {
                new CoreCache.lookup('1')
            }, TypeError)
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/FastlyBody/close/called-once", () => {
            let error = assertThrows(() => {
                CoreCache.lookup()
            }, TypeError, `CoreCache.lookup: At least 1 argument required, but only 0 passed`)
            if (error) { return error }
            return pass("ok")
        });
    }
}

// CoreCache
{
    routes.set("/core-cache/interface", () => {
        let actual = Reflect.ownKeys(CoreCache)
        let expected = ["prototype", "lookup", "insert", "transactionLookup", "length", "name"]
        error = assert(actual, expected, `Reflect.ownKeys(CoreCache)`)
        if (error) { return error }

        // Check the prototype descriptors are correct
        {
            actual = Reflect.getOwnPropertyDescriptor(CoreCache, 'prototype')
            expected = {
                "value": CoreCache.prototype,
                "writable": false,
                "enumerable": false,
                "configurable": false
            }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(CoreCache, 'prototype')`)
            if (error) { return error }
        }

        // Check the constructor function's defined parameter length is correct
        {
            actual = Reflect.getOwnPropertyDescriptor(CoreCache, 'length')
            expected = {
                "value": 0,
                "writable": false,
                "enumerable": false,
                "configurable": true
            }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(CoreCache, 'length')`)
            if (error) { return error }
        }

        // Check the constructor function's name is correct
        {
            actual = Reflect.getOwnPropertyDescriptor(CoreCache, 'name')
            expected = {
                "value": "CoreCache",
                "writable": false,
                "enumerable": false,
                "configurable": true
            }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(CoreCache, 'name')`)
            if (error) { return error }
        }

        // Check the prototype has the correct keys
        {
            actual = Reflect.ownKeys(CoreCache.prototype)
            expected = ["constructor", Symbol.toStringTag]
            error = assert(actual, expected, `Reflect.ownKeys(CoreCache.prototype)`)
            if (error) { return error }
        }

        // Check the constructor on the prototype is correct
        {
            actual = Reflect.getOwnPropertyDescriptor(CoreCache.prototype, 'constructor')
            expected = { "writable": true, "enumerable": false, "configurable": true, value: CoreCache.prototype.constructor }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(CoreCache.prototype, 'constructor')`)
            if (error) { return error }

            error = assert(typeof CoreCache.prototype.constructor, 'function', `typeof CoreCache.prototype.constructor`)
            if (error) { return error }

            actual = Reflect.getOwnPropertyDescriptor(CoreCache.prototype.constructor, 'length')
            expected = {
                "value": 0,
                "writable": false,
                "enumerable": false,
                "configurable": true
            }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(CoreCache.prototype.constructor, 'length')`)
            if (error) { return error }

            actual = Reflect.getOwnPropertyDescriptor(CoreCache.prototype.constructor, 'name')
            expected = {
                "value": "CoreCache",
                "writable": false,
                "enumerable": false,
                "configurable": true
            }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(CoreCache.prototype.constructor, 'name')`)
            if (error) { return error }
        }

        // Check the Symbol.toStringTag on the prototype is correct
        {
            actual = Reflect.getOwnPropertyDescriptor(CoreCache.prototype, Symbol.toStringTag)
            expected = { "writable": false, "enumerable": false, "configurable": true, value: "CoreCache" }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(CoreCache.prototype, [Symbol.toStringTag])`)
            if (error) { return error }

            error = assert(typeof CoreCache.prototype[Symbol.toStringTag], 'string', `typeof CoreCache.prototype[Symbol.toStringTag]`)
            if (error) { return error }
        }

        // Check the lookup static method has correct descriptors, length and name
        {
            actual = Reflect.getOwnPropertyDescriptor(CoreCache, 'lookup')
            expected = { "writable": true, "enumerable": true, "configurable": true, value: CoreCache.lookup }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(CoreCache, 'lookup')`)
            if (error) { return error }

            error = assert(typeof CoreCache.lookup, 'function', `typeof CoreCache.lookup`)
            if (error) { return error }

            actual = Reflect.getOwnPropertyDescriptor(CoreCache.lookup, 'length')
            expected = {
                "value": 1,
                "writable": false,
                "enumerable": false,
                "configurable": true
            }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(CoreCache.lookup, 'length')`)
            if (error) { return error }

            actual = Reflect.getOwnPropertyDescriptor(CoreCache.lookup, 'name')
            expected = {
                "value": "lookup",
                "writable": false,
                "enumerable": false,
                "configurable": true
            }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(CoreCache.lookup, 'name')`)
            if (error) { return error }
        }

        // Check the insert static method has correct descriptors, length and name
        {
            actual = Reflect.getOwnPropertyDescriptor(CoreCache, 'insert')
            expected = { "writable": true, "enumerable": true, "configurable": true, value: CoreCache.insert }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(CoreCache, 'insert')`)
            if (error) { return error }

            error = assert(typeof CoreCache.insert, 'function', `typeof CoreCache.insert`)
            if (error) { return error }

            actual = Reflect.getOwnPropertyDescriptor(CoreCache.insert, 'length')
            expected = {
                "value": 2,
                "writable": false,
                "enumerable": false,
                "configurable": true
            }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(CoreCache.insert, 'length')`)
            if (error) { return error }

            actual = Reflect.getOwnPropertyDescriptor(CoreCache.insert, 'name')
            expected = {
                "value": "insert",
                "writable": false,
                "enumerable": false,
                "configurable": true
            }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(CoreCache.insert, 'name')`)
            if (error) { return error }
        }

        // Check the transactionLookup static method has correct descriptors, length and name
        {
            actual = Reflect.getOwnPropertyDescriptor(CoreCache, 'transactionLookup')
            expected = { "writable": true, "enumerable": true, "configurable": true, value: CoreCache.transactionLookup }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(CoreCache, 'transactionLookup')`)
            if (error) { return error }

            error = assert(typeof CoreCache.transactionLookup, 'function', `typeof CoreCache.transactionLookup`)
            if (error) { return error }

            actual = Reflect.getOwnPropertyDescriptor(CoreCache.transactionLookup, 'length')
            expected = {
                "value": 1,
                "writable": false,
                "enumerable": false,
                "configurable": true
            }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(CoreCache.transactionLookup, 'length')`)
            if (error) { return error }

            actual = Reflect.getOwnPropertyDescriptor(CoreCache.transactionLookup, 'name')
            expected = {
                "value": "transactionLookup",
                "writable": false,
                "enumerable": false,
                "configurable": true
            }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(CoreCache.transactionLookup, 'name')`)
            if (error) { return error }
        }

        return pass("ok")
    });

    // CoreCache constructor
    {

        routes.set("/core-cache/constructor/called-as-regular-function", () => {
            error = assertThrows(() => {
                CoreCache()
            }, TypeError)
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/core-cache/constructor/throws", () => {
            error = assertThrows(() => new CoreCache(), TypeError)
            if (error) { return error }
            return pass("ok")
        });
    }

    // CoreCache lookup static method
    // static lookup(key: string): CoreCacheEntry | null;
    {
        routes.set("/core-cache/lookup/called-as-constructor", () => {
            let error = assertThrows(() => {
                new CoreCache.lookup('1')
            }, TypeError)
            if (error) { return error }
            return pass("ok")
        });
        // https://tc39.es/ecma262/#sec-tostring
        routes.set("/core-cache/lookup/key-parameter-calls-7.1.17-ToString", () => {
            let sentinel;
            const test = () => {
                sentinel = Symbol('sentinel');
                const key = {
                    toString() {
                        throw sentinel;
                    }
                }
                CoreCache.lookup(key)
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
                CoreCache.lookup(Symbol())
            }, TypeError, `can't convert symbol to string`)
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/core-cache/lookup/key-parameter-not-supplied", () => {
            let error = assertThrows(() => {
                CoreCache.lookup()
            }, TypeError, `CoreCache.lookup: At least 1 argument required, but only 0 passed`)
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/core-cache/lookup/key-parameter-empty-string", () => {
            let error = assertThrows(() => {
                CoreCache.lookup('')
            }, Error, `CoreCache.lookup: key can not be an empty string`)
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/core-cache/lookup/key-parameter-8135-character-string", () => {
            let error = assertDoesNotThrow(() => {
                const key = 'a'.repeat(8135)
                CoreCache.lookup(key)
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/core-cache/lookup/key-parameter-8136-character-string", () => {
            let error = assertThrows(() => {
                const key = 'a'.repeat(8136)
                CoreCache.lookup(key)
            }, Error, `CoreCache.lookup: key is too long, the maximum allowed length is 8135.`)
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/core-cache/lookup/key-does-not-exist-returns-null", () => {
            let result = CoreCache.lookup(Math.random())
            error = assert(result, null, `CoreCache.lookup(Math.random()) === null`)
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/core-cache/lookup/key-exists", () => {
            let key = 'c'.repeat(8135);
            let writer = CoreCache.insert(key, {
                maxAge: 1000
            });
            writer.append("meow");
            writer.close();
            let result = CoreCache.lookup(key);
            error = assert(result instanceof CacheEntry, true, `CoreCache.lookup('cat') instanceof CacheEntry`)
            if (error) { return error }
            return pass("ok")
        });

        routes.set("/core-cache/lookup/options-parameter-wrong-type", () => {
            let error = assertThrows(() => {
                CoreCache.lookup('cat', '')
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/core-cache/lookup/options-parameter-headers-field-undefined", () => {
            let entry;
            let error = assertDoesNotThrow(() => {
                entry = CoreCache.lookup('cat', { headers: undefined })
            })
            if (error) { return error }
            error = assert(entry instanceof CacheEntry, true, `CoreCache.lookup('cat', {headers:...}) instanceof CacheEntry`)
            return pass("ok")
        });
        routes.set("/core-cache/lookup/options-parameter-headers-field-valid-sequence", () => {
            let entry;
            let error = assertDoesNotThrow(() => {
                entry = CoreCache.lookup('cat', { headers: [["user-agent", "Aki 1.0"], ["Accept-Encoding", "br"]] })
            })
            if (error) { return error }
            error = assert(entry instanceof CacheEntry, true, `CoreCache.lookup('cat', {headers:...}) instanceof CacheEntry`)
            return pass("ok")
        });
        routes.set("/core-cache/lookup/options-parameter-headers-field-valid-record", () => {
            let entry;
            let error = assertDoesNotThrow(() => {
                entry = CoreCache.lookup('cat', {
                    headers: {
                        "user-agent": "Aki 1.0",
                        "Accept-Encoding": "br"
                    }
                })
            })
            if (error) { return error }
            error = assert(entry instanceof CacheEntry, true, `CoreCache.lookup('cat', {headers:...}) instanceof CacheEntry`)
            return pass("ok")
        });
        routes.set("/core-cache/lookup/options-parameter-headers-field-valid-Headers-instance", () => {
            let entry;
            let error = assertDoesNotThrow(() => {
                entry = CoreCache.lookup('cat', {
                    headers: new Headers({
                        "user-agent": "Aki 1.0",
                        "Accept-Encoding": "br"
                    })
                })
            })
            if (error) { return error }
            error = assert(entry instanceof CacheEntry, true, `CoreCache.lookup('cat', {headers:...}) instanceof CacheEntry`)
            return pass("ok")
        });
    }

    // static insert(key: string, options: InsertOptions): FastlyBody;
    {
        routes.set("/core-cache/insert/called-as-constructor", () => {
            let error = assertThrows(() => {
                new CoreCache.insert('1', { maxAge: 1 })
            }, TypeError)
            if (error) { return error }
            return pass("ok")
        });
        // https://tc39.es/ecma262/#sec-tostring
        routes.set("/core-cache/insert/key-parameter-calls-7.1.17-ToString", () => {
            let sentinel;
            const test = () => {
                sentinel = Symbol('sentinel');
                const key = {
                    toString() {
                        throw sentinel;
                    }
                }
                CoreCache.insert(key, { maxAge: 1 })
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
                CoreCache.insert(Symbol(), { maxAge: 1 })
            }, TypeError, `can't convert symbol to string`)
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/core-cache/insert/key-parameter-not-supplied", () => {
            let error = assertThrows(() => {
                CoreCache.insert()
            }, TypeError, `CoreCache.insert: At least 2 arguments required, but only 0 passed`)
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/core-cache/insert/key-parameter-empty-string", () => {
            let error = assertThrows(() => {
                CoreCache.insert('', { maxAge: 1 })
            }, Error, `CoreCache.insert: key can not be an empty string`)
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/core-cache/insert/key-parameter-8135-character-string", () => {
            let error = assertDoesNotThrow(() => {
                const key = 'a'.repeat(8135)
                CoreCache.insert(key, { maxAge: 1 })
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/core-cache/insert/key-parameter-8136-character-string", () => {
            let error = assertThrows(() => {
                const key = 'a'.repeat(8136)
                CoreCache.insert(key, { maxAge: 1 })
            }, Error, `CoreCache.insert: key is too long, the maximum allowed length is 8135.`)
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/core-cache/insert/options-parameter-wrong-type", () => {
            let error = assertThrows(() => {
                CoreCache.insert('cat', '')
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/core-cache/insert/options-parameter-headers-field-undefined", () => {
            let body;
            let error = assertDoesNotThrow(() => {
                body = CoreCache.insert('cat', { headers: undefined, maxAge: 1 })
            })
            if (error) { return error }
            error = assert(body instanceof FastlyBody, true, `CoreCache.insert('cat', {headers:undefined}) instanceof FastlyBody`)
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/core-cache/insert/options-parameter-headers-field-valid-sequence", () => {
            let body;
            let error = assertDoesNotThrow(() => {
                body = CoreCache.insert('cat', { headers: [["user-agent", "Aki 1.0"], ["Accept-Encoding", "br"]], maxAge: 1 })
            })
            if (error) { return error }
            error = assert(body instanceof FastlyBody, true, `CoreCache.insert('cat', {headers:undefined}) instanceof FastlyBody`)
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/core-cache/insert/options-parameter-headers-field-valid-record", () => {
            let body;
            let error = assertDoesNotThrow(() => {
                body = CoreCache.insert('cat', {
                    headers: {
                        "user-agent": "Aki 1.0",
                        "Accept-Encoding": "br"
                    }, maxAge: 1
                })
            })
            if (error) { return error }
            error = assert(body instanceof FastlyBody, true, `CoreCache.insert('cat', {headers:undefined}) instanceof FastlyBody`)
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/core-cache/insert/options-parameter-headers-field-valid-Headers-instance", () => {
            let body;
            let error = assertDoesNotThrow(() => {
                body = CoreCache.insert('cat', {
                    headers: new Headers({
                        "user-agent": "Aki 1.0",
                        "Accept-Encoding": "br"
                    }), maxAge: 1
                })
            })
            if (error) { return error }
            error = assert(body instanceof FastlyBody, true, `CoreCache.insert('cat', {headers:undefined}) instanceof FastlyBody`)
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/core-cache/insert/options-parameter-maxAge-field-valid-record", () => {
            let body;
            let error = assertDoesNotThrow(() => {
                body = CoreCache.insert('cat', {maxAge: 1})
            })
            if (error) { return error }
            error = assert(body instanceof FastlyBody, true, `CoreCache.insert('cat', {maxAge: 1}) instanceof FastlyBody`)
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/core-cache/insert/options-parameter-maxAge-field-NaN", () => {
            let error = assertThrows(() => {
                CoreCache.insert('cat', {
                    maxAge: NaN
                })
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/core-cache/insert/options-parameter-maxAge-field-postitive-infinity", () => {
            let error = assertThrows(() => {
                CoreCache.insert('cat', {
                    maxAge: Number.POSITIVE_INFINITY
                })
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/core-cache/insert/options-parameter-maxAge-field-negative-infinity", () => {
            let error = assertThrows(() => {
                CoreCache.insert('cat', {
                    maxAge: Number.NEGATIVE_INFINITY
                })
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/core-cache/insert/options-parameter-maxAge-field-negative-number", () => {
            let error = assertThrows(() => {
                CoreCache.insert('cat', {
                    maxAge: -1
                })
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/core-cache/insert/options-parameter-initialAge-field-valid-record", () => {
            let body;
            let error = assertDoesNotThrow(() => {
                body = CoreCache.insert('cat', {maxAge: 1,initialAge: 1})
            })
            if (error) { return error }
            error = assert(body instanceof FastlyBody, true, `CoreCache.insert('cat', {maxAge: 1,initialAge: 1}) instanceof FastlyBody`)
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/core-cache/insert/options-parameter-initialAge-field-NaN", () => {
            let error = assertThrows(() => {
                CoreCache.insert('cat', {
                    maxAge: 1,
                    initialAge: NaN
                })
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/core-cache/insert/options-parameter-initialAge-field-postitive-infinity", () => {
            let error = assertThrows(() => {
                CoreCache.insert('cat', {
                    maxAge: 1,
                    initialAge: Number.POSITIVE_INFINITY
                })
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/core-cache/insert/options-parameter-initialAge-field-negative-infinity", () => {
            let error = assertThrows(() => {
                CoreCache.insert('cat', {
                    maxAge: 1,
                    initialAge: Number.NEGATIVE_INFINITY
                })
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/core-cache/insert/options-parameter-initialAge-field-negative-number", () => {
            let error = assertThrows(() => {
                CoreCache.insert('cat', {
                    maxAge: 1,
                    initialAge: -1
                })
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/core-cache/insert/options-parameter-staleWhileRevalidate-field-valid-record", () => {
            let body;
            let error = assertDoesNotThrow(() => {
                body = CoreCache.insert('cat', {maxAge: 1,staleWhileRevalidate: 1})
            })
            if (error) { return error }
            error = assert(body instanceof FastlyBody, true, `CoreCache.insert('cat', {maxAge: 1,staleWhileRevalidate: 1}) instanceof FastlyBody`)
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/core-cache/insert/options-parameter-staleWhileRevalidate-field-NaN", () => {
            let error = assertThrows(() => {
                CoreCache.insert('cat', {
                    maxAge: 1,
                    staleWhileRevalidate: NaN
                })
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/core-cache/insert/options-parameter-staleWhileRevalidate-field-postitive-infinity", () => {
            let error = assertThrows(() => {
                CoreCache.insert('cat', {
                    maxAge: 1,
                    staleWhileRevalidate: Number.POSITIVE_INFINITY
                })
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/core-cache/insert/options-parameter-staleWhileRevalidate-field-negative-infinity", () => {
            let error = assertThrows(() => {
                CoreCache.insert('cat', {
                    maxAge: 1,
                    staleWhileRevalidate: Number.NEGATIVE_INFINITY
                })
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/core-cache/insert/options-parameter-staleWhileRevalidate-field-negative-number", () => {
            let error = assertThrows(() => {
                CoreCache.insert('cat', {
                    maxAge: 1,
                    staleWhileRevalidate: -1
                })
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/core-cache/insert/options-parameter-length-field-valid-record", () => {
            let body;
            let error = assertDoesNotThrow(() => {
                body = CoreCache.insert('cat', {maxAge: 1,length: 1})
            })
            if (error) { return error }
            error = assert(body instanceof FastlyBody, true, `CoreCache.insert('cat', {maxAge: 1,length: 1}) instanceof FastlyBody`)
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/core-cache/insert/options-parameter-length-field-NaN", () => {
            let error = assertThrows(() => {
                CoreCache.insert('cat', {
                    maxAge: 1,
                    length: NaN
                })
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/core-cache/insert/options-parameter-length-field-postitive-infinity", () => {
            let error = assertThrows(() => {
                CoreCache.insert('cat', {
                    maxAge: 1,
                    length: Number.POSITIVE_INFINITY
                })
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/core-cache/insert/options-parameter-length-field-negative-infinity", () => {
            let error = assertThrows(() => {
                CoreCache.insert('cat', {
                    maxAge: 1,
                    length: Number.NEGATIVE_INFINITY
                })
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/core-cache/insert/options-parameter-length-field-negative-number", () => {
            let error = assertThrows(() => {
                CoreCache.insert('cat', {
                    maxAge: 1,
                    length: -1
                })
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/core-cache/insert/options-parameter-sensitive-field", () => {
            let error = assertDoesNotThrow(() => {
                CoreCache.insert('cat', {
                    maxAge: 1,
                    sensitive: true
                })
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/core-cache/insert/options-parameter-vary-field", () => {
            let error = assertDoesNotThrow(() => {
                CoreCache.insert('cat', {
                    maxAge: 1,
                    vary: ["animal", "mineral", "vegetable"]
                })
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/core-cache/insert/options-parameter-userMetadata-field/arraybuffer/empty", async () => {
            let error = await assertResolves(async () => {
                let key = '/core-cache/insert/options-parameter-userMetadata-field/arraybuffer/empty' + Math.random()
                let writer = CoreCache.insert(key, {
                    maxAge: 60 * 1000,
                    userMetadata: new ArrayBuffer(0)
                });
                writer.append("hello");
                writer.close();
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/core-cache/insert/options-parameter-userMetadata-field/arraybuffer/not-empty", async () => {
            let error = await assertResolves(async () => {
                let key = '/core-cache/insert/options-parameter-userMetadata-field/arraybuffer/not-empty' + Math.random()
                let writer = CoreCache.insert(key, {
                    maxAge: 60 * 1000,
                    userMetadata: Uint8Array.from([104, 101, 108, 108, 111]).buffer
                });
                writer.append("hello");
                writer.close();
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/core-cache/insert/options-parameter-userMetadata-field/URLSearchParams", async () => {
            let error = await assertResolves(async () => {
                let key = '/core-cache/insert/options-parameter-userMetadata-field/URLSearchParams' + Math.random()
                let userMetadata = new URLSearchParams()
                userMetadata.set('hello', 'world')
                let writer = CoreCache.insert(key, {
                    maxAge: 60 * 1000,
                    userMetadata
                });
                writer.append("hello");
                writer.close();
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/core-cache/insert/options-parameter-userMetadata-field/string", async () => {
            let error = await assertResolves(async () => {
                let key = '/core-cache/insert/options-parameter-userMetadata-field/string' + Math.random()
                let writer = CoreCache.insert(key, {
                    maxAge: 60 * 1000,
                    userMetadata: 'hello'
                });
                writer.append("hello");
                writer.close();
            })
            if (error) { return error }
            return pass("ok")
        });
        // surrogateKeys?: Array<string>,-- empty string? -- toString which throws -- wrong types?
    }

    //static transactionLookup(key: string, options?: LookupOptions): CacheEntry | null;
    {
        routes.set("/core-cache/transactionLookup/called-as-constructor", () => {
            let error = assertThrows(() => {
                new CoreCache.transactionLookup('1')
            }, TypeError)
            if (error) { return error }
            return pass("ok")
        });
        // https://tc39.es/ecma262/#sec-tostring
        routes.set("/core-cache/transactionLookup/key-parameter-calls-7.1.17-ToString", () => {
            let sentinel;
            const test = () => {
                sentinel = Symbol('sentinel');
                const key = {
                    toString() {
                        throw sentinel;
                    }
                }
                CoreCache.transactionLookup(key)
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
                CoreCache.transactionLookup(Symbol())
            }, TypeError, `can't convert symbol to string`)
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/core-cache/transactionLookup/key-parameter-not-supplied", () => {
            let error = assertThrows(() => {
                CoreCache.transactionLookup()
            }, TypeError, `CoreCache.transactionLookup: At least 1 argument required, but only 0 passed`)
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/core-cache/transactionLookup/key-parameter-empty-string", () => {
            let error = assertThrows(() => {
                CoreCache.transactionLookup('')
            }, Error, `CoreCache.transactionLookup: key can not be an empty string`)
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/core-cache/transactionLookup/key-parameter-8135-character-string", () => {
            let error = assertDoesNotThrow(() => {
                const key = 'a'.repeat(8135)
                CoreCache.transactionLookup(key)
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/core-cache/transactionLookup/key-parameter-8136-character-string", () => {
            let error = assertThrows(() => {
                const key = 'a'.repeat(8136)
                CoreCache.transactionLookup(key)
            }, Error, `CoreCache.transactionLookup: key is too long, the maximum allowed length is 8135.`)
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/core-cache/transactionLookup/key-does-not-exist", () => {
            let result = CoreCache.transactionLookup(Math.random())
            error = assert(result instanceof CacheEntry, true, `CoreCache.transactionLookup(Math.random()) instanceof CacheEntry`)
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/core-cache/transactionLookup/key-exists", () => {
            let writer = CoreCache.insert('cat', {
                maxAge: 10_000
            });
            writer.append("meow");
            writer.close();
            let result = CoreCache.transactionLookup('cat');
            error = assert(result instanceof CacheEntry, true, `CoreCache.transactionLookup('cat') instanceof CacheEntry`)
            if (error) { return error }
            return pass("ok")
        });

        routes.set("/core-cache/transactionLookup/options-parameter-wrong-type", () => {
            let error = assertThrows(() => {
                CoreCache.transactionLookup('cat', '')
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/core-cache/transactionLookup/options-parameter-headers-field-undefined", () => {
            let entry;
            let error = assertDoesNotThrow(() => {
                entry = CoreCache.transactionLookup('cat', { headers: undefined })
            })
            if (error) { return error }
            error = assert(entry instanceof CacheEntry, true, `CoreCache.transactionLookup('cat', {headers:...}) instanceof CacheEntry`)
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/core-cache/transactionLookup/options-parameter-headers-field-valid-sequence", () => {
            let entry;
            let error = assertDoesNotThrow(() => {
                entry = CoreCache.transactionLookup('cat', { headers: [["user-agent", "Aki 1.0"], ["Accept-Encoding", "br"]] })
            })
            if (error) { return error }
            error = assert(entry instanceof CacheEntry, true, `CoreCache.transactionLookup('cat', {headers:...}) instanceof CacheEntry`)
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/core-cache/transactionLookup/options-parameter-headers-field-valid-record", () => {
            let entry;
            let error = assertDoesNotThrow(() => {
                entry = CoreCache.transactionLookup('cat', {
                    headers: {
                        "user-agent": "Aki 1.0",
                        "Accept-Encoding": "br"
                    }
                })
            })
            if (error) { return error }
            error = assert(entry instanceof CacheEntry, true, `CoreCache.transactionLookup('cat', {headers:...}) instanceof CacheEntry`)
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/core-cache/transactionLookup/options-parameter-headers-field-valid-Headers-instance", () => {
            let entry;
            let error = assertDoesNotThrow(() => {
                entry = CoreCache.transactionLookup('cat', {
                    headers: new Headers({
                        "user-agent": "Aki 1.0",
                        "Accept-Encoding": "br"
                    })
                })
            })
            if (error) { return error }
            error = assert(entry instanceof CacheEntry, true, `CoreCache.transactionLookup('cat', {headers:...}) instanceof CacheEntry`)
            if (error) { return error }
            return pass("ok")
        });
    }
}

// CacheEntry
{
    routes.set("/cache-entry/interface", () => {
        let actual = Reflect.ownKeys(CacheEntry)
        let expected = ["prototype", "length", "name"]
        error = assert(actual, expected, `Reflect.ownKeys(CacheEntry)`)
        if (error) { return error }

        // Check the prototype descriptors are correct
        {
            actual = Reflect.getOwnPropertyDescriptor(CacheEntry, 'prototype')
            expected = {
                "value": CacheEntry.prototype,
                "writable": false,
                "enumerable": false,
                "configurable": false
            }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(CacheEntry, 'prototype')`)
            if (error) { return error }
        }

        // Check the constructor function's defined parameter length is correct
        {
            actual = Reflect.getOwnPropertyDescriptor(CacheEntry, 'length')
            expected = {
                "value": 0,
                "writable": false,
                "enumerable": false,
                "configurable": true
            }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(CacheEntry, 'length')`)
            if (error) { return error }
        }

        // Check the constructor function's name is correct
        {
            actual = Reflect.getOwnPropertyDescriptor(CacheEntry, 'name')
            expected = {
                "value": "CacheEntry",
                "writable": false,
                "enumerable": false,
                "configurable": true
            }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(CacheEntry, 'name')`)
            if (error) { return error }
        }

        // Check the prototype has the correct keys
        {
            actual = Reflect.ownKeys(CacheEntry.prototype)
            expected = ["constructor", "close", "state", "userMetadata", "body", "length", "maxAge", "staleWhileRevalidate", "age", "hits", Symbol.toStringTag]
            error = assert(actual, expected, `Reflect.ownKeys(CacheEntry.prototype)`)
            if (error) { return error }
        }

        // Check the constructor on the prototype is correct
        {
            actual = Reflect.getOwnPropertyDescriptor(CacheEntry.prototype, 'constructor')
            expected = { "writable": true, "enumerable": false, "configurable": true, value: CacheEntry.prototype.constructor }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(CacheEntry.prototype, 'constructor')`)
            if (error) { return error }

            error = assert(typeof CacheEntry.prototype.constructor, 'function', `typeof CacheEntry.prototype.constructor`)
            if (error) { return error }

            actual = Reflect.getOwnPropertyDescriptor(CacheEntry.prototype.constructor, 'length')
            expected = {
                "value": 0,
                "writable": false,
                "enumerable": false,
                "configurable": true
            }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(CacheEntry.prototype.constructor, 'length')`)
            if (error) { return error }

            actual = Reflect.getOwnPropertyDescriptor(CacheEntry.prototype.constructor, 'name')
            expected = {
                "value": "CacheEntry",
                "writable": false,
                "enumerable": false,
                "configurable": true
            }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(CacheEntry.prototype.constructor, 'name')`)
            if (error) { return error }
        }

        // Check the Symbol.toStringTag on the prototype is correct
        {
            actual = Reflect.getOwnPropertyDescriptor(CacheEntry.prototype, Symbol.toStringTag)
            expected = { "writable": false, "enumerable": false, "configurable": true, value: "CacheEntry" }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(CacheEntry.prototype, [Symbol.toStringTag])`)
            if (error) { return error }

            error = assert(typeof CacheEntry.prototype[Symbol.toStringTag], 'string', `typeof CacheEntry.prototype[Symbol.toStringTag]`)
            if (error) { return error }
        }

        // Check the close method has correct descriptors, length and name
        {
            actual = Reflect.getOwnPropertyDescriptor(CacheEntry.prototype, 'close')
            expected = { "writable": true, "enumerable": true, "configurable": true, value: CacheEntry.prototype.close }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(CacheEntry.prototype, 'close')`)
            if (error) { return error }

            error = assert(typeof CacheEntry.prototype.close, 'function', `typeof CacheEntry.prototype.close`)
            if (error) { return error }

            actual = Reflect.getOwnPropertyDescriptor(CacheEntry.prototype.close, 'length')
            expected = {
                "value": 0,
                "writable": false,
                "enumerable": false,
                "configurable": true
            }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(CacheEntry.prototype.close, 'length')`)
            if (error) { return error }

            actual = Reflect.getOwnPropertyDescriptor(CacheEntry.prototype.close, 'name')
            expected = {
                "value": "close",
                "writable": false,
                "enumerable": false,
                "configurable": true
            }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(CacheEntry.prototype.close, 'name')`)
            if (error) { return error }
        }

        // Check the state method has correct descriptors, length and name
        {
            actual = Reflect.getOwnPropertyDescriptor(CacheEntry.prototype, 'state')
            expected = { "writable": true, "enumerable": true, "configurable": true, value: CacheEntry.prototype.state }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(CacheEntry.prototype, 'state')`)
            if (error) { return error }

            error = assert(typeof CacheEntry.prototype.state, 'function', `typeof CacheEntry.prototype.state`)
            if (error) { return error }

            actual = Reflect.getOwnPropertyDescriptor(CacheEntry.prototype.state, 'length')
            expected = {
                "value": 0,
                "writable": false,
                "enumerable": false,
                "configurable": true
            }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(CacheEntry.prototype.state, 'length')`)
            if (error) { return error }

            actual = Reflect.getOwnPropertyDescriptor(CacheEntry.prototype.state, 'name')
            expected = {
                "value": "state",
                "writable": false,
                "enumerable": false,
                "configurable": true
            }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(CacheEntry.prototype.state, 'name')`)
            if (error) { return error }
        }

        // Check the userMetadata method has correct descriptors, length and name
        {
            actual = Reflect.getOwnPropertyDescriptor(CacheEntry.prototype, 'userMetadata')
            expected = { "writable": true, "enumerable": true, "configurable": true, value: CacheEntry.prototype.userMetadata }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(CacheEntry.prototype, 'userMetadata')`)
            if (error) { return error }

            error = assert(typeof CacheEntry.prototype.userMetadata, 'function', `typeof CacheEntry.prototype.userMetadata`)
            if (error) { return error }

            actual = Reflect.getOwnPropertyDescriptor(CacheEntry.prototype.userMetadata, 'length')
            expected = {
                "value": 0,
                "writable": false,
                "enumerable": false,
                "configurable": true
            }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(CacheEntry.prototype.userMetadata, 'length')`)
            if (error) { return error }

            actual = Reflect.getOwnPropertyDescriptor(CacheEntry.prototype.userMetadata, 'name')
            expected = {
                "value": "userMetadata",
                "writable": false,
                "enumerable": false,
                "configurable": true
            }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(CacheEntry.prototype.userMetadata, 'name')`)
            if (error) { return error }
        }

        // Check the body method has correct descriptors, length and name
        {
            actual = Reflect.getOwnPropertyDescriptor(CacheEntry.prototype, 'body')
            expected = { "writable": true, "enumerable": true, "configurable": true, value: CacheEntry.prototype.body }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(CacheEntry.prototype, 'body')`)
            if (error) { return error }

            error = assert(typeof CacheEntry.prototype.body, 'function', `typeof CacheEntry.prototype.body`)
            if (error) { return error }

            actual = Reflect.getOwnPropertyDescriptor(CacheEntry.prototype.body, 'length')
            expected = {
                "value": 0,
                "writable": false,
                "enumerable": false,
                "configurable": true
            }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(CacheEntry.prototype.body, 'length')`)
            if (error) { return error }

            actual = Reflect.getOwnPropertyDescriptor(CacheEntry.prototype.body, 'name')
            expected = {
                "value": "body",
                "writable": false,
                "enumerable": false,
                "configurable": true
            }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(CacheEntry.prototype.body, 'name')`)
            if (error) { return error }
        }

        // Check the length method has correct descriptors, length and name
        {
            actual = Reflect.getOwnPropertyDescriptor(CacheEntry.prototype, 'length')
            expected = { "writable": true, "enumerable": true, "configurable": true, value: CacheEntry.prototype.length }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(CacheEntry.prototype, 'length')`)
            if (error) { return error }

            error = assert(typeof CacheEntry.prototype.length, 'function', `typeof CacheEntry.prototype.length`)
            if (error) { return error }

            actual = Reflect.getOwnPropertyDescriptor(CacheEntry.prototype.length, 'length')
            expected = {
                "value": 0,
                "writable": false,
                "enumerable": false,
                "configurable": true
            }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(CacheEntry.prototype.length, 'length')`)
            if (error) { return error }

            actual = Reflect.getOwnPropertyDescriptor(CacheEntry.prototype.length, 'name')
            expected = {
                "value": "length",
                "writable": false,
                "enumerable": false,
                "configurable": true
            }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(CacheEntry.prototype.length, 'name')`)
            if (error) { return error }
        }

        // Check the maxAge method has correct descriptors, length and name
        {
            actual = Reflect.getOwnPropertyDescriptor(CacheEntry.prototype, 'maxAge')
            expected = { "writable": true, "enumerable": true, "configurable": true, value: CacheEntry.prototype.maxAge }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(CacheEntry.prototype, 'maxAge')`)
            if (error) { return error }

            error = assert(typeof CacheEntry.prototype.maxAge, 'function', `typeof CacheEntry.prototype.maxAge`)
            if (error) { return error }

            actual = Reflect.getOwnPropertyDescriptor(CacheEntry.prototype.maxAge, 'length')
            expected = {
                "value": 0,
                "writable": false,
                "enumerable": false,
                "configurable": true
            }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(CacheEntry.prototype.maxAge, 'length')`)
            if (error) { return error }

            actual = Reflect.getOwnPropertyDescriptor(CacheEntry.prototype.maxAge, 'name')
            expected = {
                "value": "maxAge",
                "writable": false,
                "enumerable": false,
                "configurable": true
            }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(CacheEntry.prototype.maxAge, 'name')`)
            if (error) { return error }
        }

        // Check the staleWhileRevalidate method has correct descriptors, length and name
        {
            actual = Reflect.getOwnPropertyDescriptor(CacheEntry.prototype, 'staleWhileRevalidate')
            expected = { "writable": true, "enumerable": true, "configurable": true, value: CacheEntry.prototype.staleWhileRevalidate }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(CacheEntry.prototype, 'staleWhileRevalidate')`)
            if (error) { return error }

            error = assert(typeof CacheEntry.prototype.staleWhileRevalidate, 'function', `typeof CacheEntry.prototype.staleWhileRevalidate`)
            if (error) { return error }

            actual = Reflect.getOwnPropertyDescriptor(CacheEntry.prototype.staleWhileRevalidate, 'length')
            expected = {
                "value": 0,
                "writable": false,
                "enumerable": false,
                "configurable": true
            }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(CacheEntry.prototype.staleWhileRevalidate, 'length')`)
            if (error) { return error }

            actual = Reflect.getOwnPropertyDescriptor(CacheEntry.prototype.staleWhileRevalidate, 'name')
            expected = {
                "value": "staleWhileRevalidate",
                "writable": false,
                "enumerable": false,
                "configurable": true
            }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(CacheEntry.prototype.staleWhileRevalidate, 'name')`)
            if (error) { return error }
        }

        // Check the age method has correct descriptors, length and name
        {
            actual = Reflect.getOwnPropertyDescriptor(CacheEntry.prototype, 'age')
            expected = { "writable": true, "enumerable": true, "configurable": true, value: CacheEntry.prototype.age }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(CacheEntry.prototype, 'age')`)
            if (error) { return error }

            error = assert(typeof CacheEntry.prototype.age, 'function', `typeof CacheEntry.prototype.age`)
            if (error) { return error }

            actual = Reflect.getOwnPropertyDescriptor(CacheEntry.prototype.age, 'length')
            expected = {
                "value": 0,
                "writable": false,
                "enumerable": false,
                "configurable": true
            }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(CacheEntry.prototype.age, 'length')`)
            if (error) { return error }

            actual = Reflect.getOwnPropertyDescriptor(CacheEntry.prototype.age, 'name')
            expected = {
                "value": "age",
                "writable": false,
                "enumerable": false,
                "configurable": true
            }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(CacheEntry.prototype.age, 'name')`)
            if (error) { return error }
        }

        // Check the hits method has correct descriptors, length and name
        {
            actual = Reflect.getOwnPropertyDescriptor(CacheEntry.prototype, 'hits')
            expected = { "writable": true, "enumerable": true, "configurable": true, value: CacheEntry.prototype.hits }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(CacheEntry.prototype, 'hits')`)
            if (error) { return error }

            error = assert(typeof CacheEntry.prototype.hits, 'function', `typeof CacheEntry.prototype.hits`)
            if (error) { return error }

            actual = Reflect.getOwnPropertyDescriptor(CacheEntry.prototype.hits, 'length')
            expected = {
                "value": 0,
                "writable": false,
                "enumerable": false,
                "configurable": true
            }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(CacheEntry.prototype.hits, 'length')`)
            if (error) { return error }

            actual = Reflect.getOwnPropertyDescriptor(CacheEntry.prototype.hits, 'name')
            expected = {
                "value": "hits",
                "writable": false,
                "enumerable": false,
                "configurable": true
            }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(CacheEntry.prototype.hits, 'name')`)
            if (error) { return error }
        }

        return pass("ok")
    });

    // CacheEntry constructor
    {

        routes.set("/cache-entry/constructor/called-as-regular-function", () => {
            error = assertThrows(() => {
                CacheEntry()
            }, TypeError)
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/cache-entry/constructor/throws", () => {
            error = assertThrows(() => new CacheEntry(), TypeError)
            if (error) { return error }
            return pass("ok")
        });
    }

    // close(): void;
    {
        routes.set("/cache-entry/close/called-as-constructor", () => {
            let error = assertThrows(() => {
                new CacheEntry.prototype.close()
            }, TypeError)
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/cache-entry/close/called-unbound", () => {
            let error = assertThrows(() => {
                CacheEntry.prototype.close.call(undefined)
            }, Error)
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/cache-entry/close/called-on-instance", () => {
            let key = '/cache-entry/close/called-on-instance' + Math.random()
            let writer = CoreCache.insert(key, {
                maxAge: 60 * 1000
            });
            writer.append("hello");
            writer.close();
            let result = CoreCache.lookup(key).close()
            error = assert(result, undefined, `CoreCache.lookup(key).close()`)
            if (error) { return error }
            return pass("ok")
        });
    }

    // state(): CacheState;
    {
        routes.set("/cache-entry/state/called-as-constructor", () => {
            let error = assertThrows(() => {
                new CacheEntry.prototype.state()
            }, TypeError)
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/cache-entry/state/called-unbound", () => {
            let error = assertThrows(() => {
                CacheEntry.prototype.state.call(undefined)
            }, Error)
            if (error) { return error }
            return pass("ok")
        });

        routes.set("/cache-entry/state/called-on-instance", () => {
            let key = '/cache-entry/state/called-on-instance' + Math.random()
            let writer = CoreCache.insert(key, {
                maxAge: 60 * 1000
            });
            writer.append("hello");
            writer.close();
            let entry = CoreCache.lookup(key);
            let result = entry.state()
            error = assert(result instanceof CacheState, true, `CoreCache.lookup(key).state() instanceof CacheState`)
            if (error) { return error }
            return pass("ok")
        });
    }

    // userMetadata(): ArrayBuffer;
    {
        routes.set("/cache-entry/userMetadata/called-as-constructor", () => {
            let error = assertThrows(() => {
                new CacheEntry.prototype.userMetadata()
            }, TypeError)
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/cache-entry/userMetadata/called-unbound", () => {
            let error = assertThrows(() => {
                CacheEntry.prototype.userMetadata.call(undefined)
            }, Error)
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/cache-entry/userMetadata/called-on-instance", () => {
            let key = '/cache-entry/userMetadata/called-on-instance' + Math.random()
            let writer = CoreCache.insert(key, {
                maxAge: 60 * 1000
            });
            writer.append("hello");
            writer.close();
            let result = CoreCache.lookup(key).userMetadata()
            error = assert(result instanceof ArrayBuffer, true, `CoreCache.lookup(key).userMetadata() instanceof ArrayBuffer`)
            if (error) { return error }
            error = assert(result.byteLength, 0, `CoreCache.lookup(key).userMetadata().byteLength`)
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/cache-entry/userMetadata/basic", () => {
            let key = '/cache-entry/userMetadata/basic' + Math.random()
            let writer = CoreCache.insert(key, {
                maxAge: 60 * 1000,
                userMetadata: 'hello world'
            });
            writer.append("hello");
            writer.close();
            let entry = CoreCache.lookup(key);
            error = assert(entry instanceof CacheEntry, true, 'CoreCache.lookup(key) instanceof CacheEntry')
            let metadata = entry.userMetadata();
            error = assert(metadata instanceof ArrayBuffer, true, `CoreCache.lookup(key).userMetadata() instanceof ArrayBuffer`)
            if (error) { return error }
            error = assert(metadata.byteLength, 11, `CoreCache.lookup(key).userMetadata().byteLength`)
            if (error) { return error }
            let result = new TextDecoder().decode(metadata)
            error = assert(result, 'hello world', `new TextDecoder().decode(CoreCache.lookup(key).userMetadata()) === 'hello world'`)
            if (error) { return error }
            return pass("ok")
        });

    }

    // body(options?: CacheBodyOptions): ReadableStream;
    {
        routes.set("/cache-entry/body/called-as-constructor", () => {
            let error = assertThrows(() => {
                new CacheEntry.prototype.body()
            }, TypeError)
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/cache-entry/body/called-unbound", () => {
            let error = assertThrows(() => {
                CacheEntry.prototype.body.call(undefined)
            }, Error)
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/cache-entry/body/called-on-instance", async () => {
            let key = '/cache-entry/body/called-on-instance' + Math.random()
            let writer = CoreCache.insert(key, {
                maxAge: 60 * 1000
            });
            writer.append("hello");
            writer.close();
            let result = CoreCache.lookup(key).body()
            error = assert(result instanceof ReadableStream, true, `CoreCache.lookup(key).body() instanceof ReadableStream`)
            if (error) { return error }

            result = await streamToString(result);
            error = assert(result, 'hello', `await streamToString(CoreCache.lookup(key).body())`)
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/cache-entry/body/options-start-negative", async () => {
            let key = '/cache-entry/body/options-start-negative' + Math.random()
            let writer = CoreCache.insert(key, {
                maxAge: 60 * 1000
            });
            writer.append("hello");
            writer.close();
            error = assertThrows(() => {
                CoreCache.lookup(key).body({start: -1})
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/cache-entry/body/options-start-NaN", async () => {
            let key = '/cache-entry/body/options-start-NaN' + Math.random()
            let writer = CoreCache.insert(key, {
                maxAge: 60 * 1000
            });
            writer.append("hello");
            writer.close();
            error = assertThrows(() => {
                CoreCache.lookup(key).body({start: NaN})
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/cache-entry/body/options-start-Infinity", async () => {
            let key = '/cache-entry/body/options-start-Infinity' + Math.random()
            let writer = CoreCache.insert(key, {
                maxAge: 60 * 1000
            });
            writer.append("hello");
            writer.close();
            error = assertThrows(() => {
                CoreCache.lookup(key).body({start: Infinity})
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/cache-entry/body/options-start-valid", async () => {
            let key = '/cache-entry/body/options-start-valid' + Math.random()
            let writer = CoreCache.insert(key, {
                maxAge: 60 * 1000
            });
            writer.append("hello");
            writer.close();
            let result = CoreCache.lookup(key).body({start: 1})
            error = assert(result instanceof ReadableStream, true, `CoreCache.lookup(key).body() instanceof ReadableStream`)
            if (error) { return error }

            result = await streamToString(result);
            error = assert(result, 'ello', `await streamToString(CoreCache.lookup(key).body())`)
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/cache-entry/body/options-start-longer-than-body", async () => {
            let key = '/cache-entry/body/options-start-longer-than-body' + Math.random()
            let writer = CoreCache.insert(key, {
                maxAge: 60 * 1000
            });
            writer.append("hello");
            writer.close();
            let result = CoreCache.lookup(key).body({start: 1000})
            error = assert(result instanceof ReadableStream, true, `CoreCache.lookup(key).body() instanceof ReadableStream`)
            if (error) { return error }

            result = await streamToString(result);
            error = assert(result, 'hello', `await streamToString(CoreCache.lookup(key).body())`)
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/cache-entry/body/options-end-negative", async () => {
            let key = '/cache-entry/body/options-end-negative' + Math.random()
            let writer = CoreCache.insert(key, {
                maxAge: 60 * 1000
            });
            writer.append("hello");
            writer.close();
            error = assertThrows(() => {
                CoreCache.lookup(key).body({end: -1})
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/cache-entry/body/options-end-NaN", async () => {
            let key = '/cache-entry/body/options-end-NaN' + Math.random()
            let writer = CoreCache.insert(key, {
                maxAge: 60 * 1000
            });
            writer.append("hello");
            writer.close();
            error = assertThrows(() => {
                CoreCache.lookup(key).body({end: NaN})
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/cache-entry/body/options-end-Infinity", async () => {
            let key = '/cache-entry/body/options-end-Infinity' + Math.random()
            let writer = CoreCache.insert(key, {
                maxAge: 60 * 1000
            });
            writer.append("hello");
            writer.close();
            error = assertThrows(() => {
                CoreCache.lookup(key).body({end: Infinity})
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/cache-entry/body/options-end-valid", async () => {
            let key = '/cache-entry/body/options-end-valid' + Math.random()
            let writer = CoreCache.insert(key, {
                maxAge: 60 * 1000
            });
            writer.append("hello");
            writer.close();
            let result = CoreCache.lookup(key).body({start: 1, end: 1})
            error = assert(result instanceof ReadableStream, true, `CoreCache.lookup(key).body() instanceof ReadableStream`)
            if (error) { return error }

            result = await streamToString(result);
            console.log({result})
            error = assert(result, 'e', `await streamToString(CoreCache.lookup(key).body())`)
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/cache-entry/body/options-end-zero", async () => {
            let key = '/cache-entry/body/options-end-zero' + Math.random()
            let writer = CoreCache.insert(key, {
                maxAge: 60 * 1000
            });
            writer.append("hello");
            writer.close();
            let result = CoreCache.lookup(key).body({start: 1, end: 0})
            error = assert(result instanceof ReadableStream, true, `CoreCache.lookup(key).body() instanceof ReadableStream`)
            if (error) { return error }

            result = await streamToString(result);
            console.log({result})
            error = assert(result, 'hello', `await streamToString(CoreCache.lookup(key).body())`)
            if (error) { return error }
            return pass("ok")
        });
    }

    // length(): number;
    {
        routes.set("/cache-entry/length/called-as-constructor", () => {
            let error = assertThrows(() => {
                new CacheEntry.prototype.length()
            }, TypeError)
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/cache-entry/length/called-unbound", () => {
            let error = assertThrows(() => {
                CacheEntry.prototype.length.call(undefined)
            }, Error)
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/cache-entry/length/called-on-instance", () => {
            let key = '/cache-entry/length/called-on-instance' + Math.random()
            let writer = CoreCache.insert(key, {
                maxAge: 60 * 1000
            });
            writer.append("hello");
            writer.close();
            let result = CoreCache.lookup(key).length()
            error = assert(result, 5, `CoreCache.lookup(key).length()`)
            if (error) { return error }
            return pass("ok")
        });
        // TODO pass in an entry with unknown length and then call length and check it is null?
        /// The size in bytes of the cached item, if known.
        ///
        /// The length of the cached item may be unknown if the item is currently being streamed into
        /// the cache without a fixed length.
    }

    // maxAge(): number;
    {
        routes.set("/cache-entry/maxAge/called-as-constructor", () => {
            let error = assertThrows(() => {
                new CacheEntry.prototype.maxAge()
            }, TypeError)
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/cache-entry/maxAge/called-unbound", () => {
            let error = assertThrows(() => {
                CacheEntry.prototype.maxAge.call(undefined)
            }, Error)
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/cache-entry/maxAge/called-on-instance", async () => {
            let key = '/cache-entry/maxAge/called-on-instance' + Math.random()
            let writer = CoreCache.insert(key, {
                maxAge: 60 * 1000
            });
            writer.append("hello");
            writer.close();
            let result = CoreCache.lookup(key).maxAge()
            error = assert(result, 60_000, `CoreCache.lookup(key).maxAge()`)
            if (error) { return error }
            return pass("ok")
        });
    }

    // staleWhileRevalidate(): number;
    {
        routes.set("/cache-entry/staleWhileRevalidate/called-as-constructor", () => {
            let error = assertThrows(() => {
                new CacheEntry.prototype.staleWhileRevalidate()
            }, TypeError)
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/cache-entry/staleWhileRevalidate/called-unbound", () => {
            let error = assertThrows(() => {
                CacheEntry.prototype.staleWhileRevalidate.call(undefined)
            }, Error)
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/cache-entry/staleWhileRevalidate/called-on-instance", async () => {
            let key = '/cache-entry/staleWhileRevalidate/called-on-instance' + Math.random()
            let writer = CoreCache.insert(key, {
                maxAge: 60 * 1000
            });
            writer.append("hello");
            writer.close();
            let result = CoreCache.lookup(key).staleWhileRevalidate()
            error = assert(typeof result, "number", `typeof CoreCache.lookup(key).staleWhileRevalidate()`)
            if (error) { return error }
            error = assert(result >= 0, true, `CoreCache.lookup(key).staleWhileRevalidate() >= 0`)
            if (error) { return error }
            return pass("ok")
        });
    }

    // age(): number;
    {
        routes.set("/cache-entry/age/called-as-constructor", () => {
            let error = assertThrows(() => {
                new CacheEntry.prototype.age()
            }, TypeError)
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/cache-entry/age/called-unbound", () => {
            let error = assertThrows(() => {
                CacheEntry.prototype.age.call(undefined)
            }, Error)
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/cache-entry/age/called-on-instance", async () => {
            let key = '/cache-entry/age/called-on-instance' + Math.random()
            let writer = CoreCache.insert(key, {
                maxAge: 60 * 1000
            });
            writer.append("hello");
            writer.close();
            let result = CoreCache.lookup(key).age()
            error = assert(typeof result, "number", `typeof CoreCache.lookup(key).age()`)
            if (error) { return error }
            error = assert(result >= 0, true, `CoreCache.lookup(key).age() >= 0`)
            if (error) { return error }
            await sleep(1000);
            result = CoreCache.lookup(key).age()
            error = assert(result >= 1_000, true, `CoreCache.lookup(key).age() >= 1_000 (${result})`)
            if (error) { return error }
            return pass("ok")
        });
    }

    // hits(): number;
    {
        routes.set("/cache-entry/hits/called-as-constructor", () => {
            let error = assertThrows(() => {
                new CacheEntry.prototype.hits()
            }, TypeError)
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/cache-entry/hits/called-unbound", () => {
            let error = assertThrows(() => {
                CacheEntry.prototype.hits.call(undefined)
            }, Error)
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/cache-entry/hits/called-on-instance", () => {
            let key = '/cache-entry/hits/called-on-instance' + Math.random()
            let writer = CoreCache.insert(key, {
                maxAge: 60 * 1000
            });
            writer.append("hello");
            writer.close();
            let result = CoreCache.lookup(key).hits()
            error = assert(result, 1, `CoreCache.lookup(key).hits()`)
            if (error) { return error }
            result = CoreCache.lookup(key).hits()
            error = assert(result, 2, `CoreCache.lookup(key).hits()`)
            if (error) { return error }
            return pass("ok")
        });
    }
}

// TransactionCacheEntry
{
    routes.set("/transaction-cache-entry/interface", () => {
        let actual = Reflect.ownKeys(TransactionCacheEntry)
        let expected = ["prototype", "length", "name"]
        error = assert(actual, expected, `Reflect.ownKeys(TransactionCacheEntry)`)
        if (error) { return error }

        // Check the prototype descriptors are correct
        {
            actual = Reflect.getOwnPropertyDescriptor(TransactionCacheEntry, 'prototype')
            expected = {
                "value": TransactionCacheEntry.prototype,
                "writable": false,
                "enumerable": false,
                "configurable": false
            }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(TransactionCacheEntry, 'prototype')`)
            if (error) { return error }
        }

        // Check the constructor function's defined parameter length is correct
        {
            actual = Reflect.getOwnPropertyDescriptor(TransactionCacheEntry, 'length')
            expected = {
                "value": 0,
                "writable": false,
                "enumerable": false,
                "configurable": true
            }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(TransactionCacheEntry, 'length')`)
            if (error) { return error }
        }

        // Check the constructor function's name is correct
        {
            actual = Reflect.getOwnPropertyDescriptor(TransactionCacheEntry, 'name')
            expected = {
                "value": "TransactionCacheEntry",
                "writable": false,
                "enumerable": false,
                "configurable": true
            }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(TransactionCacheEntry, 'name')`)
            if (error) { return error }
        }

        // Check the prototype has the correct keys
        {
            actual = Reflect.ownKeys(TransactionCacheEntry.prototype)
            expected = ["constructor","insert","insertAndStreamBack","update","cancel",Symbol.toStringTag]
            error = assert(actual, expected, `Reflect.ownKeys(TransactionCacheEntry.prototype)`)
            if (error) { return error }
        }

        // Check the constructor on the prototype is correct
        {
            actual = Reflect.getOwnPropertyDescriptor(TransactionCacheEntry.prototype, 'constructor')
            expected = { "writable": true, "enumerable": false, "configurable": true, value: TransactionCacheEntry.prototype.constructor }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(TransactionCacheEntry.prototype, 'constructor')`)
            if (error) { return error }

            error = assert(typeof TransactionCacheEntry.prototype.constructor, 'function', `typeof TransactionCacheEntry.prototype.constructor`)
            if (error) { return error }

            actual = Reflect.getOwnPropertyDescriptor(TransactionCacheEntry.prototype.constructor, 'length')
            expected = {
                "value": 0,
                "writable": false,
                "enumerable": false,
                "configurable": true
            }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(TransactionCacheEntry.prototype.constructor, 'length')`)
            if (error) { return error }

            actual = Reflect.getOwnPropertyDescriptor(TransactionCacheEntry.prototype.constructor, 'name')
            expected = {
                "value": "TransactionCacheEntry",
                "writable": false,
                "enumerable": false,
                "configurable": true
            }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(TransactionCacheEntry.prototype.constructor, 'name')`)
            if (error) { return error }
        }

        // Check the Symbol.toStringTag on the prototype is correct
        {
            actual = Reflect.getOwnPropertyDescriptor(TransactionCacheEntry.prototype, Symbol.toStringTag)
            expected = { "writable": false, "enumerable": false, "configurable": true, value: "TransactionCacheEntry" }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(TransactionCacheEntry.prototype, [Symbol.toStringTag])`)
            if (error) { return error }

            error = assert(typeof TransactionCacheEntry.prototype[Symbol.toStringTag], 'string', `typeof TransactionCacheEntry.prototype[Symbol.toStringTag]`)
            if (error) { return error }
        }

        // Check the insert method has correct descriptors, length and name
        {
            actual = Reflect.getOwnPropertyDescriptor(TransactionCacheEntry.prototype, 'insert')
            expected = { "writable": true, "enumerable": true, "configurable": true, value: TransactionCacheEntry.prototype.insert }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(TransactionCacheEntry.prototype, 'insert')`)
            if (error) { return error }

            error = assert(typeof TransactionCacheEntry.prototype.insert, 'function', `typeof TransactionCacheEntry.prototype.insert`)
            if (error) { return error }

            actual = Reflect.getOwnPropertyDescriptor(TransactionCacheEntry.prototype.insert, 'length')
            expected = {
                "value": 1,
                "writable": false,
                "enumerable": false,
                "configurable": true
            }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(TransactionCacheEntry.prototype.insert, 'length')`)
            if (error) { return error }

            actual = Reflect.getOwnPropertyDescriptor(TransactionCacheEntry.prototype.insert, 'name')
            expected = {
                "value": "insert",
                "writable": false,
                "enumerable": false,
                "configurable": true
            }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(TransactionCacheEntry.prototype.insert, 'name')`)
            if (error) { return error }
        }

        // Check the insertAndStreamBack method has correct descriptors, length and name
        {
            actual = Reflect.getOwnPropertyDescriptor(TransactionCacheEntry.prototype, 'insertAndStreamBack')
            expected = { "writable": true, "enumerable": true, "configurable": true, value: TransactionCacheEntry.prototype.insertAndStreamBack }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(TransactionCacheEntry.prototype, 'insertAndStreamBack')`)
            if (error) { return error }

            error = assert(typeof TransactionCacheEntry.prototype.insertAndStreamBack, 'function', `typeof TransactionCacheEntry.prototype.insertAndStreamBack`)
            if (error) { return error }

            actual = Reflect.getOwnPropertyDescriptor(TransactionCacheEntry.prototype.insertAndStreamBack, 'length')
            expected = {
                "value": 1,
                "writable": false,
                "enumerable": false,
                "configurable": true
            }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(TransactionCacheEntry.prototype.insertAndStreamBack, 'length')`)
            if (error) { return error }

            actual = Reflect.getOwnPropertyDescriptor(TransactionCacheEntry.prototype.insertAndStreamBack, 'name')
            expected = {
                "value": "insertAndStreamBack",
                "writable": false,
                "enumerable": false,
                "configurable": true
            }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(TransactionCacheEntry.prototype.insertAndStreamBack, 'name')`)
            if (error) { return error }
        }

        // Check the update method has correct descriptors, length and name
        {
            actual = Reflect.getOwnPropertyDescriptor(TransactionCacheEntry.prototype, 'update')
            expected = { "writable": true, "enumerable": true, "configurable": true, value: TransactionCacheEntry.prototype.update }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(TransactionCacheEntry.prototype, 'update')`)
            if (error) { return error }

            error = assert(typeof TransactionCacheEntry.prototype.update, 'function', `typeof TransactionCacheEntry.prototype.update`)
            if (error) { return error }

            actual = Reflect.getOwnPropertyDescriptor(TransactionCacheEntry.prototype.update, 'length')
            expected = {
                "value": 1,
                "writable": false,
                "enumerable": false,
                "configurable": true
            }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(TransactionCacheEntry.prototype.update, 'length')`)
            if (error) { return error }

            actual = Reflect.getOwnPropertyDescriptor(TransactionCacheEntry.prototype.update, 'name')
            expected = {
                "value": "update",
                "writable": false,
                "enumerable": false,
                "configurable": true
            }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(TransactionCacheEntry.prototype.update, 'name')`)
            if (error) { return error }
        }

        // Check the cancel method has correct descriptors, length and name
        {
            actual = Reflect.getOwnPropertyDescriptor(TransactionCacheEntry.prototype, 'cancel')
            expected = { "writable": true, "enumerable": true, "configurable": true, value: TransactionCacheEntry.prototype.cancel }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(TransactionCacheEntry.prototype, 'cancel')`)
            if (error) { return error }

            error = assert(typeof TransactionCacheEntry.prototype.cancel, 'function', `typeof TransactionCacheEntry.prototype.cancel`)
            if (error) { return error }

            actual = Reflect.getOwnPropertyDescriptor(TransactionCacheEntry.prototype.cancel, 'length')
            expected = {
                "value": 0,
                "writable": false,
                "enumerable": false,
                "configurable": true
            }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(TransactionCacheEntry.prototype.cancel, 'length')`)
            if (error) { return error }

            actual = Reflect.getOwnPropertyDescriptor(TransactionCacheEntry.prototype.cancel, 'name')
            expected = {
                "value": "cancel",
                "writable": false,
                "enumerable": false,
                "configurable": true
            }
            error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(TransactionCacheEntry.prototype.cancel, 'name')`)
            if (error) { return error }
        }

        return pass("ok")
    });

    // insert(options: TransactionInsertOptions): FastlyBody;
    {
        routes.set("/transaction-cache-entry/insert/called-as-constructor", () => {
            let error = assertThrows(() => {
                let entry = CoreCache.transactionLookup('1')
                new entry.insert({maxAge: 1})
            }, TypeError)
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/transaction-cache-entry/insert/entry-parameter-not-supplied", () => {
            let entry = CoreCache.transactionLookup('1')
            let error = assert(entry instanceof TransactionCacheEntry, true, "entry instanceof TransactionCacheEntry");
            if (error) { return error }
            error = assertThrows(() => {
                entry.insert()
            }, TypeError, `insert: At least 1 argument required, but only 0 passed`)
            if (error) { return error }
            return pass("ok")
        });

        routes.set("/transaction-cache-entry/insert/options-parameter-maxAge-field-valid-record", () => {
            let body;
            let error = assertDoesNotThrow(() => {
                let entry = CoreCache.transactionLookup('1')
                body = entry.insert({maxAge: 1})
            })
            if (error) { return error }
            error = assert(body instanceof FastlyBody, true, `entry.insert({maxAge: 1}) instanceof FastlyBody`)
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/transaction-cache-entry/insert/options-parameter-maxAge-field-NaN", () => {
            let error = assertThrows(() => {
                let entry = CoreCache.transactionLookup('1')
                entry.insert({
                    maxAge: NaN
                })
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/transaction-cache-entry/insert/options-parameter-maxAge-field-postitive-infinity", () => {
            let error = assertThrows(() => {
                let entry = CoreCache.transactionLookup('1')
                entry.insert({
                    maxAge: Number.POSITIVE_INFINITY
                })
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/transaction-cache-entry/insert/options-parameter-maxAge-field-negative-infinity", () => {
            let error = assertThrows(() => {
                let entry = CoreCache.transactionLookup('1')
                entry.insert({
                    maxAge: Number.NEGATIVE_INFINITY
                })
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/transaction-cache-entry/insert/options-parameter-maxAge-field-negative-number", () => {
            let error = assertThrows(() => {
                let entry = CoreCache.transactionLookup('1')
                entry.insert({
                    maxAge: -1
                })
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/transaction-cache-entry/insert/options-parameter-initialAge-field-valid-record", () => {
            let body;
            let error = assertDoesNotThrow(() => {
                let entry = CoreCache.transactionLookup('1')
                body =  entry.insert({maxAge: 1,initialAge: 1})
            })
            if (error) { return error }
            error = assert(body instanceof FastlyBody, true, `entry.insert({maxAge: 1,initialAge: 1}) instanceof FastlyBody`)
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/transaction-cache-entry/insert/options-parameter-initialAge-field-NaN", () => {
            let error = assertThrows(() => {
                let entry = CoreCache.transactionLookup('1')
                entry.insert({
                    maxAge: 1,
                    initialAge: NaN
                })
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/transaction-cache-entry/insert/options-parameter-initialAge-field-postitive-infinity", () => {
            let error = assertThrows(() => {
                let entry = CoreCache.transactionLookup('1')
                entry.insert({
                    maxAge: 1,
                    initialAge: Number.POSITIVE_INFINITY
                })
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/transaction-cache-entry/insert/options-parameter-initialAge-field-negative-infinity", () => {
            let error = assertThrows(() => {
                let entry = CoreCache.transactionLookup('1')
                entry.insert({
                    maxAge: 1,
                    initialAge: Number.NEGATIVE_INFINITY
                })
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/transaction-cache-entry/insert/options-parameter-initialAge-field-negative-number", () => {
            let error = assertThrows(() => {
                let entry = CoreCache.transactionLookup('1')
                entry.insert({
                    maxAge: 1,
                    initialAge: -1
                })
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/transaction-cache-entry/insert/options-parameter-staleWhileRevalidate-field-valid-record", () => {
            let body;
            let error = assertDoesNotThrow(() => {
                let entry = CoreCache.transactionLookup('1')
                body =  entry.insert({maxAge: 1,staleWhileRevalidate: 1})
            })
            if (error) { return error }
            error = assert(body instanceof FastlyBody, true, `entry.insert({maxAge: 1,staleWhileRevalidate: 1}) instanceof FastlyBody`)
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/transaction-cache-entry/insert/options-parameter-staleWhileRevalidate-field-NaN", () => {
            let error = assertThrows(() => {
                let entry = CoreCache.transactionLookup('1')
                entry.insert({
                    maxAge: 1,
                    staleWhileRevalidate: NaN
                })
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/transaction-cache-entry/insert/options-parameter-staleWhileRevalidate-field-postitive-infinity", () => {
            let error = assertThrows(() => {
                let entry = CoreCache.transactionLookup('1')
                entry.insert({
                    maxAge: 1,
                    staleWhileRevalidate: Number.POSITIVE_INFINITY
                })
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/transaction-cache-entry/insert/options-parameter-staleWhileRevalidate-field-negative-infinity", () => {
            let error = assertThrows(() => {
                let entry = CoreCache.transactionLookup('1')
                entry.insert({
                    maxAge: 1,
                    staleWhileRevalidate: Number.NEGATIVE_INFINITY
                })
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/transaction-cache-entry/insert/options-parameter-staleWhileRevalidate-field-negative-number", () => {
            let error = assertThrows(() => {
                let entry = CoreCache.transactionLookup('1')
                entry.insert({
                    maxAge: 1,
                    staleWhileRevalidate: -1
                })
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/transaction-cache-entry/insert/options-parameter-length-field-valid-record", () => {
            let body;
            let error = assertDoesNotThrow(() => {
                let entry = CoreCache.transactionLookup('1')
                body =  entry.insert({maxAge: 1,length: 1})
            })
            if (error) { return error }
            error = assert(body instanceof FastlyBody, true, `entry.insert({maxAge: 1,length: 1}) instanceof FastlyBody`)
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/transaction-cache-entry/insert/options-parameter-length-field-NaN", () => {
            let error = assertThrows(() => {
                let entry = CoreCache.transactionLookup('1')
                entry.insert({
                    maxAge: 1,
                    length: NaN
                })
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/transaction-cache-entry/insert/options-parameter-length-field-postitive-infinity", () => {
            let error = assertThrows(() => {
                let entry = CoreCache.transactionLookup('1')
                entry.insert({
                    maxAge: 1,
                    length: Number.POSITIVE_INFINITY
                })
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/transaction-cache-entry/insert/options-parameter-length-field-negative-infinity", () => {
            let error = assertThrows(() => {
                let entry = CoreCache.transactionLookup('1')
                entry.insert({
                    maxAge: 1,
                    length: Number.NEGATIVE_INFINITY
                })
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/transaction-cache-entry/insert/options-parameter-length-field-negative-number", () => {
            let error = assertThrows(() => {
                let entry = CoreCache.transactionLookup('1')
                entry.insert({
                    maxAge: 1,
                    length: -1
                })
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/transaction-cache-entry/insert/options-parameter-sensitive-field", () => {
            let error = assertDoesNotThrow(() => {
                let entry = CoreCache.transactionLookup('1')
                entry.insert({
                    maxAge: 1,
                    sensitive: true
                })
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/transaction-cache-entry/insert/options-parameter-vary-field", () => {
            let error = assertDoesNotThrow(() => {
                let entry = CoreCache.transactionLookup('1')
                entry.insert({
                    maxAge: 1,
                    vary: ["animal", "mineral", "vegetable"]
                })
            })
            if (error) { return error }
            return pass("ok")
        });
    }

    // insertAndStreamBack(options: TransactionInsertOptions): [FastlyBody, CacheEntry];
    {
        routes.set("/transaction-cache-entry/insertAndStreamBack/called-as-constructor", (event) => {
            const path = (new URL(event.request.url)).pathname
            let error = assertThrows(() => {
                let entry = CoreCache.transactionLookup(path)
                new entry.insertAndStreamBack({maxAge: 1})
            }, TypeError)
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/transaction-cache-entry/insertAndStreamBack/entry-parameter-not-supplied", (event) => {
            const path = (new URL(event.request.url)).pathname
            let error = assertThrows(() => {
                let entry = CoreCache.transactionLookup(path)
                entry.insertAndStreamBack()
            }, TypeError, `insertAndStreamBack: At least 1 argument required, but only 0 passed`)
            if (error) { return error }
            return pass("ok")
        });

        routes.set("/transaction-cache-entry/insertAndStreamBack/options-parameter-maxAge-field-valid-record", (event) => {
            const path = (new URL(event.request.url)).pathname
            let writer;
            let reader;
            let error = assertDoesNotThrow(() => {
                let entry = CoreCache.transactionLookup(path);
                [writer, reader] = entry.insertAndStreamBack({maxAge: 1})
            })
            if (error) { return error }
            error = assert(writer instanceof FastlyBody, true, `writer instanceof FastlyBody`)
            if (error) { return error }
            error = assert(reader instanceof CacheEntry, true, `writer instanceof CacheEntry`)
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/transaction-cache-entry/insertAndStreamBack/options-parameter-maxAge-field-NaN", (event) => {
            const path = (new URL(event.request.url)).pathname
            let error = assertThrows(() => {
                let entry = CoreCache.transactionLookup(path)
                entry.insertAndStreamBack({
                    maxAge: NaN
                })
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/transaction-cache-entry/insertAndStreamBack/options-parameter-maxAge-field-postitive-infinity", (event) => {
            const path = (new URL(event.request.url)).pathname
            let error = assertThrows(() => {
                let entry = CoreCache.transactionLookup(path)
                entry.insertAndStreamBack({
                    maxAge: Number.POSITIVE_INFINITY
                })
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/transaction-cache-entry/insertAndStreamBack/options-parameter-maxAge-field-negative-infinity", (event) => {
            const path = (new URL(event.request.url)).pathname
            let error = assertThrows(() => {
                let entry = CoreCache.transactionLookup(path)
                entry.insertAndStreamBack({
                    maxAge: Number.NEGATIVE_INFINITY
                })
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/transaction-cache-entry/insertAndStreamBack/options-parameter-maxAge-field-negative-number", (event) => {
            const path = (new URL(event.request.url)).pathname
            let error = assertThrows(() => {
                let entry = CoreCache.transactionLookup(path)
                entry.insertAndStreamBack({
                    maxAge: -1
                })
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/transaction-cache-entry/insertAndStreamBack/options-parameter-initialAge-field-valid-record", (event) => {
            const path = (new URL(event.request.url)).pathname
            let writer;
            let reader;
            let error = assertDoesNotThrow(() => {
                let entry = CoreCache.transactionLookup(path);
                [writer, reader] =  entry.insertAndStreamBack({maxAge: 1,initialAge: 1})
            })
            if (error) { return error }
            error = assert(writer instanceof FastlyBody, true, `writer instanceof FastlyBody`)
            if (error) { return error }
            error = assert(reader instanceof CacheEntry, true, `writer instanceof CacheEntry`)
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/transaction-cache-entry/insertAndStreamBack/options-parameter-initialAge-field-NaN", (event) => {
            const path = (new URL(event.request.url)).pathname
            let error = assertThrows(() => {
                let entry = CoreCache.transactionLookup(path)
                entry.insertAndStreamBack({
                    maxAge: 1,
                    initialAge: NaN
                })
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/transaction-cache-entry/insertAndStreamBack/options-parameter-initialAge-field-postitive-infinity", (event) => {
            const path = (new URL(event.request.url)).pathname
            let error = assertThrows(() => {
                let entry = CoreCache.transactionLookup(path)
                entry.insertAndStreamBack({
                    maxAge: 1,
                    initialAge: Number.POSITIVE_INFINITY
                })
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/transaction-cache-entry/insertAndStreamBack/options-parameter-initialAge-field-negative-infinity", (event) => {
            const path = (new URL(event.request.url)).pathname
            let error = assertThrows(() => {
                let entry = CoreCache.transactionLookup(path)
                entry.insertAndStreamBack({
                    maxAge: 1,
                    initialAge: Number.NEGATIVE_INFINITY
                })
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/transaction-cache-entry/insertAndStreamBack/options-parameter-initialAge-field-negative-number", (event) => {
            const path = (new URL(event.request.url)).pathname
            let error = assertThrows(() => {
                let entry = CoreCache.transactionLookup(path)
                entry.insertAndStreamBack({
                    maxAge: 1,
                    initialAge: -1
                })
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/transaction-cache-entry/insertAndStreamBack/options-parameter-staleWhileRevalidate-field-valid-record", (event) => {
            const path = (new URL(event.request.url)).pathname
            let writer,reader;
            let error = assertDoesNotThrow(() => {
                let entry = CoreCache.transactionLookup(path);
                [writer, reader] =  entry.insertAndStreamBack({maxAge: 1,staleWhileRevalidate: 1})
            })
            if (error) { return error }
            error = assert(writer instanceof FastlyBody, true, `writer instanceof FastlyBody`)
            if (error) { return error }
            error = assert(reader instanceof CacheEntry, true, `writer instanceof CacheEntry`)
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/transaction-cache-entry/insertAndStreamBack/options-parameter-staleWhileRevalidate-field-NaN", (event) => {
            const path = (new URL(event.request.url)).pathname
            let error = assertThrows(() => {
                let entry = CoreCache.transactionLookup(path)
                entry.insertAndStreamBack({
                    maxAge: 1,
                    staleWhileRevalidate: NaN
                })
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/transaction-cache-entry/insertAndStreamBack/options-parameter-staleWhileRevalidate-field-postitive-infinity", (event) => {
            const path = (new URL(event.request.url)).pathname
            let error = assertThrows(() => {
                let entry = CoreCache.transactionLookup(path)
                entry.insertAndStreamBack({
                    maxAge: 1,
                    staleWhileRevalidate: Number.POSITIVE_INFINITY
                })
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/transaction-cache-entry/insertAndStreamBack/options-parameter-staleWhileRevalidate-field-negative-infinity", (event) => {
            const path = (new URL(event.request.url)).pathname
            let error = assertThrows(() => {
                let entry = CoreCache.transactionLookup(path)
                entry.insertAndStreamBack({
                    maxAge: 1,
                    staleWhileRevalidate: Number.NEGATIVE_INFINITY
                })
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/transaction-cache-entry/insertAndStreamBack/options-parameter-staleWhileRevalidate-field-negative-number", (event) => {
            const path = (new URL(event.request.url)).pathname
            let error = assertThrows(() => {
                let entry = CoreCache.transactionLookup(path)
                entry.insertAndStreamBack({
                    maxAge: 1,
                    staleWhileRevalidate: -1
                })
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/transaction-cache-entry/insertAndStreamBack/options-parameter-length-field-valid-record", (event) => {
            const path = (new URL(event.request.url)).pathname
            let writer, reader;
            let error = assertDoesNotThrow(() => {
                let entry = CoreCache.transactionLookup(path);
                [writer, reader] =  entry.insertAndStreamBack({maxAge: 1,length: 1})
            })
            if (error) { return error }
            error = assert(writer instanceof FastlyBody, true, `writer instanceof FastlyBody`)
            if (error) { return error }
            error = assert(reader instanceof CacheEntry, true, `writer instanceof CacheEntry`)
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/transaction-cache-entry/insertAndStreamBack/options-parameter-length-field-NaN", (event) => {
            const path = (new URL(event.request.url)).pathname
            let error = assertThrows(() => {
                let entry = CoreCache.transactionLookup(path)
                entry.insertAndStreamBack({
                    maxAge: 1,
                    length: NaN
                })
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/transaction-cache-entry/insertAndStreamBack/options-parameter-length-field-postitive-infinity", (event) => {
            const path = (new URL(event.request.url)).pathname
            let error = assertThrows(() => {
                let entry = CoreCache.transactionLookup(path)
                entry.insertAndStreamBack({
                    maxAge: 1,
                    length: Number.POSITIVE_INFINITY
                })
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/transaction-cache-entry/insertAndStreamBack/options-parameter-length-field-negative-infinity", (event) => {
            const path = (new URL(event.request.url)).pathname
            let error = assertThrows(() => {
                let entry = CoreCache.transactionLookup(path)
                entry.insertAndStreamBack({
                    maxAge: 1,
                    length: Number.NEGATIVE_INFINITY
                })
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/transaction-cache-entry/insertAndStreamBack/options-parameter-length-field-negative-number", (event) => {
            const path = (new URL(event.request.url)).pathname
            let error = assertThrows(() => {
                let entry = CoreCache.transactionLookup(path)
                entry.insertAndStreamBack({
                    maxAge: 1,
                    length: -1
                })
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/transaction-cache-entry/insertAndStreamBack/options-parameter-sensitive-field", (event) => {
            const path = (new URL(event.request.url)).pathname
            let error = assertDoesNotThrow(() => {
                let entry = CoreCache.transactionLookup(path)
                entry.insertAndStreamBack({
                    maxAge: 1,
                    sensitive: true
                })
            })
            if (error) { return error }
            return pass("ok")
        });

        routes.set("/transaction-cache-entry/insertAndStreamBack/write-to-writer-and-read-from-reader", async (event) => {
            const path = (new URL(event.request.url)).pathname
            let entry = CoreCache.transactionLookup(path)
            let [writer, reader] = entry.insertAndStreamBack({
                maxAge: 60 * 1000,
                sensitive: true
            })
            writer.append("hello");
            writer.close();
            const actual = await new Response(reader.body()).text();
            let error = assert(actual, "hello", `actual === "hello"`);
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/transaction-cache-entry/insertAndStreamBack/options-parameter-vary-field", (event) => {
            const path = (new URL(event.request.url)).pathname
            let error = assertDoesNotThrow(() => {
                let entry = CoreCache.transactionLookup(path)
                entry.insertAndStreamBack({
                    maxAge: 1,
                    vary: ["animal", "mineral", "vegetable"]
                })
            })
            if (error) { return error }
            return pass("ok")
        });
    }

    // update(options: TransactionInsertOptions): void;
    {
        routes.set("/transaction-cache-entry/update/called-as-constructor", (event) => {
            const path = (new URL(event.request.url)).pathname
            let error = assertThrows(() => {
                let entry = CoreCache.transactionLookup(path)
                new entry.update({maxAge: 1})
            }, TypeError)
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/transaction-cache-entry/update/entry-parameter-not-supplied", (event) => {
            const path = (new URL(event.request.url)).pathname
            let error = assertThrows(() => {
                let entry = CoreCache.transactionLookup(path)
                entry.update()
            }, TypeError, `update: At least 1 argument required, but only 0 passed`)
            if (error) { return error }
            return pass("ok")
        });

        routes.set("/transaction-cache-entry/update/options-parameter-maxAge-field-valid-record", (event) => {
            const path = (new URL(event.request.url)).pathname
            let error = assertDoesNotThrow(() => {
                let writer = CoreCache.insert(path, {
                    maxAge: 0,
                    staleWhileRevalidate: 60 * 1000
                });
                writer.append("meow");
                writer.close();
                let entry = CoreCache.transactionLookup(path);
                entry.update({maxAge: 1})
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/transaction-cache-entry/update/options-parameter-maxAge-field-NaN", (event) => {
            const path = (new URL(event.request.url)).pathname
            let error = assertThrows(() => {
                let entry = CoreCache.transactionLookup(path)
                entry.update({
                    maxAge: NaN
                })
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/transaction-cache-entry/update/options-parameter-maxAge-field-postitive-infinity", (event) => {
            const path = (new URL(event.request.url)).pathname
            let error = assertThrows(() => {
                let entry = CoreCache.transactionLookup(path)
                entry.update({
                    maxAge: Number.POSITIVE_INFINITY
                })
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/transaction-cache-entry/update/options-parameter-maxAge-field-negative-infinity", (event) => {
            const path = (new URL(event.request.url)).pathname
            let error = assertThrows(() => {
                let entry = CoreCache.transactionLookup(path)
                entry.update({
                    maxAge: Number.NEGATIVE_INFINITY
                })
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/transaction-cache-entry/update/options-parameter-maxAge-field-negative-number", (event) => {
            const path = (new URL(event.request.url)).pathname
            let error = assertThrows(() => {
                let entry = CoreCache.transactionLookup(path)
                entry.update({
                    maxAge: -1
                })
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/transaction-cache-entry/update/options-parameter-initialAge-field-valid-record", (event) => {
            const path = (new URL(event.request.url)).pathname
            let error = assertDoesNotThrow(() => {
                let writer = CoreCache.insert(path, {
                    maxAge: 0,
                    staleWhileRevalidate: 60 * 1000
                });
                writer.append("meow");
                writer.close();
                let entry = CoreCache.transactionLookup(path);
                entry.update({maxAge: 1,initialAge: 1})
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/transaction-cache-entry/update/options-parameter-initialAge-field-NaN", (event) => {
            const path = (new URL(event.request.url)).pathname
            let error = assertThrows(() => {
                let entry = CoreCache.transactionLookup(path)
                entry.update({
                    maxAge: 1,
                    initialAge: NaN
                })
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/transaction-cache-entry/update/options-parameter-initialAge-field-postitive-infinity", (event) => {
            const path = (new URL(event.request.url)).pathname
            let error = assertThrows(() => {
                let entry = CoreCache.transactionLookup(path)
                entry.update({
                    maxAge: 1,
                    initialAge: Number.POSITIVE_INFINITY
                })
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/transaction-cache-entry/update/options-parameter-initialAge-field-negative-infinity", (event) => {
            const path = (new URL(event.request.url)).pathname
            let error = assertThrows(() => {
                let entry = CoreCache.transactionLookup(path)
                entry.update({
                    maxAge: 1,
                    initialAge: Number.NEGATIVE_INFINITY
                })
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/transaction-cache-entry/update/options-parameter-initialAge-field-negative-number", (event) => {
            const path = (new URL(event.request.url)).pathname
            let error = assertThrows(() => {
                let entry = CoreCache.transactionLookup(path)
                entry.update({
                    maxAge: 1,
                    initialAge: -1
                })
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/transaction-cache-entry/update/options-parameter-staleWhileRevalidate-field-valid-record", (event) => {
            const path = (new URL(event.request.url)).pathname
            let error = assertDoesNotThrow(() => {
                let writer = CoreCache.insert(path, {
                    maxAge: 0,
                    staleWhileRevalidate: 60 * 1000
                });
                writer.append("meow");
                writer.close();
                let entry = CoreCache.transactionLookup(path);
                entry.update({maxAge: 1,staleWhileRevalidate: 1})
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/transaction-cache-entry/update/options-parameter-staleWhileRevalidate-field-NaN", (event) => {
            const path = (new URL(event.request.url)).pathname
            let error = assertThrows(() => {
                let entry = CoreCache.transactionLookup(path)
                entry.update({
                    maxAge: 1,
                    staleWhileRevalidate: NaN
                })
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/transaction-cache-entry/update/options-parameter-staleWhileRevalidate-field-postitive-infinity", (event) => {
            const path = (new URL(event.request.url)).pathname
            let error = assertThrows(() => {
                let entry = CoreCache.transactionLookup(path)
                entry.update({
                    maxAge: 1,
                    staleWhileRevalidate: Number.POSITIVE_INFINITY
                })
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/transaction-cache-entry/update/options-parameter-staleWhileRevalidate-field-negative-infinity", (event) => {
            const path = (new URL(event.request.url)).pathname
            let error = assertThrows(() => {
                let entry = CoreCache.transactionLookup(path)
                entry.update({
                    maxAge: 1,
                    staleWhileRevalidate: Number.NEGATIVE_INFINITY
                })
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/transaction-cache-entry/update/options-parameter-staleWhileRevalidate-field-negative-number", (event) => {
            const path = (new URL(event.request.url)).pathname
            let error = assertThrows(() => {
                let entry = CoreCache.transactionLookup(path)
                entry.update({
                    maxAge: 1,
                    staleWhileRevalidate: -1
                })
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/transaction-cache-entry/update/options-parameter-length-field-valid-record", (event) => {
            const path = (new URL(event.request.url)).pathname
            let error = assertDoesNotThrow(() => {
                let writer = CoreCache.insert(path, {
                    maxAge: 0,
                    staleWhileRevalidate: 60 * 1000
                });
                writer.append("meow");
                writer.close();
                let entry = CoreCache.transactionLookup(path);
                entry.update({maxAge: 1,length: 1})
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/transaction-cache-entry/update/options-parameter-length-field-NaN", (event) => {
            const path = (new URL(event.request.url)).pathname
            let error = assertThrows(() => {
                let entry = CoreCache.transactionLookup(path)
                entry.update({
                    maxAge: 1,
                    length: NaN
                })
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/transaction-cache-entry/update/options-parameter-length-field-postitive-infinity", (event) => {
            const path = (new URL(event.request.url)).pathname
            let error = assertThrows(() => {
                let entry = CoreCache.transactionLookup(path)
                entry.update({
                    maxAge: 1,
                    length: Number.POSITIVE_INFINITY
                })
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/transaction-cache-entry/update/options-parameter-length-field-negative-infinity", (event) => {
            const path = (new URL(event.request.url)).pathname
            let error = assertThrows(() => {
                let entry = CoreCache.transactionLookup(path)
                entry.update({
                    maxAge: 1,
                    length: Number.NEGATIVE_INFINITY
                })
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/transaction-cache-entry/update/options-parameter-length-field-negative-number", (event) => {
            const path = (new URL(event.request.url)).pathname
            let error = assertThrows(() => {
                let entry = CoreCache.transactionLookup(path)
                entry.update({
                    maxAge: 1,
                    length: -1
                })
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/transaction-cache-entry/update/write-to-writer-and-read-from-reader", async (event) => {
            const path = (new URL(event.request.url)).pathname
            let entry = CoreCache.transactionLookup(path);
            let writer = entry.insert({
                maxAge: 1,
                staleWhileRevalidate: 60 * 1000
            })
            writer.append("meow");
            writer.close();
            entry = CoreCache.transactionLookup(path);
            entry.update({
                maxAge: 60 * 1000,
            });
            await sleep(1000)
            entry = CoreCache.transactionLookup(path);
            let error = assert(entry.maxAge(), 60 * 1000, `entry2.maxAge() === 60 * 1000`);
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/transaction-cache-entry/update/options-parameter-vary-field", async (event) => {
            const path = (new URL(event.request.url)).pathname
            let entry = CoreCache.transactionLookup(path);
            let writer = entry.insert({
                maxAge: 1,
                staleWhileRevalidate: 60 * 1000,
                vary: ["animal", "mineral", "vegetable"]
            })
            writer.append("meow");
            writer.close();
            await sleep(1000)
            entry = CoreCache.transactionLookup(path);
            entry.update({
                maxAge: 1000,
                vary: ["animal"]
            });
            return pass("ok")
        });
        routes.set("/transaction-cache-entry/update/options-parameter-userMetadata-field", async (event) => {
            const path = (new URL(event.request.url)).pathname
            let entry = CoreCache.transactionLookup(path);
            let writer = entry.insert({
                maxAge: 1,
                staleWhileRevalidate: 60 * 1000,
            })
            writer.append("meow");
            writer.close();
            await sleep(1000)
            entry = CoreCache.transactionLookup(path);
            entry.update({
                maxAge: 1000,
                userMetadata: 'hello'
            });
            return pass("ok")
        });
        // TODO: tests for options parameter fields
        // surrogateKeys?: Array<string>,-- empty string? -- toString which throws -- wrong types?
    }

    // cancel(): void;
    {
        routes.set("/transaction-cache-entry/cancel/called-as-constructor", (event) => {
            const path = (new URL(event.request.url)).pathname
            let error = assertThrows(() => {
                let entry = CoreCache.transactionLookup(path)
                new entry.cancel()
            }, TypeError)
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/transaction-cache-entry/cancel/called-once", (event) => {
            const path = (new URL(event.request.url)).pathname
            let error = assertDoesNotThrow(() => {
                let entry = CoreCache.transactionLookup(path)
                entry.cancel()
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/transaction-cache-entry/cancel/makes-entry-cancelled", (event) => {
            const path = (new URL(event.request.url)).pathname
            let entry
            let error = assertDoesNotThrow(() => {
                entry = CoreCache.transactionLookup(path)
                entry.cancel()
            })
            if (error) { return error }
            error = assertThrows(() => {
                entry.insert({maxAge: 1})
            })
            if (error) { return error }
            return pass("ok")
        });
        routes.set("/transaction-cache-entry/cancel/called-twice-throws", (event) => {
            const path = (new URL(event.request.url)).pathname
            let entry
            let error = assertDoesNotThrow(() => {
                entry = CoreCache.transactionLookup(path)
                entry.cancel()
            })
            if (error) { return error }
            error = assertThrows(() => {
                entry.cancel()
            })
            if (error) { return error }
            return pass("ok")
        });
    }
}

{

    routes.set("/core-cache/transaction-lookup-transaction-insert-vary-works", async () => {
        const key = `/core-cache/vary-works-${Date.now()}`
        const animal = 'animal'
        let entry = CoreCache.transactionLookup(key, {
            headers: {
                [animal]: 'cat'
            }
        })
        let error = assert(entry.state().found(), false, `entry.state().found() === false`)
        if (error) { return error }
        let writer = entry.insert({
            maxAge: 60_000 * 60,
            vary: [animal],
            headers: {
                [animal]: 'cat'
            }
        })

        writer.append('cat')
        writer.close()
        entry.close()
        await sleep(1_000);

        entry = CoreCache.transactionLookup(key, {
            headers: {
                [animal]: 'cat'
            }
        })
        error = assert(entry.state().found(), true, `entry.state().found() === true`)
        if (error) { return error }

        error = assert(await streamToString(entry.body()), 'cat', `await streamToString(CoreCache.lookup(key).body())`)
        if (error) { return error }
        entry.close()

        entry = CoreCache.transactionLookup(key, {
            headers: {
                [animal]: 'dog'
            }
        })
        error = assert(entry.state().found(), false, `entry.state().found() == false`)
        if (error) { return error }

        writer = entry.insert({
            maxAge: 60_000 * 60,
            vary: [animal],
            headers: {
                [animal]: 'dog'
            }
        })

        writer.append('dog')
        writer.close()
        entry.close()
        await sleep(1_000);

        entry = CoreCache.transactionLookup(key, {
            headers: {
                [animal]: 'dog'
            }
        })
        error = assert(entry.state().found(), true, `entry.state().found() === true`)
        if (error) { return error }

        error = assert(await streamToString(entry.body()), 'dog', `await streamToString(CoreCache.lookup(key).body())`)
        if (error) { return error }
        entry.close()

        return pass("ok")
    });

    routes.set("/core-cache/lookup-insert-vary-works", async () => {
        const key = `/core-cache/vary-works-${Date.now()}`
        const animal = 'animal'
        let entry = CoreCache.lookup(key, {
            headers: {
                [animal]: 'cat'
            }
        })
        let error = assert(entry, null, `entry == null`)
        if (error) { return error }
        let writer = CoreCache.insert(key, {
            maxAge: 60_000 * 60,
            vary: [animal],
            headers: {
                [animal]: 'cat'
            }
        })

        writer.append('cat')
        writer.close()
        await sleep(1_000);

        entry = CoreCache.lookup(key, {
            headers: {
                [animal]: 'cat'
            }
        })
        error = assert(entry.state().found(), true, `entry.state().found() === true`)
        if (error) { return error }

        error = assert(await streamToString(entry.body()), 'cat', `await streamToString(CoreCache.lookup(key).body())`)
        if (error) { return error }
        entry.close()

        entry = CoreCache.lookup(key, {
            headers: {
                [animal]: 'dog'
            }
        })
        error = assert(entry, null, `entry == null`)
        if (error) { return error }

        writer = CoreCache.insert(key, {
            maxAge: 60_000 * 60,
            vary: [animal],
            headers: {
                [animal]: 'dog'
            }
        })

        writer.append('dog')
        writer.close()
        await sleep(1_000);

        entry = CoreCache.lookup(key, {
            headers: {
                [animal]: 'dog'
            }
        })
        error = assert(entry.state().found(), true, `entry.state().found() === true`)
        if (error) { return error }

        error = assert(await streamToString(entry.body()), 'dog', `await streamToString(CoreCache.lookup(key).body())`)
        if (error) { return error }

        return pass("ok")
    });
}
