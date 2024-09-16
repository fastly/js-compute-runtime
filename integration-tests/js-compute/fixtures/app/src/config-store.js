/* eslint-env serviceworker */
import { assert } from './assertions.js';
import { ConfigStore } from 'fastly:config-store';
import { routes } from './routes.js';

routes.set('/config-store', () => {
  let config = new ConfigStore('testconfig');
  let twitterValue = config.get('twitter');
  assert(
    twitterValue,
    'https://twitter.com/fastly',
    `config.get("twitter") === "https://twitter.com/fastly"`,
  );
});
