/// <reference path="../../../../../types/index.d.ts" />

import { env } from 'fastly:env';
import { pass, fail, assert, assertThrows, assertRejects } from "../../../assertions.js";

addEventListener("fetch", event => {
  event.respondWith(app(event));
});
/**
 * @param {FetchEvent} event
 * @returns {Response}
 */
async function app(event) {
  try {
    const path = (new URL(event.request.url)).pathname;
    console.log(`path: ${path}`)
    console.log(`FASTLY_SERVICE_VERSION: ${env('FASTLY_SERVICE_VERSION')}`)
    if (routes.has(path)) {
      const routeHandler = routes.get(path);
      return await routeHandler();
    }
    return fail(`${path} endpoint does not exist`)
  } catch (error) {
    return fail(`The routeHandler threw an error: ${error.message}` + '\n' + error.stack)
  }
}

const routes = new Map();
routes.set('/', () => {
  routes.delete('/');
  let test_routes = Array.from(routes.keys())
  return new Response(JSON.stringify(test_routes), { 'headers': { 'content-type': 'application/json' } });
});

let error;
routes.set("/crypto", async () => {
  error = assert(typeof crypto, 'object', `typeof crypto`)
  if (error) { return error; }
  error = assert(crypto instanceof Crypto, true, `crypto instanceof Crypto`)
  if (error) { return error; }
  return pass();
});

routes.set("/crypto.subtle", async () => {
  error = assert(typeof crypto.subtle, 'object', `typeof crypto.subtle`)
  if (error) { return error; }
  error = assert(crypto.subtle instanceof SubtleCrypto, true, `crypto.subtle instanceof SubtleCrypto`)
  if (error) { return error; }
  return pass();
});

// digest
{
  const enc = new TextEncoder();
  const data = enc.encode('hello world');
  routes.set("/crypto.subtle.digest", async () => {
    error = assert(typeof crypto.subtle.digest, 'function', `typeof crypto.subtle.digest`)
    if (error) { return error; }
    error = assert(crypto.subtle.digest, SubtleCrypto.prototype.digest, `crypto.subtle.digest === SubtleCrypto.prototype.digest`)
    if (error) { return error; }
    return pass()
  });
  routes.set("/crypto.subtle.digest/length", async () => {
    error = assert(crypto.subtle.digest.length, 2, `crypto.subtle.digest.length === 2`)
    if (error) { return error; }
    return pass()
  });
  routes.set("/crypto.subtle.digest/called-as-constructor", async () => {
    error = assertThrows(() => {
      new crypto.subtle.digest
    }, TypeError, "crypto.subtle.digest is not a constructor")
    if (error) { return error; }
    return pass()
  });
  routes.set("/crypto.subtle.digest/called-with-wrong-this", async () => {
    error = await assertRejects(async () => {
      await crypto.subtle.digest.call(undefined)
    }, TypeError, "Method SubtleCrypto.digest called on receiver that's not an instance of SubtleCrypto")
    if (error) { return error; }
    return pass()
  });
  routes.set("/crypto.subtle.digest/called-with-no-arguments", async () => {
    error = await assertRejects(async () => {
      await crypto.subtle.digest()
    }, TypeError, "SubtleCrypto.digest: At least 2 arguments required, but only 0 passed")
    if (error) { return error; }
    return pass()
  });

  // first-parameter
  {
    routes.set("/crypto.subtle.digest/first-parameter-calls-7.1.17-ToString", async () => {
      const sentinel = Symbol("sentinel");
      const test = async () => {
        await crypto.subtle.digest({
          name: {
            toString() {
              throw sentinel;
            }
          }
        }, data);
      }
      let error = await assertRejects(test);
      if (error) { return error; }
      try {
        await test();
      } catch (thrownError) {
        let error = assert(thrownError, sentinel, 'thrownError === sentinel');
        if (error) { return error; }
      }
      return pass();
    });
    routes.set("/crypto.subtle.digest/first-parameter-non-existant-format", async () => {
      let error = await assertRejects(async () => {
        await crypto.subtle.digest('jake', data);
      }, Error, "Algorithm: Unrecognized name");
      if (error) { return error; }
      return pass();
    });
  }
  // second-parameter
  {
    routes.set("/crypto.subtle.digest/second-parameter-undefined", async () => {
      let error = await assertRejects(async () => {
        await crypto.subtle.digest("sha-1", undefined);
      }, Error, "SubtleCrypto.digest: data must be of type ArrayBuffer or ArrayBufferView but got \"\"");
      if (error) { return error; }
      return pass();
    });
  }
  // happy paths 
  {
    // "SHA-1"
    routes.set("/crypto.subtle.digest/sha-1", async () => {
      const result = new Uint8Array(await crypto.subtle.digest("sha-1", new Uint8Array));
      const expected = new Uint8Array([218, 57, 163, 238, 94, 107, 75, 13, 50, 85, 191, 239, 149, 96, 24, 144, 175, 216, 7, 9]);
      error = assert(result, expected, "result deep equals expected");
      if (error) { return error; }
      return pass();
    });
    // "SHA-256"
    routes.set("/crypto.subtle.digest/sha-256", async () => {
      const result = new Uint8Array(await crypto.subtle.digest("sha-256", new Uint8Array));
      const expected = new Uint8Array([227, 176, 196, 66, 152, 252, 28, 20, 154, 251, 244, 200, 153, 111, 185, 36, 39, 174, 65, 228, 100, 155, 147, 76, 164, 149, 153, 27, 120, 82, 184, 85]);
      error = assert(result, expected, "result deep equals expected");
      if (error) { return error; }
      return pass();
    });
    // "SHA-384"
    routes.set("/crypto.subtle.digest/sha-384", async () => {
      const result = new Uint8Array(await crypto.subtle.digest("sha-384", new Uint8Array));
      const expected = new Uint8Array([56, 176, 96, 167, 81, 172, 150, 56, 76, 217, 50, 126, 177, 177, 227, 106, 33, 253, 183, 17, 20, 190, 7, 67, 76, 12, 199, 191, 99, 246, 225, 218, 39, 78, 222, 191, 231, 111, 101, 251, 213, 26, 210, 241, 72, 152, 185, 91]);
      error = assert(result, expected, "result deep equals expected");
      if (error) { return error; }
      return pass();
    });
    // "SHA-512"
    routes.set("/crypto.subtle.digest/sha-512", async () => {
      const result = new Uint8Array(await crypto.subtle.digest("sha-512", new Uint8Array));
      const expected = new Uint8Array([207, 131, 225, 53, 126, 239, 184, 189, 241, 84, 40, 80, 214, 109, 128, 7, 214, 32, 228, 5, 11, 87, 21, 220, 131, 244, 169, 33, 211, 108, 233, 206, 71, 208, 209, 60, 93, 133, 242, 176, 255, 131, 24, 210, 135, 126, 236, 47, 99, 185, 49, 189, 71, 65, 122, 129, 165, 56, 50, 122, 249, 39, 218, 62]);
      error = assert(result, expected, "result deep equals expected");
      if (error) { return error; }
      return pass();
    });
  }

}
