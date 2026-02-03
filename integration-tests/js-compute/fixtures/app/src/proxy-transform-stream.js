/* eslint-env serviceworker */

// These are tests for the dreaded proxy transform stream optimization: https://github.com/fastly/js-compute-runtime/issues/1260

import { assert, assertThrows } from './assertions.js';
import { routes } from './routes.js';

routes.set('/proxy-transform-stream/content-length/backend', async (event) => {
    console.log('Headers:', [...event.request.headers.entries()]);
    assert(event.request.headers.get('content-length'), '11', 'Content-Length header should be 11');
    return new Response('ok');
});

routes.set('/proxy-transform-stream/content-length', async (event) => {
    console.log('Headers:', [...event.request.headers.entries()]);
  let request = new Request('/proxy-transform-stream/content-length/backend', event.request);
  let proxy = new Request(request);
  const response = await fetch(proxy);
  assert(response.status, 200, 'Response status should be 200');
  return new Response('ok');
});
