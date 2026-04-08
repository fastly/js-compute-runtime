/// <reference path="../../../../../types/index.d.ts" />
/* eslint-env serviceworker */

import { CacheOverride } from 'fastly:cache-override';
import { assert, assertThrows, strictEqual } from './assertions.js';
import { isRunningLocally, routes } from './routes.js';

// CacheOverride staleIfError property
{
  routes.set(
    '/stale-if-error/cache-override/constructor-with-staleIfError',
    async () => {
      const override = new CacheOverride('override', { staleIfError: 300 });
      assert(
        override.staleIfError,
        300,
        `new CacheOverride('override', { staleIfError: 300 }).staleIfError === 300`,
      );
    },
  );

  routes.set(
    '/stale-if-error/cache-override/constructor-without-staleIfError',
    async () => {
      const override = new CacheOverride('override', { ttl: 300 });
      assert(
        override.staleIfError,
        undefined,
        `new CacheOverride('override', { ttl: 300 }).staleIfError === undefined`,
      );
    },
  );

  routes.set('/stale-if-error/cache-override/set-staleIfError', async () => {
    const override = new CacheOverride('override', {});
    override.staleIfError = 600;
    assert(
      override.staleIfError,
      600,
      `Setting override.staleIfError = 600 works correctly`,
    );
  });
}

// Response staleIfError property
{
  routes.set(
    '/stale-if-error/response/property-undefined-on-non-cached',
    async () => {
      const response = new Response('test body', {
        status: 200,
        headers: { 'Content-Type': 'text/plain' },
      });
      assert(
        response.staleIfError,
        undefined,
        `Non-cached response.staleIfError === undefined`,
      );
    },
  );

  routes.set(
    '/stale-if-error/response/setter-throws-on-non-cached',
    async () => {
      const response = new Response('test body');
      assertThrows(
        () => {
          response.staleIfError = 300;
        },
        TypeError,
        'Response set: staleIfError must be set only on unsent cache transaction responses',
      );
    },
  );

  routes.set(
    '/stale-if-error/response/staleIfErrorAvailable-throws-outside-afterSend',
    async () => {
      const response = new Response('test body');
      assertThrows(
        () => {
          response.staleIfErrorAvailable();
        },
        TypeError,
        'Response: staleIfErrorAvailable() must can only be called on candidate responses inside afterSend callback',
      );
    },
  );
}

// Integration tests with fetch
routes.set('/stale-if-error/fetch/with-cache-override', async () => {
  const url = 'https://http-me.fastly.dev/now?stale-if-error-test-1';

  // First request: populate cache with staleIfError
  const response1 = await fetch(url, {
    backend: 'httpme',
    cacheOverride: new CacheOverride('override', {
      ttl: 300,
      staleIfError: 600,
    }),
  });

  assert(
    response1.staleIfError,
    600,
    `First response has staleIfError === 600`,
  );
  assert(typeof response1.ttl, 'number', `First response has numeric ttl`);
});

routes.set(
  '/stale-if-error/fetch/staleIfErrorAvailable-after-caching',
  async () => {
    const sharedCacheKey = 'stale-if-error-available-test-' + Date.now();

    // Step 1: Prime the cache with a response that has staleIfError
    const primeRequest = new Request('https://http-me.fastly.dev/now', {
      backend: 'httpme',
      cacheOverride: new CacheOverride('override', {
        ttl: 1, // 1 second TTL - will be stale quickly
        staleIfError: 600, // Long stale-if-error window
      }),
    });
    primeRequest.setCacheKey(sharedCacheKey);
    await fetch(primeRequest);

    // Step 2: Wait for the cached response to go stale
    await new Promise((resolve) => setTimeout(resolve, 1500));

    // Step 3: Make a second request with same cache key
    // There's now a stale cached response with staleIfError available
    let staleIfErrorAvailableResult;
    let staleIfErrorValue;

    const checkRequest = new Request('https://http-me.fastly.dev/now', {
      backend: 'httpme',
      cacheOverride: new CacheOverride('override', {
        staleIfError: 600,
        afterSend(candidateResponse) {
          // Check staleIfErrorAvailable() on the candidate response
          staleIfErrorAvailableResult =
            candidateResponse.staleIfErrorAvailable();
          staleIfErrorValue = candidateResponse.staleIfError;
        },
      }),
    });
    checkRequest.setCacheKey(sharedCacheKey);
    await fetch(checkRequest);

    assert(
      staleIfErrorAvailableResult,
      true,
      `staleIfErrorAvailable() returns true when stale cached response with staleIfError exists`,
    );
    assert(
      staleIfErrorValue,
      600,
      `Cached response preserves staleIfError === 600`,
    );
  },
);

