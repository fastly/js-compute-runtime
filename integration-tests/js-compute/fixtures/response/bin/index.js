/* eslint-env serviceworker */
/* global ReadableStream */
import { env } from 'fastly:env';
addEventListener("fetch", event => {
    event.respondWith(app(event))
})
/**
 * @param {FetchEvent} event
 * @returns {Response}
 */
async function app(event) {
    try {
        const path = (new URL(event.request.url)).pathname
        console.log(`path: ${path}`)
        console.log(`FASTLY_SERVICE_VERSION: ${env('FASTLY_SERVICE_VERSION')}`)
        if (routes.has(path)) {
            const routeHandler = routes.get(path)
            return await routeHandler()
        }
        return fail(`${path} endpoint does not exist`)
    } catch (error) {
        return fail(`The routeHandler threw an error: ${error.message}` + '\n' + error.stack)
    }
}

const routes = new Map()
routes.set('/', () => {
    routes.delete('/')
    let test_routes = Array.from(routes.keys())
    return new Response(JSON.stringify(test_routes), { 'headers': { 'content-type': 'application/json' } })
})
routes.set("/response/text/guest-backed-stream", async () => {
    let contents = new Array(10).fill(new Uint8Array(500).fill(65))
    contents.push(new Uint8Array([0, 66]))
    contents.push(new Uint8Array([1,1,2,65]))
    let res = new Response(iteratableToStream(contents))
    let text = await res.text()

    let error = assert(text, "A".repeat(5000) + '\x00B\x01\x01\x02A', `await res.text() === "a".repeat(5000)`)
    if (error) { return error }
    return pass()
})
routes.set("/response/json/guest-backed-stream", async () => {
    let obj = {a:1,b:2,c:{d:3}}
    let encoder = new TextEncoder()
    let contents = encoder.encode(JSON.stringify(obj))
    let res = new Response(iteratableToStream([contents]))
    let json = await res.json()

    let error = assert(json, obj, `await res.json() === obj`)
    if (error) { return error }
    return pass()
})
routes.set("/response/arrayBuffer/guest-backed-stream", async () => {
    let obj = {a:1,b:2,c:{d:3}}
    let encoder = new TextEncoder()
    let contents = encoder.encode(JSON.stringify(obj))
    let res = new Response(iteratableToStream([contents]))
    let json = await res.arrayBuffer()

    let error = assert(json, contents.buffer, `await res.json() === contents.buffer`)
    if (error) { return error }
    return pass()
})

function iteratableToStream(iterable) {
    return new ReadableStream({
        async pull(controller) {
            for await (const value of iterable) {
                controller.enqueue(value)
            }
            controller.close()
        }
    })
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
* var bool = deepEqual( [ 1, 2, 3 ], [ 1, 2, 3 ] )
* // returns true
*
* @example
* var bool = deepEqual( [ 1, 2, 3 ], [ 1, 2, '3' ] )
* // returns false
*
* @example
* var bool = deepEqual( { 'a': 2 }, { 'a': [ 2 ] } )
* // returns false
*
* @example
* var bool = deepEqual( [], {} )
* // returns false
*
* @example
* var bool = deepEqual( null, null )
* // returns true
*/
function deepEqual(a, b) {
    var aKeys
    var bKeys
    var typeA
    var typeB
    var key
    var i

    typeA = typeof a
    typeB = typeof b
    if (a === null || typeA !== 'object') {
        if (b === null || typeB !== 'object') {
            return a === b
        }
        return false
    }
    // Case: `a` is of type 'object'
    if (typeB !== 'object') {
        return false
    }
    if (Object.getPrototypeOf(a) !== Object.getPrototypeOf(b)) {
        return false
    }
    if (a instanceof Date) {
        return a.getTime() === b.getTime()
    }
    if (a instanceof RegExp) {
        return a.source === b.source && a.flags === b.flags
    }
    if (a instanceof Error) {
        if (a.message !== b.message || a.name !== b.name) {
            return false
        }
    }

    aKeys = Object.keys(a)
    bKeys = Object.keys(b)
    if (aKeys.length !== bKeys.length) {
        return false
    }
    aKeys.sort()
    bKeys.sort()

    // Cheap key test:
    for (i = 0; i < aKeys.length; i++) {
        if (aKeys[i] !== bKeys[i]) {
            return false
        }
    }
    // Possibly expensive deep equality test for each corresponding key:
    for (i = 0; i < aKeys.length; i++) {
        key = aKeys[i]
        if (!deepEqual(a[key], b[key])) {
            return false
        }
    }
    return typeA === typeB
}
