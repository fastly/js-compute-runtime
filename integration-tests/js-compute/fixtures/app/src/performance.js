/* eslint-env serviceworker */
import { routes } from './routes.js';
import { assert, assertThrows } from './assertions.js';

routes.set('/Performance/interface', () => {
  let actual = Reflect.ownKeys(Performance);
  let expected = ['prototype', 'length', 'name'];
  assert(actual, expected, `Reflect.ownKeys(Performance)`);

  // Check the prototype descriptors are correct
  {
    actual = Reflect.getOwnPropertyDescriptor(Performance, 'prototype');
    expected = {
      value: Performance.prototype,
      writable: false,
      enumerable: false,
      configurable: false,
    };
    assert(
      actual,
      expected,
      `Reflect.getOwnPropertyDescriptor(Performance, 'prototype')`,
    );
  }

  // Check the constructor function's defined parameter length is correct
  {
    actual = Reflect.getOwnPropertyDescriptor(Performance, 'length');
    expected = {
      value: 0,
      writable: false,
      enumerable: false,
      configurable: true,
    };
    assert(
      actual,
      expected,
      `Reflect.getOwnPropertyDescriptor(Performance, 'length')`,
    );
  }

  // Check the constructor function's name is correct
  {
    actual = Reflect.getOwnPropertyDescriptor(Performance, 'name');
    expected = {
      value: 'Performance',
      writable: false,
      enumerable: false,
      configurable: true,
    };
    assert(
      actual,
      expected,
      `Reflect.getOwnPropertyDescriptor(Performance, 'name')`,
    );
  }

  // Check the prototype has the correct keys
  {
    actual = Reflect.ownKeys(Performance.prototype);
    expected = ['constructor', 'timeOrigin', 'now', Symbol.toStringTag];
    assert(actual, expected, `Reflect.ownKeys(Performance.prototype)`);
  }

  // Check the constructor on the prototype is correct
  {
    actual = Reflect.getOwnPropertyDescriptor(
      Performance.prototype,
      'constructor',
    );
    expected = {
      writable: true,
      enumerable: false,
      configurable: true,
      value: Performance.prototype.constructor,
    };
    assert(
      actual,
      expected,
      `Reflect.getOwnPropertyDescriptor(Performance.prototype, 'constructor')`,
    );

    assert(
      typeof Performance.prototype.constructor,
      'function',
      `typeof Performance.prototype.constructor`,
    );

    actual = Reflect.getOwnPropertyDescriptor(
      Performance.prototype.constructor,
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
      `Reflect.getOwnPropertyDescriptor(Performance.prototype.constructor, 'length')`,
    );

    actual = Reflect.getOwnPropertyDescriptor(
      Performance.prototype.constructor,
      'name',
    );
    expected = {
      value: 'Performance',
      writable: false,
      enumerable: false,
      configurable: true,
    };
    assert(
      actual,
      expected,
      `Reflect.getOwnPropertyDescriptor(Performance.prototype.constructor, 'name')`,
    );
  }

  // Check the Symbol.toStringTag on the prototype is correct
  {
    actual = Reflect.getOwnPropertyDescriptor(
      Performance.prototype,
      Symbol.toStringTag,
    );
    expected = {
      value: 'performance',
      writable: false,
      enumerable: false,
      configurable: true,
    };
    assert(
      actual,
      expected,
      `Reflect.getOwnPropertyDescriptor(Performance.prototype, [Symbol.toStringTag])`,
    );

    assert(
      typeof Performance.prototype[Symbol.toStringTag],
      'string',
      `typeof Performance.prototype[Symbol.toStringTag]`,
    );
  }

  // Check the timeOrigin property is correct
  {
    const descriptors = Reflect.getOwnPropertyDescriptor(
      Performance.prototype,
      'timeOrigin',
    );
    expected = { enumerable: true, configurable: true };
    assert(
      descriptors.enumerable,
      true,
      `Reflect.getOwnPropertyDescriptor(Performance, 'timeOrigin').enumerable`,
    );
    assert(
      descriptors.configurable,
      true,
      `Reflect.getOwnPropertyDescriptor(Performance, 'timeOrigin').configurable`,
    );
    assert(
      descriptors.value,
      undefined,
      `Reflect.getOwnPropertyDescriptor(Performance, 'timeOrigin').value`,
    );
    assert(
      descriptors.set,
      undefined,
      `Reflect.getOwnPropertyDescriptor(Performance, 'timeOrigin').set`,
    );
    assert(
      typeof descriptors.get,
      'function',
      `typeof Reflect.getOwnPropertyDescriptor(Performance, 'timeOrigin').get`,
    );

    assert(
      typeof Performance.prototype.timeOrigin,
      'number',
      `typeof Performance.prototype.timeOrigin`,
    );
  }

  // Check the now property is correct
  {
    actual = Reflect.getOwnPropertyDescriptor(Performance.prototype, 'now');
    expected = {
      writable: true,
      enumerable: true,
      configurable: true,
      value: Performance.prototype.now,
    };
    assert(
      actual,
      expected,
      `Reflect.getOwnPropertyDescriptor(Performance, 'now')`,
    );

    assert(
      typeof Performance.prototype.now,
      'function',
      `typeof Performance.prototype.now`,
    );

    actual = Reflect.getOwnPropertyDescriptor(
      Performance.prototype.now,
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
      `Reflect.getOwnPropertyDescriptor(Performance.prototype.now, 'length')`,
    );

    actual = Reflect.getOwnPropertyDescriptor(
      Performance.prototype.now,
      'name',
    );
    expected = {
      value: 'now',
      writable: false,
      enumerable: false,
      configurable: true,
    };
    assert(
      actual,
      expected,
      `Reflect.getOwnPropertyDescriptor(Performance.prototype.now, 'name')`,
    );
  }
});

routes.set('/globalThis.performance', () => {
  assert(
    globalThis.performance instanceof Performance,
    true,
    `globalThis.performance instanceof Performance`,
  );
});

routes.set('/globalThis.performance/now', () => {
  assertThrows(() => new performance.now());

  assert(typeof performance.now(), 'number');

  assert(performance.now() > 0, true);

  assert(Number.isNaN(performance.now()), false);

  assert(Number.isFinite(performance.now()), true);

  assert(performance.now() < Date.now(), true);
});

routes.set('/globalThis.performance/timeOrigin', () => {
  assert(typeof performance.timeOrigin, 'number');

  assert(performance.timeOrigin > 0, true);

  assert(Number.isNaN(performance.timeOrigin), false);

  assert(Number.isFinite(performance.timeOrigin), true);

  assert(performance.timeOrigin < Date.now(), true);
});
