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
routes.set('/request/clone/headers-are-independent', () => {
  const request = new Request('https://www.fastly.com', {
    headers: { 'x-foo': 'original' },
    method: 'get',
  });
  const cloned = request.clone();

  // Mutating the clone's headers must not affect the original
  cloned.headers.set('x-foo', 'mutated');
  assert(request.headers.get('x-foo'), 'original', 'original header unchanged after mutating clone');

  // Mutating the original's headers must not affect the clone
  request.headers.set('x-bar', 'added');
  assert(cloned.headers.get('x-bar'), null, 'clone does not see header added to original');

  // Clone must carry the headers that existed at clone time
  assert(cloned.headers.get('x-foo'), 'mutated', 'clone has header value set on it');
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
