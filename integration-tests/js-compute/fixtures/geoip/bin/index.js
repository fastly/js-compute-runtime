/* eslint-env serviceworker */
/* global fastly */
import { pass, fail, assert, assertThrows } from "../../../assertions.js";
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
  let test_routes = Array.from(routes.keys())
  return new Response(JSON.stringify(test_routes), { 'headers': { 'content-type': 'application/json' } });
});

routes.set("/fastly/getgeolocationforipaddress/interface", async function () {
  let actual = Reflect.getOwnPropertyDescriptor(fastly, 'getGeolocationForIpAddress')
  let expected = {
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

let geoFields = ["as_name",
  "as_number",
  "area_code",
  "city",
  "conn_speed",
  "conn_type",
  "continent",
  "country_code",
  "country_code3",
  "country_name",
  "gmt_offset",
  "latitude",
  "longitude",
  "metro_code",
  "postal_code",
  "proxy_description",
  "proxy_type",
  "region",
  "utc_offset"]

routes.set("/fastly/getgeolocationforipaddress/parameter-ipv4-string", async () => {
  let geo = fastly.getGeolocationForIpAddress('2.216.196.179')
  let error = assert(Object.keys(geo), geoFields, `Object.keys(fastly.getGeolocationForIpAddress('2.216.196.179')) == geoFields`)
  if (error) { return error }
  return pass()
});

routes.set("/fastly/getgeolocationforipaddress/parameter-compressed-ipv6-string", async () => {
  let geo = fastly.getGeolocationForIpAddress('2607:f0d0:1002:51::4')
  let error = assert(Object.keys(geo), geoFields, `Object.keys(fastly.getGeolocationForIpAddress('2607:f0d0:1002:51::4')) == geoFields`)
  if (error) { return error }
  return pass()
});
routes.set("/fastly/getgeolocationforipaddress/parameter-shortened-ipv6-string", async () => {
  let geo = fastly.getGeolocationForIpAddress('2607:f0d0:1002:0051:0:0:0:0004')
  let error = assert(Object.keys(geo), geoFields, `Object.keys(fastly.getGeolocationForIpAddress('2607:f0d0:1002:0051:0:0:0:0004')) == geoFields`)
  if (error) { return error }
  return pass()
});
routes.set("/fastly/getgeolocationforipaddress/parameter-expanded-ipv6-string", async () => {
  let geo = fastly.getGeolocationForIpAddress('2607:f0d0:1002:0051:0000:0000:0000:0004')
  let error = assert(Object.keys(geo), geoFields, `Object.keys(fastly.getGeolocationForIpAddress('2607:f0d0:1002:0051:0000:0000:0000:0004')) == geoFields`)
  if (error) { return error }
  return pass()
});
// TODO: Uncomment these tests once IPv4-in-6 is working in ipll
// routes.set("/fastly/getgeolocationforipaddress/parameter-dual-ipv4-ipv6-string", async () => {
//   let geo = fastly.getGeolocationForIpAddress('::FFFF:2.216.196.179')
//   let error = assert(Object.keys(geo), geoFields, `Object.keys(fastly.getGeolocationForIpAddress('::FFFF:2.216.196.179')) == geoFields`)
//   if (error) { return error }
//   return pass()
// });

routes.set("/fastly/getgeolocationforipaddress/called-unbound", async () => {
  let geo = fastly.getGeolocationForIpAddress.call(undefined, '2607:f0d0:1002:0051:0000:0000:0000:0004')
  let error = assert(Object.keys(geo), geoFields, `Object.keys(fastly.getGeolocationForIpAddress('2607:f0d0:1002:0051:0000:0000:0000:0004')) == geoFields`)
  if (error) { return error }
  return pass()
});

routes.set('/fastly:geolocation', async () => {
  let error = assert(getGeolocationForIpAddress, fastly.getGeolocationForIpAddress, 'getGeolocationForIpAddress === fastly.getGeolocationForIpAddress');
  if (error) { return error }
  return pass()
});
