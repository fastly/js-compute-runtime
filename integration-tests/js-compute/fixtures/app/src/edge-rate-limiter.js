/// <reference path="../../../../../types/index.d.ts" />
/* eslint-env serviceworker */

import { assert, assertThrows } from './assertions.js';
import {
  RateCounter,
  PenaltyBox,
  EdgeRateLimiter,
} from 'fastly:edge-rate-limiter';
import { routes, isRunningLocally } from './routes.js';

// RateCounter
{
  routes.set('/rate-counter/interface', () => {
    let actual = Reflect.ownKeys(RateCounter);
    let expected = ['prototype', 'length', 'name'];
    assert(actual, expected, `Reflect.ownKeys(RateCounter)`);

    // Check the prototype descriptors are correct
    {
      actual = Reflect.getOwnPropertyDescriptor(RateCounter, 'prototype');
      expected = {
        value: RateCounter.prototype,
        writable: false,
        enumerable: false,
        configurable: false,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(RateCounter, 'prototype')`,
      );
    }

    // Check the constructor function's defined parameter length is correct
    {
      actual = Reflect.getOwnPropertyDescriptor(RateCounter, 'length');
      expected = {
        value: 0,
        writable: false,
        enumerable: false,
        configurable: true,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(RateCounter, 'length')`,
      );
    }

    // Check the constructor function's name is correct
    {
      actual = Reflect.getOwnPropertyDescriptor(RateCounter, 'name');
      expected = {
        value: 'RateCounter',
        writable: false,
        enumerable: false,
        configurable: true,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(RateCounter, 'name')`,
      );
    }

    // Check the prototype has the correct keys
    {
      actual = Reflect.ownKeys(RateCounter.prototype);
      expected = [
        'constructor',
        'increment',
        'lookupRate',
        'lookupCount',
        Symbol.toStringTag,
      ];
      assert(actual, expected, `Reflect.ownKeys(RateCounter.prototype)`);
    }

    // Check the constructor on the prototype is correct
    {
      actual = Reflect.getOwnPropertyDescriptor(
        RateCounter.prototype,
        'constructor',
      );
      expected = {
        writable: true,
        enumerable: false,
        configurable: true,
        value: RateCounter.prototype.constructor,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(RateCounter.prototype, 'constructor')`,
      );

      assert(
        typeof RateCounter.prototype.constructor,
        'function',
        `typeof RateCounter.prototype.constructor`,
      );

      actual = Reflect.getOwnPropertyDescriptor(
        RateCounter.prototype.constructor,
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
        `Reflect.getOwnPropertyDescriptor(RateCounter.prototype.constructor, 'length')`,
      );

      actual = Reflect.getOwnPropertyDescriptor(
        RateCounter.prototype.constructor,
        'name',
      );
      expected = {
        value: 'RateCounter',
        writable: false,
        enumerable: false,
        configurable: true,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(RateCounter.prototype.constructor, 'name')`,
      );
    }

    // Check the Symbol.toStringTag on the prototype is correct
    {
      actual = Reflect.getOwnPropertyDescriptor(
        RateCounter.prototype,
        Symbol.toStringTag,
      );
      expected = {
        writable: false,
        enumerable: false,
        configurable: true,
        value: 'RateCounter',
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(RateCounter.prototype, [Symbol.toStringTag])`,
      );

      assert(
        typeof RateCounter.prototype[Symbol.toStringTag],
        'string',
        `typeof RateCounter.prototype[Symbol.toStringTag]`,
      );
    }

    // Check the increment method has correct descriptors, length and name
    {
      actual = Reflect.getOwnPropertyDescriptor(
        RateCounter.prototype,
        'increment',
      );
      expected = {
        writable: true,
        enumerable: true,
        configurable: true,
        value: RateCounter.prototype.increment,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(RateCounter.prototype, 'increment')`,
      );

      assert(
        typeof RateCounter.prototype.increment,
        'function',
        `typeof RateCounter.prototype.increment`,
      );

      actual = Reflect.getOwnPropertyDescriptor(
        RateCounter.prototype.increment,
        'length',
      );
      expected = {
        value: 2,
        writable: false,
        enumerable: false,
        configurable: true,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(RateCounter.prototype.increment, 'length')`,
      );

      actual = Reflect.getOwnPropertyDescriptor(
        RateCounter.prototype.increment,
        'name',
      );
      expected = {
        value: 'increment',
        writable: false,
        enumerable: false,
        configurable: true,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(RateCounter.prototype.increment, 'name')`,
      );
    }

    // Check the lookupRate method has correct descriptors, length and name
    {
      actual = Reflect.getOwnPropertyDescriptor(
        RateCounter.prototype,
        'lookupRate',
      );
      expected = {
        writable: true,
        enumerable: true,
        configurable: true,
        value: RateCounter.prototype.lookupRate,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(RateCounter.prototype, 'lookupRate')`,
      );

      assert(
        typeof RateCounter.prototype.lookupRate,
        'function',
        `typeof RateCounter.prototype.lookupRate`,
      );

      actual = Reflect.getOwnPropertyDescriptor(
        RateCounter.prototype.lookupRate,
        'length',
      );
      expected = {
        value: 2,
        writable: false,
        enumerable: false,
        configurable: true,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(RateCounter.prototype.lookupRate, 'length')`,
      );

      actual = Reflect.getOwnPropertyDescriptor(
        RateCounter.prototype.lookupRate,
        'name',
      );
      expected = {
        value: 'lookupRate',
        writable: false,
        enumerable: false,
        configurable: true,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(RateCounter.prototype.lookupRate, 'name')`,
      );
    }

    // Check the lookupCount method has correct descriptors, length and name
    {
      actual = Reflect.getOwnPropertyDescriptor(
        RateCounter.prototype,
        'lookupCount',
      );
      expected = {
        writable: true,
        enumerable: true,
        configurable: true,
        value: RateCounter.prototype.lookupCount,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(RateCounter.prototype, 'lookupCount')`,
      );

      assert(
        typeof RateCounter.prototype.lookupCount,
        'function',
        `typeof RateCounter.prototype.lookupCount`,
      );

      actual = Reflect.getOwnPropertyDescriptor(
        RateCounter.prototype.lookupCount,
        'length',
      );
      expected = {
        value: 2,
        writable: false,
        enumerable: false,
        configurable: true,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(RateCounter.prototype.lookupCount, 'length')`,
      );

      actual = Reflect.getOwnPropertyDescriptor(
        RateCounter.prototype.lookupCount,
        'name',
      );
      expected = {
        value: 'lookupCount',
        writable: false,
        enumerable: false,
        configurable: true,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(RateCounter.prototype.lookupCount, 'name')`,
      );
    }
  });

  // RateCounter constructor
  {
    routes.set('/rate-counter/constructor/called-as-regular-function', () => {
      assertThrows(
        () => {
          RateCounter();
        },
        Error,
        `calling a builtin RateCounter constructor without new is forbidden`,
      );
    });
    routes.set(
      '/rate-counter/constructor/called-as-constructor-no-arguments',
      () => {
        assertThrows(
          () => new RateCounter(),
          Error,
          `RateCounter constructor: At least 1 argument required, but only 0 passed`,
        );
      },
    );
    // Ensure we correctly coerce the parameter to a string as according to
    // https://tc39.es/ecma262/#sec-tostring
    routes.set(
      '/rate-counter/constructor/name-parameter-calls-7.1.17-ToString',
      () => {
        if (!isRunningLocally()) {
          let sentinel;
          const test = () => {
            sentinel = Symbol('sentinel');
            const name = {
              toString() {
                throw sentinel;
              },
            };
            new RateCounter(name);
          };
          assertThrows(test);
          try {
            test();
          } catch (thrownError) {
            assert(thrownError, sentinel, 'thrownError === sentinel');
          }
          assertThrows(
            () => {
              new RateCounter(Symbol());
            },
            Error,
            `can't convert symbol to string`,
          );
        }
      },
    );
    routes.set('/rate-counter/constructor/happy-path', () => {
      assert(
        new RateCounter('rc') instanceof RateCounter,
        true,
        `new RateCounter("rc") instanceof RateCounter`,
      );
    });
  }

  // RateCounter increment method
  // increment(entry: string, delta: number): void;
  {
    routes.set('/rate-counter/increment/called-as-constructor', () => {
      assertThrows(() => {
        new RateCounter.prototype.increment('entry', 1);
      }, Error);
    });
    // Ensure we correctly coerce the parameter to a string as according to
    // https://tc39.es/ecma262/#sec-tostring
    routes.set(
      '/rate-counter/increment/entry-parameter-calls-7.1.17-ToString',
      () => {
        let sentinel;
        const test = () => {
          sentinel = Symbol('sentinel');
          const entry = {
            toString() {
              throw sentinel;
            },
          };
          let rc = new RateCounter('rc');
          rc.increment(entry, 1);
        };
        assertThrows(test);
        try {
          test();
        } catch (thrownError) {
          console.log({ thrownError });
          assert(thrownError, sentinel, 'thrownError === sentinel');
        }
        assertThrows(
          () => {
            let rc = new RateCounter('rc');
            rc.increment(Symbol(), 1);
          },
          Error,
          `can't convert symbol to string`,
        );
      },
    );
    routes.set('/rate-counter/increment/entry-parameter-not-supplied', () => {
      assertThrows(
        () => {
          let rc = new RateCounter('rc');
          rc.increment();
        },
        Error,
        `increment: At least 2 arguments required, but only 0 passed`,
      );
    });
    routes.set('/rate-counter/increment/delta-parameter-not-supplied', () => {
      assertThrows(
        () => {
          let rc = new RateCounter('rc');
          rc.increment('entry');
        },
        Error,
        `increment: At least 2 arguments required, but only 1 passed`,
      );
    });
    routes.set('/rate-counter/increment/delta-parameter-negative', () => {
      assertThrows(
        () => {
          let rc = new RateCounter('rc');
          rc.increment('entry', -1);
        },
        Error,
        `increment: delta parameter is an invalid value, only positive numbers can be used for delta values.`,
      );
    });
    routes.set('/rate-counter/increment/delta-parameter-infinity', () => {
      assertThrows(
        () => {
          let rc = new RateCounter('rc');
          rc.increment('entry', Infinity);
        },
        Error,
        `increment: delta parameter is an invalid value, only positive numbers can be used for delta values.`,
      );
    });
    routes.set('/rate-counter/increment/delta-parameter-NaN', () => {
      assertThrows(
        () => {
          let rc = new RateCounter('rc');
          rc.increment('entry', NaN);
        },
        Error,
        `increment: delta parameter is an invalid value, only positive numbers can be used for delta values.`,
      );
    });
    routes.set('/rate-counter/increment/returns-undefined', () => {
      let rc = new RateCounter('rc');
      assert(rc.increment('meow', 1), undefined, "rc.increment('meow', 1)");
    });
  }

  // RateCounter lookupRate method
  // lookupRate(entry: string, window: [1, 10, 60]): number;
  {
    routes.set('/rate-counter/lookupRate/called-as-constructor', () => {
      assertThrows(() => {
        new RateCounter.prototype.lookupRate('entry', 1);
      }, Error);
    });
    // Ensure we correctly coerce the parameter to a string as according to
    // https://tc39.es/ecma262/#sec-tostring
    routes.set(
      '/rate-counter/lookupRate/entry-parameter-calls-7.1.17-ToString',
      () => {
        let sentinel;
        const test = () => {
          sentinel = Symbol('sentinel');
          const entry = {
            toString() {
              throw sentinel;
            },
          };
          let rc = new RateCounter('rc');
          rc.lookupRate(entry, 1);
        };
        assertThrows(test);
        try {
          test();
        } catch (thrownError) {
          console.log({ thrownError });
          assert(thrownError, sentinel, 'thrownError === sentinel');
        }
        assertThrows(
          () => {
            let rc = new RateCounter('rc');
            rc.lookupRate(Symbol(), 1);
          },
          Error,
          `can't convert symbol to string`,
        );
      },
    );
    routes.set('/rate-counter/lookupRate/entry-parameter-not-supplied', () => {
      assertThrows(
        () => {
          let rc = new RateCounter('rc');
          rc.lookupRate();
        },
        Error,
        `lookupRate: At least 2 arguments required, but only 0 passed`,
      );
    });
    routes.set('/rate-counter/lookupRate/window-parameter-not-supplied', () => {
      assertThrows(
        () => {
          let rc = new RateCounter('rc');
          rc.lookupRate('entry');
        },
        Error,
        `lookupRate: At least 2 arguments required, but only 1 passed`,
      );
    });
    routes.set('/rate-counter/lookupRate/window-parameter-negative', () => {
      if (!isRunningLocally()) {
        assertThrows(
          () => {
            let rc = new RateCounter('rc');
            rc.lookupRate('entry', -1);
          },
          Error,
          `lookupRate: window parameter must be either: 1, 10, or 60`,
        );
      }
    });
    routes.set('/rate-counter/lookupRate/window-parameter-infinity', () => {
      if (!isRunningLocally()) {
        assertThrows(
          () => {
            let rc = new RateCounter('rc');
            rc.lookupRate('entry', Infinity);
          },
          Error,
          `lookupRate: window parameter must be either: 1, 10, or 60`,
        );
      }
    });
    routes.set('/rate-counter/lookupRate/window-parameter-NaN', () => {
      if (!isRunningLocally()) {
        assertThrows(
          () => {
            let rc = new RateCounter('rc');
            rc.lookupRate('entry', NaN);
          },
          Error,
          `lookupRate: window parameter must be either: 1, 10, or 60`,
        );
      }
    });
    routes.set('/rate-counter/lookupRate/returns-number', () => {
      let rc = new RateCounter('rc');
      assert(
        typeof rc.lookupRate('meow', 1),
        'number',
        `typeof rc.lookupRate('meow', 1)`,
      );
    });
  }

  // RateCounter lookupCount method
  // lookupCount(entry: string, duration: [10, 20, 30, 40, 50, 60]): number;
  {
    routes.set('/rate-counter/lookupCount/called-as-constructor', () => {
      assertThrows(() => {
        new RateCounter.prototype.lookupCount('entry', 1);
      }, Error);
    });
    // Ensure we correctly coerce the parameter to a string as according to
    // https://tc39.es/ecma262/#sec-tostring
    routes.set(
      '/rate-counter/lookupCount/entry-parameter-calls-7.1.17-ToString',
      () => {
        let sentinel;
        const test = () => {
          sentinel = Symbol('sentinel');
          const entry = {
            toString() {
              throw sentinel;
            },
          };
          let rc = new RateCounter('rc');
          rc.lookupCount(entry, 1);
        };
        assertThrows(test);
        try {
          test();
        } catch (thrownError) {
          console.log({ thrownError });
          assert(thrownError, sentinel, 'thrownError === sentinel');
        }
        assertThrows(
          () => {
            let rc = new RateCounter('rc');
            rc.lookupCount(Symbol(), 1);
          },
          Error,
          `can't convert symbol to string`,
        );
      },
    );
    routes.set('/rate-counter/lookupCount/entry-parameter-not-supplied', () => {
      if (!isRunningLocally()) {
        assertThrows(
          () => {
            let rc = new RateCounter('rc');
            rc.lookupCount();
          },
          Error,
          `lookupCount: At least 2 arguments required, but only 0 passed`,
        );
      }
    });
    routes.set(
      '/rate-counter/lookupCount/duration-parameter-not-supplied',
      () => {
        if (!isRunningLocally()) {
          assertThrows(
            () => {
              let rc = new RateCounter('rc');
              rc.lookupCount('entry');
            },
            Error,
            `lookupCount: At least 2 arguments required, but only 1 passed`,
          );
        }
      },
    );
    routes.set('/rate-counter/lookupCount/duration-parameter-negative', () => {
      if (!isRunningLocally()) {
        assertThrows(
          () => {
            let rc = new RateCounter('rc');
            rc.lookupCount('entry', -1);
          },
          Error,
          `lookupCount: duration parameter must be either: 10, 20, 30, 40, 50, or 60`,
        );
      }
    });
    routes.set('/rate-counter/lookupCount/duration-parameter-infinity', () => {
      if (!isRunningLocally()) {
        assertThrows(
          () => {
            let rc = new RateCounter('rc');
            rc.lookupCount('entry', Infinity);
          },
          Error,
          `lookupCount: duration parameter must be either: 10, 20, 30, 40, 50, or 60`,
        );
      }
    });
    routes.set('/rate-counter/lookupCount/duration-parameter-NaN', () => {
      if (!isRunningLocally()) {
        assertThrows(
          () => {
            let rc = new RateCounter('rc');
            rc.lookupCount('entry', NaN);
          },
          Error,
          `lookupCount: duration parameter must be either: 10, 20, 30, 40, 50, or 60`,
        );
      }
    });
    routes.set('/rate-counter/lookupCount/returns-number', () => {
      let rc = new RateCounter('rc');
      assert(
        typeof rc.lookupCount('meow', 10),
        'number',
        `typeof rc.lookupCount('meow', 1)`,
      );
    });
  }
}

