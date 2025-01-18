/* eslint-env serviceworker */

import { routes } from './routes.js';
import { assert } from './assertions.js';
import { CacheOverride } from 'fastly:cache-override';

routes.set('/headers/construct', async () => {
  const headers = new Headers();
  headers.set('foo', 'bar');
  return new Response('check headers', { headers });
});

routes.set('/headers/non-ascii-latin1-field-value', async () => {
  let response = await fetch('https://http-me.glitch.me/meow?header=cat:é', {
    backend: 'httpme',
  });

  let text = response.headers.get('cat');
  console.log("response.headers.get('cat')", response.headers.get('cat'));

  assert(text, 'é', `response.headers.get('cat') === "é"`);
});

routes.set('/headers/from-response/set', async () => {
  const response = await fetch('https://httpbin.org/stream-bytes/11', {
    backend: 'httpbin',
    cacheOverride: new CacheOverride('pass'),
  });
  return new Response(response.body, {
    headers: {
      cuStom: 'test',
    },
  });
});

routes.set('/headers/from-response/delete-invalid', async () => {
  const response = await fetch('https://httpbin.org/stream-bytes/11', {
    backend: 'httpbin',
    cacheOverride: new CacheOverride('pass'),
  });
  const outResponse = new Response(response.body, response);
  outResponse.headers.delete('none');
  return outResponse;
});

routes.set('/headers/from-response/set-delete', async () => {
  const response = await fetch('https://httpbin.org/stream-bytes/11', {
    backend: 'httpbin',
    cacheOverride: new CacheOverride('pass'),
  });
  const outResponse = new Response(response.body, response);
  outResponse.headers.set('custom', 'test');
  outResponse.headers.set('another', 'test');
  outResponse.headers.delete('access-control-allow-origin');
  return outResponse;
});
