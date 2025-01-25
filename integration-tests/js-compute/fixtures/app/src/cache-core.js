/// <reference path="../../../../../types/index.d.ts" />
/* eslint-env serviceworker */

import {
  assert,
  assertDoesNotThrow,
  assertThrows,
  sleep,
  streamToString,
  assertResolves,
} from './assertions.js';
import { routes } from './routes.js';
import {
  CoreCache,
  CacheEntry,
  CacheState,
  TransactionCacheEntry,
} from 'fastly:cache';
import { FastlyBody } from 'fastly:body';

function iteratableToStream(iterable) {
  return new ReadableStream({
    async pull(controller) {
      for await (const value of iterable) {
        controller.enqueue(value);
      }
      controller.close();
    },
  });
}

let createdLion = false;
function ensureLion() {
  if (createdLion) return;
  const writer = CoreCache.insert('lion', {
    maxAge: 600_000,
  });
  writer.append('raar');
  writer.close();
  createdLion = true;
}

// FastlyBody
{
  routes.set('/FastlyBody/interface', () => {
    let actual = Reflect.ownKeys(FastlyBody);
    let expected = ['prototype', 'length', 'name'];
    assert(actual, expected, `Reflect.ownKeys(FastlyBody)`);

    // Check the prototype descriptors are correct
    {
      actual = Reflect.getOwnPropertyDescriptor(FastlyBody, 'prototype');
      expected = {
        value: FastlyBody.prototype,
        writable: false,
        enumerable: false,
        configurable: false,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(FastlyBody, 'prototype')`,
      );
    }

    // Check the constructor function's defined parameter length is correct
    {
      actual = Reflect.getOwnPropertyDescriptor(FastlyBody, 'length');
      expected = {
        value: 0,
        writable: false,
        enumerable: false,
        configurable: true,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(FastlyBody, 'length')`,
      );
    }

    // Check the constructor function's name is correct
    {
      actual = Reflect.getOwnPropertyDescriptor(FastlyBody, 'name');
      expected = {
        value: 'FastlyBody',
        writable: false,
        enumerable: false,
        configurable: true,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(FastlyBody, 'name')`,
      );
    }

    // Check the prototype has the correct keys
    {
      actual = Reflect.ownKeys(FastlyBody.prototype);
      expected = [
        'constructor',
        'concat',
        'read',
        'append',
        'prepend',
        'close',
        Symbol.toStringTag,
      ];
      assert(actual, expected, `Reflect.ownKeys(FastlyBody.prototype)`);
    }

    // Check the constructor on the prototype is correct
    {
      actual = Reflect.getOwnPropertyDescriptor(
        FastlyBody.prototype,
        'constructor',
      );
      expected = {
        writable: true,
        enumerable: false,
        configurable: true,
        value: FastlyBody.prototype.constructor,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(FastlyBody.prototype, 'constructor')`,
      );

      assert(
        typeof FastlyBody.prototype.constructor,
        'function',
        `typeof FastlyBody.prototype.constructor`,
      );

      actual = Reflect.getOwnPropertyDescriptor(
        FastlyBody.prototype.constructor,
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
        `Reflect.getOwnPropertyDescriptor(FastlyBody.prototype.constructor, 'length')`,
      );

      actual = Reflect.getOwnPropertyDescriptor(
        FastlyBody.prototype.constructor,
        'name',
      );
      expected = {
        value: 'FastlyBody',
        writable: false,
        enumerable: false,
        configurable: true,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(FastlyBody.prototype.constructor, 'name')`,
      );
    }

    // Check the Symbol.toStringTag on the prototype is correct
    {
      actual = Reflect.getOwnPropertyDescriptor(
        FastlyBody.prototype,
        Symbol.toStringTag,
      );
      expected = {
        writable: false,
        enumerable: false,
        configurable: true,
        value: 'FastlyBody',
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(FastlyBody.prototype, [Symbol.toStringTag])`,
      );

      assert(
        typeof FastlyBody.prototype[Symbol.toStringTag],
        'string',
        `typeof FastlyBody.prototype[Symbol.toStringTag]`,
      );
    }

    // Check the concat method has correct descriptors, length and name
    {
      actual = Reflect.getOwnPropertyDescriptor(FastlyBody.prototype, 'concat');
      expected = {
        writable: true,
        enumerable: true,
        configurable: true,
        value: FastlyBody.prototype.concat,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(FastlyBody.prototype, 'concat')`,
      );

      assert(
        typeof FastlyBody.prototype.concat,
        'function',
        `typeof FastlyBody.prototype.concat`,
      );

      actual = Reflect.getOwnPropertyDescriptor(
        FastlyBody.prototype.concat,
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
        `Reflect.getOwnPropertyDescriptor(FastlyBody.prototype.concat, 'length')`,
      );

      actual = Reflect.getOwnPropertyDescriptor(
        FastlyBody.prototype.concat,
        'name',
      );
      expected = {
        value: 'concat',
        writable: false,
        enumerable: false,
        configurable: true,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(FastlyBody.prototype.concat, 'name')`,
      );
    }

    // Check the read method has correct descriptors, length and name
    {
      actual = Reflect.getOwnPropertyDescriptor(FastlyBody.prototype, 'read');
      expected = {
        writable: true,
        enumerable: true,
        configurable: true,
        value: FastlyBody.prototype.read,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(FastlyBody.prototype, 'read')`,
      );

      assert(
        typeof FastlyBody.prototype.read,
        'function',
        `typeof FastlyBody.prototype.read`,
      );

      actual = Reflect.getOwnPropertyDescriptor(
        FastlyBody.prototype.read,
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
        `Reflect.getOwnPropertyDescriptor(FastlyBody.prototype.read, 'length')`,
      );

      actual = Reflect.getOwnPropertyDescriptor(
        FastlyBody.prototype.read,
        'name',
      );
      expected = {
        value: 'read',
        writable: false,
        enumerable: false,
        configurable: true,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(FastlyBody.prototype.read, 'name')`,
      );
    }

    // Check the append method has correct descriptors, length and name
    {
      actual = Reflect.getOwnPropertyDescriptor(FastlyBody.prototype, 'append');
      expected = {
        writable: true,
        enumerable: true,
        configurable: true,
        value: FastlyBody.prototype.append,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(FastlyBody.prototype, 'append')`,
      );

      assert(
        typeof FastlyBody.prototype.append,
        'function',
        `typeof FastlyBody.prototype.append`,
      );

      actual = Reflect.getOwnPropertyDescriptor(
        FastlyBody.prototype.append,
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
        `Reflect.getOwnPropertyDescriptor(FastlyBody.prototype.append, 'length')`,
      );

      actual = Reflect.getOwnPropertyDescriptor(
        FastlyBody.prototype.append,
        'name',
      );
      expected = {
        value: 'append',
        writable: false,
        enumerable: false,
        configurable: true,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(FastlyBody.prototype.append, 'name')`,
      );
    }

    // Check the prepend method has correct descriptors, length and name
    {
      actual = Reflect.getOwnPropertyDescriptor(
        FastlyBody.prototype,
        'prepend',
      );
      expected = {
        writable: true,
        enumerable: true,
        configurable: true,
        value: FastlyBody.prototype.prepend,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(FastlyBody.prototype, 'prepend')`,
      );

      assert(
        typeof FastlyBody.prototype.prepend,
        'function',
        `typeof FastlyBody.prototype.prepend`,
      );

      actual = Reflect.getOwnPropertyDescriptor(
        FastlyBody.prototype.prepend,
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
        `Reflect.getOwnPropertyDescriptor(FastlyBody.prototype.prepend, 'length')`,
      );

      actual = Reflect.getOwnPropertyDescriptor(
        FastlyBody.prototype.prepend,
        'name',
      );
      expected = {
        value: 'prepend',
        writable: false,
        enumerable: false,
        configurable: true,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(FastlyBody.prototype.prepend, 'name')`,
      );
    }

    // Check the close method has correct descriptors, length and name
    {
      actual = Reflect.getOwnPropertyDescriptor(FastlyBody.prototype, 'close');
      expected = {
        writable: true,
        enumerable: true,
        configurable: true,
        value: FastlyBody.prototype.close,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(FastlyBody.prototype, 'close')`,
      );

      assert(
        typeof FastlyBody.prototype.close,
        'function',
        `typeof FastlyBody.prototype.close`,
      );

      actual = Reflect.getOwnPropertyDescriptor(
        FastlyBody.prototype.close,
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
        `Reflect.getOwnPropertyDescriptor(FastlyBody.prototype.close, 'length')`,
      );

      actual = Reflect.getOwnPropertyDescriptor(
        FastlyBody.prototype.close,
        'name',
      );
      expected = {
        value: 'close',
        writable: false,
        enumerable: false,
        configurable: true,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(FastlyBody.prototype.close, 'name')`,
      );
    }
  });

  // constructor
  {
    routes.set('/FastlyBody/constructor/called-as-regular-function', () => {
      assertThrows(() => {
        FastlyBody();
      }, TypeError);
    });
    routes.set('/FastlyBody/constructor/called-as-constructor', () => {
      assertDoesNotThrow(() => new FastlyBody());
    });
  }
  // append(data: BodyInit): void;
  {
    routes.set('/FastlyBody/append/called-as-constructor', () => {
      assertThrows(() => {
        new FastlyBody.append('1');
      }, TypeError);
    });
    routes.set('/FastlyBody/append/data-parameter-not-supplied', () => {
      assertThrows(
        () => {
          const body = new FastlyBody();
          body.append();
        },
        TypeError,
        `append: At least 1 argument required, but only 0 passed`,
      );
    });
    routes.set('/FastlyBody/append/data-parameter-wrong-type', () => {
      assertThrows(() => {
        const body = new FastlyBody();
        body.append(Symbol());
      });
    });
    // - ReadableStream
    routes.set(
      '/FastlyBody/append/data-parameter-readablestream-guest-backed',
      () => {
        // TODO: update this when streams are supported
        assertThrows(
          () => {
            const stream = iteratableToStream([]);
            const body = new FastlyBody();
            body.append(stream);
          },
          TypeError,
          `Content-provided streams are not yet supported for appending onto a FastlyBody`,
        );
      },
    );
    routes.set(
      '/FastlyBody/append/data-parameter-readablestream-host-backed',
      async () => {
        const res = await fetch(
          'https://compute-sdk-test-backend.edgecompute.app/',
          {
            backend: 'TheOrigin',
          },
        );
        const body = new FastlyBody();
        let result = body.append(res.body);
        assert(result, undefined, `body.append(res.body)`);
      },
    );
    // - URLSearchParams
    routes.set('/FastlyBody/append/data-parameter-URLSearchParams', () => {
      const items = [
        new URLSearchParams(),
        new URLSearchParams({ a: 'b', c: 'd' }),
      ];
      const body = new FastlyBody();
      for (const searchParams of items) {
        let result = body.append(searchParams);
        assert(result, undefined, `await body.append(searchParams)`);
      }
    });
    // - USV strings
    routes.set('/FastlyBody/append/data-parameter-strings', () => {
      const strings = [
        // empty
        '',
        // lone surrogate
        '\uD800',
        // surrogate pair
        '𠈓',
        String('carrot'),
      ];
      const body = new FastlyBody();
      for (const string of strings) {
        let result = body.append(string);
        assert(result, undefined, `body.append(string)`);
      }
    });
    // https://tc39.es/ecma262/#sec-tostring
    routes.set(
      '/FastlyBody/append/data-parameter-calls-7.1.17-ToString',
      () => {
        let sentinel;
        const test = () => {
          sentinel = Symbol();
          const value = {
            toString() {
              throw sentinel;
            },
          };
          const body = new FastlyBody();
          body.append(value);
        };
        assertThrows(test);
        try {
          test();
        } catch (thrownError) {
          assert(thrownError, sentinel, 'thrownError === sentinel');
        }
        assertThrows(
          () => {
            const body = new FastlyBody();
            body.append(Symbol());
          },
          TypeError,
          `can't convert symbol to string`,
        );
      },
    );

    // - buffer source
    routes.set('/FastlyBody/append/data-parameter-buffer', () => {
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
      const body = new FastlyBody();
      for (const constructor of typedArrayConstructors) {
        const typedArray = new constructor(8);
        let result = body.append(typedArray.buffer);
        assert(result, undefined, `body.append(typedArray.buffer)`);
      }
    });
    routes.set('/FastlyBody/append/data-parameter-arraybuffer', () => {
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
      const body = new FastlyBody();
      for (const constructor of typedArrayConstructors) {
        const typedArray = new constructor(8);
        let result = body.append(typedArray.buffer);
        assert(result, undefined, `body.append(typedArray.buffer)`);
      }
    });
    routes.set('/FastlyBody/append/data-parameter-typed-arrays', () => {
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
      const body = new FastlyBody();
      for (const constructor of typedArrayConstructors) {
        const typedArray = new constructor(8);
        let result = body.append(typedArray);
        assert(result, undefined, `body.append(typedArray)`);
      }
    });
    routes.set('/FastlyBody/append/data-parameter-dataview', () => {
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
      const body = new FastlyBody();
      for (const constructor of typedArrayConstructors) {
        const typedArray = new constructor(8);
        const view = new DataView(typedArray.buffer);
        let result = body.append(view);
        assert(result, undefined, `body.append(typedArray)`);
      }
    });
  }
  // prepend(data: BodyInit): void;
  {
    routes.set('/FastlyBody/prepend/called-as-constructor', () => {
      assertThrows(() => {
        new FastlyBody.prepend('1');
      }, TypeError);
    });
    routes.set('/FastlyBody/prepend/data-parameter-not-supplied', () => {
      assertThrows(
        () => {
          const body = new FastlyBody();
          body.prepend();
        },
        TypeError,
        `prepend: At least 1 argument required, but only 0 passed`,
      );
    });
    routes.set('/FastlyBody/prepend/data-parameter-wrong-type', () => {
      assertThrows(() => {
        const body = new FastlyBody();
        body.prepend(Symbol());
      });
    });
    // - ReadableStream
    routes.set(
      '/FastlyBody/prepend/data-parameter-readablestream-guest-backed',
      () => {
        // TODO: update this when streams are supported
        assertThrows(
          () => {
            const stream = iteratableToStream([]);
            const body = new FastlyBody();
            body.prepend(stream);
          },
          TypeError,
          `Content-provided streams are not yet supported for prepending onto a FastlyBody`,
        );
      },
    );
    routes.set(
      '/FastlyBody/prepend/data-parameter-readablestream-host-backed',
      async () => {
        const res = await fetch(
          'https://compute-sdk-test-backend.edgecompute.app/',
          {
            backend: 'TheOrigin',
          },
        );
        const body = new FastlyBody();
        let result = body.prepend(res.body);
        assert(result, undefined, `body.prepend(res.body)`);
      },
    );
    routes.set(
      '/FastlyBody/prepend/data-parameter-readablestream-locked',
      () => {
        const stream = iteratableToStream([]);
        // getReader() causes the stream to become locked
        stream.getReader();
        const body = new FastlyBody();
        assertThrows(
          () => {
            body.prepend(stream);
          },
          TypeError,
          `Can't use a ReadableStream that's locked or has ever been read from or canceled`,
        );
      },
    );

    // - URLSearchParams
    routes.set('/FastlyBody/prepend/data-parameter-URLSearchParams', () => {
      const items = [
        new URLSearchParams(),
        new URLSearchParams({ a: 'b', c: 'd' }),
      ];
      const body = new FastlyBody();
      for (const searchParams of items) {
        let result = body.prepend(searchParams);
        assert(result, undefined, `await body.prepend(searchParams)`);
      }
    });
    // - USV strings
    routes.set('/FastlyBody/prepend/data-parameter-strings', () => {
      const strings = [
        // empty
        '',
        // lone surrogate
        '\uD800',
        // surrogate pair
        '𠈓',
        String('carrot'),
      ];
      const body = new FastlyBody();
      for (const string of strings) {
        let result = body.prepend(string);
        assert(result, undefined, `body.prepend(string)`);
      }
    });
    // https://tc39.es/ecma262/#sec-tostring
    routes.set(
      '/FastlyBody/prepend/data-parameter-calls-7.1.17-ToString',
      () => {
        let sentinel;
        const test = () => {
          sentinel = Symbol();
          const value = {
            toString() {
              throw sentinel;
            },
          };
          const body = new FastlyBody();
          body.prepend(value);
        };
        assertThrows(test);
        try {
          test();
        } catch (thrownError) {
          assert(thrownError, sentinel, 'thrownError === sentinel');
        }
        assertThrows(
          () => {
            const body = new FastlyBody();
            body.prepend(Symbol());
          },
          TypeError,
          `can't convert symbol to string`,
        );
      },
    );

    // - buffer source
    routes.set('/FastlyBody/prepend/data-parameter-buffer', () => {
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
      const body = new FastlyBody();
      for (const constructor of typedArrayConstructors) {
        const typedArray = new constructor(8);
        let result = body.prepend(typedArray.buffer);
        assert(result, undefined, `body.prepend(typedArray.buffer)`);
      }
    });
    routes.set('/FastlyBody/prepend/data-parameter-arraybuffer', () => {
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
      const body = new FastlyBody();
      for (const constructor of typedArrayConstructors) {
        const typedArray = new constructor(8);
        let result = body.prepend(typedArray.buffer);
        assert(result, undefined, `body.prepend(typedArray.buffer)`);
      }
    });
    routes.set('/FastlyBody/prepend/data-parameter-typed-arrays', () => {
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
      const body = new FastlyBody();
      for (const constructor of typedArrayConstructors) {
        const typedArray = new constructor(8);
        let result = body.prepend(typedArray);
        assert(result, undefined, `body.prepend(typedArray)`);
      }
    });
    routes.set('/FastlyBody/prepend/data-parameter-dataview', () => {
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
      const body = new FastlyBody();
      for (const constructor of typedArrayConstructors) {
        const typedArray = new constructor(8);
        const view = new DataView(typedArray.buffer);
        let result = body.prepend(view);
        assert(result, undefined, `body.prepend(typedArray)`);
      }
    });
  }
  // concat(dest: FastlyBody): void;
  {
    routes.set('/FastlyBody/concat/called-as-constructor', () => {
      assertThrows(() => {
        new FastlyBody.concat(new FastlyBody());
      }, TypeError);
    });
    routes.set('/FastlyBody/concat/dest-parameter-not-supplied', () => {
      assertThrows(
        () => {
          const body = new FastlyBody();
          body.concat();
        },
        TypeError,
        `concat: At least 1 argument required, but only 0 passed`,
      );
    });
    routes.set('/FastlyBody/concat/dest-parameter-wrong-type', () => {
      assertThrows(() => {
        const body = new FastlyBody();
        body.concat('hello');
      });
    });
    routes.set('/FastlyBody/concat/concat-same-fastlybody-twice', () => {
      assertThrows(() => {
        const body = new FastlyBody();
        const body2 = new FastlyBody();
        body.concat(body2);
        body.concat(body2);
      });
    });
    routes.set('/FastlyBody/concat/happy-path', () => {
      assertDoesNotThrow(() => {
        const body = new FastlyBody();
        body.concat(new FastlyBody());
      });
    });
  }
  // read(chunkSize: number): ArrayBuffer;
  {
    routes.set('/FastlyBody/read/called-as-constructor', () => {
      assertThrows(() => {
        new FastlyBody.read(1);
      }, TypeError);
    });
    routes.set('/FastlyBody/read/chunkSize-parameter-not-supplied', () => {
      assertThrows(
        () => {
          const body = new FastlyBody();
          body.read();
        },
        TypeError,
        `read: At least 1 argument required, but only 0 passed`,
      );
    });
    routes.set('/FastlyBody/read/chunkSize-parameter-wrong-type', () => {
      assertThrows(() => {
        const body = new FastlyBody();
        body.read(Symbol());
      });
    });
    // negative
    routes.set('/FastlyBody/read/chunkSize-parameter-negative', () => {
      assertThrows(() => {
        const body = new FastlyBody();
        body.append('hello world');
        body.read(-1);
      });
    });
    // infinity
    routes.set('/FastlyBody/read/chunkSize-parameter-infinity', () => {
      assertThrows(() => {
        const body = new FastlyBody();
        body.append('hello world');
        body.read(Infinity);
      });
    });
    // NaN
    routes.set('/FastlyBody/read/chunkSize-parameter-NaN', () => {
      assertThrows(() => {
        const body = new FastlyBody();
        body.append('hello world');
        body.read(NaN);
      });
    });
    routes.set('/FastlyBody/read/happy-path', () => {
      const body = new FastlyBody();
      body.append('world');
      body.prepend('hello ');
      const body2 = new FastlyBody();
      body2.append('!');
      body.concat(body2);
      const decoder = new TextDecoder();
      let result = decoder.decode(body.read(1));
      assert(result, 'h', `body.read(1)`);
      result = decoder.decode(body.read(1));
      assert(result, 'e', `body.read(1)`);
    });
  }
  // close(): void;
  {
    routes.set('/FastlyBody/close/called-as-constructor', () => {
      assertThrows(() => {
        new CoreCache.lookup('1');
      }, TypeError);
    });
    routes.set('/FastlyBody/close/called-once', () => {
      assertThrows(
        () => {
          CoreCache.lookup();
        },
        TypeError,
        `CoreCache.lookup: At least 1 argument required, but only 0 passed`,
      );
    });
    routes.set('/FastlyBody/close/called-twice', () => {
      assertThrows(() => {
        CoreCache.lookup('cat', { headers: '' });
      });
    });
  }
}

