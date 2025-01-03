/* eslint-env serviceworker */
import { strictEqual, assertRejects } from './assertions.js';
import { routes } from './routes.js';
import { CacheOverride } from 'fastly:cache-override';

// generate a unique URL everytime so that we never work on a populated cache
const getTestUrl = () =>
  `https://httpbin.org/anything?${Math.random().toString().slice(2)}`;

// Test suite: Error handling
{
  // Test invalid property assignments
  routes.set('/http-cache/invalid-properties', async () => {
    const url = getTestUrl();

    await assertRejects(
      () =>
        fetch(url, {
          cacheOverride: new CacheOverride({
            afterSend(res) {
              res.status = 'invalid'; // Should throw type error
            },
          }),
        }),
      TypeError,
    );

    await assertRejects(
      () =>
        fetch(url, {
          cacheOverride: new CacheOverride({
            afterSend(res) {
              res.ttl = 'invalid'; // Should throw type error
            },
          }),
        }),
      TypeError,
    );
  });

  // Test invalid transform stream
  routes.set('/http-cache/invalid-transform', async () => {
    const url = getTestUrl();

    await assertRejects(
      () =>
        fetch(url, {
          cacheOverride: new CacheOverride({
            afterSend() {
              return {
                bodyTransform: 'not a transform stream',
              };
            },
          }),
        }),
      TypeError,
    );
  });

  routes.set('/http-cache/hook-errors', async () => {
    await assertRejects(
      () =>
        fetch(url, {
          cacheOverride: new CacheOverride({
            beforeSend() {
              throw new Error('before send error');
            },
          }),
        }),
      Error,
      'before send error',
    );

    await assertRejects(
      () =>
        fetch(url, {
          cacheOverride: new CacheOverride({
            afterSend() {
              throw new Error('after send error');
            },
          }),
        }),
      Error,
      'after send error',
    );
  });
}

// Test suite: Response Property Coverage
{
  // Test readonly properties
  routes.set('/http-cache/readonly-properties', async () => {
    const url = getTestUrl();
    const backend = new Backend({
      name: 'test_backend',
      target: 'httpbin.org',
    });

    const cacheOverride = new CacheOverride({
      afterSend(res) {
        res.ttl = 3600;
        return { cache: true };
      },
    });

    // Initial request
    const res1 = await fetch(url, { backend, cacheOverride });
    strictEqual(res1.backend, backend);
    strictEqual(res1.age, 0);

    // Wait a bit and verify age increased
    await new Promise((resolve) => setTimeout(resolve, 1000));
    const res2 = await fetch(url, { backend, cacheOverride });
    strictEqual(res2.age > 0, true);
    strictEqual(res2.backend, backend);

    // Verify readonly properties cannot be modified
    assertRejects(() => {
      res2.cached = false;
    }, TypeError);
    assertRejects(() => {
      res2.isCacheable = false;
    }, TypeError);
    assertRejects(() => {
      res2.isStale = true;
    }, TypeError);
    assertRejects(() => {
      res2.age = 0;
    }, TypeError);
    assertRejects(() => {
      res2.backend = null;
    }, TypeError);
  });
}

