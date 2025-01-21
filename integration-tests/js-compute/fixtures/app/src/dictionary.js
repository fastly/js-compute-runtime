/* eslint-env serviceworker */

import { Dictionary } from 'fastly:dictionary';
import { routes } from './routes.js';
import { assertThrows, assert, assertResolves } from './assertions.js';
import { env } from 'fastly:env';

const DICTIONARY_NAME = `aZ1 __ 2__${env('FASTLY_SERVICE_NAME').replace(/-/g, '_')}`;

// Dictionary
{
  routes.set('/dictionary/exposed-as-global', () => {
    assert(typeof Dictionary, 'function', `typeof Dictionary`);
  });
  routes.set('/dictionary/interface', dictionaryInterfaceTests);
  // Dictionary constructor
  {
    routes.set('/dictionary/constructor/called-as-regular-function', () => {
      assertThrows(
        () => {
          Dictionary();
        },
        TypeError,
        `calling a builtin Dictionary constructor without new is forbidden`,
      );
    });
    // https://tc39.es/ecma262/#sec-tostring
    routes.set(
      '/dictionary/constructor/parameter-calls-7.1.17-ToString',
      () => {
        let sentinel = Symbol();
        const test = () => {
          const name = {
            toString() {
              throw sentinel;
            },
          };
          new Dictionary(name);
        };
        assertThrows(test);
        try {
          test();
        } catch (thrownError) {
          assert(thrownError, sentinel, 'thrownError === sentinel');
        }
        assertThrows(
          () => new Dictionary(Symbol()),
          TypeError,
          `can't convert symbol to string`,
        );
      },
    );
    routes.set('/dictionary/constructor/empty-parameter', () => {
      assertThrows(
        () => {
          new Dictionary();
        },
        TypeError,
        `Dictionary constructor: At least 1 argument required, but only 0 passed`,
      );
    });
    routes.set('/dictionary/constructor/found', () => {
      const store = createValidDictionary();
      assert(store instanceof Dictionary, true, `store instanceof Dictionary`);
    });
    routes.set('/dictionary/constructor/invalid-name', () => {
      // control Characters (\\u0000-\\u001F) are not allowed
      const invalidNames = ['1', '-', ' ', 'Ä'];
      for (const name of invalidNames) {
        assertThrows(
          () => {
            new Dictionary(name);
          },
          TypeError,
          `Dictionary constructor: name must start with an ascii alpabetical character`,
        );
      }
      assertThrows(
        () => {
          new Dictionary('aÄ');
        },
        TypeError,
        `Dictionary constructor: name can contain only ascii alphanumeric characters, underscores, and ascii whitespace`,
      );

      // must be less than 256 characters
      assertThrows(
        () => {
          new Dictionary('a'.repeat(256));
        },
        TypeError,
        `Dictionary constructor: name can not be more than 255 characters`,
      );

      // empty string not allowed
      assertThrows(
        () => {
          new Dictionary('');
        },
        TypeError,
        `Dictionary constructor: name can not be an empty string`,
      );
    });
  }

  // Dictionary get method
  {
    routes.set('/dictionary/get/called-as-constructor', () => {
      assertThrows(() => {
        new Dictionary.prototype.get('1');
      }, TypeError);
    });
    routes.set('/dictionary/get/called-unbound', () => {
      assertThrows(() => {
        Dictionary.prototype.get.call(undefined, '1');
      }, TypeError);
    });
    // https://tc39.es/ecma262/#sec-tostring
    routes.set('/dictionary/get/key-parameter-calls-7.1.17-ToString', () => {
      let sentinel = Symbol();
      const test = () => {
        const key = {
          toString() {
            throw sentinel;
          },
        };
        const store = createValidDictionary();
        store.get(key);
      };
      assertThrows(test);
      try {
        test();
      } catch (thrownError) {
        assert(thrownError, sentinel, 'thrownError === sentinel');
      }
      assertThrows(
        () => {
          const store = createValidDictionary();
          store.get(Symbol());
        },
        TypeError,
        `can't convert symbol to string`,
      );
    });
    routes.set('/dictionary/get/key-parameter-not-supplied', () => {
      assertThrows(
        () => {
          const store = createValidDictionary();
          store.get();
        },
        TypeError,
        `get: At least 1 argument required, but only 0 passed`,
      );
    });
    routes.set('/dictionary/get/key-parameter-empty-string', () => {
      assertThrows(
        () => {
          const store = createValidDictionary();
          store.get('');
        },
        TypeError,
        `Dictionary key can not be an empty string`,
      );
    });
    routes.set('/dictionary/get/key-parameter-255-character-string', () => {
      assertResolves(() => {
        const store = createValidDictionary();
        const key = 'a'.repeat(255);
        store.get(key);
      });
    });
    routes.set('/dictionary/get/key-parameter-256-character-string', () => {
      assertThrows(
        () => {
          const store = createValidDictionary();
          const key = 'a'.repeat(256);
          store.get(key);
        },
        TypeError,
        `Dictionary key can not be more than 255 characters`,
      );
    });
    routes.set('/dictionary/get/key-does-not-exist-returns-null', () => {
      let store = createValidDictionary();
      let result = store.get(Math.random());
      assert(result, null, `store.get(Math.random())`);
    });
    routes.set('/dictionary/get/key-exists', () => {
      let store = createValidDictionary();
      let result = store.get('twitter');
      assert(
        result,
        'https://twitter.com/fastly',
        `store.get('twitter') === "https://twitter.com/fastly"`,
      );
    });
  }
}

