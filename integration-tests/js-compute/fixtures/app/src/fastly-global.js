/// <reference path="../../../../../types/index.d.ts" />
/* eslint-env serviceworker */

import { assert } from './assertions.js';
import { routes } from './routes.js';
import { sdkVersion } from 'fastly:experimental';

routes.set('/fastly/now', function () {
  assert(typeof fastly.now, 'function', 'typeof fastly.now');

  assert(fastly.now.name, 'now', 'fastly.now.name');

  assert(fastly.now.length, 0, 'fastly.now.length');

  assert(typeof fastly.now(), 'number', `typeof fastly.now()`);

  assert(fastly.now() > Date.now(), true, `fastly.now() > Date.now()`);
});

routes.set('/fastly/version', function () {
  assert(typeof fastly.sdkVersion, 'string', 'typeof fastly.sdkVersion');

  assert(
    fastly.sdkVersion,
    sdkVersion,
    'fastly.sdkVersion matches fastly:experimental#sdkVersion',
  );
});
