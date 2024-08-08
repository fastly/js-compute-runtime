/* eslint-env serviceworker */

import { pass, assert } from "./assertions.js";
import { routes } from "./routes.js";

routes.set("/request/method/host", (event) => {
    return new Response('', {
        headers: {
            result: event.request.method
        }
    })
});

routes.set("/request/method/guest", () => {
    const methods = [
        ["get", "GET"],
        ["GET", "GET"],
        ["GeT", "GET"],
        ["head", "HEAD"],
        ["HEAD", "HEAD"],
        ["HeaD", "HEAD"],
        ["options", "OPTIONS"],
        ["OPTIONS", "OPTIONS"],
        ["OPtIOnS", "OPTIONS"],
        ["post", "POST"],
        ["POST", "POST"],
        ["pOSt", "POST"],
        ["put", "PUT"],
        ["PUT", "PUT"],
        ["Put", "PUT"],
        ["delete", "DELETE"],
        ["DELETE", "DELETE"],
        ["DELete", "DELETE"],
        ["hello", "hello"],
        ["HELLO", "HELLO"],
        ["HEllO", "HEllO"],
        // TODO: Add # $ % & when the platform supports them
        ["!*+-.^_`|~1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ", "!*+-.^_`|~1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"],
    ];
    for (const [method, expected] of methods) {
        const result = new Request('http://a.a', { method: method }).method;
        const error = assert(result, expected, `new Request('http://a.a', {method: "${method}"}).method`)
        if (error) { return error }
    }
    return pass('ok')
});
