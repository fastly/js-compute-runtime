/* eslint-env serviceworker */

/**
 * Simplified test for body loss across async boundary
 * Based on Canva's bug report: https://github.com/paulina-canva/body-bug-repro
 */

import { assert } from './assertions.js';
import { routes } from './routes.js';

// Test: Body lost across async boundary (BUG)
routes.set('/request/body-async-simple/no-workaround', async (event) => {
  const req = event.request;
  const originalLength = parseInt(req.headers.get('content-length') || '0', 10);

  console.log(`Original content-length: ${originalLength}`);

  // Create new request with body stream
  const forwardRequest = new Request('http://example.com/dummy', {
    method: 'POST',
    headers: req.headers,
    body: req.body,  // Pass the ReadableStream
  });

  // Async operations (this is where the bug happens)
  await Promise.resolve();
  await Promise.resolve();

  // Now try to read the body
  const bodyText = await forwardRequest.text();
  const receivedLength = bodyText.length;

  console.log(`Body after async: ${receivedLength} bytes (expected ${originalLength})`);

  if (receivedLength === 0 && originalLength > 0) {
    console.log(`Body was lost! Expected ${originalLength} bytes, got 0`);
    return new Response(JSON.stringify({
      bug: true,
      expected: originalLength,
      received: receivedLength,
      message: 'Body lost across async boundary'
    }), {
      status: 500,
      headers: { 'Content-Type': 'application/json' }
    });
  }

  assert(receivedLength, originalLength, 'Body should not be lost');
  return new Response('ok');
});

// Test: Body preserved with workaround
routes.set('/request/body-async-simple/with-workaround', async (event) => {
  const req = event.request;
  const originalLength = parseInt(req.headers.get('content-length') || '0', 10);

  console.log(`Original content-length (workaround): ${originalLength}`);

  // WORKAROUND: Call clone to force buffering
  const _dummy = await req.clone();

  const forwardRequest = new Request('http://example.com/dummy', {
    method: 'POST',
    headers: req.headers,
    body: req.body,
  });

  // Same async operations
  await Promise.resolve();
  await Promise.resolve();

  const bodyText = await forwardRequest.text();
  const receivedLength = bodyText.length;

  console.log(`Body with workaround: ${receivedLength} bytes`);

  assert(receivedLength, originalLength, 'Body preserved with workaround');
  return new Response('ok');
});
