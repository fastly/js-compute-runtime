/* eslint-env serviceworker */

import { assert, assertThrows } from './assertions.js';
import { routes } from './routes.js';

routes.set('/request/clone/called-as-constructor', () => {
  assertThrows(
    () => {
      new Request.prototype.clone();
    },
    TypeError,
    `Request.prototype.clone is not a constructor`,
  );
});
routes.set('/request/clone/called-unbound', () => {
  assertThrows(() => {
    Request.prototype.clone.call(undefined);
  }, TypeError);
});
routes.set('/request/clone/valid', async () => {
  let request = new Request('https://www.fastly.com', {
    headers: {
      hello: 'world',
    },
    body: 'te',
    method: 'post',
  });
  let newRequest = request.clone();
  assert(newRequest instanceof Request, true, 'newRequest instanceof Request');
  assert(newRequest.method, request.method, 'newRequest.method');
  assert(newRequest.url, request.url, 'newRequest.url');
  assert(newRequest.headers, request.headers, 'newRequest.headers');
  assert(request.bodyUsed, false, 'request.bodyUsed');
  assert(newRequest.bodyUsed, false, 'newRequest.bodyUsed');
  assert(
    newRequest.body instanceof ReadableStream,
    true,
    'newRequest.body instanceof ReadableStream',
  );

  request = new Request('https://www.fastly.com', {
    method: 'get',
  });
  newRequest = request.clone();

  assert(newRequest.bodyUsed, false, 'newRequest.bodyUsed');
  assert(newRequest.body, null, 'newRequest.body');
});
routes.set('/request/clone/invalid', async () => {
  const request = new Request('https://www.fastly.com', {
    headers: {
      hello: 'world',
    },
    body: 'te',
    method: 'post',
  });
  await request.text();
  assertThrows(() => request.clone());
});
