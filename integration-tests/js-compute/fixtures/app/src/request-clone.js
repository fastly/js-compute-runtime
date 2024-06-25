/* eslint-env serviceworker */

import { pass, assert, assertThrows } from "./assertions.js";
import { routes } from "./routes.js";

routes.set("/request/clone/called-as-constructor", () => {
    let error = assertThrows(() => {
        new Request.prototype.clone()
    }, TypeError, `Request.prototype.clone is not a constructor`)
    if (error) { return error }
    return pass()
});
routes.set("/request/clone/called-unbound", () => {
    let error = assertThrows(() => {
        Request.prototype.clone.call(undefined)
    }, TypeError)
    if (error) { return error }
    return pass()
});
routes.set("/request/clone/valid", async () => {
    let request = new Request('https://www.fastly.com', {
        headers: {
            hello: 'world'
        },
        body: 'te',
        method: 'post'
    })
    let newRequest = request.clone();
    let error = assert(newRequest instanceof Request, true, 'newRequest instanceof Request')
    if (error) { return error }
    error = assert(newRequest.method, request.method, 'newRequest.method')
    if (error) { return error }
    error = assert(newRequest.url, request.url, 'newRequest.url')
    if (error) { return error }
    error = assert(newRequest.headers, request.headers, 'newRequest.headers')
    if (error) { return error }
    error = assert(request.bodyUsed, false, 'request.bodyUsed')
    if (error) { return error }
    error = assert(newRequest.bodyUsed, false, 'newRequest.bodyUsed')
    if (error) { return error }
    error = assert(newRequest.body instanceof ReadableStream, true, 'newRequest.body instanceof ReadableStream')
    if (error) { return error }

    request = new Request('https://www.fastly.com', {
        method: 'get'
    })
    newRequest = request.clone()

    error = assert(newRequest.bodyUsed, false, 'newRequest.bodyUsed')
    if (error) { return error }
    error = assert(newRequest.body, null, 'newRequest.body')
    if (error) { return error }
    return pass()
});
routes.set("/request/clone/invalid", async () => {
    const request = new Request('https://www.fastly.com', {
        headers: {
            hello: 'world'
        },
        body: 'te',
        method: 'post'
    })
    await request.text()
    let error = assertThrows(() => request.clone())
    if (error) { return error }
    return pass()
});