routes.set(
  '/stale-if-error/fetch/staleIfErrorAvailable-false-without-staleIfError',
  async () => {
    const url = `https://http-me.fastly.dev/now?stale-if-error-test-no-sie-${Date.now()}`;

    let staleIfErrorAvailableResult;

    await fetch(url, {
      backend: 'httpme',
      cacheOverride: new CacheOverride('override', {
        ttl: 10,
        // No staleIfError configured
        afterSend(candidateResponse) {
          // Check staleIfErrorAvailable() on the candidate response
          staleIfErrorAvailableResult =
            candidateResponse.staleIfErrorAvailable();
        },
      }),
    });

    assert(
      staleIfErrorAvailableResult,
      false,
      `staleIfErrorAvailable() returns false when staleIfError is not configured`,
    );
  },
);

routes.set('/stale-if-error/fetch/with-swr-and-staleIfError', async () => {
  const url = 'https://http-me.fastly.dev/now?stale-if-error-test-4';

  const response = await fetch(url, {
    backend: 'httpme',
    cacheOverride: new CacheOverride('override', {
      ttl: 60,
      swr: 300,
      staleIfError: 600,
    }),
  });

  assert(response.ttl, 60, `response.ttl === 60`);
  assert(response.swr, 300, `response.swr === 300`);
  assert(response.staleIfError, 600, `response.staleIfError === 600`);
});

routes.set('/stale-if-error/fetch/zero-staleIfError', async () => {
  const url = 'https://http-me.fastly.dev/now?stale-if-error-test-5';

  const response = await fetch(url, {
    backend: 'httpme',
    cacheOverride: new CacheOverride('override', {
      ttl: 300,
      staleIfError: 0,
    }),
  });

  assert(
    response.staleIfError,
    0,
    `response.staleIfError === 0 when explicitly set to zero`,
  );
});

// Use cache key override to make different URLs share cache
// This tests the core stale-if-error functionality:
// 1. Cache a 200 response with short TTL and long stale-if-error window
// 2. Wait for TTL to expire (response becomes stale)
// 3. Request a URL that returns 503, but with the SAME cache key
// 4. Verify the stale 200 response is served instead of the 503
routes.set(
  '/stale-if-error/fetch/serve-stale-on-backend-error-with-cache-key',
  async () => {
    const sharedCacheKey = 'stale-if-error-test-shared-key-' + Date.now();

    // Step 1: Cache a successful response with short TTL and long stale-if-error
    const goodRequest = new Request(
      'https://http-me.fastly.dev/now?status=200',
      {
        backend: 'httpme',
        cacheOverride: new CacheOverride('override', {
          ttl: 1, // 1 second TTL - will be stale quickly
          staleIfError: 3600, // 1 hour stale-if-error window
        }),
      },
    );
    goodRequest.setCacheKey(sharedCacheKey);

    const goodResponse = await fetch(goodRequest);
    assert(goodResponse.status, 200, 'Initial response is 200');
    const initialBody = await goodResponse.text();

    // Step 2: Wait for TTL to expire (make response stale)
    await new Promise((resolve) => setTimeout(resolve, 1500));

    // Step 3: Request with same cache key but URL that returns 503
    const errorRequest = new Request('https://http-me.fastly.dev/status=503', {
      backend: 'httpme',
      cacheOverride: new CacheOverride('override', {
        staleIfError: 3600,
      }),
    });
    errorRequest.setCacheKey(sharedCacheKey);

    const staleResponse = await fetch(errorRequest);

    // Step 4: Verify we got the stale 200 response, not the 503
    assert(
      staleResponse.status,
      200,
      'Stale-if-error serves cached 200 response instead of backend 503',
    );

    const cachedBody = await staleResponse.text();
    strictEqual(
      cachedBody,
      initialBody,
      'Response body is from the original cached 200 response',
    );
  },
);
