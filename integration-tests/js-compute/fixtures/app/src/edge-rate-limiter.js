/// <reference path="../../../../../types/index.d.ts" />
/* eslint-env serviceworker */

import { pass, assert, assertThrows } from "./assertions.js";
import { RateCounter, PenaltyBox } from 'fastly:edge-rate-limiter';
import { routes, isRunningLocally } from "./routes.js";

let error;
// RateCounter
{
  routes.set("/rate-counter/interface", () => {

    console.log(111, typeof RateCounter)
    let actual = Reflect.ownKeys(RateCounter)
    let expected = ["prototype", "length", "name"]
    error = assert(actual, expected, `Reflect.ownKeys(RateCounter)`)
    if (error) { return error }

    // Check the prototype descriptors are correct
    {
      actual = Reflect.getOwnPropertyDescriptor(RateCounter, 'prototype')
      expected = {
        "value": RateCounter.prototype,
        "writable": false,
        "enumerable": false,
        "configurable": false
      }
      error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(RateCounter, 'prototype')`)
      if (error) { return error }
    }

    // Check the constructor function's defined parameter length is correct
    {
      actual = Reflect.getOwnPropertyDescriptor(RateCounter, 'length')
      expected = {
        "value": 0,
        "writable": false,
        "enumerable": false,
        "configurable": true
      }
      error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(RateCounter, 'length')`)
      if (error) { return error }
    }

    // Check the constructor function's name is correct
    {
      actual = Reflect.getOwnPropertyDescriptor(RateCounter, 'name')
      expected = {
        "value": "RateCounter",
        "writable": false,
        "enumerable": false,
        "configurable": true
      }
      error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(RateCounter, 'name')`)
      if (error) { return error }
    }

    // Check the prototype has the correct keys
    {
      actual = Reflect.ownKeys(RateCounter.prototype)
      expected = ["constructor", "increment", "lookupRate", "lookupCount", Symbol.toStringTag]
      error = assert(actual, expected, `Reflect.ownKeys(RateCounter.prototype)`)
      if (error) { return error }
    }

    // Check the constructor on the prototype is correct
    {
      actual = Reflect.getOwnPropertyDescriptor(RateCounter.prototype, 'constructor')
      expected = { "writable": true, "enumerable": false, "configurable": true, value: RateCounter.prototype.constructor }
      error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(RateCounter.prototype, 'constructor')`)
      if (error) { return error }

      error = assert(typeof RateCounter.prototype.constructor, 'function', `typeof RateCounter.prototype.constructor`)
      if (error) { return error }

      actual = Reflect.getOwnPropertyDescriptor(RateCounter.prototype.constructor, 'length')
      expected = {
        "value": 0,
        "writable": false,
        "enumerable": false,
        "configurable": true
      }
      error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(RateCounter.prototype.constructor, 'length')`)
      if (error) { return error }

      actual = Reflect.getOwnPropertyDescriptor(RateCounter.prototype.constructor, 'name')
      expected = {
        "value": "RateCounter",
        "writable": false,
        "enumerable": false,
        "configurable": true
      }
      error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(RateCounter.prototype.constructor, 'name')`)
      if (error) { return error }
    }

    // Check the Symbol.toStringTag on the prototype is correct
    {
      actual = Reflect.getOwnPropertyDescriptor(RateCounter.prototype, Symbol.toStringTag)
      expected = { "writable": false, "enumerable": false, "configurable": true, value: "RateCounter" }
      error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(RateCounter.prototype, [Symbol.toStringTag])`)
      if (error) { return error }

      error = assert(typeof RateCounter.prototype[Symbol.toStringTag], 'string', `typeof RateCounter.prototype[Symbol.toStringTag]`)
      if (error) { return error }
    }

    // Check the increment method has correct descriptors, length and name
    {
      actual = Reflect.getOwnPropertyDescriptor(RateCounter.prototype, 'increment')
      expected = { "writable": true, "enumerable": true, "configurable": true, value: RateCounter.prototype.increment }
      error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(RateCounter.prototype, 'increment')`)
      if (error) { return error }

      error = assert(typeof RateCounter.prototype.increment, 'function', `typeof RateCounter.prototype.increment`)
      if (error) { return error }

      actual = Reflect.getOwnPropertyDescriptor(RateCounter.prototype.increment, 'length')
      expected = {
        "value": 2,
        "writable": false,
        "enumerable": false,
        "configurable": true
      }
      error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(RateCounter.prototype.increment, 'length')`)
      if (error) { return error }

      actual = Reflect.getOwnPropertyDescriptor(RateCounter.prototype.increment, 'name')
      expected = {
        "value": "increment",
        "writable": false,
        "enumerable": false,
        "configurable": true
      }
      error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(RateCounter.prototype.increment, 'name')`)
      if (error) { return error }
    }

    // Check the lookupRate method has correct descriptors, length and name
    {
      actual = Reflect.getOwnPropertyDescriptor(RateCounter.prototype, 'lookupRate')
      expected = { "writable": true, "enumerable": true, "configurable": true, value: RateCounter.prototype.lookupRate }
      error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(RateCounter.prototype, 'lookupRate')`)
      if (error) { return error }

      error = assert(typeof RateCounter.prototype.lookupRate, 'function', `typeof RateCounter.prototype.lookupRate`)
      if (error) { return error }

      actual = Reflect.getOwnPropertyDescriptor(RateCounter.prototype.lookupRate, 'length')
      expected = {
        "value": 2,
        "writable": false,
        "enumerable": false,
        "configurable": true
      }
      error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(RateCounter.prototype.lookupRate, 'length')`)
      if (error) { return error }

      actual = Reflect.getOwnPropertyDescriptor(RateCounter.prototype.lookupRate, 'name')
      expected = {
        "value": "lookupRate",
        "writable": false,
        "enumerable": false,
        "configurable": true
      }
      error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(RateCounter.prototype.lookupRate, 'name')`)
      if (error) { return error }
    }

    // Check the lookupCount method has correct descriptors, length and name
    {
      actual = Reflect.getOwnPropertyDescriptor(RateCounter.prototype, 'lookupCount')
      expected = { "writable": true, "enumerable": true, "configurable": true, value: RateCounter.prototype.lookupCount }
      error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(RateCounter.prototype, 'lookupCount')`)
      if (error) { return error }

      error = assert(typeof RateCounter.prototype.lookupCount, 'function', `typeof RateCounter.prototype.lookupCount`)
      if (error) { return error }

      actual = Reflect.getOwnPropertyDescriptor(RateCounter.prototype.lookupCount, 'length')
      expected = {
        "value": 2,
        "writable": false,
        "enumerable": false,
        "configurable": true
      }
      error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(RateCounter.prototype.lookupCount, 'length')`)
      if (error) { return error }

      actual = Reflect.getOwnPropertyDescriptor(RateCounter.prototype.lookupCount, 'name')
      expected = {
        "value": "lookupCount",
        "writable": false,
        "enumerable": false,
        "configurable": true
      }
      error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(RateCounter.prototype.lookupCount, 'name')`)
      if (error) { return error }
    }

    return pass('ok')
  });

  // RateCounter constructor
  {
    routes.set("/rate-counter/constructor/called-as-regular-function", () => {
      error = assertThrows(() => {
        RateCounter()
      }, Error, `calling a builtin RateCounter constructor without new is forbidden`)
      if (error) { return error }
      return pass('ok')
    });
    routes.set("/rate-counter/constructor/called-as-constructor-no-arguments", () => {
      error = assertThrows(() => new RateCounter(), Error, `RateCounter constructor: At least 1 argument required, but only 0 passed`)
      if (error) { return error }
      return pass('ok')
    });
    // Ensure we correctly coerce the parameter to a string as according to
    // https://tc39.es/ecma262/#sec-tostring
    routes.set("/rate-counter/constructor/name-parameter-calls-7.1.17-ToString", () => {
      if (!isRunningLocally()) {
        let sentinel;
        const test = () => {
          sentinel = Symbol('sentinel');
          const name = {
            toString() {
              throw sentinel;
            }
          }
          new RateCounter(name)
        }
        error = assertThrows(test)
        if (error) { return error }
        try {
          test()
        } catch (thrownError) {
          error = assert(thrownError, sentinel, 'thrownError === sentinel')
          if (error) { return error }
        }
        error = assertThrows(() => {
          new RateCounter(Symbol())
        }, Error, `can't convert symbol to string`)
        if (error) { return error }
      }
      return pass('ok')
    });
    routes.set("/rate-counter/constructor/happy-path", () => {
      error = assert(new RateCounter("rc") instanceof RateCounter, true, `new RateCounter("rc") instanceof RateCounter`)
      if (error) { return error }
      return pass('ok')
    });
  }

  // RateCounter increment method
  // increment(entry: string, delta: number): void;
  {
    routes.set("/rate-counter/increment/called-as-constructor", () => {
      error = assertThrows(() => {
        new RateCounter.prototype.increment('entry', 1)
      }, Error, `RateCounter.prototype.increment is not a constructor`)
      if (error) { return error }
      return pass('ok')
    });
    // Ensure we correctly coerce the parameter to a string as according to
    // https://tc39.es/ecma262/#sec-tostring
    routes.set("/rate-counter/increment/entry-parameter-calls-7.1.17-ToString", () => {
      let sentinel;
      const test = () => {
        sentinel = Symbol('sentinel');
        const entry = {
          toString() {
            throw sentinel;
          }
        }
        let rc = new RateCounter("rc");
        rc.increment(entry, 1)
      }
      error = assertThrows(test)
      if (error) { return error }
      try {
        test()
      } catch (thrownError) {
        console.log({ thrownError })
        error = assert(thrownError, sentinel, 'thrownError === sentinel')
        if (error) { return error }
      }
      error = assertThrows(() => {
        let rc = new RateCounter("rc");
        rc.increment(Symbol(), 1)
      }, Error, `can't convert symbol to string`)
      if (error) { return error }
      return pass('ok')
    });
    routes.set("/rate-counter/increment/entry-parameter-not-supplied", () => {
      error = assertThrows(() => {
        let rc = new RateCounter("rc");
        rc.increment()
      }, Error, `increment: At least 2 arguments required, but only 0 passed`)
      if (error) { return error }
      return pass('ok')
    });
    routes.set("/rate-counter/increment/delta-parameter-not-supplied", () => {
      error = assertThrows(() => {
        let rc = new RateCounter("rc");
        rc.increment("entry")
      }, Error, `increment: At least 2 arguments required, but only 1 passed`)
      if (error) { return error }
      return pass('ok')
    });
    routes.set("/rate-counter/increment/delta-parameter-negative", () => {
      error = assertThrows(() => {
        let rc = new RateCounter("rc");
        rc.increment("entry", -1)
      }, Error, `increment: delta parameter is an invalid value, only positive numbers can be used for delta values.`)
      if (error) { return error }
      return pass('ok')
    });
    routes.set("/rate-counter/increment/delta-parameter-infinity", () => {
      error = assertThrows(() => {
        let rc = new RateCounter("rc");
        rc.increment("entry", Infinity)
      }, Error, `increment: delta parameter is an invalid value, only positive numbers can be used for delta values.`)
      if (error) { return error }
      return pass('ok')
    });
    routes.set("/rate-counter/increment/delta-parameter-NaN", () => {
      error = assertThrows(() => {
        let rc = new RateCounter("rc");
        rc.increment("entry", NaN)
      }, Error, `increment: delta parameter is an invalid value, only positive numbers can be used for delta values.`)
      if (error) { return error }
      return pass('ok')
    });
    routes.set("/rate-counter/increment/returns-undefined", () => {
      let rc = new RateCounter("rc");
      error = assert(rc.increment('meow', 1), undefined, "rc.increment('meow', 1)")
      if (error) { return error }
      return pass('ok')
    });
  }

  // RateCounter lookupRate method
  // lookupRate(entry: string, window: [1, 10, 60]): number;
  {
    routes.set("/rate-counter/lookupRate/called-as-constructor", () => {
      error = assertThrows(() => {
        new RateCounter.prototype.lookupRate('entry', 1)
      }, Error, `RateCounter.prototype.lookupRate is not a constructor`)
      if (error) { return error }
      return pass('ok')
    });
    // Ensure we correctly coerce the parameter to a string as according to
    // https://tc39.es/ecma262/#sec-tostring
    routes.set("/rate-counter/lookupRate/entry-parameter-calls-7.1.17-ToString", () => {
      let sentinel;
      const test = () => {
        sentinel = Symbol('sentinel');
        const entry = {
          toString() {
            throw sentinel;
          }
        }
        let rc = new RateCounter("rc");
        rc.lookupRate(entry, 1)
      }
      error = assertThrows(test)
      if (error) { return error }
      try {
        test()
      } catch (thrownError) {
        console.log({ thrownError })
        error = assert(thrownError, sentinel, 'thrownError === sentinel')
        if (error) { return error }
      }
      error = assertThrows(() => {
        let rc = new RateCounter("rc");
        rc.lookupRate(Symbol(), 1)
      }, Error, `can't convert symbol to string`)
      if (error) { return error }
      return pass('ok')
    });
    routes.set("/rate-counter/lookupRate/entry-parameter-not-supplied", () => {
      error = assertThrows(() => {
        let rc = new RateCounter("rc");
        rc.lookupRate()
      }, Error, `lookupRate: At least 2 arguments required, but only 0 passed`)
      if (error) { return error }
      return pass('ok')
    });
    routes.set("/rate-counter/lookupRate/window-parameter-not-supplied", () => {
      error = assertThrows(() => {
        let rc = new RateCounter("rc");
        rc.lookupRate("entry")
      }, Error, `lookupRate: At least 2 arguments required, but only 1 passed`)
      if (error) { return error }
      return pass('ok')
    });
    routes.set("/rate-counter/lookupRate/window-parameter-negative", () => {
      error = assertThrows(() => {
        let rc = new RateCounter("rc");
        rc.lookupRate("entry", -1)
      }, Error, `lookupRate: window parameter must be either: 1, 10, or 60`)
      if (error) { return error }
      return pass('ok')
    });
    routes.set("/rate-counter/lookupRate/window-parameter-infinity", () => {
      error = assertThrows(() => {
        let rc = new RateCounter("rc");
        rc.lookupRate("entry", Infinity)
      }, Error, `lookupRate: window parameter must be either: 1, 10, or 60`)
      if (error) { return error }
      return pass('ok')
    });
    routes.set("/rate-counter/lookupRate/window-parameter-NaN", () => {
      error = assertThrows(() => {
        let rc = new RateCounter("rc");
        rc.lookupRate("entry", NaN)
      }, Error, `lookupRate: window parameter must be either: 1, 10, or 60`)
      if (error) { return error }
      return pass('ok')
    });
    routes.set("/rate-counter/lookupRate/returns-number", () => {
      let rc = new RateCounter("rc");
      error = assert(typeof rc.lookupRate('meow', 1), "number", `typeof rc.lookupRate('meow', 1)`)
      if (error) { return error }
      return pass('ok')
    });
  }

  // RateCounter lookupCount method
  // lookupCount(entry: string, duration: [10, 20, 30, 40, 50, 60]): number;
  {
    routes.set("/rate-counter/lookupCount/called-as-constructor", () => {
      error = assertThrows(() => {
        new RateCounter.prototype.lookupCount('entry', 1)
      }, Error, `RateCounter.prototype.lookupCount is not a constructor`)
      if (error) { return error }
      return pass('ok')
    });
    // Ensure we correctly coerce the parameter to a string as according to
    // https://tc39.es/ecma262/#sec-tostring
    routes.set("/rate-counter/lookupCount/entry-parameter-calls-7.1.17-ToString", () => {
      let sentinel;
      const test = () => {
        sentinel = Symbol('sentinel');
        const entry = {
          toString() {
            throw sentinel;
          }
        }
        let rc = new RateCounter("rc");
        rc.lookupCount(entry, 1)
      }
      error = assertThrows(test)
      if (error) { return error }
      try {
        test()
      } catch (thrownError) {
        console.log({ thrownError })
        error = assert(thrownError, sentinel, 'thrownError === sentinel')
        if (error) { return error }
      }
      error = assertThrows(() => {
        let rc = new RateCounter("rc");
        rc.lookupCount(Symbol(), 1)
      }, Error, `can't convert symbol to string`)
      if (error) { return error }
      return pass('ok')
    });
    routes.set("/rate-counter/lookupCount/entry-parameter-not-supplied", () => {
      error = assertThrows(() => {
        let rc = new RateCounter("rc");
        rc.lookupCount()
      }, Error, `lookupCount: At least 2 arguments required, but only 0 passed`)
      if (error) { return error }
      return pass('ok')
    });
    routes.set("/rate-counter/lookupCount/duration-parameter-not-supplied", () => {
      error = assertThrows(() => {
        let rc = new RateCounter("rc");
        rc.lookupCount("entry")
      }, Error, `lookupCount: At least 2 arguments required, but only 1 passed`)
      if (error) { return error }
      return pass('ok')
    });
    routes.set("/rate-counter/lookupCount/duration-parameter-negative", () => {
      error = assertThrows(() => {
        let rc = new RateCounter("rc");
        rc.lookupCount("entry", -1)
      }, Error, `lookupCount: duration parameter must be either: 10, 20, 30, 40, 50, or 60`)
      if (error) { return error }
      return pass('ok')
    });
    routes.set("/rate-counter/lookupCount/duration-parameter-infinity", () => {
      error = assertThrows(() => {
        let rc = new RateCounter("rc");
        rc.lookupCount("entry", Infinity)
      }, Error, `lookupCount: duration parameter must be either: 10, 20, 30, 40, 50, or 60`)
      if (error) { return error }
      return pass('ok')
    });
    routes.set("/rate-counter/lookupCount/duration-parameter-NaN", () => {
      error = assertThrows(() => {
        let rc = new RateCounter("rc");
        rc.lookupCount("entry", NaN)
      }, Error, `lookupCount: duration parameter must be either: 10, 20, 30, 40, 50, or 60`)
      if (error) { return error }
      return pass('ok')
    });
    routes.set("/rate-counter/lookupCount/returns-number", () => {
      let rc = new RateCounter("rc");
      error = assert(typeof rc.lookupCount('meow', 10), "number", `typeof rc.lookupCount('meow', 1)`)
      if (error) { return error }
      return pass('ok')
    });
  }
}