// PenaltyBox
{
  routes.set('/penalty-box/interface', () => {
    let actual = Reflect.ownKeys(PenaltyBox);
    let expected = ['prototype', 'length', 'name'];
    assert(actual, expected, `Reflect.ownKeys(PenaltyBox)`);

    // Check the prototype descriptors are correct
    {
      actual = Reflect.getOwnPropertyDescriptor(PenaltyBox, 'prototype');
      expected = {
        value: PenaltyBox.prototype,
        writable: false,
        enumerable: false,
        configurable: false,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(PenaltyBox, 'prototype')`,
      );
    }

    // Check the constructor function's defined parameter length is correct
    {
      actual = Reflect.getOwnPropertyDescriptor(PenaltyBox, 'length');
      expected = {
        value: 0,
        writable: false,
        enumerable: false,
        configurable: true,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(PenaltyBox, 'length')`,
      );
    }

    // Check the constructor function's name is correct
    {
      actual = Reflect.getOwnPropertyDescriptor(PenaltyBox, 'name');
      expected = {
        value: 'PenaltyBox',
        writable: false,
        enumerable: false,
        configurable: true,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(PenaltyBox, 'name')`,
      );
    }

    // Check the prototype has the correct keys
    {
      actual = Reflect.ownKeys(PenaltyBox.prototype);
      expected = ['constructor', 'add', 'has', Symbol.toStringTag];
      assert(actual, expected, `Reflect.ownKeys(PenaltyBox.prototype)`);
    }

    // Check the constructor on the prototype is correct
    {
      actual = Reflect.getOwnPropertyDescriptor(
        PenaltyBox.prototype,
        'constructor',
      );
      expected = {
        writable: true,
        enumerable: false,
        configurable: true,
        value: PenaltyBox.prototype.constructor,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(PenaltyBox.prototype, 'constructor')`,
      );

      assert(
        typeof PenaltyBox.prototype.constructor,
        'function',
        `typeof PenaltyBox.prototype.constructor`,
      );

      actual = Reflect.getOwnPropertyDescriptor(
        PenaltyBox.prototype.constructor,
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
        `Reflect.getOwnPropertyDescriptor(PenaltyBox.prototype.constructor, 'length')`,
      );

      actual = Reflect.getOwnPropertyDescriptor(
        PenaltyBox.prototype.constructor,
        'name',
      );
      expected = {
        value: 'PenaltyBox',
        writable: false,
        enumerable: false,
        configurable: true,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(PenaltyBox.prototype.constructor, 'name')`,
      );
    }

    // Check the Symbol.toStringTag on the prototype is correct
    {
      actual = Reflect.getOwnPropertyDescriptor(
        PenaltyBox.prototype,
        Symbol.toStringTag,
      );
      expected = {
        writable: false,
        enumerable: false,
        configurable: true,
        value: 'PenaltyBox',
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(PenaltyBox.prototype, [Symbol.toStringTag])`,
      );

      assert(
        typeof PenaltyBox.prototype[Symbol.toStringTag],
        'string',
        `typeof PenaltyBox.prototype[Symbol.toStringTag]`,
      );
    }

    // Check the add method has correct descriptors, length and name
    {
      actual = Reflect.getOwnPropertyDescriptor(PenaltyBox.prototype, 'add');
      expected = {
        writable: true,
        enumerable: true,
        configurable: true,
        value: PenaltyBox.prototype.add,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(PenaltyBox.prototype, 'add')`,
      );

      assert(
        typeof PenaltyBox.prototype.add,
        'function',
        `typeof PenaltyBox.prototype.add`,
      );

      actual = Reflect.getOwnPropertyDescriptor(
        PenaltyBox.prototype.add,
        'length',
      );
      expected = {
        value: 2,
        writable: false,
        enumerable: false,
        configurable: true,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(PenaltyBox.prototype.add, 'length')`,
      );

      actual = Reflect.getOwnPropertyDescriptor(
        PenaltyBox.prototype.add,
        'name',
      );
      expected = {
        value: 'add',
        writable: false,
        enumerable: false,
        configurable: true,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(PenaltyBox.prototype.add, 'name')`,
      );
    }

    // Check the has method has correct descriptors, length and name
    {
      actual = Reflect.getOwnPropertyDescriptor(PenaltyBox.prototype, 'has');
      expected = {
        writable: true,
        enumerable: true,
        configurable: true,
        value: PenaltyBox.prototype.has,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(PenaltyBox.prototype, 'has')`,
      );

      assert(
        typeof PenaltyBox.prototype.has,
        'function',
        `typeof PenaltyBox.prototype.has`,
      );

      actual = Reflect.getOwnPropertyDescriptor(
        PenaltyBox.prototype.has,
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
        `Reflect.getOwnPropertyDescriptor(PenaltyBox.prototype.has, 'length')`,
      );

      actual = Reflect.getOwnPropertyDescriptor(
        PenaltyBox.prototype.has,
        'name',
      );
      expected = {
        value: 'has',
        writable: false,
        enumerable: false,
        configurable: true,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(PenaltyBox.prototype.has, 'name')`,
      );
    }
  });

  // PenaltyBox constructor
  {
    routes.set('/penalty-box/constructor/called-as-regular-function', () => {
      assertThrows(
        () => {
          PenaltyBox();
        },
        Error,
        `calling a builtin PenaltyBox constructor without new is forbidden`,
      );
    });
    routes.set(
      '/penalty-box/constructor/called-as-constructor-no-arguments',
      () => {
        assertThrows(
          () => new PenaltyBox(),
          Error,
          `PenaltyBox constructor: At least 1 argument required, but only 0 passed`,
        );
      },
    );
    // Ensure we correctly coerce the parameter to a string as according to
    // https://tc39.es/ecma262/#sec-tostring
    routes.set(
      '/penalty-box/constructor/name-parameter-calls-7.1.17-ToString',
      () => {
        if (!isRunningLocally()) {
          let sentinel;
          const test = () => {
            sentinel = Symbol('sentinel');
            const name = {
              toString() {
                throw sentinel;
              },
            };
            new PenaltyBox(name);
          };
          assertThrows(test);
          try {
            test();
          } catch (thrownError) {
            assert(thrownError, sentinel, 'thrownError === sentinel');
          }
          assertThrows(
            () => {
              new PenaltyBox(Symbol());
            },
            Error,
            `can't convert symbol to string`,
          );
        }
      },
    );
    routes.set('/penalty-box/constructor/happy-path', () => {
      assert(
        new PenaltyBox('rc') instanceof PenaltyBox,
        true,
        `new PenaltyBox("rc") instanceof PenaltyBox`,
      );
    });
  }

  // PenaltyBox has method
  // has(entry: string): boolean;
  {
    routes.set('/penalty-box/has/called-as-constructor', () => {
      assertThrows(() => {
        new PenaltyBox.prototype.has('entry');
      }, Error);
    });
    // Ensure we correctly coerce the parameter to a string as according to
    // https://tc39.es/ecma262/#sec-tostring
    routes.set('/penalty-box/has/entry-parameter-calls-7.1.17-ToString', () => {
      let sentinel;
      const test = () => {
        sentinel = Symbol('sentinel');
        const entry = {
          toString() {
            throw sentinel;
          },
        };
        let pb = new PenaltyBox('pb');
        pb.has(entry);
      };
      assertThrows(test);
      try {
        test();
      } catch (thrownError) {
        console.log({ thrownError });
        assert(thrownError, sentinel, 'thrownError === sentinel');
      }
      assertThrows(
        () => {
          let pb = new PenaltyBox('pb');
          pb.has(Symbol());
        },
        Error,
        `can't convert symbol to string`,
      );
    });
    routes.set('/penalty-box/has/entry-parameter-not-supplied', () => {
      assertThrows(
        () => {
          let pb = new PenaltyBox('pb');
          pb.has();
        },
        Error,
        `has: At least 1 argument required, but only 0 passed`,
      );
    });
    routes.set('/penalty-box/has/returns-boolean', () => {
      let pb = new PenaltyBox('pb');
      assert(pb.has('meow'), false, "pb.has('meow')");
    });
  }

  // PenaltyBox add method
  // add(entry: string, timeToLive: number): void;
  {
    routes.set('/penalty-box/add/called-as-constructor', () => {
      assertThrows(() => {
        new PenaltyBox.prototype.add('entry', 1);
      }, Error);
    });
    // Ensure we correctly coepbe the parameter to a string as according to
    // https://tc39.es/ecma262/#sec-tostring
    routes.set('/penalty-box/add/entry-parameter-calls-7.1.17-ToString', () => {
      let sentinel;
      const test = () => {
        sentinel = Symbol('sentinel');
        const entry = {
          toString() {
            throw sentinel;
          },
        };
        let pb = new PenaltyBox('pb');
        pb.add(entry, 1);
      };
      assertThrows(test);
      try {
        test();
      } catch (thrownError) {
        console.log({ thrownError });
        assert(thrownError, sentinel, 'thrownError === sentinel');
      }
      assertThrows(
        () => {
          let pb = new PenaltyBox('pb');
          pb.add(Symbol(), 1);
        },
        Error,
        `can't convert symbol to string`,
      );
    });
    routes.set('/penalty-box/add/entry-parameter-not-supplied', () => {
      assertThrows(
        () => {
          let pb = new PenaltyBox('pb');
          pb.add();
        },
        Error,
        `add: At least 2 arguments required, but only 0 passed`,
      );
    });
    routes.set('/penalty-box/add/timeToLive-parameter-not-supplied', () => {
      assertThrows(
        () => {
          let pb = new PenaltyBox('pb');
          pb.add('entry');
        },
        Error,
        `add: At least 2 arguments required, but only 1 passed`,
      );
    });
    routes.set('/penalty-box/add/timeToLive-parameter-negative', () => {
      assertThrows(
        () => {
          let pb = new PenaltyBox('pb');
          pb.add('entry', -1);
        },
        Error,
        `add: timeToLive parameter is an invalid value, only numbers from 1 to 60 can be used for timeToLive values.`,
      );
    });
    routes.set('/penalty-box/add/timeToLive-parameter-infinity', () => {
      assertThrows(
        () => {
          let pb = new PenaltyBox('pb');
          pb.add('entry', Infinity);
        },
        Error,
        `add: timeToLive parameter is an invalid value, only numbers from 1 to 60 can be used for timeToLive values.`,
      );
    });
    routes.set('/penalty-box/add/timeToLive-parameter-NaN', () => {
      assertThrows(
        () => {
          let pb = new PenaltyBox('pb');
          pb.add('entry', NaN);
        },
        Error,
        `add: timeToLive parameter is an invalid value, only numbers from 1 to 60 can be used for timeToLive values.`,
      );
    });
    routes.set('/penalty-box/add/returns-undefined', () => {
      let pb = new PenaltyBox('pb');
      assert(pb.add('meow', 1), undefined, `pb.add('meow', 1)`);
    });
  }
}

