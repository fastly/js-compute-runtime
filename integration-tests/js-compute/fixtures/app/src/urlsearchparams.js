/* eslint-env serviceworker */
import { assert } from './assertions.js';
import { routes } from './routes.js';

routes.set('/urlsearchparams/sort', async () => {
  const urlObj = new URL('http://www.example.com');
  urlObj.searchParams.sort();
  assert(urlObj.toString(), 'http://www.example.com/', `urlObj.toString()`);
});

// A regression test for a premature free that occurred when the URL associated
// with a URLSearchParams object went out of scope. This would fail at high GC zeal.
routes.set('/urlsearchparams/temp-url', async () => {
  const searchParams = (() => {
    return new URL('http://www.example.com?a=b').searchParams;
  })();

  new URL('http://www.do-some-allocations.com').searchParams;
  new URL('http://www.to-force-the-above-url-to-be-gced.com').searchParams;
  new URL('http://www.to-force-the-above-url-to-be-gced.com').searchParams;
  new URL('http://www.to-force-the-above-url-to-be-gced.com').searchParams;
  new URL('http://www.to-force-the-above-url-to-be-gced.com').searchParams;
  new URL('http://www.to-force-the-above-url-to-be-gced.com').searchParams;
  new URL('http://www.to-force-the-above-url-to-be-gced.com').searchParams;
  new URL('http://www.to-force-the-above-url-to-be-gced.com').searchParams;
  new URL('http://www.to-force-the-above-url-to-be-gced.com').searchParams;
  new URL('http://www.to-force-the-above-url-to-be-gced.com').searchParams;
  new URL('http://www.to-force-the-above-url-to-be-gced.com').searchParams;
  new URL('http://www.to-force-the-above-url-to-be-gced.com').searchParams;
  new URL('http://www.to-force-the-above-url-to-be-gced.com').searchParams;

  assert(searchParams.get('a'), 'b', "searchParams.get('a') == 'b'");
});
