import { CacheOverride } from 'fastly:cache-override';
import { assertRejects, assertThrows } from './assertions.js';

addEventListener('fetch', (event) => {
  const path = new URL(event.request.url).pathname;
  if (!path.startsWith('/fetch-promise-handoff/')) {
    return;
  }

  const testCase = path.slice('/fetch-promise-handoff/'.length);
  if (testCase === 'respondwith-success') {
    const response = fetch('https://http-me.fastly.dev/anything', {
      backend: 'httpme',
    });
    setResponseHeader(response, 'x-respondwith', 'success', 'response');
    event.respondWith(response);
    event.waitUntil(
      response.then(() =>
        assertThrows(() =>
          setResponseHeader(response, 'x-respondwith-late', 'ignored'),
        ),
      ),
    );
    return;
  }
  if (testCase === 'respondwith-rejection') {
    const response = fetch('http://[', { backend: 'httpme' });
    setResponseHeader(response, 'x-respondwith-error', 'rejected', 'error');
    event.respondWith(response);
    event.waitUntil(
      response.catch(() =>
        assertThrows(() =>
          setResponseHeader(response, 'x-respondwith-late', 'ignored'),
        ),
      ),
    );
    return;
  }
  if (testCase === 'validation-errors') {
    for (const helper of [
      setResponseHeader,
      appendResponseHeader,
      removeResponseHeader,
    ]) {
      assertThrows(() => helper());
      assertThrows(() => helper({}, 'x-test', 'value'));
      assertThrows(() =>
        helper(Promise.resolve(new Response()), 'x-test', 'value'),
      );
    }
    assertThrows(() => event.handoffRequest(new Response()));
    const response = fetch('https://http-me.fastly.dev/status/200', {
      backend: 'httpme',
    });
    assertThrows(() =>
      setResponseHeader(response, 'x-test', 'value', 'invalid'),
    );
    assertThrows(() =>
      appendResponseHeader(response, 'x-test', 'value', 'invalid'),
    );
    assertThrows(() => removeResponseHeader(response, 'x-test', 'invalid'));
    assertThrows(() =>
      setResponseHeader(
        response.then((value) => value),
        'x-test',
        'value',
      ),
    );
    event.handoffRequest(response);
    assertThrows(() => setResponseHeader(response, 'x-test', 'value'));
    assertThrows(() => appendResponseHeader(response, 'x-test', 'value'));
    assertThrows(() => removeResponseHeader(response, 'x-test'));
    assertThrows(() => event.handoffRequest(response));
    return;
  }
  const isUploadError =
    testCase.endsWith('stream-error') || testCase === 'upload-rejection';
  const isRejected = testCase === 'rejected-fetch';
  const isHttpError = testCase === 'http-error';
  const isCacheFill = testCase === 'guest-cache-fill';
  const isPost = !isCacheFill;
  const isStreamed = testCase.endsWith('streamed') || isUploadError;
  const body = isStreamed
    ? new ReadableStream({
        start(controller) {
          const encoder = new TextEncoder();
          controller.enqueue(encoder.encode('stream-'));
          setTimeout(() => {
            if (isUploadError) {
              controller.error(new Error('upload failed'));
              return;
            }
            controller.enqueue(encoder.encode('body'));
            controller.close();
          }, 25);
        },
      })
    : isPost
      ? 'buffered-body'
      : undefined;
  const url = isRejected
    ? 'http://['
    : isHttpError
      ? 'https://http-me.fastly.dev/now?status=500'
      : `https://http-me.fastly.dev/anything/${Math.random()}`;
  const response = fetch(url, {
    backend: 'httpme',
    method: isPost ? 'POST' : 'GET',
    body,
    ...(isCacheFill
      ? { cacheOverride: new CacheOverride('override', { ttl: 60 }) }
      : {}),
  });

  if (testCase === 'upload-rejection') {
    event.respondWith(
      assertRejects(() => response, Error, 'upload failed').then(
        () => new Response('ok'),
      ),
    );
    return;
  }

  setResponseHeader(response, 'x-promise-append', 'first', 'response');
  appendResponseHeader(response, 'x-promise-append', 'second', 'response');
  setResponseHeader(response, 'x-promise-set', 'set-value', 'response');
  removeResponseHeader(response, 'access-control-allow-origin', 'response');
  setResponseHeader(response, 'x-promise-error-only', 'error-value', 'error');
  setResponseHeader(response, 'x-promise-any', 'any-value');
  if (isUploadError || isRejected) {
    setResponseHeader(response, 'x-error-append', 'first', 'error');
    appendResponseHeader(response, 'x-error-append', 'second', 'error');
    setResponseHeader(response, 'x-error-remove', 'removed');
    removeResponseHeader(response, 'x-error-remove', 'error');
  }
  event.handoffRequest(response);
});
