addEventListener("fetch", () => {
  try {
    let error;
    // btoa
    {
      var everything = "";
      for (var i = 0; i < 256; i++) {
        everything += String.fromCharCode(i);
      }
      error = assert(btoa(everything), 'AAECAwQFBgcICQoLDA0ODxAREhMUFRYXGBkaGxwdHh8gISIjJCUmJygpKissLS4vMDEyMzQ1Njc4OTo7PD0+P0BBQkNERUZHSElKS0xNTk9QUVJTVFVWV1hZWltcXV5fYGFiY2RlZmdoaWprbG1ub3BxcnN0dXZ3eHl6e3x9fn+AgYKDhIWGh4iJiouMjY6PkJGSk5SVlpeYmZqbnJ2en6ChoqOkpaanqKmqq6ytrq+wsbKztLW2t7i5uru8vb6/wMHCw8TFxsfIycrLzM3Oz9DR0tPU1dbX2Nna29zd3t/g4eLj5OXm5+jp6uvs7e7v8PHy8/T19vf4+fr7/P3+/w==');
      if (error) { return error }

      error = assert(btoa(42), btoa('42'));
      if (error) { return error }
      error = assert(btoa(null), btoa('null'));
      if (error) { return error }
      error = assert(btoa({ x: 1 }), btoa('[object Object]'));
      if (error) { return error }

      error = assertThrows(() => btoa(), TypeError);
      if (error) { return error }
      error = assertThrows(() => btoa('🐱'));
      if (error) { return error }
    }

    // atob
    {
      error = assert(atob(""), "", `atob("")`)
      if (error) { return error }
      error = assert(atob("abcd"), 'i·\x1D')
      if (error) { return error }
      error = assert(atob(" abcd"), 'i·\x1D')
      if (error) { return error }
      error = assert(atob("abcd "), 'i·\x1D')
      if (error) { return error }
      error = assertThrows(() => atob(" abcd==="))
      if (error) { return error }
      error = assertThrows(() => atob("abcd=== "))
      if (error) { return error }
      error = assertThrows(() => atob("abcd ==="))
      if (error) { return error }
      error = assertThrows(() => atob("a"))
      if (error) { return error }
      error = assert(atob("ab"), 'i')
      if (error) { return error }
      error = assert(atob("abc"), 'i·')
      if (error) { return error }
      error = assertThrows(() => atob("abcde"))
      if (error) { return error }
      error = assertThrows(() => atob("𐀀"))
      if (error) { return error }
      error = assertThrows(() => atob("="))
      if (error) { return error }
      error = assertThrows(() => atob("=="))
      if (error) { return error }
      error = assertThrows(() => atob("==="))
      if (error) { return error }
      error = assertThrows(() => atob("===="))
      if (error) { return error }
      error = assertThrows(() => atob("====="))
      if (error) { return error }
      error = assertThrows(() => atob("a="))
      if (error) { return error }
      error = assertThrows(() => atob("a=="))
      if (error) { return error }
      error = assertThrows(() => atob("a==="))
      if (error) { return error }
      error = assertThrows(() => atob("a===="))
      if (error) { return error }
      error = assertThrows(() => atob("a====="))
      if (error) { return error }
      error = assertThrows(() => atob("ab="))
      if (error) { return error }
      error = assert(atob("ab=="), 'i')
      if (error) { return error }
      error = assertThrows(() => atob("ab==="))
      if (error) { return error }
      error = assertThrows(() => atob("ab===="))
      if (error) { return error }
      error = assertThrows(() => atob("ab====="))
      if (error) { return error }
      error = assert(atob("abc="), 'i·')
      if (error) { return error }
      error = assertThrows(() => atob("abc=="))
      if (error) { return error }
      error = assertThrows(() => atob("abc==="))
      if (error) { return error }
      error = assertThrows(() => atob("abc===="))
      if (error) { return error }
      error = assertThrows(() => atob("abc====="))
      if (error) { return error }
      error = assertThrows(() => atob("abcd="))
      if (error) { return error }
      error = assertThrows(() => atob("abcd=="))
      if (error) { return error }
      error = assertThrows(() => atob("abcd==="))
      if (error) { return error }
      error = assertThrows(() => atob("abcd===="))
      if (error) { return error }
      error = assertThrows(() => atob("abcd====="))
      if (error) { return error }
      error = assertThrows(() => atob("abcde="))
      if (error) { return error }
      error = assertThrows(() => atob("abcde=="))
      if (error) { return error }
      error = assertThrows(() => atob("abcde==="))
      if (error) { return error }
      error = assertThrows(() => atob("abcde===="))
      if (error) { return error }
      error = assertThrows(() => atob("abcde====="))
      if (error) { return error }
      error = assertThrows(() => atob("=a"))
      if (error) { return error }
      error = assertThrows(() => atob("=a="))
      if (error) { return error }
      error = assertThrows(() => atob("a=b"))
      if (error) { return error }
      error = assertThrows(() => atob("a=b="))
      if (error) { return error }
      error = assertThrows(() => atob("ab=c"))
      if (error) { return error }
      error = assertThrows(() => atob("ab=c="))
      if (error) { return error }
      error = assertThrows(() => atob("abc=d"))
      if (error) { return error }
      error = assertThrows(() => atob("abc=d="))
      if (error) { return error }
      error = assertThrows(() => atob("ab\u000Bcd"))
      if (error) { return error }
      error = assertThrows(() => atob("ab\u3000cd"))
      if (error) { return error }
      error = assertThrows(() => atob("ab\u3001cd"))
      if (error) { return error }
      error = assert(atob("ab\tcd"), 'i·\x1D')
      if (error) { return error }
      error = assert(atob("ab\ncd"), 'i·\x1D')
      if (error) { return error }
      error = assert(atob("ab\fcd"), 'i·\x1D')
      if (error) { return error }
      error = assert(atob("ab\rcd"), 'i·\x1D')
      if (error) { return error }
      error = assert(atob("ab cd"), 'i·\x1D')
      if (error) { return error }
      error = assertThrows(() => atob("ab\u00a0cd"))
      if (error) { return error }
      error = assert(atob("ab\t\n\f\r cd"), 'i·\x1D')
      if (error) { return error }
      error = assert(atob(" \t\n\f\r ab\t\n\f\r cd\t\n\f\r "), 'i·\x1D')
      if (error) { return error }
      error = assert(atob("ab\t\n\f\r =\t\n\f\r =\t\n\f\r "), 'i')
      if (error) { return error }
      error = assertThrows(() => atob("A"))
      if (error) { return error }
      error = assert(atob("/A"), 'ü')
      if (error) { return error }
      error = assert(atob("//A"), 'ÿð')
      if (error) { return error }
      error = assert(atob("///A"), 'ÿÿÀ')
      if (error) { return error }
      error = assertThrows(() => atob("////A"))
      if (error) { return error }
      error = assertThrows(() => atob("/"))
      if (error) { return error }
      error = assert(atob("A/"), '\x03')
      if (error) { return error }
      error = assert(atob("AA/"), '\x00\x0F')
      if (error) { return error }
      error = assertThrows(() => atob("AAAA/"))
      if (error) { return error }
      error = assert(atob("AAA/"), '\x00\x00?')
      if (error) { return error }
      error = assertThrows(() => atob("\u0000nonsense"))
      if (error) { return error }
      error = assertThrows(() => atob("abcd\u0000nonsense"))
      if (error) { return error }
      error = assert(atob("YQ"), 'a')
      if (error) { return error }
      error = assert(atob("YR"), 'a')
      if (error) { return error }
      error = assertThrows(() => atob("~~"))
      if (error) { return error }
      error = assertThrows(() => atob(".."))
      if (error) { return error }
      error = assertThrows(() => atob("--"))
      if (error) { return error }
      error = assertThrows(() => atob("__"))
      if (error) { return error }
    }

    return pass()
  } catch (error) {
    return fail(error.message)
  }
});


