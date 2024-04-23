/* eslint-env serviceworker */

import { Dictionary } from 'fastly:dictionary'
import { routes } from "./routes.js";

// Dictionary
{
  routes.set("/dictionary/exposed-as-global", () => {
    let error = assert(typeof Dictionary, 'function', `typeof Dictionary`)
    if (error) { return error }
    return pass()
  });
  routes.set("/dictionary/interface", dictionaryInterfaceTests);
  // Dictionary constructor
  {

    routes.set("/dictionary/constructor/called-as-regular-function", () => {
      let error = assertThrows(() => {
        Dictionary()
      }, TypeError, `calling a builtin Dictionary constructor without new is forbidden`)
      if (error) { return error }
      return pass()
    });
    // https://tc39.es/ecma262/#sec-tostring
    routes.set("/dictionary/constructor/parameter-calls-7.1.17-ToString", () => {
      let sentinel = Symbol();
      const test = () => {
        const name = {
          toString() {
            throw sentinel;
          }
        }
        new Dictionary(name)
      }
      let error = assertThrows(test)
      if (error) { return error }
      try {
        test()
      } catch (thrownError) {
        let error = assert(thrownError, sentinel, 'thrownError === sentinel')
        if (error) { return error }
      }
      error = assertThrows(() => new Dictionary(Symbol()), TypeError, `can't convert symbol to string`)
      if (error) { return error }
      return pass()
    });
    routes.set("/dictionary/constructor/empty-parameter", () => {
      let error = assertThrows(() => {
        new Dictionary()
      }, TypeError, `Dictionary constructor: At least 1 argument required, but only 0 passed`)
      if (error) { return error }
      return pass()
    });
    routes.set("/dictionary/constructor/found", () => {
      const store = createValidDictionary()
      let error = assert(store instanceof Dictionary, true, `store instanceof Dictionary`)
      if (error) { return error }
      return pass()
    });
    routes.set("/dictionary/constructor/invalid-name", () => {
      // control Characters (\\u0000-\\u001F) are not allowed
      const invalidNames = [
        '1', '-', ' ', 'Ä'
      ];
      for (const name of invalidNames) {
        let error = assertThrows(() => {
          new Dictionary(name)
        }, TypeError, `Dictionary constructor: name must start with an ascii alpabetical character`)
        if (error) { return error }
      }
      let error = assertThrows(() => {
        new Dictionary('aÄ')
      }, TypeError, `Dictionary constructor: name can contain only ascii alphanumeric characters, underscores, and ascii whitespace`)
      if (error) { return error }

      // must be less than 256 characters
      error = assertThrows(() => {
        new Dictionary('a'.repeat(256))
      }, TypeError, `Dictionary constructor: name can not be more than 255 characters`)
      if (error) { return error }

      // empty string not allowed
      error = assertThrows(() => {
        new Dictionary('')
      }, TypeError, `Dictionary constructor: name can not be an empty string`)
      if (error) { return error }
      return pass()
    });
  }

  // Dictionary get method
  {
    routes.set("/dictionary/get/called-as-constructor", () => {
      let error = assertThrows(() => {
        new Dictionary.prototype.get('1')
      }, TypeError, `Dictionary.prototype.get is not a constructor`)
      if (error) { return error }
      return pass()
    });
    routes.set("/dictionary/get/called-unbound", () => {
      let error = assertThrows(() => {
        Dictionary.prototype.get.call(undefined, '1')
      }, TypeError, "Method get called on receiver that's not an instance of Dictionary")
      if (error) { return error }
      return pass()
    });
    // https://tc39.es/ecma262/#sec-tostring
    routes.set("/dictionary/get/key-parameter-calls-7.1.17-ToString", () => {
      let sentinel = Symbol();
      const test = () => {
        const key = {
          toString() {
            throw sentinel;
          }
        }
        const store = createValidDictionary()
        store.get(key)
      }
      let error = assertThrows(test)
      if (error) { return error }
      try {
        test()
      } catch (thrownError) {
        let error = assert(thrownError, sentinel, 'thrownError === sentinel')
        if (error) { return error }
      }
      error = assertThrows(() => {
        const store = createValidDictionary()
        store.get(Symbol())
      }, TypeError, `can't convert symbol to string`)
      if (error) { return error }
      return pass()
    });
    routes.set("/dictionary/get/key-parameter-not-supplied", () => {
      let error = assertThrows(() => {
        const store = createValidDictionary()
        store.get()
      }, TypeError, `get: At least 1 argument required, but only 0 passed`)
      if (error) { return error }
      return pass()
    });
    routes.set("/dictionary/get/key-parameter-empty-string", () => {
      let error = assertThrows(() => {
        const store = createValidDictionary()
        store.get('')
      }, TypeError, `Dictionary key can not be an empty string`)
      if (error) { return error }
      return pass()
    });
    routes.set("/dictionary/get/key-parameter-255-character-string", () => {
      let error = assertResolves(() => {
        const store = createValidDictionary()
        const key = 'a'.repeat(255)
        store.get(key)
      })
      if (error) { return error }
      return pass()
    });
    routes.set("/dictionary/get/key-parameter-256-character-string", () => {
      let error = assertThrows(() => {
        const store = createValidDictionary()
        const key = 'a'.repeat(256)
        store.get(key)
      }, TypeError, `Dictionary key can not be more than 255 characters`)
      if (error) { return error }
      return pass()
    });
    routes.set("/dictionary/get/key-does-not-exist-returns-null", () => {
      let store = createValidDictionary()
      let result = store.get(Math.random())
      let error = assert(result, null, `store.get(Math.random())`)
      if (error) { return error }
      return pass()
    });
    routes.set("/dictionary/get/key-exists", () => {
      let store = createValidDictionary()
      let result = store.get('twitter')
      let error = assert(result, "https://twitter.com/fastly", `store.get('twitter') === "https://twitter.com/fastly"`)
      if (error) { return error }
      return pass()
    });
  }
}

