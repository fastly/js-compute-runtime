/// <reference path="../../../../../types/index.d.ts" />
/* eslint-env serviceworker */

import { pass, assert, assertRejects } from "./assertions.js";
import { routes, isRunningLocally } from "./routes.js";

let error;

async function requestInitObjectLiteral(overrideContentLength) {
    let response = await fetch(new Request('https://httpbin.org/headers', {
        backend: 'httpbin',
        overrideContentLength,
        headers: {
            "Content-Length": "1"
        }
    }));
    let body = await response.json()
    return body?.headers?.["Content-Length"];
}

async function requestInitRequestInstance(overrideContentLength) {
    let request = new Request('https://httpbin.org/headers', {
        backend: 'httpbin',
        overrideContentLength,
        headers: {
            "Content-Length": "1"
        }
    });
    let response = await fetch(new Request('https://httpbin.org/headers', request));
    let body = await response.json()
    return body?.headers?.["Content-Length"];
}

async function requestClone(overrideContentLength) {
    let request = new Request('https://httpbin.org/headers', {
        backend: 'httpbin',
        overrideContentLength,
        headers: {
            "Content-Length": "1"
        }
    });
    let response = await fetch(request.clone());
    let body = await response.json()
    return body?.headers?.["Content-Length"];
}

async function fetchInitObjectLiteral(overrideContentLength) {
    let response = await fetch('https://httpbin.org/headers', {
        backend: 'httpbin',
        overrideContentLength,
        headers: {
            "Content-Length": "1"
        }
    });
    let body = await response.json()
    return body?.headers?.["Content-Length"];
}

async function fetchInitRequestInstance(overrideContentLength) {
    let request = new Request('https://httpbin.org/headers', {
        backend: 'httpbin',
        overrideContentLength,
        headers: {
            "Content-Length": "1"
        }
    });
    let response = await fetch('https://httpbin.org/headers', request);
    let body = await response.json()
    return body?.headers?.["Content-Length"];
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
    let expected = "0"
    error = assert(actual, expected, `await requestInitObjectLiteral(false)`)
    if (error) { return error }

    return pass('ok')
});

routes.set("/override-content-length/request/init/request-instance/true", async () => {
    if (isRunningLocally()) {
        error = await assertRejects(() => requestInitRequestInstance(true))
        if (error) { return error }
    } else {
        let actual = await requestInitRequestInstance(true);
        let expected = "1"
        error = assert(actual, expected, `await requestInitRequestInstance(true)`)
        if (error) { return error }
    }

    return pass('ok')
});

routes.set("/override-content-length/request/init/request-instance/false", async () => {
    let actual = await requestInitRequestInstance(false);
    let expected = "0"
    error = assert(actual, expected, `await requestInitRequestInstance(false)`)
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
    let expected = "0"
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
    let expected = "0"
    error = assert(actual, expected, `await fetchInitObjectLiteral(false)`)
    if (error) { return error }

    return pass('ok')
});

routes.set("/override-content-length/fetch/init/request-instance/true", async () => {
    if (isRunningLocally()) {
        error = await assertRejects(() => fetchInitRequestInstance(true))
        if (error) { return error }
    } else {
        let actual = await fetchInitRequestInstance(true);
        let expected = "1"
        error = assert(actual, expected, `await fetchInitRequestInstance(true)`)
        if (error) { return error }
    }

    return pass('ok')
});

routes.set("/override-content-length/fetch/init/request-instance/false", async () => {
    let actual = await fetchInitRequestInstance(false);
    let expected = "0"
    error = assert(actual, expected, `await fetchInitRequestInstance(false)`)
    if (error) { return error }

    return pass('ok')
});