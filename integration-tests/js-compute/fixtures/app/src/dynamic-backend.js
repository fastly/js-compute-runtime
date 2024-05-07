/// <reference path="../../../../../types/index.d.ts" />
import { Backend } from 'fastly:backend';
import { CacheOverride } from 'fastly:cache-override';
import { allowDynamicBackends } from "fastly:experimental";
import { pass, assert, assertDoesNotThrow, assertThrows, assertRejects, assertResolves } from "./assertions.js";
import { isRunningLocally, routes } from "./routes.js";

/// The backend name is already in use.

routes.set("/backend/timeout", async () => {
    if (isRunningLocally()) {
      return pass('ok')
    }
    allowDynamicBackends(true);
    let backend = new Backend(
      {
        name: 'httpme1',
        target: 'http-me.glitch.me',
        hostOverride: "http-me.glitch.me",
        useSSL: true,
        dontPool: true,
        betweenBytesTimeout: 1_000,
        connectTimeout: 1_000,
        firstByteTimeout: 1_000
      }
    );
    console.time(`fetch('https://http-me.glitch.me/test?wait=5000'`)
    let error = await assertRejects(() => fetch('https://http-me.glitch.me/test?wait=5000', {
      backend,
      cacheOverride: new CacheOverride("pass"),
    }));
    console.timeEnd(`fetch('https://http-me.glitch.me/test?wait=5000'`)
    if (error) { return error }
    return pass('ok')
  });


// implicit dynamic backend
{
  routes.set("/implicit-dynamic-backend/dynamic-backends-disabled", async () => {
    allowDynamicBackends(false);
    let error = await assertRejects(() => fetch('https://http-me.glitch.me/headers'));
    if (error) { return error }
    return pass('ok')
  });
  routes.set("/implicit-dynamic-backend/dynamic-backends-enabled", async () => {
    allowDynamicBackends(true);
    let error = await assertResolves(() => fetch('https://http-me.glitch.me/headers'));
    if (error) { return error }
    error = await assertResolves(() => fetch('https://www.fastly.com'));
    if (error) { return error }
    return pass('ok')
  });
  routes.set("/implicit-dynamic-backend/dynamic-backends-enabled-called-twice", async () => {
    allowDynamicBackends(true);
    let error = await assertResolves(() => fetch('https://http-me.glitch.me/headers'));
    if (error) { return error }
    error = await assertResolves(() => fetch('https://http-me.glitch.me/headers'));
    if (error) { return error }
    return pass('ok')
  });
}

// explicit dynamic backend
{
  routes.set("/explicit-dynamic-backend/dynamic-backends-enabled-all-fields", async () => {
    allowDynamicBackends(true);
    let backend = createValidHttpMeBackend();
    let error = await assertResolves(() => fetch('https://http-me.glitch.me/headers', {
      backend,
      cacheOverride: new CacheOverride("pass"),
    }));
    if (error) { return error }
    return pass('ok')
  });
  routes.set("/explicit-dynamic-backend/dynamic-backends-enabled-minimal-fields", async () => {
    allowDynamicBackends(true);
    let backend = createValidFastlyBackend();
    let error = await assertResolves(() => fetch('https://www.fastly.com', {
      backend,
      cacheOverride: new CacheOverride("pass"),
    }));
    if (error) { return error }
    return pass('ok')
  });
}

