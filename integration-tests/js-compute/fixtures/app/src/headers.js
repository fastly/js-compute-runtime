/* eslint-env serviceworker */

import { routes } from './routes.js';
import { assert } from './assertions.js';
import { CacheOverride } from 'fastly:cache-override';

routes.set('/headers/construct', async () => {
  const headers = new Headers();
  headers.set('foo', 'bar');
  return new Response('check headers', { headers });
});

routes.set('/headers/getsetcookie', async () => {
  let response = await fetch(
    'https://http-me.fastly.dev/meow?header=Set-Cookie:name1=value1',
    {
      backend: 'httpme',
    },
  );
  response.headers.append('Set-Cookie', 'name2=value2');
  console.log(
    'response.headers.getSetCookie()',
    response.headers.getSetCookie(),
  );
});

routes.set('/headers/from-response/set', async () => {
  const response = await fetch('https://http-me.fastly.dev/anything', {
    backend: 'httpme',
    cacheOverride: new CacheOverride('pass'),
  });
  response.headers.set('cuStom', 'test');
  return response;
});

routes.set('/headers/from-response/delete-invalid', async () => {
  const response = await fetch('https://http-me.fastly.dev/anything', {
    backend: 'httpme',
    cacheOverride: new CacheOverride('pass'),
  });
  response.headers.delete('none');
  return response;
});

routes.set('/headers/from-response/set-delete', async () => {
  const response = await fetch('https://http-me.fastly.dev/anything', {
    backend: 'httpme',
    cacheOverride: new CacheOverride('pass'),
  });
  response.headers.set('custom', 'test');
  response.headers.delete('access-control-allow-origin');
  return response;
});
