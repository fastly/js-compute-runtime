/// <reference path="../../../../../types/index.d.ts" />

/* eslint-env serviceworker */
import { assert } from './assertions.js';
import { ConfigStore } from 'fastly:config-store';
import { routes } from './routes.js';
import { env } from 'fastly:env';

const FASTLY_SERVICE_NAME = env('FASTLY_SERVICE_NAME');
const CONFIG_STORE_NAME = `testconfig${FASTLY_SERVICE_NAME ? `__${FASTLY_SERVICE_NAME.replace(/-/g, '_')}` : ''}`;

routes.set('/config-store', () => {
  let config = new ConfigStore(CONFIG_STORE_NAME);
  let twitterValue = config.get('twitter');
  assert(
    twitterValue,
    'https://twitter.com/fastly',
    `config.get("twitter") === "https://twitter.com/fastly"`,
  );
});
