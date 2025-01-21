/* eslint-env serviceworker */
import { SecretStore, SecretStoreEntry } from 'fastly:secret-store';
import { assert, assertThrows, assertRejects } from './assertions.js';
import { routes } from './routes.js';
import fc from './fast-check.js';
import { env } from 'fastly:env';

const SECRET_STORE_NAME = env('SECRET_STORE_NAME');

// SecretStore
{
  routes.set('/secret-store/exposed-as-global', () => {
    assert(typeof SecretStore, 'function', `typeof SecretStore`);
  });
  routes.set('/secret-store/interface', SecretStoreInterfaceTests);
  // SecretStore constructor
  {
    routes.set('/secret-store/constructor/called-as-regular-function', () => {
      assertThrows(
        () => {
          SecretStore();
        },
        TypeError,
        `calling a builtin SecretStore constructor without new is forbidden`,
      );
    });
    // https://tc39.es/ecma262/#sec-tostring
    routes.set(
      '/secret-store/constructor/parameter-calls-7.1.17-ToString',
      () => {
        let sentinel;
        const test = () => {
          sentinel = Symbol();
          const name = {
            toString() {
              throw sentinel;
            },
          };
          new SecretStore(name);
        };
        assertThrows(test);
        try {
          test();
        } catch (thrownError) {
          assert(thrownError, sentinel, 'thrownError === sentinel');
        }
        assertThrows(
          () => new SecretStore(Symbol()),
          TypeError,
          `can't convert symbol to string`,
        );
      },
    );
    routes.set('/secret-store/constructor/empty-parameter', () => {
      assertThrows(
        () => {
          new SecretStore();
        },
        TypeError,
        `SecretStore constructor: At least 1 argument required, but only 0 passed`,
      );
    });
    routes.set('/secret-store/constructor/found-store', () => {
      const store = createValidStore();
      assert(
        store instanceof SecretStore,
        true,
        `store instanceof SecretStore`,
      );
    });
    routes.set('/secret-store/constructor/missing-store', () => {
      assertThrows(
        () => {
          new SecretStore('missing');
        },
        Error,
        `SecretStore constructor: No SecretStore named 'missing' exists`,
      );
    });
    routes.set('/secret-store/constructor/invalid-name', () => {
      // control Characters (\\u0000-\\u001F) are not allowed
      fc.configureGlobal({ verbose: true });
      const charactersOtherThanLettersNumbersDashesUnderscoresAndPeriods = fc
        .fullUnicode()
        .filter((c) => {
          if (c == '-' || c == '_' || c == '.') {
            return false;
          }
          const code = c.charCodeAt(0);
          // number ascii
          if (code >= 48 && code <= 57) {
            return false;
          }
          // upper ascii
          if (code >= 65 && code <= 90) {
            return false;
          }
          // lower ascii
          if (code >= 97 && code <= 122) {
            return false;
          }
          return true;
        });
      fc.assert(
        fc.property(
          charactersOtherThanLettersNumbersDashesUnderscoresAndPeriods,
          (character) => {
            assertThrows(
              () => {
                new SecretStore(character);
              },
              TypeError,
              `SecretStore constructor: name can contain only ascii alphanumeric characters, underscores, dashes, and ascii whitespace`,
            );
          },
        ),
      );

      // must be less than 256 characters
      assertThrows(
        () => {
          new SecretStore('a'.repeat(256));
        },
        TypeError,
        `SecretStore constructor: name can not be more than 255 characters`,
      );

      // empty string not allowed
      assertThrows(
        () => {
          new SecretStore('');
        },
        TypeError,
        `SecretStore constructor: name can not be an empty string`,
      );
    });
  }

  // SecretStore get method
  {
    routes.set('/secret-store/get/called-as-constructor', () => {
      assertThrows(() => {
        new SecretStore.prototype.get('1');
      }, TypeError);
    });
    routes.set('/secret-store/get/called-unbound', () => {
      assertThrows(() => {
        SecretStore.prototype.get.call(undefined, '1');
      }, TypeError);
    });
    // https://tc39.es/ecma262/#sec-tostring
    routes.set(
      '/secret-store/get/key-parameter-calls-7.1.17-ToString',
      async () => {
        let sentinel;
        const test = () => {
          sentinel = Symbol();
          const key = {
            toString() {
              throw sentinel;
            },
          };
          const store = createValidStore();
          return store.get(key);
        };
        assertThrows(test);
        try {
          test();
        } catch (thrownError) {
          assert(thrownError, sentinel, 'thrownError === sentinel');
        }
        assertThrows(
          () => {
            const store = createValidStore();
            return store.get(Symbol());
          },
          TypeError,
          `can't convert symbol to string`,
        );
      },
    );
    routes.set('/secret-store/get/key-parameter-not-supplied', () => {
      assertThrows(
        () => {
          const store = createValidStore();
          return store.get();
        },
        TypeError,
        `get: At least 1 argument required, but only 0 passed`,
      );
    });
    routes.set('/secret-store/get/key-parameter-empty-string', async () => {
      await assertRejects(
        async () => {
          const store = createValidStore();
          return await store.get('');
        },
        TypeError,
        `SecretStore key can not be an empty string`,
      );
    });
    routes.set(
      '/secret-store/get/key-parameter-255-character-string',
      async () => {
        const store = createValidStore();
        const key = 'a'.repeat(255);
        let result = store.get(key);
        assert(
          result instanceof Promise,
          true,
          `store.get(key) instanceof Promise`,
        );
        result = await result;
        assert(
          result instanceof SecretStoreEntry,
          true,
          `(await store.get(key)) instanceof SecretStoreEntry`,
        );
      },
    );
    routes.set(
      '/secret-store/get/key-parameter-256-character-string',
      async () => {
        await assertRejects(
          async () => {
            const store = createValidStore();
            const key = 'a'.repeat(256);
            return store.get(key);
          },
          TypeError,
          `SecretStore key can not be more than 255 characters`,
        );
      },
    );
    routes.set('/secret-store/get/key-parameter-invalid-string', async () => {
      const charactersOtherThanLettersNumbersDashesUnderscoresAndPeriods = fc
        .fullUnicode()
        .filter((c) => {
          if (c == '-' || c == '_' || c == '.') {
            return false;
          }
          const code = c.charCodeAt(0);
          // number ascii
          if (code >= 48 && code <= 57) {
            return false;
          }
          // upper ascii
          if (code >= 65 && code <= 90) {
            return false;
          }
          // lower ascii
          if (code >= 97 && code <= 122) {
            return false;
          }
          return true;
        });

      fc.assert(
        fc.asyncProperty(
          charactersOtherThanLettersNumbersDashesUnderscoresAndPeriods,
          async (character) => {
            await assertRejects(
              async () => {
                const store = createValidStore();
                return store.get(character);
              },
              TypeError,
              `SecretStore key can contain only ascii alphanumeric characters, underscores, dashes, and ascii whitespace`,
            );
          },
        ),
      );
    });
    routes.set(
      '/secret-store/get/key-does-not-exist-returns-null',
      async () => {
        let store = createValidStore();
        let result = await store.get(Math.random());
        assert(result, null, `store.get(Math.random())`);
      },
    );
    routes.set('/secret-store/get/key-exists', async () => {
      let store = createValidStore();
      let result = await store.get('first');
      assert(
        result instanceof SecretStoreEntry,
        true,
        `(store.get(key) instanceof SecretStoreEntry)`,
      );
    });
  }

  // SecretStore.fromBytes static method
  {
    routes.set('/secret-store/from-bytes/invalid', async () => {
      assertThrows(
        () => {
          SecretStore.fromBytes('blah');
        },
        TypeError,
        `SecretStore.fromBytes: bytes must be an ArrayBuffer or ArrayBufferView object`,
      );
    });
    routes.set('/secret-store/from-bytes/valid', async () => {
      let result;
      result = SecretStore.fromBytes(new Uint8Array([1, 2, 3]));
      assert(
        result instanceof SecretStoreEntry,
        true,
        `(SecretStore.fromBytes(Uint8Array) instanceof SecretStoreEntry)`,
      );
      assert(
        result.rawBytes(),
        new Uint8Array([1, 2, 3]),
        `(SecretStore.fromBytes(Uint8Array).rawBytes() === Uint8Array)`,
      );
      result = SecretStore.fromBytes(new Uint16Array([4, 5, 6]));
      assert(
        result instanceof SecretStoreEntry,
        true,
        `(SecretStore.fromBytes(Uint16Array) instanceof SecretStoreEntry)`,
      );
      // (can rely on Wasm being little endian)
      assert(
        result.rawBytes(),
        new Uint8Array([4, 0, 5, 0, 6, 0]),
        `(SecretStore.fromBytes(Uint16Array).rawBytes() === Uint8Array)`,
      );
      result = SecretStore.fromBytes(new Uint16Array([7, 8, 9]).buffer);
      assert(
        result instanceof SecretStoreEntry,
        true,
        `(SecretStore.fromBytes(ArrayBuffer) instanceof SecretStoreEntry)`,
      );
      assert(
        result.rawBytes(),
        new Uint8Array([7, 0, 8, 0, 9, 0]),
        `(SecretStore.fromBytes(ArrayBuffer).rawBytes() === Uint8Array)`,
      );
    });
  }
}
// SecretStoreEntry
{
  routes.set('/secret-store-entry/interface', () => {
    return SecretStoreEntryInterfaceTests();
  });
  routes.set('/secret-store-entry/plaintext', async () => {
    let store = createValidStore();
    let secret = await store.get('first');
    let result = secret.plaintext();
    assert(
      result,
      'This is also some secret data',
      `(await store.get('first')).plaintext()`,
    );
  });
}
function SecretStoreEntryInterfaceTests() {
  let actual = Reflect.ownKeys(SecretStoreEntry);
  let expected = ['prototype', 'length', 'name'];
  assert(actual, expected, `Reflect.ownKeys(SecretStoreEntry)`);

  actual = Reflect.getOwnPropertyDescriptor(SecretStoreEntry, 'prototype');
  expected = {
    value: SecretStoreEntry.prototype,
    writable: false,
    enumerable: false,
    configurable: false,
  };
  assert(
    actual,
    expected,
    `Reflect.getOwnPropertyDescriptor(SecretStoreEntry, 'prototype')`,
  );

  actual = Reflect.getOwnPropertyDescriptor(SecretStoreEntry, 'length');
  expected = {
    value: 0,
    writable: false,
    enumerable: false,
    configurable: true,
  };
  assert(
    actual,
    expected,
    `Reflect.getOwnPropertyDescriptor(SecretStoreEntry, 'length')`,
  );

  actual = Reflect.getOwnPropertyDescriptor(SecretStoreEntry, 'name');
  expected = {
    value: 'SecretStoreEntry',
    writable: false,
    enumerable: false,
    configurable: true,
  };
  assert(
    actual,
    expected,
    `Reflect.getOwnPropertyDescriptor(SecretStoreEntry, 'name')`,
  );

  actual = Reflect.ownKeys(SecretStoreEntry.prototype);
  expected = ['constructor', 'plaintext', 'rawBytes'];
  assert(actual, expected, `Reflect.ownKeys(SecretStoreEntry.prototype)`);

  actual = Reflect.getOwnPropertyDescriptor(
    SecretStoreEntry.prototype,
    'constructor',
  );
  expected = {
    writable: true,
    enumerable: false,
    configurable: true,
    value: SecretStoreEntry.prototype.constructor,
  };
  assert(
    actual,
    expected,
    `Reflect.getOwnPropertyDescriptor(SecretStoreEntry.prototype, 'constructor')`,
  );

  actual = Reflect.getOwnPropertyDescriptor(
    SecretStoreEntry.prototype,
    'plaintext',
  );
  expected = {
    writable: true,
    enumerable: true,
    configurable: true,
    value: SecretStoreEntry.prototype.plaintext,
  };
  assert(
    actual,
    expected,
    `Reflect.getOwnPropertyDescriptor(SecretStoreEntry.prototype, 'plaintext')`,
  );

  assert(
    typeof SecretStoreEntry.prototype.constructor,
    'function',
    `typeof SecretStoreEntry.prototype.constructor`,
  );
  assert(
    typeof SecretStoreEntry.prototype.plaintext,
    'function',
    `typeof SecretStoreEntry.prototype.plaintext`,
  );

  actual = Reflect.getOwnPropertyDescriptor(
    SecretStoreEntry.prototype.constructor,
    'length',
  );
  expected = {
    value: 0,
    writable: false,
    enumerable: false,
    configurable: true,
  };
  assert(
    actual,
    expected,
    `Reflect.getOwnPropertyDescriptor(SecretStoreEntry.prototype.constructor, 'length')`,
  );

  actual = Reflect.getOwnPropertyDescriptor(
    SecretStoreEntry.prototype.constructor,
    'name',
  );
  expected = {
    value: 'SecretStoreEntry',
    writable: false,
    enumerable: false,
    configurable: true,
  };
  assert(
    actual,
    expected,
    `Reflect.getOwnPropertyDescriptor(SecretStoreEntry.prototype.constructor, 'name')`,
  );

  actual = Reflect.getOwnPropertyDescriptor(
    SecretStoreEntry.prototype.plaintext,
    'length',
  );
  expected = {
    value: 0,
    writable: false,
    enumerable: false,
    configurable: true,
  };
  assert(
    actual,
    expected,
    `Reflect.getOwnPropertyDescriptor(SecretStoreEntry.prototype.plaintext, 'length')`,
  );

  actual = Reflect.getOwnPropertyDescriptor(
    SecretStoreEntry.prototype.plaintext,
    'name',
  );
  expected = {
    value: 'plaintext',
    writable: false,
    enumerable: false,
    configurable: true,
  };
  assert(
    actual,
    expected,
    `Reflect.getOwnPropertyDescriptor(SecretStoreEntry.prototype.plaintext, 'name')`,
  );
}

