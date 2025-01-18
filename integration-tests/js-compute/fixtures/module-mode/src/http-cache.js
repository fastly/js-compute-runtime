/// <reference path="../../../../../types/index.d.ts" />
/* eslint-env serviceworker */
import {
  assert,
  strictEqual,
  deepStrictEqual,
  assertRejects,
} from './assertions.js';
import { routes } from './routes.js';
import { CacheOverride } from 'fastly:cache-override';
import { Backend } from 'fastly:backend';

// generate a unique URL everytime so that we never work on a populated cache
const getTestUrl = (path = `/${Math.random().toString().slice(2)}`) =>
  'https://httpbin.org/anything' + path;

let _httpBinBackend;
const httpBinBackend = () =>
  _httpBinBackend ||
  (_httpBinBackend = new Backend({
    name: `httpbin-${Math.random().toString().slice(2)}`,
    target: 'httpbin.org',
  }));

// afterSend error handling
{
  routes.set('/http-cache/hook-errors', async () => {
    const url = getTestUrl();
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

  // Test invalid property assignments
  routes.set('/http-cache/invalid-properties', async () => {
    const url = getTestUrl();

    await assertRejects(
      () =>
        fetch(url, {
          backend: httpBinBackend(),
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
              res.vary = new Set(['not-an-array']);
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
              res.surrogateKeys = new Set(['not-an-array']);
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
              res.vary = [1, 2, 3]; // Should only accept strings
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
              res.surrogateKeys = [1, 2, 3]; // Should only accept strings
            },
          }),
        }),
      TypeError,
    );
  });

  routes.set('/http-cache/candidate-response-properties-uncached', async () => {
    const url = getTestUrl();

    // Test accessing cache properties on non-cached candidate response
    // (before and after hooks lifecycle) & response
    let candidateRes;
    const cacheOverride = new CacheOverride({
      afterSend(res) {
        candidateRes = res;
        strictEqual(candidateRes.cached, false);
        strictEqual(candidateRes.stale, false);
        strictEqual(candidateRes.ttl, 3600);
        strictEqual(candidateRes.age, 0);
        deepStrictEqual(candidateRes.vary, []);
        strictEqual(candidateRes.surrogateKeys.length, 1);
        strictEqual(typeof candidateRes.surrogateKeys[0], 'string');
        strictEqual(candidateRes.surrogateKeys[0].length > 10, true);
        return { cache: false };
      },
    });

    const res = await fetch(url, { cacheOverride });

    strictEqual(res.cached, false);
    strictEqual(res.stale, false);
    strictEqual(res.ttl, 3600);
    strictEqual(res.age, 0);
    deepStrictEqual(res.vary, []);
    strictEqual(res.surrogateKeys.length, 1);
    strictEqual(typeof res.surrogateKeys[0], 'string');
    strictEqual(res.surrogateKeys[0].length > 10, true);

    // in the do not store / no cache cases, the candidate response is directly
    // promoted into the response. We could possibly consider reinstancing in future, but
    // semantically this is what is defined by the caching APIs.
    assert(res === candidateRes);
  });

  routes.set('/http-cache/candidate-response-properties-cached', async () => {
    const url = getTestUrl();

    let candidateRes;
    const cacheOverride = new CacheOverride({
      afterSend(res) {
        candidateRes = res;
        return { cache: true };
      },
    });

    const res = await fetch(url, { cacheOverride });

    // in the cache case, a new response is read back from the cache for the origin request
    strictEqual(res !== candidateRes, true);

    // the response info is then taken out of the candidate response, and moved into the response
    strictEqual(candidateRes.cached, false);
    strictEqual(candidateRes.stale, false);
    strictEqual(candidateRes.ttl, undefined);
    strictEqual(candidateRes.age, undefined);
    strictEqual(candidateRes.vary, undefined);
    strictEqual(candidateRes.surrogateKeys, undefined);

    strictEqual(res.cached, false);
    strictEqual(res.stale, false);
    strictEqual(res.ttl, 3600);
    strictEqual(res.age, 0);
    deepStrictEqual(res.vary, []);
    strictEqual(res.surrogateKeys.length, 1);
    strictEqual(typeof res.surrogateKeys[0], 'string');
    strictEqual(res.surrogateKeys[0].length > 10, true);
  });

  // Test readonly properties
  routes.set('/http-cache/readonly-properties', async () => {
    const url = getTestUrl();

    const cacheOverride = new CacheOverride({
      afterSend(res) {
        res.ttl = 3600;
        return { cache: true };
      },
    });

    // Initial request
    const res1 = await fetch(url, { cacheOverride });
    strictEqual(res1.age, 0);

    // Wait a bit and verify age increased
    await new Promise((resolve) => setTimeout(resolve, 1000));
    const res2 = await fetch(url, { cacheOverride });
    strictEqual(res2.cached, true);
    strictEqual(res2.stale, false);
    strictEqual(res2.age > 0, true);

    // Verify readonly properties cannot be modified
    assertRejects(() => {
      res2.cached = false;
    }, TypeError);
    assertRejects(() => {
      res2.stale = true;
    }, TypeError);
    assertRejects(() => {
      res2.age = 0;
    }, TypeError);
  });
}

