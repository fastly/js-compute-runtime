/// <reference path="../../../../../types/index.d.ts" />
/* eslint-env serviceworker */
import { Backend } from 'fastly:backend';
import { env } from 'fastly:env';
import { CacheOverride } from 'fastly:cache-override';
import { allowDynamicBackends } from "fastly:experimental";
import { pass, fail, assert, assertDoesNotThrow, assertThrows, assertRejects, assertResolves } from "../../../assertions.js";

addEventListener("fetch", event => {
  event.respondWith(app(event))
})
/**
* @param {FetchEvent} event
* @returns {Response}
*/
async function app(event) {
  try {
    const path = (new URL(event.request.url)).pathname;
    console.log(`path: ${path}`)
    console.log(`FASTLY_SERVICE_VERSION: ${env('FASTLY_SERVICE_VERSION')}`)
    if (routes.has(path)) {
      const routeHandler = routes.get(path);
      return await routeHandler()
    }
    return fail(`${path} endpoint does not exist`)
  } catch (error) {
    return fail(`The routeHandler threw an error: ${error.message}` + '\n' + error.stack)
  }
}

const routes = new Map();
routes.set('/', () => {
  routes.delete('/');
  let test_routes = Array.from(routes.keys())
  return new Response(JSON.stringify(test_routes), { 'headers': { 'content-type': 'application/json' } });
});

/// The backend name is already in use.


// implicit dynamic backend
{
  routes.set("/implicit-dynamic-backend/dynamic-backends-disabled", async () => {
    allowDynamicBackends(false);
    let error = await assertRejects(() => fetch('https://httpbin.org/headers'));
    if (error) { return error }
    return pass()
  });
  routes.set("/implicit-dynamic-backend/dynamic-backends-enabled", async () => {
    allowDynamicBackends(true);
    let error = await assertResolves(() => fetch('https://httpbin.org/headers'));
    if (error) { return error }
    error = await assertResolves(() => fetch('https://www.fastly.com'));
    if (error) { return error }
    return pass()
  });
  routes.set("/implicit-dynamic-backend/dynamic-backends-enabled-called-twice", async () => {
    allowDynamicBackends(true);
    let error = await assertResolves(() => fetch('https://httpbin.org/headers'));
    if (error) { return error }
    error = await assertResolves(() => fetch('https://httpbin.org/headers'));
    if (error) { return error }
    return pass()
  });
}

// explicit dynamic backend
{
  routes.set("/explicit-dynamic-backend/dynamic-backends-enabled-all-fields", async () => {
    allowDynamicBackends(true);
    let backend = createValidHttpBinBackend();
    let error = await assertResolves(() => fetch('https://httpbin.org/headers', {
      backend,
      cacheOverride: new CacheOverride("pass"),
    }));
    if (error) { return error }
    return pass()
  });
  routes.set("/explicit-dynamic-backend/dynamic-backends-enabled-minimal-fields", async () => {
    allowDynamicBackends(true);
    let backend = createValidFastlyBackend();
    let error = await assertResolves(() => fetch('https://www.fastly.com', {
      backend,
      cacheOverride: new CacheOverride("pass"),
    }));
    if (error) { return error }
    return pass()
  });
}