// Test suite: Property Error Handling
{
  // Test invalid property assignments
  routes.set('/http-cache/property-errors', async () => {
    const url = getTestUrl();

    // Test invalid swr assignment
    await assertRejects(
      () =>
        fetch(url, {
          cacheOverride: new CacheOverride({
            afterSend(res) {
              res.swr = 'invalid';
            },
          }),
        }),
      TypeError,
    );

    // Test invalid vary assignment
    await assertRejects(
      () =>
        fetch(url, {
          cacheOverride: new CacheOverride({
            afterSend(res) {
              res.vary = ['not-a-set'];
            },
          }),
        }),
      TypeError,
    );

    // Test invalid surrogateKeys assignment
    await assertRejects(
      () =>
        fetch(url, {
          cacheOverride: new CacheOverride({
            afterSend(res) {
              res.surrogateKeys = ['not-a-set'];
            },
          }),
        }),
      TypeError,
    );

    // Test invalid pci assignment
    await assertRejects(
      () =>
        fetch(url, {
          cacheOverride: new CacheOverride({
            afterSend(res) {
              res.pci = 'invalid';
            },
          }),
        }),
      TypeError,
    );

    // Test invalid Set contents for vary
    await assertRejects(
      () =>
        fetch(url, {
          cacheOverride: new CacheOverride({
            afterSend(res) {
              res.vary = new Set([1, 2, 3]); // Should only accept strings
            },
          }),
        }),
      TypeError,
    );

    // Test invalid Set contents for surrogateKeys
    await assertRejects(
      () =>
        fetch(url, {
          cacheOverride: new CacheOverride({
            afterSend(res) {
              res.surrogateKeys = new Set([1, 2, 3]); // Should only accept strings
            },
          }),
        }),
      TypeError,
    );
  });

  // Test property access on invalid states
  routes.set('/http-cache/property-access-errors', async () => {
    const url = getTestUrl();

    // Test accessing cache properties on non-cached response
    const cacheOverride = new CacheOverride({
      afterSend(res) {
        return { cache: false };
      },
    });

    const res = await fetch(url, { cacheOverride });
    strictEqual(res.cached, false);

    // These should all be undefined for non-cached responses
    strictEqual(res.isStale, undefined);
    strictEqual(res.ttl, undefined);
    strictEqual(res.age, undefined);
    strictEqual(res.vary, undefined);
    strictEqual(res.surrogateKeys, undefined);
  });
}

// HTTP cache freshness
{
  routes.set('/http-cache/after-send-edge-cache', async () => {
    let calledAfterSend = false;
    const cacheOverride = new CacheOverride({
      afterSend(res) {
        calledAfterSend = true;
        res.headers.set('Cache-Control', 'private, no-store');
        res.ttl = 3600 - res.age;
        // This is the default, so it is not needed to provide explicitly
        // return { cache: true };
      },
    });
    let res = await fetch(url, { cacheOverride });
    strictEqual(calledAfterSend, true);
    strictEqual(res.headers.get('Cache-Control'), 'private, no-store');
    calledAfterSend = false;
    res = await fetch(url, { cacheOverride });
    strictEqual(calledAfterSend, true);
    strictEqual(res.headers.get('Cache-Control'), 'private, no-store');
  });

  routes.set('/http-cache/after-send-browser-cache', async () => {
    let calledAfterSend = false;
    const cacheOverride = new CacheOverride({
      afterSend(res) {
        calledAfterSend = true;
        res.headers.set('Cache-Control', 'max-age=3600');
        return { cache: false };
      },
    });
    let res = await fetch(url, { cacheOverride });
    strictEqual(calledAfterSend);
    strictEqual(res.headers.get('Cache-Control'), 'max-age=3600');
    res = await fetch(url, { cacheOverride });
    // should be cached
    strictEqual(res.cached, true);
    strictEqual(calledAfterSend, false);
    strictEqual(res.headers.get('Cache-Control'), 'max-age=3600');
  });

  routes.set('/http-cache/after-send-res-no-body-error', async () => {
    let afterSendRes;
    const cacheOverride = new CacheOverride({
      afterSend(res) {
        afterSendRes = res;
      },
    });
    await fetch(url, { cacheOverride });
    strictEqual(typeof afterSendRes, 'object');
    // this should throw -> reading a body of a candidate response is not supported
    // since revalidations have no body
    return res;
  });

  routes.set('/http-cache/before-send', async () => {
    let calledBeforeSend = false;
    const cacheOverride = new CacheOverride({
      beforeSend(req) {
        strictEqual(calledBeforeSend, false);
        calledBeforeSend = true;
        req.headers.set('Foo', 'Baz');
      },
    });
    const headers = {
      Foo: 'Bar',
    };
    const [res1, res2] = await Promise.all([
      fetch(url, { cacheOverride, headers }),
      fetch(url, { cacheOverride, headers }),
    ]);
    strictEqual(res1.cached, true);
    strictEqual(res2.cached, true);
    strictEqual(res1.headers.get('Foo'), 'Baz');
    strictEqual(res2.headers.get('Foo'), 'Baz');
  });
}