// beforeSend
{
  // Test basic request mutation via beforeSend
  routes.set('/http-cache/before-send', async () => {
    const url = getTestUrl();
    const res = await fetch(url, {
      cacheOverride: {
        beforeSend(req) {
          req.headers.set('X-Test', 'modified');
        },
      },
    });
    const body = await res.json();
    strictEqual(body, 'modified');
  });
}

// afterSend cases
{
  routes.set('/http-cache/after-send-no-cache', async () => {
    const url = getTestUrl();
    let calledAfterSend = false;
    const cacheOverride = new CacheOverride({
      afterSend(res) {
        calledAfterSend = true;
        res.headers.set('Cache-Control', 'private, no-store');
        res.ttl = 3600 - res.age;
        return { cache: false };
      },
    });
    let res = await fetch(url, { cacheOverride });
    strictEqual(calledAfterSend, true);
    strictEqual(res.headers.get('Cache-Control'), 'private, no-store');
    calledAfterSend = false;
    res = await fetch(url, { cacheOverride });
    strictEqual(calledAfterSend, true);
    strictEqual(res.headers.get('Cache-Control'), 'private, no-store');
    strictEqual(res.headers.get('x-cache'), 'MISS');
  });

  routes.set('/http-cache/after-send-cache', async () => {
    const url = getTestUrl();
    let calledAfterSend = false;
    const cacheOverride = new CacheOverride({
      afterSend(res) {
        calledAfterSend = true;
        res.headers.set('Cache-Control', 'private, no-store');
        res.ttl = 3600 - res.age;
        return { cache: true };
      },
    });
    let res = await fetch(url, { cacheOverride });
    strictEqual(calledAfterSend, true);
    strictEqual(res.headers.get('Cache-Control'), 'private, no-store');
    calledAfterSend = false;
    res = await fetch(url, { cacheOverride });
    // afterSend not called as response is cached
    strictEqual(res.cached, true);
    strictEqual(res.headers.get('x-cache'), 'HIT');
    strictEqual(calledAfterSend, false);
    strictEqual(res.headers.get('Cache-Control'), 'private, no-store');
  });

  routes.set('/http-cache/after-send-cache-expire', async () => {
    const url = getTestUrl();
    let calledAfterSend = false;
    let res = await fetch(url, {
      cacheOverride: {
        afterSend(res) {
          calledAfterSend = true;
          res.headers.set('Cache-Control', 'max-age=2');
          return { cache: true };
        },
      },
    });
    strictEqual(calledAfterSend, true);
    strictEqual(res.cached, false);
    strictEqual(res.headers.get('x-cache'), 'MISS');
    strictEqual(res.headers.get('Cache-Control'), 'max-age=2');

    await new Promise((resolve) => setTimeout(resolve, 500));
    calledAfterSend = false;
    res = await fetch(url, {
      cacheOverride: {
        afterSend(res) {
          calledAfterSend = true;
        },
      },
    });

    // should still be cached
    strictEqual(calledAfterSend, false);
    strictEqual(res.age > 0, true);
    strictEqual(res.age < 2, true);
    strictEqual(res.cached, true);
    strictEqual(res.headers.get('x-cache'), 'HIT');
    strictEqual(res.headers.get('Cache-Control'), 'max-age=2');

    // should then expire
    await new Promise((resolve) => setTimeout(resolve, 2000));

    res = await fetch(url, {
      cacheOverride: {
        afterSend(res) {
          calledAfterSend = true;
        },
      },
    });
    strictEqual(calledAfterSend, true);
    strictEqual(res.cached, false);
    strictEqual(res.age, 0);
  });

  routes.set('/http-cache/after-send-pass', async () => {
    const url = getTestUrl();

    let res = await fetch(url, {
      cacheOverride: {
        afterSend(res) {
          res.headers.set('x-hooked', '');
        },
      },
    });

    res = await fetch(url);
    strictEqual(res.headers.has('x-hooked'), true);

    res = await fetch(url, { cacheOverride: 'pass' });
    strictEqual(res.headers.has('x-hooked'), false);
  });

  routes.set('/http-cache/after-send-res-no-body', async () => {
    const url = getTestUrl();

    let calledAfterSend = false;
    const res = await fetch(url, {
      cacheOverride: {
        async afterSend(res) {
          calledAfterSend = true;
          strictEqual(res.body, null);
          // empty text gets given for opaque body
          strictEqual(await res.text(), '');
          // not cached -> this candidate response is promoted into the final resposne
          return { cache: false };
        },
      },
    });
    strictEqual(calledAfterSend, true);
    // verify we get a proper response (url included in response)
    strictEqual((await res.json()).url, url);
  });

  // Test response property mutations
  routes.set('/http-cache/response-mutations', async () => {
    const url = getTestUrl();
    let afterSendCalled = false;

    const res = await fetch(url, {
      cacheOverride: {
        afterSend(res) {
          afterSendCalled = true;

          // Test mutating various response properties

          // cannot change status
          try {
            res.status = 201;
            assert(false, 'Expected failure');
          } catch (err) {
            strictEqual(err instanceof TypeError, true);
          }

          // can change headers
          res.headers.set('X-Custom', 'test');

          // can change cache options
          res.ttl = 4000;
          res.swr = 400;
          res.pci = true;
          res.surrogateKeys = ['key1', 'key2'];
          res.vary = ['Accept', 'User-Agent'];
        },
      },
    });
    strictEqual(afterSendCalled, true);
    strictEqual(res.status, 200);
    strictEqual(res.headers.get('X-Custom'), 'test');
    strictEqual(res.ttl, 4000);
    strictEqual(res.swr, 400);
    strictEqual(res.pci, true);
    strictEqual(res.surrogateKeys.sort().join(','), 'key1,key2');
    strictEqual(res.vary.sort().join(','), 'Accept,User-Agent');
  });

  // Test stale response handling
  routes.set('/http-cache/stale-responses', async () => {
    const url = getTestUrl();

    // Initial fetch
    const res1 = await fetch(url, {
      cacheOverride: {
        afterSend(res) {
          res.ttl = 1; // Very short TTL
          res.swr = 2; // Long stale-while-revalidate
        },
      },
    });
    strictEqual(res1.cached, false);
    strictEqual(res1.stale, false);

    // Wait for response to become stale
    await new Promise((resolve) => setTimeout(resolve, 1500));

    // Fetch stale response
    let calledAfterSendStale = false;
    const res2 = await fetch(url, {
      cacheOverride: {
        // aftersend will be performed for background revalidation
        afterSend(res) {
          calledAfterSendStale = true;
        },
      },
    });
    // stale response is returned while background revalidation happens
    strictEqual(calledAfterSendStale, false);
    strictEqual(res2.cached, true);
    strictEqual(res2.stale, true);

    // Wait for stale response to be invalidated too
    await new Promise((resolve) => setTimeout(resolve, 1500));

    // Now we should get back the background revalidation we just performed
    let calledAfterSend = false;
    const res3 = await fetch(url, {
      cacheOverride: {
        afterSend(res) {
          calledAfterSend = true;
        },
      },
    });

    // TODO: properly test background revalidation here
    // strictEqual(res3.cached, true);
    // strictEqual(res3.stale, false);
    // strictEqual(calledAfterSend, false);
    // strictEqual(calledAfterStale, true);
  });
}

