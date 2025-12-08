/// <reference path="../../../../../types/index.d.ts" />
/* eslint-env serviceworker */

import {
  assert,
  assertDoesNotThrow,
  assertThrows,
  assertRejects,
  iteratableToStream,
  streamToString,
  assertResolves,
} from './assertions.js';
import { SimpleCache, SimpleCacheEntry } from 'fastly:cache';
import { routes, isRunningLocally } from './routes.js';

routes.set('/simple-cache/interface', () => {
  let actual = Reflect.ownKeys(SimpleCache);
  let expected = [
    'prototype',
    'purge',
    'get',
    'getOrSet',
    'set',
    'length',
    'name',
  ];
  assert(actual, expected, `Reflect.ownKeys(SimpleCache)`);

  // Check the prototype descriptors are correct
  {
    actual = Reflect.getOwnPropertyDescriptor(SimpleCache, 'prototype');
    expected = {
      value: SimpleCache.prototype,
      writable: false,
      enumerable: false,
      configurable: false,
    };
    assert(
      actual,
      expected,
      `Reflect.getOwnPropertyDescriptor(SimpleCache, 'prototype')`,
    );
  }

  // Check the constructor function's defined parameter length is correct
  {
    actual = Reflect.getOwnPropertyDescriptor(SimpleCache, 'length');
    expected = {
      value: 0,
      writable: false,
      enumerable: false,
      configurable: true,
    };
    assert(
      actual,
      expected,
      `Reflect.getOwnPropertyDescriptor(SimpleCache, 'length')`,
    );
  }

  // Check the constructor function's name is correct
  {
    actual = Reflect.getOwnPropertyDescriptor(SimpleCache, 'name');
    expected = {
      value: 'SimpleCache',
      writable: false,
      enumerable: false,
      configurable: true,
    };
    assert(
      actual,
      expected,
      `Reflect.getOwnPropertyDescriptor(SimpleCache, 'name')`,
    );
  }

  // Check the prototype has the correct keys
  {
    actual = Reflect.ownKeys(SimpleCache.prototype);
    expected = ['constructor', Symbol.toStringTag];
    assert(actual, expected, `Reflect.ownKeys(SimpleCache.prototype)`);
  }

  // Check the constructor on the prototype is correct
  {
    actual = Reflect.getOwnPropertyDescriptor(
      SimpleCache.prototype,
      'constructor',
    );
    expected = {
      writable: true,
      enumerable: false,
      configurable: true,
      value: SimpleCache.prototype.constructor,
    };
    assert(
      actual,
      expected,
      `Reflect.getOwnPropertyDescriptor(SimpleCache.prototype, 'constructor')`,
    );

    assert(
      typeof SimpleCache.prototype.constructor,
      'function',
      `typeof SimpleCache.prototype.constructor`,
    );

    actual = Reflect.getOwnPropertyDescriptor(
      SimpleCache.prototype.constructor,
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
      `Reflect.getOwnPropertyDescriptor(SimpleCache.prototype.constructor, 'length')`,
    );

    actual = Reflect.getOwnPropertyDescriptor(
      SimpleCache.prototype.constructor,
      'name',
    );
    expected = {
      value: 'SimpleCache',
      writable: false,
      enumerable: false,
      configurable: true,
    };
    assert(
      actual,
      expected,
      `Reflect.getOwnPropertyDescriptor(SimpleCache.prototype.constructor, 'name')`,
    );
  }

  // Check the Symbol.toStringTag on the prototype is correct
  {
    actual = Reflect.getOwnPropertyDescriptor(
      SimpleCache.prototype,
      Symbol.toStringTag,
    );
    expected = {
      writable: false,
      enumerable: false,
      configurable: true,
      value: 'SimpleCache',
    };
    assert(
      actual,
      expected,
      `Reflect.getOwnPropertyDescriptor(SimpleCache.prototype, [Symbol.toStringTag])`,
    );

    assert(
      typeof SimpleCache.prototype[Symbol.toStringTag],
      'string',
      `typeof SimpleCache.prototype[Symbol.toStringTag]`,
    );
  }

  // Check the get static method has correct descriptors, length and name
  {
    actual = Reflect.getOwnPropertyDescriptor(SimpleCache, 'get');
    expected = {
      writable: true,
      enumerable: true,
      configurable: true,
      value: SimpleCache.get,
    };
    assert(
      actual,
      expected,
      `Reflect.getOwnPropertyDescriptor(SimpleCache, 'get')`,
    );

    assert(typeof SimpleCache.get, 'function', `typeof SimpleCache.get`);

    actual = Reflect.getOwnPropertyDescriptor(SimpleCache.get, 'length');
    expected = {
      value: 1,
      writable: false,
      enumerable: false,
      configurable: true,
    };
    assert(
      actual,
      expected,
      `Reflect.getOwnPropertyDescriptor(SimpleCache.get, 'length')`,
    );

    actual = Reflect.getOwnPropertyDescriptor(SimpleCache.get, 'name');
    expected = {
      value: 'get',
      writable: false,
      enumerable: false,
      configurable: true,
    };
    assert(
      actual,
      expected,
      `Reflect.getOwnPropertyDescriptor(SimpleCache.get, 'name')`,
    );
  }

  // Check the set static method has correct descriptors, length and name
  {
    actual = Reflect.getOwnPropertyDescriptor(SimpleCache, 'set');
    expected = {
      writable: true,
      enumerable: true,
      configurable: true,
      value: SimpleCache.set,
    };
    assert(
      actual,
      expected,
      `Reflect.getOwnPropertyDescriptor(SimpleCache, 'set')`,
    );

    assert(typeof SimpleCache.set, 'function', `typeof SimpleCache.set`);

    actual = Reflect.getOwnPropertyDescriptor(SimpleCache.set, 'length');
    expected = {
      value: 3,
      writable: false,
      enumerable: false,
      configurable: true,
    };
    assert(
      actual,
      expected,
      `Reflect.getOwnPropertyDescriptor(SimpleCache.set, 'length')`,
    );

    actual = Reflect.getOwnPropertyDescriptor(SimpleCache.set, 'name');
    expected = {
      value: 'set',
      writable: false,
      enumerable: false,
      configurable: true,
    };
    assert(
      actual,
      expected,
      `Reflect.getOwnPropertyDescriptor(SimpleCache.set, 'name')`,
    );
  }

  // Check the purge static method has correct descriptors, length and name
  {
    actual = Reflect.getOwnPropertyDescriptor(SimpleCache, 'purge');
    expected = {
      writable: true,
      enumerable: true,
      configurable: true,
      value: SimpleCache.purge,
    };
    assert(
      actual,
      expected,
      `Reflect.getOwnPropertyDescriptor(SimpleCache, 'purge')`,
    );

    assert(typeof SimpleCache.purge, 'function', `typeof SimpleCache.purge`);

    actual = Reflect.getOwnPropertyDescriptor(SimpleCache.purge, 'length');
    expected = {
      value: 2,
      writable: false,
      enumerable: false,
      configurable: true,
    };
    assert(
      actual,
      expected,
      `Reflect.getOwnPropertyDescriptor(SimpleCache.purge, 'length')`,
    );

    actual = Reflect.getOwnPropertyDescriptor(SimpleCache.purge, 'name');
    expected = {
      value: 'purge',
      writable: false,
      enumerable: false,
      configurable: true,
    };
    assert(
      actual,
      expected,
      `Reflect.getOwnPropertyDescriptor(SimpleCache.purge, 'name')`,
    );
  }

  // Check the getOrSet static method has correct descriptors, length and name
  {
    actual = Reflect.getOwnPropertyDescriptor(SimpleCache, 'getOrSet');
    expected = {
      writable: true,
      enumerable: true,
      configurable: true,
      value: SimpleCache.getOrSet,
    };
    assert(
      actual,
      expected,
      `Reflect.getOwnPropertyDescriptor(SimpleCache, 'getOrSet')`,
    );

    assert(
      typeof SimpleCache.getOrSet,
      'function',
      `typeof SimpleCache.getOrSet`,
    );

    actual = Reflect.getOwnPropertyDescriptor(SimpleCache.getOrSet, 'length');
    expected = {
      value: 2,
      writable: false,
      enumerable: false,
      configurable: true,
    };
    assert(
      actual,
      expected,
      `Reflect.getOwnPropertyDescriptor(SimpleCache.getOrSet, 'length')`,
    );

    actual = Reflect.getOwnPropertyDescriptor(SimpleCache.getOrSet, 'name');
    expected = {
      value: 'getOrSet',
      writable: false,
      enumerable: false,
      configurable: true,
    };
    assert(
      actual,
      expected,
      `Reflect.getOwnPropertyDescriptor(SimpleCache.getOrSet, 'name')`,
    );
  }
});