// CoreCache
{
  routes.set('/core-cache/interface', () => {
    let actual = Reflect.ownKeys(CoreCache);
    let expected = [
      'prototype',
      'lookup',
      'insert',
      'transactionLookup',
      'length',
      'name',
    ];
    assert(actual, expected, `Reflect.ownKeys(CoreCache)`);

    // Check the prototype descriptors are correct
    {
      actual = Reflect.getOwnPropertyDescriptor(CoreCache, 'prototype');
      expected = {
        value: CoreCache.prototype,
        writable: false,
        enumerable: false,
        configurable: false,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(CoreCache, 'prototype')`,
      );
    }

    // Check the constructor function's defined parameter length is correct
    {
      actual = Reflect.getOwnPropertyDescriptor(CoreCache, 'length');
      expected = {
        value: 0,
        writable: false,
        enumerable: false,
        configurable: true,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(CoreCache, 'length')`,
      );
    }

    // Check the constructor function's name is correct
    {
      actual = Reflect.getOwnPropertyDescriptor(CoreCache, 'name');
      expected = {
        value: 'CoreCache',
        writable: false,
        enumerable: false,
        configurable: true,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(CoreCache, 'name')`,
      );
    }

    // Check the prototype has the correct keys
    {
      actual = Reflect.ownKeys(CoreCache.prototype);
      expected = ['constructor', Symbol.toStringTag];
      assert(actual, expected, `Reflect.ownKeys(CoreCache.prototype)`);
    }

    // Check the constructor on the prototype is correct
    {
      actual = Reflect.getOwnPropertyDescriptor(
        CoreCache.prototype,
        'constructor',
      );
      expected = {
        writable: true,
        enumerable: false,
        configurable: true,
        value: CoreCache.prototype.constructor,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(CoreCache.prototype, 'constructor')`,
      );

      assert(
        typeof CoreCache.prototype.constructor,
        'function',
        `typeof CoreCache.prototype.constructor`,
      );

      actual = Reflect.getOwnPropertyDescriptor(
        CoreCache.prototype.constructor,
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
        `Reflect.getOwnPropertyDescriptor(CoreCache.prototype.constructor, 'length')`,
      );

      actual = Reflect.getOwnPropertyDescriptor(
        CoreCache.prototype.constructor,
        'name',
      );
      expected = {
        value: 'CoreCache',
        writable: false,
        enumerable: false,
        configurable: true,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(CoreCache.prototype.constructor, 'name')`,
      );
    }

    // Check the Symbol.toStringTag on the prototype is correct
    {
      actual = Reflect.getOwnPropertyDescriptor(
        CoreCache.prototype,
        Symbol.toStringTag,
      );
      expected = {
        writable: false,
        enumerable: false,
        configurable: true,
        value: 'CoreCache',
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(CoreCache.prototype, [Symbol.toStringTag])`,
      );

      assert(
        typeof CoreCache.prototype[Symbol.toStringTag],
        'string',
        `typeof CoreCache.prototype[Symbol.toStringTag]`,
      );
    }

    // Check the lookup static method has correct descriptors, length and name
    {
      actual = Reflect.getOwnPropertyDescriptor(CoreCache, 'lookup');
      expected = {
        writable: true,
        enumerable: true,
        configurable: true,
        value: CoreCache.lookup,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(CoreCache, 'lookup')`,
      );

      assert(typeof CoreCache.lookup, 'function', `typeof CoreCache.lookup`);

      actual = Reflect.getOwnPropertyDescriptor(CoreCache.lookup, 'length');
      expected = {
        value: 1,
        writable: false,
        enumerable: false,
        configurable: true,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(CoreCache.lookup, 'length')`,
      );

      actual = Reflect.getOwnPropertyDescriptor(CoreCache.lookup, 'name');
      expected = {
        value: 'lookup',
        writable: false,
        enumerable: false,
        configurable: true,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(CoreCache.lookup, 'name')`,
      );
    }

    // Check the insert static method has correct descriptors, length and name
    {
      actual = Reflect.getOwnPropertyDescriptor(CoreCache, 'insert');
      expected = {
        writable: true,
        enumerable: true,
        configurable: true,
        value: CoreCache.insert,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(CoreCache, 'insert')`,
      );

      assert(typeof CoreCache.insert, 'function', `typeof CoreCache.insert`);

      actual = Reflect.getOwnPropertyDescriptor(CoreCache.insert, 'length');
      expected = {
        value: 2,
        writable: false,
        enumerable: false,
        configurable: true,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(CoreCache.insert, 'length')`,
      );

      actual = Reflect.getOwnPropertyDescriptor(CoreCache.insert, 'name');
      expected = {
        value: 'insert',
        writable: false,
        enumerable: false,
        configurable: true,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(CoreCache.insert, 'name')`,
      );
    }

    // Check the transactionLookup static method has correct descriptors, length and name
    {
      actual = Reflect.getOwnPropertyDescriptor(CoreCache, 'transactionLookup');
      expected = {
        writable: true,
        enumerable: true,
        configurable: true,
        value: CoreCache.transactionLookup,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(CoreCache, 'transactionLookup')`,
      );

      assert(
        typeof CoreCache.transactionLookup,
        'function',
        `typeof CoreCache.transactionLookup`,
      );

      actual = Reflect.getOwnPropertyDescriptor(
        CoreCache.transactionLookup,
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
        `Reflect.getOwnPropertyDescriptor(CoreCache.transactionLookup, 'length')`,
      );

      actual = Reflect.getOwnPropertyDescriptor(
        CoreCache.transactionLookup,
        'name',
      );
      expected = {
        value: 'transactionLookup',
        writable: false,
        enumerable: false,
        configurable: true,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(CoreCache.transactionLookup, 'name')`,
      );
    }
  });

  // CoreCache constructor
  {
    routes.set('/core-cache/constructor/called-as-regular-function', () => {
      assertThrows(() => {
        CoreCache();
      }, TypeError);
    });
    routes.set('/core-cache/constructor/throws', () => {
      assertThrows(() => new CoreCache(), TypeError);
    });
  }

  // CoreCache lookup static method
  // static lookup(key: string): CoreCacheEntry | null;
  {
    routes.set('/core-cache/lookup/called-as-constructor', () => {
      assertThrows(() => {
        new CoreCache.lookup('1');
      }, TypeError);
    });
    // https://tc39.es/ecma262/#sec-tostring
    routes.set('/core-cache/lookup/key-parameter-calls-7.1.17-ToString', () => {
      let sentinel;
      const test = () => {
        sentinel = Symbol('sentinel');
        const key = {
          toString() {
            throw sentinel;
          },
        };
        CoreCache.lookup(key);
      };
      assertThrows(test);
      try {
        test();
      } catch (thrownError) {
        assert(thrownError, sentinel, 'thrownError === sentinel');
      }
      assertThrows(
        () => {
          CoreCache.lookup(Symbol());
        },
        TypeError,
        `can't convert symbol to string`,
      );
    });
    routes.set('/core-cache/lookup/key-parameter-not-supplied', () => {
      assertThrows(
        () => {
          CoreCache.lookup();
        },
        TypeError,
        `CoreCache.lookup: At least 1 argument required, but only 0 passed`,
      );
    });
    routes.set('/core-cache/lookup/key-parameter-empty-string', () => {
      assertThrows(
        () => {
          CoreCache.lookup('');
        },
        Error,
        `CoreCache.lookup: key can not be an empty string`,
      );
    });
    routes.set('/core-cache/lookup/key-parameter-8135-character-string', () => {
      assertDoesNotThrow(() => {
        const key = 'a'.repeat(8135);
        CoreCache.lookup(key);
      });
    });
    routes.set('/core-cache/lookup/key-parameter-8136-character-string', () => {
      assertThrows(
        () => {
          const key = 'a'.repeat(8136);
          CoreCache.lookup(key);
        },
        Error,
        `CoreCache.lookup: key is too long, the maximum allowed length is 8135.`,
      );
    });
    routes.set('/core-cache/lookup/key-does-not-exist-returns-null', () => {
      let result = CoreCache.lookup(Math.random());
      assert(result, null, `CoreCache.lookup(Math.random()) === null`);
    });
    routes.set('/core-cache/lookup/key-exists', () => {
      let key = 'c'.repeat(8135);
      let writer = CoreCache.insert(key, {
        maxAge: 1000,
      });
      writer.append('meow');
      writer.close();
      let result = CoreCache.lookup(key);
      assert(
        result instanceof CacheEntry,
        true,
        `CoreCache.lookup('cat') instanceof CacheEntry`,
      );
    });

    routes.set('/core-cache/lookup/options-parameter-none', () => {
      ensureLion();
      let entry;
      assertDoesNotThrow(() => {
        entry = CoreCache.lookup('lion');
      });
      assert(
        entry instanceof CacheEntry,
        true,
        `CoreCache.lookup('lion', {headers:...}) instanceof CacheEntry`,
      );
    });
    routes.set('/core-cache/lookup/options-parameter-wrong-type', () => {
      ensureLion();
      assertThrows(() => {
        CoreCache.lookup('lion', '');
      });
    });
    routes.set(
      '/core-cache/lookup/options-parameter-headers-field-wrong-type',
      () => {
        ensureLion();
        assertThrows(() => {
          CoreCache.lookup('lion', { headers: '' });
        });
      },
    );
    routes.set(
      '/core-cache/lookup/options-parameter-headers-field-undefined',
      () => {
        ensureLion();
        let entry;
        assertDoesNotThrow(() => {
          entry = CoreCache.lookup('lion', { headers: undefined });
        });
        assert(
          entry instanceof CacheEntry,
          true,
          `CoreCache.lookup('lion', {headers:...}) instanceof CacheEntry`,
        );
      },
    );
    routes.set(
      '/core-cache/lookup/options-parameter-headers-field-valid-sequence',
      () => {
        ensureLion();
        let entry;
        assertDoesNotThrow(() => {
          entry = CoreCache.lookup('lion', {
            headers: [
              ['user-agent', 'Aki 1.0'],
              ['Accept-Encoding', 'br'],
            ],
          });
        });
        assert(
          entry instanceof CacheEntry,
          true,
          `CoreCache.lookup('lion', {headers:...}) instanceof CacheEntry`,
        );
      },
    );
    routes.set(
      '/core-cache/lookup/options-parameter-headers-field-valid-record',
      () => {
        ensureLion();
        let entry;
        assertDoesNotThrow(() => {
          entry = CoreCache.lookup('lion', {
            headers: {
              'user-agent': 'Aki 1.0',
              'Accept-Encoding': 'br',
            },
          });
        });
        assert(
          entry instanceof CacheEntry,
          true,
          `CoreCache.lookup('lion', {headers:...}) instanceof CacheEntry`,
        );
      },
    );
    routes.set(
      '/core-cache/lookup/options-parameter-headers-field-valid-Headers-instance',
      () => {
        ensureLion();
        let entry;
        assertDoesNotThrow(() => {
          entry = CoreCache.lookup('lion', {
            headers: new Headers({
              'user-agent': 'Aki 1.0',
              'Accept-Encoding': 'br',
            }),
          });
        });
        assert(
          entry instanceof CacheEntry,
          true,
          `CoreCache.lookup('lion', {headers:...}) instanceof CacheEntry`,
        );
      },
    );
  }

  // static insert(key: string, options: InsertOptions): FastlyBody;
  {
    routes.set('/core-cache/insert/called-as-constructor', () => {
      assertThrows(() => {
        new CoreCache.insert('1', { maxAge: 1 });
      }, TypeError);
    });
    // https://tc39.es/ecma262/#sec-tostring
    routes.set('/core-cache/insert/key-parameter-calls-7.1.17-ToString', () => {
      let sentinel;
      const test = () => {
        sentinel = Symbol('sentinel');
        const key = {
          toString() {
            throw sentinel;
          },
        };
        CoreCache.insert(key, { maxAge: 1 });
      };
      assertThrows(test);
      try {
        test();
      } catch (thrownError) {
        assert(thrownError, sentinel, 'thrownError === sentinel');
      }
      assertThrows(
        () => {
          CoreCache.insert(Symbol(), { maxAge: 1 });
        },
        TypeError,
        `can't convert symbol to string`,
      );
    });
    routes.set('/core-cache/insert/key-parameter-not-supplied', () => {
      assertThrows(
        () => {
          CoreCache.insert();
        },
        TypeError,
        `CoreCache.insert: At least 2 arguments required, but only 0 passed`,
      );
    });
    routes.set('/core-cache/insert/key-parameter-empty-string', () => {
      assertThrows(
        () => {
          CoreCache.insert('', { maxAge: 1 });
        },
        Error,
        `CoreCache.insert: key can not be an empty string`,
      );
    });
    routes.set('/core-cache/insert/key-parameter-8135-character-string', () => {
      assertDoesNotThrow(() => {
        const key = 'a'.repeat(8135);
        CoreCache.insert(key, { maxAge: 1 });
      });
    });
    routes.set('/core-cache/insert/key-parameter-8136-character-string', () => {
      assertThrows(
        () => {
          const key = 'a'.repeat(8136);
          CoreCache.insert(key, { maxAge: 1 });
        },
        Error,
        `CoreCache.insert: key is too long, the maximum allowed length is 8135.`,
      );
    });
    routes.set('/core-cache/insert/options-parameter-wrong-type', () => {
      assertThrows(() => {
        CoreCache.insert('cat', '');
      });
    });
    routes.set(
      '/core-cache/insert/options-parameter-headers-field-wrong-type',
      () => {
        assertThrows(() => {
          CoreCache.insert('cat', { headers: '', maxAge: 1 });
        });
      },
    );
    routes.set(
      '/core-cache/insert/options-parameter-headers-field-undefined',
      () => {
        let body;
        assertDoesNotThrow(() => {
          body = CoreCache.insert('cat', { headers: undefined, maxAge: 1 });
        });
        assert(
          body instanceof FastlyBody,
          true,
          `CoreCache.insert('cat', {headers:undefined}) instanceof FastlyBody`,
        );
      },
    );
    routes.set(
      '/core-cache/insert/options-parameter-headers-field-valid-sequence',
      () => {
        let body;
        assertDoesNotThrow(() => {
          body = CoreCache.insert('cat', {
            headers: [
              ['user-agent', 'Aki 1.0'],
              ['Accept-Encoding', 'br'],
            ],
            maxAge: 1,
          });
        });
        assert(
          body instanceof FastlyBody,
          true,
          `CoreCache.insert('cat', {headers:undefined}) instanceof FastlyBody`,
        );
      },
    );
    routes.set(
      '/core-cache/insert/options-parameter-headers-field-valid-record',
      () => {
        let body;
        assertDoesNotThrow(() => {
          body = CoreCache.insert('cat', {
            headers: {
              'user-agent': 'Aki 1.0',
              'Accept-Encoding': 'br',
            },
            maxAge: 1,
          });
        });
        assert(
          body instanceof FastlyBody,
          true,
          `CoreCache.insert('cat', {headers:undefined}) instanceof FastlyBody`,
        );
      },
    );
    routes.set(
      '/core-cache/insert/options-parameter-headers-field-valid-Headers-instance',
      () => {
        let body;
        assertDoesNotThrow(() => {
          body = CoreCache.insert('cat', {
            headers: new Headers({
              'user-agent': 'Aki 1.0',
              'Accept-Encoding': 'br',
            }),
            maxAge: 1,
          });
        });
        assert(
          body instanceof FastlyBody,
          true,
          `CoreCache.insert('cat', {headers:undefined}) instanceof FastlyBody`,
        );
      },
    );
    routes.set(
      '/core-cache/insert/options-parameter-maxAge-field-valid-record',
      () => {
        let body;
        assertDoesNotThrow(() => {
          body = CoreCache.insert('cat', { maxAge: 1 });
        });
        assert(
          body instanceof FastlyBody,
          true,
          `CoreCache.insert('cat', {maxAge: 1}) instanceof FastlyBody`,
        );
      },
    );
    routes.set('/core-cache/insert/options-parameter-maxAge-field-NaN', () => {
      assertThrows(() => {
        CoreCache.insert('cat', {
          maxAge: NaN,
        });
      });
    });
    routes.set(
      '/core-cache/insert/options-parameter-maxAge-field-postitive-infinity',
      () => {
        assertThrows(() => {
          CoreCache.insert('cat', {
            maxAge: Number.POSITIVE_INFINITY,
          });
        });
      },
    );
    routes.set(
      '/core-cache/insert/options-parameter-maxAge-field-negative-infinity',
      () => {
        assertThrows(() => {
          CoreCache.insert('cat', {
            maxAge: Number.NEGATIVE_INFINITY,
          });
        });
      },
    );
    routes.set(
      '/core-cache/insert/options-parameter-maxAge-field-negative-number',
      () => {
        assertThrows(() => {
          CoreCache.insert('cat', {
            maxAge: -1,
          });
        });
      },
    );
    routes.set(
      '/core-cache/insert/options-parameter-initialAge-field-valid-record',
      () => {
        let body;
        assertDoesNotThrow(() => {
          body = CoreCache.insert('cat', { maxAge: 1, initialAge: 1 });
        });
        assert(
          body instanceof FastlyBody,
          true,
          `CoreCache.insert('cat', {maxAge: 1,initialAge: 1}) instanceof FastlyBody`,
        );
      },
    );
    routes.set(
      '/core-cache/insert/options-parameter-initialAge-field-NaN',
      () => {
        assertThrows(() => {
          CoreCache.insert('cat', {
            maxAge: 1,
            initialAge: NaN,
          });
        });
      },
    );
    routes.set(
      '/core-cache/insert/options-parameter-initialAge-field-postitive-infinity',
      () => {
        assertThrows(() => {
          CoreCache.insert('cat', {
            maxAge: 1,
            initialAge: Number.POSITIVE_INFINITY,
          });
        });
      },
    );
    routes.set(
      '/core-cache/insert/options-parameter-initialAge-field-negative-infinity',
      () => {
        assertThrows(() => {
          CoreCache.insert('cat', {
            maxAge: 1,
            initialAge: Number.NEGATIVE_INFINITY,
          });
        });
      },
    );
    routes.set(
      '/core-cache/insert/options-parameter-initialAge-field-negative-number',
      () => {
        assertThrows(() => {
          CoreCache.insert('cat', {
            maxAge: 1,
            initialAge: -1,
          });
        });
      },
    );
    routes.set(
      '/core-cache/insert/options-parameter-staleWhileRevalidate-field-valid-record',
      () => {
        let body;
        assertDoesNotThrow(() => {
          body = CoreCache.insert('cat', {
            maxAge: 1,
            staleWhileRevalidate: 1,
          });
        });
        assert(
          body instanceof FastlyBody,
          true,
          `CoreCache.insert('cat', {maxAge: 1,staleWhileRevalidate: 1}) instanceof FastlyBody`,
        );
      },
    );
    routes.set(
      '/core-cache/insert/options-parameter-staleWhileRevalidate-field-NaN',
      () => {
        assertThrows(() => {
          CoreCache.insert('cat', {
            maxAge: 1,
            staleWhileRevalidate: NaN,
          });
        });
      },
    );
    routes.set(
      '/core-cache/insert/options-parameter-staleWhileRevalidate-field-postitive-infinity',
      () => {
        assertThrows(() => {
          CoreCache.insert('cat', {
            maxAge: 1,
            staleWhileRevalidate: Number.POSITIVE_INFINITY,
          });
        });
      },
    );
    routes.set(
      '/core-cache/insert/options-parameter-staleWhileRevalidate-field-negative-infinity',
      () => {
        assertThrows(() => {
          CoreCache.insert('cat', {
            maxAge: 1,
            staleWhileRevalidate: Number.NEGATIVE_INFINITY,
          });
        });
      },
    );
    routes.set(
      '/core-cache/insert/options-parameter-staleWhileRevalidate-field-negative-number',
      () => {
        assertThrows(() => {
          CoreCache.insert('cat', {
            maxAge: 1,
            staleWhileRevalidate: -1,
          });
        });
      },
    );
    routes.set(
      '/core-cache/insert/options-parameter-length-field-valid-record',
      () => {
        let body;
        assertDoesNotThrow(() => {
          body = CoreCache.insert('cat', { maxAge: 1, length: 1 });
        });
        assert(
          body instanceof FastlyBody,
          true,
          `CoreCache.insert('cat', {maxAge: 1,length: 1}) instanceof FastlyBody`,
        );
      },
    );
    routes.set('/core-cache/insert/options-parameter-length-field-NaN', () => {
      assertThrows(() => {
        CoreCache.insert('cat', {
          maxAge: 1,
          length: NaN,
        });
      });
    });
    routes.set(
      '/core-cache/insert/options-parameter-length-field-postitive-infinity',
      () => {
        assertThrows(() => {
          CoreCache.insert('cat', {
            maxAge: 1,
            length: Number.POSITIVE_INFINITY,
          });
        });
      },
    );
    routes.set(
      '/core-cache/insert/options-parameter-length-field-negative-infinity',
      () => {
        assertThrows(() => {
          CoreCache.insert('cat', {
            maxAge: 1,
            length: Number.NEGATIVE_INFINITY,
          });
        });
      },
    );
    routes.set(
      '/core-cache/insert/options-parameter-length-field-negative-number',
      () => {
        assertThrows(() => {
          CoreCache.insert('cat', {
            maxAge: 1,
            length: -1,
          });
        });
      },
    );
    routes.set('/core-cache/insert/options-parameter-sensitive-field', () => {
      assertDoesNotThrow(() => {
        CoreCache.insert('cat', {
          maxAge: 1,
          sensitive: true,
        });
      });
    });
    routes.set('/core-cache/insert/options-parameter-vary-field', () => {
      assertDoesNotThrow(() => {
        CoreCache.insert('cat', {
          maxAge: 1,
          vary: ['animal', 'mineral', 'vegetable'],
        });
      });
    });
    routes.set(
      '/core-cache/insert/options-parameter-userMetadata-field/arraybuffer/empty',
      async () => {
        await assertResolves(async () => {
          let key =
            '/core-cache/insert/options-parameter-userMetadata-field/arraybuffer/empty' +
            Math.random();
          let writer = CoreCache.insert(key, {
            maxAge: 60 * 1000,
            userMetadata: new ArrayBuffer(0),
          });
          writer.append('hello');
          writer.close();
        });
      },
    );
    routes.set(
      '/core-cache/insert/options-parameter-userMetadata-field/arraybuffer/not-empty',
      async () => {
        await assertResolves(async () => {
          let key =
            '/core-cache/insert/options-parameter-userMetadata-field/arraybuffer/not-empty' +
            Math.random();
          let writer = CoreCache.insert(key, {
            maxAge: 60 * 1000,
            userMetadata: Uint8Array.from([104, 101, 108, 108, 111]).buffer,
          });
          writer.append('hello');
          writer.close();
        });
      },
    );
    routes.set(
      '/core-cache/insert/options-parameter-userMetadata-field/URLSearchParams',
      async () => {
        await assertResolves(async () => {
          let key =
            '/core-cache/insert/options-parameter-userMetadata-field/URLSearchParams' +
            Math.random();
          let userMetadata = new URLSearchParams();
          userMetadata.set('hello', 'world');
          let writer = CoreCache.insert(key, {
            maxAge: 60 * 1000,
            userMetadata,
          });
          writer.append('hello');
          writer.close();
        });
      },
    );
    routes.set(
      '/core-cache/insert/options-parameter-userMetadata-field/string',
      async () => {
        await assertResolves(async () => {
          let key =
            '/core-cache/insert/options-parameter-userMetadata-field/string' +
            Math.random();
          let writer = CoreCache.insert(key, {
            maxAge: 60 * 1000,
            userMetadata: 'hello',
          });
          writer.append('hello');
          writer.close();
        });
      },
    );
    // surrogateKeys?: Array<string>,-- empty string? -- toString which throws -- wrong types?
  }

  //static transactionLookup(key: string, options?: LookupOptions): CacheEntry | null;
  {
    routes.set('/core-cache/transactionLookup/called-as-constructor', () => {
      assertThrows(() => {
        new CoreCache.transactionLookup('1');
      }, TypeError);
    });
    // https://tc39.es/ecma262/#sec-tostring
    routes.set(
      '/core-cache/transactionLookup/key-parameter-calls-7.1.17-ToString',
      () => {
        let sentinel;
        const test = () => {
          sentinel = Symbol('sentinel');
          const key = {
            toString() {
              throw sentinel;
            },
          };
          CoreCache.transactionLookup(key);
        };
        assertThrows(test);
        try {
          test();
        } catch (thrownError) {
          assert(thrownError, sentinel, 'thrownError === sentinel');
        }
        assertThrows(
          () => {
            CoreCache.transactionLookup(Symbol());
          },
          TypeError,
          `can't convert symbol to string`,
        );
      },
    );
    routes.set(
      '/core-cache/transactionLookup/key-parameter-not-supplied',
      () => {
        assertThrows(
          () => {
            CoreCache.transactionLookup();
          },
          TypeError,
          `CoreCache.transactionLookup: At least 1 argument required, but only 0 passed`,
        );
      },
    );
    routes.set(
      '/core-cache/transactionLookup/key-parameter-empty-string',
      () => {
        assertThrows(
          () => {
            CoreCache.transactionLookup('');
          },
          Error,
          `CoreCache.transactionLookup: key can not be an empty string`,
        );
      },
    );
    routes.set(
      '/core-cache/transactionLookup/key-parameter-8135-character-string',
      () => {
        assertDoesNotThrow(() => {
          const key = 'a'.repeat(8135);
          CoreCache.transactionLookup(key);
        });
      },
    );
    routes.set(
      '/core-cache/transactionLookup/key-parameter-8136-character-string',
      () => {
        assertThrows(
          () => {
            const key = 'a'.repeat(8136);
            CoreCache.transactionLookup(key);
          },
          Error,
          `CoreCache.transactionLookup: key is too long, the maximum allowed length is 8135.`,
        );
      },
    );
    routes.set('/core-cache/transactionLookup/key-does-not-exist', () => {
      let result = CoreCache.transactionLookup(Math.random());
      assert(
        result instanceof CacheEntry,
        true,
        `CoreCache.transactionLookup(Math.random()) instanceof CacheEntry`,
      );
    });
    routes.set('/core-cache/transactionLookup/key-exists', () => {
      let writer = CoreCache.insert('cat', {
        maxAge: 10_000,
      });
      writer.append('meow');
      writer.close();
      let result = CoreCache.transactionLookup('cat');
      assert(
        result instanceof CacheEntry,
        true,
        `CoreCache.transactionLookup('cat') instanceof CacheEntry`,
      );
    });

    routes.set(
      '/core-cache/transactionLookup/options-parameter-wrong-type',
      () => {
        assertThrows(() => {
          CoreCache.transactionLookup('cat', '');
        });
      },
    );
    routes.set(
      '/core-cache/transactionLookup/options-parameter-headers-field-wrong-type',
      () => {
        assertThrows(() => {
          CoreCache.transactionLookup('cat', { headers: '' });
        });
      },
    );
    routes.set(
      '/core-cache/transactionLookup/options-parameter-headers-field-undefined',
      () => {
        let entry;
        assertDoesNotThrow(() => {
          entry = CoreCache.transactionLookup('cat', { headers: undefined });
        });
        assert(
          entry instanceof CacheEntry,
          true,
          `CoreCache.transactionLookup('cat', {headers:...}) instanceof CacheEntry`,
        );
      },
    );
    routes.set(
      '/core-cache/transactionLookup/options-parameter-headers-field-valid-sequence',
      () => {
        let entry;
        assertDoesNotThrow(() => {
          entry = CoreCache.transactionLookup('cat', {
            headers: [
              ['user-agent', 'Aki 1.0'],
              ['Accept-Encoding', 'br'],
            ],
          });
        });
        assert(
          entry instanceof CacheEntry,
          true,
          `CoreCache.transactionLookup('cat', {headers:...}) instanceof CacheEntry`,
        );
      },
    );
    routes.set(
      '/core-cache/transactionLookup/options-parameter-headers-field-valid-record',
      () => {
        let entry;
        assertDoesNotThrow(() => {
          entry = CoreCache.transactionLookup('cat', {
            headers: {
              'user-agent': 'Aki 1.0',
              'Accept-Encoding': 'br',
            },
          });
        });
        assert(
          entry instanceof CacheEntry,
          true,
          `CoreCache.transactionLookup('cat', {headers:...}) instanceof CacheEntry`,
        );
      },
    );
    routes.set(
      '/core-cache/transactionLookup/options-parameter-headers-field-valid-Headers-instance',
      () => {
        let entry;
        assertDoesNotThrow(() => {
          entry = CoreCache.transactionLookup('cat', {
            headers: new Headers({
              'user-agent': 'Aki 1.0',
              'Accept-Encoding': 'br',
            }),
          });
        });
        assert(
          entry instanceof CacheEntry,
          true,
          `CoreCache.transactionLookup('cat', {headers:...}) instanceof CacheEntry`,
        );
      },
    );
  }
}

