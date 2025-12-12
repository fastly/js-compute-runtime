/* eslint-env serviceworker */
import { assert, assertDoesNotThrow, assertThrows } from './assertions.js';
import { routes } from './routes.js';
import { CacheOverride } from 'fastly:cache-override';

// setInterval
{
  routes.set('/setInterval/exposed-as-global', async () => {
    assert(typeof setInterval, 'function', `typeof setInterval`);
  });
  routes.set('/setInterval/interface', async () => {
    let actual = Reflect.getOwnPropertyDescriptor(globalThis, 'setInterval');
    let expected = {
      writable: true,
      enumerable: true,
      configurable: true,
      value: globalThis.setInterval,
    };
    assert(
      actual,
      expected,
      `Reflect.getOwnPropertyDescriptor(globalThis, 'setInterval)`,
    );

    assert(
      typeof globalThis.setInterval,
      'function',
      `typeof globalThis.setInterval`,
    );

    actual = Reflect.getOwnPropertyDescriptor(globalThis.setInterval, 'length');
    expected = {
      value: 1,
      writable: false,
      enumerable: false,
      configurable: true,
    };
    assert(
      actual,
      expected,
      `Reflect.getOwnPropertyDescriptor(globalThis.setInterval, 'length')`,
    );

    actual = Reflect.getOwnPropertyDescriptor(globalThis.setInterval, 'name');
    expected = {
      value: 'setInterval',
      writable: false,
      enumerable: false,
      configurable: true,
    };
    assert(
      actual,
      expected,
      `Reflect.getOwnPropertyDescriptor(globalThis.setInterval, 'name')`,
    );
  });
  routes.set('/setInterval/called-as-constructor-function', async () => {
    assertThrows(
      () => {
        new setInterval();
      },
      TypeError,
      `setInterval is not a constructor`,
    );
  });
  routes.set('/setInterval/empty-parameter', async () => {
    assertThrows(
      () => {
        setInterval();
      },
      TypeError,
      `setInterval: At least 1 argument required, but only 0 passed`,
    );
  });
  routes.set('/setInterval/handler-parameter-not-supplied', async () => {
    assertThrows(
      () => {
        setInterval();
      },
      TypeError,
      `setInterval: At least 1 argument required, but only 0 passed`,
    );
  });
  routes.set('/setInterval/handler-parameter-not-callable', async () => {
    let non_callable_types = [
      // Primitive types
      null,
      undefined,
      true,
      1,
      1n,
      'hello',
      Symbol(),
      // After primitive types, the only remaining types are Objects and Functions
      {},
    ];
    for (const type of non_callable_types) {
      assertThrows(() => {
        setInterval(type);
      }, Error);
    }
  });
  routes.set('/setInterval/timeout-parameter-not-supplied', async () => {
    assertDoesNotThrow(() => {
      setInterval(function () { });
    });
  });
  // https://tc39.es/ecma262/#sec-tonumber
  routes.set(
    '/setInterval/timeout-parameter-calls-7.1.4-ToNumber',
    async () => {
      let sentinel;
      let requestedType;
      const test = () => {
        sentinel = Symbol();
        const timeout = {
          [Symbol.toPrimitive](type) {
            requestedType = type;
            throw sentinel;
          },
        };
        setInterval(function () { }, timeout);
      };
      assertThrows(test);
      try {
        test();
      } catch (thrownError) {
        assert(thrownError, sentinel, 'thrownError === sentinel');
        assert(requestedType, 'number', 'requestedType === "number"');
      }
      assertThrows(
        () => setInterval(function () { }, Symbol()),
        TypeError,
        `can't convert symbol to number`,
      );
    },
  );

  routes.set('/setInterval/timeout-parameter-negative', async () => {
    assertDoesNotThrow(() => setInterval(() => { }, -1));
    assertDoesNotThrow(() => setInterval(() => { }, -1.1));
    assertDoesNotThrow(() => setInterval(() => { }, Number.MIN_SAFE_INTEGER));
    assertDoesNotThrow(() => setInterval(() => { }, Number.MIN_VALUE));
    assertDoesNotThrow(() => setInterval(() => { }, -Infinity));
  });
  routes.set('/setInterval/timeout-parameter-positive', async () => {
    assertDoesNotThrow(() => setInterval(() => { }, 1));
    assertDoesNotThrow(() => setInterval(() => { }, 1.1));
    assertDoesNotThrow(() => setInterval(() => { }, Number.MAX_SAFE_INTEGER));
    assertDoesNotThrow(() => setInterval(() => { }, Number.MAX_VALUE));
    assertDoesNotThrow(() => setInterval(() => { }, Infinity));
  });
  routes.set('/setInterval/returns-integer', async () => {
    let id = setInterval(() => { }, 1);
    assert(typeof id, 'number', `typeof id === "number"`);
  });
  routes.set('/setInterval/called-unbound', async () => {
    assertDoesNotThrow(() => {
      setInterval.call(undefined, () => { }, 1);
    });
  });
}