// SimpleCache constructor
{
  routes.set('/simple-store/constructor/called-as-regular-function', () => {
    assertThrows(() => {
      SimpleCache();
    }, TypeError);
  });
  routes.set('/simple-cache/constructor/throws', () => {
    assertThrows(() => new SimpleCache(), TypeError);
  });
}

// SimpleCache purge static method
// static purge(key: string, options: PurgeOptions): undefined;
{
  routes.set('/simple-cache/purge/called-as-constructor', () => {
    if (!isRunningLocally()) {
      assertThrows(() => {
        new SimpleCache.purge('1', { scope: 'global' });
      }, TypeError);
    }
  });
  // Ensure we correctly coerce the parameter to a string as according to
  // https://tc39.es/ecma262/#sec-tostring
  routes.set('/simple-cache/purge/key-parameter-calls-7.1.17-ToString', () => {
    if (!isRunningLocally()) {
      let sentinel;
      const test = () => {
        sentinel = Symbol('sentinel');
        const key = {
          toString() {
            throw sentinel;
          },
        };
        SimpleCache.purge(key, { scope: 'global' });
      };
      assertThrows(test);
      try {
        test();
      } catch (thrownError) {
        assert(thrownError, sentinel, 'thrownError === sentinel');
      }
      assertThrows(
        () => {
          SimpleCache.purge(Symbol(), { scope: 'global' });
        },
        TypeError,
        `can't convert symbol to string`,
      );
    }
  });
  routes.set('/simple-cache/purge/key-parameter-not-supplied', () => {
    if (!isRunningLocally()) {
      assertThrows(
        () => {
          SimpleCache.purge();
        },
        TypeError,
        `SimpleCache.purge: At least 2 arguments required, but only 0 passed`,
      );
    }
  });
  routes.set('/simple-cache/purge/key-parameter-empty-string', () => {
    if (!isRunningLocally()) {
      assertThrows(
        () => {
          SimpleCache.purge('', { scope: 'global' });
        },
        Error,
        `SimpleCache.purge: key can not be an empty string`,
      );
    }
  });
  routes.set('/simple-cache/purge/key-parameter-8135-character-string', () => {
    if (!isRunningLocally()) {
      assertDoesNotThrow(() => {
        const key = 'a'.repeat(8135);
        SimpleCache.purge(key, { scope: 'global' });
      });
    }
  });
  routes.set('/simple-cache/purge/key-parameter-8136-character-string', () => {
    if (!isRunningLocally()) {
      assertThrows(
        () => {
          const key = 'a'.repeat(8136);
          SimpleCache.purge(key, { scope: 'global' });
        },
        Error,
        `SimpleCache.purge: key is too long, the maximum allowed length is 8135.`,
      );
    }
  });
  routes.set('/simple-cache/purge/options-parameter', () => {
    if (!isRunningLocally()) {
      assertThrows(
        () => {
          const key = 'a';
          SimpleCache.purge(key, 'hello');
        },
        Error,
        `SimpleCache.purge: options parameter is not an object.`,
      );
      assertThrows(
        () => {
          const key = 'a';
          SimpleCache.purge(key, { scope: Symbol() });
        },
        Error,
        `can't convert symbol to string`,
      );
      assertThrows(
        () => {
          const key = 'a';
          SimpleCache.purge(key, { scope: '' });
        },
        Error,
        `SimpleCache.purge: scope field of options parameter must be either 'pop', or 'global'.`,
      );
      assertDoesNotThrow(() => {
        const key = 'a';
        SimpleCache.purge(key, { scope: 'pop' });
      });
      assertDoesNotThrow(() => {
        const key = 'a';
        SimpleCache.purge(key, { scope: 'global' });
      });
    }
  });
  routes.set('/simple-cache/purge/returns-undefined', () => {
    if (!isRunningLocally()) {
      assert(
        SimpleCache.purge('meow', { scope: 'global' }),
        undefined,
        "SimpleCache.purge('meow', {scope'global'})",
      );
    }
  });
}

