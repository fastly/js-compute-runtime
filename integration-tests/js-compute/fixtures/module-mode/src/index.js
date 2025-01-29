/// <reference path="../../../../../types/index.d.ts" />
/* eslint-env serviceworker */

import { routes } from './routes.js';
import { env } from 'fastly:env';
import { enableDebugLogging } from 'fastly:experimental';

import './console.js';
import './dynamic-backend.js';
import './hello-world.js';
import './hono.js';
import './http-cache.js';
import './kv-store.js';
import './transform-stream.js';

addEventListener('fetch', (event) => {
  const responsePromise = app(event);
  if (responsePromise) event.respondWith(responsePromise);
});

if (env('FASTLY_DEBUG_LOGGING') === '1') {
  if (fastly.debugMessages) {
    const { debug: consoleDebug } = console;
    console.debug = function debug(...args) {
      fastly.debugMessages.push(...args);
      consoleDebug(...args);
    };
  }
  enableDebugLogging(true);
}

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
      res = await routeHandler(event);
      if (res !== null) {
        res = res || new Response('ok');
      }
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
            error.stack +
            (fastly.debugMessages
              ? '\n[DEBUG BUILD MESSAGES]:\n\n  - ' +
                fastly.debugMessages.join('\n  - ')
              : ''),
          { status: 500 },
        ));
      } catch (errRes) {
        res = errRes;
      }
    }
  } finally {
    res.headers.set('fastly_service_version', FASTLY_SERVICE_VERSION);
  }

  return res;
}
