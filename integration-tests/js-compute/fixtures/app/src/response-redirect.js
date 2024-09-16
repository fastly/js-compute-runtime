/* eslint-env serviceworker */
import { assert, assertThrows } from './assertions.js';
import { routes } from './routes.js';

routes.set('/response/redirect', async () => {
  const url = 'http://test.url:1234/';
  const redirectResponse = Response.redirect(url);
  assert(redirectResponse.type, 'default');
  assert(redirectResponse.redirected, false);
  assert(redirectResponse.ok, false);
  assert(redirectResponse.status, 302, 'Default redirect status is 302');
  assert(redirectResponse.headers.get('Location'), url);
  assert(redirectResponse.statusText, '');

  for (const status of [301, 302, 303, 307, 308]) {
    const redirectResponse = Response.redirect(url, status);
    assert(redirectResponse.type, 'default');
    assert(redirectResponse.redirected, false);
    assert(redirectResponse.ok, false);
    assert(redirectResponse.status, status, 'Redirect status is ' + status);
    assert(redirectResponse.headers.get('Location'), url);
    assert(redirectResponse.statusText, '');
  }
  const invalidUrl = 'http://:This is not an url';
  assertThrows(function () {
    Response.redirect(invalidUrl);
  }, TypeError);
  for (const invalidStatus of [200, 309, 400, 500]) {
    assertThrows(function () {
      Response.redirect(url, invalidStatus);
    }, RangeError);
  }
});