// Backend
{
  routes.set("/backend/interface", async () => {
    let actual = Reflect.ownKeys(Backend)
    let expected = ["prototype", "exists", "fromName", "health", "length", "name"]
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

    actual = Reflect.getOwnPropertyDescriptor(Backend, 'exists')
    expected = {
      "value": Backend.exists,
      "writable": true,
      "enumerable": true,
      "configurable": true
    }
    error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(Backend, 'exists')`)
    if (error) { return error }

    error = assert(typeof Backend.exists, 'function', `typeof Backend.exists`)
    if (error) { return error }

    error = assert(Backend.exists.length, 1, `Backend.exists.length`)
    if (error) { return error }
    error = assert(Backend.exists.name, "exists", `Backend.exists.name`)
    if (error) { return error }

    actual = Reflect.getOwnPropertyDescriptor(Backend, 'fromName')
    expected = {
      "value": Backend.fromName,
      "writable": true,
      "enumerable": true,
      "configurable": true
    }
    error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(Backend, 'fromName')`)
    if (error) { return error }

    error = assert(typeof Backend.fromName, 'function', `typeof Backend.fromName`)
    if (error) { return error }

    error = assert(Backend.fromName.length, 1, `Backend.fromName.length`)
    if (error) { return error }
    error = assert(Backend.fromName.name, "fromName", `Backend.fromName.name`)
    if (error) { return error }

    actual = Reflect.getOwnPropertyDescriptor(Backend, 'health')
    expected = {
      "value": Backend.health,
      "writable": true,
      "enumerable": true,
      "configurable": true
    }
    error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(Backend, 'health')`)
    if (error) { return error }

    error = assert(typeof Backend.health, 'function', `typeof Backend.health`)
    if (error) { return error }

    error = assert(Backend.health.length, 1, `Backend.health.length`)
    if (error) { return error }
    error = assert(Backend.health.name, "health", `Backend.health.name`)
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
    expected = ["constructor", "toString", "toName"]
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
    error = assert(typeof Backend.prototype.toName, 'function', `typeof Backend.prototype.toName`)
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

    actual = Reflect.getOwnPropertyDescriptor(Backend.prototype.toName, 'length')
    expected = {
      "value": 0,
      "writable": false,
      "enumerable": false,
      "configurable": true
    }
    error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(Backend.prototype.toName, 'length')`)
    if (error) { return error }

    actual = Reflect.getOwnPropertyDescriptor(Backend.prototype.toName, 'name')
    expected = {
      "value": "toName",
      "writable": false,
      "enumerable": false,
      "configurable": true
    }
    error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(Backend.prototype.toName, 'name')`)
    if (error) { return error }

    return pass('ok')
  });

  // constructor
  {

    routes.set("/backend/constructor/called-as-regular-function", async () => {
      let error = assertThrows(() => {
        Backend()
      }, TypeError, `calling a builtin Backend constructor without new is forbidden`)
      if (error) { return error }
      return pass('ok')
    });
    routes.set("/backend/constructor/empty-parameter", async () => {
      let error = assertThrows(() => {
        new Backend()
      }, TypeError, `Backend constructor: At least 1 argument required, but only 0 passed`)
      if (error) { return error }
      return pass('ok')
    });
    routes.set("/backend/constructor/parameter-not-an-object", async () => {
      const constructorArguments = [true, false, 1, 1n, "hello", null, undefined, Symbol(), NaN]
      for (const argument of constructorArguments) {
        let error = assertThrows(() => {
          new Backend(argument)
        }, TypeError, `Backend constructor: configuration parameter must be an Object`)
        if (error) { return error }
      }
      return pass('ok')
    });
    // name property
    {
      routes.set("/backend/constructor/parameter-name-property-null", async () => {
        let error = assertThrows(() => {
          new Backend({ name: null })
        }, TypeError, `Backend constructor: name can not be null or undefined`)
        if (error) { return error }
        return pass('ok')
      });
      routes.set("/backend/constructor/parameter-name-property-undefined", async () => {
        let error = assertThrows(() => {
          new Backend({ name: undefined })
        }, TypeError, `Backend constructor: name can not be null or undefined`)
        if (error) { return error }
        return pass('ok')
      });
      routes.set("/backend/constructor/parameter-name-property-too-long", async () => {
        let error = assertThrows(() => {
          new Backend({ name: "a".repeat(255) })
        }, TypeError, `Backend constructor: name can not be more than 254 characters`)
        if (error) { return error }
        return pass('ok')
      });
      routes.set("/backend/constructor/parameter-name-property-empty-string", async () => {
        let error = assertThrows(() => {
          new Backend({ name: "" })
        }, TypeError, `Backend constructor: name can not be an empty string`)
        if (error) { return error }
        return pass('ok')
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
        return pass('ok')
      });
    }

    // target property
    {
      routes.set("/backend/constructor/parameter-target-property-null", async () => {
        let error = assertThrows(() => {
          new Backend({ name: 'a', target: null })
        }, TypeError, `Backend constructor: target can not be null or undefined`)
        if (error) { return error }
        return pass('ok')
      });
      routes.set("/backend/constructor/parameter-target-property-undefined", async () => {
        let error = assertThrows(() => {
          new Backend({ name: 'a', target: undefined })
        }, TypeError, `Backend constructor: target can not be null or undefined`)
        if (error) { return error }
        return pass('ok')
      });
      routes.set("/backend/constructor/parameter-target-property-empty-string", async () => {
        let error = assertThrows(() => {
          new Backend({ name: 'a', target: "" })
        }, TypeError, `Backend constructor: target can not be an empty string`)
        if (error) { return error }
        return pass('ok')
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
        return pass('ok')
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
        return pass('ok')
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
        return pass('ok')
      });
    }

    // ciphers property
    {
      routes.set("/backend/constructor/parameter-ciphers-property-empty-string", async () => {
        let error = assertThrows(() => {
          new Backend({ name: 'a', target: 'a', ciphers: "" })
        }, TypeError, `Backend constructor: ciphers can not be an empty string`)
        if (error) { return error }
        return pass('ok')
      });
      routes.set("/backend/constructor/parameter-ciphers-property-invalid-cipherlist-string", async () => {
        let error = assertThrows(() => {
          new Backend({ name: 'a', target: 'a', ciphers: ",;'^%$#^" })
        }, TypeError, `Backend constructor: none of the provided ciphers are supported by Fastly. The list of supported ciphers is available on https://developer.fastly.com/learning/concepts/routing-traffic-to-fastly/#use-a-tls-configuration`)
        if (error) { return error }
        return pass('ok')
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
        return pass('ok')
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
        return pass('ok')
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
        return pass('ok')
      });
    }

    // hostOverride property
    {
      routes.set("/backend/constructor/parameter-hostOverride-property-empty-string", async () => {
        let error = assertThrows(() => {
          new Backend({ name: 'hostOverride-property-empty-string', target: 'a', hostOverride: "" })
        }, TypeError, `Backend constructor: hostOverride can not be an empty string`)
        if (error) { return error }
        return pass('ok')
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
        return pass('ok')
      });

      routes.set("/backend/constructor/parameter-hostOverride-property-valid-string", async () => {
        let error = assertDoesNotThrow(() => {
          new Backend({ name: 'hostOverride-property-valid-string', target: 'a', hostOverride: "www.fastly.com" })
        })
        if (error) { return error }
        return pass('ok')
      });
    }

    // connectTimeout property
    {
      routes.set("/backend/constructor/parameter-connectTimeout-property-negative-number", async () => {
        let error = assertThrows(() => {
          new Backend({ name: 'connectTimeout-property-negative-number', target: 'a', connectTimeout: -1 })
        }, RangeError, `Backend constructor: connectTimeout can not be a negative number`)
        if (error) { return error }
        return pass('ok')
      });
      routes.set("/backend/constructor/parameter-connectTimeout-property-too-big", async () => {
        let error = assertThrows(() => {
          new Backend({ name: 'connectTimeout-property-too-big', target: 'a', connectTimeout: Math.pow(2, 32) })
        }, RangeError, `Backend constructor: connectTimeout must be less than 2^32`)
        if (error) { return error }
        return pass('ok')
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
        return pass('ok')
      });

      routes.set("/backend/constructor/parameter-connectTimeout-property-valid-number", async () => {
        let error = assertDoesNotThrow(() => {
          new Backend({ name: 'connectTimeout-property-valid-number', target: 'a', connectTimeout: 1 })
        })
        if (error) { return error }
        return pass('ok')
      });
    }

    // firstByteTimeout property
    {
      routes.set("/backend/constructor/parameter-firstByteTimeout-property-negative-number", async () => {
        let error = assertThrows(() => {
          new Backend({ name: 'firstByteTimeout-property-negative-number', target: 'a', firstByteTimeout: -1 })
        }, RangeError, `Backend constructor: firstByteTimeout can not be a negative number`)
        if (error) { return error }
        return pass('ok')
      });
      routes.set("/backend/constructor/parameter-firstByteTimeout-property-too-big", async () => {
        let error = assertThrows(() => {
          new Backend({ name: 'firstByteTimeout-property-too-big', target: 'a', firstByteTimeout: Math.pow(2, 32) })
        }, RangeError, `Backend constructor: firstByteTimeout must be less than 2^32`)
        if (error) { return error }
        return pass('ok')
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
        return pass('ok')
      });

      routes.set("/backend/constructor/parameter-firstByteTimeout-property-valid-number", async () => {
        let error = assertDoesNotThrow(() => {
          new Backend({ name: 'firstByteTimeout-property-valid-number', target: 'a', firstByteTimeout: 1 })
        })
        if (error) { return error }
        return pass('ok')
      });
    }

    // betweenBytesTimeout property
    {
      routes.set("/backend/constructor/parameter-betweenBytesTimeout-property-negative-number", async () => {
        let error = assertThrows(() => {
          new Backend({ name: 'betweenBytesTimeout-property-negative-number', target: 'a', betweenBytesTimeout: -1 })
        }, RangeError, `Backend constructor: betweenBytesTimeout can not be a negative number`)
        if (error) { return error }
        return pass('ok')
      });
      routes.set("/backend/constructor/parameter-betweenBytesTimeout-property-too-big", async () => {
        let error = assertThrows(() => {
          new Backend({ name: 'betweenBytesTimeout-property-too-big', target: 'a', betweenBytesTimeout: Math.pow(2, 32) })
        }, RangeError, `Backend constructor: betweenBytesTimeout must be less than 2^32`)
        if (error) { return error }
        return pass('ok')
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
        return pass('ok')
      });

      routes.set("/backend/constructor/parameter-betweenBytesTimeout-property-valid-number", async () => {
        let error = assertDoesNotThrow(() => {
          new Backend({ name: 'betweenBytesTimeout-property-valid-number', target: 'a', betweenBytesTimeout: 1 })
        })
        if (error) { return error }
        return pass('ok')
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
        return pass('ok')
      });
    }

    // dontPool property
    {
      routes.set("/backend/constructor/parameter-dontPool-property-valid-boolean", async () => {
        const types = [{}, [], Symbol(), 1, "2"];
        for (const type of types) {
          let error = assertDoesNotThrow(() => {
            new Backend({ name: 'dontPool-property-valid-boolean' + String(type), target: 'a', dontPool: type })
          })
          if (error) { return error }
        }
        let error = assertDoesNotThrow(() => {
          new Backend({ name: 'dontPool-property-valid-boolean-true', target: 'a', dontPool: true })
        })
        if (error) { return error }
        error = assertDoesNotThrow(() => {
          new Backend({ name: 'dontPool-property-valid-boolean-false', target: 'a', dontPool: false })
        })
        if (error) { return error }
        return pass('ok')
      });
    }

    // tlsMinVersion property
    {
      routes.set("/backend/constructor/parameter-tlsMinVersion-property-nan", async () => {
        let error = assertThrows(() => {
          new Backend({ name: 'tlsMinVersion-property-nan', target: 'a', tlsMinVersion: NaN })
        }, RangeError, `Backend constructor: tlsMinVersion must be either 1, 1.1, 1.2, or 1.3`)
        if (error) { return error }
        return pass('ok')
      });
      routes.set("/backend/constructor/parameter-tlsMinVersion-property-invalid-number", async () => {
        let error = assertThrows(() => {
          new Backend({ name: 'tlsMinVersion-property-invalid-number', target: 'a', tlsMinVersion: 1.4 })
        }, RangeError, `Backend constructor: tlsMinVersion must be either 1, 1.1, 1.2, or 1.3`)
        if (error) { return error }
        return pass('ok')
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
        return pass('ok')
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
        return pass('ok')
      });

      routes.set("/backend/constructor/parameter-tlsMinVersion-greater-than-tlsMaxVersion", async () => {
        let error = assertThrows(() => {
          new Backend({ name: 'tlsMinVersion-property-valid-number', target: 'a', tlsMinVersion: 1.3, tlsMaxVersion: 1.2 })
        }, RangeError, `Backend constructor: tlsMinVersion must be less than or equal to tlsMaxVersion`)
        if (error) { return error }
        return pass('ok')
      });
    }

    // tlsMaxVersion property
    {
      routes.set("/backend/constructor/parameter-tlsMaxVersion-property-nan", async () => {
        let error = assertThrows(() => {
          new Backend({ name: 'tlsMaxVersion-property-nan', target: 'a', tlsMaxVersion: NaN })
        }, RangeError, `Backend constructor: tlsMaxVersion must be either 1, 1.1, 1.2, or 1.3`)
        if (error) { return error }
        return pass('ok')
      });
      routes.set("/backend/constructor/parameter-tlsMaxVersion-property-invalid-number", async () => {
        let error = assertThrows(() => {
          new Backend({ name: 'tlsMaxVersion-property-invalid-number', target: 'a', tlsMaxVersion: 1.4 })
        }, RangeError, `Backend constructor: tlsMaxVersion must be either 1, 1.1, 1.2, or 1.3`)
        if (error) { return error }
        return pass('ok')
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
        return pass('ok')
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
        return pass('ok')
      });
    }

    // certificateHostname property
    {
      routes.set("/backend/constructor/parameter-certificateHostname-property-empty-string", async () => {
        let error = assertThrows(() => {
          new Backend({ name: 'certificateHostname-property-empty-string', target: 'a', certificateHostname: "" })
        }, TypeError, `Backend constructor: certificateHostname can not be an empty string`)
        if (error) { return error }
        return pass('ok')
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
        return pass('ok')
      });

      routes.set("/backend/constructor/parameter-certificateHostname-property-valid-string", async () => {
        let error = assertDoesNotThrow(() => {
          new Backend({ name: 'certificateHostname-property-valid-string', target: 'a', certificateHostname: "www.fastly.com" })
        })
        if (error) { return error }
        return pass('ok')
      });
    }

    // caCertificate property
    {
      routes.set("/backend/constructor/parameter-caCertificate-property-empty-string", async () => {
        let error = assertThrows(() => {
          new Backend({ name: 'caCertificate-property-empty-string', target: 'a', caCertificate: "" })
        }, TypeError, `Backend constructor: caCertificate can not be an empty string`)
        if (error) { return error }
        return pass('ok')
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
        return pass('ok')
      });

      routes.set("/backend/constructor/parameter-caCertificate-property-valid-string", async () => {
        let error = assertDoesNotThrow(() => {
          new Backend({ name: 'caCertificate-property-valid-string', target: 'a', caCertificate: "www.fastly.com" })
        })
        if (error) { return error }
        return pass('ok')
      });
    }

    // sniHostname property
    {
      routes.set("/backend/constructor/parameter-sniHostname-property-empty-string", async () => {
        let error = assertThrows(() => {
          new Backend({ name: 'sniHostname-property-empty-string', target: 'a', sniHostname: "" })
        }, TypeError, `Backend constructor: sniHostname can not be an empty string`)
        if (error) { return error }
        return pass('ok')
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
        return pass('ok')
      });

      routes.set("/backend/constructor/parameter-sniHostname-property-valid-string", async () => {
        let error = assertDoesNotThrow(() => {
          new Backend({ name: 'sniHostname-property-valid-string', target: 'a', sniHostname: "www.fastly.com" })
        })
        if (error) { return error }
        return pass('ok')
      });
    }
  }

  // exists
  {
    routes.set("/backend/exists/called-as-constructor-function", async () => {
      let error = assertThrows(() => {
        new Backend.exists()
      }, TypeError, `Backend.exists is not a constructor`)
      if (error) { return error }
      return pass('ok')
    });
    routes.set("/backend/exists/empty-parameter", async () => {
      let error = assertThrows(() => {
        Backend.exists()
      }, TypeError, `Backend.exists: At least 1 argument required, but only 0 passed`)
      if (error) { return error }
      return pass('ok')
    });
    // https://tc39.es/ecma262/#sec-tostring
    routes.set("/backend/exists/parameter-calls-7.1.17-ToString", async () => {
      let sentinel;
      const test = () => {
        sentinel = Symbol();
        const name = {
          toString() {
            throw sentinel;
          }
        }
        Backend.exists(name)
      }
      let error = assertThrows(test)
      if (error) { return error }
      try {
        test()
      } catch (thrownError) {
        let error = assert(thrownError, sentinel, 'thrownError === sentinel')
        if (error) { return error }
      }
      error = assertThrows(() => Backend.exists(Symbol()), TypeError, `can't convert symbol to string`)
      if (error) { return error }
      return pass('ok')
    });

    routes.set("/backend/exists/parameter-invalid", async () => {
      // null
      let error = assertThrows(() => Backend.exists(null), TypeError)
      if (error) { return error }
      // undefined
      error = assertThrows(() => Backend.exists(undefined), TypeError)
      if (error) { return error }
      // .length > 254
      error = assertThrows(() => Backend.exists('a'.repeat(255)), TypeError)
      if (error) { return error }
      // .length == 0
      error = assertThrows(() => Backend.exists(''), TypeError)
      if (error) { return error }
      return pass('ok')
    });
    routes.set("/backend/exists/happy-path-backend-exists", async () => {
      let error = assert(Backend.exists('TheOrigin'), true, `Backend.exists('TheOrigin')`)
      if (error) { return error }
      return pass('ok')
    });
    routes.set("/backend/exists/happy-path-backend-does-not-exist", async () => {
      let error = assert(Backend.exists('meow'), false, `Backend.exists('meow')`)
      if (error) { return error }
      return pass('ok')
    });
  }

  // fromName
  {
    routes.set("/backend/fromName/called-as-constructor-function", async () => {
      let error = assertThrows(() => {
        new Backend.fromName()
      }, TypeError, `Backend.fromName is not a constructor`)
      if (error) { return error }
      return pass('ok')
    });
    routes.set("/backend/fromName/empty-parameter", async () => {
      let error = assertThrows(() => {
        Backend.fromName()
      }, TypeError, `Backend.fromName: At least 1 argument required, but only 0 passed`)
      if (error) { return error }
      return pass('ok')
    });
    // https://tc39.es/ecma262/#sec-tostring
    routes.set("/backend/fromName/parameter-calls-7.1.17-ToString", async () => {
      let sentinel;
      const test = () => {
        sentinel = Symbol();
        const name = {
          toString() {
            throw sentinel;
          }
        }
        Backend.fromName(name)
      }
      let error = assertThrows(test)
      if (error) { return error }
      try {
        test()
      } catch (thrownError) {
        let error = assert(thrownError, sentinel, 'thrownError === sentinel')
        if (error) { return error }
      }
      error = assertThrows(() => Backend.fromName(Symbol()), TypeError, `can't convert symbol to string`)
      if (error) { return error }
      return pass('ok')
    });

    routes.set("/backend/fromName/parameter-invalid", async () => {
      // null
      let error = assertThrows(() => Backend.fromName(null), TypeError)
      if (error) { return error }
      // undefined
      error = assertThrows(() => Backend.fromName(undefined), TypeError)
      if (error) { return error }
      // .length > 254
      error = assertThrows(() => Backend.fromName('a'.repeat(255)), TypeError)
      if (error) { return error }
      // .length == 0
      error = assertThrows(() => Backend.fromName(''), TypeError)
      if (error) { return error }
      return pass('ok')
    });
    routes.set("/backend/fromName/happy-path-backend-exists", async () => {
      allowDynamicBackends(false);
      let error = assert(Backend.fromName('TheOrigin') instanceof Backend, true, `Backend.fromName('TheOrigin') instanceof Backend`)
      if (error) { return error }

      error = await assertResolves(() => fetch('https://http-me.glitch.me/headers', {
        backend: Backend.fromName('TheOrigin'),
      }));
      if (error) { return error }

      return pass('ok')
    });
    routes.set("/backend/fromName/happy-path-backend-does-not-exist", async () => {
      let error = assertThrows(() => Backend.fromName('meow'), Error, "Backend.fromName: backend named 'meow' does not exist")
      if (error) { return error }
      return pass('ok')
    });
  }

  // health
  {
    routes.set("/backend/health/called-as-constructor-function", async () => {
      let error = assertThrows(() => {
        new Backend.health()
      }, TypeError)
      if (error) { return error }
      return pass('ok')
    });
    routes.set("/backend/health/empty-parameter", async () => {
      let error = assertThrows(() => {
        Backend.health()
      }, TypeError, `Backend.health: At least 1 argument required, but only 0 passed`)
      if (error) { return error }
      return pass('ok')
    });
    // https://tc39.es/ecma262/#sec-tostring
    routes.set("/backend/health/parameter-calls-7.1.17-ToString", async () => {
      let sentinel;
      const test = () => {
        sentinel = Symbol();
        const name = {
          toString() {
            throw sentinel;
          }
        }
        Backend.health(name)
      }
      let error = assertThrows(test)
      if (error) { return error }
      try {
        test()
      } catch (thrownError) {
        let error = assert(thrownError, sentinel, 'thrownError === sentinel')
        if (error) { return error }
      }
      error = assertThrows(() => Backend.health(Symbol()), TypeError, `can't convert symbol to string`)
      if (error) { return error }
      return pass('ok')
    });

    routes.set("/backend/health/parameter-invalid", async () => {
      // null
      let error = assertThrows(() => Backend.health(null), TypeError)
      if (error) { return error }
      // undefined
      error = assertThrows(() => Backend.health(undefined), TypeError)
      if (error) { return error }
      // .length > 254
      error = assertThrows(() => Backend.health('a'.repeat(255)), TypeError)
      if (error) { return error }
      // .length == 0
      error = assertThrows(() => Backend.health(''), TypeError)
      if (error) { return error }
      return pass('ok')
    });
    routes.set("/backend/health/happy-path-backend-exists", async () => {
      let error = assert(typeof Backend.health('TheOrigin'), 'string', "typeof Backend.health('TheOrigin')");
      if (error) { return error }
      error = assert(Backend.health('TheOrigin'), 'unknown', "Backend.health('TheOrigin')");
      if (error) { return error }

      return pass('ok')
    });
    routes.set("/backend/health/happy-path-backend-does-not-exist", async () => {
      let error = assertThrows(() => Backend.health('meow'), Error, "Backend.health: backend named 'meow' does not exist")
      if (error) { return error }
      return pass('ok')
    });
  }
}

function createValidHttpMeBackend() {
  // We are defining all the possible fields here but any number of fields can be defined - the ones which are not defined will use their default value instead.
  return new Backend(
    {
      name: 'http-me',
      target: 'http-me.glitch.me',
      hostOverride: "http-me.glitch.me",
      connectTimeout: 1000,
      firstByteTimeout: 15000,
      betweenBytesTimeout: 10000,
      useSSL: true,
      dontPool: false,
      tlsMinVersion: 1.2,
      tlsMaxVersion: 1.2,
      certificateHostname: "http-me.glitch.me",
      // Colon-delimited list of permitted SSL Ciphers
      ciphers: "ECDHE-RSA-AES128-GCM-SHA256:!RC4",
      sniHostname: "http-me.glitch.me",
    }
  );
}

function createValidFastlyBackend() {
  return new Backend(
    {
      name: 'fastly',
      target: 'www.fastly.com',
      hostOverride: "www.fastly.com",
      useSSL: true,
      dontPool: true,
    }
  );
}
