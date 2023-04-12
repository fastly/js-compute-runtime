/// <reference path="../../../../../types/index.d.ts" />
/* eslint-env serviceworker, shared-node-browser, browser */

import { env } from 'fastly:env';
import { pass, fail, assert, assertThrows, assertRejects, assertResolves } from "../../../assertions.js";

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

// From https://www.rfc-editor.org/rfc/rfc7517#appendix-A.1
const publicJsonWebKeyData = {
  "alg":"RS256",
  "e":"AQAB",
  "ext": true,
  "key_ops": [
    "verify"
  ],
  "kty":"RSA",
  "n": "0vx7agoebGcQSuuPiLJXZptN9nndrQmbXEps2aiAFbWhM78LhWx4cbbfAAtVT86zwu1RK7aPFFxuhDR1L6tSoc_BJECPebWKRXjBZCiFV4n3oknjhMstn64tZ_2W-5JsGY4Hc5n9yBXArwl93lqt7_RN5w6Cf0h4QyQ5v-65YGjQR0_FDW2QvzqY368QQMicAtaSqzs8KJZgnYb9c7d0zgdAZHzu6qMQvRL5hajrn1n91CbOpbISD08qNLyrdkt-bFTWhAI4vMQFh6WeZu0fM4lFd2NcRwr3XPksINHaQ-G_xBniIqbw0Ls1jF44-csFCur-kEgU8awapJzKnqDKgw"
};

// From https://www.rfc-editor.org/rfc/rfc7517#appendix-A.2
const privateJsonWebKeyData = {
  "alg":"RS256",
  "d":"X4cTteJY_gn4FYPsXB8rdXix5vwsg1FLN5E3EaG6RJoVH-HLLKD9M7dx5oo7GURknchnrRweUkC7hT5fJLM0WbFAKNLWY2vv7B6NqXSzUvxT0_YSfqijwp3RTzlBaCxWp4doFk5N2o8Gy_nHNKroADIkJ46pRUohsXywbReAdYaMwFs9tv8d_cPVY3i07a3t8MN6TNwm0dSawm9v47UiCl3Sk5ZiG7xojPLu4sbg1U2jx4IBTNBznbJSzFHK66jT8bgkuqsk0GjskDJk19Z4qwjwbsnn4j2WBii3RL-Us2lGVkY8fkFzme1z0HbIkfz0Y6mqnOYtqc0X4jfcKoAC8Q",
  "dp":"G4sPXkc6Ya9y8oJW9_ILj4xuppu0lzi_H7VTkS8xj5SdX3coE0oimYwxIi2emTAue0UOa5dpgFGyBJ4c8tQ2VF402XRugKDTP8akYhFo5tAA77Qe_NmtuYZc3C3m3I24G2GvR5sSDxUyAN2zq8Lfn9EUms6rY3Ob8YeiKkTiBj0",
  "dq":"s9lAH9fggBsoFR8Oac2R_E2gw282rT2kGOAhvIllETE1efrA6huUUvMfBcMpn8lqeW6vzznYY5SSQF7pMdC_agI3nG8Ibp1BUb0JUiraRNqUfLhcQb_d9GF4Dh7e74WbRsobRonujTYN1xCaP6TO61jvWrX-L18txXw494Q_cgk",
  "e":"AQAB",
  "ext": true,
  "key_ops": [
    "sign"
  ],
  "kty":"RSA",
  "n":"0vx7agoebGcQSuuPiLJXZptN9nndrQmbXEps2aiAFbWhM78LhWx4cbbfAAtVT86zwu1RK7aPFFxuhDR1L6tSoc_BJECPebWKRXjBZCiFV4n3oknjhMstn64tZ_2W-5JsGY4Hc5n9yBXArwl93lqt7_RN5w6Cf0h4QyQ5v-65YGjQR0_FDW2QvzqY368QQMicAtaSqzs8KJZgnYb9c7d0zgdAZHzu6qMQvRL5hajrn1n91CbOpbISD08qNLyrdkt-bFTWhAI4vMQFh6WeZu0fM4lFd2NcRwr3XPksINHaQ-G_xBniIqbw0Ls1jF44-csFCur-kEgU8awapJzKnqDKgw",
  "p":"83i-7IvMGXoMXCskv73TKr8637FiO7Z27zv8oj6pbWUQyLPQBQxtPVnwD20R-60eTDmD2ujnMt5PoqMrm8RfmNhVWDtjjMmCMjOpSXicFHj7XOuVIYQyqVWlWEh6dN36GVZYk93N8Bc9vY41xy8B9RzzOGVQzXvNEvn7O0nVbfs",
  "q":"3dfOR9cuYq-0S-mkFLzgItgMEfFzB2q3hWehMuG0oCuqnb3vobLyumqjVZQO1dIrdwgTnCdpYzBcOfW5r370AFXjiWft_NGEiovonizhKpo9VVS78TzFgxkIdrecRezsZ-1kYd_s1qDbxtkDEgfAITAG9LUnADun4vIcb6yelxk",
  "qi":"GyM_p6JrXySiz1toFgKbWV-JdI3jQ4ypu9rbMWx3rQJBfmt0FoYzgUIZEVFEcOqwemRN81zoDAaa-Bk0KWNGDjJHZDdDmFhW3AN7lI-puxk_mHZGJ11rxyR8O55XLSe3SPmRfKwZI6yU24ZxvQKFYItdldUKGzO6Ia6zTKhAVRU",
};