// setTimeout
{
  routes.set('/setTimeout/exposed-as-global', async () => {
    assert(typeof setTimeout, 'function', `typeof setTimeout`);
  });
  routes.set('/setTimeout/interface', async () => {
    let actual = Reflect.getOwnPropertyDescriptor(globalThis, 'setTimeout');
    let expected = {
      writable: true,
      enumerable: true,
      configurable: true,
      value: globalThis.setTimeout,
    };
    assert(
      actual,
      expected,
      `Reflect.getOwnPropertyDescriptor(globalThis, 'setTimeout)`,
    );

    assert(
      typeof globalThis.setTimeout,
      'function',
      `typeof globalThis.setTimeout`,
    );

    actual = Reflect.getOwnPropertyDescriptor(globalThis.setTimeout, 'length');
    expected = {
      value: 1,
      writable: false,
      enumerable: false,
      configurable: true,
    };
    assert(
      actual,
      expected,
      `Reflect.getOwnPropertyDescriptor(globalThis.setTimeout, 'length')`,
    );

    actual = Reflect.getOwnPropertyDescriptor(globalThis.setTimeout, 'name');
    expected = {
      value: 'setTimeout',
      writable: false,
      enumerable: false,
      configurable: true,
    };
    assert(
      actual,
      expected,
      `Reflect.getOwnPropertyDescriptor(globalThis.setTimeout, 'name')`,
    );
  });
  routes.set('/setTimeout/called-as-constructor-function', async () => {
    assertThrows(
      () => {
        new setTimeout();
      },
      TypeError,
      `setTimeout is not a constructor`,
    );
  });
  routes.set('/setTimeout/empty-parameter', async () => {
    assertThrows(
      () => {
        setTimeout();
      },
      TypeError,
      `setTimeout: At least 1 argument required, but only 0 passed`,
    );
  });
  routes.set('/setTimeout/handler-parameter-not-supplied', async () => {
    assertThrows(
      () => {
        setTimeout();
      },
      TypeError,
      `setTimeout: At least 1 argument required, but only 0 passed`,
    );
  });
  routes.set('/setTimeout/handler-parameter-not-callable', async () => {
    let non_callable_types = [
      // Primitive types
      null,
      undefined,
      true,
      1,
      1n,
      'hello',
      Symbol(),
      // After primitive types, the only remaining types are Objects and Functions
      {},
    ];
    for (const type of non_callable_types) {
      assertThrows(() => {
        setTimeout(type);
      }, Error);
    }
  });
  routes.set('/setTimeout/timeout-parameter-not-supplied', async () => {
    assertDoesNotThrow(() => {
      setTimeout(function () { });
    });
  });
  // https://tc39.es/ecma262/#sec-tonumber
  routes.set('/setTimeout/timeout-parameter-calls-7.1.4-ToNumber', async () => {
    let sentinel;
    let requestedType;
    const test = () => {
      sentinel = Symbol();
      const timeout = {
        [Symbol.toPrimitive](type) {
          requestedType = type;
          throw sentinel;
        },
      };
      setTimeout(function () { }, timeout);
    };
    assertThrows(test);
    try {
      test();
    } catch (thrownError) {
      assert(thrownError, sentinel, 'thrownError === sentinel');
      assert(requestedType, 'number', 'requestedType === "number"');
    }
    assertThrows(
      () => setTimeout(function () { }, Symbol()),
      TypeError,
      `can't convert symbol to number`,
    );
  });

  routes.set('/setTimeout/timeout-parameter-negative', async () => {
    assertDoesNotThrow(() => setTimeout(() => { }, -1));
    assertDoesNotThrow(() => setTimeout(() => { }, -1.1));
    assertDoesNotThrow(() => setTimeout(() => { }, Number.MIN_SAFE_INTEGER));
    assertDoesNotThrow(() => setTimeout(() => { }, Number.MIN_VALUE));
    assertDoesNotThrow(() => setTimeout(() => { }, -Infinity));
  });
  routes.set('/setTimeout/timeout-parameter-positive', async () => {
    assertDoesNotThrow(() => setTimeout(() => { }, 1));
    assertDoesNotThrow(() => setTimeout(() => { }, 1.1));
    assertDoesNotThrow(() => setTimeout(() => { }, Number.MAX_SAFE_INTEGER));
    assertDoesNotThrow(() => setTimeout(() => { }, Number.MAX_VALUE));
    assertDoesNotThrow(() => setTimeout(() => { }, Infinity));
  });
  routes.set('/setTimeout/returns-integer', async () => {
    let id = setTimeout(() => { }, 1);
    assert(typeof id, 'number', `typeof id === "number"`);
  });
  routes.set('/setTimeout/called-unbound', async () => {
    assertDoesNotThrow(() => {
      setTimeout.call(undefined, () => { }, 1);
    });
  });
  routes.set('/setTimeout/200-ms', async () => {
    let controller, start;
    setTimeout(() => {
      const end = Date.now();
      controller.enqueue(new TextEncoder().encode(`END\n`));
      if (end - start < 190) {
        controller.enqueue(
          new TextEncoder().encode(
            `ERROR: Timer took ${end - start} instead of 200ms`,
          ),
        );
      }
      controller.close();
    }, 200);
    return new Response(
      new ReadableStream({
        start(_controller) {
          controller = _controller;
          start = Date.now();
          controller.enqueue(new TextEncoder().encode(`START\n`));
        },
      }),
    );
  });
  routes.set('/setTimeout/fetch-timeout', async () => {
    let timedOut = false;
    const first = fetch('https://http-me.fastly.dev/wait=200', {
      backend: 'httpme',
      cacheOverride: new CacheOverride('pass'),
    });
    const second = Promise.race([
      fetch('https://http-me.fastly.dev/wait=200', {
        backend: 'httpme',
        cacheOverride: new CacheOverride('pass'),
      }),
      new Promise((resolve) => setTimeout(resolve, 5)).then(() => {
        timedOut = true;
        return { status: 504, errors: 'timeout' };
      }),
    ]);
    const firstValue = await first;
    assert(timedOut, true, 'should have timed out');
    assert(firstValue.status, 200, 'should get first value');
    const secondValue = await second;
    assert(secondValue.status, 504, 'should get second value timeout');
  });
}

