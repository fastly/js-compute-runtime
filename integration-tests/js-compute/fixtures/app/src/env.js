/* eslint-env serviceworker */
import { env } from 'fastly:env';
import { routes, isRunningLocally } from './routes.js';
import { strictEqual } from './assertions.js';

// hostname didn't exist at initialization, so can still be captured at runtime
const wizerHostname = env('FASTLY_HOSTNAME');
const wizerLocal = env('LOCAL_TEST');

routes.set('/env', () => {
  strictEqual(wizerHostname, undefined);

  if (isRunningLocally()) {
    strictEqual(
      env('FASTLY_HOSTNAME'),
      'localhost',
      `env("FASTLY_HOSTNAME") === "localhost"`,
    );
  } else {
    strictEqual(env('FASTLY_HOSTNAME'), undefined);
  }

  strictEqual(wizerLocal, 'local val');

  // at runtime these remain captured from Wizer time, even if we didn't call env
  strictEqual(env('LOCAL_TEST'), 'local val');
  strictEqual(env('TEST'), 'foo');
});
