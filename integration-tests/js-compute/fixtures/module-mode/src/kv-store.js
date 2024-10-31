/// <reference path="../../../../../types/index.d.ts" />
import {
  strictEqual,
  assertThrows,
  assertRejects,
  assertResolves,
  deepStrictEqual,
} from './assertions.js';
import { KVStore } from 'fastly:kv-store';
import { routes, isRunningLocally } from './routes.js';

// kvstore e2e tests
{
  routes.set('/kv-store-e2e/list', async () => {
    const store = new KVStore('example-test-kv-store');
    // bad metadata
    await store.put('a', 'b');
    const aEntry = await store.get('a');
    strictEqual(aEntry.metadata(), null);

    for (let i = 0; i < 100; i++) {
      await store.put('c' + i, 'd', {
        metadata: i % 2 === 0 ? '42' : new Uint8Array([42]),
      });
    }
    const cEntry = await store.get('c1');
    strictEqual(await cEntry.text(), 'd');

    const metadata = await cEntry.metadata();
    strictEqual(metadata.byteLength, 1);
    strictEqual(metadata[0], 42);

    const cEntry2 = await store.get('c2');
    strictEqual(await cEntry2.metadataText(), '42');

    const metadataText = await cEntry.metadataText();
    strictEqual(metadataText, '*');

    await store.put('c5', 'cba', {
      mode: 'prepend',
      metadata: new Uint8Array([0xf0, 0xf0]),
      ttl: 5,
    });
    const c5Entry = await store.get('c5');
    strictEqual(await c5Entry.text(), 'cbad');
    assertRejects(async () => await c5Entry.metadataText(), TypeError);

    // TTL only supported on compute not viceroy
    if (!isRunningLocally()) {
      await new Promise((resolve) => setTimeout(resolve, 10));
      strictEqual(await store.get('c5'), null);
    }

    assertThrows(() => {
      store.list({ limit: 'booooo' });
    }, TypeError);

    assertThrows(() => {
      store.list({ limit: 5.5 });
    }, TypeError);

    strictEqual(await store.get('noone'), null);

    let { list, cursor } = await store.list({ limit: 10, prefix: 'c' });

    deepStrictEqual(list, [
      'c0',
      'c1',
      'c10',
      'c11',
      'c12',
      'c13',
      'c14',
      'c15',
      'c16',
      'c17',
    ]);

    ({ list, cursor } = await store.list({ limit: 10, prefix: 'c', cursor }));

    deepStrictEqual(list, [
      'c18',
      'c19',
      'c2',
      'c20',
      'c21',
      'c22',
      'c23',
      'c24',
      'c25',
      'c26',
    ]);
  });
}

