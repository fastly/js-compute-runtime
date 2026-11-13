/// <reference path="../../../../../types/index.d.ts" />
/* eslint-env serviceworker, shared-node-browser, browser */

import {
  assert,
  assertThrows,
  assertRejects,
  assertResolves,
} from './assertions.js';
import { routes } from './routes.js';

// From https://www.rfc-editor.org/rfc/rfc7517#appendix-A.1
const publicRsaJsonWebKeyData = {
  alg: 'RS256',
  e: 'AQAB',
  ext: true,
  key_ops: ['verify'],
  kty: 'RSA',
  n: '0vx7agoebGcQSuuPiLJXZptN9nndrQmbXEps2aiAFbWhM78LhWx4cbbfAAtVT86zwu1RK7aPFFxuhDR1L6tSoc_BJECPebWKRXjBZCiFV4n3oknjhMstn64tZ_2W-5JsGY4Hc5n9yBXArwl93lqt7_RN5w6Cf0h4QyQ5v-65YGjQR0_FDW2QvzqY368QQMicAtaSqzs8KJZgnYb9c7d0zgdAZHzu6qMQvRL5hajrn1n91CbOpbISD08qNLyrdkt-bFTWhAI4vMQFh6WeZu0fM4lFd2NcRwr3XPksINHaQ-G_xBniIqbw0Ls1jF44-csFCur-kEgU8awapJzKnqDKgw',
};

// From https://www.rfc-editor.org/rfc/rfc7517#appendix-A.1
const publicEcdsaJsonWebKeyData = {
  kty: 'EC',
  crv: 'P-256',
  x: 'MKBCTNIcKUSDii11ySs3526iDZ8AiTo7Tu6KPAqv7D4',
  y: '4Etl6SRW2YiLUrN5vfvVHuhp7x8PxltmWWlbbM4IFyM',
  kid: '1',
  ext: true,
  key_ops: ['verify'],
};

// From https://www.rfc-editor.org/rfc/rfc7517#appendix-A.2
const privateEcdsaJsonWebKeyData = {
  kty: 'EC',
  crv: 'P-256',
  x: 'MKBCTNIcKUSDii11ySs3526iDZ8AiTo7Tu6KPAqv7D4',
  y: '4Etl6SRW2YiLUrN5vfvVHuhp7x8PxltmWWlbbM4IFyM',
  d: '870MB6gfuTJ4HtUnUvYMyJpr5eUZNP4Bk43bVdj3eAE',
  use: 'sig',
  kid: '1',
  ext: true,
  key_ops: ['sign'],
};

// From https://www.rfc-editor.org/rfc/rfc7517#appendix-A.2
const privateRsaJsonWebKeyData = {
  alg: 'RS256',
  d: 'X4cTteJY_gn4FYPsXB8rdXix5vwsg1FLN5E3EaG6RJoVH-HLLKD9M7dx5oo7GURknchnrRweUkC7hT5fJLM0WbFAKNLWY2vv7B6NqXSzUvxT0_YSfqijwp3RTzlBaCxWp4doFk5N2o8Gy_nHNKroADIkJ46pRUohsXywbReAdYaMwFs9tv8d_cPVY3i07a3t8MN6TNwm0dSawm9v47UiCl3Sk5ZiG7xojPLu4sbg1U2jx4IBTNBznbJSzFHK66jT8bgkuqsk0GjskDJk19Z4qwjwbsnn4j2WBii3RL-Us2lGVkY8fkFzme1z0HbIkfz0Y6mqnOYtqc0X4jfcKoAC8Q',
  dp: 'G4sPXkc6Ya9y8oJW9_ILj4xuppu0lzi_H7VTkS8xj5SdX3coE0oimYwxIi2emTAue0UOa5dpgFGyBJ4c8tQ2VF402XRugKDTP8akYhFo5tAA77Qe_NmtuYZc3C3m3I24G2GvR5sSDxUyAN2zq8Lfn9EUms6rY3Ob8YeiKkTiBj0',
  dq: 's9lAH9fggBsoFR8Oac2R_E2gw282rT2kGOAhvIllETE1efrA6huUUvMfBcMpn8lqeW6vzznYY5SSQF7pMdC_agI3nG8Ibp1BUb0JUiraRNqUfLhcQb_d9GF4Dh7e74WbRsobRonujTYN1xCaP6TO61jvWrX-L18txXw494Q_cgk',
  e: 'AQAB',
  ext: true,
  key_ops: ['sign'],
  kty: 'RSA',
  n: '0vx7agoebGcQSuuPiLJXZptN9nndrQmbXEps2aiAFbWhM78LhWx4cbbfAAtVT86zwu1RK7aPFFxuhDR1L6tSoc_BJECPebWKRXjBZCiFV4n3oknjhMstn64tZ_2W-5JsGY4Hc5n9yBXArwl93lqt7_RN5w6Cf0h4QyQ5v-65YGjQR0_FDW2QvzqY368QQMicAtaSqzs8KJZgnYb9c7d0zgdAZHzu6qMQvRL5hajrn1n91CbOpbISD08qNLyrdkt-bFTWhAI4vMQFh6WeZu0fM4lFd2NcRwr3XPksINHaQ-G_xBniIqbw0Ls1jF44-csFCur-kEgU8awapJzKnqDKgw',
  p: '83i-7IvMGXoMXCskv73TKr8637FiO7Z27zv8oj6pbWUQyLPQBQxtPVnwD20R-60eTDmD2ujnMt5PoqMrm8RfmNhVWDtjjMmCMjOpSXicFHj7XOuVIYQyqVWlWEh6dN36GVZYk93N8Bc9vY41xy8B9RzzOGVQzXvNEvn7O0nVbfs',
  q: '3dfOR9cuYq-0S-mkFLzgItgMEfFzB2q3hWehMuG0oCuqnb3vobLyumqjVZQO1dIrdwgTnCdpYzBcOfW5r370AFXjiWft_NGEiovonizhKpo9VVS78TzFgxkIdrecRezsZ-1kYd_s1qDbxtkDEgfAITAG9LUnADun4vIcb6yelxk',
  qi: 'GyM_p6JrXySiz1toFgKbWV-JdI3jQ4ypu9rbMWx3rQJBfmt0FoYzgUIZEVFEcOqwemRN81zoDAaa-Bk0KWNGDjJHZDdDmFhW3AN7lI-puxk_mHZGJ11rxyR8O55XLSe3SPmRfKwZI6yU24ZxvQKFYItdldUKGzO6Ia6zTKhAVRU',
};

// Helper functions to create fresh copies of test data for tests that mutate them
function freshPublicRsaKey() {
  return structuredClone(publicRsaJsonWebKeyData);
}
function freshPublicEcdsaKey() {
  return structuredClone(publicEcdsaJsonWebKeyData);
}
function freshPrivateEcdsaKey() {
  return structuredClone(privateEcdsaJsonWebKeyData);
}
function freshPrivateRsaKey() {
  return structuredClone(privateRsaJsonWebKeyData);
}

const rsaJsonWebKeyAlgorithm = {
  name: 'RSASSA-PKCS1-v1_5',
  hash: { name: 'SHA-256' },
};

const ecdsaJsonWebKeyAlgorithm = {
  name: 'ECDSA',
  namedCurve: 'P-256',
  hash: { name: 'SHA-256' },
};

routes.set('/crypto', async () => {
  assert(typeof crypto, 'object', `typeof crypto`);
  assert(crypto instanceof Crypto, true, `crypto instanceof Crypto`);
});

routes.set('/crypto.subtle', async () => {
  assert(typeof crypto.subtle, 'object', `typeof crypto.subtle`);
  assert(
    crypto.subtle instanceof SubtleCrypto,
    true,
    `crypto.subtle instanceof SubtleCrypto`,
  );
});

