/* eslint-env serviceworker */

import { assert } from './assertions.js';
import { routes } from './routes.js';

// Regression test: new Request(url, { body: other_request.body }) loses the body
// when the source request's body was created from a UTF-8 string.
routes.set('/request/body/body-from-string-passthrough', async () => {
  const source = new Request('https://example.com', {
    method: 'POST',
    body: 'hello world',
  });

  // This was the broken path: source.body returned null because the request
  // was not downstream, so the new request silently had no body.
  const forwarded = new Request('https://example.com', {
    method: 'POST',
    body: source.body,
  });

  assert(
    forwarded.body instanceof ReadableStream,
    true,
    'forwarded.body instanceof ReadableStream',
  );

  const text = await forwarded.text();
  assert(text, 'hello world', 'forwarded body text matches original');
});

routes.set('/request/body/body-property-non-downstream', () => {
  const req = new Request('https://example.com', {
    method: 'POST',
    body: 'test body',
  });

  // .body should always return a ReadableStream for a request with a body,
  // regardless of whether the request is downstream (from the client) or not.
  assert(req.body instanceof ReadableStream, true, 'req.body instanceof ReadableStream');
  assert(req.bodyUsed, false, 'req.bodyUsed is false before reading');
});

routes.set('/request/body/body-from-arraybuffer-passthrough', async () => {
  const encoder = new TextEncoder();
  const bytes = encoder.encode('binary body');

  const source = new Request('https://example.com', {
    method: 'POST',
    body: bytes.buffer,
  });

  const forwarded = new Request('https://example.com', {
    method: 'POST',
    body: source.body,
  });

  const text = await forwarded.text();
  assert(text, 'binary body', 'forwarded ArrayBuffer body text matches original');
});
