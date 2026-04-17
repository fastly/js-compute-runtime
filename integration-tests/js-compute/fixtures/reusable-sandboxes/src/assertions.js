export function strictEqual(actual, expected, message) {
  if (actual !== expected) {
    throw new Error(
      `Expected \`${JSON.stringify(actual)}\` to equal \`${JSON.stringify(expected)}\`${message ? '\n' + message : ''}`,
    );
  }
}

export function assert(truthy, maybeMessage) {
  if (!truthy) {
    throw new Error(`Assertion failed: ${maybeMessage}`);
  }
}

export { assert as ok };

export async function assertResolves(func) {
  try {
    await func();
  } catch (error) {
    throw new Error(
      `Expected \`${func.toString()}\` to resolve - Found it rejected: ${error.name}: ${error.message}`,
    );
  }
}

export async function assertRejects(func, errorClass, errorMessage) {
  try {
    await func();
  } catch (error) {
    if (errorClass) {
      if (error instanceof errorClass === false) {
        throw new Error(
          `Expected \`${func.toString()}\` to reject instance of \`${errorClass.name}\` - Found instance of \`${error.name}\``,
        );
      }
    }

    if (errorMessage) {
      if (error.message !== errorMessage) {
        throw new Error(
          `Expected \`${func.toString()}\` to reject error message of \`${errorMessage}\` - Found \`${error.message}\``,
        );
      }
    }

    return;
  }
  throw new Error(
    `Expected \`${func.toString()}\` to reject - Found it did not reject`,
  );
}

export function assertThrows(func, errorClass, errorMessage) {
  try {
    func();
  } catch (error) {
    if (errorClass) {
      if (error instanceof errorClass === false) {
        throw new Error(
          `Expected \`${func.toString()}\` to throw instance of \`${errorClass.name}\` - Found instance of \`${error.name}\`: ${error.message}\n${error.stack}`,
        );
      }
    }

    if (errorMessage) {
      if (error.message !== errorMessage) {
        throw new Error(
          `Expected \`${func.toString()}\` to throw error message of \`${errorMessage}\` - Found \`${error.message}\``,
        );
      }
    }

    return;
  }
  throw new Error(
    `Expected \`${func.toString()}\` to throw - Found it did not throw`,
  );
}

export function assertDoesNotThrow(func) {
  try {
    func();
  } catch (error) {
    throw new Error(
      `Expected \`${func.toString()}\` to not throw - Found it did throw: ${error.name}: ${error.message}`,
    );
  }
}

export function deepStrictEqual(a, b) {
  if (!deepEqual(a, b)) {
    throw new Error(
      `Expected ${a} to equal ${b}, got ${JSON.stringify(a, null, 2)}`,
    );
  }
}

export function deepEqual(a, b) {
  var aKeys;
  var bKeys;
  var typeA;
  var typeB;
  var key;
  var i;

  typeA = typeof a;
  typeB = typeof b;
  if (a === null || typeA !== 'object') {
    if (b === null || typeB !== 'object') {
      return a === b;
    }
    return false;
  }

  // Case: `a` is of type 'object'
  if (b === null || typeB !== 'object') {
    return false;
  }
  if (Array.isArray(a) && Array.isArray(b)) {
    if (a.length !== b.length) return false;
    for (let i = 0; i < a.length; i++) {
      if (!deepEqual(a[i], b[i])) {
        return false;
      }
    }
    return true;
  }
  if (Object.getPrototypeOf(a) !== Object.getPrototypeOf(b)) {
    return false;
  }
  if (a instanceof Date) {
    return a.getTime() === b.getTime();
  }
  if (a instanceof RegExp) {
    return a.source === b.source && a.flags === b.flags;
  }
  if (a instanceof Error) {
    if (a.message !== b.message || a.name !== b.name) {
      return false;
    }
  }

  aKeys = Object.keys(a);
  bKeys = Object.keys(b);
  if (aKeys.length !== bKeys.length) {
    return false;
  }
  aKeys.sort();
  bKeys.sort();

  // Cheap key test:
  for (i = 0; i < aKeys.length; i++) {
    if (aKeys[i] !== bKeys[i]) {
      return false;
    }
  }
  // Possibly expensive deep equality test for each corresponding key:
  for (i = 0; i < aKeys.length; i++) {
    key = aKeys[i];
    if (!deepEqual(a[key], b[key])) {
      return false;
    }
  }
  return typeA === typeB;
}