function SecretStoreInterfaceTests() {
  let actual = Reflect.ownKeys(SecretStore);
  let expected = ['prototype', 'fromBytes', 'length', 'name'];
  assert(actual, expected, `Reflect.ownKeys(SecretStore)`);

  actual = Reflect.getOwnPropertyDescriptor(SecretStore, 'prototype');
  expected = {
    value: SecretStore.prototype,
    writable: false,
    enumerable: false,
    configurable: false,
  };
  assert(
    actual,
    expected,
    `Reflect.getOwnPropertyDescriptor(SecretStore, 'prototype')`,
  );

  actual = Reflect.getOwnPropertyDescriptor(SecretStore, 'length');
  expected = {
    value: 1,
    writable: false,
    enumerable: false,
    configurable: true,
  };
  assert(
    actual,
    expected,
    `Reflect.getOwnPropertyDescriptor(SecretStore, 'length')`,
  );

  actual = Reflect.getOwnPropertyDescriptor(SecretStore, 'name');
  expected = {
    value: 'SecretStore',
    writable: false,
    enumerable: false,
    configurable: true,
  };
  assert(
    actual,
    expected,
    `Reflect.getOwnPropertyDescriptor(SecretStore, 'name')`,
  );

  actual = Reflect.ownKeys(SecretStore.prototype);
  expected = ['constructor', 'get'];
  assert(actual, expected, `Reflect.ownKeys(SecretStore.prototype)`);

  actual = Reflect.getOwnPropertyDescriptor(
    SecretStore.prototype,
    'constructor',
  );
  expected = {
    writable: true,
    enumerable: false,
    configurable: true,
    value: SecretStore.prototype.constructor,
  };
  assert(
    actual,
    expected,
    `Reflect.getOwnPropertyDescriptor(SecretStore.prototype, 'constructor')`,
  );

  actual = Reflect.getOwnPropertyDescriptor(SecretStore.prototype, 'get');
  expected = {
    writable: true,
    enumerable: true,
    configurable: true,
    value: SecretStore.prototype.get,
  };
  assert(
    actual,
    expected,
    `Reflect.getOwnPropertyDescriptor(SecretStore.prototype, 'get')`,
  );

  assert(
    typeof SecretStore.prototype.constructor,
    'function',
    `typeof SecretStore.prototype.constructor`,
  );
  assert(
    typeof SecretStore.prototype.get,
    'function',
    `typeof SecretStore.prototype.get`,
  );

  actual = Reflect.getOwnPropertyDescriptor(
    SecretStore.prototype.constructor,
    'length',
  );
  expected = {
    value: 1,
    writable: false,
    enumerable: false,
    configurable: true,
  };
  assert(
    actual,
    expected,
    `Reflect.getOwnPropertyDescriptor(SecretStore.prototype.constructor, 'length')`,
  );

  actual = Reflect.getOwnPropertyDescriptor(
    SecretStore.prototype.constructor,
    'name',
  );
  expected = {
    value: 'SecretStore',
    writable: false,
    enumerable: false,
    configurable: true,
  };
  assert(
    actual,
    expected,
    `Reflect.getOwnPropertyDescriptor(SecretStore.prototype.constructor, 'name')`,
  );

  actual = Reflect.getOwnPropertyDescriptor(
    SecretStore.prototype.get,
    'length',
  );
  expected = {
    value: 1,
    writable: false,
    enumerable: false,
    configurable: true,
  };
  assert(
    actual,
    expected,
    `Reflect.getOwnPropertyDescriptor(SecretStore.prototype.get, 'length')`,
  );

  actual = Reflect.getOwnPropertyDescriptor(SecretStore.prototype.get, 'name');
  expected = {
    value: 'get',
    writable: false,
    enumerable: false,
    configurable: true,
  };
  assert(
    actual,
    expected,
    `Reflect.getOwnPropertyDescriptor(SecretStore.prototype.get, 'name')`,
  );
}

function createValidStore() {
  return new SecretStore(SECRET_STORE_NAME);
}