// SimpleCache set static method
// static set(key: string, value: BodyInit, ttl: number): undefined;
{
  routes.set('/simple-cache/set/called-as-constructor', () => {
    if (!isRunningLocally()) {
      assertThrows(() => {
        new SimpleCache.set('1', 'meow', 1);
      }, TypeError);
    }
  });
  // Ensure we correctly coerce the key parameter to a string as according to
  // https://tc39.es/ecma262/#sec-tostring
  routes.set('/simple-cache/set/key-parameter-calls-7.1.17-ToString', () => {
    if (!isRunningLocally()) {
      let sentinel;
      const test = () => {
        sentinel = Symbol('sentinel');
        const key = {
          toString() {
            throw sentinel;
          },
        };
        SimpleCache.set(key, 'meow', 1);
      };
      assertThrows(test);
      try {
        test();
      } catch (thrownError) {
        assert(thrownError, sentinel, 'thrownError === sentinel');
      }
      assertThrows(
        () => {
          SimpleCache.set(Symbol(), 'meow', 1);
        },
        TypeError,
        `can't convert symbol to string`,
      );
    }
  });
  // Ensure we correctly coerce the tll parameter to a number as according to
  // https://tc39.es/ecma262/#sec-tonumber
  routes.set('/simple-cache/set/tll-parameter-7.1.4-ToNumber', () => {
    if (!isRunningLocally()) {
      let sentinel;
      let requestedType;
      const test = () => {
        sentinel = Symbol('sentinel');
        const ttl = {
          [Symbol.toPrimitive](type) {
            requestedType = type;
            throw sentinel;
          },
        };
        SimpleCache.set('1', 'meow', ttl);
      };
      assertThrows(test);
      try {
        test();
      } catch (thrownError) {
        assert(thrownError, sentinel, 'thrownError === sentinel');
        assert(requestedType, 'number', 'requestedType === "number"');
      }
      assertThrows(
        () => SimpleCache.set('1', 'meow', Symbol()),
        TypeError,
        `can't convert symbol to number`,
      );
    }
  });
  routes.set('/simple-cache/set/no-parameters-supplied', () => {
    if (!isRunningLocally()) {
      assertThrows(
        () => {
          SimpleCache.set();
        },
        TypeError,
        `SimpleCache.set: At least 3 arguments required, but only 0 passed`,
      );
    }
  });
  routes.set('/simple-cache/set/key-parameter-empty-string', () => {
    if (!isRunningLocally()) {
      assertThrows(
        () => {
          SimpleCache.set('', 'meow', 1);
        },
        Error,
        `SimpleCache.set: key can not be an empty string`,
      );
    }
  });
  routes.set('/simple-cache/set/key-parameter-8135-character-string', () => {
    if (!isRunningLocally()) {
      assertDoesNotThrow(() => {
        const key = 'a'.repeat(8135);
        SimpleCache.set(key, 'meow', 1);
      });
    }
  });
  routes.set('/simple-cache/set/key-parameter-8136-character-string', () => {
    if (!isRunningLocally()) {
      assertThrows(
        () => {
          const key = 'a'.repeat(8136);
          SimpleCache.set(key, 'meow', 1);
        },
        Error,
        `SimpleCache.set: key is too long, the maximum allowed length is 8135.`,
      );
    }
  });
  routes.set('/simple-cache/set/ttl-parameter-negative-number', () => {
    if (!isRunningLocally()) {
      assertThrows(
        () => {
          SimpleCache.set('cat', 'meow', -1);
        },
        Error,
        `SimpleCache.set: TTL parameter is an invalid value, only positive numbers can be used for TTL values.`,
      );
    }
  });
  routes.set('/simple-cache/set/ttl-parameter-NaN', () => {
    if (!isRunningLocally()) {
      assertThrows(
        () => {
          SimpleCache.set('cat', 'meow', NaN);
        },
        Error,
        `SimpleCache.set: TTL parameter is an invalid value, only positive numbers can be used for TTL values.`,
      );
    }
  });
  routes.set('/simple-cache/set/ttl-parameter-Infinity', () => {
    if (!isRunningLocally()) {
      assertThrows(
        () => {
          SimpleCache.set('cat', 'meow', Number.POSITIVE_INFINITY);
        },
        Error,
        `SimpleCache.set: TTL parameter is an invalid value, only positive numbers can be used for TTL values.`,
      );
      assertThrows(
        () => {
          SimpleCache.set('cat', 'meow', Number.NEGATIVE_INFINITY);
        },
        Error,
        `SimpleCache.set: TTL parameter is an invalid value, only positive numbers can be used for TTL values.`,
      );
    }
  });
  routes.set('/simple-cache/set/value-parameter-as-undefined', () => {
    if (!isRunningLocally()) {
      assert(
        SimpleCache.set('meow', undefined, 1),
        undefined,
        'SimpleCache.set("meow", undefined, 1) === undefined',
      );
    }
  });
  // - ReadableStream
  routes.set(
    '/simple-cache/set/value-parameter-readablestream-missing-length-parameter',
    () => {
      if (!isRunningLocally()) {
        // TODO: remove this when streams are supported
        assertThrows(
          () => {
            const stream = iteratableToStream([]);
            SimpleCache.set('meow', stream, 1);
          },
          TypeError,
          `Content-provided streams are not yet supported for streaming into SimpleCache`,
        );
      }
    },
  );
  routes.set(
    '/simple-cache/set/value-parameter-readablestream-negative-length-parameter',
    () => {
      if (!isRunningLocally()) {
        // TODO: remove this when streams are supported
        assertThrows(
          () => {
            const stream = iteratableToStream([]);
            SimpleCache.set('meow', stream, 1, -1);
          },
          TypeError,
          `Content-provided streams are not yet supported for streaming into SimpleCache`,
        );
      }
    },
  );
  routes.set(
    '/simple-cache/set/value-parameter-readablestream-nan-length-parameter',
    () => {
      if (!isRunningLocally()) {
        // TODO: remove this when streams are supported
        assertThrows(
          () => {
            const stream = iteratableToStream([]);
            SimpleCache.set('meow', stream, 1, NaN);
          },
          TypeError,
          `Content-provided streams are not yet supported for streaming into SimpleCache`,
        );
      }
    },
  );
  routes.set(
    '/simple-cache/set/value-parameter-readablestream-negative-infinity-length-parameter',
    () => {
      if (!isRunningLocally()) {
        // TODO: remove this when streams are supported
        assertThrows(
          () => {
            const stream = iteratableToStream([]);
            SimpleCache.set('meow', stream, 1, Number.NEGATIVE_INFINITY);
          },
          TypeError,
          `Content-provided streams are not yet supported for streaming into SimpleCache`,
        );
      }
    },
  );
  routes.set(
    '/simple-cache/set/value-parameter-readablestream-positive-infinity-length-parameter',
    () => {
      if (!isRunningLocally()) {
        // TODO: remove this when streams are supported
        assertThrows(
          () => {
            const stream = iteratableToStream([]);
            SimpleCache.set('meow', stream, 1, Number.POSITIVE_INFINITY);
          },
          TypeError,
          `Content-provided streams are not yet supported for streaming into SimpleCache`,
        );
      }
    },
  );
  // Ensure we correctly coerce the tll parameter to a number as according to
  // https://tc39.es/ecma262/#sec-tonumber
  routes.set('/simple-cache/set/length-parameter-7.1.4-ToNumber', async () => {
    if (!isRunningLocally()) {
      const res = await fetch(
        'https://compute-sdk-test-backend.edgecompute.app/',
        {
          backend: 'TheOrigin',
        },
      );
      let sentinel;
      let requestedType;
      const test = () => {
        sentinel = Symbol('sentinel');
        const length = {
          [Symbol.toPrimitive](type) {
            requestedType = type;
            throw sentinel;
          },
        };
        SimpleCache.set('1', res.body, 1, length);
      };
      assertThrows(test);
      try {
        test();
      } catch (thrownError) {
        assert(thrownError, sentinel, 'thrownError === sentinel');
        assert(requestedType, 'number', 'requestedType === "number"');
      }
      assertThrows(
        () => SimpleCache.set('1', res.body, 1, Symbol()),
        TypeError,
        `can't convert symbol to number`,
      );
    }
  });
  routes.set('/simple-cache/set/value-parameter-readablestream-empty', () => {
    if (!isRunningLocally()) {
      // TODO: remove this when streams are supported
      assertThrows(
        () => {
          const stream = iteratableToStream([]);
          SimpleCache.set('meow', stream, 1, 0);
        },
        TypeError,
        `Content-provided streams are not yet supported for streaming into SimpleCache`,
      );
    }
  });
  routes.set('/simple-cache/set/value-parameter-readablestream-locked', () => {
    if (!isRunningLocally()) {
      const stream = iteratableToStream([]);
      // getReader() causes the stream to become locked
      stream.getReader();
      assertThrows(
        () => {
          SimpleCache.set('meow', stream, 1, 0);
        },
        TypeError,
        `Can't use a ReadableStream that's locked or has ever been read from or canceled`,
      );
    }
  });
  routes.set('/simple-cache/set/value-parameter-readablestream', async () => {
    if (!isRunningLocally()) {
      const res = await fetch(
        'https://compute-sdk-test-backend.edgecompute.app/',
        {
          backend: 'TheOrigin',
        },
      );
      let result = SimpleCache.set(
        'readablestream',
        res.body,
        100,
        res.headers.get('content-length'),
      );
      assert(
        result,
        undefined,
        `SimpleCache.set('readablestream', res.body, 100)`,
      );
    }
  });

  // - URLSearchParams
  routes.set('/simple-cache/set/value-parameter-URLSearchParams', () => {
    if (!isRunningLocally()) {
      const items = [
        new URLSearchParams(),
        new URLSearchParams({ a: 'b', c: 'd' }),
      ];
      for (const searchParams of items) {
        let result = SimpleCache.set('meow', searchParams, 1);
        assert(
          result,
          undefined,
          `SimpleCache.set("meow", searchParams, 1) === undefiend`,
        );
      }
    }
  });
  // - USV strings
  routes.set('/simple-cache/set/value-parameter-strings', () => {
    if (!isRunningLocally()) {
      const strings = [
        // empty
        '',
        // lone surrogate
        '\uD800',
        // surrogate pair
        '𠈓',
        String('carrot'),
      ];
      for (const string of strings) {
        let result = SimpleCache.set('meow', string, 1);
        assert(
          result,
          undefined,
          `SimpleCache.set("meow", string, 1) === undefined`,
        );
      }
    }
  });

  // https://tc39.es/ecma262/#sec-tostring
  routes.set('/simple-cache/set/value-parameter-calls-7.1.17-ToString', () => {
    if (!isRunningLocally()) {
      let sentinel;
      const test = () => {
        sentinel = Symbol('sentinel');
        const value = {
          toString() {
            throw sentinel;
          },
        };
        SimpleCache.set('meow', value, 1);
      };
      assertThrows(test);
      try {
        test();
      } catch (thrownError) {
        assert(thrownError, sentinel, 'thrownError === sentinel');
      }
      assertThrows(
        () => {
          SimpleCache.set('meow', Symbol(), 1);
        },
        TypeError,
        `can't convert symbol to string`,
      );
    }
  });

  // - buffer source
  routes.set('/simple-cache/set/value-parameter-buffer', () => {
    if (!isRunningLocally()) {
      const typedArrayConstructors = [
        Int8Array,
        Int16Array,
        Int32Array,
        Float32Array,
        Float64Array,
        BigInt64Array,
        Uint8Array,
        Uint8ClampedArray,
        Uint16Array,
        Uint32Array,
        BigUint64Array,
      ];
      for (const constructor of typedArrayConstructors) {
        const typedArray = new constructor(8);
        let result = SimpleCache.set('meow', typedArray.buffer, 1);
        assert(
          result,
          undefined,
          `SimpleCache.set("meow", typedArray.buffer, 1) === undefined`,
        );
      }
    }
  });
  routes.set('/simple-cache/set/value-parameter-arraybuffer', () => {
    if (!isRunningLocally()) {
      const typedArrayConstructors = [
        Int8Array,
        Int16Array,
        Int32Array,
        Float32Array,
        Float64Array,
        BigInt64Array,
        Uint8Array,
        Uint8ClampedArray,
        Uint16Array,
        Uint32Array,
        BigUint64Array,
      ];
      for (const constructor of typedArrayConstructors) {
        const typedArray = new constructor(8);
        let result = SimpleCache.set('meow', typedArray.buffer, 1);
        assert(
          result,
          undefined,
          `SimpleCache.set("meow", typedArray.buffer, 1) === undefined`,
        );
      }
    }
  });
  routes.set('/simple-cache/set/value-parameter-typed-arrays', () => {
    if (!isRunningLocally()) {
      const typedArrayConstructors = [
        Int8Array,
        Int16Array,
        Int32Array,
        Float32Array,
        Float64Array,
        BigInt64Array,
        Uint8Array,
        Uint8ClampedArray,
        Uint16Array,
        Uint32Array,
        BigUint64Array,
      ];
      for (const constructor of typedArrayConstructors) {
        const typedArray = new constructor(8);
        let result = SimpleCache.set('meow', typedArray, 1);
        assert(
          result,
          undefined,
          `SimpleCache.set("meow", typedArray, 1) === undefined`,
        );
      }
    }
  });
  routes.set('/simple-cache/set/value-parameter-dataview', () => {
    if (!isRunningLocally()) {
      const typedArrayConstructors = [
        Int8Array,
        Uint8Array,
        Uint8ClampedArray,
        Int16Array,
        Uint16Array,
        Int32Array,
        Uint32Array,
        Float32Array,
        Float64Array,
        BigInt64Array,
        BigUint64Array,
      ];
      for (const constructor of typedArrayConstructors) {
        const typedArray = new constructor(8);
        const view = new DataView(typedArray.buffer);
        let result = SimpleCache.set('meow', view, 1);
        assert(
          result,
          undefined,
          `SimpleCache.set("meow", view, 1) === undefined`,
        );
      }
    }
  });
  routes.set('/simple-cache/set/returns-undefined', () => {
    if (!isRunningLocally()) {
      assert(
        SimpleCache.set('1', 'meow', 1),
        undefined,
        "SimpleCache.set('1', 'meow', 1) === undefined",
      );
    }
  });
}

