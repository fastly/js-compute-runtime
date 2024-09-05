/// <reference path="../../../../../types/index.d.ts" />
/* eslint-env serviceworker */

import { routes } from "./routes.js";
import { env } from "fastly:env";
import { fail } from "./assertions.js";
import "./hello-world.js";

// TLA
await Promise.resolve();

addEventListener("fetch", (event) => {
  event.respondWith(app(event));
});

/**
 * @param {FetchEvent} event
 * @returns {Response}
 */
async function app(event) {
  const FASTLY_SERVICE_VERSION = env("FASTLY_SERVICE_VERSION") || "local";
  console.log(`FASTLY_SERVICE_VERSION: ${FASTLY_SERVICE_VERSION}`);
  const path = new URL(event.request.url).pathname;
  console.log(`path: ${path}`);
  let res = new Response("Internal Server Error", { status: 500 });
  try {
    const routeHandler = routes.get(path);
    if (routeHandler) {
      res = await routeHandler(event);
    } else {
      res = fail(`${path} endpoint does not exist`);
    }
  } catch (error) {
    if (error instanceof Response) {
      res = error;
    } else {
      res = fail(
        `The routeHandler for ${path} threw an error: ${error.message || error}` +
          "\n" +
          error.stack,
      );
    }
  } finally {
    res.headers.set("fastly_service_version", FASTLY_SERVICE_VERSION);
  }

  return res;
}