// KVStore
{
  routes.set('/kv-store/exposed-as-global', async () => {
    strictEqual(typeof KVStore, 'function', `typeof KVStore`);
  });
  routes.set('/kv-store/interface', kvStoreInterfaceTests);
  // KVStore constructor
  {
    routes.set('/kv-store/constructor/called-as-regular-function', async () => {
      assertThrows(
        () => {
          KVStore();
        },
        TypeError,
        `calling a builtin KVStore constructor without new is forbidden`,
      );
    });
    // https://tc39.es/ecma262/#sec-tostring
    routes.set(
      '/kv-store/constructor/parameter-calls-7.1.17-ToString',
      async () => {
        let sentinel;
        const test = () => {
          sentinel = Symbol();
          const name = {
            toString() {
              throw sentinel;
            },
          };
          new KVStore(name);
        };
        assertThrows(test);
        try {
          test();
        } catch (thrownError) {
          strictEqual(thrownError, sentinel, 'thrownError === sentinel');
        }
        assertThrows(
          () => new KVStore(Symbol()),
          TypeError,
          `can't convert symbol to string`,
        );
      },
    );
    routes.set('/kv-store/constructor/empty-parameter', async () => {
      assertThrows(
        () => {
          new KVStore();
        },
        TypeError,
        `KVStore constructor: At least 1 argument required, but only 0 passed`,
      );
    });
    routes.set('/kv-store/constructor/found-store', async () => {
      const store = new KVStore('example-test-kv-store');
      strictEqual(store instanceof KVStore, true, `store instanceof KVStore`);
    });
    routes.set('/kv-store/constructor/missing-store', async () => {
      assertThrows(
        () => {
          new KVStore('missing');
        },
        Error,
        `KVStore constructor: No KVStore named 'missing' exists`,
      );
    });
    routes.set('/kv-store/constructor/invalid-name', async () => {
      // control Characters (\\u0000-\\u001F) are not allowed
      const controlCharacters = [
        '\u0000',
        '\u0001',
        '\u0002',
        '\u0003',
        '\u0004',
        '\u0005',
        '\u0006',
        '\u0007',
        '\u0008',
        '\u0009',
        '\u000A',
        '\u000B',
        '\u000C',
        '\u000D',
        '\u000E',
        '\u0010',
        '\u0011',
        '\u0012',
        '\u0013',
        '\u0014',
        '\u0015',
        '\u0016',
        '\u0017',
        '\u0018',
        '\u0019',
        '\u001A',
        '\u001B',
        '\u001C',
        '\u001D',
        '\u001E',
        '\u001F',
      ];
      for (const character of controlCharacters) {
        assertThrows(
          () => {
            new KVStore(character);
          },
          TypeError,
          `KVStore constructor: name can not contain control characters (\\u0000-\\u001F)`,
        );
      }

      // must be less than 256 characters
      assertThrows(
        () => {
          new KVStore('1'.repeat(256));
        },
        TypeError,
        `KVStore constructor: name can not be more than 255 characters`,
      );

      // empty string not allowed
      assertThrows(
        () => {
          new KVStore('');
        },
        TypeError,
        `KVStore constructor: name can not be an empty string`,
      );
    });
  }

  // KVStore put method
  {
    routes.set('/kv-store/put/called-as-constructor', async () => {
      assertThrows(() => {
        new KVStore.prototype.put('1', '1');
      }, TypeError);
    });
    routes.set('/kv-store/put/called-unbound', async () => {
      assertThrows(() => {
        KVStore.prototype.put.call(undefined, '1', '2');
      }, TypeError);
    });
    // https://tc39.es/ecma262/#sec-tostring
    routes.set(
      '/kv-store/put/key-parameter-calls-7.1.17-ToString',
      async () => {
        let sentinel;
        const test = async () => {
          sentinel = Symbol();
          const key = {
            toString() {
              throw sentinel;
            },
          };
          const store = new KVStore('example-test-kv-store');
          await store.put(key, '');
        };
        await assertRejects(test);
        try {
          await test();
        } catch (thrownError) {
          strictEqual(thrownError, sentinel, 'thrownError === sentinel');
        }
        await assertRejects(
          async () => {
            const store = new KVStore('example-test-kv-store');
            await store.put(Symbol(), '');
          },
          Error,
          `can't convert symbol to string`,
        );
      },
    );
    routes.set('/kv-store/put/key-parameter-not-supplied', async () => {
      await assertRejects(
        async () => {
          const store = new KVStore('example-test-kv-store');
          await store.put();
        },
        TypeError,
        `put: At least 2 arguments required, but only 0 passed`,
      );
    });
    routes.set('/kv-store/put/key-parameter-empty-string', async () => {
      await assertRejects(
        async () => {
          const store = new KVStore('example-test-kv-store');
          await store.put('', '');
        },
        TypeError,
        `KVStore key can not be an empty string`,
      );
    });
    routes.set(
      '/kv-store/put/key-parameter-1024-character-string',
      async () => {
        await assertResolves(async () => {
          const store = new KVStore('example-test-kv-store');
          const key = 'a'.repeat(1024);
          await store.put(key, '');
        });
      },
    );
    routes.set(
      '/kv-store/put/key-parameter-1025-character-string',
      async () => {
        await assertRejects(
          async () => {
            const store = new KVStore('example-test-kv-store');
            const key = 'a'.repeat(1025);
            await store.put(key, '');
          },
          TypeError,
          `KVStore key can not be more than 1024 characters`,
        );
      },
    );
    routes.set('/kv-store/put/key-parameter-containing-newline', async () => {
      await assertRejects(
        async () => {
          let store = new KVStore('example-test-kv-store');
          await store.put('\n', '');
        },
        TypeError,
        `KVStore key can not contain newline character`,
      );
    });
    routes.set(
      '/kv-store/put/key-parameter-containing-carriage-return',
      async () => {
        await assertRejects(
          async () => {
            let store = new KVStore('example-test-kv-store');
            await store.put('\r', '');
          },
          TypeError,
          `KVStore key can not contain carriage return character`,
        );
      },
    );
    routes.set(
      '/kv-store/put/key-parameter-starting-with-well-known-acme-challenge',
      async () => {
        await assertRejects(
          async () => {
            let store = new KVStore('example-test-kv-store');
            await store.put('.well-known/acme-challenge/', '');
          },
          TypeError,
          `KVStore key can not start with .well-known/acme-challenge/`,
        );
      },
    );
    routes.set('/kv-store/put/key-parameter-single-dot', async () => {
      await assertRejects(
        async () => {
          let store = new KVStore('example-test-kv-store');
          await store.put('.', '');
        },
        TypeError,
        `KVStore key can not be '.' or '..'`,
      );
    });
    routes.set('/kv-store/put/key-parameter-double-dot', async () => {
      await assertRejects(
        async () => {
          let store = new KVStore('example-test-kv-store');
          await store.put('..', '');
        },
        TypeError,
        `KVStore key can not be '.' or '..'`,
      );
    });
    routes.set(
      '/kv-store/put/key-parameter-containing-special-characters',
      async () => {
        const specialCharacters = ['[', ']', '*', '?', '#'];
        for (const character of specialCharacters) {
          await assertRejects(
            async () => {
              let store = new KVStore('example-test-kv-store');
              await store.put(character, '');
            },
            TypeError,
            `KVStore key can not contain ${character} character`,
          );
        }
      },
    );
    routes.set('/kv-store/put/value-parameter-as-undefined', async () => {
      const store = new KVStore('example-test-kv-store');
      let result = store.put('undefined', undefined);
      strictEqual(
        result instanceof Promise,
        true,
        'store.put("undefined", undefined) instanceof Promise',
      );
      strictEqual(
        await result,
        undefined,
        'await store.put("undefined", undefined)',
      );
    });
    routes.set('/kv-store/put/value-parameter-not-supplied', async () => {
      await assertRejects(
        async () => {
          const store = new KVStore('example-test-kv-store');
          await store.put('test');
        },
        TypeError,
        `put: At least 2 arguments required, but only 1 passed`,
      );
    });
    // - ReadableStream
    routes.set(
      '/kv-store/put/value-parameter-readablestream-empty',
      async () => {
        // TODO: remove this when streams are supported
        await assertRejects(
          async () => {
            const stream = iteratableToStream([]);
            const store = new KVStore('example-test-kv-store');
            await store.put('readablestream-empty', stream);
          },
          TypeError,
          `Content-provided streams are not yet supported for streaming into KVStore`,
        );
        // TODO: uncomment this when conte-provided (guest) streams are supported
        // const stream = iteratableToStream([])
        // const store = new KVStore('example-test-kv-store')
        // let result = store.put('readablestream-empty', stream)
        // strictEqual(result instanceof Promise, true, `store.put('readablestream-empty', stream) instanceof Promise`)
        // strictEqual(await result, undefined, `await store.put('readablestream-empty', stream)`)
      },
    );
    routes.set(
      '/kv-store/put/value-parameter-readablestream-under-30mb',
      async () => {
        const res = await fetch(
          'https://compute-sdk-test-backend.edgecompute.app/',
          {
            backend: 'TheOrigin',
          },
        );
        const store = new KVStore('example-test-kv-store');
        let result = store.put('readablestream-under-30mb', res.body);
        strictEqual(
          result instanceof Promise,
          true,
          `store.put('readablestream-under-30mb', stream) instanceof Promise`,
        );
        strictEqual(
          await result,
          undefined,
          `await store.put('readablestream-under-30mb', stream)`,
        );
      },
    );
    routes.set('/kv-store/put/request-body', async ({ request }) => {
      const store = new KVStore('example-test-kv-store');
      let result = store.put('readablestream-req', request.body);
      strictEqual(
        result instanceof Promise,
        true,
        `store.put('readablestream-req', request.body) instanceof Promise`,
      );
      strictEqual(
        await result,
        undefined,
        `await store.put('readablestream-req', request.body)`,
      );
    });
    routes.set(
      '/kv-store/put/value-parameter-readablestream-over-30mb',
      async () => {
        // TODO: remove this when streams are supported
        await assertRejects(
          async () => {
            const stream = iteratableToStream([
              'x'.repeat(30 * 1024 * 1024) + 'x',
            ]);
            const store = new KVStore('example-test-kv-store');
            await store.put('readablestream-over-30mb', stream);
          },
          Error,
          `Content-provided streams are not yet supported for streaming into KVStore`,
        );
        // TODO: uncomment this when conte-provided (guest) streams are supported
        // const stream = iteratableToStream(['x'.repeat(30*1024*1024) + 'x'])
        // const store = new KVStore('example-test-kv-store')
        // let result = store.put('readablestream-over-30mb', stream)
        // strictEqual(result instanceof Promise, true, `store.put('readablestream-over-30mb', stream) instanceof Promise`)
        // strictEqual(await result, undefined, `await store.put('readablestream-over-30mb', stream)`)
      },
    );
    routes.set(
      '/kv-store/put/value-parameter-readablestream-locked',
      async () => {
        const stream = iteratableToStream([]);
        // getReader() causes the stream to become locked
        stream.getReader();
        const store = new KVStore('example-test-kv-store');
        await assertRejects(
          async () => {
            await store.put('readablestream-locked', stream);
            // await store.put("test", stream)
          },
          TypeError,
          `Can't use a ReadableStream that's locked or has ever been read from or canceled`,
        );
      },
    );

    // - URLSearchParams
    routes.set('/kv-store/put/value-parameter-URLSearchParams', async () => {
      const items = [
        new URLSearchParams(),
        new URLSearchParams({ a: 'b', c: 'd' }),
      ];
      const store = new KVStore('example-test-kv-store');
      for (const searchParams of items) {
        let result = store.put('URLSearchParams', searchParams);
        strictEqual(
          result instanceof Promise,
          true,
          `store.put('URLSearchParams', searchParams) instanceof Promise`,
        );
        strictEqual(
          await result,
          undefined,
          `await store.put('URLSearchParams', searchParams)`,
        );
      }
    });
    // - USV strings
    routes.set('/kv-store/put/value-parameter-strings', async () => {
      const strings = [
        // empty
        '',
        // lone surrogate
        '\uD800',
        // surrogate pair
        '𠈓',
        String('carrot'),
      ];
      const store = new KVStore('example-test-kv-store');
      for (const string of strings) {
        let result = store.put('string', string);
        strictEqual(
          result instanceof Promise,
          true,
          `store.put('string', string) instanceof Promise`,
        );
        strictEqual(
          await result,
          undefined,
          `await store.put('string', string)`,
        );
      }
    });

    routes.set('/kv-store/put/value-parameter-string-over-30mb', async () => {
      const string = 'x'.repeat(35 * 1024 * 1024) + 'x';
      const store = new KVStore('example-test-kv-store');
      await assertRejects(
        () => store.put('string-over-30mb', string),
        TypeError,
        `KVStore value can not be more than 30 Megabytes in size`,
      );
    });

    // https://tc39.es/ecma262/#sec-tostring
    routes.set(
      '/kv-store/put/value-parameter-calls-7.1.17-ToString',
      async () => {
        let sentinel;
        const test = async () => {
          sentinel = Symbol();
          const value = {
            toString() {
              throw sentinel;
            },
          };
          const store = new KVStore('example-test-kv-store');
          await store.put('toString', value);
        };
        await assertRejects(test);
        try {
          await test();
        } catch (thrownError) {
          strictEqual(thrownError, sentinel, 'thrownError === sentinel');
        }
        await assertRejects(
          async () => {
            const store = new KVStore('example-test-kv-store');
            await store.put('Symbol()', Symbol());
          },
          TypeError,
          `can't convert symbol to string`,
        );
      },
    );

    // - buffer source
    routes.set('/kv-store/put/value-parameter-buffer', async () => {
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
      const store = new KVStore('example-test-kv-store');
      for (const constructor of typedArrayConstructors) {
        const typedArray = new constructor(8);
        let result = store.put(constructor.name, typedArray.buffer);
        strictEqual(
          result instanceof Promise,
          true,
          `store.put(${constructor.name}, typedArray.buffer) instanceof Promise`,
        );
        strictEqual(
          await result,
          undefined,
          `await store.put(${constructor.name}, typedArray.buffer)`,
        );
      }
    });
    routes.set('/kv-store/put/value-parameter-arraybuffer', async () => {
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
      const store = new KVStore('example-test-kv-store');
      for (const constructor of typedArrayConstructors) {
        const typedArray = new constructor(8);
        let result = store.put(constructor.name, typedArray.buffer);
        strictEqual(
          result instanceof Promise,
          true,
          `store.put(${constructor.name}, typedArray.buffer) instanceof Promise`,
        );
        strictEqual(
          await result,
          undefined,
          `await store.put(${constructor.name}, typedArray.buffer)`,
        );
      }
    });
    routes.set('/kv-store/put/value-parameter-typed-arrays', async () => {
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
      const store = new KVStore('example-test-kv-store');
      for (const constructor of typedArrayConstructors) {
        const typedArray = new constructor(8);
        let result = store.put(constructor.name, typedArray);
        strictEqual(
          result instanceof Promise,
          true,
          `store.put(${constructor.name}, typedArray) instanceof Promise`,
        );
        strictEqual(
          await result,
          undefined,
          `await store.put(${constructor.name}, typedArray)`,
        );
      }
    });
    routes.set('/kv-store/put/value-parameter-dataview', async () => {
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
      const store = new KVStore('example-test-kv-store');
      for (const constructor of typedArrayConstructors) {
        const typedArray = new constructor(8);
        const view = new DataView(typedArray.buffer);
        let result = store.put(`new DataView(${constructor.name})`, view);
        strictEqual(
          result instanceof Promise,
          true,
          `store.put(new DataView(${constructor.name}), typedArray) instanceof Promise`,
        );
        strictEqual(
          await result,
          undefined,
          `await store.put(new DataView(${constructor.name}), typedArray)`,
        );
      }
    });
  }

  // KVStore delete method
  {
    routes.set('/kv-store/delete/called-as-constructor', async () => {
      assertThrows(() => {
        new KVStore.prototype.delete('1');
      }, TypeError);
    });
    routes.set('/kv-store/delete/called-unbound', async () => {
      await assertRejects(async () => {
        await KVStore.prototype.delete.call(undefined, '1');
      }, TypeError);
    });
    // https://tc39.es/ecma262/#sec-tostring
    routes.set(
      '/kv-store/delete/key-parameter-calls-7.1.17-ToString',
      async () => {
        let sentinel;
        const test = async () => {
          sentinel = Symbol();
          const key = {
            toString() {
              throw sentinel;
            },
          };
          const store = new KVStore('example-test-kv-store');
          await store.delete(key);
        };
        await assertRejects(test);
        try {
          await test();
        } catch (thrownError) {
          strictEqual(thrownError, sentinel, 'thrownError === sentinel');
        }
        await assertRejects(
          async () => {
            const store = new KVStore('example-test-kv-store');
            await store.delete(Symbol());
          },
          TypeError,
          `can't convert symbol to string`,
        );
      },
    );
    routes.set('/kv-store/delete/key-parameter-not-supplied', async () => {
      await assertRejects(
        async () => {
          const store = new KVStore('example-test-kv-store');
          await store.delete();
        },
        TypeError,
        `delete: At least 1 argument required, but only 0 passed`,
      );
    });
    routes.set('/kv-store/delete/key-parameter-empty-string', async () => {
      await assertRejects(
        async () => {
          const store = new KVStore('example-test-kv-store');
          await store.delete('');
        },
        TypeError,
        `KVStore key can not be an empty string`,
      );
    });
    routes.set(
      '/kv-store/delete/key-parameter-1024-character-string',
      async () => {
        await assertResolves(async () => {
          const store = new KVStore('example-test-kv-store');
          const key = 'a'.repeat(1024);
          await store.put(key, '');
          await store.delete(key);
        });
      },
    );
    routes.set(
      '/kv-store/delete/key-parameter-1025-character-string',
      async () => {
        await assertRejects(
          async () => {
            const store = new KVStore('example-test-kv-store');
            const key = 'a'.repeat(1025);
            await store.delete(key);
          },
          TypeError,
          `KVStore key can not be more than 1024 characters`,
        );
      },
    );
    routes.set(
      '/kv-store/delete/key-parameter-containing-newline',
      async () => {
        await assertRejects(
          async () => {
            let store = new KVStore('example-test-kv-store');
            await store.delete('\n');
          },
          TypeError,
          `KVStore key can not contain newline character`,
        );
      },
    );
    routes.set(
      '/kv-store/delete/key-parameter-containing-carriage-return',
      async () => {
        await assertRejects(
          async () => {
            let store = new KVStore('example-test-kv-store');
            await store.delete('\r');
          },
          TypeError,
          `KVStore key can not contain carriage return character`,
        );
      },
    );
    routes.set(
      '/kv-store/delete/key-parameter-starting-with-well-known-acme-challenge',
      async () => {
        await assertRejects(
          async () => {
            let store = new KVStore('example-test-kv-store');
            await store.delete('.well-known/acme-challenge/');
          },
          TypeError,
          `KVStore key can not start with .well-known/acme-challenge/`,
        );
      },
    );
    routes.set('/kv-store/delete/key-parameter-single-dot', async () => {
      await assertRejects(
        async () => {
          let store = new KVStore('example-test-kv-store');
          await store.delete('.');
        },
        TypeError,
        `KVStore key can not be '.' or '..'`,
      );
    });
    routes.set('/kv-store/delete/key-parameter-double-dot', async () => {
      await assertRejects(
        async () => {
          let store = new KVStore('example-test-kv-store');
          await store.delete('..');
        },
        TypeError,
        `KVStore key can not be '.' or '..'`,
      );
    });
    routes.set(
      '/kv-store/delete/key-parameter-containing-special-characters',
      async () => {
        const specialCharacters = ['[', ']', '*', '?', '#'];
        for (const character of specialCharacters) {
          await assertRejects(
            async () => {
              let store = new KVStore('example-test-kv-store');
              await store.delete(character);
            },
            TypeError,
            `KVStore key can not contain ${character} character`,
          );
        }
      },
    );
    routes.set(
      '/kv-store/delete/key-does-not-exist-returns-undefined',
      async () => {
        if (isRunningLocally()) {
          return;
        }
        let store = new KVStore('example-test-kv-store');
        await assertRejects(
          () => store.delete(Math.random()),
          TypeError,
          'KVStore delete: Not found.',
        );
      },
    );
    routes.set('/kv-store/delete/key-exists', async () => {
      let store = new KVStore('example-test-kv-store');
      let key = `key-exists-${Math.random()}`;
      await store.put(key, 'hello');
      let result = store.delete(key);
      strictEqual(
        result instanceof Promise,
        true,
        `store.delete(key) instanceof Promise`,
      );
      result = await result;
      strictEqual(result, undefined, `(await store.delete(key) === undefined)`);
    });
    routes.set('/kv-store/delete/delete-key-twice', async () => {
      if (isRunningLocally()) {
        return;
      }
      let store = new KVStore('example-test-kv-store');
      let key = `key-exists-${Math.random()}`;
      await store.put(key, 'hello');
      let result = store.delete(key);
      strictEqual(
        result instanceof Promise,
        true,
        `store.delete(key) instanceof Promise`,
      );
      result = await result;
      strictEqual(result, undefined, `(await store.delete(key) === undefined)`);
      await assertRejects(
        () => store.delete(key),
        TypeError,
        'KVStore delete: Not found.',
      );
    });
    routes.set('/kv-store/delete/multiple-deletes-at-once', async () => {
      let store = new KVStore('example-test-kv-store');
      let key1 = `key-exists-${Math.random()}`;
      await store.put(key1, '1hello1');
      let key2 = `key-exists-${Math.random()}`;
      await store.put(key2, '2hello2');
      let key3 = `key-exists-${Math.random()}`;
      await store.put(key3, '3hello3');
      let key4 = `key-exists-${Math.random()}`;
      await store.put(key4, '4hello4');
      let key5 = `key-exists-${Math.random()}`;
      await store.put(key5, '5hello5');
      await assertResolves(() => {
        return Promise.all([
          store.delete(key1),
          store.delete(key2),
          store.delete(key3),
          store.delete(key4),
          store.delete(key5),
        ]);
      });
    });
  }

  // KVStore get method
  {
    routes.set('/kv-store/get/called-as-constructor', async () => {
      assertThrows(() => {
        new KVStore.prototype.get('1');
      }, TypeError);
    });
    routes.set('/kv-store/get/called-unbound', async () => {
      await assertRejects(async () => {
        await KVStore.prototype.get.call(undefined, '1');
      }, TypeError);
    });
    // https://tc39.es/ecma262/#sec-tostring
    routes.set(
      '/kv-store/get/key-parameter-calls-7.1.17-ToString',
      async () => {
        let sentinel;
        const test = async () => {
          sentinel = Symbol();
          const key = {
            toString() {
              throw sentinel;
            },
          };
          const store = new KVStore('example-test-kv-store');
          await store.get(key);
        };
        await assertRejects(test);
        try {
          await test();
        } catch (thrownError) {
          strictEqual(thrownError, sentinel, 'thrownError === sentinel');
        }
        await assertRejects(
          async () => {
            const store = new KVStore('example-test-kv-store');
            await store.get(Symbol());
          },
          TypeError,
          `can't convert symbol to string`,
        );
      },
    );
    routes.set('/kv-store/get/key-parameter-not-supplied', async () => {
      await assertRejects(
        async () => {
          const store = new KVStore('example-test-kv-store');
          await store.get();
        },
        TypeError,
        `get: At least 1 argument required, but only 0 passed`,
      );
    });
    routes.set('/kv-store/get/key-parameter-empty-string', async () => {
      await assertRejects(
        async () => {
          const store = new KVStore('example-test-kv-store');
          await store.get('');
        },
        TypeError,
        `KVStore key can not be an empty string`,
      );
    });
    routes.set(
      '/kv-store/get/key-parameter-1024-character-string',
      async () => {
        await assertResolves(async () => {
          const store = new KVStore('example-test-kv-store');
          const key = 'a'.repeat(1024);
          await store.get(key);
        });
      },
    );
    routes.set(
      '/kv-store/get/key-parameter-1025-character-string',
      async () => {
        await assertRejects(
          async () => {
            const store = new KVStore('example-test-kv-store');
            const key = 'a'.repeat(1025);
            await store.get(key);
          },
          TypeError,
          `KVStore key can not be more than 1024 characters`,
        );
      },
    );
    routes.set('/kv-store/get/key-parameter-containing-newline', async () => {
      await assertRejects(
        async () => {
          let store = new KVStore('example-test-kv-store');
          await store.get('\n');
        },
        TypeError,
        `KVStore key can not contain newline character`,
      );
    });
    routes.set(
      '/kv-store/get/key-parameter-containing-carriage-return',
      async () => {
        await assertRejects(
          async () => {
            let store = new KVStore('example-test-kv-store');
            await store.get('\r');
          },
          TypeError,
          `KVStore key can not contain carriage return character`,
        );
      },
    );
    routes.set(
      '/kv-store/get/key-parameter-starting-with-well-known-acme-challenge',
      async () => {
        await assertRejects(
          async () => {
            let store = new KVStore('example-test-kv-store');
            await store.get('.well-known/acme-challenge/');
          },
          TypeError,
          `KVStore key can not start with .well-known/acme-challenge/`,
        );
      },
    );
    routes.set('/kv-store/get/key-parameter-single-dot', async () => {
      await assertRejects(
        async () => {
          let store = new KVStore('example-test-kv-store');
          await store.get('.');
        },
        TypeError,
        `KVStore key can not be '.' or '..'`,
      );
    });
    routes.set('/kv-store/get/key-parameter-double-dot', async () => {
      await assertRejects(
        async () => {
          let store = new KVStore('example-test-kv-store');
          await store.get('..');
        },
        TypeError,
        `KVStore key can not be '.' or '..'`,
      );
    });
    routes.set(
      '/kv-store/get/key-parameter-containing-special-characters',
      async () => {
        const specialCharacters = ['[', ']', '*', '?', '#'];
        for (const character of specialCharacters) {
          await assertRejects(
            async () => {
              let store = new KVStore('example-test-kv-store');
              await store.get(character);
            },
            TypeError,
            `KVStore key can not contain ${character} character`,
          );
        }
      },
    );
    routes.set('/kv-store/get/key-does-not-exist-returns-null', async () => {
      let store = new KVStore('example-test-kv-store');
      let result = store.get(Math.random());
      strictEqual(
        result instanceof Promise,
        true,
        `store.get(Math.random()) instanceof Promise`,
      );
      strictEqual(await result, null, `await store.get(Math.random())`);
    });
    routes.set('/kv-store/get/key-does-not-exist-returns-null', async () => {
      let store = new KVStore('example-test-kv-store');
      let result = store.get(Math.random());
      strictEqual(
        result instanceof Promise,
        true,
        `store.get(Math.random()) instanceof Promise`,
      );
      strictEqual(await result, null, `await store.get(Math.random())`);
    });
    routes.set('/kv-store/get/key-exists', async () => {
      let store = new KVStore('example-test-kv-store');
      let key = `key-exists-${Math.random()}`;
      await store.put(key, 'hello');
      let result = store.get(key);
      strictEqual(
        result instanceof Promise,
        true,
        `store.get(key) instanceof Promise`,
      );
      result = await result;
      strictEqual(
        result instanceof KVStoreEntry,
        true,
        `(await store.get(key) instanceof KVStoreEntry)`,
      );
    });

    routes.set('/kv-store/get/multiple-lookups-at-once', async () => {
      let store = new KVStore('example-test-kv-store');
      let key1 = `key-exists-${Math.random()}`;
      await store.put(key1, '1hello1');
      let key2 = `key-exists-${Math.random()}`;
      await store.put(key2, '2hello2');
      let key3 = `key-exists-${Math.random()}`;
      await store.put(key3, '3hello3');
      let key4 = `key-exists-${Math.random()}`;
      await store.put(key4, '4hello4');
      let key5 = `key-exists-${Math.random()}`;
      await store.put(key5, '5hello5');
      let results = await Promise.all([
        store.get(key1),
        store.get(key2),
        store.get(key3),
        store.get(key4),
        store.get(key5),
      ]);
      strictEqual(
        await results[0].text(),
        '1hello1',
        `await results[0].text()`,
      );
      strictEqual(
        await results[1].text(),
        '2hello2',
        `await results[1].text()`,
      );
      strictEqual(
        await results[2].text(),
        '3hello3',
        `await results[2].text()`,
      );
      strictEqual(
        await results[3].text(),
        '4hello4',
        `await results[3].text()`,
      );
      strictEqual(
        await results[4].text(),
        '5hello5',
        `await results[4].text()`,
      );
    });
  }
}