// Test suite: Body transform
{
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

// TODO: Enable this suite once we support full asynchrony for transaction lookups
// Test suite: Concurrent / Request Collapsing Behaviors
// {
//
// // Test request mutation order with multiple concurrent requests
// routes.set('/http-cache/request-mutation-order', async () => {
//   const url = getTestUrl();

//   let beforeSendCount = 0;

//   const cacheOverride = new CacheOverride({
//     beforeSend(req) {
//       beforeSendCount++;
//       req.headers.set('X-Count', beforeSendCount.toString());
//     },
//   });

//   const [res1, res2] = await Promise.all([
//     fetch(url, { cacheOverride }),
//     fetch(url, { cacheOverride }),
//   ]);

//   // Only one beforeSend should execute due to request collapsing
//   strictEqual(beforeSendCount, 1);
//   strictEqual(res1.cached, false);
//   strictEqual(res2.cached, true);
// });
//   routes.set('/http-cache/concurrent-cache', async () => {
//     const url = getTestUrl();

//     const [res2, res3] = await Promise.all([
//       // Make concurrent requests with different modifications
//       fetch(url, {
//         cacheOverride: {
//           afterSend(res) {
//             res.headers.set('X-Custom', 'value1');
//             res.ttl = 1800; // Different TTL
//             return { cache: true };
//           },
//         }
//       }),
//       // Concurrent modifications to same cache entry
//       fetch(url, {
//         cacheOverride: {
//           afterSend(res) {
//             res.headers.set('X-Custom', 'value2');
//             res.swr = 300; // Add stale-while-revalidate
//             return { cache: true };
//           },
//         }
//       }),
//     ]);

//     // Check that modifications were applied in order
//     strictEqual(res2.ttl, 1800);
//     strictEqual(res2.headers.get('X-Custom'), 'value1');
//     strictEqual(res3.swr, 300);
//     strictEqual(res3.headers.get('X-Custom'), 'value2');
//   });

//   Test request collapsing with different cache options
//   routes.set('/http-cache/request-collapsing-options', async () => {
//     const url = getTestUrl();
//     let backendCalls = 0;

//     const cacheOverride = new CacheOverride({
//       beforeSend() {
//         backendCalls++;
//       },
//       afterSend(res) {
//         // Simulate slow backend
//         return new Promise((resolve) =>
//           setTimeout(() => {
//             resolve({ cache: true });
//           }, 100),
//         );
//       },
//     });

//     // Make multiple concurrent requests
//     const results = await Promise.all([
//       fetch(url, { cacheOverride }),
//       fetch(url, { cacheOverride }),
//       fetch(url, { cacheOverride }),
//     ]);

//     // Only one backend call should occur due to request collapsing
//     strictEqual(backendCalls, 1);
//     strictEqual(results[0].cached, false);
//     strictEqual(results[1].cached, true);
//     strictEqual(results[2].cached, true);
//   });

//   // Test request collapsing with uncacheable responses
//   routes.set('/http-cache/request-collapsing-uncacheable', async () => {
//     const url = getTestUrl();
//     let backendCalls = 0;

//     const cacheOverride = new CacheOverride({
//       beforeSend() {
//         backendCalls++;
//       },
//       afterSend() {
//         return { cache: 'uncacheable' };
//       },
//     });

//     // First batch of concurrent requests
//     const results1 = await Promise.all([
//       fetch(url, { cacheOverride }),
//       fetch(url, { cacheOverride }),
//       fetch(url, { cacheOverride }),
//     ]);

//     // Should have collapsed to one backend call
//     strictEqual(backendCalls, 1);
//     results1.forEach((res) => strictEqual(res.cached, false));

//     // Second request after small delay
//     await new Promise((resolve) => setTimeout(resolve, 50));
//     const res = await fetch(url, { cacheOverride });

//     // Should trigger new backend call since previous response was marked uncacheable
//     strictEqual(backendCalls, 2);
//     strictEqual(res.cached, false);
//   });

//   // Test request collapsing with varying headers
//   routes.set('/http-cache/request-collapsing-vary', async () => {
//     const url = getTestUrl();
//     let backendCalls = 0;

//     const cacheOverride = new CacheOverride({
//       beforeSend() {
//         backendCalls++;
//       },
//       afterSend(res) {
//         res.vary = new Set(['User-Agent']);
//         return { cache: true };
//       },
//     });

//     // Concurrent requests with same User-Agent
//     const headers1 = { 'User-Agent': 'bot1' };
//     const results1 = await Promise.all([
//       fetch(url, { cacheOverride, headers: headers1 }),
//       fetch(url, { cacheOverride, headers: headers1 }),
//     ]);

//     strictEqual(backendCalls, 1); // Should collapse
//     strictEqual(results1[0].cached, false);
//     strictEqual(results1[1].cached, true);

//     // Concurrent requests with different User-Agent
//     const headers2 = { 'User-Agent': 'bot2' };
//     const results2 = await Promise.all([
//       fetch(url, { cacheOverride, headers: headers2 }),
//       fetch(url, { cacheOverride, headers: headers2 }),
//     ]);

//     strictEqual(backendCalls, 2); // Should trigger new backend call
//     strictEqual(results2[0].cached, false);
//     strictEqual(results2[1].cached, true);
//   });
//
// // Test race conditions with body transforms
// routes.set('/http-cache/concurrent-transforms', async () => {
//   const url = getTestUrl();

//   // Create two different transforms
//   const transform1 = new TransformStream({
//     transform(chunk, controller) {
//       const text = new TextDecoder().decode(chunk);
//       controller.enqueue(new TextEncoder().encode(text.toUpperCase()));
//     },
//   });

//   const transform2 = new TransformStream({
//     transform(chunk, controller) {
//       const text = new TextDecoder().decode(chunk);
//       controller.enqueue(new TextEncoder().encode(text.toLowerCase()));
//     },
//   });

//   const cacheOverride1 = new CacheOverride({
//     afterSend() {
//       return { bodyTransform: transform1, cache: true };
//     },
//   });

//   const cacheOverride2 = new CacheOverride({
//     afterSend() {
//       return { bodyTransform: transform2, cache: true };
//     },
//   });

//   // Make concurrent requests with different transforms
//   const [res1, res2] = await Promise.all([
//     fetch(url, { cacheOverride: cacheOverride1 }),
//     fetch(url, { cacheOverride: cacheOverride2 }),
//   ]);

//   // Check that transforms were applied correctly
//   const text1 = await res1.text();
//   const text2 = await res2.text();
//   strictEqual(text1, text1.toUpperCase());
//   strictEqual(text2, text2.toLowerCase());
// });
// }