// CacheEntry
{
  routes.set('/cache-entry/interface', () => {
    let actual = Reflect.ownKeys(CacheEntry);
    let expected = ['prototype', 'length', 'name'];
    assert(actual, expected, `Reflect.ownKeys(CacheEntry)`);

    // Check the prototype descriptors are correct
    {
      actual = Reflect.getOwnPropertyDescriptor(CacheEntry, 'prototype');
      expected = {
        value: CacheEntry.prototype,
        writable: false,
        enumerable: false,
        configurable: false,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(CacheEntry, 'prototype')`,
      );
    }

    // Check the constructor function's defined parameter length is correct
    {
      actual = Reflect.getOwnPropertyDescriptor(CacheEntry, 'length');
      expected = {
        value: 0,
        writable: false,
        enumerable: false,
        configurable: true,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(CacheEntry, 'length')`,
      );
    }

    // Check the constructor function's name is correct
    {
      actual = Reflect.getOwnPropertyDescriptor(CacheEntry, 'name');
      expected = {
        value: 'CacheEntry',
        writable: false,
        enumerable: false,
        configurable: true,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(CacheEntry, 'name')`,
      );
    }

    // Check the prototype has the correct keys
    {
      actual = Reflect.ownKeys(CacheEntry.prototype);
      expected = [
        'constructor',
        'close',
        'state',
        'userMetadata',
        'body',
        'length',
        'maxAge',
        'staleWhileRevalidate',
        'age',
        'hits',
        Symbol.toStringTag,
      ];
      assert(actual, expected, `Reflect.ownKeys(CacheEntry.prototype)`);
    }

    // Check the constructor on the prototype is correct
    {
      actual = Reflect.getOwnPropertyDescriptor(
        CacheEntry.prototype,
        'constructor',
      );
      expected = {
        writable: true,
        enumerable: false,
        configurable: true,
        value: CacheEntry.prototype.constructor,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(CacheEntry.prototype, 'constructor')`,
      );

      assert(
        typeof CacheEntry.prototype.constructor,
        'function',
        `typeof CacheEntry.prototype.constructor`,
      );

      actual = Reflect.getOwnPropertyDescriptor(
        CacheEntry.prototype.constructor,
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
        `Reflect.getOwnPropertyDescriptor(CacheEntry.prototype.constructor, 'length')`,
      );

      actual = Reflect.getOwnPropertyDescriptor(
        CacheEntry.prototype.constructor,
        'name',
      );
      expected = {
        value: 'CacheEntry',
        writable: false,
        enumerable: false,
        configurable: true,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(CacheEntry.prototype.constructor, 'name')`,
      );
    }

    // Check the Symbol.toStringTag on the prototype is correct
    {
      actual = Reflect.getOwnPropertyDescriptor(
        CacheEntry.prototype,
        Symbol.toStringTag,
      );
      expected = {
        writable: false,
        enumerable: false,
        configurable: true,
        value: 'CacheEntry',
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(CacheEntry.prototype, [Symbol.toStringTag])`,
      );

      assert(
        typeof CacheEntry.prototype[Symbol.toStringTag],
        'string',
        `typeof CacheEntry.prototype[Symbol.toStringTag]`,
      );
    }

    // Check the close method has correct descriptors, length and name
    {
      actual = Reflect.getOwnPropertyDescriptor(CacheEntry.prototype, 'close');
      expected = {
        writable: true,
        enumerable: true,
        configurable: true,
        value: CacheEntry.prototype.close,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(CacheEntry.prototype, 'close')`,
      );

      assert(
        typeof CacheEntry.prototype.close,
        'function',
        `typeof CacheEntry.prototype.close`,
      );

      actual = Reflect.getOwnPropertyDescriptor(
        CacheEntry.prototype.close,
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
        `Reflect.getOwnPropertyDescriptor(CacheEntry.prototype.close, 'length')`,
      );

      actual = Reflect.getOwnPropertyDescriptor(
        CacheEntry.prototype.close,
        'name',
      );
      expected = {
        value: 'close',
        writable: false,
        enumerable: false,
        configurable: true,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(CacheEntry.prototype.close, 'name')`,
      );
    }

    // Check the state method has correct descriptors, length and name
    {
      actual = Reflect.getOwnPropertyDescriptor(CacheEntry.prototype, 'state');
      expected = {
        writable: true,
        enumerable: true,
        configurable: true,
        value: CacheEntry.prototype.state,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(CacheEntry.prototype, 'state')`,
      );

      assert(
        typeof CacheEntry.prototype.state,
        'function',
        `typeof CacheEntry.prototype.state`,
      );

      actual = Reflect.getOwnPropertyDescriptor(
        CacheEntry.prototype.state,
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
        `Reflect.getOwnPropertyDescriptor(CacheEntry.prototype.state, 'length')`,
      );

      actual = Reflect.getOwnPropertyDescriptor(
        CacheEntry.prototype.state,
        'name',
      );
      expected = {
        value: 'state',
        writable: false,
        enumerable: false,
        configurable: true,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(CacheEntry.prototype.state, 'name')`,
      );
    }

    // Check the userMetadata method has correct descriptors, length and name
    {
      actual = Reflect.getOwnPropertyDescriptor(
        CacheEntry.prototype,
        'userMetadata',
      );
      expected = {
        writable: true,
        enumerable: true,
        configurable: true,
        value: CacheEntry.prototype.userMetadata,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(CacheEntry.prototype, 'userMetadata')`,
      );

      assert(
        typeof CacheEntry.prototype.userMetadata,
        'function',
        `typeof CacheEntry.prototype.userMetadata`,
      );

      actual = Reflect.getOwnPropertyDescriptor(
        CacheEntry.prototype.userMetadata,
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
        `Reflect.getOwnPropertyDescriptor(CacheEntry.prototype.userMetadata, 'length')`,
      );

      actual = Reflect.getOwnPropertyDescriptor(
        CacheEntry.prototype.userMetadata,
        'name',
      );
      expected = {
        value: 'userMetadata',
        writable: false,
        enumerable: false,
        configurable: true,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(CacheEntry.prototype.userMetadata, 'name')`,
      );
    }

    // Check the body method has correct descriptors, length and name
    {
      actual = Reflect.getOwnPropertyDescriptor(CacheEntry.prototype, 'body');
      expected = {
        writable: true,
        enumerable: true,
        configurable: true,
        value: CacheEntry.prototype.body,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(CacheEntry.prototype, 'body')`,
      );

      assert(
        typeof CacheEntry.prototype.body,
        'function',
        `typeof CacheEntry.prototype.body`,
      );

      actual = Reflect.getOwnPropertyDescriptor(
        CacheEntry.prototype.body,
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
        `Reflect.getOwnPropertyDescriptor(CacheEntry.prototype.body, 'length')`,
      );

      actual = Reflect.getOwnPropertyDescriptor(
        CacheEntry.prototype.body,
        'name',
      );
      expected = {
        value: 'body',
        writable: false,
        enumerable: false,
        configurable: true,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(CacheEntry.prototype.body, 'name')`,
      );
    }

    // Check the length method has correct descriptors, length and name
    {
      actual = Reflect.getOwnPropertyDescriptor(CacheEntry.prototype, 'length');
      expected = {
        writable: true,
        enumerable: true,
        configurable: true,
        value: CacheEntry.prototype.length,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(CacheEntry.prototype, 'length')`,
      );

      assert(
        typeof CacheEntry.prototype.length,
        'function',
        `typeof CacheEntry.prototype.length`,
      );

      actual = Reflect.getOwnPropertyDescriptor(
        CacheEntry.prototype.length,
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
        `Reflect.getOwnPropertyDescriptor(CacheEntry.prototype.length, 'length')`,
      );

      actual = Reflect.getOwnPropertyDescriptor(
        CacheEntry.prototype.length,
        'name',
      );
      expected = {
        value: 'length',
        writable: false,
        enumerable: false,
        configurable: true,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(CacheEntry.prototype.length, 'name')`,
      );
    }

    // Check the maxAge method has correct descriptors, length and name
    {
      actual = Reflect.getOwnPropertyDescriptor(CacheEntry.prototype, 'maxAge');
      expected = {
        writable: true,
        enumerable: true,
        configurable: true,
        value: CacheEntry.prototype.maxAge,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(CacheEntry.prototype, 'maxAge')`,
      );

      assert(
        typeof CacheEntry.prototype.maxAge,
        'function',
        `typeof CacheEntry.prototype.maxAge`,
      );

      actual = Reflect.getOwnPropertyDescriptor(
        CacheEntry.prototype.maxAge,
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
        `Reflect.getOwnPropertyDescriptor(CacheEntry.prototype.maxAge, 'length')`,
      );

      actual = Reflect.getOwnPropertyDescriptor(
        CacheEntry.prototype.maxAge,
        'name',
      );
      expected = {
        value: 'maxAge',
        writable: false,
        enumerable: false,
        configurable: true,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(CacheEntry.prototype.maxAge, 'name')`,
      );
    }

    // Check the staleWhileRevalidate method has correct descriptors, length and name
    {
      actual = Reflect.getOwnPropertyDescriptor(
        CacheEntry.prototype,
        'staleWhileRevalidate',
      );
      expected = {
        writable: true,
        enumerable: true,
        configurable: true,
        value: CacheEntry.prototype.staleWhileRevalidate,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(CacheEntry.prototype, 'staleWhileRevalidate')`,
      );

      assert(
        typeof CacheEntry.prototype.staleWhileRevalidate,
        'function',
        `typeof CacheEntry.prototype.staleWhileRevalidate`,
      );

      actual = Reflect.getOwnPropertyDescriptor(
        CacheEntry.prototype.staleWhileRevalidate,
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
        `Reflect.getOwnPropertyDescriptor(CacheEntry.prototype.staleWhileRevalidate, 'length')`,
      );

      actual = Reflect.getOwnPropertyDescriptor(
        CacheEntry.prototype.staleWhileRevalidate,
        'name',
      );
      expected = {
        value: 'staleWhileRevalidate',
        writable: false,
        enumerable: false,
        configurable: true,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(CacheEntry.prototype.staleWhileRevalidate, 'name')`,
      );
    }

    // Check the age method has correct descriptors, length and name
    {
      actual = Reflect.getOwnPropertyDescriptor(CacheEntry.prototype, 'age');
      expected = {
        writable: true,
        enumerable: true,
        configurable: true,
        value: CacheEntry.prototype.age,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(CacheEntry.prototype, 'age')`,
      );

      assert(
        typeof CacheEntry.prototype.age,
        'function',
        `typeof CacheEntry.prototype.age`,
      );

      actual = Reflect.getOwnPropertyDescriptor(
        CacheEntry.prototype.age,
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
        `Reflect.getOwnPropertyDescriptor(CacheEntry.prototype.age, 'length')`,
      );

      actual = Reflect.getOwnPropertyDescriptor(
        CacheEntry.prototype.age,
        'name',
      );
      expected = {
        value: 'age',
        writable: false,
        enumerable: false,
        configurable: true,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(CacheEntry.prototype.age, 'name')`,
      );
    }

    // Check the hits method has correct descriptors, length and name
    {
      actual = Reflect.getOwnPropertyDescriptor(CacheEntry.prototype, 'hits');
      expected = {
        writable: true,
        enumerable: true,
        configurable: true,
        value: CacheEntry.prototype.hits,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(CacheEntry.prototype, 'hits')`,
      );

      assert(
        typeof CacheEntry.prototype.hits,
        'function',
        `typeof CacheEntry.prototype.hits`,
      );

      actual = Reflect.getOwnPropertyDescriptor(
        CacheEntry.prototype.hits,
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
        `Reflect.getOwnPropertyDescriptor(CacheEntry.prototype.hits, 'length')`,
      );

      actual = Reflect.getOwnPropertyDescriptor(
        CacheEntry.prototype.hits,
        'name',
      );
      expected = {
        value: 'hits',
        writable: false,
        enumerable: false,
        configurable: true,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(CacheEntry.prototype.hits, 'name')`,
      );
    }
  });

  // CacheEntry constructor
  {
    routes.set('/cache-entry/constructor/called-as-regular-function', () => {
      assertThrows(() => {
        CacheEntry();
      }, TypeError);
    });
    routes.set('/cache-entry/constructor/throws', () => {
      assertThrows(() => new CacheEntry(), TypeError);
    });
  }

  // close(): void;
  {
    routes.set('/cache-entry/close/called-as-constructor', () => {
      assertThrows(() => {
        new CacheEntry.prototype.close();
      }, TypeError);
    });
    routes.set('/cache-entry/close/called-unbound', () => {
      assertThrows(() => {
        CacheEntry.prototype.close.call(undefined);
      }, Error);
    });
    routes.set('/cache-entry/close/called-on-instance', () => {
      let key = '/cache-entry/close/called-on-instance' + Math.random();
      let writer = CoreCache.insert(key, {
        maxAge: 60 * 1000,
      });
      writer.append('hello');
      writer.close();
      let result = CoreCache.lookup(key).close();
      assert(result, undefined, `CoreCache.lookup(key).close()`);
    });
  }

  // state(): CacheState;
  {
    routes.set('/cache-entry/state/called-as-constructor', () => {
      assertThrows(() => {
        new CacheEntry.prototype.state();
      }, TypeError);
    });
    routes.set('/cache-entry/state/called-unbound', () => {
      assertThrows(() => {
        CacheEntry.prototype.state.call(undefined);
      }, Error);
    });

    routes.set('/cache-entry/state/called-on-instance', () => {
      let key = '/cache-entry/state/called-on-instance' + Math.random();
      let writer = CoreCache.insert(key, {
        maxAge: 60 * 1000,
      });
      writer.append('hello');
      writer.close();
      let entry = CoreCache.lookup(key);
      let result = entry.state();
      assert(
        result instanceof CacheState,
        true,
        `CoreCache.lookup(key).state() instanceof CacheState`,
      );
    });
  }

  // userMetadata(): ArrayBuffer;
  {
    routes.set('/cache-entry/userMetadata/called-as-constructor', () => {
      assertThrows(() => {
        new CacheEntry.prototype.userMetadata();
      }, TypeError);
    });
    routes.set('/cache-entry/userMetadata/called-unbound', () => {
      assertThrows(() => {
        CacheEntry.prototype.userMetadata.call(undefined);
      }, Error);
    });
    routes.set('/cache-entry/userMetadata/called-on-instance', () => {
      let key = '/cache-entry/userMetadata/called-on-instance' + Math.random();
      let writer = CoreCache.insert(key, {
        maxAge: 60 * 1000,
      });
      writer.append('hello');
      writer.close();
      let result = CoreCache.lookup(key).userMetadata();
      assert(
        result instanceof ArrayBuffer,
        true,
        `CoreCache.lookup(key).userMetadata() instanceof ArrayBuffer`,
      );
      assert(
        result.byteLength,
        0,
        `CoreCache.lookup(key).userMetadata().byteLength`,
      );
    });
    routes.set('/cache-entry/userMetadata/basic', () => {
      let key = '/cache-entry/userMetadata/basic' + Math.random();
      let writer = CoreCache.insert(key, {
        maxAge: 60 * 1000,
        userMetadata: 'hello world',
      });
      writer.append('hello');
      writer.close();
      let entry = CoreCache.lookup(key);
      assert(
        entry instanceof CacheEntry,
        true,
        'CoreCache.lookup(key) instanceof CacheEntry',
      );
      let metadata = entry.userMetadata();
      assert(
        metadata instanceof ArrayBuffer,
        true,
        `CoreCache.lookup(key).userMetadata() instanceof ArrayBuffer`,
      );
      assert(
        metadata.byteLength,
        11,
        `CoreCache.lookup(key).userMetadata().byteLength`,
      );
      let result = new TextDecoder().decode(metadata);
      assert(
        result,
        'hello world',
        `new TextDecoder().decode(CoreCache.lookup(key).userMetadata()) === 'hello world'`,
      );
    });
  }

  // body(options?: CacheBodyOptions): ReadableStream;
  {
    routes.set('/cache-entry/body/called-as-constructor', () => {
      assertThrows(() => {
        new CacheEntry.prototype.body();
      }, TypeError);
    });
    routes.set('/cache-entry/body/called-unbound', () => {
      assertThrows(() => {
        CacheEntry.prototype.body.call(undefined);
      }, Error);
    });
    routes.set('/cache-entry/body/called-on-instance', async () => {
      let key = '/cache-entry/body/called-on-instance' + Math.random();
      let writer = CoreCache.insert(key, {
        maxAge: 60 * 1000,
      });
      writer.append('hello');
      writer.close();
      let result = CoreCache.lookup(key).body();
      assert(
        result instanceof ReadableStream,
        true,
        `CoreCache.lookup(key).body() instanceof ReadableStream`,
      );

      result = await streamToString(result);
      assert(
        result,
        'hello',
        `await streamToString(CoreCache.lookup(key).body())`,
      );
    });
    routes.set('/cache-entry/body/options-start-negative', async () => {
      let key = '/cache-entry/body/options-start-negative' + Math.random();
      let writer = CoreCache.insert(key, {
        maxAge: 60 * 1000,
      });
      writer.append('hello');
      writer.close();
      assertThrows(() => {
        CoreCache.lookup(key).body({ start: -1 });
      });
    });
    routes.set('/cache-entry/body/options-start-NaN', async () => {
      let key = '/cache-entry/body/options-start-NaN' + Math.random();
      let writer = CoreCache.insert(key, {
        maxAge: 60 * 1000,
      });
      writer.append('hello');
      writer.close();
      assertThrows(() => {
        CoreCache.lookup(key).body({ start: NaN });
      });
    });
    routes.set('/cache-entry/body/options-start-Infinity', async () => {
      let key = '/cache-entry/body/options-start-Infinity' + Math.random();
      let writer = CoreCache.insert(key, {
        maxAge: 60 * 1000,
      });
      writer.append('hello');
      writer.close();
      assertThrows(() => {
        CoreCache.lookup(key).body({ start: Infinity });
      });
    });
    routes.set('/cache-entry/body/options-start-valid', async () => {
      let key = '/cache-entry/body/options-start-valid' + Math.random();
      let writer = CoreCache.insert(key, {
        maxAge: 60 * 1000,
      });
      writer.append('hello');
      writer.close();
      let result = CoreCache.lookup(key).body({ start: 1 });
      assert(
        result instanceof ReadableStream,
        true,
        `CoreCache.lookup(key).body() instanceof ReadableStream`,
      );

      result = await streamToString(result);
      assert(
        result,
        'ello',
        `await streamToString(CoreCache.lookup(key).body())`,
      );
    });
    routes.set('/cache-entry/body/options-start-longer-than-body', async () => {
      let key =
        '/cache-entry/body/options-start-longer-than-body' + Math.random();
      let writer = CoreCache.insert(key, {
        maxAge: 60 * 1000,
      });
      writer.append('hello');
      writer.close();
      let result = CoreCache.lookup(key).body({ start: 1000 });
      assert(
        result instanceof ReadableStream,
        true,
        `CoreCache.lookup(key).body() instanceof ReadableStream`,
      );

      result = await streamToString(result);
      assert(
        result,
        'hello',
        `await streamToString(CoreCache.lookup(key).body())`,
      );
    });
    routes.set('/cache-entry/body/options-end-negative', async () => {
      let key = '/cache-entry/body/options-end-negative' + Math.random();
      let writer = CoreCache.insert(key, {
        maxAge: 60 * 1000,
      });
      writer.append('hello');
      writer.close();
      assertThrows(() => {
        CoreCache.lookup(key).body({ end: -1 });
      });
    });
    routes.set('/cache-entry/body/options-end-NaN', async () => {
      let key = '/cache-entry/body/options-end-NaN' + Math.random();
      let writer = CoreCache.insert(key, {
        maxAge: 60 * 1000,
      });
      writer.append('hello');
      writer.close();
      assertThrows(() => {
        CoreCache.lookup(key).body({ end: NaN });
      });
    });
    routes.set('/cache-entry/body/options-end-Infinity', async () => {
      let key = '/cache-entry/body/options-end-Infinity' + Math.random();
      let writer = CoreCache.insert(key, {
        maxAge: 60 * 1000,
      });
      writer.append('hello');
      writer.close();
      assertThrows(() => {
        CoreCache.lookup(key).body({ end: Infinity });
      });
    });
    routes.set('/cache-entry/body/options-end-valid', async () => {
      let key = '/cache-entry/body/options-end-valid' + Math.random();
      let writer = CoreCache.insert(key, {
        maxAge: 60 * 1000,
      });
      writer.append('hello');
      writer.close();
      let result = CoreCache.lookup(key).body({ start: 1, end: 1 });
      assert(
        result instanceof ReadableStream,
        true,
        `CoreCache.lookup(key).body() instanceof ReadableStream`,
      );

      result = await streamToString(result);
      console.log({ result });
      assert(result, 'e', `await streamToString(CoreCache.lookup(key).body())`);
    });
    routes.set('/cache-entry/body/options-end-zero', async () => {
      let key = '/cache-entry/body/options-end-zero' + Math.random();
      let writer = CoreCache.insert(key, {
        maxAge: 60 * 1000,
      });
      writer.append('hello');
      writer.close();
      let result = CoreCache.lookup(key).body({ start: 1, end: 0 });
      assert(
        result instanceof ReadableStream,
        true,
        `CoreCache.lookup(key).body() instanceof ReadableStream`,
      );

      result = await streamToString(result);
      console.log({ result });
      assert(
        result,
        'hello',
        `await streamToString(CoreCache.lookup(key).body())`,
      );
    });
  }

  // length(): number;
  {
    routes.set('/cache-entry/length/called-as-constructor', () => {
      assertThrows(() => {
        new CacheEntry.prototype.length();
      }, TypeError);
    });
    routes.set('/cache-entry/length/called-unbound', () => {
      assertThrows(() => {
        CacheEntry.prototype.length.call(undefined);
      }, Error);
    });
    routes.set('/cache-entry/length/called-on-instance', () => {
      let key = '/cache-entry/length/called-on-instance' + Math.random();
      let writer = CoreCache.insert(key, {
        maxAge: 60 * 1000,
      });
      writer.append('hello');
      writer.close();
      let result = CoreCache.lookup(key).length();
      assert(result, 5, `CoreCache.lookup(key).length()`);
    });
    // TODO pass in an entry with unknown length and then call length and check it is null?
    /// The size in bytes of the cached item, if known.
    ///
    /// The length of the cached item may be unknown if the item is currently being streamed into
    /// the cache without a fixed length.
  }

  // maxAge(): number;
  {
    routes.set('/cache-entry/maxAge/called-as-constructor', () => {
      assertThrows(() => {
        new CacheEntry.prototype.maxAge();
      }, TypeError);
    });
    routes.set('/cache-entry/maxAge/called-unbound', () => {
      assertThrows(() => {
        CacheEntry.prototype.maxAge.call(undefined);
      }, Error);
    });
    routes.set('/cache-entry/maxAge/called-on-instance', async () => {
      let key = '/cache-entry/maxAge/called-on-instance' + Math.random();
      let writer = CoreCache.insert(key, {
        maxAge: 60 * 1000,
      });
      writer.append('hello');
      writer.close();
      let result = CoreCache.lookup(key).maxAge();
      assert(result, 60_000, `CoreCache.lookup(key).maxAge()`);
    });
  }

  // staleWhileRevalidate(): number;
  {
    routes.set(
      '/cache-entry/staleWhileRevalidate/called-as-constructor',
      () => {
        assertThrows(() => {
          new CacheEntry.prototype.staleWhileRevalidate();
        }, TypeError);
      },
    );
    routes.set('/cache-entry/staleWhileRevalidate/called-unbound', () => {
      assertThrows(() => {
        CacheEntry.prototype.staleWhileRevalidate.call(undefined);
      }, Error);
    });
    routes.set(
      '/cache-entry/staleWhileRevalidate/called-on-instance',
      async () => {
        let key =
          '/cache-entry/staleWhileRevalidate/called-on-instance' +
          Math.random();
        let writer = CoreCache.insert(key, {
          maxAge: 60 * 1000,
        });
        writer.append('hello');
        writer.close();
        let result = CoreCache.lookup(key).staleWhileRevalidate();
        assert(
          typeof result,
          'number',
          `typeof CoreCache.lookup(key).staleWhileRevalidate()`,
        );
        assert(
          result >= 0,
          true,
          `CoreCache.lookup(key).staleWhileRevalidate() >= 0`,
        );
      },
    );
  }

  // age(): number;
  {
    routes.set('/cache-entry/age/called-as-constructor', () => {
      assertThrows(() => {
        new CacheEntry.prototype.age();
      }, TypeError);
    });
    routes.set('/cache-entry/age/called-unbound', () => {
      assertThrows(() => {
        CacheEntry.prototype.age.call(undefined);
      }, Error);
    });
    routes.set('/cache-entry/age/called-on-instance', async () => {
      let key = '/cache-entry/age/called-on-instance' + Math.random();
      let writer = CoreCache.insert(key, {
        maxAge: 60 * 1000,
      });
      writer.append('hello');
      writer.close();
      let result = CoreCache.lookup(key).age();
      assert(typeof result, 'number', `typeof CoreCache.lookup(key).age()`);
      assert(result >= 0, true, `CoreCache.lookup(key).age() >= 0`);
      await sleep(1000);
      result = CoreCache.lookup(key).age();
      assert(
        result >= 1_000,
        true,
        `CoreCache.lookup(key).age() >= 1_000 (${result})`,
      );
    });
  }

  // hits(): number;
  {
    routes.set('/cache-entry/hits/called-as-constructor', () => {
      assertThrows(() => {
        new CacheEntry.prototype.hits();
      }, TypeError);
    });
    routes.set('/cache-entry/hits/called-unbound', () => {
      assertThrows(() => {
        CacheEntry.prototype.hits.call(undefined);
      }, Error);
    });
    routes.set('/cache-entry/hits/called-on-instance', () => {
      let key = '/cache-entry/hits/called-on-instance' + Math.random();
      let writer = CoreCache.insert(key, {
        maxAge: 60 * 1000,
      });
      writer.append('hello');
      writer.close();
      let result = CoreCache.lookup(key).hits();
      assert(result, 1, `CoreCache.lookup(key).hits()`);
      result = CoreCache.lookup(key).hits();
      assert(result, 2, `CoreCache.lookup(key).hits()`);
    });
  }
}

