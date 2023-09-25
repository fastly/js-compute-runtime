/* eslint-env serviceworker */
import { SecretStore, SecretStoreEntry } from 'fastly:secret-store'
import { pass, assert, assertThrows, assertRejects } from "./assertions.js";
import { routes } from "./routes.js";
import fc from 'fast-check';

// SecretStore
{
  routes.set("/secret-store/exposed-as-global", () => {
    let error = assert(typeof SecretStore, 'function', `typeof SecretStore`)
    if (error) { return error }
    return pass()
  });
  routes.set("/secret-store/interface", SecretStoreInterfaceTests);
  // SecretStore constructor
  {

    routes.set("/secret-store/constructor/called-as-regular-function", () => {
      let error = assertThrows(() => {
        SecretStore()
      }, TypeError, `calling a builtin SecretStore constructor without new is forbidden`)
      if (error) { return error }
      return pass()
    });
    // https://tc39.es/ecma262/#sec-tostring
    routes.set("/secret-store/constructor/parameter-calls-7.1.17-ToString", () => {
      let sentinel;
      const test = () => {
        sentinel = Symbol();
        const name = {
          toString() {
            throw sentinel;
          }
        }
        new SecretStore(name)
      }
      let error = assertThrows(test)
      if (error) { return error }
      try {
        test()
      } catch (thrownError) {
        let error = assert(thrownError, sentinel, 'thrownError === sentinel')
        if (error) { return error }
      }
      error = assertThrows(() => new SecretStore(Symbol()), TypeError, `can't convert symbol to string`)
      if (error) { return error }
      return pass()
    });
    routes.set("/secret-store/constructor/empty-parameter", () => {
      let error = assertThrows(() => {
        new SecretStore()
      }, TypeError, `SecretStore constructor: At least 1 argument required, but only 0 passed`)
      if (error) { return error }
      return pass()
    });
    routes.set("/secret-store/constructor/found-store", () => {
      const store = createValidStore()
      let error = assert(store instanceof SecretStore, true, `store instanceof SecretStore`)
      if (error) { return error }
      return pass()
    });
    routes.set("/secret-store/constructor/missing-store", () => {
      let error = assertThrows(() => {
        new SecretStore('missing')
      }, Error, `SecretStore constructor: No SecretStore named 'missing' exists`)
      if (error) { return error }
      return pass()
    });
    routes.set("/secret-store/constructor/invalid-name", () => {
      // control Characters (\\u0000-\\u001F) are not allowed
      fc.configureGlobal({ verbose: true });
      const charactersOtherThanLettersNumbersDashesUnderscoresAndPeriods = fc.fullUnicode().filter((c) => {
        if (c == '-' || c == '_' || c == '.') {
          return false;
        }
        const code = c.charCodeAt(0);
        // number ascii
        if (code >= 48 && code <= 57) {
          return false;
        }
        // upper ascii
        if (code >= 65 && code <= 90) {
          return false;
        }
        // lower ascii
        if (code >= 97 && code <= 122) {
          return false;
        }
        return true;
      });
      let error;
      fc.assert(fc.property(charactersOtherThanLettersNumbersDashesUnderscoresAndPeriods, (character) => {
        error = assertThrows(() => {
          new SecretStore(character) 
        }, TypeError, `SecretStore constructor: name can contain only ascii alphanumeric characters, underscores, dashes, and ascii whitespace`);
        return error == undefined;
      }));
      if (error) { return error }

      // must be less than 256 characters
      error = assertThrows(() => {
        new SecretStore('a'.repeat(256))
      }, TypeError, `SecretStore constructor: name can not be more than 255 characters`)
      if (error) { return error }

      // empty string not allowed
      error = assertThrows(() => {
        new SecretStore('')
      }, TypeError, `SecretStore constructor: name can not be an empty string`)
      if (error) { return error }
      return pass()
    });
  }

  // SecretStore get method
  {
    routes.set("/secret-store/get/called-as-constructor", () => {
      let error = assertThrows(() => {
        new SecretStore.prototype.get('1')
      }, TypeError, `SecretStore.prototype.get is not a constructor`)
      if (error) { return error }
      return pass()
    });
    routes.set("/secret-store/get/called-unbound", () => {
      let error = assertThrows(() => {
        SecretStore.prototype.get.call(undefined, '1')
      }, TypeError, "Method get called on receiver that's not an instance of SecretStore")
      if (error) { return error }
      return pass()
    });
    // https://tc39.es/ecma262/#sec-tostring
    routes.set("/secret-store/get/key-parameter-calls-7.1.17-ToString", async () => {
      let sentinel;
      const test =  () => {
        sentinel = Symbol();
        const key = {
          toString() {
            throw sentinel;
          }
        }
        const store = createValidStore()
        return store.get(key)
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
        const store = createValidStore()
        return store.get(Symbol())
      }, TypeError, `can't convert symbol to string`)
      if (error) { return error }
      return pass()
    });
    routes.set("/secret-store/get/key-parameter-not-supplied", () => {
      let error = assertThrows(() => {
        const store = createValidStore()
        return store.get()
      }, TypeError, `get: At least 1 argument required, but only 0 passed`)
      if (error) { return error }
      return pass()
    });
    routes.set("/secret-store/get/key-parameter-empty-string", async () => {
      let error = await assertRejects(async () => {
        const store = createValidStore()
        return await store.get('')
      }, TypeError, `SecretStore key can not be an empty string`)
      if (error) { return error }
      return pass()
    });
    routes.set("/secret-store/get/key-parameter-255-character-string", async () => {
      const store = createValidStore()
      const key = 'a'.repeat(255)
      let result = store.get(key)
      let error = assert(result instanceof Promise, true, `store.get(key) instanceof Promise`)
      result = await result
      error = assert(result instanceof SecretStoreEntry, true, `(await store.get(key)) instanceof SecretStoreEntry`)
      if (error) { return error }
      return pass()
    });
    routes.set("/secret-store/get/key-parameter-256-character-string", async () => {
      let error = await assertRejects(async () => {
        const store = createValidStore()
        const key = 'a'.repeat(256)
        return store.get(key)
      }, TypeError, `SecretStore key can not be more than 255 characters`)
      if (error) { return error }
      return pass()
    });
    routes.set("/secret-store/get/key-parameter-invalid-string", async () => {
      const charactersOtherThanLettersNumbersDashesUnderscoresAndPeriods = fc.fullUnicode().filter((c) => {
        if (c == '-' || c == '_' || c == '.') {
          return false;
        }
        const code = c.charCodeAt(0);
        // number ascii
        if (code >= 48 && code <= 57) {
          return false;
        }
        // upper ascii
        if (code >= 65 && code <= 90) {
          return false;
        }
        // lower ascii
        if (code >= 97 && code <= 122) {
          return false;
        }
        return true;
      });

      let error;
      fc.assert(fc.asyncProperty(charactersOtherThanLettersNumbersDashesUnderscoresAndPeriods, async (character) => {
        let error = await assertRejects(async () => {
          const store = createValidStore()
          return store.get(character)
        }, TypeError, `SecretStore key can contain only ascii alphanumeric characters, underscores, dashes, and ascii whitespace`);
        return error == undefined;
      }));
      if (error) { return error }
      return pass()
    });
    routes.set("/secret-store/get/key-does-not-exist-returns-null", async () => {
      let store = createValidStore()
      let result = await store.get(Math.random())
      let error = assert(result, null, `store.get(Math.random())`)
      if (error) { return error }
      return pass()
    });
    routes.set("/secret-store/get/key-exists", async () => {
      let store = createValidStore()
      let result = await store.get('first')
      let error = assert(result instanceof SecretStoreEntry, true, `(store.get(key) instanceof SecretStoreEntry)`)
      if (error) { return error }
      return pass()
    });
  }
}
// SecretStoreEntry
{
  routes.set("/secret-store-entry/interface", () => {
    return SecretStoreEntryInterfaceTests()
  });
  routes.set("/secret-store-entry/plaintext", async () => {
    let store = createValidStore()
    let secret = await store.get('first')
    let result = secret.plaintext();
    let error = assert(result, 'This is also some secret data', `(await store.get('first')).plaintext()`)
    if (error) { return error }
    return pass()
  });
}
function SecretStoreEntryInterfaceTests() {
  let actual = Reflect.ownKeys(SecretStoreEntry)
  let expected = ["prototype", "length", "name"]
  let error = assert(actual, expected, `Reflect.ownKeys(SecretStoreEntry)`)
  if (error) { return error }

  actual = Reflect.getOwnPropertyDescriptor(SecretStoreEntry, 'prototype')
  expected = {
    "value": SecretStoreEntry.prototype,
    "writable": false,
    "enumerable": false,
    "configurable": false
  }
  error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(SecretStoreEntry, 'prototype')`)
  if (error) { return error }

  actual = Reflect.getOwnPropertyDescriptor(SecretStoreEntry, 'length')
  expected = {
    "value": 0,
    "writable": false,
    "enumerable": false,
    "configurable": true
  }
  error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(SecretStoreEntry, 'length')`)
  if (error) { return error }

  actual = Reflect.getOwnPropertyDescriptor(SecretStoreEntry, 'name')
  expected = {
    "value": "SecretStoreEntry",
    "writable": false,
    "enumerable": false,
    "configurable": true
  }
  error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(SecretStoreEntry, 'name')`)
  if (error) { return error }

  actual = Reflect.ownKeys(SecretStoreEntry.prototype)
  expected = ["constructor", "plaintext"]
  error = assert(actual, expected, `Reflect.ownKeys(SecretStoreEntry.prototype)`)
  if (error) { return error }

  actual = Reflect.getOwnPropertyDescriptor(SecretStoreEntry.prototype, 'constructor')
  expected = { "writable": true, "enumerable": false, "configurable": true, value: SecretStoreEntry.prototype.constructor }
  error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(SecretStoreEntry.prototype, 'constructor')`)
  if (error) { return error }

  actual = Reflect.getOwnPropertyDescriptor(SecretStoreEntry.prototype, 'plaintext')
  expected = { "writable": true, "enumerable": true, "configurable": true, value: SecretStoreEntry.prototype.plaintext }
  error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(SecretStoreEntry.prototype, 'plaintext')`)
  if (error) { return error }

  error = assert(typeof SecretStoreEntry.prototype.constructor, 'function', `typeof SecretStoreEntry.prototype.constructor`)
  if (error) { return error }
  error = assert(typeof SecretStoreEntry.prototype.plaintext, 'function', `typeof SecretStoreEntry.prototype.plaintext`)
  if (error) { return error }

  actual = Reflect.getOwnPropertyDescriptor(SecretStoreEntry.prototype.constructor, 'length')
  expected = {
    "value": 0,
    "writable": false,
    "enumerable": false,
    "configurable": true
  }
  error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(SecretStoreEntry.prototype.constructor, 'length')`)
  if (error) { return error }

  actual = Reflect.getOwnPropertyDescriptor(SecretStoreEntry.prototype.constructor, 'name')
  expected = {
    "value": "SecretStoreEntry",
    "writable": false,
    "enumerable": false,
    "configurable": true
  }
  error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(SecretStoreEntry.prototype.constructor, 'name')`)
  if (error) { return error }

  actual = Reflect.getOwnPropertyDescriptor(SecretStoreEntry.prototype.plaintext, 'length')
  expected = {
    "value": 0,
    "writable": false,
    "enumerable": false,
    "configurable": true
  }
  error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(SecretStoreEntry.prototype.plaintext, 'length')`)
  if (error) { return error }

  actual = Reflect.getOwnPropertyDescriptor(SecretStoreEntry.prototype.plaintext, 'name')
  expected = {
    "value": "plaintext",
    "writable": false,
    "enumerable": false,
    "configurable": true
  }
  error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(SecretStoreEntry.prototype.plaintext, 'name')`)
  if (error) { return error }

  return pass()
}

function SecretStoreInterfaceTests() {
  let actual = Reflect.ownKeys(SecretStore)
  let expected = ["prototype", "length", "name"]
  let error = assert(actual, expected, `Reflect.ownKeys(SecretStore)`)
  if (error) { return error }

  actual = Reflect.getOwnPropertyDescriptor(SecretStore, 'prototype')
  expected = {
    "value": SecretStore.prototype,
    "writable": false,
    "enumerable": false,
    "configurable": false
  }
  error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(SecretStore, 'prototype')`)
  if (error) { return error }

  actual = Reflect.getOwnPropertyDescriptor(SecretStore, 'length')
  expected = {
    "value": 1,
    "writable": false,
    "enumerable": false,
    "configurable": true
  }
  error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(SecretStore, 'length')`)
  if (error) { return error }

  actual = Reflect.getOwnPropertyDescriptor(SecretStore, 'name')
  expected = {
    "value": "SecretStore",
    "writable": false,
    "enumerable": false,
    "configurable": true
  }
  error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(SecretStore, 'name')`)
  if (error) { return error }

  actual = Reflect.ownKeys(SecretStore.prototype)
  expected = ["constructor", "get"]
  error = assert(actual, expected, `Reflect.ownKeys(SecretStore.prototype)`)
  if (error) { return error }

  actual = Reflect.getOwnPropertyDescriptor(SecretStore.prototype, 'constructor')
  expected = { "writable": true, "enumerable": false, "configurable": true, value: SecretStore.prototype.constructor }
  error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(SecretStore.prototype, 'constructor')`)
  if (error) { return error }

  actual = Reflect.getOwnPropertyDescriptor(SecretStore.prototype, 'get')
  expected = { "writable": true, "enumerable": true, "configurable": true, value: SecretStore.prototype.get }
  error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(SecretStore.prototype, 'get')`)
  if (error) { return error }

  error = assert(typeof SecretStore.prototype.constructor, 'function', `typeof SecretStore.prototype.constructor`)
  if (error) { return error }
  error = assert(typeof SecretStore.prototype.get, 'function', `typeof SecretStore.prototype.get`)
  if (error) { return error }

  actual = Reflect.getOwnPropertyDescriptor(SecretStore.prototype.constructor, 'length')
  expected = {
    "value": 1,
    "writable": false,
    "enumerable": false,
    "configurable": true
  }
  error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(SecretStore.prototype.constructor, 'length')`)
  if (error) { return error }

  actual = Reflect.getOwnPropertyDescriptor(SecretStore.prototype.constructor, 'name')
  expected = {
    "value": "SecretStore",
    "writable": false,
    "enumerable": false,
    "configurable": true
  }
  error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(SecretStore.prototype.constructor, 'name')`)
  if (error) { return error }

  actual = Reflect.getOwnPropertyDescriptor(SecretStore.prototype.get, 'length')
  expected = {
    "value": 1,
    "writable": false,
    "enumerable": false,
    "configurable": true
  }
  error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(SecretStore.prototype.get, 'length')`)
  if (error) { return error }

  actual = Reflect.getOwnPropertyDescriptor(SecretStore.prototype.get, 'name')
  expected = {
    "value": "get",
    "writable": false,
    "enumerable": false,
    "configurable": true
  }
  error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(SecretStore.prototype.get, 'name')`)
  if (error) { return error }

  return pass()
}

function createValidStore() {
  return new SecretStore('example-test-secret-store')
}