// clearInterval
{
  routes.set('/clearInterval/exposed-as-global', async () => {
    assert(typeof clearInterval, 'function', `typeof clearInterval`);
  });
  routes.set('/clearInterval/interface', async () => {
    let actual = Reflect.getOwnPropertyDescriptor(globalThis, 'clearInterval');
    let expected = {
      writable: true,
      enumerable: true,
      configurable: true,
      value: globalThis.clearInterval,
    };
    assert(
      actual,
      expected,
      `Reflect.getOwnPropertyDescriptor(globalThis, 'clearInterval)`,
    );

    assert(
      typeof globalThis.clearInterval,
      'function',
      `typeof globalThis.clearInterval`,
    );

    actual = Reflect.getOwnPropertyDescriptor(
      globalThis.clearInterval,
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
      `Reflect.getOwnPropertyDescriptor(globalThis.clearInterval, 'length')`,
    );

    actual = Reflect.getOwnPropertyDescriptor(globalThis.clearInterval, 'name');
    expected = {
      value: 'clearInterval',
      writable: false,
      enumerable: false,
      configurable: true,
    };
    assert(
      actual,
      expected,
      `Reflect.getOwnPropertyDescriptor(globalThis.clearInterval, 'name')`,
    );
  });
  routes.set('/clearInterval/called-as-constructor-function', async () => {
    assertThrows(
      () => {
        new clearInterval();
      },
      TypeError,
      `clearInterval is not a constructor`,
    );
  });
  routes.set('/clearInterval/id-parameter-not-supplied', async () => {
    assertThrows(
      () => {
        clearInterval();
      },
      TypeError,
      `clearInterval: At least 1 argument required, but only 0 passed`,
    );
  });
  // https://tc39.es/ecma262/#sec-tonumber
  routes.set('/clearInterval/id-parameter-calls-7.1.4-ToNumber', async () => {
    let sentinel;
    let requestedType;
    const test = () => {
      sentinel = Symbol();
      const timeout = {
        [Symbol.toPrimitive](type) {
          requestedType = type;
          throw sentinel;
        },
      };
      clearInterval(timeout);
    };
    assertThrows(test);
    try {
      test();
    } catch (thrownError) {
      assert(thrownError, sentinel, 'thrownError === sentinel');
      assert(requestedType, 'number', 'requestedType === "number"');
    }
    assertThrows(
      () => clearInterval(Symbol()),
      TypeError,
      `can't convert symbol to number`,
    );
  });

  routes.set('/clearInterval/id-parameter-negative', async () => {
    assertDoesNotThrow(() => clearInterval(-1));
    assertDoesNotThrow(() => clearInterval(-1.1));
    assertDoesNotThrow(() => clearInterval(Number.MIN_SAFE_INTEGER));
    assertDoesNotThrow(() => clearInterval(Number.MIN_VALUE));
    assertDoesNotThrow(() => clearInterval(-Infinity));
  });
  routes.set('/clearInterval/id-parameter-positive', async () => {
    assertDoesNotThrow(() => clearInterval(1));
    assertDoesNotThrow(() => clearInterval(1.1));
    assertDoesNotThrow(() => clearInterval(Number.MAX_SAFE_INTEGER));
    assertDoesNotThrow(() => clearInterval(Number.MAX_VALUE));
    assertDoesNotThrow(() => clearInterval(Infinity));
  });
  routes.set('/clearInterval/returns-undefined', async () => {
    let result = clearInterval(1);
    assert(typeof result, 'undefined', `typeof result === "undefined"`);
  });
  routes.set('/clearInterval/called-unbound', async () => {
    assertDoesNotThrow(() => {
      clearInterval.call(undefined, 1);
    });
  });
}

