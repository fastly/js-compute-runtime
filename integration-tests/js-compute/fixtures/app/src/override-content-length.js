/// <reference path="../../../../../types/index.d.ts" />
/* eslint-env serviceworker */

import { pass, assert, assertRejects } from "./assertions.js";
import { routes, isRunningLocally } from "./routes.js";

let error;

async function requestInitObjectLiteral(overrideContentLength) {
    let request = new Request('https://http-me.glitch.me/anything', {
        backend: 'httpme',
        method: 'POST',
        body: 'meow',
        overrideContentLength,
        headers: {
            "content-length": "1"
        }
    });
    let response = await fetch(request);
    let body = await response.json()
    return body?.headers?.["content-length"];
}

async function requestClone(overrideContentLength) {
    let request = new Request('https://http-me.glitch.me/anything', {
        backend: 'httpme',
        method: 'POST',
        body: 'meow',
        overrideContentLength,
        headers: {
            "content-length": "1"
        }
    });
    let response = await fetch(request.clone());
    let body = await response.json()
    return body?.headers?.["content-length"];
}

async function fetchInitObjectLiteral(overrideContentLength) {
    let response = await fetch('https://http-me.glitch.me/anything', {
        backend: 'httpme',
        method: 'POST',
        body: 'meow',
        overrideContentLength,
        headers: {
            "content-length": "1"
        }
    });
    let body = await response.json()
    return body?.headers?.["content-length"];
}

routes.set("/override-content-length/request/init/object-literal/true", async () => {
    if (isRunningLocally()) {
        error = await assertRejects(() => requestInitObjectLiteral(true))
        if (error) { return error }
    } else {
        let actual = await requestInitObjectLiteral(true);
        let expected = "1"
        error = assert(actual, expected, `await requestInitObjectLiteral(true)`)
        if (error) { return error }
    }

    return pass('ok')
});

routes.set("/override-content-length/request/init/object-literal/false", async () => {
    let actual = await requestInitObjectLiteral(false);
    let expected = "4"
    error = assert(actual, expected, `await requestInitObjectLiteral(false)`)
    if (error) { return error }

    return pass('ok')
});

routes.set("/override-content-length/request/clone/true", async () => {
    if (isRunningLocally()) {
        error = await assertRejects(() => requestClone(true))
        if (error) { return error }
    } else {
        let actual = await requestClone(true);
        let expected = "1"
        error = assert(actual, expected, `await requestClone(true)`)
        if (error) { return error }
    }

    return pass('ok')
});

routes.set("/override-content-length/request/clone/false", async () => {
    let actual = await requestClone(false);
    let expected = "4"
    error = assert(actual, expected, `await requestClone(false)`)
    if (error) { return error }

    return pass('ok')
});

routes.set("/override-content-length/fetch/init/object-literal/true", async () => {
    if (isRunningLocally()) {
        error = await assertRejects(() => fetchInitObjectLiteral(true))
        if (error) { return error }
    } else {
        let actual = await fetchInitObjectLiteral(true);
        let expected = "1"
        error = assert(actual, expected, `await fetchInitObjectLiteral(true)`)
        if (error) { return error }
    }

    return pass('ok')
});

routes.set("/override-content-length/fetch/init/object-literal/false", async () => {
    let actual = await fetchInitObjectLiteral(false);
    let expected = "4"
    error = assert(actual, expected, `await fetchInitObjectLiteral(false)`)
    if (error) { return error }

    return pass('ok')
});

async function responseInitObjectLiteral(overrideContentLength) {
    let response = new Response('meow', {
        overrideContentLength,
        headers: {
            "transfer-encoding": "chunked"
        }
    });
    return response;
}

async function responseInitresponseInstance(overrideContentLength) {
    let response = new Response('meow', {
        overrideContentLength,
        headers: {
            "transfer-encoding": "chunked"
        }
    });
    response = new Response('meow', response);
    return response;
}

routes.set("/override-content-length/response/init/object-literal/true", async () => {
    return responseInitObjectLiteral(true);
});

routes.set("/override-content-length/response/init/object-literal/false", async () => {
    return responseInitObjectLiteral(false);
});

routes.set("/override-content-length/response/init/response-instance/true", async () => {
    return responseInitresponseInstance(true);
});

routes.set("/override-content-length/response/init/response-instance/false", async () => {
    return responseInitresponseInstance(false);
});


// TODO: uncomment when we have an implementation of Response.prototype.clone
// async function responseClone(overrideContentLength) {
//     let response = new Response('meow', {
//         backend: 'httpme',
//         method: 'POST',
//         body: 'meow',
//         overrideContentLength,
//         headers: {
//             "transfer-encoding": "chunked"
//         }
//     });
//     response = response.clone();
//     return response;
// }

// routes.set("/override-content-length/response/clone/true", async () => {
//     return responseClone(true);
// });

// routes.set("/override-content-length/response/clone/false", async () => {
//     return responseClone(false);
// });