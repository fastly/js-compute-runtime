import { env } from 'fastly:env';
import { pass, fail, assert } from "../../../assertions.js";

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
