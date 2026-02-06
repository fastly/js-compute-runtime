/// <reference path="../../../../../types/index.d.ts" />
/* eslint-env serviceworker */

import { routes } from './routes.js';
import { env } from 'fastly:env';
import { enableDebugLogging } from 'fastly:experimental';

import './async-select.js';
import './btoa.js';
import './byob.js';
import './byte-repeater.js';
import './cache-override.js';
import './cache-core.js';
import './cache-simple.js';
import './client.js';
import './compute.js';
import './config-store.js';
import './crypto.js';
import './device.js';
import './dictionary.js';
import './early-hints.js';
import './edge-rate-limiter.js';
import './env.js';
import './fanout.js';
import './websocket.js';
import './fastly-global.js';
import './fetch-errors.js';
import './geoip.js';
import './headers.js';
import './html-rewriter.js';
import './include-bytes.js';
import './image-optimizer.js';
import './logger.js';
import './manual-framing-headers.js';
import './missing-backend.js';
import './multiple-set-cookie.js';
import './performance.js';
import './random.js';
import './react-byob.js';
import './request-auto-decompress.js';
import './request-body-async-simple.js';
import './request-cache-key.js';
import './request-clone.js';
import './request-headers.js';
import './request-method.js';
import './response-json.js';
import './response-redirect.js';
import './response.js';
import './secret-store.js';
import './security.js';
import './server.js';
import './shielding.js';
import './tee.js';
import './timers.js';
import './urlsearchparams.js';

addEventListener('fetch', (event) => {
  event.respondWith(app(event));
});

if (env('FASTLY_DEBUG_LOGGING') === '1') {
  if (fastly.debugMessages) {
    const { debug: consoleDebug } = console;
    console.debug = function debug(...args) {
      fastly.debugLog(...args);
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
      res = (await routeHandler(event)) || new Response('ok');
    } else {
      try {
        return (res = new Response(`${path} endpoint does not exist`, {
          status: 500,
        }));
      } catch (errRes) {
        res = errRes;
      }
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