// importKey
{
  routes.set('/crypto.subtle.importKey', async () => {
    assert(
      typeof crypto.subtle.importKey,
      'function',
      `typeof crypto.subtle.importKey`,
    );
    assert(
      crypto.subtle.importKey,
      SubtleCrypto.prototype.importKey,
      `crypto.subtle.importKey === SubtleCrypto.prototype.importKey`,
    );
  });
  routes.set('/crypto.subtle.importKey/length', async () => {
    assert(
      crypto.subtle.importKey.length,
      5,
      `crypto.subtle.importKey.length === 5`,
    );
  });
  routes.set('/crypto.subtle.importKey/called-as-constructor', async () => {
    assertThrows(
      () => {
        new crypto.subtle.importKey();
      },
      TypeError,
      'crypto.subtle.importKey is not a constructor',
    );
  });
  routes.set('/crypto.subtle.importKey/called-with-wrong-this', async () => {
    await assertRejects(async () => {
      await crypto.subtle.importKey.call(
        undefined,
        'jwk',
        publicRsaJsonWebKeyData,
        rsaJsonWebKeyAlgorithm,
        publicRsaJsonWebKeyData.ext,
        publicRsaJsonWebKeyData.key_ops,
      );
    }, TypeError);
  });
  routes.set('/crypto.subtle.importKey/called-with-no-arguments', async () => {
    await assertRejects(
      async () => {
        await crypto.subtle.importKey();
      },
      TypeError,
      'SubtleCrypto.importKey: At least 5 arguments required, but only 0 passed',
    );
  });

  // first-parameter
  {
    routes.set(
      '/crypto.subtle.importKey/first-parameter-calls-7.1.17-ToString',
      async () => {
        const sentinel = Symbol('sentinel');
        const test = async () => {
          const format = {
            toString() {
              throw sentinel;
            },
          };
          await crypto.subtle.importKey(
            format,
            publicRsaJsonWebKeyData,
            rsaJsonWebKeyAlgorithm,
            publicRsaJsonWebKeyData.ext,
            publicRsaJsonWebKeyData.key_ops,
          );
        };
        await assertRejects(test);
        try {
          await test();
        } catch (thrownError) {
          assert(thrownError, sentinel, 'thrownError === sentinel');
        }
      },
    );
    routes.set(
      '/crypto.subtle.importKey/first-parameter-non-existant-format',
      async () => {
        await assertRejects(async () => {
          await crypto.subtle.importKey(
            'jake',
            publicRsaJsonWebKeyData,
            rsaJsonWebKeyAlgorithm,
            publicRsaJsonWebKeyData.ext,
            publicRsaJsonWebKeyData.key_ops,
          );
        }, Error);
      },
    );
  }

  // second-parameter
  {
    routes.set(
      '/crypto.subtle.importKey/second-parameter-invalid-format',
      async () => {
        await assertRejects(async () => {
          await crypto.subtle.importKey(
            'jwk',
            Symbol(),
            rsaJsonWebKeyAlgorithm,
            publicRsaJsonWebKeyData.ext,
            publicRsaJsonWebKeyData.key_ops,
          );
        }, Error);
      },
    );
    // jwk public key
    {
      routes.set(
        '/crypto.subtle.importKey/rsa-jwk-public/second-parameter-missing-e-field',
        async () => {
          await assertRejects(async () => {
            const testData = structuredClone(publicRsaJsonWebKeyData);
            delete testData.e;
            await crypto.subtle.importKey(
              'jwk',
              testData,
              rsaJsonWebKeyAlgorithm,
              testData.ext,
              testData.key_ops,
            );
          }, DOMException);
        },
      );
      routes.set(
        '/crypto.subtle.importKey/rsa-jwk-public/second-parameter-e-field-calls-7.1.17-ToString',
        async () => {
          let sentinel = Symbol('sentinel');
          const test = async () => {
            sentinel = Symbol();
            let webKeyData = structuredClone(publicRsaJsonWebKeyData);
            webKeyData.e = {
              toString() {
                throw sentinel;
              },
            };
            await crypto.subtle.importKey(
              'jwk',
              webKeyData,
              rsaJsonWebKeyAlgorithm,
              webKeyData.ext,
              webKeyData.key_ops,
            );
          };
          await assertRejects(test);
          try {
            await test();
          } catch (thrownError) {
            assert(thrownError, sentinel, 'thrownError === sentinel');
          }
        },
      );
      routes.set(
        '/crypto.subtle.importKey/rsa-jwk-public/second-parameter-invalid-e-field',
        async () => {
          await assertRejects(
            async () => {
              let webKeyData = structuredClone(publicRsaJsonWebKeyData);
              webKeyData.e = '`~!@#@#$Q%^%&^*';
              await crypto.subtle.importKey(
                'jwk',
                webKeyData,
                rsaJsonWebKeyAlgorithm,
                webKeyData.ext,
                webKeyData.key_ops,
              );
            },
            Error,
            "The JWK member 'e' could not be base64url decoded or contained padding",
          );
        },
      );
      routes.set(
        '/crypto.subtle.importKey/rsa-jwk-public/second-parameter-missing-kty-field',
        async () => {
          await assertRejects(async () => {
            let webKeyData = structuredClone(publicRsaJsonWebKeyData);
            delete webKeyData.kty;
            await crypto.subtle.importKey(
              'jwk',
              webKeyData,
              rsaJsonWebKeyAlgorithm,
              webKeyData.ext,
              webKeyData.key_ops,
            );
          }, Error);
        },
      );
      routes.set(
        '/crypto.subtle.importKey/rsa-jwk-public/second-parameter-invalid-kty-field',
        async () => {
          await assertRejects(async () => {
            let webKeyData = structuredClone(publicRsaJsonWebKeyData);
            webKeyData.kty = 'jake';
            await crypto.subtle.importKey(
              'jwk',
              webKeyData,
              rsaJsonWebKeyAlgorithm,
              webKeyData.ext,
              webKeyData.key_ops,
            );
          }, Error);
        },
      );
      routes.set(
        '/crypto.subtle.importKey/rsa-jwk-public/second-parameter-missing-key_ops-field',
        async () => {
          await assertResolves(async () => {
            let webKeyData = structuredClone(publicRsaJsonWebKeyData);
            const key_ops = Array.from(webKeyData.key_ops);
            delete webKeyData.key_ops;
            await crypto.subtle.importKey(
              'jwk',
              webKeyData,
              rsaJsonWebKeyAlgorithm,
              webKeyData.ext,
              key_ops,
            );
          });
        },
      );
      routes.set(
        '/crypto.subtle.importKey/rsa-jwk-public/second-parameter-non-sequence-key_ops-field',
        async () => {
          await assertRejects(async () => {
            let webKeyData = structuredClone(publicRsaJsonWebKeyData);
            const key_ops = Array.from(webKeyData.key_ops);
            webKeyData.key_ops = 'jake';
            await crypto.subtle.importKey(
              'jwk',
              webKeyData,
              rsaJsonWebKeyAlgorithm,
              webKeyData.ext,
              key_ops,
            );
          }, Error);
        },
      );

      routes.set(
        '/crypto.subtle.importKey/rsa-jwk-public/second-parameter-empty-key_ops-field',
        async () => {
          await assertRejects(async () => {
            let webKeyData = structuredClone(publicRsaJsonWebKeyData);
            const key_ops = Array.from(webKeyData.key_ops);
            webKeyData.key_ops = [];
            await crypto.subtle.importKey(
              'jwk',
              webKeyData,
              rsaJsonWebKeyAlgorithm,
              webKeyData.ext,
              key_ops,
            );
          }, DOMException);
        },
      );
      routes.set(
        '/crypto.subtle.importKey/rsa-jwk-public/second-parameter-duplicated-key_ops-field',
        async () => {
          await assertRejects(async () => {
            let webKeyData = structuredClone(publicRsaJsonWebKeyData);
            const key_ops = Array.from(webKeyData.key_ops);
            webKeyData.key_ops = ['sign', 'sign'];
            await crypto.subtle.importKey(
              'jwk',
              webKeyData,
              rsaJsonWebKeyAlgorithm,
              webKeyData.ext,
              key_ops,
            );
          }, Error);
        },
      );
      routes.set(
        '/crypto.subtle.importKey/rsa-jwk-public/second-parameter-invalid-key_ops-field',
        async () => {
          await assertRejects(async () => {
            let webKeyData = structuredClone(publicRsaJsonWebKeyData);
            const key_ops = Array.from(webKeyData.key_ops);
            webKeyData.key_ops = ['sign', 'jake'];
            await crypto.subtle.importKey(
              'jwk',
              webKeyData,
              rsaJsonWebKeyAlgorithm,
              webKeyData.ext,
              key_ops,
            );
          }, Error);
        },
      );

      routes.set(
        '/crypto.subtle.importKey/rsa-jwk-public/second-parameter-key_ops-field-calls-7.1.17-ToString',
        async () => {
          let sentinel = Symbol('sentinel');
          const key_ops = Array.from(publicRsaJsonWebKeyData.key_ops);
          const test = async () => {
            sentinel = Symbol();
            const op = {
              toString() {
                throw sentinel;
              },
            };
            let webKeyData = structuredClone(publicRsaJsonWebKeyData);
            const key_ops = Array.from(webKeyData.key_ops);
            webKeyData.key_ops = ['sign', op];
            await crypto.subtle.importKey(
              'jwk',
              webKeyData,
              rsaJsonWebKeyAlgorithm,
              webKeyData.ext,
              key_ops,
            );
          };
          await assertRejects(test);
          try {
            await test();
          } catch (thrownError) {
            assert(thrownError, sentinel, 'thrownError === sentinel');
          }
        },
      );

      routes.set(
        '/crypto.subtle.importKey/rsa-jwk-public/second-parameter-missing-n-field',
        async () => {
          await assertRejects(
            async () => {
              const testData = structuredClone(publicRsaJsonWebKeyData);
              delete testData.n;
              await crypto.subtle.importKey(
                'jwk',
                testData,
                rsaJsonWebKeyAlgorithm,
                testData.ext,
                testData.key_ops,
              );
            },
            Error,
            'Data provided to an operation does not meet requirements',
          );
        },
      );
      routes.set(
        '/crypto.subtle.importKey/rsa-jwk-public/second-parameter-n-field-calls-7.1.17-ToString',
        async () => {
          let sentinel = Symbol('sentinel');
          const test = async () => {
            sentinel = Symbol();
            let webKeyData = structuredClone(publicRsaJsonWebKeyData);
            webKeyData.n = {
              toString() {
                throw sentinel;
              },
            };
            await crypto.subtle.importKey(
              'jwk',
              webKeyData,
              rsaJsonWebKeyAlgorithm,
              webKeyData.ext,
              webKeyData.key_ops,
            );
          };
          await assertRejects(test);
          try {
            await test();
          } catch (thrownError) {
            assert(thrownError, sentinel, 'thrownError === sentinel');
          }
        },
      );
      routes.set(
        '/crypto.subtle.importKey/rsa-jwk-public/second-parameter-invalid-n-field',
        async () => {
          await assertRejects(
            async () => {
              let webKeyData = structuredClone(publicRsaJsonWebKeyData);
              webKeyData.n = '`~!@#@#$Q%^%&^*';
              await crypto.subtle.importKey(
                'jwk',
                webKeyData,
                rsaJsonWebKeyAlgorithm,
                webKeyData.ext,
                webKeyData.key_ops,
              );
            },
            Error,
            "The JWK member 'n' could not be base64url decoded or contained padding",
          );
        },
      );
    }
    // jwk private key
    // TODO
    // raw HMAC secret keys
    // TODO
    // raw Elliptic Curve public keys
    {
      routes.set(
        '/crypto.subtle.importKey/ecdsa-jwk-public/second-parameter-missing-x-field',
        async () => {
          await assertRejects(
            async () => {
              let webKeyData = structuredClone(publicEcdsaJsonWebKeyData);
              delete webKeyData.x;
              await crypto.subtle.importKey(
                'jwk',
                webKeyData,
                ecdsaJsonWebKeyAlgorithm,
                webKeyData.ext,
                webKeyData.key_ops,
              );
            },
            DOMException,
            'Data provided to an operation does not meet requirements',
          );
        },
      );
      routes.set(
        '/crypto.subtle.importKey/ecdsa-jwk-public/second-parameter-x-field-calls-7.1.17-ToString',
        async () => {
          let sentinel = Symbol('sentinel');
          const test = async () => {
            sentinel = Symbol();
            let webKeyData = structuredClone(publicEcdsaJsonWebKeyData);
            webKeyData.x = {
              toString() {
                throw sentinel;
              },
            };
            await crypto.subtle.importKey(
              'jwk',
              webKeyData,
              ecdsaJsonWebKeyAlgorithm,
              webKeyData.ext,
              webKeyData.key_ops,
            );
          };
          await assertRejects(test);
          try {
            await test();
          } catch (thrownError) {
            assert(thrownError, sentinel, 'thrownError === sentinel');
          }
        },
      );
      routes.set(
        '/crypto.subtle.importKey/ecdsa-jwk-public/second-parameter-invalid-x-field',
        async () => {
          await assertRejects(
            async () => {
              let webKeyData = structuredClone(publicEcdsaJsonWebKeyData);
              webKeyData.x = '`~!@#@#$Q%^%&^*';
              await crypto.subtle.importKey(
                'jwk',
                webKeyData,
                ecdsaJsonWebKeyAlgorithm,
                webKeyData.ext,
                webKeyData.key_ops,
              );
            },
            Error,
            "The JWK member 'x' could not be base64url decoded or contained padding",
          );
        },
      );
      routes.set(
        '/crypto.subtle.importKey/ecdsa-jwk-public/second-parameter-missing-y-field',
        async () => {
          await assertRejects(
            async () => {
              let webKeyData = structuredClone(publicEcdsaJsonWebKeyData);
              delete webKeyData.y;
              await crypto.subtle.importKey(
                'jwk',
                webKeyData,
                ecdsaJsonWebKeyAlgorithm,
                webKeyData.ext,
                webKeyData.key_ops,
              );
            },
            DOMException,
            'Data provided to an operation does not meet requirements',
          );
        },
      );
      routes.set(
        '/crypto.subtle.importKey/ecdsa-jwk-public/second-parameter-y-field-calls-7.1.17-ToString',
        async () => {
          let sentinel = Symbol('sentinel');
          const test = async () => {
            sentinel = Symbol();
            let webKeyData = structuredClone(publicEcdsaJsonWebKeyData);
            webKeyData.y = {
              toString() {
                throw sentinel;
              },
            };
            await crypto.subtle.importKey(
              'jwk',
              webKeyData,
              ecdsaJsonWebKeyAlgorithm,
              webKeyData.ext,
              webKeyData.key_ops,
            );
          };
          await assertRejects(test);
          try {
            await test();
          } catch (thrownError) {
            assert(thrownError, sentinel, 'thrownError === sentinel');
          }
        },
      );
      routes.set(
        '/crypto.subtle.importKey/ecdsa-jwk-public/second-parameter-invalid-y-field',
        async () => {
          await assertRejects(
            async () => {
              let webKeyData = structuredClone(publicEcdsaJsonWebKeyData);
              webKeyData.y = '`~!@#@#$Q%^%&^*';
              await crypto.subtle.importKey(
                'jwk',
                webKeyData,
                ecdsaJsonWebKeyAlgorithm,
                webKeyData.ext,
                webKeyData.key_ops,
              );
            },
            Error,
            "The JWK member 'y' could not be base64url decoded or contained padding",
          );
        },
      );
      routes.set(
        '/crypto.subtle.importKey/ecdsa-jwk-public/second-parameter-missing-kty-field',
        async () => {
          await assertRejects(async () => {
            let webKeyData = structuredClone(publicEcdsaJsonWebKeyData);
            delete webKeyData.kty;
            await crypto.subtle.importKey(
              'jwk',
              webKeyData,
              ecdsaJsonWebKeyAlgorithm,
              webKeyData.ext,
              webKeyData.key_ops,
            );
          }, Error);
        },
      );
      routes.set(
        '/crypto.subtle.importKey/ecdsa-jwk-public/second-parameter-invalid-kty-field',
        async () => {
          await assertRejects(async () => {
            let webKeyData = structuredClone(publicEcdsaJsonWebKeyData);
            webKeyData.kty = 'jake';
            await crypto.subtle.importKey(
              'jwk',
              webKeyData,
              ecdsaJsonWebKeyAlgorithm,
              webKeyData.ext,
              webKeyData.key_ops,
            );
          }, Error);
        },
      );
      routes.set(
        '/crypto.subtle.importKey/ecdsa-jwk-public/second-parameter-missing-key_ops-field',
        async () => {
          await assertResolves(async () => {
            let webKeyData = structuredClone(publicEcdsaJsonWebKeyData);
            const key_ops = Array.from(webKeyData.key_ops);
            delete webKeyData.key_ops;
            await crypto.subtle.importKey(
              'jwk',
              webKeyData,
              ecdsaJsonWebKeyAlgorithm,
              webKeyData.ext,
              key_ops,
            );
          });
        },
      );
      routes.set(
        '/crypto.subtle.importKey/ecdsa-jwk-public/second-parameter-non-sequence-key_ops-field',
        async () => {
          await assertRejects(async () => {
            let webKeyData = structuredClone(publicEcdsaJsonWebKeyData);
            const key_ops = Array.from(webKeyData.key_ops);
            webKeyData.key_ops = 'jake';
            await crypto.subtle.importKey(
              'jwk',
              webKeyData,
              ecdsaJsonWebKeyAlgorithm,
              webKeyData.ext,
              key_ops,
            );
          }, Error);
        },
      );

      routes.set(
        '/crypto.subtle.importKey/ecdsa-jwk-public/second-parameter-empty-key_ops-field',
        async () => {
          await assertRejects(async () => {
            let webKeyData = structuredClone(publicEcdsaJsonWebKeyData);
            const key_ops = Array.from(webKeyData.key_ops);
            webKeyData.key_ops = [];
            await crypto.subtle.importKey(
              'jwk',
              webKeyData,
              ecdsaJsonWebKeyAlgorithm,
              webKeyData.ext,
              key_ops,
            );
          }, DOMException);
        },
      );
      routes.set(
        '/crypto.subtle.importKey/ecdsa-jwk-public/second-parameter-duplicated-key_ops-field',
        async () => {
          await assertRejects(async () => {
            const testData = structuredClone(publicEcdsaJsonWebKeyData);
            const key_ops = Array.from(testData.key_ops);
            testData.key_ops = ['sign', 'sign'];
            await crypto.subtle.importKey(
              'jwk',
              testData,
              ecdsaJsonWebKeyAlgorithm,
              testData.ext,
              key_ops,
            );
          }, Error);
        },
      );
      routes.set(
        '/crypto.subtle.importKey/ecdsa-jwk-public/second-parameter-invalid-key_ops-field',
        async () => {
          await assertRejects(async () => {
            const testData = structuredClone(publicEcdsaJsonWebKeyData);
            const key_ops = Array.from(testData.key_ops);
            testData.key_ops = ['sign', 'jake'];
            await crypto.subtle.importKey(
              'jwk',
              testData,
              ecdsaJsonWebKeyAlgorithm,
              testData.ext,
              key_ops,
            );
          }, Error);
        },
      );

      routes.set(
        '/crypto.subtle.importKey/ecdsa-jwk-public/second-parameter-key_ops-field-calls-7.1.17-ToString',
        async () => {
          let sentinel = Symbol('sentinel');
          const test = async () => {
            sentinel = Symbol();
            const testData = structuredClone(publicEcdsaJsonWebKeyData);
            const key_ops = Array.from(testData.key_ops);
            const op = {
              toString() {
                throw sentinel;
              },
            };
            testData.key_ops = ['sign', op];
            await crypto.subtle.importKey(
              'jwk',
              testData,
              ecdsaJsonWebKeyAlgorithm,
              testData.ext,
              key_ops,
            );
          };
          await assertRejects(test);
          try {
            await test();
          } catch (thrownError) {
            assert(thrownError, sentinel, 'thrownError === sentinel');
          }
        },
      );

      routes.set(
        '/crypto.subtle.importKey/ecdsa-jwk-private/second-parameter-d-field-calls-7.1.17-ToString',
        async () => {
          let sentinel = Symbol('sentinel');
          const test = async () => {
            sentinel = Symbol();
            const testData = structuredClone(privateEcdsaJsonWebKeyData);
            testData.d = {
              toString() {
                throw sentinel;
              },
            };
            await crypto.subtle.importKey(
              'jwk',
              testData,
              ecdsaJsonWebKeyAlgorithm,
              testData.ext,
              testData.key_ops,
            );
          };
          await assertRejects(test);
          try {
            await test();
          } catch (thrownError) {
            console.log(
              thrownError,
              typeof thrownError,
              Object.keys(thrownError),
            );
            assert(thrownError, sentinel, 'thrownError === sentinel');
          }
        },
      );
      routes.set(
        '/crypto.subtle.importKey/ecdsa-jwk-private/second-parameter-invalid-d-field',
        async () => {
          await assertRejects(
            async () => {
              const testData = structuredClone(privateEcdsaJsonWebKeyData);
              testData.d = '`~!@#@#$Q%^%&^*';
              await crypto.subtle.importKey(
                'jwk',
                testData,
                ecdsaJsonWebKeyAlgorithm,
                testData.ext,
                testData.key_ops,
              );
            },
            Error,
            "The JWK member 'd' could not be base64url decoded or contained padding",
          );
        },
      );
    }
    // pkcs8 Elliptic Curve private keys
    // TODO
    // pkcs8 RSA private keys
    // raw AES
  }
  // third-parameter
  {
    routes.set(
      '/crypto.subtle.importKey/third-parameter-undefined',
      async () => {
        await assertRejects(
          async () => {
            await crypto.subtle.importKey(
              'jwk',
              publicRsaJsonWebKeyData,
              undefined,
              publicRsaJsonWebKeyData.ext,
              publicRsaJsonWebKeyData.key_ops,
            );
          },
          Error,
          'Algorithm: Unrecognized name',
        );
      },
    );
    routes.set(
      '/crypto.subtle.importKey/third-parameter-name-field-calls-7.1.17-ToString',
      async () => {
        const sentinel = Symbol('sentinel');
        const test = async () => {
          let algorithm = structuredClone(rsaJsonWebKeyAlgorithm);
          algorithm.name = {
            toString() {
              throw sentinel;
            },
          };
          await crypto.subtle.importKey(
            'jwk',
            publicRsaJsonWebKeyData,
            algorithm,
            publicRsaJsonWebKeyData.ext,
            publicRsaJsonWebKeyData.key_ops,
          );
        };
        await assertRejects(test);
        try {
          await test();
        } catch (thrownError) {
          assert(thrownError, sentinel, 'thrownError === sentinel');
        }
      },
    );
    routes.set(
      '/crypto.subtle.importKey/third-parameter-invalid-name-field',
      async () => {
        await assertRejects(async () => {
          let algorithm = structuredClone(rsaJsonWebKeyAlgorithm);
          algorithm.name = '`~!@#@#$Q%^%&^*';
          await crypto.subtle.importKey(
            'jwk',
            publicRsaJsonWebKeyData,
            algorithm,
            publicRsaJsonWebKeyData.ext,
            publicRsaJsonWebKeyData.key_ops,
          );
        }, Error);
      },
    );
    routes.set(
      '/crypto.subtle.importKey/third-parameter-hash-name-field-calls-7.1.17-ToString',
      async () => {
        const sentinel = Symbol('sentinel');
        const test = async () => {
          let algorithm = structuredClone(rsaJsonWebKeyAlgorithm);
          algorithm.hash.name = {
            toString() {
              throw sentinel;
            },
          };
          await crypto.subtle.importKey(
            'jwk',
            publicRsaJsonWebKeyData,
            algorithm,
            publicRsaJsonWebKeyData.ext,
            publicRsaJsonWebKeyData.key_ops,
          );
        };
        await assertRejects(test);
        try {
          await test();
        } catch (thrownError) {
          assert(thrownError, sentinel, 'thrownError === sentinel');
        }
      },
    );
    routes.set(
      '/crypto.subtle.importKey/third-parameter-hash-algorithm-does-not-match-json-web-key-hash-algorithm',
      async () => {
        await assertRejects(
          async () => {
            let algorithm = structuredClone(rsaJsonWebKeyAlgorithm);
            algorithm.hash.name = 'SHA-1';
            await crypto.subtle.importKey(
              'jwk',
              publicRsaJsonWebKeyData,
              algorithm,
              publicRsaJsonWebKeyData.ext,
              publicRsaJsonWebKeyData.key_ops,
            );
          },
          Error,
          "The JWK 'alg' member was inconsistent with that specified by the Web Crypto call",
        );
      },
    );
  }

  // fifth-parameter
  {
    routes.set(
      '/crypto.subtle.importKey/fifth-parameter-undefined',
      async () => {
        await assertRejects(async () => {
          await crypto.subtle.importKey(
            'jwk',
            publicRsaJsonWebKeyData,
            rsaJsonWebKeyAlgorithm,
            publicRsaJsonWebKeyData.ext,
            undefined,
          );
        }, Error);
      },
    );
    routes.set('/crypto.subtle.importKey/fifth-parameter-invalid', async () => {
      await assertRejects(async () => {
        await crypto.subtle.importKey(
          'jwk',
          publicRsaJsonWebKeyData,
          rsaJsonWebKeyAlgorithm,
          publicRsaJsonWebKeyData.ext,
          ['jake'],
        );
      }, Error);
    });
    routes.set(
      '/crypto.subtle.importKey/fifth-parameter-duplicate-operations',
      async () => {
        await assertResolves(async () => {
          const key_ops = publicRsaJsonWebKeyData.key_ops.concat(
            publicRsaJsonWebKeyData.key_ops,
          );
          await crypto.subtle.importKey(
            'jwk',
            publicRsaJsonWebKeyData,
            rsaJsonWebKeyAlgorithm,
            publicRsaJsonWebKeyData.ext,
            key_ops,
          );
        });
      },
    );

    routes.set(
      '/crypto.subtle.importKey/fifth-parameter-operations-do-not-match-json-web-key-operations',
      async () => {
        await assertRejects(
          async () => {
            await crypto.subtle.importKey(
              'jwk',
              publicRsaJsonWebKeyData,
              rsaJsonWebKeyAlgorithm,
              publicRsaJsonWebKeyData.ext,
              ['sign'],
            );
          },
          Error,
          "The JWK 'key_ops' member was inconsistent with that specified by the Web Crypto call. The JWK usage must be a superset of those requested",
        );
      },
    );

    routes.set(
      '/crypto.subtle.importKey/fifth-parameter-operation-fields-calls-7.1.17-ToString',
      async () => {
        let sentinel = Symbol('sentinel');
        const test = async () => {
          sentinel = Symbol();
          const op = {
            toString() {
              throw sentinel;
            },
          };
          await crypto.subtle.importKey(
            'jwk',
            publicRsaJsonWebKeyData,
            rsaJsonWebKeyAlgorithm,
            publicRsaJsonWebKeyData.ext,
            ['sign', op],
          );
        };
        await assertRejects(test);
        try {
          await test();
        } catch (thrownError) {
          assert(thrownError, sentinel, 'thrownError === sentinel');
        }
      },
    );
  }

  // happy paths
  {
    routes.set('/crypto.subtle.importKey/JWK-RS256-Public', async () => {
      const key = await crypto.subtle.importKey(
        'jwk',
        publicRsaJsonWebKeyData,
        rsaJsonWebKeyAlgorithm,
        publicRsaJsonWebKeyData.ext,
        publicRsaJsonWebKeyData.key_ops,
      );
      await assert(key instanceof CryptoKey, true, `key instanceof CryptoKey`);
      await assert(
        key.algorithm,
        {
          name: 'RSASSA-PKCS1-v1_5',
          hash: {
            name: 'SHA-256',
          },
          modulusLength: 2048,
          publicExponent: new Uint8Array([1, 0, 1]),
        },
        `key.algorithm`,
      );
      await assert(key.extractable, true, `key.extractable === true`);
      await assert(key.type, 'public', `key.type === "public"`);
      await assert(key.usages, ['verify'], `key.usages deep equals ["verify"]`);
    });

    routes.set('/crypto.subtle.importKey/JWK-EC256-Public', async () => {
      const key = await crypto.subtle.importKey(
        'jwk',
        publicEcdsaJsonWebKeyData,
        ecdsaJsonWebKeyAlgorithm,
        publicEcdsaJsonWebKeyData.ext,
        publicEcdsaJsonWebKeyData.key_ops,
      );
      await assert(key instanceof CryptoKey, true, `key instanceof CryptoKey`);
      await assert(
        key.algorithm,
        {
          name: 'ECDSA',
          namedCurve: 'P-256',
        },
        `key.algorithm`,
      );
      await assert(key.extractable, true, `key.extractable === true`);
      await assert(key.type, 'public', `key.type === "public"`);
      await assert(key.usages, ['verify'], `key.usages deep equals ["verify"]`);
    });

    routes.set('/crypto.subtle.importKey/HMAC', async () => {
      const keyUint8Array = new Uint8Array([1, 0, 1]);

      for (const algorithm of ['SHA-1', 'SHA-256', 'SHA-384', 'SHA-512']) {
        const key = await globalThis.crypto.subtle.importKey(
          'raw',
          keyUint8Array,
          { name: 'HMAC', hash: algorithm },
          false,
          ['sign', 'verify'],
        );
        await assert(
          key instanceof CryptoKey,
          true,
          `key instanceof CryptoKey`,
        );
        await assert(
          key.algorithm,
          {
            name: 'HMAC',
            hash: { name: algorithm },
            length: 24,
          },
          `key.algorithm`,
        );
        await assert(key.extractable, false, `key.extractable`);
        await assert(key.type, 'secret', `key.type`);
        await assert(key.usages, ['sign', 'verify'], `key.usages`);
      }
    });
    routes.set('/crypto.subtle.importKey/JWK-HS256-Public', async () => {
      const key = await crypto.subtle.importKey(
        'jwk',
        {
          kty: 'oct',
          k: 'Y0zt37HgOx-BY7SQjYVmrqhPkO44Ii2Jcb9yydUDPfE',
          alg: 'HS256',
          ext: true,
        },
        {
          name: 'HMAC',
          hash: { name: 'SHA-256' },
        },
        false,
        ['sign', 'verify'],
      );
      await assert(key instanceof CryptoKey, true, `key instanceof CryptoKey`);
      await assert(
        key.algorithm,
        {
          name: 'HMAC',
          hash: { name: 'SHA-256' },
          length: 256,
        },
        `key.algorithm`,
      );
      await assert(key.extractable, false, `key.extractable`);
      await assert(key.type, 'secret', `key.type`);
      await assert(key.usages, ['sign', 'verify'], `key.usages`);
    });
  }
}

