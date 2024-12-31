/// <reference path="../../../../../types/index.d.ts" />
/* eslint-env serviceworker */

import { routes } from './routes.js';
import { env } from 'fastly:env';

import './console.js';
import './dynamic-backend.js';
import './hello-world.js';
import './hono.js';
import './http-cache.js';
import './kv-store.js';

addEventListener('fetch', (event) => {
  event.respondWith(app(event));
});

/**
 * @param {FetchEvent} event
 * @returns {Response}
 */
async function app(event) {
  const FASTLY_SERVICE_VERSION = env('FASTLY_SERVICE_VERSION') || 'local';
  console.log(`FASTLY_SERVICE_VERSION: ${FASTLY_SERVICE_VERSION}`);
  const path = new URL(event.request.url).pathname;
  console.log(`path: ${path}`);
  let res;
  try {
    const routeHandler = routes.get(path);
    if (routeHandler) {
      res = (await routeHandler(event)) || new Response('ok');
    } else {
      return (res = new Response(`${path} endpoint does not exist`, {
        status: 500,
      }));
    }
  } catch (error) {
    if (error instanceof Response) {
      res = error;
    } else {
      try {
        return (res = new Response(
          `The routeHandler for ${path} threw a [${error.constructor?.name ?? error.name}] error: ${error.message || error}` +
            '\n' +
            error.stack,
          { status: 500 },
        ));
      } catch (errRes) {
        console.error('err2', errRes);
        res = errRes;
      }
    }
  } finally {
    res.headers.set('fastly_service_version', FASTLY_SERVICE_VERSION);
  }

  return res;
}
