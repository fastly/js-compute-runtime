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
  assert(
    req.body instanceof ReadableStream,
    true,
    'req.body instanceof ReadableStream',
  );
  assert(req.bodyUsed, false, 'req.bodyUsed is false before reading');
});

// Per the Fetch spec, body.text() runs UTF-8 decode (replacement mode), so
// invalid byte sequences must be substituted with U+FFFD rather than fail.
routes.set('/request/body/text-replaces-invalid-utf8', async () => {
  // 0x48 0x69 ("Hi"), then 0xC3 0x28 (invalid 2-byte sequence), then 0x21 ("!")
  const bytes = new Uint8Array([0x48, 0x69, 0xc3, 0x28, 0x21]);

  const req = new Request('https://example.com', {
    method: 'POST',
    body: bytes.buffer,
  });

  const text = await req.text();
  assert(text, 'Hi\uFFFD(!', 'invalid UTF-8 is replaced with U+FFFD');
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
  assert(
    text,
    'binary body',
    'forwarded ArrayBuffer body text matches original',
  );
});