// digest
{
  const enc = new TextEncoder();
  const data = enc.encode('hello world');
  routes.set('/crypto.subtle.digest', async () => {
    assert(
      typeof crypto.subtle.digest,
      'function',
      `typeof crypto.subtle.digest`,
    );
    assert(
      crypto.subtle.digest,
      SubtleCrypto.prototype.digest,
      `crypto.subtle.digest === SubtleCrypto.prototype.digest`,
    );
  });
  routes.set('/crypto.subtle.digest/length', async () => {
    assert(crypto.subtle.digest.length, 2, `crypto.subtle.digest.length === 2`);
  });
  routes.set('/crypto.subtle.digest/called-as-constructor', async () => {
    assertThrows(
      () => {
        new crypto.subtle.digest();
      },
      TypeError,
      'crypto.subtle.digest is not a constructor',
    );
  });
  routes.set('/crypto.subtle.digest/called-with-wrong-this', async () => {
    await assertRejects(async () => {
      await crypto.subtle.digest.call(undefined);
    }, TypeError);
  });
  routes.set('/crypto.subtle.digest/called-with-no-arguments', async () => {
    await assertRejects(
      async () => {
        await crypto.subtle.digest();
      },
      TypeError,
      'SubtleCrypto.digest: At least 2 arguments required, but only 0 passed',
    );
  });

  // first-parameter
  {
    routes.set(
      '/crypto.subtle.digest/first-parameter-calls-7.1.17-ToString',
      async () => {
        const sentinel = Symbol('sentinel');
        const test = async () => {
          await crypto.subtle.digest(
            {
              name: {
                toString() {
                  throw sentinel;
                },
              },
            },
            data,
          );
        };
        await assertRejects(test);
        try {
          await test();
        } catch (thrownError) {
          assert(thrownError, sentinel, 'thrownError === sentinel');
        }
      },
    );
    routes.set(
      '/crypto.subtle.digest/first-parameter-non-existant-format',
      async () => {
        await assertRejects(
          async () => {
            await crypto.subtle.digest('jake', data);
          },
          Error,
          'Algorithm: Unrecognized name',
        );
      },
    );
  }
  // second-parameter
  {
    routes.set('/crypto.subtle.digest/second-parameter-undefined', async () => {
      await assertRejects(async () => {
        await crypto.subtle.digest('sha-1', undefined);
      }, Error);
    });
  }
  // happy paths
  {
    // "MD5"
    routes.set('/crypto.subtle.digest/md5', async () => {
      const result = new Uint8Array(
        await crypto.subtle.digest('md5', new Uint8Array()),
      );
      const expected = new Uint8Array([
        212, 29, 140, 217, 143, 0, 178, 4, 233, 128, 9, 152, 236, 248, 66, 126,
      ]);
      assert(result, expected, 'result deep equals expected');
    });
    // "SHA-1"
    routes.set('/crypto.subtle.digest/sha-1', async () => {
      const result = new Uint8Array(
        await crypto.subtle.digest('sha-1', new Uint8Array()),
      );
      const expected = new Uint8Array([
        218, 57, 163, 238, 94, 107, 75, 13, 50, 85, 191, 239, 149, 96, 24, 144,
        175, 216, 7, 9,
      ]);
      assert(result, expected, 'result deep equals expected');
    });
    // "SHA-256"
    routes.set('/crypto.subtle.digest/sha-256', async () => {
      const result = new Uint8Array(
        await crypto.subtle.digest('sha-256', new Uint8Array()),
      );
      const expected = new Uint8Array([
        227, 176, 196, 66, 152, 252, 28, 20, 154, 251, 244, 200, 153, 111, 185,
        36, 39, 174, 65, 228, 100, 155, 147, 76, 164, 149, 153, 27, 120, 82,
        184, 85,
      ]);
      assert(result, expected, 'result deep equals expected');
    });
    // "SHA-384"
    routes.set('/crypto.subtle.digest/sha-384', async () => {
      const result = new Uint8Array(
        await crypto.subtle.digest('sha-384', new Uint8Array()),
      );
      const expected = new Uint8Array([
        56, 176, 96, 167, 81, 172, 150, 56, 76, 217, 50, 126, 177, 177, 227,
        106, 33, 253, 183, 17, 20, 190, 7, 67, 76, 12, 199, 191, 99, 246, 225,
        218, 39, 78, 222, 191, 231, 111, 101, 251, 213, 26, 210, 241, 72, 152,
        185, 91,
      ]);
      assert(result, expected, 'result deep equals expected');
    });
    // "SHA-512"
    routes.set('/crypto.subtle.digest/sha-512', async () => {
      const result = new Uint8Array(
        await crypto.subtle.digest('sha-512', new Uint8Array()),
      );
      const expected = new Uint8Array([
        207, 131, 225, 53, 126, 239, 184, 189, 241, 84, 40, 80, 214, 109, 128,
        7, 214, 32, 228, 5, 11, 87, 21, 220, 131, 244, 169, 33, 211, 108, 233,
        206, 71, 208, 209, 60, 93, 133, 242, 176, 255, 131, 24, 210, 135, 126,
        236, 47, 99, 185, 49, 189, 71, 65, 122, 129, 165, 56, 50, 122, 249, 39,
        218, 62,
      ]);
      assert(result, expected, 'result deep equals expected');
    });
  }
}