// Test suite: Request properties and behaviors
{
  // Test basic request mutation via beforeSend
  routes.set('/http-cache/request-mutation', async () => {
    const url = getTestUrl();
    const cacheOverride = new CacheOverride({
      beforeSend(req) {
        req.headers.set('X-Test', 'modified');
      },
    });

    const res = await fetch(url, { cacheOverride });
    const body = await res.json();
    strictEqual(body.headers['x-test'], 'modified');
  });

  // Test request mutation order with multiple concurrent requests
  routes.set('/http-cache/request-mutation-order', async () => {
    const url = getTestUrl();
    let beforeSendCount = 0;

    const cacheOverride = new CacheOverride({
      beforeSend(req) {
        beforeSendCount++;
        req.headers.set('X-Count', beforeSendCount.toString());
      },
    });

    const [res1, res2] = await Promise.all([
      fetch(url, { cacheOverride }),
      fetch(url, { cacheOverride }),
    ]);

    // Only one beforeSend should execute due to request collapsing
    strictEqual(beforeSendCount, 1);
    strictEqual(res1.cached, false);
    strictEqual(res2.cached, true);
  });
}

// Test suite: Response properties and behaviors
{
  // Test response property mutations
  routes.set('/http-cache/response-mutations', async () => {
    const url = getTestUrl();
    let afterSendCalled = false;

    const cacheOverride = new CacheOverride({
      afterSend(res) {
        afterSendCalled = true;

        // Test mutating various response properties
        res.status = 201;
        res.headers.set('X-Custom', 'test');
        res.ttl = 3600;
        res.swr = 300;
        res.pci = true;
        res.surrogateKeys = new Set(['key1', 'key2']);
        res.vary = new Set(['Accept', 'User-Agent']);
      },
    });

    const res = await fetch(url, { cacheOverride });
    strictEqual(afterSendCalled, true);
    strictEqual(res.status, 201);
    strictEqual(res.headers.get('X-Custom'), 'test');
    strictEqual(res.ttl, 3600);
    strictEqual(res.swr, 300);
    strictEqual(res.pci, true);
    strictEqual([...res.surrogateKeys].sort().join(','), 'key1,key2');
    strictEqual([...res.vary].sort().join(','), 'Accept,User-Agent');
  });

  // Test cacheability properties
  routes.set('/http-cache/cacheability', async () => {
    const url = getTestUrl();

    // Test uncacheable response
    const uncacheableOverride = new CacheOverride({
      afterSend(res) {
        return { cache: 'uncacheable' };
      },
    });

    const uncacheableRes = await fetch(url, {
      cacheOverride: uncacheableOverride,
    });
    strictEqual(uncacheableRes.isCacheable, false);

    // Test forced cacheable response
    const cacheableOverride = new CacheOverride({
      afterSend(res) {
        res.headers.set('Cache-Control', 'no-store');
        return { cache: true }; // Force caching despite headers
      },
    });

    const cacheableRes = await fetch(url, { cacheOverride: cacheableOverride });
    strictEqual(cacheableRes.isCacheable, true);
  });

  // Test stale response handling
  routes.set('/http-cache/stale-responses', async () => {
    const url = getTestUrl();

    const cacheOverride = new CacheOverride({
      afterSend(res) {
        res.ttl = 1; // Very short TTL
        res.swr = 3600; // Long stale-while-revalidate
      },
    });

    // Initial fetch
    const res1 = await fetch(url, { cacheOverride });
    strictEqual(res1.isStale, false);

    // Wait for response to become stale
    await new Promise((resolve) => setTimeout(resolve, 1100));

    // Fetch stale response
    const res2 = await fetch(url, { cacheOverride });
    strictEqual(res2.isStale, true);
    strictEqual(res2.cached, true);
  });
}

