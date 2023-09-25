/* eslint-env serviceworker */
import { env } from 'fastly:env';
import { routes, isRunningLocally } from "./routes.js";
import { pass, assert } from './assertions';

routes.set("/env", () => {
  if (isRunningLocally()) {
    let error = assert(env("FASTLY_HOSTNAME"), "localhost", `env("FASTLY_HOSTNAME") === "localhost"`)
    if (error) { return error; }
  }
  return pass()
});