// sign
{
  const enc = new TextEncoder();
  const data = enc.encode('hello world');
  routes.set('/crypto.subtle.sign', async () => {
    assert(typeof crypto.subtle.sign, 'function', `typeof crypto.subtle.sign`);
    assert(
      crypto.subtle.sign,
      SubtleCrypto.prototype.sign,
      `crypto.subtle.sign === SubtleCrypto.prototype.sign`,
    );
  });
  routes.set('/crypto.subtle.sign/length', async () => {
    assert(crypto.subtle.sign.length, 3, `crypto.subtle.sign.length === 3`);
  });
  routes.set('/crypto.subtle.sign/called-as-constructor', async () => {
    assertThrows(
      () => {
        new crypto.subtle.sign();
      },
      TypeError,
      'crypto.subtle.sign is not a constructor',
    );
  });
  routes.set('/crypto.subtle.sign/called-with-wrong-this', async () => {
    await assertRejects(async () => {
      await crypto.subtle.sign.call(
        undefined,
        rsaJsonWebKeyAlgorithm,
        publicRsaJsonWebKeyData,
        data,
      );
    }, TypeError);
  });
  routes.set('/crypto.subtle.sign/called-with-no-arguments', async () => {
    await assertRejects(
      async () => {
        await crypto.subtle.sign();
      },
      TypeError,
      'SubtleCrypto.sign: At least 3 arguments required, but only 0 passed',
    );
  });
  // first-parameter
  {
    routes.set(
      '/crypto.subtle.sign/first-parameter-calls-7.1.17-ToString',
      async () => {
        const sentinel = Symbol('sentinel');
        const key = await crypto.subtle.importKey(
          'jwk',
          privateRsaJsonWebKeyData,
          rsaJsonWebKeyAlgorithm,
          privateRsaJsonWebKeyData.ext,
          privateRsaJsonWebKeyData.key_ops,
        );
        const test = async () => {
          await crypto.subtle.sign(
            {
              name: {
                toString() {
                  throw sentinel;
                },
              },
            },
            key,
            data,
          );
        };
        await assertRejects(test);
        try {
          await test();
        } catch (thrownError) {
          assert(thrownError, sentinel, 'thrownError === sentinel');
        }
      },
    );
    routes.set(
      '/crypto.subtle.sign/first-parameter-non-existant-algorithm',
      async () => {
        await assertRejects(
          async () => {
            const key = await crypto.subtle.importKey(
              'jwk',
              privateRsaJsonWebKeyData,
              rsaJsonWebKeyAlgorithm,
              privateRsaJsonWebKeyData.ext,
              privateRsaJsonWebKeyData.key_ops,
            );
            await crypto.subtle.sign('jake', key, data);
          },
          Error,
          'Algorithm: Unrecognized name',
        );
      },
    );
  }
  // second-parameter
  {
    routes.set(
      '/crypto.subtle.sign/second-parameter-invalid-format',
      async () => {
        await assertRejects(async () => {
          await crypto.subtle.sign(rsaJsonWebKeyAlgorithm, 'jake', data);
        }, Error);
      },
    );
    routes.set(
      '/crypto.subtle.sign/second-parameter-invalid-usages',
      async () => {
        await assertRejects(
          async () => {
            const key = await crypto.subtle.importKey(
              'jwk',
              publicRsaJsonWebKeyData,
              rsaJsonWebKeyAlgorithm,
              publicRsaJsonWebKeyData.ext,
              publicRsaJsonWebKeyData.key_ops,
            );
            await crypto.subtle.sign(rsaJsonWebKeyAlgorithm, key, data);
          },
          Error,
          "CryptoKey doesn't support signing",
        );
      },
    );
  }
  // third-parameter
  {
    routes.set(
      '/crypto.subtle.sign/third-parameter-invalid-format',
      async () => {
        await assertRejects(async () => {
          const key = await crypto.subtle.importKey(
            'jwk',
            publicRsaJsonWebKeyData,
            rsaJsonWebKeyAlgorithm,
            publicRsaJsonWebKeyData.ext,
            publicRsaJsonWebKeyData.key_ops,
          );
          await crypto.subtle.sign(rsaJsonWebKeyAlgorithm, key, undefined);
        }, Error);
      },
    );
  }
  // happy-path
  {
    routes.set('/crypto.subtle.sign/happy-path-jwk', async () => {
      const key = await crypto.subtle.importKey(
        'jwk',
        privateRsaJsonWebKeyData,
        rsaJsonWebKeyAlgorithm,
        privateRsaJsonWebKeyData.ext,
        privateRsaJsonWebKeyData.key_ops,
      );
      const signature = new Uint8Array(
        await crypto.subtle.sign(rsaJsonWebKeyAlgorithm, key, data),
      );
      const expected = new Uint8Array([
        70, 96, 33, 185, 93, 42, 67, 49, 243, 70, 88, 68, 194, 148, 53, 249,
        255, 192, 232, 132, 161, 194, 41, 244, 174, 211, 218, 203, 7, 238, 71,
        182, 101, 49, 139, 222, 165, 70, 222, 105, 82, 156, 184, 44, 100, 108,
        121, 237, 250, 119, 66, 228, 156, 243, 71, 105, 62, 246, 22, 2, 160,
        116, 71, 147, 202, 168, 24, 92, 224, 41, 148, 161, 124, 80, 212, 169,
        212, 64, 29, 189, 2, 171, 174, 188, 159, 89, 93, 122, 219, 166, 105, 92,
        107, 173, 103, 238, 145, 226, 94, 139, 71, 124, 17, 233, 49, 138, 89,
        246, 3, 82, 238, 154, 169, 188, 66, 198, 32, 23, 230, 90, 164, 140, 51,
        47, 221, 149, 161, 14, 254, 169, 224, 223, 119, 94, 27, 63, 199, 93, 65,
        53, 24, 151, 146, 242, 239, 41, 108, 136, 31, 99, 42, 213, 128, 244,
        140, 238, 157, 107, 117, 241, 219, 137, 97, 39, 109, 185, 176, 97, 193,
        60, 117, 244, 106, 62, 193, 188, 87, 199, 37, 70, 137, 37, 231, 110,
        228, 228, 139, 53, 240, 56, 92, 102, 220, 176, 127, 248, 24, 217, 208,
        29, 209, 216, 29, 251, 100, 252, 243, 183, 195, 96, 126, 102, 136, 48,
        39, 186, 45, 202, 10, 187, 22, 52, 183, 190, 149, 153, 32, 12, 90, 66,
        49, 122, 190, 154, 167, 9, 12, 32, 77, 177, 222, 54, 211, 233, 219, 205,
        133, 0, 113, 77, 158, 1, 125, 5, 15, 195,
      ]);
      assert(signature, expected, 'signature deep equals expected');
    });
    routes.set('/crypto.subtle.sign/happy-path-hmac', async () => {
      const encoder = new TextEncoder();
      const messageUint8Array = encoder.encode('aki');
      const keyUint8Array = new Uint8Array([1, 0, 1]);
      const results = {
        'SHA-1': new Uint8Array([
          222, 61, 81, 133, 232, 89, 130, 225, 248, 25, 220, 34, 245, 103, 89,
          127, 136, 77, 146, 166,
        ]),
        'SHA-256': new Uint8Array([
          92, 237, 16, 210, 91, 89, 194, 36, 95, 98, 27, 175, 64, 25, 15, 160,
          152, 178, 145, 235, 62, 92, 23, 202, 125, 228, 8, 25, 148, 26, 215,
          242,
        ]),
        'SHA-384': new Uint8Array([
          238, 20, 74, 173, 238, 236, 161, 229, 250, 167, 72, 210, 188, 239,
          233, 39, 233, 166, 114, 241, 140, 229, 201, 129, 243, 173, 74, 198,
          223, 145, 228, 96, 253, 91, 166, 111, 244, 23, 141, 62, 112, 156, 90,
          166, 214, 69, 185, 48,
        ]),
        'SHA-512': new Uint8Array([
          211, 127, 139, 149, 23, 225, 84, 230, 82, 249, 109, 254, 168, 236,
          217, 112, 174, 52, 231, 62, 167, 197, 33, 11, 181, 21, 162, 236, 214,
          132, 43, 161, 92, 112, 230, 182, 140, 69, 169, 229, 87, 98, 57, 81,
          140, 134, 219, 253, 139, 169, 85, 181, 195, 195, 166, 241, 219, 33, 9,
          56, 67, 213, 51, 224,
        ]),
      };

      for (const algorithm of ['SHA-1', 'SHA-256', 'SHA-384', 'SHA-512']) {
        const key = await globalThis.crypto.subtle.importKey(
          'raw',
          keyUint8Array,
          { name: 'HMAC', hash: algorithm },
          false,
          ['sign', 'verify'],
        );
        // Sign the message with HMAC and the CryptoKey
        const signature = new Uint8Array(
          await globalThis.crypto.subtle.sign('HMAC', key, messageUint8Array),
        );
        const expected = results[algorithm];
        assert(
          signature,
          expected,
          `${algorithm} signature deep equals expected`,
        );
      }
    });
  }
}