// EdgeRateLimiter
{
  routes.set('/edge-rate-limiter/interface', () => {
    let actual = Reflect.ownKeys(EdgeRateLimiter);
    let expected = ['prototype', 'length', 'name'];
    assert(actual, expected, `Reflect.ownKeys(EdgeRateLimiter)`);

    // Check the prototype descriptors are correct
    {
      actual = Reflect.getOwnPropertyDescriptor(EdgeRateLimiter, 'prototype');
      expected = {
        value: EdgeRateLimiter.prototype,
        writable: false,
        enumerable: false,
        configurable: false,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(EdgeRateLimiter, 'prototype')`,
      );
    }

    // Check the constructor function's defined parameter length is correct
    {
      actual = Reflect.getOwnPropertyDescriptor(EdgeRateLimiter, 'length');
      expected = {
        value: 0,
        writable: false,
        enumerable: false,
        configurable: true,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(EdgeRateLimiter, 'length')`,
      );
    }

    // Check the constructor function's name is correct
    {
      actual = Reflect.getOwnPropertyDescriptor(EdgeRateLimiter, 'name');
      expected = {
        value: 'EdgeRateLimiter',
        writable: false,
        enumerable: false,
        configurable: true,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(EdgeRateLimiter, 'name')`,
      );
    }

    // Check the prototype has the correct keys
    {
      actual = Reflect.ownKeys(EdgeRateLimiter.prototype);
      expected = ['constructor', 'checkRate', Symbol.toStringTag];
      assert(actual, expected, `Reflect.ownKeys(EdgeRateLimiter.prototype)`);
    }

    // Check the constructor on the prototype is correct
    {
      actual = Reflect.getOwnPropertyDescriptor(
        EdgeRateLimiter.prototype,
        'constructor',
      );
      expected = {
        writable: true,
        enumerable: false,
        configurable: true,
        value: EdgeRateLimiter.prototype.constructor,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(EdgeRateLimiter.prototype, 'constructor')`,
      );

      assert(
        typeof EdgeRateLimiter.prototype.constructor,
        'function',
        `typeof EdgeRateLimiter.prototype.constructor`,
      );

      actual = Reflect.getOwnPropertyDescriptor(
        EdgeRateLimiter.prototype.constructor,
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
        `Reflect.getOwnPropertyDescriptor(EdgeRateLimiter.prototype.constructor, 'length')`,
      );

      actual = Reflect.getOwnPropertyDescriptor(
        EdgeRateLimiter.prototype.constructor,
        'name',
      );
      expected = {
        value: 'EdgeRateLimiter',
        writable: false,
        enumerable: false,
        configurable: true,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(EdgeRateLimiter.prototype.constructor, 'name')`,
      );
    }

    // Check the Symbol.toStringTag on the prototype is correct
    {
      actual = Reflect.getOwnPropertyDescriptor(
        EdgeRateLimiter.prototype,
        Symbol.toStringTag,
      );
      expected = {
        writable: false,
        enumerable: false,
        configurable: true,
        value: 'EdgeRateLimiter',
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(EdgeRateLimiter.prototype, [Symbol.toStringTag])`,
      );

      assert(
        typeof EdgeRateLimiter.prototype[Symbol.toStringTag],
        'string',
        `typeof EdgeRateLimiter.prototype[Symbol.toStringTag]`,
      );
    }

    // Check the checkRate method has correct descriptors, length and name
    {
      actual = Reflect.getOwnPropertyDescriptor(
        EdgeRateLimiter.prototype,
        'checkRate',
      );
      expected = {
        writable: true,
        enumerable: true,
        configurable: true,
        value: EdgeRateLimiter.prototype.checkRate,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(EdgeRateLimiter.prototype, 'checkRate')`,
      );

      assert(
        typeof EdgeRateLimiter.prototype.checkRate,
        'function',
        `typeof EdgeRateLimiter.prototype.checkRate`,
      );

      actual = Reflect.getOwnPropertyDescriptor(
        EdgeRateLimiter.prototype.checkRate,
        'length',
      );
      expected = {
        value: 5,
        writable: false,
        enumerable: false,
        configurable: true,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(EdgeRateLimiter.prototype.checkRate, 'length')`,
      );

      actual = Reflect.getOwnPropertyDescriptor(
        EdgeRateLimiter.prototype.checkRate,
        'name',
      );
      expected = {
        value: 'checkRate',
        writable: false,
        enumerable: false,
        configurable: true,
      };
      assert(
        actual,
        expected,
        `Reflect.getOwnPropertyDescriptor(EdgeRateLimiter.prototype.checkRate, 'name')`,
      );
    }
  });

  // EdgeRateLimiter constructor
  {
    routes.set(
      '/edge-rate-limiter/constructor/called-as-regular-function',
      () => {
        assertThrows(
          () => {
            EdgeRateLimiter();
          },
          Error,
          `calling a builtin EdgeRateLimiter constructor without new is forbidden`,
        );
      },
    );
    routes.set(
      '/edge-rate-limiter/constructor/called-as-constructor-no-arguments',
      () => {
        assertThrows(
          () => new EdgeRateLimiter(),
          Error,
          `EdgeRateLimiter constructor: At least 2 arguments required, but only 0 passed`,
        );
      },
    );
    // Ensure we correctly coerce the parameter to a string as according to
    // https://tc39.es/ecma262/#sec-tostring
    routes.set(
      '/edge-rate-limiter/constructor/rate-counter-not-instance-of-rateCounter',
      () => {
        assertThrows(
          () => {
            new EdgeRateLimiter(true, true);
          },
          Error,
          `EdgeRateLimiter constructor: rateCounter parameter must be an instance of RateCounter`,
        );
      },
    );
    routes.set(
      '/edge-rate-limiter/constructor/penalty-box-not-instance-of-penaltyBox',
      () => {
        assertThrows(
          () => {
            new EdgeRateLimiter(new RateCounter('rc'), true);
          },
          Error,
          `EdgeRateLimiter constructor: penaltyBox parameter must be an instance of PenaltyBox`,
        );
      },
    );
    routes.set('/edge-rate-limiter/constructor/happy-path', () => {
      assert(
        new EdgeRateLimiter(
          new RateCounter('rc'),
          new PenaltyBox('pb'),
        ) instanceof EdgeRateLimiter,
        true,
        `new EdgeRateLimiter(new RateCounter("rc"), new PenaltyBox('pb')) instanceof EdgeRateLimiter`,
      );
    });
  }

  // EdgeRateLimiter checkRate method
  // checkRate(entry: string, delta: number, window: [1, 10, 60], limit: number, timeToLive: number): boolean;
  {
    routes.set('/edge-rate-limiter/checkRate/called-as-constructor', () => {
      assertThrows(() => {
        new EdgeRateLimiter.prototype.checkRate('entry');
      }, Error);
    });
    // Ensure we correctly coerce the parameter to a string as according to
    // https://tc39.es/ecma262/#sec-tostring
    routes.set(
      '/edge-rate-limiter/checkRate/entry-parameter-calls-7.1.17-ToString',
      () => {
        let sentinel;
        const test = () => {
          sentinel = Symbol('sentinel');
          const entry = {
            toString() {
              throw sentinel;
            },
          };
          let rc = new RateCounter('rc');
          let pb = new PenaltyBox('pb');
          let erl = new EdgeRateLimiter(rc, pb);
          erl.checkRate(entry, 1, 1, 1, 1);
        };
        assertThrows(test);
        try {
          test();
        } catch (thrownError) {
          console.log({ thrownError });
          assert(thrownError, sentinel, 'thrownError === sentinel');
        }
        assertThrows(
          () => {
            let rc = new RateCounter('rc');
            let pb = new PenaltyBox('pb');
            let erl = new EdgeRateLimiter(rc, pb);
            erl.checkRate(Symbol(), 1, 1, 1, 1);
          },
          Error,
          `can't convert symbol to string`,
        );
      },
    );
    routes.set(
      '/edge-rate-limiter/checkRate/entry-parameter-not-supplied',
      () => {
        assertThrows(
          () => {
            let rc = new RateCounter('rc');
            let pb = new PenaltyBox('pb');
            let erl = new EdgeRateLimiter(rc, pb);
            erl.checkRate();
          },
          Error,
          `checkRate: At least 5 arguments required, but only 0 passed`,
        );
      },
    );
    routes.set('/edge-rate-limiter/checkRate/delta-parameter-negative', () => {
      assertThrows(
        () => {
          let rc = new RateCounter('rc');
          let pb = new PenaltyBox('pb');
          let erl = new EdgeRateLimiter(rc, pb);
          erl.checkRate('entry', -1, 1, 1, 1);
        },
        Error,
        `checkRate: delta parameter is an invalid value, only positive numbers can be used for delta values.`,
      );
    });
    routes.set('/edge-rate-limiter/checkRate/delta-parameter-infinity', () => {
      assertThrows(
        () => {
          let rc = new RateCounter('rc');
          let pb = new PenaltyBox('pb');
          let erl = new EdgeRateLimiter(rc, pb);
          erl.checkRate('entry', Infinity, 1, 1, 1);
        },
        Error,
        `checkRate: delta parameter is an invalid value, only positive numbers can be used for delta values.`,
      );
    });
    routes.set('/edge-rate-limiter/checkRate/delta-parameter-NaN', () => {
      assertThrows(
        () => {
          let rc = new RateCounter('rc');
          let pb = new PenaltyBox('pb');
          let erl = new EdgeRateLimiter(rc, pb);
          erl.checkRate('entry', NaN, 1, 1, 1);
        },
        Error,
        `checkRate: delta parameter is an invalid value, only positive numbers can be used for delta values.`,
      );
    });
    routes.set('/edge-rate-limiter/checkRate/window-parameter-negative', () => {
      assertThrows(
        () => {
          let rc = new RateCounter('rc');
          let pb = new PenaltyBox('pb');
          let erl = new EdgeRateLimiter(rc, pb);
          erl.checkRate('entry', 1, -1, 1, 1);
        },
        Error,
        `checkRate: window parameter must be either: 1, 10, or 60`,
      );
    });
    routes.set('/edge-rate-limiter/checkRate/window-parameter-infinity', () => {
      assertThrows(
        () => {
          let rc = new RateCounter('rc');
          let pb = new PenaltyBox('pb');
          let erl = new EdgeRateLimiter(rc, pb);
          erl.checkRate('entry', 1, Infinity, 1, 1);
        },
        Error,
        `checkRate: window parameter must be either: 1, 10, or 60`,
      );
    });
    routes.set('/edge-rate-limiter/checkRate/window-parameter-NaN', () => {
      assertThrows(
        () => {
          let rc = new RateCounter('rc');
          let pb = new PenaltyBox('pb');
          let erl = new EdgeRateLimiter(rc, pb);
          erl.checkRate('entry', 1, NaN, 1, 1);
        },
        Error,
        `checkRate: window parameter must be either: 1, 10, or 60`,
      );
    });
    routes.set('/edge-rate-limiter/checkRate/limit-parameter-negative', () => {
      assertThrows(
        () => {
          let rc = new RateCounter('rc');
          let pb = new PenaltyBox('pb');
          let erl = new EdgeRateLimiter(rc, pb);
          erl.checkRate('entry', 1, 1, -1, 1);
        },
        Error,
        `checkRate: limit parameter is an invalid value, only positive numbers can be used for limit values.`,
      );
    });
    routes.set('/edge-rate-limiter/checkRate/limit-parameter-infinity', () => {
      assertThrows(
        () => {
          let rc = new RateCounter('rc');
          let pb = new PenaltyBox('pb');
          let erl = new EdgeRateLimiter(rc, pb);
          erl.checkRate('entry', 1, 1, Infinity, 1);
        },
        Error,
        `checkRate: limit parameter is an invalid value, only positive numbers can be used for limit values.`,
      );
    });
    routes.set('/edge-rate-limiter/checkRate/limit-parameter-NaN', () => {
      assertThrows(
        () => {
          let rc = new RateCounter('rc');
          let pb = new PenaltyBox('pb');
          let erl = new EdgeRateLimiter(rc, pb);
          erl.checkRate('entry', 1, 1, NaN, 1);
        },
        Error,
        `checkRate: limit parameter is an invalid value, only positive numbers can be used for limit values.`,
      );
    });
    routes.set(
      '/edge-rate-limiter/checkRate/timeToLive-parameter-negative',
      () => {
        assertThrows(
          () => {
            let rc = new RateCounter('rc');
            let pb = new PenaltyBox('pb');
            let erl = new EdgeRateLimiter(rc, pb);
            erl.checkRate('entry', 1, 1, 1, -1);
          },
          Error,
          `checkRate: timeToLive parameter is an invalid value, only numbers from 1 to 60 can be used for timeToLive values.`,
        );
      },
    );
    routes.set(
      '/edge-rate-limiter/checkRate/timeToLive-parameter-infinity',
      () => {
        assertThrows(
          () => {
            let rc = new RateCounter('rc');
            let pb = new PenaltyBox('pb');
            let erl = new EdgeRateLimiter(rc, pb);
            erl.checkRate('entry', 1, 1, 1, Infinity);
          },
          Error,
          `checkRate: timeToLive parameter is an invalid value, only numbers from 1 to 60 can be used for timeToLive values.`,
        );
      },
    );
    routes.set('/edge-rate-limiter/checkRate/timeToLive-parameter-NaN', () => {
      assertThrows(
        () => {
          let rc = new RateCounter('rc');
          let pb = new PenaltyBox('pb');
          let erl = new EdgeRateLimiter(rc, pb);
          erl.checkRate('entry', 1, 1, 1, NaN);
        },
        Error,
        `checkRate: timeToLive parameter is an invalid value, only numbers from 1 to 60 can be used for timeToLive values.`,
      );
    });
    routes.set('/edge-rate-limiter/checkRate/returns-boolean', () => {
      let rc = new RateCounter('rc');
      let pb = new PenaltyBox('pb');
      let erl = new EdgeRateLimiter(rc, pb);
      assert(
        erl.checkRate('woof', 1, 10, 100, 5),
        false,
        "erl.checkRate('meow', 1, 10, 100, 5)",
      );
    });
  }
}