// Backend
{
  routes.set("/backend/interface", async () => {
    let actual = Reflect.ownKeys(Backend)
    let expected = ["prototype", "length", "name"]
    let error = assert(actual, expected, `Reflect.ownKeys(Backend)`)
    if (error) { return error }

    actual = Reflect.getOwnPropertyDescriptor(Backend, 'prototype')
    expected = {
      "value": Backend.prototype,
      "writable": false,
      "enumerable": false,
      "configurable": false
    }
    error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(Backend, 'prototype')`)
    if (error) { return error }

    actual = Reflect.getOwnPropertyDescriptor(Backend, 'length')
    expected = {
      "value": 1,
      "writable": false,
      "enumerable": false,
      "configurable": true
    }
    error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(Backend, 'length')`)
    if (error) { return error }

    actual = Reflect.getOwnPropertyDescriptor(Backend, 'name')
    expected = {
      "value": "Backend",
      "writable": false,
      "enumerable": false,
      "configurable": true
    }
    error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(Backend, 'name')`)
    if (error) { return error }

    actual = Reflect.ownKeys(Backend.prototype)
    expected = ["constructor", "toString"]
    error = assert(actual, expected, `Reflect.ownKeys(Backend.prototype)`)
    if (error) { return error }

    actual = Reflect.getOwnPropertyDescriptor(Backend.prototype, 'constructor')
    expected = { "writable": true, "enumerable": false, "configurable": true, value: Backend.prototype.constructor }
    error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(Backend.prototype, 'constructor')`)
    if (error) { return error }

    actual = Reflect.getOwnPropertyDescriptor(Backend.prototype, 'toString')
    expected = { "writable": true, "enumerable": true, "configurable": true, value: Backend.prototype.toString }
    error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(Backend.prototype, 'toString')`)
    if (error) { return error }

    error = assert(typeof Backend.prototype.constructor, 'function', `typeof Backend.prototype.constructor`)
    if (error) { return error }
    error = assert(typeof Backend.prototype.toString, 'function', `typeof Backend.prototype.toString`)
    if (error) { return error }

    actual = Reflect.getOwnPropertyDescriptor(Backend.prototype.constructor, 'length')
    expected = {
      "value": 1,
      "writable": false,
      "enumerable": false,
      "configurable": true
    }
    error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(Backend.prototype.constructor, 'length')`)
    if (error) { return error }

    actual = Reflect.getOwnPropertyDescriptor(Backend.prototype.constructor, 'name')
    expected = {
      "value": "Backend",
      "writable": false,
      "enumerable": false,
      "configurable": true
    }
    error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(Backend.prototype.constructor, 'name')`)
    if (error) { return error }

    actual = Reflect.getOwnPropertyDescriptor(Backend.prototype.toString, 'length')
    expected = {
      "value": 0,
      "writable": false,
      "enumerable": false,
      "configurable": true
    }
    error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(Backend.prototype.toString, 'length')`)
    if (error) { return error }

    actual = Reflect.getOwnPropertyDescriptor(Backend.prototype.toString, 'name')
    expected = {
      "value": "toString",
      "writable": false,
      "enumerable": false,
      "configurable": true
    }
    error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(Backend.prototype.toString, 'name')`)
    if (error) { return error }

    return pass()
  });

  // constructor
  {

    routes.set("/backend/constructor/called-as-regular-function", async () => {
      let error = assertThrows(() => {
        Backend()
      }, TypeError, `calling a builtin Backend constructor without new is forbidden`)
      if (error) { return error }
      return pass()
    });
    routes.set("/backend/constructor/empty-parameter", async () => {
      let error = assertThrows(() => {
        new Backend()
      }, TypeError, `Backend constructor: At least 1 argument required, but only 0 passed`)
      if (error) { return error }
      return pass()
    });
    routes.set("/backend/constructor/parameter-not-an-object", async () => {
      const constructorArguments = [true, false, 1, 1n, "hello", null, undefined, Symbol(), NaN]
      for (const argument of constructorArguments) {
        let error = assertThrows(() => {
          new Backend(argument)
        }, TypeError, `Backend constructor: configuration parameter must be an Object`)
        if (error) { return error }
      }
      return pass()
    });
    // name property
    {
      routes.set("/backend/constructor/parameter-name-property-null", async () => {
        let error = assertThrows(() => {
          new Backend({ name: null })
        }, TypeError, `Backend constructor: name can not be null or undefined`)
        if (error) { return error }
        return pass()
      });
      routes.set("/backend/constructor/parameter-name-property-undefined", async () => {
        let error = assertThrows(() => {
          new Backend({ name: undefined })
        }, TypeError, `Backend constructor: name can not be null or undefined`)
        if (error) { return error }
        return pass()
      });
      routes.set("/backend/constructor/parameter-name-property-too-long", async () => {
        let error = assertThrows(() => {
          new Backend({ name: "a".repeat(255) })
        }, TypeError, `Backend constructor: name can not be more than 254 characters`)
        if (error) { return error }
        return pass()
      });
      routes.set("/backend/constructor/parameter-name-property-empty-string", async () => {
        let error = assertThrows(() => {
          new Backend({ name: "" })
        }, TypeError, `Backend constructor: name can not be an empty string`)
        if (error) { return error }
        return pass()
      });
      // https://tc39.es/ecma262/#sec-tostring
      routes.set("/backend/constructor/parameter-name-property-calls-7.1.17-ToString", async () => {
        let sentinel;
        const test = () => {
          sentinel = Symbol();
          const name = {
            toString() {
              throw sentinel;
            }
          }
          new Backend({ name })
        }
        let error = assertThrows(test)
        if (error) { return error }
        try {
          test()
        } catch (thrownError) {
          let error = assert(thrownError, sentinel, 'thrownError === sentinel')
          if (error) { return error }
        }
        error = assertThrows(() => new Backend({ name: Symbol() }), TypeError, `can't convert symbol to string`)
        if (error) { return error }
        return pass()
      });
    }

    // target property
    {
      routes.set("/backend/constructor/parameter-target-property-null", async () => {
        let error = assertThrows(() => {
          new Backend({ name: 'a', target: null })
        }, TypeError, `Backend constructor: target can not be null or undefined`)
        if (error) { return error }
        return pass()
      });
      routes.set("/backend/constructor/parameter-target-property-undefined", async () => {
        let error = assertThrows(() => {
          new Backend({ name: 'a', target: undefined })
        }, TypeError, `Backend constructor: target can not be null or undefined`)
        if (error) { return error }
        return pass()
      });
      routes.set("/backend/constructor/parameter-target-property-empty-string", async () => {
        let error = assertThrows(() => {
          new Backend({ name: 'a', target: "" })
        }, TypeError, `Backend constructor: target can not be an empty string`)
        if (error) { return error }
        return pass()
      });
      // https://tc39.es/ecma262/#sec-tostring
      routes.set("/backend/constructor/parameter-target-property-calls-7.1.17-ToString", async () => {
        let sentinel;
        const test = () => {
          sentinel = Symbol();
          const target = {
            toString() {
              throw sentinel;
            }
          }
          new Backend({ name: 'a', target })
        }
        let error = assertThrows(test)
        if (error) { return error }
        try {
          test()
        } catch (thrownError) {
          let error = assert(thrownError, sentinel, 'thrownError === sentinel')
          if (error) { return error }
        }
        error = assertThrows(() => new Backend({ name: 'a', target: Symbol() }), TypeError, `can't convert symbol to string`)
        if (error) { return error }
        return pass()
      });

      routes.set("/backend/constructor/parameter-target-property-valid-host", async () => {
        const targets = [
          "www.fastly.com",
          "w-w-w.f-a-s-t-l-y.c-o-m",
          "a".repeat(63) + ".com",
          `${"a".repeat(63)}.${"a".repeat(63)}.${"a".repeat(63)}.${"a".repeat(57)}.com`,
          "ai",
          "w.a:1",
          "fastly.com:1",
          "fastly.com:80",
          "fastly.com:443",
          "fastly.com:65535",
          // Basic zero IPv4 address.
          "0.0.0.0",
          // Basic non-zero IPv4 address.
          "192.168.140.255",

          // TODO: These are commented out as the hostcall currently yields an error of "invalid authority" when given an ipv6 address
          // Localhost IPv6.
          // "::1",
          // Fully expanded IPv6 address.
          // "fd7a:115c:a1e0:ab12:4843:cd96:626b:430b",
          // IPv6 with elided fields in the middle.
          // "fd7a:115c::626b:430b",
          // IPv6 with elided fields at the end.
          // "fd7a:115c:a1e0:ab12:4843:cd96::",
          // IPv6 with single elided field at the end.
          // "fd7a:115c:a1e0:ab12:4843:cd96:626b::",
          // IPv6 with single elided field in the middle.
          // "fd7a:115c:a1e0::4843:cd96:626b:430b",
          // IPv6 with the trailing 32 bits written as IPv4 dotted decimal. (4in6)
          // "::ffff:192.168.140.255",
          // IPv6 with capital letters.
          // "FD9E:1A04:F01D::1",
        ];
        let i = 0;
        for (const target of targets) {
          let error = assertDoesNotThrow(() => {
            new Backend({ name: 'target-property-valid-host-' + i, target })
          })
          if (error) { return error }
          i++;
        }
        return pass()
      });

      routes.set("/backend/constructor/parameter-target-property-invalid-host", async () => {
        const targets = [
          "-www.fastly.com",
          "www.fastly.com-",
          "a".repeat(64) + ".com",
          `${"a".repeat(63)}.${"a".repeat(63)}.${"a".repeat(63)}.${"a".repeat(58)}.com`,
          "w$.com",
          "w.a:a",
          "fastly.com:-1",
          "fastly.com:0",
          "fastly.com:65536",
          // IPv4 address in windows-style "print all the digits" form.
          // "010.000.015.001",
          // IPv4 address with a silly amount of leading zeros.
          // "000001.00000002.00000003.000000004",
          // 4-in-6 with octet with leading zero
          // "::ffff:1.2.03.4", 
          // Basic zero IPv6 address.
          "::",
          // IPv6 with not enough fields
          "1:2:3:4:5:6:7",
          // IPv6 with too many fields
          "1:2:3:4:5:6:7:8:9",
          // IPv6 with 8 fields and a :: expander
          "1:2:3:4::5:6:7:8",
          // IPv6 with a field bigger than 2b
          "fe801::1",
          // IPv6 with non-hex values in field
          "fe80:tail:scal:e::",
          // IPv6 with a zone delimiter but no zone.
          "fe80::1%",
          // IPv6 (without ellipsis) with too many fields for trailing embedded IPv4.
          "ffff:ffff:ffff:ffff:ffff:ffff:ffff:192.168.140.255",
          // IPv6 (with ellipsis) with too many fields for trailing embedded IPv4.
          "ffff::ffff:ffff:ffff:ffff:ffff:ffff:192.168.140.255",
          // IPv6 with invalid embedded IPv4.
          "::ffff:192.168.140.bad",
          // IPv6 with multiple ellipsis ::.
          "fe80::1::1",
          // IPv6 with invalid non hex/colon character.
          "fe80:1?:1",
          // IPv6 with truncated bytes after single colon.
          "fe80:",
          ":::1",
          ":0:1",
          ":",
        ];
        let i = 0;
        for (const target of targets) {
          let error = assertThrows(() => {
            new Backend({ name: 'target-property-invalid-host-' + i, target })
          }, TypeError, `Backend constructor: target does not contain a valid IPv4, IPv6, or hostname address`)
          if (error) { return error }
          i++;
        }
        return pass()
      });
    }

    // ciphers property
    {
      routes.set("/backend/constructor/parameter-ciphers-property-empty-string", async () => {
        let error = assertThrows(() => {
          new Backend({ name: 'a', target: 'a', ciphers: "" })
        }, TypeError, `Backend constructor: ciphers can not be an empty string`)
        if (error) { return error }
        return pass()
      });
      routes.set("/backend/constructor/parameter-ciphers-property-invalid-cipherlist-string", async () => {
        let error = assertThrows(() => {
          new Backend({ name: 'a', target: 'a', ciphers: ",;'^%$#^" })
        }, TypeError, `Backend constructor: none of the provided ciphers are supported by Fastly. The list of supported ciphers is available on https://developer.fastly.com/learning/concepts/routing-traffic-to-fastly/#use-a-tls-configuration`)
        if (error) { return error }
        return pass()
      });
      const validCiphers = [
        "ALL",
        "DEFAULT",
        "COMPLEMENTOFDEFAULT",
        "aRSA",
        "RSA",
        "kEECDH",
        "kECDHE",
        "ECDH",
        "ECDHE",
        "EECDH",
        "TLSv1.2",
        "TLSv1.0",
        "SSLv3",
        "AES128",
        "AES256",
        "AES",
        "AESGCM",
        "CHACHA20",
        "3DES",
        "SHA1",
        "SHA",
        "SHA256",
        "SHA384",

        "DES-CBC3-SHA",
        "AES128-SHA",
        "AES256-SHA",
        "ECDHE-RSA-AES128-SHA",
        "ECDHE-RSA-AES256-SHA",
        "AES128-GCM-SHA256",
        "ECDHE-RSA-AES128-SHA256",
        "ECDHE-RSA-AES256-SHA384",
        "ECDHE-RSA-AES128-GCM-SHA256",
        "ECDHE-RSA-AES256-GCM-SHA384",
        "ECDHE-RSA-CHACHA20-POLY1305",
      ];

      const invalidCiphers = [
        "COMPLEMENTOFALL",
        "eNULL",
        "NULL",
        "aNULL",
        "kDHr",
        "kDHd",
        "kDH",
        "kDHE",
        "kEDH",
        "DH",
        "DHE",
        "EDH",
        "ADH",
        "AECDH",
        "aDSS",
        "DSS",
        "aDH",
        "aECDSA",
        "ECDSA",
        "AESCCM",
        "AESCCM8",
        "ARIA128",
        "ARIA256",
        "ARIA",
        "CAMELLIA128",
        "CAMELLIA256",
        "CAMELLIA",
        "DES",
        "RC4",
        "RC2",
        "IDEA",
        "SEED",
        "MD5",
        "aGOST",
        "aGOST01",
        "kGOST",
        "GOST94",
        "GOST89MAC",
        "PSK",
        "kPSK",
        "kECDHEPSK",
        "kDHEPSK",
        "kRSAPSK",
        "aPSK",
        "SUITEB128",
        "SUITEB128ONLY",
        "SUITEB192",
        "CBC",

        "NULL-MD5",
        "NULL-SHA",
        "RC4-MD5",
        "RC4-SHA",
        "IDEA-CBC-SHA",
        "DH-DSS-DES-CBC3-SHA",
        "DH-RSA-DES-CBC3-SHA",
        "DHE-DSS-DES-CBC3-SHA",
        "DHE-RSA-DES-CBC3-SHA",
        "ADH-RC4-MD5",
        "ADH-DES-CBC3-SHA",
        "NULL-MD5",
        "NULL-SHA",
        "RC4-MD5",
        "RC4-SHA",
        "IDEA-CBC-SHA",
        "DHE-DSS-DES-CBC3-SHA",
        "DHE-RSA-DES-CBC3-SHA",
        "ADH-RC4-MD5",
        "ADH-DES-CBC3-SHA",

        "DH-DSS-AES128-SHA",
        "DH-DSS-AES256-SHA",
        "DH-RSA-AES128-SHA",
        "DH-RSA-AES256-SHA",
        "DHE-DSS-AES128-SHA",
        "DHE-DSS-AES256-SHA",
        "DHE-RSA-AES128-SHA",
        "DHE-RSA-AES256-SHA",
        "ADH-AES128-SHA",
        "ADH-AES256-SHA",
        "CAMELLIA128-SHA",
        "CAMELLIA256-SHA",
        "DH-DSS-CAMELLIA128-SHA",
        "DH-DSS-CAMELLIA256-SHA",
        "DH-RSA-CAMELLIA128-SHA",
        "DH-RSA-CAMELLIA256-SHA",
        "DHE-DSS-CAMELLIA128-SHA",
        "DHE-DSS-CAMELLIA256-SHA",
        "DHE-RSA-CAMELLIA128-SHA",
        "DHE-RSA-CAMELLIA256-SHA",
        "ADH-CAMELLIA128-SHA",
        "ADH-CAMELLIA256-SHA",
        "SEED-SHA",
        "DH-DSS-SEED-SHA",
        "DH-RSA-SEED-SHA",
        "DHE-DSS-SEED-SHA",
        "DHE-RSA-SEED-SHA",
        "ADH-SEED-SHA",
        "GOST94-GOST89-GOST89",
        "GOST2001-GOST89-GOST89",
        "GOST94-NULL-GOST94",
        "GOST2001-NULL-GOST94",
        "GOST2012-GOST8912-GOST8912",
        "GOST2012-NULL-GOST12",
        "DHE-DSS-RC4-SHA",
        "ECDHE-RSA-NULL-SHA",
        "ECDHE-RSA-RC4-SHA",
        "ECDHE-RSA-DES-CBC3-SHA",
        "ECDHE-ECDSA-NULL-SHA",
        "ECDHE-ECDSA-RC4-SHA",
        "ECDHE-ECDSA-DES-CBC3-SHA",
        "ECDHE-ECDSA-AES128-SHA",
        "ECDHE-ECDSA-AES256-SHA",
        "AECDH-NULL-SHA",
        "AECDH-RC4-SHA",
        "AECDH-DES-CBC3-SHA",
        "AECDH-AES128-SHA",
        "AECDH-AES256-SHA",
        "NULL-SHA256",
        "AES128-SHA256",
        "AES256-SHA256",
        "AES256-GCM-SHA384",
        "DH-RSA-AES128-SHA256",
        "DH-RSA-AES256-SHA256",
        "DH-RSA-AES128-GCM-SHA256",
        "DH-RSA-AES256-GCM-SHA384",
        "DH-DSS-AES128-SHA256",
        "DH-DSS-AES256-SHA256",
        "DH-DSS-AES128-GCM-SHA256",
        "DH-DSS-AES256-GCM-SHA384",
        "DHE-RSA-AES128-SHA256",
        "DHE-RSA-AES256-SHA256",
        "DHE-RSA-AES128-GCM-SHA256",
        "DHE-RSA-AES256-GCM-SHA384",
        "DHE-DSS-AES128-SHA256",
        "DHE-DSS-AES256-SHA256",
        "DHE-DSS-AES128-GCM-SHA256",
        "DHE-DSS-AES256-GCM-SHA384",
        "ECDHE-ECDSA-AES128-SHA256",
        "ECDHE-ECDSA-AES256-SHA384",
        "ECDHE-ECDSA-AES128-GCM-SHA256",
        "ECDHE-ECDSA-AES256-GCM-SHA384",
        "ADH-AES128-SHA256",
        "ADH-AES256-SHA256",
        "ADH-AES128-GCM-SHA256",
        "ADH-AES256-GCM-SHA384",
        "AES128-CCM",
        "AES256-CCM",
        "DHE-RSA-AES128-CCM",
        "DHE-RSA-AES256-CCM",
        "AES128-CCM8",
        "AES256-CCM8",
        "DHE-RSA-AES128-CCM8",
        "DHE-RSA-AES256-CCM8",
        "ECDHE-ECDSA-AES128-CCM",
        "ECDHE-ECDSA-AES256-CCM",
        "ECDHE-ECDSA-AES128-CCM8",
        "ECDHE-ECDSA-AES256-CCM8",
        "ARIA128-GCM-SHA256",
        "ARIA256-GCM-SHA384",
        "DHE-RSA-ARIA128-GCM-SHA256",
        "DHE-RSA-ARIA256-GCM-SHA384",
        "DHE-DSS-ARIA128-GCM-SHA256",
        "DHE-DSS-ARIA256-GCM-SHA384",
        "ECDHE-ECDSA-ARIA128-GCM-SHA256",
        "ECDHE-ECDSA-ARIA256-GCM-SHA384",
        "ECDHE-ARIA128-GCM-SHA256",
        "ECDHE-ARIA256-GCM-SHA384",
        "PSK-ARIA128-GCM-SHA256",
        "PSK-ARIA256-GCM-SHA384",
        "DHE-PSK-ARIA128-GCM-SHA256",
        "DHE-PSK-ARIA256-GCM-SHA384",
        "RSA-PSK-ARIA128-GCM-SHA256",
        "RSA-PSK-ARIA256-GCM-SHA384",
        "ECDHE-ECDSA-CAMELLIA128-SHA256",
        "ECDHE-ECDSA-CAMELLIA256-SHA384",
        "ECDHE-RSA-CAMELLIA128-SHA256",
        "ECDHE-RSA-CAMELLIA256-SHA384",
        "PSK-NULL-SHA",
        "DHE-PSK-NULL-SHA",
        "RSA-PSK-NULL-SHA",
        "PSK-RC4-SHA",
        "PSK-3DES-EDE-CBC-SHA",
        "PSK-AES128-CBC-SHA",
        "PSK-AES256-CBC-SHA",
        "DHE-PSK-RC4-SHA",
        "DHE-PSK-3DES-EDE-CBC-SHA",
        "DHE-PSK-AES128-CBC-SHA",
        "DHE-PSK-AES256-CBC-SHA",
        "RSA-PSK-RC4-SHA",
        "RSA-PSK-3DES-EDE-CBC-SHA",
        "RSA-PSK-AES128-CBC-SHA",
        "RSA-PSK-AES256-CBC-SHA",
        "PSK-AES128-GCM-SHA256",
        "PSK-AES256-GCM-SHA384",
        "DHE-PSK-AES128-GCM-SHA256",
        "DHE-PSK-AES256-GCM-SHA384",
        "RSA-PSK-AES128-GCM-SHA256",
        "RSA-PSK-AES256-GCM-SHA384",
        "PSK-AES128-CBC-SHA256",
        "PSK-AES256-CBC-SHA384",
        "PSK-NULL-SHA256",
        "PSK-NULL-SHA384",
        "DHE-PSK-AES128-CBC-SHA256",
        "DHE-PSK-AES256-CBC-SHA384",
        "DHE-PSK-NULL-SHA256",
        "DHE-PSK-NULL-SHA384",
        "RSA-PSK-AES128-CBC-SHA256",
        "RSA-PSK-AES256-CBC-SHA384",
        "RSA-PSK-NULL-SHA256",
        "RSA-PSK-NULL-SHA384",
        "PSK-AES128-GCM-SHA256",
        "PSK-AES256-GCM-SHA384",
        "ECDHE-PSK-RC4-SHA",
        "ECDHE-PSK-3DES-EDE-CBC-SHA",
        "ECDHE-PSK-AES128-CBC-SHA",
        "ECDHE-PSK-AES256-CBC-SHA",
        "ECDHE-PSK-AES128-CBC-SHA256",
        "ECDHE-PSK-AES256-CBC-SHA384",
        "ECDHE-PSK-NULL-SHA",
        "ECDHE-PSK-NULL-SHA256",
        "ECDHE-PSK-NULL-SHA384",
        "PSK-CAMELLIA128-SHA256",
        "PSK-CAMELLIA256-SHA384",
        "DHE-PSK-CAMELLIA128-SHA256",
        "DHE-PSK-CAMELLIA256-SHA384",
        "RSA-PSK-CAMELLIA128-SHA256",
        "RSA-PSK-CAMELLIA256-SHA384",
        "ECDHE-PSK-CAMELLIA128-SHA256",
        "ECDHE-PSK-CAMELLIA256-SHA384",
        "PSK-AES128-CCM",
        "PSK-AES256-CCM",
        "DHE-PSK-AES128-CCM",
        "DHE-PSK-AES256-CCM",
        "PSK-AES128-CCM8",
        "PSK-AES256-CCM8",
        "DHE-PSK-AES128-CCM8",
        "DHE-PSK-AES256-CCM8",
        "ECDHE-ECDSA-CHACHA20-POLY1305",
        "DHE-RSA-CHACHA20-POLY1305",
        "PSK-CHACHA20-POLY1305",
        "ECDHE-PSK-CHACHA20-POLY1305",
        "DHE-PSK-CHACHA20-POLY1305",
        "RSA-PSK-CHACHA20-POLY1305",
      ];
      routes.set("/backend/constructor/parameter-ciphers-property-valid-cipherlist-strings-supported-by-fastly", async () => {
        const ciphers = [...validCiphers];
        for (const cipher of ciphers) {
          let error = assertDoesNotThrow(() => {
            new Backend({ name: cipher, target: 'a', ciphers: cipher })
          })
          if (error) { return error }
        }

        let error = assertDoesNotThrow(() => {
          new Backend({ name: 'all-valid-ciphers', target: 'a', ciphers: validCiphers.join(':') })
          new Backend({ name: 'all-ciphers-invalid-marked-as-exclude', target: 'a', ciphers: validCiphers.concat(invalidCiphers.map(c => '!' + c)).join(':') })
        })
        if (error) { return error }
        return pass()
      });
      routes.set("/backend/constructor/parameter-ciphers-property-valid-cipherlist-strings-but-not-supported-by-fastly", async () => {
        const ciphers = [...invalidCiphers];
        for (const cipher of ciphers) {
          let error = assertThrows(() => {
            new Backend({ name: cipher, target: 'a', ciphers: cipher })
          }, TypeError, `Backend constructor: none of the provided ciphers are supported by Fastly. The list of supported ciphers is available on https://developer.fastly.com/learning/concepts/routing-traffic-to-fastly/#use-a-tls-configuration`)
          if (error) { return error }
        }
        let error = assertThrows(() => {
          new Backend({ name: 'all-invalid-ciphers', target: 'a', ciphers: invalidCiphers.join(':') })
        }, TypeError, `Backend constructor: none of the provided ciphers are supported by Fastly. The list of supported ciphers is available on https://developer.fastly.com/learning/concepts/routing-traffic-to-fastly/#use-a-tls-configuration`)
        if (error) { return error }
        error = assertThrows(() => {
          new Backend({ name: 'all-ciphers-valid-marked-as-exclude', target: 'a', ciphers: invalidCiphers.concat(validCiphers.map(c => '!' + c)).join(':') })
        }, TypeError, `Backend constructor: none of the provided ciphers are supported by Fastly. The list of supported ciphers is available on https://developer.fastly.com/learning/concepts/routing-traffic-to-fastly/#use-a-tls-configuration`)
        if (error) { return error }
        return pass()
      });
      // https://tc39.es/ecma262/#sec-tostring
      routes.set("/backend/constructor/parameter-ciphers-property-calls-7.1.17-ToString", async () => {
        let sentinel;
        const test = () => {
          sentinel = Symbol();
          const ciphers = {
            toString() {
              throw sentinel;
            }
          }
          new Backend({ name: 'a', target: 'a', ciphers })
        }
        let error = assertThrows(test)
        if (error) { return error }
        try {
          test()
        } catch (thrownError) {
          let error = assert(thrownError, sentinel, 'thrownError === sentinel')
          if (error) { return error }
        }
        error = assertThrows(() => new Backend({ name: 'a', target: 'a', ciphers: Symbol() }), TypeError, `can't convert symbol to string`)
        if (error) { return error }
        return pass()
      });
    }

    // hostOverride property
    {
      routes.set("/backend/constructor/parameter-hostOverride-property-empty-string", async () => {
        let error = assertThrows(() => {
          new Backend({ name: 'hostOverride-property-empty-string', target: 'a', hostOverride: "" })
        }, TypeError, `Backend constructor: hostOverride can not be an empty string`)
        if (error) { return error }
        return pass()
      });
      // https://tc39.es/ecma262/#sec-tostring
      routes.set("/backend/constructor/parameter-hostOverride-property-calls-7.1.17-ToString", async () => {
        let sentinel;
        const test = () => {
          sentinel = Symbol();
          const hostOverride = {
            toString() {
              throw sentinel;
            }
          }
          new Backend({ name: 'hostOverride-property-calls-ToString', target: 'a', hostOverride })
        }
        let error = assertThrows(test)
        if (error) { return error }
        try {
          test()
        } catch (thrownError) {
          let error = assert(thrownError, sentinel, 'thrownError === sentinel')
          if (error) { return error }
        }
        error = assertThrows(() => new Backend({ name: 'hostOverride-property-calls-ToString', target: 'a', hostOverride: Symbol() }), TypeError, `can't convert symbol to string`)
        if (error) { return error }
        return pass()
      });

      routes.set("/backend/constructor/parameter-hostOverride-property-valid-string", async () => {
        let error = assertDoesNotThrow(() => {
          new Backend({ name: 'hostOverride-property-valid-string', target: 'a', hostOverride: "www.fastly.com" })
        })
        if (error) { return error }
        return pass()
      });
    }

    // connectTimeout property
    {
      routes.set("/backend/constructor/parameter-connectTimeout-property-negative-number", async () => {
        let error = assertThrows(() => {
          new Backend({ name: 'connectTimeout-property-negative-number', target: 'a', connectTimeout: -1 })
        }, RangeError, `Backend constructor: connectTimeout can not be a negative number`)
        if (error) { return error }
        return pass()
      });
      routes.set("/backend/constructor/parameter-connectTimeout-property-too-big", async () => {
        let error = assertThrows(() => {
          new Backend({ name: 'connectTimeout-property-too-big', target: 'a', connectTimeout: Math.pow(2, 32) })
        }, RangeError, `Backend constructor: connectTimeout must be less than 2^32`)
        if (error) { return error }
        return pass()
      });
      // https://tc39.es/ecma262/#sec-tonumber
      routes.set("/backend/constructor/parameter-connectTimeout-property-calls-7.1.4-ToNumber", async () => {
        let sentinel;
        let requestedType;
        const test = () => {
          sentinel = Symbol();
          const connectTimeout = {
            [Symbol.toPrimitive](type) {
              requestedType = type;
              throw sentinel;
            }
          }
          new Backend({ name: 'connectTimeout-property-calls-ToNumber', target: 'a', connectTimeout })
        }
        let error = assertThrows(test)
        if (error) { return error }
        try {
          test()
        } catch (thrownError) {
          let error = assert(thrownError, sentinel, 'thrownError === sentinel')
          if (error) { return error }
          error = assert(requestedType, 'number', 'requestedType === "number"')
          if (error) { return error }
        }
        error = assertThrows(() => new Backend({ name: 'connectTimeout-property-calls-ToNumber', target: 'a', connectTimeout: Symbol() }), TypeError, `can't convert symbol to number`)
        if (error) { return error }
        return pass()
      });

      routes.set("/backend/constructor/parameter-connectTimeout-property-valid-number", async () => {
        let error = assertDoesNotThrow(() => {
          new Backend({ name: 'connectTimeout-property-valid-number', target: 'a', connectTimeout: 1 })
        })
        if (error) { return error }
        return pass()
      });
    }

    // firstByteTimeout property
    {
      routes.set("/backend/constructor/parameter-firstByteTimeout-property-negative-number", async () => {
        let error = assertThrows(() => {
          new Backend({ name: 'firstByteTimeout-property-negative-number', target: 'a', firstByteTimeout: -1 })
        }, RangeError, `Backend constructor: firstByteTimeout can not be a negative number`)
        if (error) { return error }
        return pass()
      });
      routes.set("/backend/constructor/parameter-firstByteTimeout-property-too-big", async () => {
        let error = assertThrows(() => {
          new Backend({ name: 'firstByteTimeout-property-too-big', target: 'a', firstByteTimeout: Math.pow(2, 32) })
        }, RangeError, `Backend constructor: firstByteTimeout must be less than 2^32`)
        if (error) { return error }
        return pass()
      });
      // https://tc39.es/ecma262/#sec-tonumber
      routes.set("/backend/constructor/parameter-firstByteTimeout-property-calls-7.1.4-ToNumber", async () => {
        let sentinel;
        let requestedType;
        const test = () => {
          sentinel = Symbol();
          const firstByteTimeout = {
            [Symbol.toPrimitive](type) {
              requestedType = type;
              throw sentinel;
            }
          }
          new Backend({ name: 'firstByteTimeout-property-calls-ToNumber', target: 'a', firstByteTimeout })
        }
        let error = assertThrows(test)
        if (error) { return error }
        try {
          test()
        } catch (thrownError) {
          let error = assert(thrownError, sentinel, 'thrownError === sentinel')
          if (error) { return error }
          error = assert(requestedType, 'number', 'requestedType === "number"')
          if (error) { return error }
        }
        error = assertThrows(() => new Backend({ name: 'firstByteTimeout-property-calls-ToNumber', target: 'a', firstByteTimeout: Symbol() }), TypeError, `can't convert symbol to number`)
        if (error) { return error }
        return pass()
      });

      routes.set("/backend/constructor/parameter-firstByteTimeout-property-valid-number", async () => {
        let error = assertDoesNotThrow(() => {
          new Backend({ name: 'firstByteTimeout-property-valid-number', target: 'a', firstByteTimeout: 1 })
        })
        if (error) { return error }
        return pass()
      });
    }

    // betweenBytesTimeout property
    {
      routes.set("/backend/constructor/parameter-betweenBytesTimeout-property-negative-number", async () => {
        let error = assertThrows(() => {
          new Backend({ name: 'betweenBytesTimeout-property-negative-number', target: 'a', betweenBytesTimeout: -1 })
        }, RangeError, `Backend constructor: betweenBytesTimeout can not be a negative number`)
        if (error) { return error }
        return pass()
      });
      routes.set("/backend/constructor/parameter-betweenBytesTimeout-property-too-big", async () => {
        let error = assertThrows(() => {
          new Backend({ name: 'betweenBytesTimeout-property-too-big', target: 'a', betweenBytesTimeout: Math.pow(2, 32) })
        }, RangeError, `Backend constructor: betweenBytesTimeout must be less than 2^32`)
        if (error) { return error }
        return pass()
      });
      // https://tc39.es/ecma262/#sec-tonumber
      routes.set("/backend/constructor/parameter-betweenBytesTimeout-property-calls-7.1.4-ToNumber", async () => {
        let sentinel;
        let requestedType;
        const test = () => {
          sentinel = Symbol();
          const betweenBytesTimeout = {
            [Symbol.toPrimitive](type) {
              requestedType = type;
              throw sentinel;
            }
          }
          new Backend({ name: 'betweenBytesTimeout-property-calls-ToNumber', target: 'a', betweenBytesTimeout })
        }
        let error = assertThrows(test)
        if (error) { return error }
        try {
          test()
        } catch (thrownError) {
          let error = assert(thrownError, sentinel, 'thrownError === sentinel')
          if (error) { return error }
          error = assert(requestedType, 'number', 'requestedType === "number"')
          if (error) { return error }
        }
        error = assertThrows(() => new Backend({ name: 'betweenBytesTimeout-property-calls-ToNumber', target: 'a', betweenBytesTimeout: Symbol() }), TypeError, `can't convert symbol to number`)
        if (error) { return error }
        return pass()
      });

      routes.set("/backend/constructor/parameter-betweenBytesTimeout-property-valid-number", async () => {
        let error = assertDoesNotThrow(() => {
          new Backend({ name: 'betweenBytesTimeout-property-valid-number', target: 'a', betweenBytesTimeout: 1 })
        })
        if (error) { return error }
        return pass()
      });
    }

    // useSSL property
    {
      routes.set("/backend/constructor/parameter-useSSL-property-valid-boolean", async () => {
        const types = [{}, [], Symbol(), 1, "2"];
        for (const type of types) {
          let error = assertDoesNotThrow(() => {
            new Backend({ name: 'useSSL-property-valid-boolean' + String(type), target: 'a', useSSL: type })
          })
          if (error) { return error }
        }
        let error = assertDoesNotThrow(() => {
          new Backend({ name: 'useSSL-property-valid-boolean-true', target: 'a', useSSL: true })
        })
        if (error) { return error }
        error = assertDoesNotThrow(() => {
          new Backend({ name: 'useSSL-property-valid-boolean-false', target: 'a', useSSL: false })
        })
        if (error) { return error }
        return pass()
      });
    }

    // tlsMinVersion property
    {
      routes.set("/backend/constructor/parameter-tlsMinVersion-property-nan", async () => {
        let error = assertThrows(() => {
          new Backend({ name: 'tlsMinVersion-property-nan', target: 'a', tlsMinVersion: NaN })
        }, RangeError, `Backend constructor: tlsMinVersion must be either 1, 1.1, 1.2, or 1.3`)
        if (error) { return error }
        return pass()
      });
      routes.set("/backend/constructor/parameter-tlsMinVersion-property-invalid-number", async () => {
        let error = assertThrows(() => {
          new Backend({ name: 'tlsMinVersion-property-invalid-number', target: 'a', tlsMinVersion: 1.4 })
        }, RangeError, `Backend constructor: tlsMinVersion must be either 1, 1.1, 1.2, or 1.3`)
        if (error) { return error }
        return pass()
      });
      // https://tc39.es/ecma262/#sec-tonumber
      routes.set("/backend/constructor/parameter-tlsMinVersion-property-calls-7.1.4-ToNumber", async () => {
        let sentinel;
        let requestedType;
        const test = () => {
          sentinel = Symbol();
          const tlsMinVersion = {
            [Symbol.toPrimitive](type) {
              requestedType = type;
              throw sentinel;
            }
          }
          new Backend({ name: 'tlsMinVersion-property-calls-ToNumber', target: 'a', tlsMinVersion })
        }
        let error = assertThrows(test)
        if (error) { return error }
        try {
          test()
        } catch (thrownError) {
          let error = assert(thrownError, sentinel, 'thrownError === sentinel')
          if (error) { return error }
          error = assert(requestedType, 'number', 'requestedType === "number"')
          if (error) { return error }
        }
        error = assertThrows(() => new Backend({ name: 'tlsMinVersion-property-calls-ToNumber', target: 'a', tlsMinVersion: Symbol() }), TypeError, `can't convert symbol to number`)
        if (error) { return error }
        return pass()
      });

      routes.set("/backend/constructor/parameter-tlsMinVersion-property-valid-number", async () => {
        let error = assertDoesNotThrow(() => {
          new Backend({ name: 'tlsMinVersion-property-valid-number-1', target: 'a', tlsMinVersion: 1 })
        })
        if (error) { return error }
        error = assertDoesNotThrow(() => {
          new Backend({ name: 'tlsMinVersion-property-valid-number-1.0', target: 'a', tlsMinVersion: 1.0 })
        })
        if (error) { return error }
        error = assertDoesNotThrow(() => {
          new Backend({ name: 'tlsMinVersion-property-valid-number-1.1', target: 'a', tlsMinVersion: 1.1 })
        })
        if (error) { return error }
        error = assertDoesNotThrow(() => {
          new Backend({ name: 'tlsMinVersion-property-valid-number-1.2', target: 'a', tlsMinVersion: 1.2 })
        })
        if (error) { return error }
        error = assertDoesNotThrow(() => {

          new Backend({ name: 'tlsMinVersion-property-valid-number-1.3', target: 'a', tlsMinVersion: 1.3 })
        })
        if (error) { return error }
        return pass()
      });

      routes.set("/backend/constructor/parameter-tlsMinVersion-greater-than-tlsMaxVersion", async () => {
        let error = assertThrows(() => {
          new Backend({ name: 'tlsMinVersion-property-valid-number', target: 'a', tlsMinVersion: 1.3, tlsMaxVersion: 1.2 })
        }, RangeError, `Backend constructor: tlsMinVersion must be less than or equal to tlsMaxVersion`)
        if (error) { return error }
        return pass()
      });
    }

    // tlsMaxVersion property
    {
      routes.set("/backend/constructor/parameter-tlsMaxVersion-property-nan", async () => {
        let error = assertThrows(() => {
          new Backend({ name: 'tlsMaxVersion-property-nan', target: 'a', tlsMaxVersion: NaN })
        }, RangeError, `Backend constructor: tlsMaxVersion must be either 1, 1.1, 1.2, or 1.3`)
        if (error) { return error }
        return pass()
      });
      routes.set("/backend/constructor/parameter-tlsMaxVersion-property-invalid-number", async () => {
        let error = assertThrows(() => {
          new Backend({ name: 'tlsMaxVersion-property-invalid-number', target: 'a', tlsMaxVersion: 1.4 })
        }, RangeError, `Backend constructor: tlsMaxVersion must be either 1, 1.1, 1.2, or 1.3`)
        if (error) { return error }
        return pass()
      });
      // https://tc39.es/ecma262/#sec-tonumber
      routes.set("/backend/constructor/parameter-tlsMaxVersion-property-calls-7.1.4-ToNumber", async () => {
        let sentinel;
        let requestedType;
        const test = () => {
          sentinel = Symbol();
          const tlsMaxVersion = {
            [Symbol.toPrimitive](type) {
              requestedType = type;
              throw sentinel;
            }
          }
          new Backend({ name: 'tlsMaxVersion-property-calls-ToNumber', target: 'a', tlsMaxVersion })
        }
        let error = assertThrows(test)
        if (error) { return error }
        try {
          test()
        } catch (thrownError) {
          let error = assert(thrownError, sentinel, 'thrownError === sentinel')
          if (error) { return error }
          error = assert(requestedType, 'number', 'requestedType === "number"')
          if (error) { return error }
        }
        error = assertThrows(() => new Backend({ name: 'tlsMaxVersion-property-calls-ToNumber', target: 'a', tlsMaxVersion: Symbol() }), TypeError, `can't convert symbol to number`)
        if (error) { return error }
        return pass()
      });

      routes.set("/backend/constructor/parameter-tlsMaxVersion-property-valid-number", async () => {
        let error = assertDoesNotThrow(() => {
          new Backend({ name: 'tlsMaxVersion-property-valid-number-1', target: 'a', tlsMaxVersion: 1 })
          new Backend({ name: 'tlsMaxVersion-property-valid-number-1.0', target: 'a', tlsMaxVersion: 1.0 })
          new Backend({ name: 'tlsMaxVersion-property-valid-number-1.1', target: 'a', tlsMaxVersion: 1.1 })
          new Backend({ name: 'tlsMaxVersion-property-valid-number-1.2', target: 'a', tlsMaxVersion: 1.2 })
          new Backend({ name: 'tlsMaxVersion-property-valid-number-1.3', target: 'a', tlsMaxVersion: 1.3 })
        })
        if (error) { return error }
        return pass()
      });
    }

    // certificateHostname property
    {
      routes.set("/backend/constructor/parameter-certificateHostname-property-empty-string", async () => {
        let error = assertThrows(() => {
          new Backend({ name: 'certificateHostname-property-empty-string', target: 'a', certificateHostname: "" })
        }, TypeError, `Backend constructor: certificateHostname can not be an empty string`)
        if (error) { return error }
        return pass()
      });
      // https://tc39.es/ecma262/#sec-tostring
      routes.set("/backend/constructor/parameter-certificateHostname-property-calls-7.1.17-ToString", async () => {
        let sentinel;
        const test = () => {
          sentinel = Symbol();
          const certificateHostname = {
            toString() {
              throw sentinel;
            }
          }
          new Backend({ name: 'certificateHostname-property-calls-ToString', target: 'a', certificateHostname })
        }
        let error = assertThrows(test)
        if (error) { return error }
        try {
          test()
        } catch (thrownError) {
          let error = assert(thrownError, sentinel, 'thrownError === sentinel')
          if (error) { return error }
        }
        error = assertThrows(() => new Backend({ name: 'certificateHostname-property-calls-ToString', target: 'a', certificateHostname: Symbol() }), TypeError, `can't convert symbol to string`)
        if (error) { return error }
        return pass()
      });

      routes.set("/backend/constructor/parameter-certificateHostname-property-valid-string", async () => {
        let error = assertDoesNotThrow(() => {
          new Backend({ name: 'certificateHostname-property-valid-string', target: 'a', certificateHostname: "www.fastly.com" })
        })
        if (error) { return error }
        return pass()
      });
    }

    // checkCertificate property
    {
      routes.set("/backend/constructor/parameter-checkCertificate-property-valid-boolean", async () => {
        const types = [{}, [], Symbol(), 1, "2"];
        for (const type of types) {
          let error = assertDoesNotThrow(() => {
            new Backend({ name: 'checkCertificate-property-valid-boolean' + String(type), target: 'a', checkCertificate: type })
          })
          if (error) { return error }
        }
        let error = assertDoesNotThrow(() => {
          new Backend({ name: 'checkCertificate-property-valid-boolean-true', target: 'a', checkCertificate: true })
        })
        if (error) { return error }
        error = assertDoesNotThrow(() => {
          new Backend({ name: 'checkCertificate-property-valid-boolean-false', target: 'a', checkCertificate: false })
        })
        if (error) { return error }
        return pass()
      });
    }

    // caCertificate property
    {
      routes.set("/backend/constructor/parameter-caCertificate-property-empty-string", async () => {
        let error = assertThrows(() => {
          new Backend({ name: 'caCertificate-property-empty-string', target: 'a', caCertificate: "" })
        }, TypeError, `Backend constructor: caCertificate can not be an empty string`)
        if (error) { return error }
        return pass()
      });
      // https://tc39.es/ecma262/#sec-tostring
      routes.set("/backend/constructor/parameter-caCertificate-property-calls-7.1.17-ToString", async () => {
        let sentinel;
        const test = () => {
          sentinel = Symbol();
          const caCertificate = {
            toString() {
              throw sentinel;
            }
          }
          new Backend({ name: 'caCertificate-property-calls-ToString', target: 'a', caCertificate })
        }
        let error = assertThrows(test)
        if (error) { return error }
        try {
          test()
        } catch (thrownError) {
          let error = assert(thrownError, sentinel, 'thrownError === sentinel')
          if (error) { return error }
        }
        error = assertThrows(() => new Backend({ name: 'caCertificate-property-calls-ToString', target: 'a', caCertificate: Symbol() }), TypeError, `can't convert symbol to string`)
        if (error) { return error }
        return pass()
      });

      routes.set("/backend/constructor/parameter-caCertificate-property-valid-string", async () => {
        let error = assertDoesNotThrow(() => {
          new Backend({ name: 'caCertificate-property-valid-string', target: 'a', caCertificate: "www.fastly.com" })
        })
        if (error) { return error }
        return pass()
      });
    }

    // sniHostname property
    {
      routes.set("/backend/constructor/parameter-sniHostname-property-empty-string", async () => {
        let error = assertThrows(() => {
          new Backend({ name: 'sniHostname-property-empty-string', target: 'a', sniHostname: "" })
        }, TypeError, `Backend constructor: sniHostname can not be an empty string`)
        if (error) { return error }
        return pass()
      });
      // https://tc39.es/ecma262/#sec-tostring
      routes.set("/backend/constructor/parameter-sniHostname-property-calls-7.1.17-ToString", async () => {
        let sentinel;
        const test = () => {
          sentinel = Symbol();
          const sniHostname = {
            toString() {
              throw sentinel;
            }
          }
          new Backend({ name: 'sniHostname-property-calls-ToString', target: 'a', sniHostname })
        }
        let error = assertThrows(test)
        if (error) { return error }
        try {
          test()
        } catch (thrownError) {
          let error = assert(thrownError, sentinel, 'thrownError === sentinel')
          if (error) { return error }
        }
        error = assertThrows(() => new Backend({ name: 'sniHostname-property-calls-ToString', target: 'a', sniHostname: Symbol() }), TypeError, `can't convert symbol to string`)
        if (error) { return error }
        return pass()
      });

      routes.set("/backend/constructor/parameter-sniHostname-property-valid-string", async () => {
        let error = assertDoesNotThrow(() => {
          new Backend({ name: 'sniHostname-property-valid-string', target: 'a', sniHostname: "www.fastly.com" })
        })
        if (error) { return error }
        return pass()
      });
    }
  }
}