// verify
{
  routes.set('/crypto.subtle.verify', async () => {
    assert(
      typeof crypto.subtle.verify,
      'function',
      `typeof crypto.subtle.verify`,
    );
    assert(
      crypto.subtle.verify,
      SubtleCrypto.prototype.verify,
      `crypto.subtle.verify === SubtleCrypto.prototype.verify`,
    );
  });
  routes.set('/crypto.subtle.verify/length', async () => {
    assert(crypto.subtle.verify.length, 4, `crypto.subtle.verify.length === 4`);
  });
  routes.set('/crypto.subtle.verify/called-as-constructor', async () => {
    assertThrows(
      () => {
        new crypto.subtle.verify();
      },
      TypeError,
      'crypto.subtle.verify is not a constructor',
    );
  });
  routes.set('/crypto.subtle.verify/called-with-wrong-this', async () => {
    await assertRejects(async () => {
      const key = await crypto.subtle.importKey(
        'jwk',
        publicRsaJsonWebKeyData,
        rsaJsonWebKeyAlgorithm,
        publicRsaJsonWebKeyData.ext,
        publicRsaJsonWebKeyData.key_ops,
      );
      await crypto.subtle.verify.call(
        undefined,
        rsaJsonWebKeyAlgorithm,
        key,
        new Uint8Array(),
        new Uint8Array(),
      );
    }, TypeError);
  });
  routes.set('/crypto.subtle.verify/called-with-no-arguments', async () => {
    await assertRejects(
      async () => {
        await crypto.subtle.verify();
      },
      TypeError,
      'SubtleCrypto.verify: At least 4 arguments required, but only 0 passed',
    );
  });
  // first-parameter
  {
    routes.set(
      '/crypto.subtle.verify/first-parameter-calls-7.1.17-ToString',
      async () => {
        const sentinel = Symbol('sentinel');
        const test = async () => {
          const key = await crypto.subtle.importKey(
            'jwk',
            publicRsaJsonWebKeyData,
            rsaJsonWebKeyAlgorithm,
            publicRsaJsonWebKeyData.ext,
            publicRsaJsonWebKeyData.key_ops,
          );
          await crypto.subtle.verify(
            {
              name: {
                toString() {
                  throw sentinel;
                },
              },
            },
            key,
            new Uint8Array(),
            new Uint8Array(),
          );
        };
        await assertRejects(test);
        try {
          await test();
        } catch (thrownError) {
          assert(thrownError, sentinel, 'thrownError === sentinel');
        }
      },
    );
    routes.set(
      '/crypto.subtle.verify/first-parameter-non-existant-algorithm',
      async () => {
        await assertRejects(
          async () => {
            const key = await crypto.subtle.importKey(
              'jwk',
              publicRsaJsonWebKeyData,
              rsaJsonWebKeyAlgorithm,
              publicRsaJsonWebKeyData.ext,
              publicRsaJsonWebKeyData.key_ops,
            );
            await crypto.subtle.verify(
              'jake',
              key,
              new Uint8Array(),
              new Uint8Array(),
            );
          },
          Error,
          'Algorithm: Unrecognized name',
        );
      },
    );
  }
  // second-parameter
  {
    routes.set(
      '/crypto.subtle.verify/second-parameter-invalid-format',
      async () => {
        await assertRejects(async () => {
          await crypto.subtle.verify(
            rsaJsonWebKeyAlgorithm,
            'jake',
            new Uint8Array(),
            new Uint8Array(),
          );
        }, Error);
      },
    );
    routes.set(
      '/crypto.subtle.verify/second-parameter-invalid-usages',
      async () => {
        await assertRejects(
          async () => {
            const key = await crypto.subtle.importKey(
              'jwk',
              privateRsaJsonWebKeyData,
              rsaJsonWebKeyAlgorithm,
              privateRsaJsonWebKeyData.ext,
              privateRsaJsonWebKeyData.key_ops,
            );
            await crypto.subtle.verify(
              rsaJsonWebKeyAlgorithm,
              key,
              new Uint8Array(),
              new Uint8Array(),
            );
          },
          Error,
          "CryptoKey doesn't support verification",
        );
      },
    );
  }
  // third-parameter
  {
    routes.set(
      '/crypto.subtle.verify/third-parameter-invalid-format',
      async () => {
        await assertRejects(async () => {
          const key = await crypto.subtle.importKey(
            'jwk',
            publicRsaJsonWebKeyData,
            rsaJsonWebKeyAlgorithm,
            publicRsaJsonWebKeyData.ext,
            publicRsaJsonWebKeyData.key_ops,
          );
          await crypto.subtle.verify(
            rsaJsonWebKeyAlgorithm,
            key,
            undefined,
            new Uint8Array(),
          );
        }, Error);
      },
    );
  }
  // fourth-parameter
  {
    routes.set(
      '/crypto.subtle.verify/fourth-parameter-invalid-format',
      async () => {
        await assertRejects(async () => {
          const key = await crypto.subtle.importKey(
            'jwk',
            publicRsaJsonWebKeyData,
            rsaJsonWebKeyAlgorithm,
            publicRsaJsonWebKeyData.ext,
            publicRsaJsonWebKeyData.key_ops,
          );
          await crypto.subtle.verify(
            rsaJsonWebKeyAlgorithm,
            key,
            new Uint8Array(),
            undefined,
          );
        }, Error);
      },
    );
  }
  // incorrect-signature
  {
    routes.set('/crypto.subtle.verify/incorrect-signature-jwk', async () => {
      const key = await crypto.subtle.importKey(
        'jwk',
        publicRsaJsonWebKeyData,
        rsaJsonWebKeyAlgorithm,
        publicRsaJsonWebKeyData.ext,
        publicRsaJsonWebKeyData.key_ops,
      );
      const signature = new Uint8Array();
      const enc = new TextEncoder();
      const data = enc.encode('hello world');
      const result = await crypto.subtle.verify(
        rsaJsonWebKeyAlgorithm,
        key,
        signature,
        data,
      );
      assert(result, false, 'result === false');
    });
    routes.set('/crypto.subtle.verify/incorrect-signature-hmac', async () => {
      const keyUint8Array = new Uint8Array([1, 0, 1]);
      const signature = new Uint8Array();
      const enc = new TextEncoder();
      const data = enc.encode('hello world');

      for (const algorithm of ['SHA-1', 'SHA-256', 'SHA-384', 'SHA-512']) {
        const key = await globalThis.crypto.subtle.importKey(
          'raw',
          keyUint8Array,
          { name: 'HMAC', hash: algorithm },
          false,
          ['sign', 'verify'],
        );
        const result = await crypto.subtle.verify('HMAC', key, signature, data);
        assert(result, false, 'result');
      }
    });
  }
  // correct-signature
  {
    routes.set('/crypto.subtle.verify/correct-signature-jwk-rsa', async () => {
      const pkey = await crypto.subtle.importKey(
        'jwk',
        privateRsaJsonWebKeyData,
        rsaJsonWebKeyAlgorithm,
        privateRsaJsonWebKeyData.ext,
        privateRsaJsonWebKeyData.key_ops,
      );
      const key = await crypto.subtle.importKey(
        'jwk',
        publicRsaJsonWebKeyData,
        rsaJsonWebKeyAlgorithm,
        publicRsaJsonWebKeyData.ext,
        publicRsaJsonWebKeyData.key_ops,
      );
      const enc = new TextEncoder();
      const data = enc.encode('hello world');
      const signature = await crypto.subtle.sign(
        rsaJsonWebKeyAlgorithm,
        pkey,
        data,
      );
      const result = await crypto.subtle.verify(
        rsaJsonWebKeyAlgorithm,
        key,
        signature,
        data,
      );
      assert(result, true, 'result === true');
    });
    routes.set(
      '/crypto.subtle.verify/correct-signature-jwk-ecdsa',
      async () => {
        const pkey = await crypto.subtle.importKey(
          'jwk',
          privateEcdsaJsonWebKeyData,
          ecdsaJsonWebKeyAlgorithm,
          privateEcdsaJsonWebKeyData.ext,
          privateEcdsaJsonWebKeyData.key_ops,
        );
        const key = await crypto.subtle.importKey(
          'jwk',
          publicEcdsaJsonWebKeyData,
          ecdsaJsonWebKeyAlgorithm,
          publicEcdsaJsonWebKeyData.ext,
          publicEcdsaJsonWebKeyData.key_ops,
        );
        const enc = new TextEncoder();
        const data = enc.encode('hello world');
        const signature = await crypto.subtle.sign(
          ecdsaJsonWebKeyAlgorithm,
          pkey,
          data,
        );
        const result = await crypto.subtle.verify(
          ecdsaJsonWebKeyAlgorithm,
          key,
          signature,
          data,
        );
        assert(result, true, 'result === true');
      },
    );
    routes.set('/crypto.subtle.verify/correct-signature-hmac', async () => {
      const results = {
        'SHA-1': new Uint8Array([
          222, 61, 81, 133, 232, 89, 130, 225, 248, 25, 220, 34, 245, 103, 89,
          127, 136, 77, 146, 166,
        ]),
        'SHA-256': new Uint8Array([
          92, 237, 16, 210, 91, 89, 194, 36, 95, 98, 27, 175, 64, 25, 15, 160,
          152, 178, 145, 235, 62, 92, 23, 202, 125, 228, 8, 25, 148, 26, 215,
          242,
        ]),
        'SHA-384': new Uint8Array([
          238, 20, 74, 173, 238, 236, 161, 229, 250, 167, 72, 210, 188, 239,
          233, 39, 233, 166, 114, 241, 140, 229, 201, 129, 243, 173, 74, 198,
          223, 145, 228, 96, 253, 91, 166, 111, 244, 23, 141, 62, 112, 156, 90,
          166, 214, 69, 185, 48,
        ]),
        'SHA-512': new Uint8Array([
          211, 127, 139, 149, 23, 225, 84, 230, 82, 249, 109, 254, 168, 236,
          217, 112, 174, 52, 231, 62, 167, 197, 33, 11, 181, 21, 162, 236, 214,
          132, 43, 161, 92, 112, 230, 182, 140, 69, 169, 229, 87, 98, 57, 81,
          140, 134, 219, 253, 139, 169, 85, 181, 195, 195, 166, 241, 219, 33, 9,
          56, 67, 213, 51, 224,
        ]),
      };
      const encoder = new TextEncoder();
      const messageUint8Array = encoder.encode('aki');
      const keyUint8Array = new Uint8Array([1, 0, 1]);

      for (const algorithm of ['SHA-1', 'SHA-256', 'SHA-384', 'SHA-512']) {
        const key = await globalThis.crypto.subtle.importKey(
          'raw',
          keyUint8Array,
          { name: 'HMAC', hash: algorithm },
          false,
          ['sign', 'verify'],
        );
        // Sign the message with HMAC and the CryptoKey
        // const signature = new Uint8Array(await globalThis.crypto.subtle.sign("HMAC", key, messageUint8Array));
        const signature = results[algorithm];
        const result = await crypto.subtle.verify(
          'HMAC',
          key,
          signature,
          messageUint8Array,
        );
        assert(result, true, 'result');
      }
    });
  }
}