const jsonWebKeyAlgorithm = {
  name: "RSASSA-PKCS1-v1_5",
  hash: { name: "SHA-256" },
};

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

// importKey
{
  routes.set("/crypto.subtle.importKey", async () => {
    error = assert(typeof crypto.subtle.importKey, 'function', `typeof crypto.subtle.importKey`)
    if (error) { return error; }
    error = assert(crypto.subtle.importKey, SubtleCrypto.prototype.importKey, `crypto.subtle.importKey === SubtleCrypto.prototype.importKey`)
    if (error) { return error; }
    return pass();
  });
  routes.set("/crypto.subtle.importKey/length", async () => {
    error = assert(crypto.subtle.importKey.length, 5, `crypto.subtle.importKey.length === 5`)
    if (error) { return error; }
    return pass();
  });
  routes.set("/crypto.subtle.importKey/called-as-constructor", async () => {
    error = assertThrows(() => {
      new crypto.subtle.importKey
    }, TypeError, "crypto.subtle.importKey is not a constructor")
    if (error) { return error; }
    return pass();
  });
  routes.set("/crypto.subtle.importKey/called-with-wrong-this", async () => {
    error = await assertRejects(async () => {
      await crypto.subtle.importKey.call(undefined, 'jwk', publicJsonWebKeyData, jsonWebKeyAlgorithm, publicJsonWebKeyData.ext, publicJsonWebKeyData.key_ops)
    }, TypeError, "Method SubtleCrypto.importKey called on receiver that's not an instance of SubtleCrypto")
    if (error) { return error; }
    return pass();
  });
  routes.set("/crypto.subtle.importKey/called-with-no-arguments", async () => {
    error = await assertRejects(async () => {
      await crypto.subtle.importKey()
    }, TypeError, "SubtleCrypto.importKey: At least 5 arguments required, but only 0 passed")
    if (error) { return error; }
    return pass();
  });

  // first-parameter
  {
    routes.set("/crypto.subtle.importKey/first-parameter-calls-7.1.17-ToString", async () => {
      const sentinel = Symbol("sentinel");
      const test = async () => {
        const format = {
          toString() {
            throw sentinel;
          }
        }
        await crypto.subtle.importKey(format, publicJsonWebKeyData, jsonWebKeyAlgorithm, publicJsonWebKeyData.ext, publicJsonWebKeyData.key_ops)
      }
      let error = await assertRejects(test)
      if (error) { return error; }
      try {
        await test()
      } catch (thrownError) {
        let error = assert(thrownError, sentinel, 'thrownError === sentinel')
        if (error) { return error; }
      }
      return pass();
    });
    routes.set("/crypto.subtle.importKey/first-parameter-non-existant-format", async () => {
      let error = await assertRejects(async () => {
        await crypto.subtle.importKey('jake', publicJsonWebKeyData, jsonWebKeyAlgorithm, publicJsonWebKeyData.ext, publicJsonWebKeyData.key_ops)
      }, Error, "Provided format parameter is not supported. Supported formats are: 'spki', 'pkcs8', 'jwk', and 'raw'")
      if (error) { return error; }
      return pass();
    });
  }

  // second-parameter
  {
    routes.set("/crypto.subtle.importKey/second-parameter-invalid-format", async () => {
      let error = await assertRejects(async () => {
        await crypto.subtle.importKey('jwk', Symbol(), jsonWebKeyAlgorithm, publicJsonWebKeyData.ext, publicJsonWebKeyData.key_ops)
      }, Error, "The provided value is not of type JsonWebKey")
      if (error) { return error; }
      return pass();
    });
    // jwk public key
    {
      routes.set("/crypto.subtle.importKey/second-parameter-missing-e-field", async () => {
        let error = await assertRejects(async () => {
          delete publicJsonWebKeyData.e;
          await crypto.subtle.importKey('jwk', publicJsonWebKeyData, jsonWebKeyAlgorithm, publicJsonWebKeyData.ext, publicJsonWebKeyData.key_ops)
        }, Error, "Data provided to an operation does not meet requirements")
        if (error) { return error; }
        return pass();
      });
      routes.set("/crypto.subtle.importKey/second-parameter-e-field-calls-7.1.17-ToString", async () => {
        let sentinel = Symbol("sentinel");
        const test = async () => {
          sentinel = Symbol();
          publicJsonWebKeyData.e = {
            toString() {
              throw sentinel;
            }
          }
          await crypto.subtle.importKey('jwk', publicJsonWebKeyData, jsonWebKeyAlgorithm, publicJsonWebKeyData.ext, publicJsonWebKeyData.key_ops)
        }
        let error = await assertRejects(test)
        if (error) { return error; }
        try {
          await test()
        } catch (thrownError) {
          let error = assert(thrownError, sentinel, 'thrownError === sentinel')
          if (error) { return error; }
        }
        return pass();
      });
      routes.set("/crypto.subtle.importKey/second-parameter-invalid-e-field", async () => {
        let error = await assertRejects(async () => {
          publicJsonWebKeyData.e = "`~!@#@#$Q%^%&^*";
          await crypto.subtle.importKey('jwk', publicJsonWebKeyData, jsonWebKeyAlgorithm, publicJsonWebKeyData.ext, publicJsonWebKeyData.key_ops)
        }, Error, "The JWK member 'e' could not be base64url decoded or contained padding")
        if (error) { return error; }
        return pass();
      });
      routes.set("/crypto.subtle.importKey/second-parameter-missing-kty-field", async () => {
        let error = await assertRejects(async () => {
          delete publicJsonWebKeyData.kty;
          await crypto.subtle.importKey('jwk', publicJsonWebKeyData, jsonWebKeyAlgorithm, publicJsonWebKeyData.ext, publicJsonWebKeyData.key_ops)
        }, Error, "The required JWK member 'kty' was missing")
        if (error) { return error; }
        return pass();
      });
      routes.set("/crypto.subtle.importKey/second-parameter-invalid-kty-field", async () => {
        let error = await assertRejects(async () => {
          publicJsonWebKeyData.kty = "jake";
          await crypto.subtle.importKey('jwk', publicJsonWebKeyData, jsonWebKeyAlgorithm, publicJsonWebKeyData.ext, publicJsonWebKeyData.key_ops)
        }, Error, "The JWK 'kty' member was not 'RSA'")
        if (error) { return error; }
        return pass();
      });
      routes.set("/crypto.subtle.importKey/second-parameter-missing-key_ops-field", async () => {
        let error = await assertResolves(async () => {
          const key_ops = Array.from(publicJsonWebKeyData.key_ops);
          delete publicJsonWebKeyData.key_ops;
          await crypto.subtle.importKey('jwk', publicJsonWebKeyData, jsonWebKeyAlgorithm, publicJsonWebKeyData.ext, key_ops)
        })
        if (error) { return error; }
        return pass();
      });
      routes.set("/crypto.subtle.importKey/second-parameter-non-sequence-key_ops-field", async () => {
        let error = await assertRejects(async () => {
          const key_ops = Array.from(publicJsonWebKeyData.key_ops);
          publicJsonWebKeyData.key_ops = "jake";
          await crypto.subtle.importKey('jwk', publicJsonWebKeyData, jsonWebKeyAlgorithm, publicJsonWebKeyData.ext, key_ops)
        }, Error, "Failed to read the 'key_ops' property from 'JsonWebKey': The provided value cannot be converted to a sequence")
        if (error) { return error; }
        return pass();
      });

      routes.set("/crypto.subtle.importKey/second-parameter-empty-key_ops-field", async () => {
        let error = await assertResolves(async () => {
          const key_ops = Array.from(publicJsonWebKeyData.key_ops);
          publicJsonWebKeyData.key_ops = [];
          await crypto.subtle.importKey('jwk', publicJsonWebKeyData, jsonWebKeyAlgorithm, publicJsonWebKeyData.ext, key_ops)
        })
        if (error) { return error; }
        return pass();
      });
      routes.set("/crypto.subtle.importKey/second-parameter-duplicated-key_ops-field", async () => {
        let error = await assertRejects(async () => {
          const key_ops = Array.from(publicJsonWebKeyData.key_ops);
          publicJsonWebKeyData.key_ops = ["sign", "sign"];
          await crypto.subtle.importKey('jwk', publicJsonWebKeyData, jsonWebKeyAlgorithm, publicJsonWebKeyData.ext, key_ops)
        }, Error, "The 'key_ops' member of the JWK dictionary contains duplicate usages")
        if (error) { return error; }
        return pass();
      });
      routes.set("/crypto.subtle.importKey/second-parameter-invalid-key_ops-field", async () => {
        let error = await assertRejects(async () => {
          const key_ops = Array.from(publicJsonWebKeyData.key_ops);
          publicJsonWebKeyData.key_ops = ["sign", "jake"];
          await crypto.subtle.importKey('jwk', publicJsonWebKeyData, jsonWebKeyAlgorithm, publicJsonWebKeyData.ext, key_ops)
        }, TypeError, "Invalid keyUsages argument")
        if (error) { return error; }
        return pass();
      });

      routes.set("/crypto.subtle.importKey/second-parameter-key_ops-field-calls-7.1.17-ToString", async () => {
        let sentinel = Symbol("sentinel");
        const key_ops = Array.from(publicJsonWebKeyData.key_ops);
        const test = async () => {
          sentinel = Symbol();
          const op = {
            toString() {
              throw sentinel;
            }
          }
          publicJsonWebKeyData.key_ops = ["sign", op];
          await crypto.subtle.importKey('jwk', publicJsonWebKeyData, jsonWebKeyAlgorithm, publicJsonWebKeyData.ext, key_ops)
        }
        let error = await assertRejects(test)
        if (error) { return error; }
        try {
          await test()
        } catch (thrownError) {
          let error = assert(thrownError, sentinel, 'thrownError === sentinel')
          if (error) { return error; }
        }
        return pass();
      });

      routes.set("/crypto.subtle.importKey/second-parameter-missing-n-field", async () => {
        let error = await assertRejects(async () => {
          delete publicJsonWebKeyData.n;
          await crypto.subtle.importKey('jwk', publicJsonWebKeyData, jsonWebKeyAlgorithm, publicJsonWebKeyData.ext, publicJsonWebKeyData.key_ops)
        }, Error, "Data provided to an operation does not meet requirements")
        if (error) { return error; }
        return pass();
      });
      routes.set("/crypto.subtle.importKey/second-parameter-n-field-calls-7.1.17-ToString", async () => {
        let sentinel = Symbol("sentinel");
        const test = async () => {
          sentinel = Symbol();
          publicJsonWebKeyData.n = {
            toString() {
              throw sentinel;
            }
          }
          await crypto.subtle.importKey('jwk', publicJsonWebKeyData, jsonWebKeyAlgorithm, publicJsonWebKeyData.ext, publicJsonWebKeyData.key_ops)
        }
        let error = await assertRejects(test)
        if (error) { return error; }
        try {
          await test()
        } catch (thrownError) {
          let error = assert(thrownError, sentinel, 'thrownError === sentinel')
          if (error) { return error; }
        }
        return pass();
      });
      routes.set("/crypto.subtle.importKey/second-parameter-invalid-n-field", async () => {
        let error = await assertRejects(async () => {
          publicJsonWebKeyData.n = "`~!@#@#$Q%^%&^*";
          await crypto.subtle.importKey('jwk', publicJsonWebKeyData, jsonWebKeyAlgorithm, publicJsonWebKeyData.ext, publicJsonWebKeyData.key_ops)
        }, Error, "The JWK member 'n' could not be base64url decoded or contained padding")
        if (error) { return error; }
        return pass();
      });
    }
    // jwk private key
    // raw AES
    // raw HMAC secret keys
    // raw Elliptic Curve public keys
    // pkcs8 RSA private keys
    // pkcs8 Elliptic Curve private keys
  }
  // third-parameter 
  {
    routes.set("/crypto.subtle.importKey/third-parameter-undefined", async () => {
      let error = await assertRejects(async () => {
        await crypto.subtle.importKey('jwk', publicJsonWebKeyData, undefined, publicJsonWebKeyData.ext, publicJsonWebKeyData.key_ops)
      }, Error, "Algorithm: Unrecognized name")
      if (error) { return error; }
      return pass();
    });
    routes.set("/crypto.subtle.importKey/third-parameter-name-field-calls-7.1.17-ToString", async () => {
      const sentinel = Symbol("sentinel");
      const test = async () => {
        jsonWebKeyAlgorithm.name = {
          toString() {
            throw sentinel;
          }
        }
        await crypto.subtle.importKey('jwk', publicJsonWebKeyData, jsonWebKeyAlgorithm, publicJsonWebKeyData.ext, publicJsonWebKeyData.key_ops)
      }
      let error = await assertRejects(test)
      if (error) { return error; }
      try {
        await test()
      } catch (thrownError) {
        let error = assert(thrownError, sentinel, 'thrownError === sentinel')
        if (error) { return error; }
      }
      return pass();
    });
    routes.set("/crypto.subtle.importKey/third-parameter-invalid-name-field", async () => {
      let error = await assertRejects(async () => {
        jsonWebKeyAlgorithm.name = "`~!@#@#$Q%^%&^*";
        await crypto.subtle.importKey('jwk', publicJsonWebKeyData, jsonWebKeyAlgorithm, publicJsonWebKeyData.ext, publicJsonWebKeyData.key_ops)
      }, Error, "Algorithm: Unrecognized name")
      if (error) { return error; }
      return pass();
    });
    routes.set("/crypto.subtle.importKey/third-parameter-hash-name-field-calls-7.1.17-ToString", async () => {
      const sentinel = Symbol("sentinel");
      const test = async () => {
        jsonWebKeyAlgorithm.hash.name = {
          toString() {
            throw sentinel;
          }
        }
        await crypto.subtle.importKey('jwk', publicJsonWebKeyData, jsonWebKeyAlgorithm, publicJsonWebKeyData.ext, publicJsonWebKeyData.key_ops)
      }
      let error = await assertRejects(test)
      if (error) { return error; }
      try {
        await test()
      } catch (thrownError) {
        let error = assert(thrownError, sentinel, 'thrownError === sentinel')
        if (error) { return error; }
      }
      return pass();
    });
    routes.set("/crypto.subtle.importKey/third-parameter-hash-algorithm-does-not-match-json-web-key-hash-algorithm", async () => {
      let error = await assertRejects(async () => {
        jsonWebKeyAlgorithm.hash.name = "SHA-1";
        await crypto.subtle.importKey('jwk', publicJsonWebKeyData, jsonWebKeyAlgorithm, publicJsonWebKeyData.ext, publicJsonWebKeyData.key_ops)
      }, Error, "The JWK 'alg' member was inconsistent with that specified by the Web Crypto call")
      if (error) { return error; }
      return pass();
    });
  }

  // fifth-parameter
  {
    routes.set("/crypto.subtle.importKey/fifth-parameter-undefined", async () => {
      let error = await assertRejects(async () => {
        await crypto.subtle.importKey('jwk', publicJsonWebKeyData, jsonWebKeyAlgorithm, publicJsonWebKeyData.ext, undefined)
      }, Error, "The provided value cannot be converted to a sequence")
      if (error) { return error; }
      return pass();
    });
    routes.set("/crypto.subtle.importKey/fifth-parameter-invalid", async () => {
      let error = await assertRejects(async () => {
        await crypto.subtle.importKey('jwk', publicJsonWebKeyData, jsonWebKeyAlgorithm, publicJsonWebKeyData.ext, ["jake"])
      }, Error, "SubtleCrypto.importKey: Invalid keyUsages argument")
      if (error) { return error; }
      return pass();
    });
    routes.set("/crypto.subtle.importKey/fifth-parameter-duplicate-operations", async () => {
      let error = await assertResolves(async () => {
        const key_ops = publicJsonWebKeyData.key_ops.concat(publicJsonWebKeyData.key_ops);
        await crypto.subtle.importKey('jwk', publicJsonWebKeyData, jsonWebKeyAlgorithm, publicJsonWebKeyData.ext, key_ops)
      })
      if (error) { return error; }
      return pass();
    });

    routes.set("/crypto.subtle.importKey/fifth-parameter-operations-do-not-match-json-web-key-operations", async () => {
      let error = await assertRejects(async () => {
        await crypto.subtle.importKey('jwk', publicJsonWebKeyData, jsonWebKeyAlgorithm, publicJsonWebKeyData.ext, ["sign"])
      }, Error, "The JWK 'key_ops' member was inconsistent with that specified by the Web Crypto call. The JWK usage must be a superset of those requested")
      if (error) { return error; }
      return pass();
    });

    routes.set("/crypto.subtle.importKey/fifth-parameter-operation-fields-calls-7.1.17-ToString", async () => {
      let sentinel = Symbol("sentinel");
      const test = async () => {
        sentinel = Symbol();
        const op = {
          toString() {
            throw sentinel;
          }
        }
        await crypto.subtle.importKey('jwk', publicJsonWebKeyData, jsonWebKeyAlgorithm, publicJsonWebKeyData.ext, ["sign", op])
      }
      let error = await assertRejects(test)
      if (error) { return error; }
      try {
        await test()
      } catch (thrownError) {
        let error = assert(thrownError, sentinel, 'thrownError === sentinel')
        if (error) { return error; }
      }
      return pass();
    });
  }

  // happy paths
  {
    routes.set("/crypto.subtle.importKey/JWK-RS256-Public", async () => {
      const key = await crypto.subtle.importKey('jwk', publicJsonWebKeyData, jsonWebKeyAlgorithm, publicJsonWebKeyData.ext, publicJsonWebKeyData.key_ops);
      error = await assert(key instanceof CryptoKey, true, `key instanceof CryptoKey`);
      if (error) { return error; }
      error = await assert(key.algorithm, {
        "name": "RSASSA-PKCS1-v1_5", 
        "hash": {
          "name": "SHA-256"
        },
        "modulusLength":2048,
        "publicExponent": new Uint8Array([1,0,1])
      }, `key.algorithm deep equals "carrot"`);
      if (error) { return error; }
      error = await assert(key.extractable, true, `key.extractable === true`);
      if (error) { return error; }
      error = await assert(key.type, "public", `key.type === "public"`);
      if (error) { return error; }
      error = await assert(key.usages, ["verify"], `key.usages deep equals ["verify"]`);
      if (error) { return error; }
      return pass();
    });
  }
}

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

