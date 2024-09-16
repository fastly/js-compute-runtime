/* eslint-env serviceworker */
import { assert, assertThrows } from './assertions.js';
import { routes } from './routes.js';

// Request setCacheKey method
{
  routes.set('/request/setCacheKey/called-as-constructor', () => {
    assertThrows(
      () => {
        new Request.prototype.setCacheKey('1', '1');
      },
      TypeError,
      `Request.prototype.setCacheKey is not a constructor`,
    );
  });
  routes.set('/request/setCacheKey/called-unbound', () => {
    assertThrows(() => {
      Request.prototype.setCacheKey.call(undefined, '1', '2');
    }, TypeError);
  });
  // https://tc39.es/ecma262/#sec-tostring
  routes.set('/request/setCacheKey/key-parameter-calls-7.1.17-ToString', () => {
    let sentinel;
    const test = () => {
      sentinel = Symbol();
      const key = {
        toString() {
          throw sentinel;
        },
      };
      const request = new Request('https://www.fastly.com');
      request.setCacheKey(key);
    };
    assertThrows(test);
    try {
      test();
    } catch (thrownError) {
      assert(thrownError, sentinel, 'thrownError === sentinel');
    }
    assertThrows(
      () => {
        const request = new Request('https://www.fastly.com');
        request.setCacheKey(Symbol());
      },
      Error,
      `can't convert symbol to string`,
    );
  });
  routes.set('/request/setCacheKey/key-parameter-not-supplied', () => {
    assertThrows(
      () => {
        const request = new Request('https://www.fastly.com');
        request.setCacheKey();
      },
      TypeError,
      `setCacheKey: At least 1 argument required, but only 0 passed`,
    );
  });
  routes.set('/request/setCacheKey/key-valid', () => {
    const request = new Request('https://www.fastly.com');
    request.setCacheKey('meow');
    assert(
      request.headers.get('fastly-xqd-cache-key'),
      '404CDD7BC109C432F8CC2443B45BCFE95980F5107215C645236E577929AC3E52',
      `request.headers.get('fastly-xqd-cache-key'`,
    );
  });
  routes.set('/request/constructor/cacheKey', () => {
    const request = new Request('https://www.fastly.com', { cacheKey: 'meow' });
    assert(
      request.headers.get('fastly-xqd-cache-key'),
      '404CDD7BC109C432F8CC2443B45BCFE95980F5107215C645236E577929AC3E52',
      `request.headers.get('fastly-xqd-cache-key'`,
    );
  });
}