// deriveBits / encrypt / decrypt
{
  function fromHex(hex) {
    return new Uint8Array(hex.match(/../g).map((byte) => parseInt(byte, 16)));
  }
  async function assertRejectsWithName(func, name) {
    let rejected = false;
    try {
      await func();
    } catch (error) {
      rejected = true;
      assert(error instanceof DOMException, true, `rejects with DOMException`);
      assert(error.name, name, `error.name === '${name}'`);
    }
    assert(rejected, true, `\`${func.toString()}\` rejects`);
  }
  const enc = new TextEncoder();
  const dec = new TextDecoder();

  // PBKDF2-HMAC-SHA-256, password "password", salt "salt", 32 bytes.
  // Computed with Python hashlib.pbkdf2_hmac and Node crypto.pbkdf2Sync.
  const pbkdf2Vectors = {
    500: 'd64e8195b42b448b3b11993fe808dba4ab8b27ee81e672ee8977bc7416e258a3',
    1000: '632c2812e46d4604102ba7618e9d6d7d2f8128f6266b4a03264d2a0460b7dcb3',
  };
  function pbkdf2Params(iterations, hash = 'SHA-256') {
    return { name: 'PBKDF2', hash, salt: enc.encode('salt'), iterations };
  }
  async function importPbkdf2Key(usages = ['deriveBits']) {
    return crypto.subtle.importKey(
      'raw',
      enc.encode('password'),
      'PBKDF2',
      false,
      usages,
    );
  }

  // AES-CBC known answers. Ciphertexts include the PKCS#7 padding block that
  // WebCrypto appends, so the first blocks match the published vectors and
  // the full buffers were recomputed with openssl and Node createCipheriv.
  // NIST SP 800-38A F.2.1 (AES-128) and F.2.5 (AES-256), four blocks.
  const nistIv = fromHex('000102030405060708090a0b0c0d0e0f');
  const nistPlaintext = fromHex(
    '6bc1bee22e409f96e93d7e117393172aae2d8a571e03ac9c9eb76fac45af8e51' +
      '30c81c46a35ce411e5fbc1191a0a52eff69f2445df4f9b17ad2b417be66c3710',
  );
  const aes128Key = fromHex('2b7e151628aed2a6abf7158809cf4f3c');
  const aes128KeyJwkK = 'K34VFiiu0qar9xWICc9PPA';
  const aes128Ciphertext = fromHex(
    '7649abac8119b246cee98e9b12e9197d5086cb9b507219ee95db113a917678b2' +
      '73bed6b8e3c1743b7116e69e222295163ff1caa1681fac09120eca307586e1a7' +
      '8cb82807230e1321d3fae00d18cc2012',
  );
  const aes256Key = fromHex(
    '603deb1015ca71be2b73aef0857d77811f352c073b6108d72d9810a30914dff4',
  );
  const aes256Ciphertext = fromHex(
    'f58c4c04d6e5f1ba779eabfb5f7bfbd69cfc4e967edb808d679f777bc6702c7d' +
      '39f23369a9d9bacfa530e26304231461b2eb05e2c39be9fcda6c19078c6a9d1b' +
      '3f461796d6b0d6b2e0c2a72b4d80e644',
  );
  // RFC 3602 section 4, case 1 (AES-128-CBC, "Single block msg").
  const rfc3602Key = fromHex('06a9214036b8a15b512e03d534120006');
  const rfc3602Iv = fromHex('3dafba429d9eb430b422da802c9fac41');
  const rfc3602Ciphertext = fromHex(
    'e353779c1079aeb82708942dbe77181ab97c825e1c785146542d396941bce55d',
  );
  async function importAesCbcKey(keyData, usages = ['encrypt', 'decrypt']) {
    return crypto.subtle.importKey(
      'raw',
      keyData,
      { name: 'AES-CBC' },
      false,
      usages,
    );
  }

  // deriveBits
  {
    routes.set('/crypto.subtle.deriveBits', async () => {
      assert(
        typeof crypto.subtle.deriveBits,
        'function',
        `typeof crypto.subtle.deriveBits`,
      );
      assert(
        crypto.subtle.deriveBits,
        SubtleCrypto.prototype.deriveBits,
        `crypto.subtle.deriveBits === SubtleCrypto.prototype.deriveBits`,
      );
    });
    routes.set('/crypto.subtle.deriveBits/length', async () => {
      assert(
        crypto.subtle.deriveBits.length,
        2,
        `crypto.subtle.deriveBits.length === 2`,
      );
    });
    routes.set('/crypto.subtle.deriveBits/called-as-constructor', async () => {
      assertThrows(
        () => {
          new crypto.subtle.deriveBits();
        },
        TypeError,
        'crypto.subtle.deriveBits is not a constructor',
      );
    });
    routes.set('/crypto.subtle.deriveBits/called-with-wrong-this', async () => {
      await assertRejects(async () => {
        await crypto.subtle.deriveBits.call(undefined);
      }, TypeError);
    });
    routes.set(
      '/crypto.subtle.deriveBits/called-with-no-arguments',
      async () => {
        await assertRejects(
          async () => {
            await crypto.subtle.deriveBits();
          },
          TypeError,
          'SubtleCrypto.deriveBits: At least 2 arguments required, but only 0 passed',
        );
      },
    );
    routes.set('/crypto.subtle.deriveBits/pbkdf2-import-key', async () => {
      const key = await importPbkdf2Key();
      assert(key instanceof CryptoKey, true, `key instanceof CryptoKey`);
      assert(key.type, 'secret', `key.type`);
      assert(key.extractable, false, `key.extractable`);
      assert(key.algorithm, { name: 'PBKDF2' }, `key.algorithm`);
      assert(key.usages, ['deriveBits'], `key.usages`);
    });
    routes.set(
      '/crypto.subtle.deriveBits/pbkdf2-import-key-invalid-usages',
      async () => {
        for (const usages of [['encrypt'], ['sign', 'deriveBits'], []]) {
          await assertRejectsWithName(async () => {
            await importPbkdf2Key(usages);
          }, 'SyntaxError');
        }
      },
    );
    routes.set(
      '/crypto.subtle.deriveBits/pbkdf2-import-key-extractable',
      async () => {
        await assertRejectsWithName(async () => {
          await crypto.subtle.importKey(
            'raw',
            enc.encode('password'),
            'PBKDF2',
            true,
            ['deriveBits'],
          );
        }, 'SyntaxError');
      },
    );
    routes.set('/crypto.subtle.deriveBits/pbkdf2-sha-256', async () => {
      const key = await importPbkdf2Key();
      for (const iterations of [500, 1000]) {
        const bits = await crypto.subtle.deriveBits(
          pbkdf2Params(iterations),
          key,
          256,
        );
        assert(
          bits instanceof ArrayBuffer,
          true,
          `bits instanceof ArrayBuffer`,
        );
        assert(
          new Uint8Array(bits),
          fromHex(pbkdf2Vectors[iterations]),
          `deriveBits(PBKDF2-SHA-256, ${iterations} iterations)`,
        );
      }
    });
    routes.set(
      '/crypto.subtle.deriveBits/pbkdf2-sha-256-partial-length',
      async () => {
        const key = await importPbkdf2Key();
        const bits = await crypto.subtle.deriveBits(pbkdf2Params(500), key, 64);
        assert(
          new Uint8Array(bits),
          fromHex(pbkdf2Vectors[500].slice(0, 16)),
          `first 8 bytes of the 500-iteration vector`,
        );
      },
    );
    routes.set('/crypto.subtle.deriveBits/wrong-usage-key', async () => {
      const deriveKeyOnly = await importPbkdf2Key(['deriveKey']);
      await assertRejectsWithName(async () => {
        await crypto.subtle.deriveBits(pbkdf2Params(500), deriveKeyOnly, 256);
      }, 'InvalidAccessError');
    });
    routes.set('/crypto.subtle.deriveBits/wrong-algorithm-key', async () => {
      const hmacKey = await crypto.subtle.importKey(
        'raw',
        enc.encode('password'),
        { name: 'HMAC', hash: 'SHA-256' },
        false,
        ['sign'],
      );
      await assertRejectsWithName(async () => {
        await crypto.subtle.deriveBits(pbkdf2Params(500), hmacKey, 256);
      }, 'InvalidAccessError');
    });
    routes.set('/crypto.subtle.deriveBits/unsupported-algorithm', async () => {
      const key = await importPbkdf2Key();
      for (const algorithm of [
        {
          name: 'HKDF',
          hash: 'SHA-256',
          salt: enc.encode('salt'),
          info: enc.encode(''),
        },
        { name: 'ECDH' },
        'X25519',
      ]) {
        await assertRejectsWithName(async () => {
          await crypto.subtle.deriveBits(algorithm, key, 256);
        }, 'NotSupportedError');
      }
    });
    routes.set(
      '/crypto.subtle.deriveBits/length-not-multiple-of-8',
      async () => {
        const key = await importPbkdf2Key();
        await assertRejectsWithName(async () => {
          await crypto.subtle.deriveBits(pbkdf2Params(500), key, 100);
        }, 'OperationError');
      },
    );
    routes.set('/crypto.subtle.deriveBits/zero-iterations', async () => {
      const key = await importPbkdf2Key();
      await assertRejectsWithName(async () => {
        await crypto.subtle.deriveBits(pbkdf2Params(0), key, 256);
      }, 'OperationError');
    });
  }

  // encrypt / decrypt
  {
    for (const method of ['encrypt', 'decrypt']) {
      routes.set(`/crypto.subtle.${method}`, async () => {
        assert(
          typeof crypto.subtle[method],
          'function',
          `typeof crypto.subtle.${method}`,
        );
        assert(
          crypto.subtle[method],
          SubtleCrypto.prototype[method],
          `crypto.subtle.${method} === SubtleCrypto.prototype.${method}`,
        );
      });
      routes.set(`/crypto.subtle.${method}/length`, async () => {
        assert(
          crypto.subtle[method].length,
          3,
          `crypto.subtle.${method}.length === 3`,
        );
      });
      routes.set(`/crypto.subtle.${method}/called-as-constructor`, async () => {
        assertThrows(
          () => {
            new crypto.subtle[method]();
          },
          TypeError,
          `crypto.subtle[method] is not a constructor`,
        );
      });
      routes.set(
        `/crypto.subtle.${method}/called-with-wrong-this`,
        async () => {
          await assertRejects(async () => {
            await crypto.subtle[method].call(undefined);
          }, TypeError);
        },
      );
      routes.set(
        `/crypto.subtle.${method}/called-with-no-arguments`,
        async () => {
          await assertRejects(
            async () => {
              await crypto.subtle[method]();
            },
            TypeError,
            `SubtleCrypto.${method}: At least 3 arguments required, but only 0 passed`,
          );
        },
      );
    }

    routes.set('/crypto.subtle.importKey/aes-cbc-raw', async () => {
      for (const [keyData, length] of [
        [aes128Key, 128],
        [aes256Key, 256],
      ]) {
        const key = await importAesCbcKey(keyData);
        assert(key instanceof CryptoKey, true, `key instanceof CryptoKey`);
        assert(key.type, 'secret', `key.type`);
        assert(key.extractable, false, `key.extractable`);
        assert(key.algorithm, { name: 'AES-CBC', length }, `key.algorithm`);
        assert([...key.usages].sort(), ['decrypt', 'encrypt'], `key.usages`);
      }
    });
    routes.set('/crypto.subtle.importKey/aes-cbc-raw-invalid', async () => {
      // 15-byte key: not 128, 192 or 256 bits.
      await assertRejectsWithName(async () => {
        await importAesCbcKey(new Uint8Array(15));
      }, 'DataError');
      // AES-CBC keys can only encrypt, decrypt, wrapKey or unwrapKey.
      await assertRejectsWithName(async () => {
        await importAesCbcKey(aes128Key, ['sign']);
      }, 'SyntaxError');
      await assertRejectsWithName(async () => {
        await importAesCbcKey(aes128Key, []);
      }, 'SyntaxError');
    });
    routes.set('/crypto.subtle.importKey/aes-cbc-jwk', async () => {
      const jwk = {
        kty: 'oct',
        k: aes128KeyJwkK,
        alg: 'A128CBC',
        ext: true,
        key_ops: ['encrypt', 'decrypt'],
      };
      const key = await crypto.subtle.importKey(
        'jwk',
        jwk,
        { name: 'AES-CBC' },
        true,
        ['decrypt'],
      );
      assert(key.type, 'secret', `key.type`);
      assert(key.extractable, true, `key.extractable`);
      assert(key.algorithm, { name: 'AES-CBC', length: 128 }, `key.algorithm`);
      assert(key.usages, ['decrypt'], `key.usages`);
      const plaintext = await crypto.subtle.decrypt(
        { name: 'AES-CBC', iv: nistIv },
        key,
        aes128Ciphertext,
      );
      assert(
        new Uint8Array(plaintext),
        nistPlaintext,
        `jwk key decrypts NIST F.2.1`,
      );
    });
    routes.set('/crypto.subtle.importKey/aes-cbc-jwk-invalid', async () => {
      const jwk = {
        kty: 'oct',
        k: aes128KeyJwkK,
        alg: 'A128CBC',
        ext: true,
        key_ops: ['encrypt', 'decrypt'],
      };
      // alg does not match the 128-bit key length.
      await assertRejectsWithName(async () => {
        await crypto.subtle.importKey(
          'jwk',
          { ...jwk, alg: 'A256CBC' },
          { name: 'AES-CBC' },
          true,
          ['encrypt'],
        );
      }, 'DataError');
      // kty must be "oct".
      await assertRejectsWithName(async () => {
        await crypto.subtle.importKey(
          'jwk',
          { ...jwk, kty: 'RSA' },
          { name: 'AES-CBC' },
          true,
          ['encrypt'],
        );
      }, 'DataError');
      // use "sig" is not allowed for an encryption key.
      await assertRejectsWithName(async () => {
        await crypto.subtle.importKey(
          'jwk',
          { ...jwk, use: 'sig' },
          { name: 'AES-CBC' },
          true,
          ['encrypt'],
        );
      }, 'DataError');
      // key_ops must be a superset of usages.
      await assertRejectsWithName(async () => {
        await crypto.subtle.importKey(
          'jwk',
          { ...jwk, key_ops: ['encrypt'] },
          { name: 'AES-CBC' },
          true,
          ['encrypt', 'decrypt'],
        );
      }, 'DataError');
      // key_ops present but empty is not a superset of any usage.
      await assertRejectsWithName(async () => {
        await crypto.subtle.importKey(
          'jwk',
          { ...jwk, key_ops: [] },
          { name: 'AES-CBC' },
          true,
          ['encrypt'],
        );
      }, 'DataError');
      // ext: false but extractable requested.
      await assertRejectsWithName(async () => {
        await crypto.subtle.importKey(
          'jwk',
          { ...jwk, ext: false },
          { name: 'AES-CBC' },
          true,
          ['encrypt'],
        );
      }, 'DataError');
    });

    routes.set('/crypto.subtle.encrypt/aes-cbc-round-trip', async () => {
      const message = enc.encode('The quick brown fox jumps over the lazy dog');
      for (const keyData of [aes128Key, aes256Key]) {
        const key = await importAesCbcKey(keyData);
        const iv = crypto.getRandomValues(new Uint8Array(16));
        const ciphertext = await crypto.subtle.encrypt(
          { name: 'AES-CBC', iv },
          key,
          message,
        );
        assert(
          ciphertext instanceof ArrayBuffer,
          true,
          `ciphertext instanceof ArrayBuffer`,
        );
        // 43 bytes of plaintext pad up to 48 bytes.
        assert(ciphertext.byteLength, 48, `ciphertext.byteLength`);
        const plaintext = await crypto.subtle.decrypt(
          { name: 'AES-CBC', iv },
          key,
          ciphertext,
        );
        assert(new Uint8Array(plaintext), message, `decrypt(encrypt(message))`);
      }
    });
    routes.set('/crypto.subtle.encrypt/aes-cbc-empty-plaintext', async () => {
      const key = await importAesCbcKey(aes128Key);
      const ciphertext = await crypto.subtle.encrypt(
        { name: 'AES-CBC', iv: nistIv },
        key,
        new Uint8Array(),
      );
      // Empty plaintext still produces one block of padding.
      assert(ciphertext.byteLength, 16, `ciphertext.byteLength`);
      const plaintext = await crypto.subtle.decrypt(
        { name: 'AES-CBC', iv: nistIv },
        key,
        ciphertext,
      );
      assert(plaintext.byteLength, 0, `plaintext.byteLength`);
    });
    routes.set('/crypto.subtle.encrypt/aes-128-cbc-known-answer', async () => {
      const nistKey = await importAesCbcKey(aes128Key, ['encrypt']);
      const nistResult = await crypto.subtle.encrypt(
        { name: 'AES-CBC', iv: nistIv },
        nistKey,
        nistPlaintext,
      );
      assert(
        new Uint8Array(nistResult),
        aes128Ciphertext,
        `NIST SP 800-38A F.2.1`,
      );

      const rfcKey = await importAesCbcKey(rfc3602Key, ['encrypt']);
      const rfcResult = await crypto.subtle.encrypt(
        { name: 'AES-CBC', iv: rfc3602Iv },
        rfcKey,
        enc.encode('Single block msg'),
      );
      assert(new Uint8Array(rfcResult), rfc3602Ciphertext, `RFC 3602 case 1`);
    });
    routes.set('/crypto.subtle.encrypt/aes-256-cbc-known-answer', async () => {
      const key = await importAesCbcKey(aes256Key, ['encrypt']);
      const result = await crypto.subtle.encrypt(
        { name: 'AES-CBC', iv: nistIv },
        key,
        nistPlaintext,
      );
      assert(new Uint8Array(result), aes256Ciphertext, `NIST SP 800-38A F.2.5`);
    });
    routes.set('/crypto.subtle.decrypt/aes-128-cbc-known-answer', async () => {
      const nistKey = await importAesCbcKey(aes128Key, ['decrypt']);
      const nistResult = await crypto.subtle.decrypt(
        { name: 'AES-CBC', iv: nistIv },
        nistKey,
        aes128Ciphertext,
      );
      assert(
        new Uint8Array(nistResult),
        nistPlaintext,
        `NIST SP 800-38A F.2.1`,
      );

      const rfcKey = await importAesCbcKey(rfc3602Key, ['decrypt']);
      const rfcResult = await crypto.subtle.decrypt(
        { name: 'AES-CBC', iv: rfc3602Iv },
        rfcKey,
        rfc3602Ciphertext,
      );
      assert(dec.decode(rfcResult), 'Single block msg', `RFC 3602 case 1`);
    });
    routes.set('/crypto.subtle.decrypt/aes-256-cbc-known-answer', async () => {
      const key = await importAesCbcKey(aes256Key, ['decrypt']);
      const result = await crypto.subtle.decrypt(
        { name: 'AES-CBC', iv: nistIv },
        key,
        aes256Ciphertext,
      );
      assert(new Uint8Array(result), nistPlaintext, `NIST SP 800-38A F.2.5`);
    });
    routes.set('/crypto.subtle.encrypt/wrong-usage-key', async () => {
      const decryptOnly = await importAesCbcKey(aes128Key, ['decrypt']);
      await assertRejectsWithName(async () => {
        await crypto.subtle.encrypt(
          { name: 'AES-CBC', iv: nistIv },
          decryptOnly,
          nistPlaintext,
        );
      }, 'InvalidAccessError');
      const hmacKey = await crypto.subtle.importKey(
        'raw',
        aes128Key,
        { name: 'HMAC', hash: 'SHA-256' },
        false,
        ['sign'],
      );
      await assertRejectsWithName(async () => {
        await crypto.subtle.encrypt(
          { name: 'AES-CBC', iv: nistIv },
          hmacKey,
          nistPlaintext,
        );
      }, 'InvalidAccessError');
    });
    routes.set('/crypto.subtle.decrypt/wrong-usage-key', async () => {
      const encryptOnly = await importAesCbcKey(aes128Key, ['encrypt']);
      await assertRejectsWithName(async () => {
        await crypto.subtle.decrypt(
          { name: 'AES-CBC', iv: nistIv },
          encryptOnly,
          aes128Ciphertext,
        );
      }, 'InvalidAccessError');
    });
    routes.set('/crypto.subtle.encrypt/unsupported-algorithm', async () => {
      const key = await importAesCbcKey(aes128Key);
      for (const algorithm of [
        { name: 'AES-CTR', counter: nistIv, length: 64 },
        { name: 'AES-GCM', iv: nistIv },
        { name: 'AES-KW' },
        { name: 'RSA-OAEP' },
      ]) {
        await assertRejectsWithName(async () => {
          await crypto.subtle.encrypt(algorithm, key, nistPlaintext);
        }, 'NotSupportedError');
        await assertRejectsWithName(async () => {
          await crypto.subtle.decrypt(algorithm, key, aes128Ciphertext);
        }, 'NotSupportedError');
      }
    });
    routes.set('/crypto.subtle.encrypt/invalid-iv-length', async () => {
      const key = await importAesCbcKey(aes128Key);
      for (const iv of [
        new Uint8Array(0),
        new Uint8Array(15),
        new Uint8Array(17),
      ]) {
        await assertRejectsWithName(async () => {
          await crypto.subtle.encrypt(
            { name: 'AES-CBC', iv },
            key,
            nistPlaintext,
          );
        }, 'OperationError');
        await assertRejectsWithName(async () => {
          await crypto.subtle.decrypt(
            { name: 'AES-CBC', iv },
            key,
            aes128Ciphertext,
          );
        }, 'OperationError');
      }
    });
    routes.set('/crypto.subtle.decrypt/corrupt-ciphertext', async () => {
      const key = await importAesCbcKey(aes128Key);
      // Flipping a byte in the final block corrupts the PKCS#7 padding.
      const badPadding = aes128Ciphertext.slice();
      badPadding[badPadding.length - 1] ^= 0xff;
      await assertRejectsWithName(async () => {
        await crypto.subtle.decrypt(
          { name: 'AES-CBC', iv: nistIv },
          key,
          badPadding,
        );
      }, 'OperationError');
      // Not a multiple of the block size.
      await assertRejectsWithName(async () => {
        await crypto.subtle.decrypt(
          { name: 'AES-CBC', iv: nistIv },
          key,
          aes128Ciphertext.subarray(0, 20),
        );
      }, 'OperationError');
      // Empty ciphertext.
      await assertRejectsWithName(async () => {
        await crypto.subtle.decrypt(
          { name: 'AES-CBC', iv: nistIv },
          key,
          new Uint8Array(),
        );
      }, 'OperationError');
      // Wrong key: the padding check fails.
      const otherKey = await importAesCbcKey(aes256Key);
      await assertRejectsWithName(async () => {
        await crypto.subtle.decrypt(
          { name: 'AES-CBC', iv: nistIv },
          otherKey,
          aes128Ciphertext,
        );
      }, 'OperationError');
    });

    // PBKDF2-SHA-256 -> 384 bits -> 32-byte AES-256 key + 16-byte IV -> AES-CBC -> JSON.
    // Expected values computed with Python hashlib/openssl and Node crypto.
    routes.set(
      '/crypto.subtle.deriveBits/pbkdf2-aes-cbc-end-to-end',
      async () => {
        const baseKey = await crypto.subtle.importKey(
          'raw',
          enc.encode('px3-password'),
          'PBKDF2',
          false,
          ['deriveBits'],
        );
        const bits = new Uint8Array(
          await crypto.subtle.deriveBits(
            {
              name: 'PBKDF2',
              hash: 'SHA-256',
              salt: enc.encode('px3-salt'),
              iterations: 1000,
            },
            baseKey,
            384,
          ),
        );
        assert(
          bits,
          fromHex(
            '15a7cea7209c02746580b374df7a64139cc6a6e59916d8e3719763ff3dad2a77' +
              '69d7c04f476d9042487ef55f08ea8dfe',
          ),
          `derived key material`,
        );
        const key = await importAesCbcKey(bits.subarray(0, 32), ['decrypt']);
        const iv = bits.subarray(32, 48);
        const plaintext = await crypto.subtle.decrypt(
          { name: 'AES-CBC', iv },
          key,
          fromHex(
            'c2e1954700c908351802d73df5494d4a3c316ee8786fc4943a2e771bd84f7b0d' +
              '35c2038bec4cff457f1617f8ce0ff9fa4f030f45d3a15a0bd850732581524411',
          ),
        );
        const payload = JSON.parse(dec.decode(plaintext));
        assert(
          payload,
          { user: 'dan', roles: ['admin'], exp: 1767225600 },
          `decrypted JSON payload`,
        );
      },
    );
  }
}