// SimpleCache get static method
// static get(key: string): SimpleCacheEntry | null;
{
  routes.set('/simple-cache/get/called-as-constructor', () => {
    if (!isRunningLocally()) {
      assertThrows(() => {
        new SimpleCache.get('1');
      }, TypeError);
    }
  });
  // https://tc39.es/ecma262/#sec-tostring
  routes.set('/simple-cache/get/key-parameter-calls-7.1.17-ToString', () => {
    if (!isRunningLocally()) {
      let sentinel;
      const test = () => {
        sentinel = Symbol('sentinel');
        const key = {
          toString() {
            throw sentinel;
          },
        };
        SimpleCache.get(key);
      };
      assertThrows(test);
      try {
        test();
      } catch (thrownError) {
        assert(thrownError, sentinel, 'thrownError === sentinel');
      }
      assertThrows(
        () => {
          SimpleCache.get(Symbol());
        },
        TypeError,
        `can't convert symbol to string`,
      );
    }
  });
  routes.set('/simple-cache/get/key-parameter-not-supplied', () => {
    if (!isRunningLocally()) {
      assertThrows(
        () => {
          SimpleCache.get();
        },
        TypeError,
        `SimpleCache.get: At least 1 argument required, but only 0 passed`,
      );
    }
  });
  routes.set('/simple-cache/get/key-parameter-empty-string', () => {
    if (!isRunningLocally()) {
      assertThrows(
        () => {
          SimpleCache.get('');
        },
        Error,
        `SimpleCache.get: key can not be an empty string`,
      );
    }
  });
  routes.set('/simple-cache/get/key-parameter-8135-character-string', () => {
    if (!isRunningLocally()) {
      assertDoesNotThrow(() => {
        const key = 'a'.repeat(8135);
        SimpleCache.get(key);
      });
    }
  });
  routes.set('/simple-cache/get/key-parameter-8136-character-string', () => {
    if (!isRunningLocally()) {
      assertThrows(
        () => {
          const key = 'a'.repeat(8136);
          SimpleCache.get(key);
        },
        Error,
        `SimpleCache.get: key is too long, the maximum allowed length is 8135.`,
      );
    }
  });
  routes.set('/simple-cache/get/key-does-not-exist-returns-null', () => {
    if (!isRunningLocally()) {
      let result = SimpleCache.get(Math.random());
      assert(result, null, `SimpleCache.get(Math.random()) === null`);
    }
  });
  routes.set('/simple-cache/get/key-exists', () => {
    if (!isRunningLocally()) {
      SimpleCache.set('cat', 'meow', 100);
      let result = SimpleCache.get('cat');
      assert(
        result instanceof SimpleCacheEntry,
        true,
        `SimpleCache.get('cat') instanceof SimpleCacheEntry`,
      );
    }
  });
}

