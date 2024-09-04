/* eslint-env serviceworker */
import { routes } from "./routes.js";
import { pass, assert, assertThrows } from "./assertions.js";

let error;
routes.set("/Performance/interface", () => {
  let actual = Reflect.ownKeys(Performance);
  let expected = ["prototype", "length", "name"];
  error = assert(actual, expected, `Reflect.ownKeys(Performance)`);
  if (error) {
    return error;
  }

  // Check the prototype descriptors are correct
  {
    actual = Reflect.getOwnPropertyDescriptor(Performance, "prototype");
    expected = {
      value: Performance.prototype,
      writable: false,
      enumerable: false,
      configurable: false,
    };
    error = assert(
      actual,
      expected,
      `Reflect.getOwnPropertyDescriptor(Performance, 'prototype')`,
    );
    if (error) {
      return error;
    }
  }

  // Check the constructor function's defined parameter length is correct
  {
    actual = Reflect.getOwnPropertyDescriptor(Performance, "length");
    expected = {
      value: 0,
      writable: false,
      enumerable: false,
      configurable: true,
    };
    error = assert(
      actual,
      expected,
      `Reflect.getOwnPropertyDescriptor(Performance, 'length')`,
    );
    if (error) {
      return error;
    }
  }

  // Check the constructor function's name is correct
  {
    actual = Reflect.getOwnPropertyDescriptor(Performance, "name");
    expected = {
      value: "Performance",
      writable: false,
      enumerable: false,
      configurable: true,
    };
    error = assert(
      actual,
      expected,
      `Reflect.getOwnPropertyDescriptor(Performance, 'name')`,
    );
    if (error) {
      return error;
    }
  }

  // Check the prototype has the correct keys
  {
    actual = Reflect.ownKeys(Performance.prototype);
    expected = ["constructor", "timeOrigin", "now", Symbol.toStringTag];
    error = assert(actual, expected, `Reflect.ownKeys(Performance.prototype)`);
    if (error) {
      return error;
    }
  }

  // Check the constructor on the prototype is correct
  {
    actual = Reflect.getOwnPropertyDescriptor(
      Performance.prototype,
      "constructor",
    );
    expected = {
      writable: true,
      enumerable: false,
      configurable: true,
      value: Performance.prototype.constructor,
    };
    error = assert(
      actual,
      expected,
      `Reflect.getOwnPropertyDescriptor(Performance.prototype, 'constructor')`,
    );
    if (error) {
      return error;
    }

    error = assert(
      typeof Performance.prototype.constructor,
      "function",
      `typeof Performance.prototype.constructor`,
    );
    if (error) {
      return error;
    }

    actual = Reflect.getOwnPropertyDescriptor(
      Performance.prototype.constructor,
      "length",
    );
    expected = {
      value: 0,
      writable: false,
      enumerable: false,
      configurable: true,
    };
    error = assert(
      actual,
      expected,
      `Reflect.getOwnPropertyDescriptor(Performance.prototype.constructor, 'length')`,
    );
    if (error) {
      return error;
    }

    actual = Reflect.getOwnPropertyDescriptor(
      Performance.prototype.constructor,
      "name",
    );
    expected = {
      value: "Performance",
      writable: false,
      enumerable: false,
      configurable: true,
    };
    error = assert(
      actual,
      expected,
      `Reflect.getOwnPropertyDescriptor(Performance.prototype.constructor, 'name')`,
    );
    if (error) {
      return error;
    }
  }

  // Check the Symbol.toStringTag on the prototype is correct
  {
    actual = Reflect.getOwnPropertyDescriptor(
      Performance.prototype,
      Symbol.toStringTag,
    );
    expected = {
      value: "performance",
      writable: false,
      enumerable: false,
      configurable: true,
    };
    error = assert(
      actual,
      expected,
      `Reflect.getOwnPropertyDescriptor(Performance.prototype, [Symbol.toStringTag])`,
    );
    if (error) {
      return error;
    }

    error = assert(
      typeof Performance.prototype[Symbol.toStringTag],
      "string",
      `typeof Performance.prototype[Symbol.toStringTag]`,
    );
    if (error) {
      return error;
    }
  }

  // Check the timeOrigin property is correct
  {
    const descriptors = Reflect.getOwnPropertyDescriptor(
      Performance.prototype,
      "timeOrigin",
    );
    expected = { enumerable: true, configurable: true };
    error = assert(
      descriptors.enumerable,
      true,
      `Reflect.getOwnPropertyDescriptor(Performance, 'timeOrigin').enumerable`,
    );
    error = assert(
      descriptors.configurable,
      true,
      `Reflect.getOwnPropertyDescriptor(Performance, 'timeOrigin').configurable`,
    );
    error = assert(
      descriptors.value,
      undefined,
      `Reflect.getOwnPropertyDescriptor(Performance, 'timeOrigin').value`,
    );
    error = assert(
      descriptors.set,
      undefined,
      `Reflect.getOwnPropertyDescriptor(Performance, 'timeOrigin').set`,
    );
    error = assert(
      typeof descriptors.get,
      "function",
      `typeof Reflect.getOwnPropertyDescriptor(Performance, 'timeOrigin').get`,
    );
    if (error) {
      return error;
    }

    error = assert(
      typeof Performance.prototype.timeOrigin,
      "number",
      `typeof Performance.prototype.timeOrigin`,
    );
    if (error) {
      return error;
    }
  }

  // Check the now property is correct
  {
    actual = Reflect.getOwnPropertyDescriptor(Performance.prototype, "now");
    expected = {
      writable: true,
      enumerable: true,
      configurable: true,
      value: Performance.prototype.now,
    };
    error = assert(
      actual,
      expected,
      `Reflect.getOwnPropertyDescriptor(Performance, 'now')`,
    );
    if (error) {
      return error;
    }

    error = assert(
      typeof Performance.prototype.now,
      "function",
      `typeof Performance.prototype.now`,
    );
    if (error) {
      return error;
    }

    actual = Reflect.getOwnPropertyDescriptor(
      Performance.prototype.now,
      "length",
    );
    expected = {
      value: 0,
      writable: false,
      enumerable: false,
      configurable: true,
    };
    error = assert(
      actual,
      expected,
      `Reflect.getOwnPropertyDescriptor(Performance.prototype.now, 'length')`,
    );
    if (error) {
      return error;
    }

    actual = Reflect.getOwnPropertyDescriptor(
      Performance.prototype.now,
      "name",
    );
    expected = {
      value: "now",
      writable: false,
      enumerable: false,
      configurable: true,
    };
    error = assert(
      actual,
      expected,
      `Reflect.getOwnPropertyDescriptor(Performance.prototype.now, 'name')`,
    );
    if (error) {
      return error;
    }
  }

  return pass("ok");
});

