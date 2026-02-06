/* eslint-env serviceworker */

// These are tests for the dreaded proxy transform stream optimization: https://github.com/fastly/js-compute-runtime/issues/1260

import { assert, assertThrows } from './assertions.js';
import { routes } from './routes.js';

routes.set('/proxy-transform-stream/content-length/backend', async (event) => {
    assert(event.request.headers.get('content-length'), '11', 'Content-Length header should be 11');
    return new Response('ok');
});

routes.set('/proxy-transform-stream/content-length', async (event) => {
    let request = new Request('/proxy-transform-stream/content-length/backend', event.request);
    let proxy = new Request(request);
    const response = await fetch(proxy);
    assert(response.status, 200, 'Response status should be 200');
    return new Response('ok');
});

routes.set('/proxy-transform-stream/simple-response-body-chain/backend', async (event) => {
    const res = new Response("body");
    return new Response(res.body, res);
});

routes.set('/proxy-transform-stream/simple-response-body-chain', async (event) => {
    const req = new Request('/proxy-transform-stream/simple-response-body-chain/backend');
    const res = await fetch(req);
    assert(res.status, 200, 'Status should be 200');
    assert(res.headers.get('content-length'), '4', 'Should use Content-Length instead of Transfer-Encoding');
    return new Response('ok');
});

routes.set('/proxy-transform-stream/multi-response-body-chain/backend', async (event) => {
    let res = new Response("body");
    res = new Response(res.body, res);
    return new Response(res.body, res);
});

routes.set('/proxy-transform-stream/multi-response-body-chain', async (event) => {
    const req = new Request('/proxy-transform-stream/multi-response-body-chain/backend');
    const res = await fetch(req);
    assert(res.status, 200, 'Status should be 200');
    assert(res.headers.get('content-length'), '4', 'Should use Content-Length instead of Transfer-Encoding');
    return new Response('ok');
});

routes.set('/proxy-transform-stream/response-body-into-js/backend', async (event) => {
    let res = new Response("body");
    const body = await res.text();
    return new Response(body, res);
});

routes.set('/proxy-transform-stream/response-body-into-js', async (event) => {
    const req = new Request('/proxy-transform-stream/response-body-into-js/backend');
    const res = await fetch(req);
    assert(res.status, 200, 'Status should be 200');
    assert(res.headers.get('content-length'), '4', 'Should use Content-Length instead of Transfer-Encoding');
    return new Response('ok');
});