// SimpleCacheEntry
{
  routes.set('/simple-cache-entry/interface', async () => {
    return simpleCacheEntryInterfaceTests();
  });
  routes.set('/simple-cache-entry/text/valid', async () => {
    if (!isRunningLocally()) {
      let key = `entry-text-valid`;
      SimpleCache.set(key, 'hello', 100);
      let entry = SimpleCache.get(key);
      let result = entry.text();
      assert(
        result instanceof Promise,
        true,
        `entry.text() instanceof Promise`,
      );
      result = await result;
      assert(result, 'hello', `await entry.text()`);
    }
  });
  routes.set('/simple-cache-entry/json/valid', async () => {
    if (!isRunningLocally()) {
      let key = `entry-json-valid`;
      const obj = { a: 1, b: 2, c: 3 };
      SimpleCache.set(key, JSON.stringify(obj), 100);
      let entry = SimpleCache.get(key);
      let result = entry.json();
      assert(
        result instanceof Promise,
        true,
        `entry.json() instanceof Promise`,
      );
      result = await result;
      assert(result, obj, `await entry.json()`);
    }
  });
  routes.set('/simple-cache-entry/json/invalid', async () => {
    if (!isRunningLocally()) {
      let key = `entry-json-invalid`;
      SimpleCache.set(key, "132abc;['-=9", 100);
      let entry = SimpleCache.get(key);
      await assertRejects(
        () => entry.json(),
        SyntaxError,
        `JSON.parse: unexpected non-whitespace character after JSON data at line 1 column 4 of the JSON data`,
      );
    }
  });
  routes.set('/simple-cache-entry/arrayBuffer/valid', async () => {
    if (!isRunningLocally()) {
      let key = `entry-arraybuffer-valid`;
      SimpleCache.set(key, new Int8Array([0, 1, 2, 3]), 100);
      let entry = SimpleCache.get(key);
      let result = entry.arrayBuffer();
      assert(
        result instanceof Promise,
        true,
        `entry.arrayBuffer() instanceof Promise`,
      );
      result = await result;
      assert(
        result instanceof ArrayBuffer,
        true,
        `(await entry.arrayBuffer()) instanceof ArrayBuffer`,
      );
    }
  });
  routes.set('/simple-cache-entry/body', async () => {
    if (!isRunningLocally()) {
      let key = `entry-body`;
      SimpleCache.set(key, 'body body body', 100);
      let entry = SimpleCache.get(key);
      let result = entry.body;
      assert(
        result instanceof ReadableStream,
        true,
        `entry.body instanceof ReadableStream`,
      );
      let text = await streamToString(result);
      assert(text, 'body body body', `entry.body contents as string`);
    }
  });
  routes.set('/simple-cache-entry/bodyUsed', async () => {
    if (!isRunningLocally()) {
      let key = `entry-bodyUsed`;
      SimpleCache.set(key, 'body body body', 100);
      let entry = SimpleCache.get(key);
      assert(entry.bodyUsed, false, `entry.bodyUsed`);
      await entry.text();
      assert(entry.bodyUsed, true, `entry.bodyUsed`);
    }
  });
  routes.set('/simple-cache-entry/readablestream', async () => {
    if (!isRunningLocally()) {
      const res = await fetch(
        'https://compute-sdk-test-backend.edgecompute.app/',
        {
          backend: 'TheOrigin',
        },
      );
      let key = `readablestream`;
      SimpleCache.set(key, res.body, 100, res.headers.get('content-length'));
      let entry = SimpleCache.get(key);
      assert(
        await entry.text(),
        'Compute SDK Test Backend',
        `await entry.text()`,
      );
    }
  });
}
async function simpleCacheEntryInterfaceTests() {
  let actual = Reflect.ownKeys(SimpleCacheEntry);
  let expected = ['prototype', 'length', 'name'];
  assert(actual, expected, `Reflect.ownKeys(SimpleCacheEntry)`);

  actual = Reflect.getOwnPropertyDescriptor(SimpleCacheEntry, 'prototype');
  expected = {
    value: SimpleCacheEntry.prototype,
    writable: false,
    enumerable: false,
    configurable: false,
  };
  assert(
    actual,
    expected,
    `Reflect.getOwnPropertyDescriptor(SimpleCacheEntry, 'prototype')`,
  );

  actual = Reflect.getOwnPropertyDescriptor(SimpleCacheEntry, 'length');
  expected = {
    value: 0,
    writable: false,
    enumerable: false,
    configurable: true,
  };
  assert(
    actual,
    expected,
    `Reflect.getOwnPropertyDescriptor(SimpleCacheEntry, 'length')`,
  );

  actual = Reflect.getOwnPropertyDescriptor(SimpleCacheEntry, 'name');
  expected = {
    value: 'SimpleCacheEntry',
    writable: false,
    enumerable: false,
    configurable: true,
  };
  assert(
    actual,
    expected,
    `Reflect.getOwnPropertyDescriptor(SimpleCacheEntry, 'name')`,
  );

  actual = Reflect.ownKeys(SimpleCacheEntry.prototype);
  expected = [
    'constructor',
    'body',
    'bodyUsed',
    'arrayBuffer',
    'json',
    'text',
    Symbol.toStringTag,
  ];
  assert(actual, expected, `Reflect.ownKeys(SimpleCacheEntry.prototype)`);

  actual = Reflect.getOwnPropertyDescriptor(
    SimpleCacheEntry.prototype,
    'constructor',
  );
  expected = {
    writable: true,
    enumerable: false,
    configurable: true,
    value: SimpleCacheEntry.prototype.constructor,
  };
  assert(
    actual,
    expected,
    `Reflect.getOwnPropertyDescriptor(SimpleCacheEntry.prototype, 'constructor')`,
  );

  actual = Reflect.getOwnPropertyDescriptor(SimpleCacheEntry.prototype, 'text');
  expected = {
    writable: true,
    enumerable: true,
    configurable: true,
    value: SimpleCacheEntry.prototype.text,
  };
  assert(
    actual,
    expected,
    `Reflect.getOwnPropertyDescriptor(SimpleCacheEntry.prototype, 'text')`,
  );

  actual = Reflect.getOwnPropertyDescriptor(SimpleCacheEntry.prototype, 'json');
  expected = {
    writable: true,
    enumerable: true,
    configurable: true,
    value: SimpleCacheEntry.prototype.json,
  };
  assert(
    actual,
    expected,
    `Reflect.getOwnPropertyDescriptor(SimpleCacheEntry.prototype, 'json')`,
  );

  actual = Reflect.getOwnPropertyDescriptor(
    SimpleCacheEntry.prototype,
    'arrayBuffer',
  );
  expected = {
    writable: true,
    enumerable: true,
    configurable: true,
    value: SimpleCacheEntry.prototype.arrayBuffer,
  };
  assert(
    actual,
    expected,
    `Reflect.getOwnPropertyDescriptor(SimpleCacheEntry.prototype, 'arrayBuffer')`,
  );
  actual = Reflect.getOwnPropertyDescriptor(SimpleCacheEntry.prototype, 'body');
  assert(
    actual.enumerable,
    true,
    `Reflect.getOwnPropertyDescriptor(SimpleCacheEntry.prototype, 'body').enumerable`,
  );
  assert(
    actual.configurable,
    true,
    `Reflect.getOwnPropertyDescriptor(SimpleCacheEntry.prototype, 'body').configurable`,
  );
  assert(
    'set' in actual,
    true,
    `'set' in Reflect.getOwnPropertyDescriptor(SimpleCacheEntry.prototype, 'body')`,
  );
  assert(
    actual.set,
    undefined,
    `Reflect.getOwnPropertyDescriptor(SimpleCacheEntry.prototype, 'body').set`,
  );
  assert(
    typeof actual.get,
    'function',
    `typeof Reflect.getOwnPropertyDescriptor(SimpleCacheEntry.prototype, 'body').get`,
  );
  actual = Reflect.getOwnPropertyDescriptor(
    SimpleCacheEntry.prototype,
    'bodyUsed',
  );
  assert(
    actual.enumerable,
    true,
    `Reflect.getOwnPropertyDescriptor(SimpleCacheEntry.prototype, 'bodyUsed').enumerable`,
  );
  assert(
    actual.configurable,
    true,
    `Reflect.getOwnPropertyDescriptor(SimpleCacheEntry.prototype, 'bodyUsed').configurable`,
  );
  assert(
    'set' in actual,
    true,
    `'set' in Reflect.getOwnPropertyDescriptor(SimpleCacheEntry.prototype, 'bodyUsed')`,
  );
  assert(
    actual.set,
    undefined,
    `Reflect.getOwnPropertyDescriptor(SimpleCacheEntry.prototype, 'bodyUsed').set`,
  );
  assert(
    typeof actual.get,
    'function',
    `typeof Reflect.getOwnPropertyDescriptor(SimpleCacheEntry.prototype, 'bodyUsed').get`,
  );

  assert(
    typeof SimpleCacheEntry.prototype.constructor,
    'function',
    `typeof SimpleCacheEntry.prototype.constructor`,
  );
  assert(
    typeof SimpleCacheEntry.prototype.text,
    'function',
    `typeof SimpleCacheEntry.prototype.text`,
  );
  assert(
    typeof SimpleCacheEntry.prototype.json,
    'function',
    `typeof SimpleCacheEntry.prototype.json`,
  );
  assert(
    typeof SimpleCacheEntry.prototype.arrayBuffer,
    'function',
    `typeof SimpleCacheEntry.prototype.arrayBuffer`,
  );

  actual = Reflect.getOwnPropertyDescriptor(
    SimpleCacheEntry.prototype.constructor,
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
    `Reflect.getOwnPropertyDescriptor(SimpleCacheEntry.prototype.constructor, 'length')`,
  );

  actual = Reflect.getOwnPropertyDescriptor(
    SimpleCacheEntry.prototype.constructor,
    'name',
  );
  expected = {
    value: 'SimpleCacheEntry',
    writable: false,
    enumerable: false,
    configurable: true,
  };
  assert(
    actual,
    expected,
    `Reflect.getOwnPropertyDescriptor(SimpleCacheEntry.prototype.constructor, 'name')`,
  );

  actual = Reflect.getOwnPropertyDescriptor(
    SimpleCacheEntry.prototype.text,
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
    `Reflect.getOwnPropertyDescriptor(SimpleCacheEntry.prototype.text, 'length')`,
  );

  actual = Reflect.getOwnPropertyDescriptor(
    SimpleCacheEntry.prototype.text,
    'name',
  );
  expected = {
    value: 'text',
    writable: false,
    enumerable: false,
    configurable: true,
  };
  assert(
    actual,
    expected,
    `Reflect.getOwnPropertyDescriptor(SimpleCacheEntry.prototype.text, 'name')`,
  );

  actual = Reflect.getOwnPropertyDescriptor(
    SimpleCacheEntry.prototype.json,
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
    `Reflect.getOwnPropertyDescriptor(SimpleCacheEntry.prototype.json, 'length')`,
  );

  actual = Reflect.getOwnPropertyDescriptor(
    SimpleCacheEntry.prototype.json,
    'name',
  );
  expected = {
    value: 'json',
    writable: false,
    enumerable: false,
    configurable: true,
  };
  assert(
    actual,
    expected,
    `Reflect.getOwnPropertyDescriptor(SimpleCacheEntry.prototype.json, 'name')`,
  );

  actual = Reflect.getOwnPropertyDescriptor(
    SimpleCacheEntry.prototype.arrayBuffer,
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
    `Reflect.getOwnPropertyDescriptor(SimpleCacheEntry.prototype.arrayBuffer, 'length')`,
  );

  actual = Reflect.getOwnPropertyDescriptor(
    SimpleCacheEntry.prototype.arrayBuffer,
    'name',
  );
  expected = {
    value: 'arrayBuffer',
    writable: false,
    enumerable: false,
    configurable: true,
  };
  assert(
    actual,
    expected,
    `Reflect.getOwnPropertyDescriptor(SimpleCacheEntry.prototype.arrayBuffer, 'name')`,
  );
}