function dictionaryInterfaceTests() {
  let actual = Reflect.ownKeys(Dictionary);
  let expected = ['prototype', 'length', 'name'];
  assert(actual, expected, `Reflect.ownKeys(Dictionary)`);

  actual = Reflect.getOwnPropertyDescriptor(Dictionary, 'prototype');
  expected = {
    value: Dictionary.prototype,
    writable: false,
    enumerable: false,
    configurable: false,
  };
  assert(
    actual,
    expected,
    `Reflect.getOwnPropertyDescriptor(Dictionary, 'prototype')`,
  );

  actual = Reflect.getOwnPropertyDescriptor(Dictionary, 'length');
  expected = {
    value: 1,
    writable: false,
    enumerable: false,
    configurable: true,
  };
  assert(
    actual,
    expected,
    `Reflect.getOwnPropertyDescriptor(Dictionary, 'length')`,
  );

  actual = Reflect.getOwnPropertyDescriptor(Dictionary, 'name');
  expected = {
    value: 'Dictionary',
    writable: false,
    enumerable: false,
    configurable: true,
  };
  assert(
    actual,
    expected,
    `Reflect.getOwnPropertyDescriptor(Dictionary, 'name')`,
  );

  actual = Reflect.ownKeys(Dictionary.prototype);
  expected = ['constructor', 'get'];
  assert(actual, expected, `Reflect.ownKeys(Dictionary.prototype)`);

  actual = Reflect.getOwnPropertyDescriptor(
    Dictionary.prototype,
    'constructor',
  );
  expected = {
    writable: true,
    enumerable: false,
    configurable: true,
    value: Dictionary.prototype.constructor,
  };
  assert(
    actual,
    expected,
    `Reflect.getOwnPropertyDescriptor(Dictionary.prototype, 'constructor')`,
  );

  actual = Reflect.getOwnPropertyDescriptor(Dictionary.prototype, 'get');
  expected = {
    writable: true,
    enumerable: true,
    configurable: true,
    value: Dictionary.prototype.get,
  };
  assert(
    actual,
    expected,
    `Reflect.getOwnPropertyDescriptor(Dictionary.prototype, 'get')`,
  );

  assert(
    typeof Dictionary.prototype.constructor,
    'function',
    `typeof Dictionary.prototype.constructor`,
  );
  assert(
    typeof Dictionary.prototype.get,
    'function',
    `typeof Dictionary.prototype.get`,
  );

  actual = Reflect.getOwnPropertyDescriptor(
    Dictionary.prototype.constructor,
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
    `Reflect.getOwnPropertyDescriptor(Dictionary.prototype.constructor, 'length')`,
  );

  actual = Reflect.getOwnPropertyDescriptor(
    Dictionary.prototype.constructor,
    'name',
  );
  expected = {
    value: 'Dictionary',
    writable: false,
    enumerable: false,
    configurable: true,
  };
  assert(
    actual,
    expected,
    `Reflect.getOwnPropertyDescriptor(Dictionary.prototype.constructor, 'name')`,
  );

  actual = Reflect.getOwnPropertyDescriptor(Dictionary.prototype.get, 'length');
  expected = {
    value: 1,
    writable: false,
    enumerable: false,
    configurable: true,
  };
  assert(
    actual,
    expected,
    `Reflect.getOwnPropertyDescriptor(Dictionary.prototype.get, 'length')`,
  );

  actual = Reflect.getOwnPropertyDescriptor(Dictionary.prototype.get, 'name');
  expected = {
    value: 'get',
    writable: false,
    enumerable: false,
    configurable: true,
  };
  assert(
    actual,
    expected,
    `Reflect.getOwnPropertyDescriptor(Dictionary.prototype.get, 'name')`,
  );
}

function createValidDictionary() {
  return new Dictionary(DICTIONARY_NAME);
}