// TransactionCacheEntry
{
  routes.set('/transaction-cache-entry/interface', () => {
    let actual = Reflect.ownKeys(TransactionCacheEntry);
    let expected = ['prototype', 'length', 'name'];
    assert(actual, expected, `Reflect.ownKeys(TransactionCacheEntry)`);

    // Check the prototype descriptors are correct
    {
      actual = Reflect.getOwnPropertyDescriptor(
        TransactionCacheEntry,
        'prototype',
      );
      expected = {
        value: TransactionCacheEntry.prototype,
        writable: false,
        enumerable: false,
        configurable: false,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(TransactionCacheEntry, 'prototype')`,
      );
    }

    // Check the constructor function's defined parameter length is correct
    {
      actual = Reflect.getOwnPropertyDescriptor(
        TransactionCacheEntry,
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
        `Reflect.getOwnPropertyDescriptor(TransactionCacheEntry, 'length')`,
      );
    }

    // Check the constructor function's name is correct
    {
      actual = Reflect.getOwnPropertyDescriptor(TransactionCacheEntry, 'name');
      expected = {
        value: 'TransactionCacheEntry',
        writable: false,
        enumerable: false,
        configurable: true,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(TransactionCacheEntry, 'name')`,
      );
    }

    // Check the prototype has the correct keys
    {
      actual = Reflect.ownKeys(TransactionCacheEntry.prototype);
      expected = [
        'constructor',
        'insert',
        'insertAndStreamBack',
        'update',
        'cancel',
        Symbol.toStringTag,
      ];
      assert(
        actual,
        expected,
        `Reflect.ownKeys(TransactionCacheEntry.prototype)`,
      );
    }

    // Check the constructor on the prototype is correct
    {
      actual = Reflect.getOwnPropertyDescriptor(
        TransactionCacheEntry.prototype,
        'constructor',
      );
      expected = {
        writable: true,
        enumerable: false,
        configurable: true,
        value: TransactionCacheEntry.prototype.constructor,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(TransactionCacheEntry.prototype, 'constructor')`,
      );

      assert(
        typeof TransactionCacheEntry.prototype.constructor,
        'function',
        `typeof TransactionCacheEntry.prototype.constructor`,
      );

      actual = Reflect.getOwnPropertyDescriptor(
        TransactionCacheEntry.prototype.constructor,
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
        `Reflect.getOwnPropertyDescriptor(TransactionCacheEntry.prototype.constructor, 'length')`,
      );

      actual = Reflect.getOwnPropertyDescriptor(
        TransactionCacheEntry.prototype.constructor,
        'name',
      );
      expected = {
        value: 'TransactionCacheEntry',
        writable: false,
        enumerable: false,
        configurable: true,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(TransactionCacheEntry.prototype.constructor, 'name')`,
      );
    }

    // Check the Symbol.toStringTag on the prototype is correct
    {
      actual = Reflect.getOwnPropertyDescriptor(
        TransactionCacheEntry.prototype,
        Symbol.toStringTag,
      );
      expected = {
        writable: false,
        enumerable: false,
        configurable: true,
        value: 'TransactionCacheEntry',
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(TransactionCacheEntry.prototype, [Symbol.toStringTag])`,
      );

      assert(
        typeof TransactionCacheEntry.prototype[Symbol.toStringTag],
        'string',
        `typeof TransactionCacheEntry.prototype[Symbol.toStringTag]`,
      );
    }

    // Check the insert method has correct descriptors, length and name
    {
      actual = Reflect.getOwnPropertyDescriptor(
        TransactionCacheEntry.prototype,
        'insert',
      );
      expected = {
        writable: true,
        enumerable: true,
        configurable: true,
        value: TransactionCacheEntry.prototype.insert,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(TransactionCacheEntry.prototype, 'insert')`,
      );

      assert(
        typeof TransactionCacheEntry.prototype.insert,
        'function',
        `typeof TransactionCacheEntry.prototype.insert`,
      );

      actual = Reflect.getOwnPropertyDescriptor(
        TransactionCacheEntry.prototype.insert,
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
        `Reflect.getOwnPropertyDescriptor(TransactionCacheEntry.prototype.insert, 'length')`,
      );

      actual = Reflect.getOwnPropertyDescriptor(
        TransactionCacheEntry.prototype.insert,
        'name',
      );
      expected = {
        value: 'insert',
        writable: false,
        enumerable: false,
        configurable: true,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(TransactionCacheEntry.prototype.insert, 'name')`,
      );
    }

    // Check the insertAndStreamBack method has correct descriptors, length and name
    {
      actual = Reflect.getOwnPropertyDescriptor(
        TransactionCacheEntry.prototype,
        'insertAndStreamBack',
      );
      expected = {
        writable: true,
        enumerable: true,
        configurable: true,
        value: TransactionCacheEntry.prototype.insertAndStreamBack,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(TransactionCacheEntry.prototype, 'insertAndStreamBack')`,
      );

      assert(
        typeof TransactionCacheEntry.prototype.insertAndStreamBack,
        'function',
        `typeof TransactionCacheEntry.prototype.insertAndStreamBack`,
      );

      actual = Reflect.getOwnPropertyDescriptor(
        TransactionCacheEntry.prototype.insertAndStreamBack,
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
        `Reflect.getOwnPropertyDescriptor(TransactionCacheEntry.prototype.insertAndStreamBack, 'length')`,
      );

      actual = Reflect.getOwnPropertyDescriptor(
        TransactionCacheEntry.prototype.insertAndStreamBack,
        'name',
      );
      expected = {
        value: 'insertAndStreamBack',
        writable: false,
        enumerable: false,
        configurable: true,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(TransactionCacheEntry.prototype.insertAndStreamBack, 'name')`,
      );
    }

    // Check the update method has correct descriptors, length and name
    {
      actual = Reflect.getOwnPropertyDescriptor(
        TransactionCacheEntry.prototype,
        'update',
      );
      expected = {
        writable: true,
        enumerable: true,
        configurable: true,
        value: TransactionCacheEntry.prototype.update,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(TransactionCacheEntry.prototype, 'update')`,
      );

      assert(
        typeof TransactionCacheEntry.prototype.update,
        'function',
        `typeof TransactionCacheEntry.prototype.update`,
      );

      actual = Reflect.getOwnPropertyDescriptor(
        TransactionCacheEntry.prototype.update,
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
        `Reflect.getOwnPropertyDescriptor(TransactionCacheEntry.prototype.update, 'length')`,
      );

      actual = Reflect.getOwnPropertyDescriptor(
        TransactionCacheEntry.prototype.update,
        'name',
      );
      expected = {
        value: 'update',
        writable: false,
        enumerable: false,
        configurable: true,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(TransactionCacheEntry.prototype.update, 'name')`,
      );
    }

    // Check the cancel method has correct descriptors, length and name
    {
      actual = Reflect.getOwnPropertyDescriptor(
        TransactionCacheEntry.prototype,
        'cancel',
      );
      expected = {
        writable: true,
        enumerable: true,
        configurable: true,
        value: TransactionCacheEntry.prototype.cancel,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(TransactionCacheEntry.prototype, 'cancel')`,
      );

      assert(
        typeof TransactionCacheEntry.prototype.cancel,
        'function',
        `typeof TransactionCacheEntry.prototype.cancel`,
      );

      actual = Reflect.getOwnPropertyDescriptor(
        TransactionCacheEntry.prototype.cancel,
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
        `Reflect.getOwnPropertyDescriptor(TransactionCacheEntry.prototype.cancel, 'length')`,
      );

      actual = Reflect.getOwnPropertyDescriptor(
        TransactionCacheEntry.prototype.cancel,
        'name',
      );
      expected = {
        value: 'cancel',
        writable: false,
        enumerable: false,
        configurable: true,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(TransactionCacheEntry.prototype.cancel, 'name')`,
      );
    }
  });

  // insert(options: TransactionInsertOptions): FastlyBody;
  {
    routes.set('/transaction-cache-entry/insert/called-as-constructor', () => {
      assertThrows(() => {
        let entry = CoreCache.transactionLookup('1');
        new entry.insert({ maxAge: 1 });
      }, TypeError);
    });
    routes.set(
      '/transaction-cache-entry/insert/entry-parameter-not-supplied',
      () => {
        let entry = CoreCache.transactionLookup('1');
        assert(
          entry instanceof TransactionCacheEntry,
          true,
          'entry instanceof TransactionCacheEntry',
        );
        assertThrows(
          () => {
            entry.insert();
          },
          TypeError,
          `insert: At least 1 argument required, but only 0 passed`,
        );
      },
    );

    routes.set(
      '/transaction-cache-entry/insert/options-parameter-maxAge-field-valid-record',
      () => {
        let body;
        assertDoesNotThrow(() => {
          let entry = CoreCache.transactionLookup('1');
          body = entry.insert({ maxAge: 1 });
        });
        assert(
          body instanceof FastlyBody,
          true,
          `entry.insert({maxAge: 1}) instanceof FastlyBody`,
        );
      },
    );
    routes.set(
      '/transaction-cache-entry/insert/options-parameter-maxAge-field-NaN',
      () => {
        assertThrows(() => {
          let entry = CoreCache.transactionLookup('1');
          entry.insert({
            maxAge: NaN,
          });
        });
      },
    );
    routes.set(
      '/transaction-cache-entry/insert/options-parameter-maxAge-field-postitive-infinity',
      () => {
        assertThrows(() => {
          let entry = CoreCache.transactionLookup('1');
          entry.insert({
            maxAge: Number.POSITIVE_INFINITY,
          });
        });
      },
    );
    routes.set(
      '/transaction-cache-entry/insert/options-parameter-maxAge-field-negative-infinity',
      () => {
        assertThrows(() => {
          let entry = CoreCache.transactionLookup('1');
          entry.insert({
            maxAge: Number.NEGATIVE_INFINITY,
          });
        });
      },
    );
    routes.set(
      '/transaction-cache-entry/insert/options-parameter-maxAge-field-negative-number',
      () => {
        assertThrows(() => {
          let entry = CoreCache.transactionLookup('1');
          entry.insert({
            maxAge: -1,
          });
        });
      },
    );
    routes.set(
      '/transaction-cache-entry/insert/options-parameter-initialAge-field-valid-record',
      () => {
        let body;
        assertDoesNotThrow(() => {
          let entry = CoreCache.transactionLookup('1');
          body = entry.insert({ maxAge: 1, initialAge: 1 });
        });
        assert(
          body instanceof FastlyBody,
          true,
          `entry.insert({maxAge: 1,initialAge: 1}) instanceof FastlyBody`,
        );
      },
    );
    routes.set(
      '/transaction-cache-entry/insert/options-parameter-initialAge-field-NaN',
      () => {
        assertThrows(() => {
          let entry = CoreCache.transactionLookup('1');
          entry.insert({
            maxAge: 1,
            initialAge: NaN,
          });
        });
      },
    );
    routes.set(
      '/transaction-cache-entry/insert/options-parameter-initialAge-field-postitive-infinity',
      () => {
        assertThrows(() => {
          let entry = CoreCache.transactionLookup('1');
          entry.insert({
            maxAge: 1,
            initialAge: Number.POSITIVE_INFINITY,
          });
        });
      },
    );
    routes.set(
      '/transaction-cache-entry/insert/options-parameter-initialAge-field-negative-infinity',
      () => {
        assertThrows(() => {
          let entry = CoreCache.transactionLookup('1');
          entry.insert({
            maxAge: 1,
            initialAge: Number.NEGATIVE_INFINITY,
          });
        });
      },
    );
    routes.set(
      '/transaction-cache-entry/insert/options-parameter-initialAge-field-negative-number',
      () => {
        assertThrows(() => {
          let entry = CoreCache.transactionLookup('1');
          entry.insert({
            maxAge: 1,
            initialAge: -1,
          });
        });
      },
    );
    routes.set(
      '/transaction-cache-entry/insert/options-parameter-staleWhileRevalidate-field-valid-record',
      () => {
        let body;
        assertDoesNotThrow(() => {
          let entry = CoreCache.transactionLookup('1');
          body = entry.insert({ maxAge: 1, staleWhileRevalidate: 1 });
        });
        assert(
          body instanceof FastlyBody,
          true,
          `entry.insert({maxAge: 1,staleWhileRevalidate: 1}) instanceof FastlyBody`,
        );
      },
    );
    routes.set(
      '/transaction-cache-entry/insert/options-parameter-staleWhileRevalidate-field-NaN',
      () => {
        assertThrows(() => {
          let entry = CoreCache.transactionLookup('1');
          entry.insert({
            maxAge: 1,
            staleWhileRevalidate: NaN,
          });
        });
      },
    );
    routes.set(
      '/transaction-cache-entry/insert/options-parameter-staleWhileRevalidate-field-postitive-infinity',
      () => {
        assertThrows(() => {
          let entry = CoreCache.transactionLookup('1');
          entry.insert({
            maxAge: 1,
            staleWhileRevalidate: Number.POSITIVE_INFINITY,
          });
        });
      },
    );
    routes.set(
      '/transaction-cache-entry/insert/options-parameter-staleWhileRevalidate-field-negative-infinity',
      () => {
        assertThrows(() => {
          let entry = CoreCache.transactionLookup('1');
          entry.insert({
            maxAge: 1,
            staleWhileRevalidate: Number.NEGATIVE_INFINITY,
          });
        });
      },
    );
    routes.set(
      '/transaction-cache-entry/insert/options-parameter-staleWhileRevalidate-field-negative-number',
      () => {
        assertThrows(() => {
          let entry = CoreCache.transactionLookup('1');
          entry.insert({
            maxAge: 1,
            staleWhileRevalidate: -1,
          });
        });
      },
    );
    routes.set(
      '/transaction-cache-entry/insert/options-parameter-length-field-valid-record',
      () => {
        let body;
        assertDoesNotThrow(() => {
          let entry = CoreCache.transactionLookup('1');
          body = entry.insert({ maxAge: 1, length: 1 });
        });
        assert(
          body instanceof FastlyBody,
          true,
          `entry.insert({maxAge: 1,length: 1}) instanceof FastlyBody`,
        );
      },
    );
    routes.set(
      '/transaction-cache-entry/insert/options-parameter-length-field-NaN',
      () => {
        assertThrows(() => {
          let entry = CoreCache.transactionLookup('1');
          entry.insert({
            maxAge: 1,
            length: NaN,
          });
        });
      },
    );
    routes.set(
      '/transaction-cache-entry/insert/options-parameter-length-field-postitive-infinity',
      () => {
        assertThrows(() => {
          let entry = CoreCache.transactionLookup('1');
          entry.insert({
            maxAge: 1,
            length: Number.POSITIVE_INFINITY,
          });
        });
      },
    );
    routes.set(
      '/transaction-cache-entry/insert/options-parameter-length-field-negative-infinity',
      () => {
        assertThrows(() => {
          let entry = CoreCache.transactionLookup('1');
          entry.insert({
            maxAge: 1,
            length: Number.NEGATIVE_INFINITY,
          });
        });
      },
    );
    routes.set(
      '/transaction-cache-entry/insert/options-parameter-length-field-negative-number',
      () => {
        assertThrows(() => {
          let entry = CoreCache.transactionLookup('1');
          entry.insert({
            maxAge: 1,
            length: -1,
          });
        });
      },
    );
    routes.set(
      '/transaction-cache-entry/insert/options-parameter-sensitive-field',
      () => {
        assertDoesNotThrow(() => {
          let entry = CoreCache.transactionLookup('1');
          entry.insert({
            maxAge: 1,
            sensitive: true,
          });
        });
      },
    );
    routes.set(
      '/transaction-cache-entry/insert/options-parameter-vary-field',
      () => {
        assertDoesNotThrow(() => {
          let entry = CoreCache.transactionLookup('1');
          entry.insert({
            maxAge: 1,
            vary: ['animal', 'mineral', 'vegetable'],
          });
        });
      },
    );
  }

  // insertAndStreamBack(options: TransactionInsertOptions): [FastlyBody, CacheEntry];
  {
    routes.set(
      '/transaction-cache-entry/insertAndStreamBack/called-as-constructor',
      (event) => {
        const path = new URL(event.request.url).pathname;
        assertThrows(() => {
          let entry = CoreCache.transactionLookup(path);
          new entry.insertAndStreamBack({ maxAge: 1 });
        }, TypeError);
      },
    );
    routes.set(
      '/transaction-cache-entry/insertAndStreamBack/entry-parameter-not-supplied',
      (event) => {
        const path = new URL(event.request.url).pathname;
        assertThrows(
          () => {
            let entry = CoreCache.transactionLookup(path);
            entry.insertAndStreamBack();
          },
          TypeError,
          `insertAndStreamBack: At least 1 argument required, but only 0 passed`,
        );
      },
    );

    routes.set(
      '/transaction-cache-entry/insertAndStreamBack/options-parameter-maxAge-field-valid-record',
      (event) => {
        const path = new URL(event.request.url).pathname;
        let writer;
        let reader;
        assertDoesNotThrow(() => {
          let entry = CoreCache.transactionLookup(path);
          [writer, reader] = entry.insertAndStreamBack({ maxAge: 1 });
        });
        assert(
          writer instanceof FastlyBody,
          true,
          `writer instanceof FastlyBody`,
        );
        assert(
          reader instanceof CacheEntry,
          true,
          `writer instanceof CacheEntry`,
        );
      },
    );
    routes.set(
      '/transaction-cache-entry/insertAndStreamBack/options-parameter-maxAge-field-NaN',
      (event) => {
        const path = new URL(event.request.url).pathname;
        assertThrows(() => {
          let entry = CoreCache.transactionLookup(path);
          entry.insertAndStreamBack({
            maxAge: NaN,
          });
        });
      },
    );
    routes.set(
      '/transaction-cache-entry/insertAndStreamBack/options-parameter-maxAge-field-postitive-infinity',
      (event) => {
        const path = new URL(event.request.url).pathname;
        assertThrows(() => {
          let entry = CoreCache.transactionLookup(path);
          entry.insertAndStreamBack({
            maxAge: Number.POSITIVE_INFINITY,
          });
        });
      },
    );
    routes.set(
      '/transaction-cache-entry/insertAndStreamBack/options-parameter-maxAge-field-negative-infinity',
      (event) => {
        const path = new URL(event.request.url).pathname;
        assertThrows(() => {
          let entry = CoreCache.transactionLookup(path);
          entry.insertAndStreamBack({
            maxAge: Number.NEGATIVE_INFINITY,
          });
        });
      },
    );
    routes.set(
      '/transaction-cache-entry/insertAndStreamBack/options-parameter-maxAge-field-negative-number',
      (event) => {
        const path = new URL(event.request.url).pathname;
        assertThrows(() => {
          let entry = CoreCache.transactionLookup(path);
          entry.insertAndStreamBack({
            maxAge: -1,
          });
        });
      },
    );
    routes.set(
      '/transaction-cache-entry/insertAndStreamBack/options-parameter-initialAge-field-valid-record',
      (event) => {
        const path = new URL(event.request.url).pathname;
        let writer;
        let reader;
        assertDoesNotThrow(() => {
          let entry = CoreCache.transactionLookup(path);
          [writer, reader] = entry.insertAndStreamBack({
            maxAge: 1,
            initialAge: 1,
          });
        });
        assert(
          writer instanceof FastlyBody,
          true,
          `writer instanceof FastlyBody`,
        );
        assert(
          reader instanceof CacheEntry,
          true,
          `writer instanceof CacheEntry`,
        );
      },
    );
    routes.set(
      '/transaction-cache-entry/insertAndStreamBack/options-parameter-initialAge-field-NaN',
      (event) => {
        const path = new URL(event.request.url).pathname;
        assertThrows(() => {
          let entry = CoreCache.transactionLookup(path);
          entry.insertAndStreamBack({
            maxAge: 1,
            initialAge: NaN,
          });
        });
      },
    );
    routes.set(
      '/transaction-cache-entry/insertAndStreamBack/options-parameter-initialAge-field-postitive-infinity',
      (event) => {
        const path = new URL(event.request.url).pathname;
        assertThrows(() => {
          let entry = CoreCache.transactionLookup(path);
          entry.insertAndStreamBack({
            maxAge: 1,
            initialAge: Number.POSITIVE_INFINITY,
          });
        });
      },
    );
    routes.set(
      '/transaction-cache-entry/insertAndStreamBack/options-parameter-initialAge-field-negative-infinity',
      (event) => {
        const path = new URL(event.request.url).pathname;
        assertThrows(() => {
          let entry = CoreCache.transactionLookup(path);
          entry.insertAndStreamBack({
            maxAge: 1,
            initialAge: Number.NEGATIVE_INFINITY,
          });
        });
      },
    );
    routes.set(
      '/transaction-cache-entry/insertAndStreamBack/options-parameter-initialAge-field-negative-number',
      (event) => {
        const path = new URL(event.request.url).pathname;
        assertThrows(() => {
          let entry = CoreCache.transactionLookup(path);
          entry.insertAndStreamBack({
            maxAge: 1,
            initialAge: -1,
          });
        });
      },
    );
    routes.set(
      '/transaction-cache-entry/insertAndStreamBack/options-parameter-staleWhileRevalidate-field-valid-record',
      (event) => {
        const path = new URL(event.request.url).pathname;
        let writer, reader;
        assertDoesNotThrow(() => {
          let entry = CoreCache.transactionLookup(path);
          [writer, reader] = entry.insertAndStreamBack({
            maxAge: 1,
            staleWhileRevalidate: 1,
          });
        });
        assert(
          writer instanceof FastlyBody,
          true,
          `writer instanceof FastlyBody`,
        );
        assert(
          reader instanceof CacheEntry,
          true,
          `writer instanceof CacheEntry`,
        );
      },
    );
    routes.set(
      '/transaction-cache-entry/insertAndStreamBack/options-parameter-staleWhileRevalidate-field-NaN',
      (event) => {
        const path = new URL(event.request.url).pathname;
        assertThrows(() => {
          let entry = CoreCache.transactionLookup(path);
          entry.insertAndStreamBack({
            maxAge: 1,
            staleWhileRevalidate: NaN,
          });
        });
      },
    );
    routes.set(
      '/transaction-cache-entry/insertAndStreamBack/options-parameter-staleWhileRevalidate-field-postitive-infinity',
      (event) => {
        const path = new URL(event.request.url).pathname;
        assertThrows(() => {
          let entry = CoreCache.transactionLookup(path);
          entry.insertAndStreamBack({
            maxAge: 1,
            staleWhileRevalidate: Number.POSITIVE_INFINITY,
          });
        });
      },
    );
    routes.set(
      '/transaction-cache-entry/insertAndStreamBack/options-parameter-staleWhileRevalidate-field-negative-infinity',
      (event) => {
        const path = new URL(event.request.url).pathname;
        assertThrows(() => {
          let entry = CoreCache.transactionLookup(path);
          entry.insertAndStreamBack({
            maxAge: 1,
            staleWhileRevalidate: Number.NEGATIVE_INFINITY,
          });
        });
      },
    );
    routes.set(
      '/transaction-cache-entry/insertAndStreamBack/options-parameter-staleWhileRevalidate-field-negative-number',
      (event) => {
        const path = new URL(event.request.url).pathname;
        assertThrows(() => {
          let entry = CoreCache.transactionLookup(path);
          entry.insertAndStreamBack({
            maxAge: 1,
            staleWhileRevalidate: -1,
          });
        });
      },
    );
    routes.set(
      '/transaction-cache-entry/insertAndStreamBack/options-parameter-length-field-valid-record',
      (event) => {
        const path = new URL(event.request.url).pathname;
        let writer, reader;
        assertDoesNotThrow(() => {
          let entry = CoreCache.transactionLookup(path);
          [writer, reader] = entry.insertAndStreamBack({
            maxAge: 1,
            length: 1,
          });
        });
        assert(
          writer instanceof FastlyBody,
          true,
          `writer instanceof FastlyBody`,
        );
        assert(
          reader instanceof CacheEntry,
          true,
          `writer instanceof CacheEntry`,
        );
      },
    );
    routes.set(
      '/transaction-cache-entry/insertAndStreamBack/options-parameter-length-field-NaN',
      (event) => {
        const path = new URL(event.request.url).pathname;
        assertThrows(() => {
          let entry = CoreCache.transactionLookup(path);
          entry.insertAndStreamBack({
            maxAge: 1,
            length: NaN,
          });
        });
      },
    );
    routes.set(
      '/transaction-cache-entry/insertAndStreamBack/options-parameter-length-field-postitive-infinity',
      (event) => {
        const path = new URL(event.request.url).pathname;
        assertThrows(() => {
          let entry = CoreCache.transactionLookup(path);
          entry.insertAndStreamBack({
            maxAge: 1,
            length: Number.POSITIVE_INFINITY,
          });
        });
      },
    );
    routes.set(
      '/transaction-cache-entry/insertAndStreamBack/options-parameter-length-field-negative-infinity',
      (event) => {
        const path = new URL(event.request.url).pathname;
        assertThrows(() => {
          let entry = CoreCache.transactionLookup(path);
          entry.insertAndStreamBack({
            maxAge: 1,
            length: Number.NEGATIVE_INFINITY,
          });
        });
      },
    );
    routes.set(
      '/transaction-cache-entry/insertAndStreamBack/options-parameter-length-field-negative-number',
      (event) => {
        const path = new URL(event.request.url).pathname;
        assertThrows(() => {
          let entry = CoreCache.transactionLookup(path);
          entry.insertAndStreamBack({
            maxAge: 1,
            length: -1,
          });
        });
      },
    );
    routes.set(
      '/transaction-cache-entry/insertAndStreamBack/options-parameter-sensitive-field',
      (event) => {
        const path = new URL(event.request.url).pathname;
        assertDoesNotThrow(() => {
          let entry = CoreCache.transactionLookup(path);
          entry.insertAndStreamBack({
            maxAge: 1,
            sensitive: true,
          });
        });
      },
    );

    routes.set(
      '/transaction-cache-entry/insertAndStreamBack/write-to-writer-and-read-from-reader',
      async (event) => {
        const path = new URL(event.request.url).pathname;
        let entry = CoreCache.transactionLookup(path);
        let [writer, reader] = entry.insertAndStreamBack({
          maxAge: 60 * 1000,
          sensitive: true,
        });
        writer.append('hello');
        writer.close();
        const actual = await new Response(reader.body()).text();
        assert(actual, 'hello', `actual === "hello"`);
      },
    );
    routes.set(
      '/transaction-cache-entry/insertAndStreamBack/options-parameter-vary-field',
      (event) => {
        const path = new URL(event.request.url).pathname;
        assertDoesNotThrow(() => {
          let entry = CoreCache.transactionLookup(path);
          entry.insertAndStreamBack({
            maxAge: 1,
            vary: ['animal', 'mineral', 'vegetable'],
          });
        });
      },
    );
  }

  // update(options: TransactionInsertOptions): void;
  {
    routes.set(
      '/transaction-cache-entry/update/called-as-constructor',
      (event) => {
        const path = new URL(event.request.url).pathname;
        assertThrows(() => {
          let entry = CoreCache.transactionLookup(path);
          new entry.update({ maxAge: 1 });
        }, TypeError);
      },
    );
    routes.set(
      '/transaction-cache-entry/update/entry-parameter-not-supplied',
      (event) => {
        const path = new URL(event.request.url).pathname;
        assertThrows(
          () => {
            let entry = CoreCache.transactionLookup(path);
            entry.update();
          },
          TypeError,
          `update: At least 1 argument required, but only 0 passed`,
        );
      },
    );

    routes.set(
      '/transaction-cache-entry/update/options-parameter-maxAge-field-valid-record',
      (event) => {
        const path = new URL(event.request.url).pathname;
        assertDoesNotThrow(() => {
          let writer = CoreCache.insert(path, {
            maxAge: 0,
            staleWhileRevalidate: 60 * 1000,
          });
          writer.append('meow');
          writer.close();
          let entry = CoreCache.transactionLookup(path);
          entry.update({ maxAge: 1 });
        });
      },
    );
    routes.set(
      '/transaction-cache-entry/update/options-parameter-maxAge-field-NaN',
      (event) => {
        const path = new URL(event.request.url).pathname;
        assertThrows(() => {
          let entry = CoreCache.transactionLookup(path);
          entry.update({
            maxAge: NaN,
          });
        });
      },
    );
    routes.set(
      '/transaction-cache-entry/update/options-parameter-maxAge-field-postitive-infinity',
      (event) => {
        const path = new URL(event.request.url).pathname;
        assertThrows(() => {
          let entry = CoreCache.transactionLookup(path);
          entry.update({
            maxAge: Number.POSITIVE_INFINITY,
          });
        });
      },
    );
    routes.set(
      '/transaction-cache-entry/update/options-parameter-maxAge-field-negative-infinity',
      (event) => {
        const path = new URL(event.request.url).pathname;
        assertThrows(() => {
          let entry = CoreCache.transactionLookup(path);
          entry.update({
            maxAge: Number.NEGATIVE_INFINITY,
          });
        });
      },
    );
    routes.set(
      '/transaction-cache-entry/update/options-parameter-maxAge-field-negative-number',
      (event) => {
        const path = new URL(event.request.url).pathname;
        assertThrows(() => {
          let entry = CoreCache.transactionLookup(path);
          entry.update({
            maxAge: -1,
          });
        });
      },
    );
    routes.set(
      '/transaction-cache-entry/update/options-parameter-initialAge-field-valid-record',
      (event) => {
        const path = new URL(event.request.url).pathname;
        assertDoesNotThrow(() => {
          let writer = CoreCache.insert(path, {
            maxAge: 0,
            staleWhileRevalidate: 60 * 1000,
          });
          writer.append('meow');
          writer.close();
          let entry = CoreCache.transactionLookup(path);
          entry.update({ maxAge: 1, initialAge: 1 });
        });
      },
    );
    routes.set(
      '/transaction-cache-entry/update/options-parameter-initialAge-field-NaN',
      (event) => {
        const path = new URL(event.request.url).pathname;
        assertThrows(() => {
          let entry = CoreCache.transactionLookup(path);
          entry.update({
            maxAge: 1,
            initialAge: NaN,
          });
        });
      },
    );
    routes.set(
      '/transaction-cache-entry/update/options-parameter-initialAge-field-postitive-infinity',
      (event) => {
        const path = new URL(event.request.url).pathname;
        assertThrows(() => {
          let entry = CoreCache.transactionLookup(path);
          entry.update({
            maxAge: 1,
            initialAge: Number.POSITIVE_INFINITY,
          });
        });
      },
    );
    routes.set(
      '/transaction-cache-entry/update/options-parameter-initialAge-field-negative-infinity',
      (event) => {
        const path = new URL(event.request.url).pathname;
        assertThrows(() => {
          let entry = CoreCache.transactionLookup(path);
          entry.update({
            maxAge: 1,
            initialAge: Number.NEGATIVE_INFINITY,
          });
        });
      },
    );
    routes.set(
      '/transaction-cache-entry/update/options-parameter-initialAge-field-negative-number',
      (event) => {
        const path = new URL(event.request.url).pathname;
        assertThrows(() => {
          let entry = CoreCache.transactionLookup(path);
          entry.update({
            maxAge: 1,
            initialAge: -1,
          });
        });
      },
    );
    routes.set(
      '/transaction-cache-entry/update/options-parameter-staleWhileRevalidate-field-valid-record',
      (event) => {
        const path = new URL(event.request.url).pathname;
        assertDoesNotThrow(() => {
          let writer = CoreCache.insert(path, {
            maxAge: 0,
            staleWhileRevalidate: 60 * 1000,
          });
          writer.append('meow');
          writer.close();
          let entry = CoreCache.transactionLookup(path);
          entry.update({ maxAge: 1, staleWhileRevalidate: 1 });
        });
      },
    );
    routes.set(
      '/transaction-cache-entry/update/options-parameter-staleWhileRevalidate-field-NaN',
      (event) => {
        const path = new URL(event.request.url).pathname;
        assertThrows(() => {
          let entry = CoreCache.transactionLookup(path);
          entry.update({
            maxAge: 1,
            staleWhileRevalidate: NaN,
          });
        });
      },
    );
    routes.set(
      '/transaction-cache-entry/update/options-parameter-staleWhileRevalidate-field-postitive-infinity',
      (event) => {
        const path = new URL(event.request.url).pathname;
        assertThrows(() => {
          let entry = CoreCache.transactionLookup(path);
          entry.update({
            maxAge: 1,
            staleWhileRevalidate: Number.POSITIVE_INFINITY,
          });
        });
      },
    );
    routes.set(
      '/transaction-cache-entry/update/options-parameter-staleWhileRevalidate-field-negative-infinity',
      (event) => {
        const path = new URL(event.request.url).pathname;
        assertThrows(() => {
          let entry = CoreCache.transactionLookup(path);
          entry.update({
            maxAge: 1,
            staleWhileRevalidate: Number.NEGATIVE_INFINITY,
          });
        });
      },
    );
    routes.set(
      '/transaction-cache-entry/update/options-parameter-staleWhileRevalidate-field-negative-number',
      (event) => {
        const path = new URL(event.request.url).pathname;
        assertThrows(() => {
          let entry = CoreCache.transactionLookup(path);
          entry.update({
            maxAge: 1,
            staleWhileRevalidate: -1,
          });
        });
      },
    );
    routes.set(
      '/transaction-cache-entry/update/options-parameter-length-field-valid-record',
      (event) => {
        const path = new URL(event.request.url).pathname;
        assertDoesNotThrow(() => {
          let writer = CoreCache.insert(path, {
            maxAge: 0,
            staleWhileRevalidate: 60 * 1000,
          });
          writer.append('meow');
          writer.close();
          let entry = CoreCache.transactionLookup(path);
          entry.update({ maxAge: 1, length: 1 });
        });
      },
    );
    routes.set(
      '/transaction-cache-entry/update/options-parameter-length-field-NaN',
      (event) => {
        const path = new URL(event.request.url).pathname;
        assertThrows(() => {
          let entry = CoreCache.transactionLookup(path);
          entry.update({
            maxAge: 1,
            length: NaN,
          });
        });
      },
    );
    routes.set(
      '/transaction-cache-entry/update/options-parameter-length-field-postitive-infinity',
      (event) => {
        const path = new URL(event.request.url).pathname;
        assertThrows(() => {
          let entry = CoreCache.transactionLookup(path);
          entry.update({
            maxAge: 1,
            length: Number.POSITIVE_INFINITY,
          });
        });
      },
    );
    routes.set(
      '/transaction-cache-entry/update/options-parameter-length-field-negative-infinity',
      (event) => {
        const path = new URL(event.request.url).pathname;
        assertThrows(() => {
          let entry = CoreCache.transactionLookup(path);
          entry.update({
            maxAge: 1,
            length: Number.NEGATIVE_INFINITY,
          });
        });
      },
    );
    routes.set(
      '/transaction-cache-entry/update/options-parameter-length-field-negative-number',
      (event) => {
        const path = new URL(event.request.url).pathname;
        assertThrows(() => {
          let entry = CoreCache.transactionLookup(path);
          entry.update({
            maxAge: 1,
            length: -1,
          });
        });
      },
    );
    routes.set(
      '/transaction-cache-entry/update/write-to-writer-and-read-from-reader',
      async (event) => {
        const path = new URL(event.request.url).pathname;
        let entry = CoreCache.transactionLookup(path);
        let writer = entry.insert({
          maxAge: 1,
          staleWhileRevalidate: 60 * 1000,
        });
        writer.append('meow');
        writer.close();
        entry = CoreCache.transactionLookup(path);
        entry.update({
          maxAge: 60 * 1000,
        });
        await sleep(1000);
        entry = CoreCache.transactionLookup(path);
        assert(entry.maxAge(), 60 * 1000, `entry2.maxAge() === 60 * 1000`);
      },
    );
    routes.set(
      '/transaction-cache-entry/update/options-parameter-vary-field',
      async (event) => {
        const path = new URL(event.request.url).pathname;
        let entry = CoreCache.transactionLookup(path);
        let writer = entry.insert({
          maxAge: 1,
          staleWhileRevalidate: 60 * 1000,
          vary: ['animal', 'mineral', 'vegetable'],
        });
        writer.append('meow');
        writer.close();
        await sleep(1000);
        entry = CoreCache.transactionLookup(path);
        entry.update({
          maxAge: 1000,
          vary: ['animal'],
        });
      },
    );
    routes.set(
      '/transaction-cache-entry/update/options-parameter-userMetadata-field',
      async (event) => {
        const path = new URL(event.request.url).pathname;
        let entry = CoreCache.transactionLookup(path);
        let writer = entry.insert({
          maxAge: 1,
          staleWhileRevalidate: 60 * 1000,
        });
        writer.append('meow');
        writer.close();
        await sleep(1000);
        entry = CoreCache.transactionLookup(path);
        entry.update({
          maxAge: 1000,
          userMetadata: 'hello',
        });
      },
    );
    // TODO: tests for options parameter fields
    // surrogateKeys?: Array<string>,-- empty string? -- toString which throws -- wrong types?
  }

  // cancel(): void;
  {
    routes.set(
      '/transaction-cache-entry/cancel/called-as-constructor',
      (event) => {
        const path = new URL(event.request.url).pathname;
        assertThrows(() => {
          let entry = CoreCache.transactionLookup(path);
          new entry.cancel();
        }, TypeError);
      },
    );
    routes.set('/transaction-cache-entry/cancel/called-once', (event) => {
      const path = new URL(event.request.url).pathname;
      assertDoesNotThrow(() => {
        let entry = CoreCache.transactionLookup(path);
        entry.cancel();
      });
    });
    routes.set(
      '/transaction-cache-entry/cancel/makes-entry-cancelled',
      (event) => {
        const path = new URL(event.request.url).pathname;
        let entry;
        assertDoesNotThrow(() => {
          entry = CoreCache.transactionLookup(path);
          entry.cancel();
        });
        assertThrows(() => {
          entry.insert({ maxAge: 1 });
        });
      },
    );
    routes.set(
      '/transaction-cache-entry/cancel/called-twice-throws',
      (event) => {
        const path = new URL(event.request.url).pathname;
        let entry;
        assertDoesNotThrow(() => {
          entry = CoreCache.transactionLookup(path);
          entry.cancel();
        });
        assertThrows(() => {
          entry.cancel();
        });
      },
    );
  }
}

{
  routes.set(
    '/core-cache/transaction-lookup-transaction-insert-vary-works',
    async () => {
      const key = `/core-cache/vary-works-${Date.now()}`;
      const animal = 'animal';
      let entry = CoreCache.transactionLookup(key, {
        headers: {
          [animal]: 'cat',
        },
      });
      assert(entry.state().found(), false, `entry.state().found() === false`);
      let writer = entry.insert({
        maxAge: 60_000 * 60,
        vary: [animal],
        headers: {
          [animal]: 'cat',
        },
      });

      writer.append('cat');
      writer.close();
      entry.close();
      await sleep(1_000);

      entry = CoreCache.transactionLookup(key, {
        headers: {
          [animal]: 'cat',
        },
      });
      assert(entry.state().found(), true, `entry.state().found() === true`);

      assert(
        await streamToString(entry.body()),
        'cat',
        `await streamToString(CoreCache.lookup(key).body())`,
      );
      entry.close();

      entry = CoreCache.transactionLookup(key, {
        headers: {
          [animal]: 'dog',
        },
      });
      assert(entry.state().found(), false, `entry.state().found() == false`);

      writer = entry.insert({
        maxAge: 60_000 * 60,
        vary: [animal],
        headers: {
          [animal]: 'dog',
        },
      });

      writer.append('dog');
      writer.close();
      entry.close();
      await sleep(1_000);

      entry = CoreCache.transactionLookup(key, {
        headers: {
          [animal]: 'dog',
        },
      });
      assert(entry.state().found(), true, `entry.state().found() === true`);

      assert(
        await streamToString(entry.body()),
        'dog',
        `await streamToString(CoreCache.lookup(key).body())`,
      );
      entry.close();
    },
  );

  routes.set('/core-cache/lookup-insert-vary-works', async () => {
    const key = `/core-cache/vary-works-${Date.now()}`;
    const animal = 'animal';
    let entry = CoreCache.lookup(key, {
      headers: {
        [animal]: 'cat',
      },
    });
    assert(entry, null, `entry == null`);
    let writer = CoreCache.insert(key, {
      maxAge: 60_000 * 60,
      vary: [animal],
      headers: {
        [animal]: 'cat',
      },
    });

    writer.append('cat');
    writer.close();
    await sleep(1_000);

    entry = CoreCache.lookup(key, {
      headers: {
        [animal]: 'cat',
      },
    });
    assert(entry.state().found(), true, `entry.state().found() === true`);

    assert(
      await streamToString(entry.body()),
      'cat',
      `await streamToString(CoreCache.lookup(key).body())`,
    );
    entry.close();

    entry = CoreCache.lookup(key, {
      headers: {
        [animal]: 'dog',
      },
    });
    assert(entry, null, `entry == null`);

    writer = CoreCache.insert(key, {
      maxAge: 60_000 * 60,
      vary: [animal],
      headers: {
        [animal]: 'dog',
      },
    });

    writer.append('dog');
    writer.close();
    await sleep(1_000);

    entry = CoreCache.lookup(key, {
      headers: {
        [animal]: 'dog',
      },
    });
    assert(entry.state().found(), true, `entry.state().found() === true`);

    assert(
      await streamToString(entry.body()),
      'dog',
      `await streamToString(CoreCache.lookup(key).body())`,
    );
  });
}