// Test suite: Body transform
{
  routes.set('/http-cache/body-transform', async () => {
    const url = getTestUrl();

    const cacheOverride = new CacheOverride({
      afterSend(res) {
        // Create a transform that uppercases the response
        const transformer = new TransformStream({
          transform(chunk, controller) {
            const text = new TextDecoder().decode(chunk);
            const upperText = text.toUpperCase();
            const upperChunk = new TextEncoder().encode(upperText);
            controller.enqueue(upperChunk);
          },
        });

        return {
          bodyTransform: transformer,
          cache: true,
        };
      },
    });

    const res = await fetch(url, { cacheOverride });
    const text = await res.text();
    strictEqual(text, text.toUpperCase());
  });

  // Test transform that throws an error
  routes.set('/http-cache/body-transform-error', async () => {
    const url = getTestUrl();

    const cacheOverride = new CacheOverride({
      afterSend() {
        const transformer = new TransformStream({
          transform() {
            throw new Error('Transform failed');
          },
        });

        return { bodyTransform: transformer };
      },
    });

    // Should reject due to transform error
    await assertRejects(
      () => fetch(url, { cacheOverride }).then((res) => res.text()),
      Error,
      'Transform failed',
    );
  });

  // Test transform with invalid chunk type
  routes.set('/http-cache/body-transform-invalid-chunk', async () => {
    const url = getTestUrl();

    const cacheOverride = new CacheOverride({
      afterSend() {
        const transformer = new TransformStream({
          transform(chunk, controller) {
            // Try to enqueue invalid chunk type
            controller.enqueue('string instead of Uint8Array');
          },
        });

        return { bodyTransform: transformer };
      },
    });

    // Should reject due to invalid chunk type
    await assertRejects(
      () => fetch(url, { cacheOverride }).then((res) => res.text()),
      TypeError,
    );
  });

  // Test transform that tries to write after stream is closed
  routes.set('/http-cache/body-transform-write-after-close', async () => {
    const url = getTestUrl();
    let streamController;

    const cacheOverride = new CacheOverride({
      afterSend() {
        const transformer = new TransformStream({
          transform(chunk, controller) {
            streamController = controller;
            controller.enqueue(chunk);
          },
          flush() {
            // Try to write after stream is closed
            setTimeout(() => {
              try {
                streamController.enqueue(new Uint8Array([1, 2, 3]));
              } catch (e) {
                // Should throw as stream is closed
                strictEqual(e instanceof TypeError, true);
              }
            }, 0);
          },
        });

        return { bodyTransform: transformer };
      },
    });

    const res = await fetch(url, { cacheOverride });
    await res.text(); // Should complete successfully
  });

  // Test cancellation during transform
  routes.set('/http-cache/body-transform-cancel', async () => {
    const url = getTestUrl();
    let transformCalled = false;
    let cancelCalled = false;

    const cacheOverride = new CacheOverride({
      afterSend() {
        const transformer = new TransformStream({
          transform(chunk, controller) {
            transformCalled = true;
            // Simulate slow transform
            return new Promise((resolve) =>
              setTimeout(() => {
                controller.enqueue(chunk);
                resolve();
              }, 1000),
            );
          },
          cancel() {
            cancelCalled = true;
          },
        });

        return { bodyTransform: transformer };
      },
    });

    const res = await fetch(url, { cacheOverride });

    // Start reading the body then abort
    const reader = res.body.getReader();
    await reader.read(); // This will trigger transform
    await reader.cancel(); // Cancel mid-transform

    strictEqual(transformCalled, true);
    strictEqual(cancelCalled, true);
  });

  // Test transform with backpressure
  routes.set('/http-cache/body-transform-backpressure', async () => {
    const url = getTestUrl();
    let chunks = 0;

    const cacheOverride = new CacheOverride({
      afterSend() {
        const transformer = new TransformStream({
          async transform(chunk, controller) {
            chunks++;
            // Simulate slow processing of each chunk
            await new Promise((resolve) => setTimeout(resolve, 100));
            controller.enqueue(chunk);
          },
        });

        return { bodyTransform: transformer };
      },
    });

    const res = await fetch(url, { cacheOverride });
    await res.arrayBuffer(); // Read entire response

    // Verify transform was called for each chunk
    strictEqual(chunks > 0, true);
  });
}