// KVStoreEntry
{
  routes.set('/kv-store-entry/interface', async () => {
    return kvStoreEntryInterfaceTests();
  });
  routes.set('/kv-store-entry/text/valid', async () => {
    let store = new KVStore('example-test-kv-store');
    let key = `entry-text-valid`;
    await store.put(key, 'hello');
    let entry = await store.get(key);
    let result = entry.text();
    strictEqual(
      result instanceof Promise,
      true,
      `entry.text() instanceof Promise`,
    );
    result = await result;
    strictEqual(result, 'hello', `await entry.text())`);
  });
  routes.set('/kv-store-entry/json/valid', async () => {
    let store = new KVStore('example-test-kv-store');
    let key = `entry-json-valid`;
    const obj = { a: 1, b: 2, c: 3 };
    await store.put(key, JSON.stringify(obj));
    let entry = await store.get(key);
    let result = entry.json();
    strictEqual(
      result instanceof Promise,
      true,
      `entry.json() instanceof Promise`,
    );
    result = await result;
    strictEqual(result, obj, `await entry.json())`);
  });
  routes.set('/kv-store-entry/json/invalid', async () => {
    let store = new KVStore('example-test-kv-store');
    let key = `entry-json-invalid`;
    await store.put(key, "132abc;['-=9");
    let entry = await store.get(key);
    await assertRejects(
      () => entry.json(),
      SyntaxError,
      `JSON.parse: unexpected non-whitespace character after JSON data at line 1 column 4 of the JSON data`,
    );
  });
  routes.set('/kv-store-entry/arrayBuffer/valid', async () => {
    let store = new KVStore('example-test-kv-store');
    let key = `entry-arraybuffer-valid`;
    await store.put(key, new Int8Array([0, 1, 2, 3]));
    let entry = await store.get(key);
    let result = entry.arrayBuffer();
    strictEqual(
      result instanceof Promise,
      true,
      `entry.arrayBuffer() instanceof Promise`,
    );
    result = await result;
    strictEqual(
      result instanceof ArrayBuffer,
      true,
      `(await entry.arrayBuffer()) instanceof ArrayBuffer`,
    );
  });

  routes.set('/kv-store-entry/body', async () => {
    let store = new KVStore('example-test-kv-store');
    let key = `entry-body`;
    await store.put(key, 'body body body');
    let entry = await store.get(key);
    let result = entry.body;
    strictEqual(
      result instanceof ReadableStream,
      true,
      `entry.body instanceof ReadableStream`,
    );
    let text = await streamToString(result);
    strictEqual(text, 'body body body', `entry.body contents as string`);
  });
  routes.set('/kv-store-entry/bodyUsed', async () => {
    let store = new KVStore('example-test-kv-store');
    let key = `entry-bodyUsed`;
    await store.put(key, 'body body body');
    let entry = await store.get(key);
    strictEqual(entry.bodyUsed, false, `entry.bodyUsed`);
    await entry.text();
    strictEqual(entry.bodyUsed, true, `entry.bodyUsed`);
  });
}
async function kvStoreEntryInterfaceTests() {
  let actual = Reflect.ownKeys(KVStoreEntry);
  let expected = ['prototype', 'length', 'name'];
  strictEqual(actual, expected, `Reflect.ownKeys(KVStoreEntry)`);

  actual = Reflect.getOwnPropertyDescriptor(KVStoreEntry, 'prototype');
  expected = {
    value: KVStoreEntry.prototype,
    writable: false,
    enumerable: false,
    configurable: false,
  };
  strictEqual(
    actual,
    expected,
    `Reflect.getOwnPropertyDescriptor(KVStoreEntry, 'prototype')`,
  );

  actual = Reflect.getOwnPropertyDescriptor(KVStoreEntry, 'length');
  expected = {
    value: 0,
    writable: false,
    enumerable: false,
    configurable: true,
  };
  strictEqual(
    actual,
    expected,
    `Reflect.getOwnPropertyDescriptor(KVStoreEntry, 'length')`,
  );

  actual = Reflect.getOwnPropertyDescriptor(KVStoreEntry, 'name');
  expected = {
    value: 'KVStoreEntry',
    writable: false,
    enumerable: false,
    configurable: true,
  };
  strictEqual(
    actual,
    expected,
    `Reflect.getOwnPropertyDescriptor(KVStoreEntry, 'name')`,
  );

  actual = Reflect.ownKeys(KVStoreEntry.prototype);
  expected = [
    'constructor',
    'body',
    'bodyUsed',
    'arrayBuffer',
    'json',
    'text',
    'metadata',
    'metadataText',
  ];
  strictEqual(actual, expected, `Reflect.ownKeys(KVStoreEntry.prototype)`);

  actual = Reflect.getOwnPropertyDescriptor(
    KVStoreEntry.prototype,
    'constructor',
  );
  expected = {
    writable: true,
    enumerable: false,
    configurable: true,
    value: KVStoreEntry.prototype.constructor,
  };
  strictEqual(
    actual,
    expected,
    `Reflect.getOwnPropertyDescriptor(KVStoreEntry.prototype, 'constructor')`,
  );

  actual = Reflect.getOwnPropertyDescriptor(KVStoreEntry.prototype, 'text');
  expected = {
    writable: true,
    enumerable: true,
    configurable: true,
    value: KVStoreEntry.prototype.text,
  };
  strictEqual(
    actual,
    expected,
    `Reflect.getOwnPropertyDescriptor(KVStoreEntry.prototype, 'text')`,
  );

  actual = Reflect.getOwnPropertyDescriptor(KVStoreEntry.prototype, 'json');
  expected = {
    writable: true,
    enumerable: true,
    configurable: true,
    value: KVStoreEntry.prototype.json,
  };
  strictEqual(
    actual,
    expected,
    `Reflect.getOwnPropertyDescriptor(KVStoreEntry.prototype, 'json')`,
  );

  actual = Reflect.getOwnPropertyDescriptor(
    KVStoreEntry.prototype,
    'arrayBuffer',
  );
  expected = {
    writable: true,
    enumerable: true,
    configurable: true,
    value: KVStoreEntry.prototype.arrayBuffer,
  };
  strictEqual(
    actual,
    expected,
    `Reflect.getOwnPropertyDescriptor(KVStoreEntry.prototype, 'arrayBuffer')`,
  );
  actual = Reflect.getOwnPropertyDescriptor(KVStoreEntry.prototype, 'body');
  strictEqual(
    actual.enumerable,
    true,
    `Reflect.getOwnPropertyDescriptor(KVStoreEntry.prototype, 'body').enumerable`,
  );
  strictEqual(
    actual.configurable,
    true,
    `Reflect.getOwnPropertyDescriptor(KVStoreEntry.prototype, 'body').configurable`,
  );
  strictEqual(
    'set' in actual,
    true,
    `'set' in Reflect.getOwnPropertyDescriptor(KVStoreEntry.prototype, 'body')`,
  );
  strictEqual(
    actual.set,
    undefined,
    `Reflect.getOwnPropertyDescriptor(KVStoreEntry.prototype, 'body').set`,
  );
  strictEqual(
    typeof actual.get,
    'function',
    `typeof Reflect.getOwnPropertyDescriptor(KVStoreEntry.prototype, 'body').get`,
  );
  actual = Reflect.getOwnPropertyDescriptor(KVStoreEntry.prototype, 'bodyUsed');
  strictEqual(
    actual.enumerable,
    true,
    `Reflect.getOwnPropertyDescriptor(KVStoreEntry.prototype, 'bodyUsed').enumerable`,
  );
  strictEqual(
    actual.configurable,
    true,
    `Reflect.getOwnPropertyDescriptor(KVStoreEntry.prototype, 'bodyUsed').configurable`,
  );
  strictEqual(
    'set' in actual,
    true,
    `'set' in Reflect.getOwnPropertyDescriptor(KVStoreEntry.prototype, 'bodyUsed')`,
  );
  strictEqual(
    actual.set,
    undefined,
    `Reflect.getOwnPropertyDescriptor(KVStoreEntry.prototype, 'bodyUsed').set`,
  );
  strictEqual(
    typeof actual.get,
    'function',
    `typeof Reflect.getOwnPropertyDescriptor(KVStoreEntry.prototype, 'bodyUsed').get`,
  );

  strictEqual(
    typeof KVStoreEntry.prototype.constructor,
    'function',
    `typeof KVStoreEntry.prototype.constructor`,
  );
  strictEqual(
    typeof KVStoreEntry.prototype.text,
    'function',
    `typeof KVStoreEntry.prototype.text`,
  );
  strictEqual(
    typeof KVStoreEntry.prototype.json,
    'function',
    `typeof KVStoreEntry.prototype.json`,
  );
  strictEqual(
    typeof KVStoreEntry.prototype.arrayBuffer,
    'function',
    `typeof KVStoreEntry.prototype.arrayBuffer`,
  );

  actual = Reflect.getOwnPropertyDescriptor(
    KVStoreEntry.prototype.constructor,
    'length',
  );
  expected = {
    value: 0,
    writable: false,
    enumerable: false,
    configurable: true,
  };
  strictEqual(
    actual,
    expected,
    `Reflect.getOwnPropertyDescriptor(KVStoreEntry.prototype.constructor, 'length')`,
  );

  actual = Reflect.getOwnPropertyDescriptor(
    KVStoreEntry.prototype.constructor,
    'name',
  );
  expected = {
    value: 'KVStoreEntry',
    writable: false,
    enumerable: false,
    configurable: true,
  };
  strictEqual(
    actual,
    expected,
    `Reflect.getOwnPropertyDescriptor(KVStoreEntry.prototype.constructor, 'name')`,
  );

  actual = Reflect.getOwnPropertyDescriptor(
    KVStoreEntry.prototype.text,
    'length',
  );
  expected = {
    value: 0,
    writable: false,
    enumerable: false,
    configurable: true,
  };
  strictEqual(
    actual,
    expected,
    `Reflect.getOwnPropertyDescriptor(KVStoreEntry.prototype.text, 'length')`,
  );

  actual = Reflect.getOwnPropertyDescriptor(
    KVStoreEntry.prototype.text,
    'name',
  );
  expected = {
    value: 'text',
    writable: false,
    enumerable: false,
    configurable: true,
  };
  strictEqual(
    actual,
    expected,
    `Reflect.getOwnPropertyDescriptor(KVStoreEntry.prototype.text, 'name')`,
  );

  actual = Reflect.getOwnPropertyDescriptor(
    KVStoreEntry.prototype.json,
    'length',
  );
  expected = {
    value: 0,
    writable: false,
    enumerable: false,
    configurable: true,
  };
  strictEqual(
    actual,
    expected,
    `Reflect.getOwnPropertyDescriptor(KVStoreEntry.prototype.json, 'length')`,
  );

  actual = Reflect.getOwnPropertyDescriptor(
    KVStoreEntry.prototype.json,
    'name',
  );
  expected = {
    value: 'json',
    writable: false,
    enumerable: false,
    configurable: true,
  };
  strictEqual(
    actual,
    expected,
    `Reflect.getOwnPropertyDescriptor(KVStoreEntry.prototype.json, 'name')`,
  );

  actual = Reflect.getOwnPropertyDescriptor(
    KVStoreEntry.prototype.arrayBuffer,
    'length',
  );
  expected = {
    value: 0,
    writable: false,
    enumerable: false,
    configurable: true,
  };
  strictEqual(
    actual,
    expected,
    `Reflect.getOwnPropertyDescriptor(KVStoreEntry.prototype.arrayBuffer, 'length')`,
  );

  actual = Reflect.getOwnPropertyDescriptor(
    KVStoreEntry.prototype.arrayBuffer,
    'name',
  );
  expected = {
    value: 'arrayBuffer',
    writable: false,
    enumerable: false,
    configurable: true,
  };
  strictEqual(
    actual,
    expected,
    `Reflect.getOwnPropertyDescriptor(KVStoreEntry.prototype.arrayBuffer, 'name')`,
  );
}

