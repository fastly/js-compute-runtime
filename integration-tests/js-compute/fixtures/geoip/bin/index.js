/* eslint-env serviceworker */
/* global fastly */
import { env } from 'fastly:env';
import { getGeolocationForIpAddress } from 'fastly:geolocation';

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

routes.set("/fastly/getgeolocationforipaddress/interface", async function () {
  let actual = Reflect.getOwnPropertyDescriptor(fastly, 'getGeolocationForIpAddress')
  expected = {
    writable: true,
    enumerable: true,
    configurable: true,
    value: fastly.getGeolocationForIpAddress
  }
  let error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(fastly, 'getGeolocationForIpAddress)`)
  if (error) { return error }

  error = assert(typeof fastly.getGeolocationForIpAddress, 'function', `typeof fastly.getGeolocationForIpAddress`)
  if (error) { return error }

  actual = Reflect.getOwnPropertyDescriptor(fastly.getGeolocationForIpAddress, 'length')
  expected = {
    value: 1,
    writable: false,
    enumerable: false,
    configurable: true
  }
  error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(fastly.getGeolocationForIpAddress, 'length')`)
  if (error) { return error }

  actual = Reflect.getOwnPropertyDescriptor(fastly.getGeolocationForIpAddress, 'name')
  expected = {
    value: "getGeolocationForIpAddress",
    writable: false,
    enumerable: false,
    configurable: true
  }
  error = assert(actual, expected, `Reflect.getOwnPropertyDescriptor(fastly.getGeolocationForIpAddress, 'name')`)
  if (error) { return error }

  return pass()
});

routes.set("/fastly/getgeolocationforipaddress/called-as-constructor", async () => {
  let error = assertThrows(() => {
    new fastly.getGeolocationForIpAddress('1.2.3.4')
  }, TypeError, `fastly.getGeolocationForIpAddress is not a constructor`)
  if (error) { return error }
  return pass()
});
// https://tc39.es/ecma262/#sec-tostring
routes.set("/fastly/getgeolocationforipaddress/parameter-calls-7.1.17-ToString", async () => {
  let sentinel;
  const test = () => {
    sentinel = Symbol();
    const key = {
      toString() {
        throw sentinel;
      }
    }
    fastly.getGeolocationForIpAddress(key)
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
    fastly.getGeolocationForIpAddress(Symbol())
  }, Error, `can't convert symbol to string`)
  if (error) { return error }
  return pass()
});
routes.set("/fastly/getgeolocationforipaddress/parameter-not-supplied", async () => {
  let error = assertThrows(() => {
    fastly.getGeolocationForIpAddress()
  }, TypeError, `fastly.getGeolocationForIpAddress: At least 1 argument required, but only 0 passed`)
  if (error) { return error }
  return pass()
});
routes.set("/fastly/getgeolocationforipaddress/parameter-empty-string", async () => {
  let error = assertThrows(() => {
    fastly.getGeolocationForIpAddress('')
  }, Error, `Invalid address passed to fastly.getGeolocationForIpAddress`)
  if (error) { return error }
  return pass()
});

let ipv4Expected = {
  as_name: "sky uk limited",
  as_number: 5607,
  area_code: 0,
  city: "tower hamlets",
  conn_speed: "broadband",
  conn_type: "wifi",
  continent: "EU",
  country_code: "GB",
  country_code3: "GBR",
  country_name: "united kingdom",
  gmt_offset: 0,
  latitude: 51.52,
  longitude: -0.06,
  metro_code: 826044,
  postal_code: "e1 5bt",
  proxy_description: "?",
  proxy_type: "?",
  region: "TWH",
  utc_offset: 0
};

routes.set("/fastly/getgeolocationforipaddress/parameter-ipv4-string", async () => {
  let geo = fastly.getGeolocationForIpAddress('2.216.196.179')
  let error = assert(geo, ipv4Expected, `fastly.getGeolocationForIpAddress('2.216.196.179') == ipv4Expected`)
  if (error) { return error }
  return pass()
});

let expected = {
  as_name: "softlayer technologies inc.",
  as_number: 36351,
  area_code: 214,
  city: "dallas",
  conn_speed: "broadband",
  conn_type: "wired",
  continent: "NA",
  country_code: "US",
  country_code3: "USA",
  country_name: "united states",
  gmt_offset: -600,
  latitude: 32.94,
  longitude: -96.84,
  metro_code: 623,
  postal_code: "75244",
  proxy_description: "?",
  proxy_type: "hosting",
  region: "TX",
  utc_offset: -600
}
routes.set("/fastly/getgeolocationforipaddress/parameter-compressed-ipv6-string", async () => {
  let geo = fastly.getGeolocationForIpAddress('2607:f0d0:1002:51::4')
  console.log({ geo })
  let error = assert(geo, expected, `fastly.getGeolocationForIpAddress('2607:f0d0:1002:51::4') == expected`)
  if (error) { return error }
  return pass()
});
routes.set("/fastly/getgeolocationforipaddress/parameter-shortened-ipv6-string", async () => {
  let geo = fastly.getGeolocationForIpAddress('2607:f0d0:1002:0051:0:0:0:0004')
  let error = assert(geo, expected, `fastly.getGeolocationForIpAddress('2607:f0d0:1002:0051:0:0:0:0004') == expected`)
  if (error) { return error }
  return pass()
});
routes.set("/fastly/getgeolocationforipaddress/parameter-expanded-ipv6-string", async () => {
  let geo = fastly.getGeolocationForIpAddress('2607:f0d0:1002:0051:0000:0000:0000:0004')
  let error = assert(geo, expected, `fastly.getGeolocationForIpAddress('2607:f0d0:1002:0051:0000:0000:0000:0004') == expected`)
  if (error) { return error }
  return pass()
});
// TODO: Uncomment these tests once IPv4-in-6 is working in ipll
// routes.set("/fastly/getgeolocationforipaddress/parameter-dual-ipv4-ipv6-string", async () => {
//   let geo = fastly.getGeolocationForIpAddress('::FFFF:2.216.196.179')
//   let error = assert(geo, expected, `fastly.getGeolocationForIpAddress('::2.216.196.179') == expected`)
//   if (error) { return error }
//   return pass()
// });

routes.set("/fastly/getgeolocationforipaddress/called-unbound", async () => {
  let geo = fastly.getGeolocationForIpAddress.call(undefined, '2607:f0d0:1002:0051:0000:0000:0000:0004')
  let error = assert(geo, expected, `fastly.getGeolocationForIpAddress.call(undefined, '2607:f0d0:1002:0051:0000:0000:0000:0004') == expected`)
  if (error) { return error }
  return pass()
});

routes.set('/fastly:geolocation', async () => {
  let error = assert(getGeolocationForIpAddress, fastly.getGeolocationForIpAddress, 'getGeolocationForIpAddress === fastly.getGeolocationForIpAddress');
  if (error) { return error }
  return pass()
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