// Test suite: Request Collapsing Behaviors
{
  // Test request collapsing with different cache options
  routes.set('/http-cache/request-collapsing-options', async () => {
    const url = getTestUrl();
    let backendCalls = 0;

    const cacheOverride = new CacheOverride({
      beforeSend() {
        backendCalls++;
      },
      afterSend(res) {
        // Simulate slow backend
        return new Promise((resolve) =>
          setTimeout(() => {
            resolve({ cache: true });
          }, 100),
        );
      },
    });

    // Make multiple concurrent requests
    const results = await Promise.all([
      fetch(url, { cacheOverride }),
      fetch(url, { cacheOverride }),
      fetch(url, { cacheOverride }),
    ]);

    // Only one backend call should occur due to request collapsing
    strictEqual(backendCalls, 1);
    strictEqual(results[0].cached, false);
    strictEqual(results[1].cached, true);
    strictEqual(results[2].cached, true);
  });

  // Test request collapsing with uncacheable responses
  routes.set('/http-cache/request-collapsing-uncacheable', async () => {
    const url = getTestUrl();
    let backendCalls = 0;

    const cacheOverride = new CacheOverride({
      beforeSend() {
        backendCalls++;
      },
      afterSend() {
        return { cache: 'uncacheable' };
      },
    });

    // First batch of concurrent requests
    const results1 = await Promise.all([
      fetch(url, { cacheOverride }),
      fetch(url, { cacheOverride }),
      fetch(url, { cacheOverride }),
    ]);

    // Should have collapsed to one backend call
    strictEqual(backendCalls, 1);
    results1.forEach((res) => strictEqual(res.cached, false));

    // Second request after small delay
    await new Promise((resolve) => setTimeout(resolve, 50));
    const res = await fetch(url, { cacheOverride });

    // Should trigger new backend call since previous response was marked uncacheable
    strictEqual(backendCalls, 2);
    strictEqual(res.cached, false);
  });

  // Test request collapsing with varying headers
  routes.set('/http-cache/request-collapsing-vary', async () => {
    const url = getTestUrl();
    let backendCalls = 0;

    const cacheOverride = new CacheOverride({
      beforeSend() {
        backendCalls++;
      },
      afterSend(res) {
        res.vary = new Set(['User-Agent']);
        return { cache: true };
      },
    });

    // Concurrent requests with same User-Agent
    const headers1 = { 'User-Agent': 'bot1' };
    const results1 = await Promise.all([
      fetch(url, { cacheOverride, headers: headers1 }),
      fetch(url, { cacheOverride, headers: headers1 }),
    ]);

    strictEqual(backendCalls, 1); // Should collapse
    strictEqual(results1[0].cached, false);
    strictEqual(results1[1].cached, true);

    // Concurrent requests with different User-Agent
    const headers2 = { 'User-Agent': 'bot2' };
    const results2 = await Promise.all([
      fetch(url, { cacheOverride, headers: headers2 }),
      fetch(url, { cacheOverride, headers: headers2 }),
    ]);

    strictEqual(backendCalls, 2); // Should trigger new backend call
    strictEqual(results2[0].cached, false);
    strictEqual(results2[1].cached, true);
  });
}

