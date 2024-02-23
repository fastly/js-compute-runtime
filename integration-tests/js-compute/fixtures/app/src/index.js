/// <reference path="../../../../../types/index.d.ts" />
/* eslint-env serviceworker */

import { routes } from "./routes.js";
import { env } from 'fastly:env';
import { fail } from "./assertions.js";

import "./async-select.js"
import "./btoa.js"
import "./byob.js"
import "./byte-repeater.js"
import "./cache-override.js"
import "./cache-core.js"
import "./cache-simple.js"
import "./client.js"
import "./config-store.js"
import "./console.js"
import "./crypto.js"
import "./dictionary.js"
import "./dynamic-backend.js"
import "./env.js"
import "./fanout.js"
import "./fastly-now.js"
import "./fetch-errors.js"
import "./geoip.js"
import "./headers.js"
import "./include-bytes.js"
import "./kv-store.js"
import "./logger.js"
import "./manual-framing-headers.js"
import "./missing-backend.js"
import "./multiple-set-cookie.js"
import "./performance.js"
import "./random.js"
import "./react-byob.js"
import "./request-auto-decompress.js"
import "./request-cache-key.js"
import "./request-clone.js"
import "./request-headers.js"
import "./response-json.js"
import "./response-redirect.js"
import "./response.js"
import "./secret-store.js"
import "./tee.js"
import "./timers.js"
import "./transform-stream.js"
import "./urlsearchparams.js"


addEventListener("fetch", event => {
    event.respondWith(app(event))
})

/**
 * @param {FetchEvent} event
 * @returns {Response}
*/
async function app(event) {
    const FASTLY_SERVICE_VERSION = env('FASTLY_SERVICE_VERSION') || 'local'
    console.log(`FASTLY_SERVICE_VERSION: ${FASTLY_SERVICE_VERSION}`)
    const path = (new URL(event.request.url)).pathname
    console.log(`path: ${path}`)
    let res = new Response('Internal Server Error', { status: 500 });
    try {
        const routeHandler = routes.get(path)
        if (routeHandler) {
            res = await routeHandler(event)
        } else {
            res = fail(`${path} endpoint does not exist`)
        }
    } catch (error) {
        res = fail(`The routeHandler for ${path} threw an error: ${error.message || error}` + '\n' + error.stack)
    } finally {
        res.headers.set('fastly_service_version', FASTLY_SERVICE_VERSION);
    }

    return res;
}