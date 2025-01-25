import { CacheOverride } from 'fastly:cache-override';
import { assert, assertThrows, assertDoesNotThrow } from './assertions.js';
import { isRunningLocally, routes } from './routes.js';

// CacheOverride constructor
{
  routes.set(
    '/cache-override/constructor/called-as-regular-function',
    async () => {
      assertThrows(
        () => {
          CacheOverride();
        },
        TypeError,
        `calling a builtin CacheOverride constructor without new is forbidden`,
      );
    },
  );
  routes.set('/cache-override/constructor/empty-parameter', async () => {
    assertThrows(
      () => {
        new CacheOverride();
      },
      TypeError,
      `CacheOverride constructor: At least 1 argument required, but only 0 passed`,
    );
  });
  routes.set('/cache-override/constructor/invalid-mode', async () => {
    // empty string not allowed
    assertThrows(
      () => {
        new CacheOverride('');
      },
      TypeError,
      `CacheOverride constructor: 'mode' has to be "none", "pass", or "override", but got ""`,
    );

    assertThrows(
      () => {
        new CacheOverride('be nice to the cache');
      },
      TypeError,
      `CacheOverride constructor: 'mode' has to be "none", "pass", or "override", but got "be nice to the cache"`,
    );
  });
  routes.set('/cache-override/constructor/valid-mode', async () => {
    assertDoesNotThrow(() => {
      new CacheOverride('none');
    });
    assertDoesNotThrow(() => {
      new CacheOverride('pass');
    });
    assertDoesNotThrow(() => {
      new CacheOverride('override', {});
    });
    assertDoesNotThrow(() => {
      new CacheOverride({});
    });
  });
}
// Using CacheOverride
{
  routes.set('/cache-override/fetch/mode-none', async () => {
    if (isRunningLocally()) return;
    {
      const response = await fetch('https://http-me.glitch.me/now?status=200', {
        backend: 'httpme',
        cacheOverride: new CacheOverride('none'),
      });
      assert(
        response.headers.has('x-cache'),
        true,
        `CacheOveride('none'); response.headers.has('x-cache') === true`,
      );
    }

    {
      const response = await fetch('https://http-me.glitch.me/now?status=200', {
        backend: 'httpme',
        cacheOverride: 'none',
      });
      assert(
        response.headers.has('x-cache'),
        true,
        `CacheOveride('none'); response.headers.has('x-cache') === true`,
      );
    }
  });
  routes.set('/cache-override/fetch/mode-pass', async () => {
    if (isRunningLocally()) return;

    {
      const response = await fetch('https://http-me.glitch.me/now?status=200', {
        backend: 'httpme',
        cacheOverride: new CacheOverride('pass'),
      });
      assert(
        response.headers.has('x-cache'),
        false,
        `CacheOveride('pass'); response.headers.has('x-cache') === false`,
      );
    }

    {
      const response = await fetch('https://http-me.glitch.me/now?status=200', {
        backend: 'httpme',
        cacheOverride: 'pass',
      });
      assert(
        response.headers.has('x-cache'),
        false,
        `CacheOveride('pass'); response.headers.has('x-cache') === false`,
      );
    }
  });
}