// clearTimeout
{
  routes.set('/clearTimeout/exposed-as-global', async () => {
    assert(typeof clearTimeout, 'function', `typeof clearTimeout`);
  });
  routes.set('/clearTimeout/interface', async () => {
    let actual = Reflect.getOwnPropertyDescriptor(globalThis, 'clearTimeout');
    let expected = {
      writable: true,
      enumerable: true,
      configurable: true,
      value: globalThis.clearTimeout,
    };
    assert(
      actual,
      expected,
      `Reflect.getOwnPropertyDescriptor(globalThis, 'clearTimeout)`,
    );

    assert(
      typeof globalThis.clearTimeout,
      'function',
      `typeof globalThis.clearTimeout`,
    );

    actual = Reflect.getOwnPropertyDescriptor(
      globalThis.clearTimeout,
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
      `Reflect.getOwnPropertyDescriptor(globalThis.clearTimeout, 'length')`,
    );

    actual = Reflect.getOwnPropertyDescriptor(globalThis.clearTimeout, 'name');
    expected = {
      value: 'clearTimeout',
      writable: false,
      enumerable: false,
      configurable: true,
    };
    assert(
      actual,
      expected,
      `Reflect.getOwnPropertyDescriptor(globalThis.clearTimeout, 'name')`,
    );
  });
  routes.set('/clearTimeout/called-as-constructor-function', async () => {
    assertThrows(
      () => {
        new clearTimeout();
      },
      TypeError,
      `clearTimeout is not a constructor`,
    );
  });
  routes.set('/clearTimeout/id-parameter-not-supplied', async () => {
    assertThrows(
      () => {
        clearTimeout();
      },
      TypeError,
      `clearTimeout: At least 1 argument required, but only 0 passed`,
    );
  });
  // https://tc39.es/ecma262/#sec-tonumber
  routes.set('/clearTimeout/id-parameter-calls-7.1.4-ToNumber', async () => {
    let sentinel;
    let requestedType;
    const test = () => {
      sentinel = Symbol();
      const timeout = {
        [Symbol.toPrimitive](type) {
          requestedType = type;
          throw sentinel;
        },
      };
      clearTimeout(timeout);
    };
    assertThrows(test);
    try {
      test();
    } catch (thrownError) {
      assert(thrownError, sentinel, 'thrownError === sentinel');
      assert(requestedType, 'number', 'requestedType === "number"');
    }
    assertThrows(
      () => clearTimeout(Symbol()),
      TypeError,
      `can't convert symbol to number`,
    );
  });

  routes.set('/clearTimeout/id-parameter-negative', async () => {
    assertDoesNotThrow(() => clearTimeout(-1));
    assertDoesNotThrow(() => clearTimeout(-1.1));
    assertDoesNotThrow(() => clearTimeout(Number.MIN_SAFE_INTEGER));
    assertDoesNotThrow(() => clearTimeout(Number.MIN_VALUE));
    assertDoesNotThrow(() => clearTimeout(-Infinity));
  });
  routes.set('/clearTimeout/id-parameter-positive', async () => {
    assertDoesNotThrow(() => clearTimeout(1));
    assertDoesNotThrow(() => clearTimeout(1.1));
    assertDoesNotThrow(() => clearTimeout(Number.MAX_SAFE_INTEGER));
    assertDoesNotThrow(() => clearTimeout(Number.MAX_VALUE));
    assertDoesNotThrow(() => clearTimeout(Infinity));
  });
  routes.set('/clearTimeout/returns-undefined', async () => {
    let result = clearTimeout(1);
    assert(typeof result, 'undefined', `typeof result === "undefined"`);
  });
  routes.set('/clearTimeout/called-unbound', async () => {
    assertDoesNotThrow(() => {
      clearTimeout.call(undefined, 1);
    });
  });
}
