/* eslint-env serviceworker */
import { env } from 'fastly:env';
import { routes, isRunningLocally } from './routes.js';
import { assert } from './assertions.js';

routes.set('/env', () => {
  if (isRunningLocally()) {
    assert(
      env('FASTLY_HOSTNAME'),
      'localhost',
      `env("FASTLY_HOSTNAME") === "localhost"`,
    );
  }
});