routes.set("/globalThis.performance", () => {
  error = assert(
    globalThis.performance instanceof Performance,
    true,
    `globalThis.performance instanceof Performance`,
  );
  if (error) {
    return error;
  }

  return pass("ok");
});

routes.set("/globalThis.performance/now", () => {
  error = assertThrows(() => new performance.now());
  if (error) {
    return error;
  }

  error = assert(typeof performance.now(), "number");
  if (error) {
    return error;
  }

  error = assert(performance.now() > 0, true);
  if (error) {
    return error;
  }

  error = assert(Number.isNaN(performance.now()), false);
  if (error) {
    return error;
  }

  error = assert(Number.isFinite(performance.now()), true);
  if (error) {
    return error;
  }

  error = assert(performance.now() < Date.now(), true);
  if (error) {
    return error;
  }

  return pass("ok");
});

routes.set("/globalThis.performance/timeOrigin", () => {
  error = assert(typeof performance.timeOrigin, "number");
  if (error) {
    return error;
  }

  error = assert(performance.timeOrigin > 0, true);
  if (error) {
    return error;
  }

  error = assert(Number.isNaN(performance.timeOrigin), false);
  if (error) {
    return error;
  }

  error = assert(Number.isFinite(performance.timeOrigin), true);
  if (error) {
    return error;
  }

  error = assert(performance.timeOrigin < Date.now(), true);
  if (error) {
    return error;
  }

  return pass("ok");
});
