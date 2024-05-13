/// <reference path="../../../../../types/index.d.ts" />
/* eslint-env serviceworker */

import { pass, assert } from "./assertions.js";
import { routes } from "./routes.js";
import { sdkVersion } from "fastly:experimental";

routes.set("/fastly/now", function () {
    let error = assert(typeof fastly.now, 'function', 'typeof fastly.now')
    if (error) { return error }

    error = assert(fastly.now.name, 'now', 'fastly.now.name')
    if (error) { return error }

    error = assert(fastly.now.length, 0, 'fastly.now.length')
    if (error) { return error }

    error = assert(typeof fastly.now(), 'number', `typeof fastly.now()`)
    if (error) { return error }

    error = assert(fastly.now() > Date.now(), true, `fastly.now() > Date.now()`)
    if (error) { return error }

    return pass()
})

routes.set("/fastly/version", function () {
  let error = assert(typeof fastly.sdkVersion, 'string', 'typeof fastly.sdkVersion')
  if (error) { return error }

  error = assert(fastly.sdkVersion, sdkVersion, 'fastly.sdkVersion matches fastly:experimental#sdkVersion')
  if (error) { return error }

  return pass()
})