// PenaltyBox
{
  routes.set("/penalty-box/interface", () => {

    console.log(111, typeof PenaltyBox)
    let actual = Reflect.ownKeys(PenaltyBox)
    let expected = ["prototype", "length", "name"]
    error = assert(actual, expected, `Reflect.ownKeys(PenaltyBox)`)
    if (error) { return error }

    // Check the prototype descriptors are correct
    {
      actual = Reflect.getOwnPropertyDescriptor(PenaltyBox, 'prototype')
      expected = {
        "value": PenaltyBox.prototype,
        "writable": false,
        "enumerable": false,
        "configurable": false
      }
      error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(PenaltyBox, 'prototype')`)
      if (error) { return error }
    }

    // Check the constructor function's defined parameter length is correct
    {
      actual = Reflect.getOwnPropertyDescriptor(PenaltyBox, 'length')
      expected = {
        "value": 0,
        "writable": false,
        "enumerable": false,
        "configurable": true
      }
      error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(PenaltyBox, 'length')`)
      if (error) { return error }
    }

    // Check the constructor function's name is correct
    {
      actual = Reflect.getOwnPropertyDescriptor(PenaltyBox, 'name')
      expected = {
        "value": "PenaltyBox",
        "writable": false,
        "enumerable": false,
        "configurable": true
      }
      error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(PenaltyBox, 'name')`)
      if (error) { return error }
    }

    // Check the prototype has the correct keys
    {
      actual = Reflect.ownKeys(PenaltyBox.prototype)
      expected = ["constructor", "add", "has", Symbol.toStringTag]
      error = assert(actual, expected, `Reflect.ownKeys(PenaltyBox.prototype)`)
      if (error) { return error }
    }

    // Check the constructor on the prototype is correct
    {
      actual = Reflect.getOwnPropertyDescriptor(PenaltyBox.prototype, 'constructor')
      expected = { "writable": true, "enumerable": false, "configurable": true, value: PenaltyBox.prototype.constructor }
      error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(PenaltyBox.prototype, 'constructor')`)
      if (error) { return error }

      error = assert(typeof PenaltyBox.prototype.constructor, 'function', `typeof PenaltyBox.prototype.constructor`)
      if (error) { return error }

      actual = Reflect.getOwnPropertyDescriptor(PenaltyBox.prototype.constructor, 'length')
      expected = {
        "value": 0,
        "writable": false,
        "enumerable": false,
        "configurable": true
      }
      error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(PenaltyBox.prototype.constructor, 'length')`)
      if (error) { return error }

      actual = Reflect.getOwnPropertyDescriptor(PenaltyBox.prototype.constructor, 'name')
      expected = {
        "value": "PenaltyBox",
        "writable": false,
        "enumerable": false,
        "configurable": true
      }
      error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(PenaltyBox.prototype.constructor, 'name')`)
      if (error) { return error }
    }

    // Check the Symbol.toStringTag on the prototype is correct
    {
      actual = Reflect.getOwnPropertyDescriptor(PenaltyBox.prototype, Symbol.toStringTag)
      expected = { "writable": false, "enumerable": false, "configurable": true, value: "PenaltyBox" }
      error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(PenaltyBox.prototype, [Symbol.toStringTag])`)
      if (error) { return error }

      error = assert(typeof PenaltyBox.prototype[Symbol.toStringTag], 'string', `typeof PenaltyBox.prototype[Symbol.toStringTag]`)
      if (error) { return error }
    }

    // Check the add method has correct descriptors, length and name
    {
      actual = Reflect.getOwnPropertyDescriptor(PenaltyBox.prototype, 'add')
      expected = { "writable": true, "enumerable": true, "configurable": true, value: PenaltyBox.prototype.add }
      error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(PenaltyBox.prototype, 'add')`)
      if (error) { return error }

      error = assert(typeof PenaltyBox.prototype.add, 'function', `typeof PenaltyBox.prototype.add`)
      if (error) { return error }

      actual = Reflect.getOwnPropertyDescriptor(PenaltyBox.prototype.add, 'length')
      expected = {
        "value": 2,
        "writable": false,
        "enumerable": false,
        "configurable": true
      }
      error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(PenaltyBox.prototype.add, 'length')`)
      if (error) { return error }

      actual = Reflect.getOwnPropertyDescriptor(PenaltyBox.prototype.add, 'name')
      expected = {
        "value": "add",
        "writable": false,
        "enumerable": false,
        "configurable": true
      }
      error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(PenaltyBox.prototype.add, 'name')`)
      if (error) { return error }
    }

    // Check the has method has correct descriptors, length and name
    {
      actual = Reflect.getOwnPropertyDescriptor(PenaltyBox.prototype, 'has')
      expected = { "writable": true, "enumerable": true, "configurable": true, value: PenaltyBox.prototype.has }
      error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(PenaltyBox.prototype, 'has')`)
      if (error) { return error }

      error = assert(typeof PenaltyBox.prototype.has, 'function', `typeof PenaltyBox.prototype.has`)
      if (error) { return error }

      actual = Reflect.getOwnPropertyDescriptor(PenaltyBox.prototype.has, 'length')
      expected = {
        "value": 1,
        "writable": false,
        "enumerable": false,
        "configurable": true
      }
      error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(PenaltyBox.prototype.has, 'length')`)
      if (error) { return error }

      actual = Reflect.getOwnPropertyDescriptor(PenaltyBox.prototype.has, 'name')
      expected = {
        "value": "has",
        "writable": false,
        "enumerable": false,
        "configurable": true
      }
      error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(PenaltyBox.prototype.has, 'name')`)
      if (error) { return error }
    }

    return pass('ok')
  });

  // PenaltyBox constructor
  {
    routes.set("/penalty-box/constructor/called-as-regular-function", () => {
      error = assertThrows(() => {
        PenaltyBox()
      }, Error, `calling a builtin PenaltyBox constructor without new is forbidden`)
      if (error) { return error }
      return pass('ok')
    });
    routes.set("/penalty-box/constructor/called-as-constructor-no-arguments", () => {
      error = assertThrows(() => new PenaltyBox(), Error, `PenaltyBox constructor: At least 1 argument required, but only 0 passed`)
      if (error) { return error }
      return pass('ok')
    });
    // Ensure we correctly coerce the parameter to a string as according to
    // https://tc39.es/ecma262/#sec-tostring
    routes.set("/penalty-box/constructor/name-parameter-calls-7.1.17-ToString", () => {
      if (!isRunningLocally()) {
        let sentinel;
        const test = () => {
          sentinel = Symbol('sentinel');
          const name = {
            toString() {
              throw sentinel;
            }
          }
          new PenaltyBox(name)
        }
        error = assertThrows(test)
        if (error) { return error }
        try {
          test()
        } catch (thrownError) {
          error = assert(thrownError, sentinel, 'thrownError === sentinel')
          if (error) { return error }
        }
        error = assertThrows(() => {
          new PenaltyBox(Symbol())
        }, Error, `can't convert symbol to string`)
        if (error) { return error }
      }
      return pass('ok')
    });
    routes.set("/penalty-box/constructor/happy-path", () => {
      error = assert(new PenaltyBox("rc") instanceof PenaltyBox, true, `new PenaltyBox("rc") instanceof PenaltyBox`)
      if (error) { return error }
      return pass('ok')
    });
  }

  // PenaltyBox has method
  // has(entry: string): boolean;
  {
    routes.set("/penalty-box/has/called-as-constructor", () => {
      error = assertThrows(() => {
        new PenaltyBox.prototype.has('entry')
      }, Error, `PenaltyBox.prototype.has is not a constructor`)
      if (error) { return error }
      return pass('ok')
    });
    // Ensure we correctly coerce the parameter to a string as according to
    // https://tc39.es/ecma262/#sec-tostring
    routes.set("/penalty-box/has/entry-parameter-calls-7.1.17-ToString", () => {
      let sentinel;
      const test = () => {
        sentinel = Symbol('sentinel');
        const entry = {
          toString() {
            throw sentinel;
          }
        }
        let pb = new PenaltyBox("pb");
        pb.has(entry)
      }
      error = assertThrows(test)
      if (error) { return error }
      try {
        test()
      } catch (thrownError) {
        console.log({ thrownError })
        error = assert(thrownError, sentinel, 'thrownError === sentinel')
        if (error) { return error }
      }
      error = assertThrows(() => {
        let pb = new PenaltyBox("pb");
        pb.has(Symbol())
      }, Error, `can't convert symbol to string`)
      if (error) { return error }
      return pass('ok')
    });
    routes.set("/penalty-box/has/entry-parameter-not-supplied", () => {
      error = assertThrows(() => {
        let pb = new PenaltyBox("pb");
        pb.has()
      }, Error, `has: At least 1 argument required, but only 0 passed`)
      if (error) { return error }
      return pass('ok')
    });
    routes.set("/penalty-box/has/returns-boolean", () => {
      let pb = new PenaltyBox("pb");
      error = assert(pb.has('meow'), false, "pb.has('meow')")
      if (error) { return error }
      return pass('ok')
    });
  }

  // PenaltyBox add method
  // add(entry: string, timeToLive: number): void;
  {
    routes.set("/penalty-box/add/called-as-constructor", () => {
      error = assertThrows(() => {
        new PenaltyBox.prototype.add('entry', 1)
      }, Error, `PenaltyBox.prototype.add is not a constructor`)
      if (error) { return error }
      return pass('ok')
    });
    // Ensure we correctly coepbe the parameter to a string as according to
    // https://tc39.es/ecma262/#sec-tostring
    routes.set("/penalty-box/add/entry-parameter-calls-7.1.17-ToString", () => {
      let sentinel;
      const test = () => {
        sentinel = Symbol('sentinel');
        const entry = {
          toString() {
            throw sentinel;
          }
        }
        let pb = new PenaltyBox("pb");
        pb.add(entry, 1)
      }
      error = assertThrows(test)
      if (error) { return error }
      try {
        test()
      } catch (thrownError) {
        console.log({ thrownError })
        error = assert(thrownError, sentinel, 'thrownError === sentinel')
        if (error) { return error }
      }
      error = assertThrows(() => {
        let pb = new PenaltyBox("pb");
        pb.add(Symbol(), 1)
      }, Error, `can't convert symbol to string`)
      if (error) { return error }
      return pass('ok')
    });
    routes.set("/penalty-box/add/entry-parameter-not-supplied", () => {
      error = assertThrows(() => {
        let pb = new PenaltyBox("pb");
        pb.add()
      }, Error, `add: At least 2 arguments required, but only 0 passed`)
      if (error) { return error }
      return pass('ok')
    });
    routes.set("/penalty-box/add/timeToLive-parameter-not-supplied", () => {
      error = assertThrows(() => {
        let pb = new PenaltyBox("pb");
        pb.add("entry")
      }, Error, `add: At least 2 arguments required, but only 1 passed`)
      if (error) { return error }
      return pass('ok')
    });
    routes.set("/penalty-box/add/timeToLive-parameter-negative", () => {
      error = assertThrows(() => {
        let pb = new PenaltyBox("pb");
        pb.add("entry", -1)
      }, Error, `add: timeToLive parameter is an invalid value, only positive numbers can be used for timeToLive values.`)
      if (error) { return error }
      return pass('ok')
    });
    routes.set("/penalty-box/add/timeToLive-parameter-infinity", () => {
      error = assertThrows(() => {
        let pb = new PenaltyBox("pb");
        pb.add("entry", Infinity)
      }, Error, `add: timeToLive parameter is an invalid value, only positive numbers can be used for timeToLive values.`)
      if (error) { return error }
      return pass('ok')
    });
    routes.set("/penalty-box/add/timeToLive-parameter-NaN", () => {
      error = assertThrows(() => {
        let pb = new PenaltyBox("pb");
        pb.add("entry", NaN)
      }, Error, `add: timeToLive parameter is an invalid value, only positive numbers can be used for timeToLive values.`)
      if (error) { return error }
      return pass('ok')
    });
    routes.set("/penalty-box/add/returns-undefined", () => {
      let pb = new PenaltyBox("pb");
      error = assert(pb.add('meow', 1), undefined, `pb.add('meow', 1)`)
      if (error) { return error }
      return pass('ok')
    });
  }
}