// sign
{
  const enc = new TextEncoder();
  const data = enc.encode('hello world');
  routes.set("/crypto.subtle.sign", async () => {
    error = assert(typeof crypto.subtle.sign, 'function', `typeof crypto.subtle.sign`)
    if (error) { return error; }
    error = assert(crypto.subtle.sign, SubtleCrypto.prototype.sign, `crypto.subtle.sign === SubtleCrypto.prototype.sign`)
    if (error) { return error; }
    return pass();
  });
  routes.set("/crypto.subtle.sign/length", async () => {
    error = assert(crypto.subtle.sign.length, 3, `crypto.subtle.sign.length === 3`)
    if (error) { return error; }
    return pass();
  });
  routes.set("/crypto.subtle.sign/called-as-constructor", async () => {
    error = assertThrows(() => {
      new crypto.subtle.sign
    }, TypeError, "crypto.subtle.sign is not a constructor")
    if (error) { return error; }
    return pass();
  });
  routes.set("/crypto.subtle.sign/called-with-wrong-this", async () => {
    error = await assertRejects(async () => {
      await crypto.subtle.sign.call(undefined, jsonWebKeyAlgorithm, publicJsonWebKeyData, data)
    }, TypeError, "Method SubtleCrypto.sign called on receiver that's not an instance of SubtleCrypto")
    if (error) { return error; }
    return pass();
  });
  routes.set("/crypto.subtle.sign/called-with-no-arguments", async () => {
    error = await assertRejects(async () => {
      await crypto.subtle.sign()
    }, TypeError, "SubtleCrypto.sign: At least 3 arguments required, but only 0 passed")
    if (error) { return error; }
    return pass();
  });
  // first-parameter
  {
    routes.set("/crypto.subtle.sign/first-parameter-calls-7.1.17-ToString", async () => {
      const sentinel = Symbol("sentinel");
      const key = await crypto.subtle.importKey('jwk', privateJsonWebKeyData, jsonWebKeyAlgorithm, privateJsonWebKeyData.ext, privateJsonWebKeyData.key_ops);
      const test = async () => {
        await crypto.subtle.sign({
          name: {
            toString() {
              throw sentinel;
            }
          }
        }, key, data);
      }
      let error = await assertRejects(test)
      if (error) { return error; }
      try {
        await test()
      } catch (thrownError) {
        let error = assert(thrownError, sentinel, 'thrownError === sentinel')
        if (error) { return error; }
      }
      return pass();
    });
    routes.set("/crypto.subtle.sign/first-parameter-non-existant-algorithm", async () => {
      let error = await assertRejects(async () => {
        const key = await crypto.subtle.importKey('jwk', privateJsonWebKeyData, jsonWebKeyAlgorithm, privateJsonWebKeyData.ext, privateJsonWebKeyData.key_ops);
        await crypto.subtle.sign('jake', key, data)
      }, Error, "Algorithm: Unrecognized name")
      if (error) { return error; }
      return pass();
    });
  }
  // second-parameter
  {
    routes.set("/crypto.subtle.sign/second-parameter-invalid-format", async () => {
      let error = await assertRejects(async () => {
        await crypto.subtle.sign(jsonWebKeyAlgorithm, "jake", data)
      }, Error, "parameter 2 is not of type 'CryptoKey'")
      if (error) { return error; }
      return pass();
    });
    routes.set("/crypto.subtle.sign/second-parameter-invalid-usages", async () => {
      let error = await assertRejects(async () => {
        const key = await crypto.subtle.importKey('jwk', publicJsonWebKeyData, jsonWebKeyAlgorithm, publicJsonWebKeyData.ext, publicJsonWebKeyData.key_ops);
        await crypto.subtle.sign(jsonWebKeyAlgorithm, key, data);
      }, Error, "CryptoKey doesn't support signing")
      if (error) { return error; }
      return pass();
    });
  }
  // third-parameter
  {
    routes.set("/crypto.subtle.sign/third-parameter-invalid-format", async () => {
      let error = await assertRejects(async () => {
        const key = await crypto.subtle.importKey('jwk', publicJsonWebKeyData, jsonWebKeyAlgorithm, publicJsonWebKeyData.ext, publicJsonWebKeyData.key_ops);
        await crypto.subtle.sign(jsonWebKeyAlgorithm, key, undefined)
      }, Error, "SubtleCrypto.sign: data must be of type ArrayBuffer or ArrayBufferView but got \"\"")
      if (error) { return error; }
      return pass();
    });
  }
  // happy-path
  {
    routes.set("/crypto.subtle.sign/happy-path", async () => {
      const key = await crypto.subtle.importKey('jwk', privateJsonWebKeyData, jsonWebKeyAlgorithm, privateJsonWebKeyData.ext, privateJsonWebKeyData.key_ops);
      const signature = new Uint8Array(await crypto.subtle.sign(jsonWebKeyAlgorithm, key, data));
      const expected = new Uint8Array([70,96,33,185,93,42,67,49,243,70,88,68,194,148,53,249,255,192,232,132,161,194,41,244,174,211,218,203,7,238,71,182,101,49,139,222,165,70,222,105,82,156,184,44,100,108,121,237,250,119,66,228,156,243,71,105,62,246,22,2,160,116,71,147,202,168,24,92,224,41,148,161,124,80,212,169,212,64,29,189,2,171,174,188,159,89,93,122,219,166,105,92,107,173,103,238,145,226,94,139,71,124,17,233,49,138,89,246,3,82,238,154,169,188,66,198,32,23,230,90,164,140,51,47,221,149,161,14,254,169,224,223,119,94,27,63,199,93,65,53,24,151,146,242,239,41,108,136,31,99,42,213,128,244,140,238,157,107,117,241,219,137,97,39,109,185,176,97,193,60,117,244,106,62,193,188,87,199,37,70,137,37,231,110,228,228,139,53,240,56,92,102,220,176,127,248,24,217,208,29,209,216,29,251,100,252,243,183,195,96,126,102,136,48,39,186,45,202,10,187,22,52,183,190,149,153,32,12,90,66,49,122,190,154,167,9,12,32,77,177,222,54,211,233,219,205,133,0,113,77,158,1,125,5,15,195]);
      error = assert(signature, expected, "signature deep equals expected");
      if (error) { return error; }
      return pass();
    });
  }
}