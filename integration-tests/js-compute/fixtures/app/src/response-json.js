/* eslint-env serviceworker */
import { assert, assertThrows } from './assertions.js';
import { routes } from './routes.js';

routes.set('/response/json', async () => {
  const APPLICATION_JSON = 'application/json';
  const FOO_BAR = 'foo/bar';

  const INIT_TESTS = [
    [undefined, 200, '', APPLICATION_JSON, {}],
    [{ status: 400 }, 400, '', APPLICATION_JSON, {}],
    [{ statusText: 'foo' }, 200, 'foo', APPLICATION_JSON, {}],
    [{ headers: {} }, 200, '', APPLICATION_JSON, {}],
    [{ headers: { 'content-type': FOO_BAR } }, 200, '', FOO_BAR, {}],
    [
      { headers: { 'x-foo': 'bar' } },
      200,
      '',
      APPLICATION_JSON,
      { 'x-foo': 'bar' },
    ],
  ];

  for (const [
    init,
    expectedStatus,
    expectedStatusText,
    expectedContentType,
    expectedHeaders,
  ] of INIT_TESTS) {
    const response = Response.json('hello world', init);
    assert(response.type, 'default', 'response.type');
    assert(response.status, expectedStatus, 'response.status');
    assert(response.statusText, expectedStatusText, 'response.statusText');
    assert(
      response.headers.get('content-type'),
      expectedContentType,
      'response.headers.get("content-type")',
    );
    for (const key in expectedHeaders) {
      assert(
        response.headers.get(key),
        expectedHeaders[key],
        'response.headers.get(key)',
      );
    }
    const data = await response.json();
    assert(data, 'hello world', 'data');
  }

  const nullBodyStatus = [204, 205, 304];
  for (const status of nullBodyStatus) {
    assertThrows(function () {
      Response.json('hello world', { status: status });
    }, TypeError);
  }

  const response = Response.json({ foo: 'bar' });
  const data = await response.json();
  assert(typeof data, 'object', 'typeof data');
  assert(data.foo, 'bar', 'data.foo');

  assertThrows(function () {
    Response.json(Symbol('foo'));
  }, TypeError);

  const a = { b: 1 };
  a.a = a;
  assertThrows(function () {
    Response.json(a);
  }, TypeError);

  class CustomError extends Error {
    name = 'CustomError';
  }
  assertThrows(function () {
    Response.json({
      get foo() {
        throw new CustomError('bar');
      },
    });
  }, CustomError);
});
