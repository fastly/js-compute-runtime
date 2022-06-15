/// <reference types="@fastly/js-compute" />

addEventListener("fetch", (event) => {
  let geo: Geolocation = fastly.getGeolocationForIpAddress("2.216.196.179");

  // Json response should be
  /*
  {
    "as_name":"heavenly uk limited",
    "as_number":5607,
    "area_code":0,
    "city":"city of westminster",
    "conn_speed":"broadband",
    "conn_type":"",
    "continent":"EU",
    "country_code":"GB",
    "country_code3":"GBR",
    "country_name":"United Kingdom",
    "gmt_offset":"100",
    "latitude":51.518,
    "longitude":-0.142,
    "metro_code":826044,
    "postal_code":"w1w 7bf",
    "proxy_description":"",
    "proxy_type":"",
    "region":"WSM",
    "utc_offset":100
  }
  */

  // Let's assert our values are correctly set
  assert_eq(geo.as_name, "heavenly uk limited");
  assert_eq(geo.as_number, 5607);
  assert_eq(geo.area_code, 0);
  assert_eq(geo.city, "city of westminster");
  assert_eq(geo.conn_speed, "broadband");
  assert_eq(geo.conn_type, "");
  assert_eq(geo.continent, "EU");
  assert_eq(geo.country_code, "GB");
  assert_eq(geo.country_code3, "GBR");
  assert_eq(geo.country_name, "United Kingdom");
  assert_eq(geo.gmt_offset, "100");
  assert_eq(geo.latitude, 51.518);
  assert_eq(geo.longitude, -0.142);
  assert_eq(geo.metro_code, 826044);
  assert_eq(geo.postal_code, "w1w 7bf");
  assert_eq(geo.proxy_description, "");
  assert_eq(geo.proxy_type, "");
  assert_eq(geo.region, "WSM");
  assert_eq(geo.utc_offset, 100);

  // Send a response back down
  event.respondWith(new Response(JSON.stringify(geo)));
});

function assert(condition: boolean) {
  if (!condition) {
    throw new Error("assert failed!");
  }
}

function assert_eq<T>(left: T, right: T) {
  if (left !== right) {
    throw new Error(`assert failed! ${left} !== ${right}`);
  }
}