// Testing/Assertion functions //

function pass(message = '') {
  return new Response(message)
}

function fail(message = '') {
  return new Response(message, { status: 500 })
}

function assert(actual, expected, code) {
  if (!deepEqual(actual, expected)) {
    return fail(`Expected \`${code}\` to equal \`${JSON.stringify(expected)}\` - Found \`${JSON.stringify(actual)}\``)
  }
}

async function assertResolves(func) {
  try {
    await func()
  } catch (error) {
    return fail(`Expected \`${func.toString()}\` to resolve - Found it rejected: ${error.name}: ${error.message}`)
  }
}

async function assertRejects(func, errorClass, errorMessage) {
  try {
    await func()
    return fail(`Expected \`${func.toString()}\` to reject - Found it did not reject`)
  } catch (error) {
    if (errorClass) {
      if ((error instanceof errorClass) === false) {
        return fail(`Expected \`${func.toString()}\` to reject instance of \`${errorClass.name}\` - Found instance of \`${error.name}\``)
      }
    }

    if (errorMessage) {
      if (error.message !== errorMessage) {
        return fail(`Expected \`${func.toString()}\` to reject error message of \`${errorMessage}\` - Found \`${error.message}\``)
      }
    }
  }
}

function assertThrows(func, errorClass, errorMessage) {
  try {
    func()
    return fail(`Expected \`${func.toString()}\` to throw - Found it did not throw`)
  } catch (error) {
    if (errorClass) {
      if ((error instanceof errorClass) === false) {
        return fail(`Expected \`${func.toString()}\` to throw instance of \`${errorClass.name}\` - Found instance of \`${error.name}\``)
      }
    }

    if (errorMessage) {
      if (error.message !== errorMessage) {
        return fail(`Expected \`${func.toString()}\` to throw error message of \`${errorMessage}\` - Found \`${error.message}\``)
      }
    }
  }
}

// eslint-disable-next-line no-unused-vars
function assertDoesNotThrow(func) {
  try {
    func()
  } catch (error) {
    return fail(`Expected \`${func.toString()}\` to not throw - Found it did throw: ${error.name}: ${error.message}`)
  }
}

/**
* Tests for deep equality between two values.
*
* @param {*} a - first comparison value
* @param {*} b - second comparison value
* @returns {boolean} boolean indicating if `a` is deep equal to `b`
*
* @example
* var bool = deepEqual( [ 1, 2, 3 ], [ 1, 2, 3 ] );
* // returns true
*
* @example
* var bool = deepEqual( [ 1, 2, 3 ], [ 1, 2, '3' ] );
* // returns false
*
* @example
* var bool = deepEqual( { 'a': 2 }, { 'a': [ 2 ] } );
* // returns false
*
* @example
* var bool = deepEqual( [], {} );
* // returns false
*
* @example
* var bool = deepEqual( null, null );
* // returns true
*/
function deepEqual(a, b) {
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
  if (typeB !== 'object') {
    return false;
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