// Test suite: Concurrent Cache Modifications
{
  // Test concurrent modifications to cache entries
  routes.set('/http-cache/concurrent-modifications', async () => {
    const url = getTestUrl();
    let modificationCount = 0;

    const cacheOverride = new CacheOverride({
      afterSend(res) {
        modificationCount++;
        res.headers.set('X-Modification', modificationCount.toString());
        res.ttl = 3600;
        return { cache: true };
      },
    });

    // Initial request to create cache entry
    const res1 = await fetch(url, { cacheOverride });
    strictEqual(res1.headers.get('X-Modification'), '1');

    // Concurrent modifications to same cache entry
    const cacheOverride2 = new CacheOverride({
      afterSend(res) {
        res.headers.set('X-Custom', 'value1');
        res.ttl = 1800; // Different TTL
        return { cache: true };
      },
    });

    const cacheOverride3 = new CacheOverride({
      afterSend(res) {
        res.headers.set('X-Custom', 'value2');
        res.swr = 300; // Add stale-while-revalidate
        return { cache: true };
      },
    });

    // Make concurrent requests with different modifications
    const [res2, res3] = await Promise.all([
      fetch(url, { cacheOverride: cacheOverride2 }),
      fetch(url, { cacheOverride: cacheOverride3 }),
    ]);

    // Check that modifications were applied in order
    strictEqual(res2.ttl, 1800);
    strictEqual(res2.headers.get('X-Custom'), 'value1');
    strictEqual(res3.swr, 300);
    strictEqual(res3.headers.get('X-Custom'), 'value2');
  });

  // Test race conditions with body transforms
  routes.set('/http-cache/concurrent-transforms', async () => {
    const url = getTestUrl();

    // Create two different transforms
    const transform1 = new TransformStream({
      transform(chunk, controller) {
        const text = new TextDecoder().decode(chunk);
        controller.enqueue(new TextEncoder().encode(text.toUpperCase()));
      },
    });

    const transform2 = new TransformStream({
      transform(chunk, controller) {
        const text = new TextDecoder().decode(chunk);
        controller.enqueue(new TextEncoder().encode(text.toLowerCase()));
      },
    });

    const cacheOverride1 = new CacheOverride({
      afterSend() {
        return { bodyTransform: transform1, cache: true };
      },
    });

    const cacheOverride2 = new CacheOverride({
      afterSend() {
        return { bodyTransform: transform2, cache: true };
      },
    });

    // Make concurrent requests with different transforms
    const [res1, res2] = await Promise.all([
      fetch(url, { cacheOverride: cacheOverride1 }),
      fetch(url, { cacheOverride: cacheOverride2 }),
    ]);

    // Check that transforms were applied correctly
    const text1 = await res1.text();
    const text2 = await res2.text();
    strictEqual(text1, text1.toUpperCase());
    strictEqual(text2, text2.toLowerCase());
  });

  // Test cache entry updates during revalidation
  routes.set('/http-cache/revalidation-updates', async () => {
    const url = getTestUrl();
    let backendCalls = 0;

    const cacheOverride = new CacheOverride({
      beforeSend() {
        backendCalls++;
      },
      afterSend(res) {
        res.ttl = 1; // Very short TTL
        res.swr = 3600; // Long stale-while-revalidate
        res.headers.set('X-Backend-Call', backendCalls.toString());
        return { cache: true };
      },
    });

    // Initial request
    const res1 = await fetch(url, { cacheOverride });
    strictEqual(backendCalls, 1);
    strictEqual(res1.headers.get('X-Backend-Call'), '1');

    // Wait for response to become stale
    await new Promise((resolve) => setTimeout(resolve, 1100));

    // Make concurrent requests during revalidation
    const results = await Promise.all([
      fetch(url, { cacheOverride }),
      fetch(url, { cacheOverride }),
      fetch(url, { cacheOverride }),
    ]);

    // Should see stale response while revalidating
    strictEqual(backendCalls, 2); // Only one new backend call
    results.forEach((res) => {
      strictEqual(res.isStale, true);
      strictEqual(res.cached, true);
    });
  });
}

// Testing TODO:
// - new properties
// - body transform (and not being called for revalidations)
