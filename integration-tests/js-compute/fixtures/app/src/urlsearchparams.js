/* eslint-env serviceworker */
import { assert } from './assertions.js';
import { routes } from './routes.js';

routes.set('/urlsearchparams/sort', async () => {
  const urlObj = new URL('http://www.example.com');
  urlObj.searchParams.sort();
  assert(urlObj.toString(), 'http://www.example.com/', `urlObj.toString()`);
});