function dictionaryInterfaceTests() {
  let actual = Reflect.ownKeys(Dictionary)
  let expected = ["prototype", "length", "name"]
  let error = assert(actual, expected, `Reflect.ownKeys(Dictionary)`)
  if (error) { return error }

  actual = Reflect.getOwnPropertyDescriptor(Dictionary, 'prototype')
  expected = {
    "value": Dictionary.prototype,
    "writable": false,
    "enumerable": false,
    "configurable": false
  }
  error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(Dictionary, 'prototype')`)
  if (error) { return error }

  actual = Reflect.getOwnPropertyDescriptor(Dictionary, 'length')
  expected = {
    "value": 1,
    "writable": false,
    "enumerable": false,
    "configurable": true
  }
  error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(Dictionary, 'length')`)
  if (error) { return error }

  actual = Reflect.getOwnPropertyDescriptor(Dictionary, 'name')
  expected = {
    "value": "Dictionary",
    "writable": false,
    "enumerable": false,
    "configurable": true
  }
  error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(Dictionary, 'name')`)
  if (error) { return error }

  actual = Reflect.ownKeys(Dictionary.prototype)
  expected = ["constructor", "get"]
  error = assert(actual, expected, `Reflect.ownKeys(Dictionary.prototype)`)
  if (error) { return error }

  actual = Reflect.getOwnPropertyDescriptor(Dictionary.prototype, 'constructor')
  expected = { "writable": true, "enumerable": false, "configurable": true, value: Dictionary.prototype.constructor }
  error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(Dictionary.prototype, 'constructor')`)
  if (error) { return error }

  actual = Reflect.getOwnPropertyDescriptor(Dictionary.prototype, 'get')
  expected = { "writable": true, "enumerable": true, "configurable": true, value: Dictionary.prototype.get }
  error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(Dictionary.prototype, 'get')`)
  if (error) { return error }

  error = assert(typeof Dictionary.prototype.constructor, 'function', `typeof Dictionary.prototype.constructor`)
  if (error) { return error }
  error = assert(typeof Dictionary.prototype.get, 'function', `typeof Dictionary.prototype.get`)
  if (error) { return error }

  actual = Reflect.getOwnPropertyDescriptor(Dictionary.prototype.constructor, 'length')
  expected = {
    "value": 1,
    "writable": false,
    "enumerable": false,
    "configurable": true
  }
  error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(Dictionary.prototype.constructor, 'length')`)
  if (error) { return error }

  actual = Reflect.getOwnPropertyDescriptor(Dictionary.prototype.constructor, 'name')
  expected = {
    "value": "Dictionary",
    "writable": false,
    "enumerable": false,
    "configurable": true
  }
  error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(Dictionary.prototype.constructor, 'name')`)
  if (error) { return error }

  actual = Reflect.getOwnPropertyDescriptor(Dictionary.prototype.get, 'length')
  expected = {
    "value": 1,
    "writable": false,
    "enumerable": false,
    "configurable": true
  }
  error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(Dictionary.prototype.get, 'length')`)
  if (error) { return error }

  actual = Reflect.getOwnPropertyDescriptor(Dictionary.prototype.get, 'name')
  expected = {
    "value": "get",
    "writable": false,
    "enumerable": false,
    "configurable": true
  }
  error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(Dictionary.prototype.get, 'name')`)
  if (error) { return error }

  return pass()
}

function createValidDictionary() {
  return new Dictionary('aZ1 __ 2')
}

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

function assertResolves(func) {
  try {
    func()
  } catch (error) {
    return fail(`Expected \`${func.toString()}\` to resolve - Found it rejected: ${error.name}: ${error.message}`)
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