async function kvStoreInterfaceTests() {
  let actual = Reflect.ownKeys(KVStore);
  let expected = ['prototype', 'length', 'name'];
  strictEqual(actual, expected, `Reflect.ownKeys(KVStore)`);

  actual = Reflect.getOwnPropertyDescriptor(KVStore, 'prototype');
  expected = {
    value: KVStore.prototype,
    writable: false,
    enumerable: false,
    configurable: false,
  };
  strictEqual(
    actual,
    expected,
    `Reflect.getOwnPropertyDescriptor(KVStore, 'prototype')`,
  );

  actual = Reflect.getOwnPropertyDescriptor(KVStore, 'length');
  expected = {
    value: 1,
    writable: false,
    enumerable: false,
    configurable: true,
  };
  strictEqual(
    actual,
    expected,
    `Reflect.getOwnPropertyDescriptor(KVStore, 'length')`,
  );

  actual = Reflect.getOwnPropertyDescriptor(KVStore, 'name');
  expected = {
    value: 'KVStore',
    writable: false,
    enumerable: false,
    configurable: true,
  };
  strictEqual(
    actual,
    expected,
    `Reflect.getOwnPropertyDescriptor(KVStore, 'name')`,
  );

  actual = Reflect.ownKeys(KVStore.prototype);
  expected = ['constructor', 'delete', 'get', 'put', 'list'];
  strictEqual(actual, expected, `Reflect.ownKeys(KVStore.prototype)`);

  actual = Reflect.getOwnPropertyDescriptor(KVStore.prototype, 'constructor');
  expected = {
    writable: true,
    enumerable: false,
    configurable: true,
    value: KVStore.prototype.constructor,
  };
  strictEqual(
    actual,
    expected,
    `Reflect.getOwnPropertyDescriptor(KVStore.prototype, 'constructor')`,
  );

  actual = Reflect.getOwnPropertyDescriptor(KVStore.prototype, 'delete');
  expected = {
    writable: true,
    enumerable: true,
    configurable: true,
    value: KVStore.prototype.delete,
  };
  strictEqual(
    actual,
    expected,
    `Reflect.getOwnPropertyDescriptor(KVStore.prototype, 'delete')`,
  );

  actual = Reflect.getOwnPropertyDescriptor(KVStore.prototype, 'get');
  expected = {
    writable: true,
    enumerable: true,
    configurable: true,
    value: KVStore.prototype.get,
  };
  strictEqual(
    actual,
    expected,
    `Reflect.getOwnPropertyDescriptor(KVStore.prototype, 'get')`,
  );

  actual = Reflect.getOwnPropertyDescriptor(KVStore.prototype, 'put');
  expected = {
    writable: true,
    enumerable: true,
    configurable: true,
    value: KVStore.prototype.put,
  };
  strictEqual(
    actual,
    expected,
    `Reflect.getOwnPropertyDescriptor(KVStore.prototype, 'put')`,
  );

  strictEqual(
    typeof KVStore.prototype.constructor,
    'function',
    `typeof KVStore.prototype.constructor`,
  );
  strictEqual(
    typeof KVStore.prototype.delete,
    'function',
    `typeof KVStore.prototype.delete`,
  );
  strictEqual(
    typeof KVStore.prototype.get,
    'function',
    `typeof KVStore.prototype.get`,
  );
  strictEqual(
    typeof KVStore.prototype.put,
    'function',
    `typeof KVStore.prototype.put`,
  );

  actual = Reflect.getOwnPropertyDescriptor(
    KVStore.prototype.constructor,
    'length',
  );
  expected = {
    value: 1,
    writable: false,
    enumerable: false,
    configurable: true,
  };
  strictEqual(
    actual,
    expected,
    `Reflect.getOwnPropertyDescriptor(KVStore.prototype.constructor, 'length')`,
  );

  actual = Reflect.getOwnPropertyDescriptor(
    KVStore.prototype.constructor,
    'name',
  );
  expected = {
    value: 'KVStore',
    writable: false,
    enumerable: false,
    configurable: true,
  };
  strictEqual(
    actual,
    expected,
    `Reflect.getOwnPropertyDescriptor(KVStore.prototype.constructor, 'name')`,
  );

  actual = Reflect.getOwnPropertyDescriptor(KVStore.prototype.delete, 'length');
  expected = {
    value: 1,
    writable: false,
    enumerable: false,
    configurable: true,
  };
  strictEqual(
    actual,
    expected,
    `Reflect.getOwnPropertyDescriptor(KVStore.prototype.delete, 'length')`,
  );

  actual = Reflect.getOwnPropertyDescriptor(KVStore.prototype.delete, 'name');
  expected = {
    value: 'delete',
    writable: false,
    enumerable: false,
    configurable: true,
  };
  strictEqual(
    actual,
    expected,
    `Reflect.getOwnPropertyDescriptor(KVStore.prototype.delete, 'name')`,
  );

  actual = Reflect.getOwnPropertyDescriptor(KVStore.prototype.get, 'length');
  expected = {
    value: 1,
    writable: false,
    enumerable: false,
    configurable: true,
  };
  strictEqual(
    actual,
    expected,
    `Reflect.getOwnPropertyDescriptor(KVStore.prototype.get, 'length')`,
  );

  actual = Reflect.getOwnPropertyDescriptor(KVStore.prototype.get, 'name');
  expected = {
    value: 'get',
    writable: false,
    enumerable: false,
    configurable: true,
  };
  strictEqual(
    actual,
    expected,
    `Reflect.getOwnPropertyDescriptor(KVStore.prototype.get, 'name')`,
  );

  actual = Reflect.getOwnPropertyDescriptor(KVStore.prototype.put, 'length');
  expected = {
    value: 1,
    writable: false,
    enumerable: false,
    configurable: true,
  };
  strictEqual(
    actual,
    expected,
    `Reflect.getOwnPropertyDescriptor(KVStore.prototype.put, 'length')`,
  );

  actual = Reflect.getOwnPropertyDescriptor(KVStore.prototype.put, 'name');
  expected = {
    value: 'put',
    writable: false,
    enumerable: false,
    configurable: true,
  };
  strictEqual(
    actual,
    expected,
    `Reflect.getOwnPropertyDescriptor(KVStore.prototype.put, 'name')`,
  );
}

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

// TODO: Implement ReadableStream getIterator() and [@@asyncIterator]() methods
async function streamToString(stream) {
  const decoder = new TextDecoder();
  let string = '';
  let reader = stream.getReader();
  // eslint-disable-next-line no-constant-condition
  while (true) {
    const { done, value } = await reader.read();
    if (done) {
      return string;
    }
    string += decoder.decode(value);
  }
}