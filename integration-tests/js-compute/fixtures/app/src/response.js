/* eslint-env serviceworker */

import { routes } from './routes.js';
import { assert, assertThrows } from './assertions.js';
import { allowDynamicBackends } from 'fastly:experimental';

routes.set('/response/stall', async (event) => {
  // keep connection open 10 seconds
  event.waitUntil(new Promise((resolve) => setTimeout(resolve, 10_000)));
  return new Response(
    new ReadableStream({
      start(controller) {
        // stall
      },
    }),
  );
});

routes.set('/response/text/guest-backed-stream', async () => {
  let contents = new Array(10).fill(new Uint8Array(500).fill(65));
  contents.push(new Uint8Array([0, 66]));
  contents.push(new Uint8Array([1, 1, 2, 65]));
  let res = new Response(iteratableToStream(contents));
  let text = await res.text();

  assert(
    text,
    'A'.repeat(5000) + '\x00B\x01\x01\x02A',
    `await res.text() === "a".repeat(5000)`,
  );
});

routes.set('/response/json/guest-backed-stream', async () => {
  let obj = { a: 1, b: 2, c: { d: 3 } };
  let encoder = new TextEncoder();
  let contents = encoder.encode(JSON.stringify(obj));
  let res = new Response(iteratableToStream([contents]));
  let json = await res.json();

  assert(json, obj, `await res.json() === obj`);
});

routes.set('/response/arrayBuffer/guest-backed-stream', async () => {
  let obj = { a: 1, b: 2, c: { d: 3 } };
  let encoder = new TextEncoder();
  let contents = encoder.encode(JSON.stringify(obj));
  let res = new Response(iteratableToStream([contents]));
  let json = await res.arrayBuffer();

  assert(json, contents.buffer, `await res.json() === contents.buffer`);
});

routes.set('/response/ip-port-undefined', async () => {
  let res = new Response();
  assert(res.ip, undefined);
  assert(res.port, undefined);
});

routes.set('/response/request-body-init', async () => {
  allowDynamicBackends(true);
  // fetch an image
  const downloadResp = await fetch('https://httpbin.org/image', {
    headers: {
      accept: 'image/webp',
    },
  });

  assertThrows(() => {
    downloadResp.headers.set('should-be', 'immutable');
  }, TypeError);

  // stream it through an echo proxy
  const postResp = await fetch(
    new Request('https://httpbin.org/anything', {
      method: 'POST',
      body: downloadResp.body,
    }),
  );
  // finally stream back to user
  return postResp;
});

function iteratableToStream(iterable) {
  return new ReadableStream({
    async pull(controller) {
      for await (const value of iterable) {
        controller.enqueue(value);
      }
      controller.close();
    },
  });
}
