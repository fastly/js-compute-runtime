/* eslint-env serviceworker */
/* global fastly */

// This module contains routes that allow us to verify the behaviour
// of interleaving different types of downstream responses, such as
// WebSocket redirects mixed with normal HTTP responses, mixed with
// streaming responses to backend requests.

import { assert } from './assertions.js';
import { CacheOverride } from 'fastly:cache-override';
import { getGeolocationForIpAddress } from 'fastly:geolocation';
import { createFanoutHandoff } from 'fastly:fanout';
import { routes } from './routes.js';

function timeout(ms) {
  return new Promise((resolve) => setTimeout(resolve, ms));
}

// Handler for generating the websocket redirect hostcall:
routes.set('/createFanoutHandoff', (event) => {
  return createFanoutHandoff(event.request, 'TheOrigin');
});

// Handler for generating  a backend request to http-me.fastly.dev:
routes.set('/httpMeRequest', async (event) => {
  const req = new Request(
    'https://http-me.fastly.dev/status=200&header=FooName:FooValue',
    {
      method: 'GET',
      headers: event.request.headers,
    },
  );

  const resp = await fetch(req, {
    backend: 'httpme',
    cacheOverride: new CacheOverride('pass'),
  });

  return resp;
});

routes.set('/getGeolocationForIpAddress', async (event) => {
  let ip = event.request.headers.get('x-forwarded-for');
  assert(ip !== undefined, 'request has x-forwarded-for header');

  // Sleep to ensure other items on the event loop get a chance to run:
  await timeout(1000);

  let geo = getGeolocationForIpAddress(ip);
  assert(geo !== null, `resolved ${ip} to geo information`);

  return new Response('found', {
    status: 200,
    headers: new Headers({
      'geo-as-name': geo.as_name,
      'geo-as-number': geo.as_number,
      'geo-city': geo.city,
    }),
  });
});
