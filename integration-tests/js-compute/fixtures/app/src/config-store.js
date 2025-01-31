/// <reference path="../../../../../types/index.d.ts" />

/* eslint-env serviceworker */
import { assert } from './assertions.js';
import { ConfigStore } from 'fastly:config-store';
import { routes } from './routes.js';
import { env } from 'fastly:env';

const CONFIG_STORE_NAME = env('CONFIG_STORE_NAME');

routes.set('/config-store', () => {
  let config = new ConfigStore(CONFIG_STORE_NAME);
  let twitterValue = config.get('twitter');
  assert(
    twitterValue,
    'https://twitter.com/fastly',
    `config.get("twitter") === "https://twitter.com/fastly"`,
  );
});