function createValidHttpBinBackend() {
  // We are defining all the possible fields here but any number of fields can be defined - the ones which are not defined will use their default value instead.
  return new Backend(
    {
      name: 'httpbin',
      target: 'httpbin.org',
      hostOverride: "httpbin.org",
      connectTimeout: 1000,
      firstByteTimeout: 15000,
      betweenBytesTimeout: 10000,
      useSSL: true,
      tlsMinVersion: 1.2,
      tlsMaxVersion: 1.2,
      certificateHostname: "httpbin.org",
      checkCertificate: true,
      caCertificate: `-----BEGIN CERTIFICATE-----
MIIGgjCCBWqgAwIBAgIQASW9vtgUNIzM07bVg9HdEzANBgkqhkiG9w0BAQsFADBY
MQswCQYDVQQGEwJCRTEZMBcGA1UEChMQR2xvYmFsU2lnbiBudi1zYTEuMCwGA1UE
AxMlR2xvYmFsU2lnbiBBdGxhcyBSMyBEViBUTFMgQ0EgMjAyMiBRMTAeFw0yMjAz
MzExNDI4NTBaFw0yMzA1MDIxNDI4NDlaMBkxFzAVBgNVBAMMDnd3dy5mYXN0bHku
Y29tMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAmtLSpTiRzJaFiAEi
Eua36KwkZBm647y7cEkYEu6GgStXAWtzDF4ttzI8q9iRpqlljFDjQF2EeWLXN2X2
MEQPxKOOkM+OuJny0pXDCkKiSGS6MwVzNqCNW5tnxkJMMUAq5ciyQFwF72Z+ymlo
NGCl3CIDAJbHcudm4CiVR5ip7KYFH7uaBRLPXtMLX2lmQ1q2TbU/GXrbDhz5OXCD
H9SKAUSHLqwLplj8mojxWayda9zy3bqbTVXsqRVW0Xar62T33Uis0fm+xW4hJbxf
bya5I0aQbAjtxvEjfUa1kSalUqPwfHOXMvvXwoqUIKk1ndZScWl9TGLH84izTCPw
bOVsTQIDAQABo4IDhTCCA4EwOwYDVR0RBDQwMoIOd3d3LmZhc3RseS5jb22CFGRl
dmVsb3Blci5mYXN0bHkuY29tggpmYXN0bHkuY29tMA4GA1UdDwEB/wQEAwIFoDAd
BgNVHSUEFjAUBggrBgEFBQcDAQYIKwYBBQUHAwIwHQYDVR0OBBYEFCYC4rPzDtR8
EI3XULw/wr8lfnv2MFcGA1UdIARQME4wCAYGZ4EMAQIBMEIGCisGAQQBoDIKAQMw
NDAyBggrBgEFBQcCARYmaHR0cHM6Ly93d3cuZ2xvYmFsc2lnbi5jb20vcmVwb3Np
dG9yeS8wDAYDVR0TAQH/BAIwADCBngYIKwYBBQUHAQEEgZEwgY4wQAYIKwYBBQUH
MAGGNGh0dHA6Ly9vY3NwLmdsb2JhbHNpZ24uY29tL2NhL2dzYXRsYXNyM2R2dGxz
Y2EyMDIycTEwSgYIKwYBBQUHMAKGPmh0dHA6Ly9zZWN1cmUuZ2xvYmFsc2lnbi5j
b20vY2FjZXJ0L2dzYXRsYXNyM2R2dGxzY2EyMDIycTEuY3J0MB8GA1UdIwQYMBaA
FJwqZ9C0gzFbgMspllV1CTc4uDylMEgGA1UdHwRBMD8wPaA7oDmGN2h0dHA6Ly9j
cmwuZ2xvYmFsc2lnbi5jb20vY2EvZ3NhdGxhc3IzZHZ0bHNjYTIwMjJxMS5jcmww
ggF/BgorBgEEAdZ5AgQCBIIBbwSCAWsBaQB3AK33vvp8/xDIi509nB4+GGq0Zyld
z7EMJMqFhjTr3IKKAAABf+BhC9UAAAQDAEgwRgIhAI/TiJpulEywYHMuudQ6fYcK
dl18wDnJtAD+3JqFkrXuAiEA+i6ryPZTeENZJ6wEgRnndsggk8blsfch13e4s76u
WngAdQB6MoxU2LcttiDqOOBSHumEFnAyE4VNO9IrwTpXo1LrUgAAAX/gYQwqAAAE
AwBGMEQCIEldTkse3joAWr1llSoi9EOle0K0hz086GrCjL5YDhXkAiARW5dUit2q
Tq/mD+aN+XHfupYiYC7htPvHMWM6iUHPFgB3ALNzdwfhhFD4Y4bWBancEQlKeS2x
ZwwLh9zwAw55NqWaAAABf+BhDFQAAAQDAEgwRgIhAIQ1nuPDm08OuXDmLBkreA7L
LdGLmdhnJOdzOW0n7PDGAiEA4cEZYQ+uFrAoIHezQXBe05/ovXLacu5SVoInRJIK
gBYwDQYJKoZIhvcNAQELBQADggEBAPH+sQGtGBE6BlC0Zp02rJo/1QCPC+/L1T1+
uErHb0175NKnxXmT0HYoXr3gduMoMFXQr0VkhtQzCq7OaAX89+UdQ/OkfdU6CK0z
H4318J11ZShIHqGUCL+w27JsAgzJw/9UQBfT3FLCto735lwfo/l6HyD7qNqNYwhK
UlkETWIEFB5CUAvG5mVmNbnUI8fnF3CvEB7IJcX9DxIkfsS08+4lMi3jHFMthJqY
N5RGelXh07FAawRDugRPO0gh5bHo+hxZnWYTW9s6+A/D9E3pP2OJcyvhnvtWUrjg
rgzylmjRmAH2KP86mK9jRwFvTdn666JZFKzpubei6XjWVdG/eS8=
-----END CERTIFICATE-----
`,
      // Colon-delimited list of permitted SSL Ciphers
      ciphers: "ECDHE-RSA-AES128-GCM-SHA256:!RC4",
      sniHostname: "httpbin.org",
    }
  );
}

function createValidFastlyBackend() {
  return new Backend(
    {
      name: 'fastly',
      target: 'www.fastly.com',
      hostOverride: "www.fastly.com",
      useSSL: true
    }
  );
}
