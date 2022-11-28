/* eslint-env serviceworker */
/* global ReadableStream fastly ObjectStore ObjectStoreEntry */
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
// ObjectStore
{
    routes.set("/object-store/exposed-as-global", async () => {
        let error = assert(typeof ObjectStore, 'function', `typeof ObjectStore`)
        if (error) { return error }
        return pass()
    });
    routes.set("/object-store/interface", objectStoreInterfaceTests);
    // ObjectStore constructor
    {

        routes.set("/object-store/constructor/called-as-regular-function", async () => {
            let error = assertThrows(() => {
                ObjectStore()
            }, TypeError, `calling a builtin ObjectStore constructor without new is forbidden`)
            if (error) { return error }
            return pass()
        });
        // https://tc39.es/ecma262/#sec-tostring
        routes.set("/object-store/constructor/parameter-calls-7.1.17-ToString", async () => {
            let sentinel;
            const test = () => {
                sentinel = Symbol();
                const name = {
                    toString() {
                        throw sentinel;
                    }
                }
                new ObjectStore(name)
            }
            let error = assertThrows(test)
            if (error) { return error }
            try {
                test()
            } catch (thrownError) {
                let error = assert(thrownError, sentinel, 'thrownError === sentinel')
                if (error) { return error }
            }
            error = assertThrows(() => new ObjectStore(Symbol()), TypeError, `can't convert symbol to string`)
            if (error) { return error }
            return pass()
        });
        routes.set("/object-store/constructor/empty-parameter", async () => {
            let error = assertThrows(() => {
                new ObjectStore()
            }, TypeError, `ObjectStore constructor: At least 1 argument required, but only 0 passed`)
            if (error) { return error }
            return pass()
        });
        routes.set("/object-store/constructor/found-store", async () => {
            const store = createValidStore()
            let error = assert(store instanceof ObjectStore, true, `store instanceof ObjectStore`)
            if (error) { return error }
            return pass()
        });
        routes.set("/object-store/constructor/missing-store", async () => {
            let error = assertThrows(() => {
                new ObjectStore('missing')
            }, Error, `ObjectStore constructor: No ObjectStore named 'missing' exists`)
            if (error) { return error }
            return pass()
        });
        routes.set("/object-store/constructor/invalid-name", async () => {
            // control Characters (\\u0000-\\u001F) are not allowed
            const controlCharacters = [
                '\u0000', '\u0001', '\u0002', '\u0003', '\u0004', '\u0005',
                '\u0006', '\u0007', '\u0008', '\u0009', '\u000A', '\u000B',
                '\u000C', '\u000D', '\u000E', '\u0010', '\u0011', '\u0012',
                '\u0013', '\u0014', '\u0015', '\u0016', '\u0017', '\u0018',
                '\u0019', '\u001A', '\u001B', '\u001C', '\u001D', '\u001E',
                '\u001F'
            ];
            for (const character of controlCharacters) {
                let error = assertThrows(() => {
                    new ObjectStore(character)
                }, TypeError, `ObjectStore constructor: name can not contain control characters (\\u0000-\\u001F)`)
                if (error) { return error }
            }

            // must be less than 256 characters
            let error = assertThrows(() => {
                new ObjectStore('1'.repeat(256))
            }, TypeError, `ObjectStore constructor: name can not be more than 255 characters`)
            if (error) { return error }

            // empty string not allowed
            error = assertThrows(() => {
                new ObjectStore('')
            }, TypeError, `ObjectStore constructor: name can not be an empty string`)
            if (error) { return error }
            return pass()
        });
    }
    // ObjectStore put method
    {

        routes.set("/object-store/put/called-as-constructor", async () => {
            let error = assertThrows(() => {
                new ObjectStore.prototype.put('1', '1')
            }, TypeError, `ObjectStore.prototype.put is not a constructor`)
            if (error) { return error }
            return pass()
        });
        routes.set("/object-store/put/called-unbound", async () => {
            let error = assertThrows(() => {
                ObjectStore.prototype.put.call(undefined, '1', '2')
            }, TypeError, "Method put called on receiver that's not an instance of ObjectStore")
            if (error) { return error }
            return pass()
        });
        // https://tc39.es/ecma262/#sec-tostring
        routes.set("/object-store/put/key-parameter-calls-7.1.17-ToString", async () => {
            let sentinel;
            const test = async () => {
                sentinel = Symbol();
                const key = {
                    toString() {
                        throw sentinel;
                    }
                }
                const store = createValidStore()
                await store.put(key, '')
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
                const store = createValidStore()
                await store.put(Symbol(), "")
            }, Error, `can't convert symbol to string`)
            if (error) { return error }
            return pass()
        });
        routes.set("/object-store/put/key-parameter-not-supplied", async () => {
            let error = await assertRejects(async () => {
                const store = createValidStore()
                await store.put()
            }, TypeError, `put: At least 2 arguments required, but only 0 passed`)
            if (error) { return error }
            return pass()
        });
        routes.set("/object-store/put/key-parameter-empty-string", async () => {
            let error = await assertRejects(async () => {
                const store = createValidStore()
                await store.put('', '')
            }, TypeError, `ObjectStore key can not be an empty string`)
            if (error) { return error }
            return pass()
        });
        routes.set("/object-store/put/key-parameter-1024-character-string", async () => {
            let error = await assertResolves(async () => {
                const store = createValidStore()
                const key = 'a'.repeat(1024)
                await store.put(key, '')
            })
            if (error) { return error }
            return pass()
        });
        routes.set("/object-store/put/key-parameter-1025-character-string", async () => {
            let error = await assertRejects(async () => {
                const store = createValidStore()
                const key = 'a'.repeat(1025)
                await store.put(key, '')
            }, TypeError, `ObjectStore key can not be more than 1024 characters`)
            if (error) { return error }
            return pass()
        });
        routes.set("/object-store/put/key-parameter-containing-newline", async () => {
            let error = await assertRejects(async () => {
                let store = createValidStore()
                await store.put('\n', '')
            }, TypeError, `ObjectStore key can not contain newline character`)
            if (error) { return error }
            return pass()
        });
        routes.set("/object-store/put/key-parameter-containing-carriage-return", async () => {
            let error = await assertRejects(async () => {
                let store = createValidStore()
                await store.put('\r', '')
            }, TypeError, `ObjectStore key can not contain carriage return character`)
            if (error) { return error }
            return pass()
        });
        routes.set("/object-store/put/key-parameter-starting-with-well-known-acme-challenge", async () => {
            let error = await assertRejects(async () => {
                let store = createValidStore()
                await store.put('.well-known/acme-challenge/', '')
            }, TypeError, `ObjectStore key can not start with .well-known/acme-challenge/`)
            if (error) { return error }
            return pass()
        });
        routes.set("/object-store/put/key-parameter-single-dot", async () => {
            let error = await assertRejects(async () => {
                let store = createValidStore()
                await store.put('.', '')
            }, TypeError, `ObjectStore key can not be '.' or '..'`)
            if (error) { return error }
            return pass()
        });
        routes.set("/object-store/put/key-parameter-double-dot", async () => {
            let error = await assertRejects(async () => {
                let store = createValidStore()
                await store.put('..', '')
            }, TypeError, `ObjectStore key can not be '.' or '..'`)
            if (error) { return error }
            return pass()
        });
        routes.set("/object-store/put/key-parameter-containing-special-characters", async () => {
            const specialCharacters = ['[', ']', '*', '?', '#'];
            for (const character of specialCharacters) {
                let error = await assertRejects(async () => {
                    let store = createValidStore()
                    await store.put(character, '')
                }, TypeError, `ObjectStore key can not contain ${character} character`)
                if (error) { return error }
            }
            return pass()
        });
        routes.set("/object-store/put/value-parameter-as-undefined", async () => {
            const store = createValidStore()
            let result = store.put("undefined", undefined)
            let error = assert(result instanceof Promise, true, 'store.put("undefined", undefined) instanceof Promise')
            if (error) { return error }
            error = assert(await result, undefined, 'await store.put("undefined", undefined)')
            if (error) { return error }
            return pass()
        });
        routes.set("/object-store/put/value-parameter-not-supplied", async () => {
            let error = await assertRejects(async () => {
                const store = createValidStore()
                await store.put("test")
            }, TypeError, `put: At least 2 arguments required, but only 1 passed`)
            if (error) { return error }
            return pass()
        });
        // - ReadableStream
        routes.set("/object-store/put/value-parameter-readablestream-empty", async () => {
            // TODO: remove this when streams are supported
            let error = await assertRejects(async () => {
                const stream = iteratableToStream([])
                const store = createValidStore()
                await store.put('readablestream-empty', stream)
            }, TypeError, `Content-provided streams are not yet supported for streaming into ObjectStore`)
            if (error) { return error }
            return pass()
            // TODO: uncomment this when conte-provided (guest) streams are supported
            // const stream = iteratableToStream([])
            // const store = createValidStore()
            // let result = store.put('readablestream-empty', stream)
            // let error = assert(result instanceof Promise, true, `store.put('readablestream-empty', stream) instanceof Promise`)
            // if (error) { return error }
            // error = assert(await result, undefined, `await store.put('readablestream-empty', stream)`)
            // if (error) { return error }
            // return pass()
        });
        routes.set("/object-store/put/value-parameter-readablestream-under-30mb", async () => {
            const res = await fetch('https://compute-sdk-test-backend.edgecompute.app/', {
                backend: "TheOrigin",
            })
            const store = createValidStore()
            let result = store.put('readablestream-under-30mb', res.body)
            let error = assert(result instanceof Promise, true, `store.put('readablestream-under-30mb', stream) instanceof Promise`)
            if (error) { return error }
            error = assert(await result, undefined, `await store.put('readablestream-under-30mb', stream)`)
            if (error) { return error }
            return pass()
        });
        routes.set("/object-store/put/value-parameter-readablestream-over-30mb", async () => {
            // TODO: remove this when streams are supported
            let error = await assertRejects(async () => {
                const stream = iteratableToStream(['x'.repeat(30 * 1024 * 1024) + 'x'])
                const store = createValidStore()
                await store.put('readablestream-over-30mb', stream)
            }, Error, `Content-provided streams are not yet supported for streaming into ObjectStore`)
            if (error) { return error }
            return pass()
            // TODO: uncomment this when conte-provided (guest) streams are supported
            // const stream = iteratableToStream(['x'.repeat(30*1024*1024) + 'x'])
            // const store = createValidStore()
            // let result = store.put('readablestream-over-30mb', stream)
            // let error = assert(result instanceof Promise, true, `store.put('readablestream-over-30mb', stream) instanceof Promise`)
            // if (error) { return error }
            // error = assert(await result, undefined, `await store.put('readablestream-over-30mb', stream)`)
            // if (error) { return error }
            // return pass()
        });
        routes.set("/object-store/put/value-parameter-readablestream-locked", async () => {
            const stream = iteratableToStream([])
            // getReader() causes the stream to become locked
            stream.getReader()
            const store = createValidStore()
            let error = await assertRejects(async () => {
                await store.put('readablestream-locked', stream)
                // await store.put("test", stream)
            }, TypeError, `Can't use a ReadableStream that's locked or has ever been read from or canceled`)
            if (error) { return error }
            return pass()
        });

        // - URLSearchParams
        routes.set("/object-store/put/value-parameter-URLSearchParams", async () => {
            const items = [
                new URLSearchParams,
                new URLSearchParams({ a: 'b', c: 'd' }),
            ];
            const store = createValidStore()
            for (const searchParams of items) {
                let result = store.put('URLSearchParams', searchParams)
                let error = assert(result instanceof Promise, true, `store.put('URLSearchParams', searchParams) instanceof Promise`)
                if (error) { return error }
                error = assert(await result, undefined, `await store.put('URLSearchParams', searchParams)`)
                if (error) { return error }
            }
            return pass()
        });
        // - USV strings
        routes.set("/object-store/put/value-parameter-strings", async () => {
            const strings = [
                // empty
                '',
                // lone surrogate
                '\uD800',
                // surrogate pair
                '𠈓',
                String('carrot'),
            ];
            const store = createValidStore()
            for (const string of strings) {
                let result = store.put('string', string)
                let error = assert(result instanceof Promise, true, `store.put('string', string) instanceof Promise`)
                if (error) { return error }
                error = assert(await result, undefined, `await store.put('string', string)`)
                if (error) { return error }
            }
            return pass()
        });

        routes.set("/object-store/put/value-parameter-string-over-30mb", async () => {
            const string = 'x'.repeat(35 * 1024 * 1024) + 'x'
            const store = createValidStore()
            let error = await assertRejects(() => store.put('string-over-30mb', string), TypeError, `ObjectStore value can not be more than 30 Megabytes in size`)
            if (error) { return error }
            return pass()
        });

        // https://tc39.es/ecma262/#sec-tostring
        routes.set("/object-store/put/value-parameter-calls-7.1.17-ToString", async () => {
            let sentinel;
            const test = async () => {
                sentinel = Symbol();
                const value = {
                    toString() {
                        throw sentinel;
                    }
                }
                const store = createValidStore()
                await store.put('toString', value)
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
                const store = createValidStore()
                await store.put("Symbol()", Symbol())
            }, TypeError, `can't convert symbol to string`)
            if (error) { return error }
            return pass()
        });

        // - buffer source
        routes.set("/object-store/put/value-parameter-buffer", async () => {
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
            const store = createValidStore()
            for (const constructor of typedArrayConstructors) {
                const typedArray = new constructor(8);
                let result = store.put(constructor.name, typedArray.buffer)
                let error = assert(result instanceof Promise, true, `store.put(${constructor.name}, typedArray.buffer) instanceof Promise`)
                if (error) { return error }
                error = assert(await result, undefined, `await store.put(${constructor.name}, typedArray.buffer)`)
                if (error) { return error }
            }
            return pass()
        });
        routes.set("/object-store/put/value-parameter-arraybuffer", async () => {
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
            const store = createValidStore()
            for (const constructor of typedArrayConstructors) {
                const typedArray = new constructor(8);
                let result = store.put(constructor.name, typedArray.buffer)
                let error = assert(result instanceof Promise, true, `store.put(${constructor.name}, typedArray.buffer) instanceof Promise`)
                if (error) { return error }
                error = assert(await result, undefined, `await store.put(${constructor.name}, typedArray.buffer)`)
                if (error) { return error }
            }
            return pass()
        });
        routes.set("/object-store/put/value-parameter-typed-arrays", async () => {
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
            const store = createValidStore()
            for (const constructor of typedArrayConstructors) {
                const typedArray = new constructor(8);
                let result = store.put(constructor.name, typedArray)
                let error = assert(result instanceof Promise, true, `store.put(${constructor.name}, typedArray) instanceof Promise`)
                if (error) { return error }
                error = assert(await result, undefined, `await store.put(${constructor.name}, typedArray)`)
                if (error) { return error }
            }
            return pass()
        });
        routes.set("/object-store/put/value-parameter-dataview", async () => {
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
            const store = createValidStore()
            for (const constructor of typedArrayConstructors) {
                const typedArray = new constructor(8);
                const view = new DataView(typedArray.buffer);
                let result = store.put(`new DataView(${constructor.name})`, view)
                let error = assert(result instanceof Promise, true, `store.put(new DataView(${constructor.name}), typedArray) instanceof Promise`)
                if (error) { return error }
                error = assert(await result, undefined, `await store.put(new DataView(${constructor.name}), typedArray)`)
                if (error) { return error }
            }
            return pass()
        });
    }

    // ObjectStore get method
    {
        routes.set("/object-store/get/called-as-constructor", async () => {
            let error = assertThrows(() => {
                new ObjectStore.prototype.get('1')
            }, TypeError, `ObjectStore.prototype.get is not a constructor`)
            if (error) { return error }
            return pass()
        });
        routes.set("/object-store/get/called-unbound", async () => {
            let error = await assertRejects(async () => {
                await ObjectStore.prototype.get.call(undefined, '1')
            }, TypeError, "Method get called on receiver that's not an instance of ObjectStore")
            if (error) { return error }
            return pass()
        });
        // https://tc39.es/ecma262/#sec-tostring
        routes.set("/object-store/get/key-parameter-calls-7.1.17-ToString", async () => {
            let sentinel;
            const test = async () => {
                sentinel = Symbol();
                const key = {
                    toString() {
                        throw sentinel;
                    }
                }
                const store = createValidStore()
                await store.get(key)
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
                const store = createValidStore()
                await store.get(Symbol())
            }, TypeError, `can't convert symbol to string`)
            if (error) { return error }
            return pass()
        });
        routes.set("/object-store/get/key-parameter-not-supplied", async () => {
            let error = await assertRejects(async () => {
                const store = createValidStore()
                await store.get()
            }, TypeError, `get: At least 1 argument required, but only 0 passed`)
            if (error) { return error }
            return pass()
        });
        routes.set("/object-store/get/key-parameter-empty-string", async () => {
            let error = await assertRejects(async () => {
                const store = createValidStore()
                await store.get('')
            }, TypeError, `ObjectStore key can not be an empty string`)
            if (error) { return error }
            return pass()
        });
        routes.set("/object-store/get/key-parameter-1024-character-string", async () => {
            let error = await assertResolves(async () => {
                const store = createValidStore()
                const key = 'a'.repeat(1024)
                await store.get(key)
            })
            if (error) { return error }
            return pass()
        });
        routes.set("/object-store/get/key-parameter-1025-character-string", async () => {
            let error = await assertRejects(async () => {
                const store = createValidStore()
                const key = 'a'.repeat(1025)
                await store.get(key)
            }, TypeError, `ObjectStore key can not be more than 1024 characters`)
            if (error) { return error }
            return pass()
        });
        routes.set("/object-store/get/key-parameter-containing-newline", async () => {
            let error = await assertRejects(async () => {
                let store = createValidStore()
                await store.get('\n')
            }, TypeError, `ObjectStore key can not contain newline character`)
            if (error) { return error }
            return pass()
        });
        routes.set("/object-store/get/key-parameter-containing-carriage-return", async () => {
            let error = await assertRejects(async () => {
                let store = createValidStore()
                await store.get('\r')
            }, TypeError, `ObjectStore key can not contain carriage return character`)
            if (error) { return error }
            return pass()
        });
        routes.set("/object-store/get/key-parameter-starting-with-well-known-acme-challenge", async () => {
            let error = await assertRejects(async () => {
                let store = createValidStore()
                await store.get('.well-known/acme-challenge/')
            }, TypeError, `ObjectStore key can not start with .well-known/acme-challenge/`)
            if (error) { return error }
            return pass()
        });
        routes.set("/object-store/get/key-parameter-single-dot", async () => {
            let error = await assertRejects(async () => {
                let store = createValidStore()
                await store.get('.')
            }, TypeError, `ObjectStore key can not be '.' or '..'`)
            if (error) { return error }
            return pass()
        });
        routes.set("/object-store/get/key-parameter-double-dot", async () => {
            let error = await assertRejects(async () => {
                let store = createValidStore()
                await store.get('..')
            }, TypeError, `ObjectStore key can not be '.' or '..'`)
            if (error) { return error }
            return pass()
        });
        routes.set("/object-store/get/key-parameter-containing-special-characters", async () => {
            const specialCharacters = ['[', ']', '*', '?', '#'];
            for (const character of specialCharacters) {
                let error = await assertRejects(async () => {
                    let store = createValidStore()
                    await store.get(character)
                }, TypeError, `ObjectStore key can not contain ${character} character`)
                if (error) { return error }
            }
            return pass()
        });
        routes.set("/object-store/get/key-does-not-exist-returns-null", async () => {
            let store = createValidStore()
            let result = store.get(Math.random())
            let error = assert(result instanceof Promise, true, `store.get(Math.random()) instanceof Promise`)
            if (error) { return error }
            error = assert(await result, null, `await store.get(Math.random())`)
            if (error) { return error }
            return pass()
        });
        routes.set("/object-store/get/key-does-not-exist-returns-null", async () => {
            let store = createValidStore()
            let result = store.get(Math.random())
            let error = assert(result instanceof Promise, true, `store.get(Math.random()) instanceof Promise`)
            if (error) { return error }
            error = assert(await result, null, `await store.get(Math.random())`)
            if (error) { return error }
            return pass()
        });
        routes.set("/object-store/get/key-exists", async () => {
            let store = createValidStore()
            let key = `key-exists-${Math.random()}`;
            await store.put(key, 'hello')
            let result = store.get(key)
            let error = assert(result instanceof Promise, true, `store.get(key) instanceof Promise`)
            if (error) { return error }
            result = await result
            error = assert(result instanceof ObjectStoreEntry, true, `(await store.get(key) instanceof ObjectStoreEntry)`)
            if (error) { return error }
            return pass()
        });
    }
}
// ObjectStoreEntry
{
    routes.set("/object-store-entry/interface", async () => {
        return objectStoreEntryInterfaceTests()
    });
    routes.set("/object-store-entry/text/valid", async () => {
        let store = createValidStore()
        let key = `entry-text-valid`;
        await store.put(key, 'hello')
        let entry = await store.get(key)
        let result = entry.text()
        let error = assert(result instanceof Promise, true, `entry.text() instanceof Promise`)
        if (error) { return error }
        result = await result
        error = assert(result, 'hello', `await entry.text())`)
        if (error) { return error }
        return pass()
    });
    routes.set("/object-store-entry/json/valid", async () => {
        let store = createValidStore()
        let key = `entry-json-valid`;
        const obj = { a: 1, b: 2, c: 3 }
        await store.put(key, JSON.stringify(obj))
        let entry = await store.get(key)
        let result = entry.json()
        let error = assert(result instanceof Promise, true, `entry.json() instanceof Promise`)
        if (error) { return error }
        result = await result
        error = assert(result, obj, `await entry.json())`)
        if (error) { return error }
        return pass()
    });
    routes.set("/object-store-entry/json/invalid", async () => {
        let store = createValidStore()
        let key = `entry-json-invalid`;
        await store.put(key, "132abc;['-=9")
        let entry = await store.get(key)
        let error = await assertRejects(() => entry.json(), SyntaxError, `JSON.parse: unexpected non-whitespace character after JSON data at line 1 column 4 of the JSON data`)
        if (error) { return error }
        return pass()
    });
    routes.set("/object-store-entry/arrayBuffer/valid", async () => {
        let store = createValidStore()
        let key = `entry-arraybuffer-valid`;
        await store.put(key, new Int8Array([0, 1, 2, 3]))
        let entry = await store.get(key)
        let result = entry.arrayBuffer()
        let error = assert(result instanceof Promise, true, `entry.arrayBuffer() instanceof Promise`)
        if (error) { return error }
        result = await result
        error = assert(result instanceof ArrayBuffer, true, `(await entry.arrayBuffer()) instanceof ArrayBuffer`)
        if (error) { return error }
        return pass()
    });

    routes.set("/object-store-entry/body", async () => {
        let store = createValidStore()
        let key = `entry-body`;
        await store.put(key, 'body body body')
        let entry = await store.get(key)
        let result = entry.body;
        let error = assert(result instanceof ReadableStream, true, `entry.body instanceof ReadableStream`)
        if (error) { return error }
        let text = await streamToString(result);
        error = assert(text, 'body body body', `entry.body contents as string`)
        if (error) { return error }
        return pass()
    });
    routes.set("/object-store-entry/bodyUsed", async () => {
        let store = createValidStore()
        let key = `entry-bodyUsed`;
        await store.put(key, 'body body body')
        let entry = await store.get(key)
        let error = assert(entry.bodyUsed, false, `entry.bodyUsed`)
        if (error) { return error }
        await entry.text();
        error = assert(entry.bodyUsed, true, `entry.bodyUsed`)
        if (error) { return error }
        return pass()
    });
}
async function objectStoreEntryInterfaceTests() {
    let actual = Reflect.ownKeys(ObjectStoreEntry)
    let expected = ["prototype", "length", "name"]
    let error = assert(actual, expected, `Reflect.ownKeys(ObjectStoreEntry)`)
    if (error) { return error }

    actual = Reflect.getOwnPropertyDescriptor(ObjectStoreEntry, 'prototype')
    expected = {
        "value": ObjectStoreEntry.prototype,
        "writable": false,
        "enumerable": false,
        "configurable": false
    }
    error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(ObjectStoreEntry, 'prototype')`)
    if (error) { return error }

    actual = Reflect.getOwnPropertyDescriptor(ObjectStoreEntry, 'length')
    expected = {
        "value": 0,
        "writable": false,
        "enumerable": false,
        "configurable": true
    }
    error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(ObjectStoreEntry, 'length')`)
    if (error) { return error }

    actual = Reflect.getOwnPropertyDescriptor(ObjectStoreEntry, 'name')
    expected = {
        "value": "ObjectStoreEntry",
        "writable": false,
        "enumerable": false,
        "configurable": true
    }
    error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(ObjectStoreEntry, 'name')`)
    if (error) { return error }

    actual = Reflect.ownKeys(ObjectStoreEntry.prototype)
    expected = ["constructor", "body", "bodyUsed", "arrayBuffer", "json", "text"]
    error = assert(actual, expected, `Reflect.ownKeys(ObjectStoreEntry.prototype)`)
    if (error) { return error }

    actual = Reflect.getOwnPropertyDescriptor(ObjectStoreEntry.prototype, 'constructor')
    expected = { "writable": true, "enumerable": false, "configurable": true, value: ObjectStoreEntry.prototype.constructor }
    error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(ObjectStoreEntry.prototype, 'constructor')`)
    if (error) { return error }

    actual = Reflect.getOwnPropertyDescriptor(ObjectStoreEntry.prototype, 'text')
    expected = { "writable": true, "enumerable": true, "configurable": true, value: ObjectStoreEntry.prototype.text }
    error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(ObjectStoreEntry.prototype, 'text')`)
    if (error) { return error }

    actual = Reflect.getOwnPropertyDescriptor(ObjectStoreEntry.prototype, 'json')
    expected = { "writable": true, "enumerable": true, "configurable": true, value: ObjectStoreEntry.prototype.json }
    error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(ObjectStoreEntry.prototype, 'json')`)
    if (error) { return error }

    actual = Reflect.getOwnPropertyDescriptor(ObjectStoreEntry.prototype, 'arrayBuffer')
    expected = { "writable": true, "enumerable": true, "configurable": true, value: ObjectStoreEntry.prototype.arrayBuffer }
    error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(ObjectStoreEntry.prototype, 'arrayBuffer')`)
    if (error) { return error }
    actual = Reflect.getOwnPropertyDescriptor(ObjectStoreEntry.prototype, 'body')
    error = assert(actual.enumerable, true, `Reflect.getOwnPropertyDescriptor(ObjectStoreEntry.prototype, 'body').enumerable`)
    error = assert(actual.configurable, true, `Reflect.getOwnPropertyDescriptor(ObjectStoreEntry.prototype, 'body').configurable`)
    error = assert('set' in actual, true, `'set' in Reflect.getOwnPropertyDescriptor(ObjectStoreEntry.prototype, 'body')`)
    error = assert(actual.set, undefined, `Reflect.getOwnPropertyDescriptor(ObjectStoreEntry.prototype, 'body').set`)
    error = assert(typeof actual.get, 'function', `typeof Reflect.getOwnPropertyDescriptor(ObjectStoreEntry.prototype, 'body').get`)
    if (error) { return error }
    actual = Reflect.getOwnPropertyDescriptor(ObjectStoreEntry.prototype, 'bodyUsed')
    error = assert(actual.enumerable, true, `Reflect.getOwnPropertyDescriptor(ObjectStoreEntry.prototype, 'bodyUsed').enumerable`)
    error = assert(actual.configurable, true, `Reflect.getOwnPropertyDescriptor(ObjectStoreEntry.prototype, 'bodyUsed').configurable`)
    error = assert('set' in actual, true, `'set' in Reflect.getOwnPropertyDescriptor(ObjectStoreEntry.prototype, 'bodyUsed')`)
    error = assert(actual.set, undefined, `Reflect.getOwnPropertyDescriptor(ObjectStoreEntry.prototype, 'bodyUsed').set`)
    error = assert(typeof actual.get, 'function', `typeof Reflect.getOwnPropertyDescriptor(ObjectStoreEntry.prototype, 'bodyUsed').get`)
    if (error) { return error }

    error = assert(typeof ObjectStoreEntry.prototype.constructor, 'function', `typeof ObjectStoreEntry.prototype.constructor`)
    if (error) { return error }
    error = assert(typeof ObjectStoreEntry.prototype.text, 'function', `typeof ObjectStoreEntry.prototype.text`)
    if (error) { return error }
    error = assert(typeof ObjectStoreEntry.prototype.json, 'function', `typeof ObjectStoreEntry.prototype.json`)
    if (error) { return error }
    error = assert(typeof ObjectStoreEntry.prototype.arrayBuffer, 'function', `typeof ObjectStoreEntry.prototype.arrayBuffer`)
    if (error) { return error }

    actual = Reflect.getOwnPropertyDescriptor(ObjectStoreEntry.prototype.constructor, 'length')
    expected = {
        "value": 0,
        "writable": false,
        "enumerable": false,
        "configurable": true
    }
    error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(ObjectStoreEntry.prototype.constructor, 'length')`)
    if (error) { return error }

    actual = Reflect.getOwnPropertyDescriptor(ObjectStoreEntry.prototype.constructor, 'name')
    expected = {
        "value": "ObjectStoreEntry",
        "writable": false,
        "enumerable": false,
        "configurable": true
    }
    error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(ObjectStoreEntry.prototype.constructor, 'name')`)
    if (error) { return error }

    actual = Reflect.getOwnPropertyDescriptor(ObjectStoreEntry.prototype.text, 'length')
    expected = {
        "value": 0,
        "writable": false,
        "enumerable": false,
        "configurable": true
    }
    error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(ObjectStoreEntry.prototype.text, 'length')`)
    if (error) { return error }

    actual = Reflect.getOwnPropertyDescriptor(ObjectStoreEntry.prototype.text, 'name')
    expected = {
        "value": "text",
        "writable": false,
        "enumerable": false,
        "configurable": true
    }
    error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(ObjectStoreEntry.prototype.text, 'name')`)
    if (error) { return error }

    actual = Reflect.getOwnPropertyDescriptor(ObjectStoreEntry.prototype.json, 'length')
    expected = {
        "value": 0,
        "writable": false,
        "enumerable": false,
        "configurable": true
    }
    error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(ObjectStoreEntry.prototype.json, 'length')`)
    if (error) { return error }

    actual = Reflect.getOwnPropertyDescriptor(ObjectStoreEntry.prototype.json, 'name')
    expected = {
        "value": "json",
        "writable": false,
        "enumerable": false,
        "configurable": true
    }
    error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(ObjectStoreEntry.prototype.json, 'name')`)
    if (error) { return error }

    actual = Reflect.getOwnPropertyDescriptor(ObjectStoreEntry.prototype.arrayBuffer, 'length')
    expected = {
        "value": 0,
        "writable": false,
        "enumerable": false,
        "configurable": true
    }
    error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(ObjectStoreEntry.prototype.arrayBuffer, 'length')`)
    if (error) { return error }

    actual = Reflect.getOwnPropertyDescriptor(ObjectStoreEntry.prototype.arrayBuffer, 'name')
    expected = {
        "value": "arrayBuffer",
        "writable": false,
        "enumerable": false,
        "configurable": true
    }
    error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(ObjectStoreEntry.prototype.arrayBuffer, 'name')`)
    if (error) { return error }

    return pass()
}

async function objectStoreInterfaceTests() {
    let actual = Reflect.ownKeys(ObjectStore)
    let expected = ["prototype", "length", "name"]
    let error = assert(actual, expected, `Reflect.ownKeys(ObjectStore)`)
    if (error) { return error }

    actual = Reflect.getOwnPropertyDescriptor(ObjectStore, 'prototype')
    expected = {
        "value": ObjectStore.prototype,
        "writable": false,
        "enumerable": false,
        "configurable": false
    }
    error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(ObjectStore, 'prototype')`)
    if (error) { return error }

    actual = Reflect.getOwnPropertyDescriptor(ObjectStore, 'length')
    expected = {
        "value": 1,
        "writable": false,
        "enumerable": false,
        "configurable": true
    }
    error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(ObjectStore, 'length')`)
    if (error) { return error }

    actual = Reflect.getOwnPropertyDescriptor(ObjectStore, 'name')
    expected = {
        "value": "ObjectStore",
        "writable": false,
        "enumerable": false,
        "configurable": true
    }
    error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(ObjectStore, 'name')`)
    if (error) { return error }

    actual = Reflect.ownKeys(ObjectStore.prototype)
    expected = ["constructor", "get", "put"]
    error = assert(actual, expected, `Reflect.ownKeys(ObjectStore.prototype)`)
    if (error) { return error }

    actual = Reflect.getOwnPropertyDescriptor(ObjectStore.prototype, 'constructor')
    expected = { "writable": true, "enumerable": false, "configurable": true, value: ObjectStore.prototype.constructor }
    error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(ObjectStore.prototype, 'constructor')`)
    if (error) { return error }

    actual = Reflect.getOwnPropertyDescriptor(ObjectStore.prototype, 'get')
    expected = { "writable": true, "enumerable": true, "configurable": true, value: ObjectStore.prototype.get }
    error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(ObjectStore.prototype, 'get')`)
    if (error) { return error }

    actual = Reflect.getOwnPropertyDescriptor(ObjectStore.prototype, 'put')
    expected = { "writable": true, "enumerable": true, "configurable": true, value: ObjectStore.prototype.put }
    error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(ObjectStore.prototype, 'put')`)
    if (error) { return error }

    error = assert(typeof ObjectStore.prototype.constructor, 'function', `typeof ObjectStore.prototype.constructor`)
    if (error) { return error }
    error = assert(typeof ObjectStore.prototype.get, 'function', `typeof ObjectStore.prototype.get`)
    if (error) { return error }
    error = assert(typeof ObjectStore.prototype.put, 'function', `typeof ObjectStore.prototype.put`)
    if (error) { return error }

    actual = Reflect.getOwnPropertyDescriptor(ObjectStore.prototype.constructor, 'length')
    expected = {
        "value": 1,
        "writable": false,
        "enumerable": false,
        "configurable": true
    }
    error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(ObjectStore.prototype.constructor, 'length')`)
    if (error) { return error }

    actual = Reflect.getOwnPropertyDescriptor(ObjectStore.prototype.constructor, 'name')
    expected = {
        "value": "ObjectStore",
        "writable": false,
        "enumerable": false,
        "configurable": true
    }
    error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(ObjectStore.prototype.constructor, 'name')`)
    if (error) { return error }

    actual = Reflect.getOwnPropertyDescriptor(ObjectStore.prototype.get, 'length')
    expected = {
        "value": 1,
        "writable": false,
        "enumerable": false,
        "configurable": true
    }
    error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(ObjectStore.prototype.get, 'length')`)
    if (error) { return error }

    actual = Reflect.getOwnPropertyDescriptor(ObjectStore.prototype.get, 'name')
    expected = {
        "value": "get",
        "writable": false,
        "enumerable": false,
        "configurable": true
    }
    error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(ObjectStore.prototype.get, 'name')`)
    if (error) { return error }

    actual = Reflect.getOwnPropertyDescriptor(ObjectStore.prototype.put, 'length')
    expected = {
        "value": 1,
        "writable": false,
        "enumerable": false,
        "configurable": true
    }
    error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(ObjectStore.prototype.put, 'length')`)
    if (error) { return error }

    actual = Reflect.getOwnPropertyDescriptor(ObjectStore.prototype.put, 'name')
    expected = {
        "value": "put",
        "writable": false,
        "enumerable": false,
        "configurable": true
    }
    error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(ObjectStore.prototype.put, 'name')`)
    if (error) { return error }

    return pass()
}

function createValidStore() {
    return new ObjectStore('example-test-object-store')
}

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

// TODO: Implement ReadableStream getIterator() and [@@asyncIterator]() methods
async function streamToString(stream) {
    const decoder = new TextDecoder();
    let string = '';
    let reader = stream.getReader()
    // eslint-disable-next-line no-constant-condition
    while (true) {
        const { done, value } = await reader.read();
        if (done) {
            return string;
        }
        string += decoder.decode(value)
    }
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

async function assertResolves(func) {
    try {
        await func()
    } catch (error) {
        return fail(`Expected \`${func.toString()}\` to resolve - Found it rejected: ${error.name}: ${error.message}`)
    }
}

async function assertRejects(func, errorClass, errorMessage) {
    try {
        await func()
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