// SimpleCache getOrSet static method
// static getOrSet(key: string, set: () => Promise<{value: BodyInit,  ttl: number}>): Promise<SimpleCacheEntry>;
// static getOrSet(key: string, set: () => Promise<{value: ReadableStream, ttl: number, length: number}>): Promise<SimpleCacheEntry>;
{
  routes.set('/simple-cache/getOrSet/rejection-rejects-outer', async () => {
    if (!isRunningLocally()) {
      let key = String(Math.random());
      SimpleCache.get(key);
      assertRejects(
        () =>
          SimpleCache.getOrSet(key, async () => {
            throw RangeError('inner rejection');
          }),
        RangeError,
        'inner rejection',
      );
    }
  });

  routes.set('/simple-cache/getOrSet/called-as-constructor', () => {
    if (!isRunningLocally()) {
      let key = '/simple-cache/getOrSet/called-as-constructor';
      assertThrows(() => {
        new SimpleCache.getOrSet(key, async () => {
          return {
            value: 'meow',
            ttl: 10,
          };
        });
      }, TypeError);
    }
  });
  routes.set('/simple-cache/getOrSet/no-parameters-supplied', async () => {
    if (!isRunningLocally()) {
      await assertRejects(
        async () => {
          await SimpleCache.getOrSet();
        },
        TypeError,
        `SimpleCache.getOrSet: At least 2 arguments required, but only 0 passed`,
      );
    }
  });
  // Ensure we correctly coerce the key parameter to a string as according to
  // https://tc39.es/ecma262/#sec-tostring
  routes.set(
    '/simple-cache/getOrSet/key-parameter-calls-7.1.17-ToString',
    async () => {
      if (!isRunningLocally()) {
        let sentinel = { sentinel: 'sentinel' };
        const test = async () => {
          const key = {
            toString() {
              throw sentinel;
            },
          };
          await SimpleCache.getOrSet(key, async () => {
            return {
              value: 'meow',
              ttl: 10,
            };
          });
        };
        await assertRejects(test);
        try {
          await test();
        } catch (thrownError) {
          assert(thrownError, sentinel, 'thrownError === sentinel');
        }
        await assertRejects(
          async () => {
            await SimpleCache.getOrSet(Symbol(), async () => {
              return {
                value: 'meow',
                ttl: 10,
              };
            });
          },
          TypeError,
          `can't convert symbol to string`,
        );
      }
    },
  );

  // TODO: second parameter not a function

  routes.set('/simple-cache/getOrSet/key-parameter-empty-string', async () => {
    if (!isRunningLocally()) {
      await assertRejects(
        async () => {
          await SimpleCache.getOrSet('', async () => {
            return {
              value: 'meow',
              ttl: 10,
            };
          });
        },
        Error,
        `SimpleCache.getOrSet: key can not be an empty string`,
      );
    }
  });
  routes.set(
    '/simple-cache/getOrSet/key-parameter-8135-character-string',
    async () => {
      if (!isRunningLocally()) {
        await assertDoesNotThrow(async () => {
          const key = 'a'.repeat(8135);
          await SimpleCache.getOrSet(key, async () => {
            return {
              value: 'meow',
              ttl: 10,
            };
          });
        });
      }
    },
  );
  routes.set(
    '/simple-cache/getOrSet/key-parameter-8136-character-string',
    async () => {
      if (!isRunningLocally()) {
        await assertRejects(
          async () => {
            const key = 'a'.repeat(8136);
            await SimpleCache.getOrSet(key, async () => {
              return {
                value: 'meow',
                ttl: 10,
              };
            });
          },
          Error,
          `SimpleCache.getOrSet: key is too long, the maximum allowed length is 8135.`,
        );
      }
    },
  );
  // Ensure we correctly coerce the ttl field to a number as according to
  // https://tc39.es/ecma262/#sec-tonumber
  routes.set('/simple-cache/getOrSet/ttl-field-7.1.4-ToNumber', async () => {
    if (!isRunningLocally()) {
      let key = '/simple-cache/getOrSet/ttl-field-7.1.4-ToNumber';
      let sentinel = { sentinel: 'sentinel' };
      let requestedType;
      const test = async () => {
        const ttl = {
          [Symbol.toPrimitive](type) {
            requestedType = type;
            throw sentinel;
          },
        };
        await SimpleCache.getOrSet(key, async () => {
          return {
            value: 'meow',
            ttl,
          };
        });
      };
      await assertRejects(test);
      try {
        await test();
      } catch (thrownError) {
        assert(thrownError, sentinel, 'thrownError === sentinel');
        assert(requestedType, 'number', 'requestedType === "number"');
      }
      await assertRejects(
        () =>
          SimpleCache.getOrSet(key, async () => {
            return {
              value: 'meow',
              ttl: Symbol(),
            };
          }),
        TypeError,
        `can't convert symbol to number`,
      );
    }
  });
  routes.set('/simple-cache/getOrSet/ttl-field-negative-number', async () => {
    if (!isRunningLocally()) {
      let key = '/simple-cache/getOrSet/ttl-field-negative-number';
      await assertRejects(
        async () => {
          await SimpleCache.getOrSet(key, async () => {
            return {
              value: 'meow',
              ttl: -1,
            };
          });
        },
        Error,
        `SimpleCache.getOrSet: TTL field is an invalid value, only positive numbers can be used for TTL values.`,
      );
    }
  });
  routes.set('/simple-cache/getOrSet/ttl-field-NaN', async () => {
    if (!isRunningLocally()) {
      let key = '/simple-cache/getOrSet/ttl-field-NaN';
      await assertRejects(
        async () => {
          await SimpleCache.getOrSet(key, async () => {
            return {
              value: 'meow',
              ttl: NaN,
            };
          });
        },
        Error,
        `SimpleCache.getOrSet: TTL field is an invalid value, only positive numbers can be used for TTL values.`,
      );
    }
  });
  routes.set('/simple-cache/getOrSet/ttl-field-Infinity', async () => {
    if (!isRunningLocally()) {
      let key = '/simple-cache/getOrSet/ttl-field-Infinity';
      await assertRejects(
        async () => {
          await SimpleCache.getOrSet(key, async () => {
            return {
              value: 'meow',
              ttl: Number.POSITIVE_INFINITY,
            };
          });
        },
        Error,
        `SimpleCache.getOrSet: TTL field is an invalid value, only positive numbers can be used for TTL values.`,
      );
      await assertRejects(
        async () => {
          await SimpleCache.getOrSet(key, async () => {
            return {
              value: 'meow',
              ttl: Number.NEGATIVE_INFINITY,
            };
          });
        },
        Error,
        `SimpleCache.getOrSet: TTL field is an invalid value, only positive numbers can be used for TTL values.`,
      );
    }
  });
  routes.set('/simple-cache/getOrSet/value-field-as-undefined', async () => {
    if (!isRunningLocally()) {
      let key = '/simple-cache/getOrSet/value-field-as-undefined';
      await assertResolves(async () => {
        await SimpleCache.getOrSet(key, async () => {
          return { value: undefined, ttl: 1 };
        });
      });
    }
  });
  // - ReadableStream
  routes.set(
    '/simple-cache/getOrSet/value-field-readablestream-missing-length-field',
    async () => {
      if (!isRunningLocally()) {
        let key =
          '/simple-cache/getOrSet/value-field-readablestream-missing-length-field';
        // TODO: remove this when streams are supported
        await assertRejects(
          async () => {
            const stream = iteratableToStream([]);
            await SimpleCache.getOrSet(key, async () => {
              return { value: stream, ttl: 1 };
            });
          },
          TypeError,
          `Content-provided streams are not yet supported for streaming into SimpleCache`,
        );
      }
    },
  );
  routes.set(
    '/simple-cache/getOrSet/value-field-readablestream-negative-length-field',
    async () => {
      if (!isRunningLocally()) {
        let key =
          '/simple-cache/getOrSet/value-field-readablestream-negative-length-field';
        // TODO: remove this when streams are supported
        await assertRejects(
          async () => {
            const stream = iteratableToStream([]);
            await SimpleCache.getOrSet(key, async () => {
              return { value: stream, ttl: 1, length: -1 };
            });
          },
          TypeError,
          `Content-provided streams are not yet supported for streaming into SimpleCache`,
        );
      }
    },
  );
  routes.set(
    '/simple-cache/getOrSet/value-field-readablestream-nan-length-field',
    async () => {
      if (!isRunningLocally()) {
        let key =
          '/simple-cache/getOrSet/value-field-readablestream-nan-length-field';
        // TODO: remove this when streams are supported
        await assertRejects(
          async () => {
            const stream = iteratableToStream([]);
            await SimpleCache.getOrSet(key, async () => {
              return { value: stream, ttl: 1, length: NaN };
            });
          },
          TypeError,
          `Content-provided streams are not yet supported for streaming into SimpleCache`,
        );
      }
    },
  );
  routes.set(
    '/simple-cache/getOrSet/value-field-readablestream-negative-infinity-length-field',
    async () => {
      if (!isRunningLocally()) {
        let key =
          '/simple-cache/getOrSet/value-field-readablestream-negative-infinity-length-field';
        // TODO: remove this when streams are supported
        await assertRejects(
          async () => {
            const stream = iteratableToStream([]);
            await SimpleCache.getOrSet(key, async () => {
              return {
                value: stream,
                ttl: 1,
                length: Number.NEGATIVE_INFINITY,
              };
            });
          },
          TypeError,
          `Content-provided streams are not yet supported for streaming into SimpleCache`,
        );
      }
    },
  );
  routes.set(
    '/simple-cache/getOrSet/value-field-readablestream-positive-infinity-length-field',
    async () => {
      if (!isRunningLocally()) {
        let key =
          '/simple-cache/getOrSet/value-field-readablestream-positive-infinity-length-field';
        // TODO: remove this when streams are supported
        await assertRejects(
          async () => {
            const stream = iteratableToStream([]);
            await SimpleCache.getOrSet(key, async () => {
              return {
                value: stream,
                ttl: 1,
                length: Number.POSITIVE_INFINITY,
              };
            });
          },
          TypeError,
          `Content-provided streams are not yet supported for streaming into SimpleCache`,
        );
      }
    },
  );
  // Ensure we correctly coerce the length field to a number as according to
  // https://tc39.es/ecma262/#sec-tonumber
  routes.set('/simple-cache/getOrSet/length-field-7.1.4-ToNumber', async () => {
    if (!isRunningLocally()) {
      let key = '/simple-cache/getOrSet/length-field-7.1.4-ToNumber';
      const res = await fetch(
        'https://compute-sdk-test-backend.edgecompute.app/',
        {
          backend: 'TheOrigin',
        },
      );
      let sentinel = { sentinel: 'sentinel' };
      let requestedType;
      const test = async () => {
        const length = {
          [Symbol.toPrimitive](type) {
            requestedType = type;
            throw sentinel;
          },
        };
        await SimpleCache.getOrSet(key, async () => {
          return { value: res.body, ttl: 1, length };
        });
      };
      await assertRejects(test);
      try {
        await test();
      } catch (thrownError) {
        assert(thrownError, sentinel, 'thrownError === sentinel');
        assert(requestedType, 'number', 'requestedType === "number"');
      }
      await assertRejects(
        async () =>
          await SimpleCache.getOrSet(key, async () => {
            return { value: res.body, ttl: 1, length: Symbol() };
          }),
        TypeError,
        `can't convert symbol to number`,
      );
    }
  });
  routes.set(
    '/simple-cache/getOrSet/value-field-readablestream-empty',
    async () => {
      if (!isRunningLocally()) {
        let key = '/simple-cache/getOrSet/value-field-readablestream-empty';
        // TODO: remove this when streams are supported
        await assertRejects(
          async () => {
            const stream = iteratableToStream([]);
            await SimpleCache.getOrSet(key, async () => {
              return { value: stream, ttl: 1, length: 0 };
            });
          },
          TypeError,
          `Content-provided streams are not yet supported for streaming into SimpleCache`,
        );
      }
    },
  );
  routes.set(
    '/simple-cache/getOrSet/value-field-readablestream-locked',
    async () => {
      if (!isRunningLocally()) {
        let key = '/simple-cache/getOrSet/value-field-readablestream-locked';
        const stream = iteratableToStream([]);
        // getReader() causes the stream to become locked
        stream.getReader();
        await assertRejects(
          async () => {
            await SimpleCache.getOrSet(key, async () => {
              return { value: stream, ttl: 1, length: 0 };
            });
          },
          TypeError,
          `Can't use a ReadableStream that's locked or has ever been read from or canceled`,
        );
      }
    },
  );
  routes.set('/simple-cache/getOrSet/value-field-readablestream', async () => {
    if (!isRunningLocally()) {
      let key = '/simple-cache/getOrSet/value-field-readablestream';
      const res = await fetch(
        'https://compute-sdk-test-backend.edgecompute.app/',
        {
          backend: 'TheOrigin',
        },
      );
      await assertResolves(async () => {
        await SimpleCache.getOrSet(key, async () => {
          return {
            value: res.body,
            ttl: 100,
            length: res.headers.get('content-length'),
          };
        });
      });
    }
  });

  // - URLSearchParams
  routes.set('/simple-cache/getOrSet/value-field-URLSearchParams', async () => {
    if (!isRunningLocally()) {
      let key = '/simple-cache/getOrSet/value-field-URLSearchParams';
      const items = [
        new URLSearchParams(),
        new URLSearchParams({ a: 'b', c: 'd' }),
      ];
      for (const searchParams of items) {
        await assertResolves(async () => {
          await SimpleCache.getOrSet(key, async () => {
            return { value: searchParams, ttl: 1 };
          });
        });
      }
    }
  });
  // - USV strings
  routes.set('/simple-cache/getOrSet/value-field-strings', async () => {
    if (!isRunningLocally()) {
      let key = '/simple-cache/getOrSet/value-field-strings';
      const strings = [
        // empty
        '',
        // lone surrogate
        '\uD800',
        // surrogate pair
        '𠈓',
        String('carrot'),
      ];
      for (const string of strings) {
        await assertResolves(async () => {
          await SimpleCache.getOrSet(key, async () => {
            return { value: string, ttl: 1 };
          });
        });
      }
    }
  });

  // https://tc39.es/ecma262/#sec-tostring
  routes.set(
    '/simple-cache/getOrSet/value-field-calls-7.1.17-ToString',
    async () => {
      if (!isRunningLocally()) {
        let key = '/simple-cache/getOrSet/value-field-calls-7.1.17-ToString';
        let sentinel = { sentinel: 'sentinel' };
        const test = async () => {
          const value = {
            toString() {
              throw sentinel;
            },
          };
          await SimpleCache.getOrSet(key, async () => {
            return { value, ttl: 1 };
          });
        };
        await assertRejects(test);
        try {
          await test();
        } catch (thrownError) {
          assert(thrownError, sentinel, 'thrownError === sentinel');
        }
        await assertRejects(
          async () => {
            await SimpleCache.getOrSet(key, async () => {
              return { value: Symbol(), ttl: 1 };
            });
          },
          TypeError,
          `can't convert symbol to string`,
        );
      }
    },
  );

  // - buffer source
  routes.set('/simple-cache/getOrSet/value-field-buffer', async () => {
    if (!isRunningLocally()) {
      let key = '/simple-cache/getOrSet/value-field-buffer';
      const typedArrayConstructors = [
        Int8Array,
        Int16Array,
        Int32Array,
        Float32Array,
        Float64Array,
        BigInt64Array,
        Uint8Array,
        Uint8ClampedArray,
        Uint16Array,
        Uint32Array,
        BigUint64Array,
      ];
      for (const constructor of typedArrayConstructors) {
        const typedArray = new constructor(8);
        await assertResolves(async () => {
          await SimpleCache.getOrSet(key + constructor.name, async () => {
            return { value: typedArray.buffer, ttl: 1 };
          });
        });
      }
    }
  });

  routes.set('/simple-cache/getOrSet/value-field-typed-arrays', async () => {
    if (!isRunningLocally()) {
      let key = '/simple-cache/getOrSet/value-field-typed-arrays';
      const typedArrayConstructors = [
        Int8Array,
        Int16Array,
        Int32Array,
        Float32Array,
        Float64Array,
        BigInt64Array,
        Uint8Array,
        Uint8ClampedArray,
        Uint16Array,
        Uint32Array,
        BigUint64Array,
      ];
      for (const constructor of typedArrayConstructors) {
        const typedArray = new constructor(8);
        await assertResolves(async () => {
          await SimpleCache.getOrSet(key + constructor.name, async () => {
            return { value: typedArray, ttl: 1 };
          });
        });
      }
    }
  });
  routes.set('/simple-cache/getOrSet/value-field-dataview', async () => {
    if (!isRunningLocally()) {
      let key = '/simple-cache/getOrSet/value-field-dataview';
      const typedArrayConstructors = [
        Int8Array,
        Uint8Array,
        Uint8ClampedArray,
        Int16Array,
        Uint16Array,
        Int32Array,
        Uint32Array,
        Float32Array,
        Float64Array,
        BigInt64Array,
        BigUint64Array,
      ];
      for (const constructor of typedArrayConstructors) {
        const typedArray = new constructor(8);
        const view = new DataView(typedArray.buffer);
        await assertResolves(async () => {
          await SimpleCache.getOrSet(key + constructor.name, async () => {
            return { value: view, ttl: 1 };
          });
        });
      }
    }
  });
  routes.set('/simple-cache/getOrSet/returns-SimpleCacheEntry', async () => {
    if (!isRunningLocally()) {
      let key = '/simple-cache/getOrSet/returns-SimpleCacheEntry';
      let result = await SimpleCache.getOrSet(key, async () => {
        return {
          value: 'meow',
          ttl: 10,
        };
      });
      assert(
        result instanceof SimpleCacheEntry,
        true,
        'result instanceof SimpleCacheEntry',
      );
    }
  });
  routes.set(
    '/simple-cache/getOrSet/executes-the-set-method-when-key-not-in-cache',
    async () => {
      if (!isRunningLocally()) {
        let called = false;
        await SimpleCache.getOrSet(Math.random(), async () => {
          called = true;
          return {
            value: 'meow',
            ttl: 10,
          };
        });
        assert(called, true, 'called === true');
      }
    },
  );
  routes.set(
    '/simple-cache/getOrSet/does-not-execute-the-set-method-when-key-is-in-cache',
    async () => {
      if (!isRunningLocally()) {
        let key = Math.random();
        SimpleCache.set(key, 'meow', 100);
        let called = false;
        await SimpleCache.getOrSet(key, async () => {
          called = true;
          return {
            value: 'meow',
            ttl: 10,
          };
        });
        assert(called, false, 'called === false');
      }
    },
  );
  routes.set(
    '/simple-cache/getOrSet/does-not-freeze-when-called-after-a-get',
    async () => {
      if (!isRunningLocally()) {
        let key = String(Math.random());
        SimpleCache.get(key);
        await SimpleCache.getOrSet(key, async () => {
          return {
            value: key,
            ttl: 10,
          };
        });
      }
    },
  );
  routes.set('/simple-cache/getOrSet/integers-1-to-50', async () => {
    await Promise.all(
      Array.from({ length: 50 }, (_, i) => i + 1).map((i) =>
        SimpleCache.getOrSet(`key-${i}`, async () => ({
          value: await fetch("https://http-me.glitch.me/time", { backend: 'httpme', cacheOverride: new CacheOverride('pass') }),
          ttl: 10,
        })),
      ),
    );
  });
}
