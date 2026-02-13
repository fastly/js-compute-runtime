/* eslint-env serviceworker */

// These are tests for the dreaded proxy transform stream optimization: https://github.com/fastly/js-compute-runtime/issues/1260

import { assert, assertThrows } from './assertions.js';
import { routes } from './routes.js';

routes.set('/proxy-transform-stream/content-length', async (event) => {
  let request = new Request(
    'https://http-me.fastly.dev/anything',
    event.request,
  );
  let proxy = new Request(request);
  const response = await fetch(proxy);
  assert(response.status, 200, 'status == 200');
  assert((await response.json()).headers['content-length'], '11', 'headers["content-length"] == 11');
  return new Response('ok');
});

routes.set(
  '/proxy-transform-stream/simple-response-body-chain',
  async (event) => {
    const res = new Response('body');
    return new Response(res.body, res);
  },
);

routes.set(
  '/proxy-transform-stream/multi-response-body-chain',
  async (event) => {
    let res = new Response('body');
    res = new Response(res.body, res);
    res = new Response(res.body, res);
    return res;
  },
);

routes.set(
  '/proxy-transform-stream/response-body-into-js',
  async (event) => {
    let res = new Response('body');
    const body = await res.text();
    return new Response(body, res);
  },
);

routes.set('/proxy-transform-stream/framing', async (event) => {
  const newUrl = new URL('https://http-me.fastly.dev/anything');
  let req = new Request(newUrl, {
    headers: event.request.headers,
    body: event.request.body,
    method: event.request.method,
  });
  req = new Request(req.url, {
    headers: req.headers,
    body: req.body,
    method: req.method,
  });
  req = new Request(req.url, {
    headers: req.headers,
    body: req.body,
    method: req.method,
  });
  const cacheOverride = new CacheOverride('pass');
  const res = await fetch(req, {
    backend: 'httpme',
    cacheOverride,
  });
  let json = await res.json();
  assert(res.status, 200, 'Status should be 200');
  assert(
    json['headers']['content-length'],
    '11',
    'Content-Length header should be 11',
  );
  return new Response('ok');
});
