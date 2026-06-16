import { assert, assertDoesNotThrow } from './assertions.js';
import { routes } from './routes.js';

routes.set('/form-data/boundary-is-content-type-safe', async () => {
  const data = new FormData();
  data.append('test', 'field');
  let response = await fetch('https://http-me.fastly.dev/anything', {
    method: 'POST',
    body: data,
  });
  assert(response.ok, true, 'response.ok === true');
  let body = await response.json();

  // Ensure that the boundary contains only alphanumeric characters, which are safe for use in a content type header.
  // The implementation randomly generates a string, so we can't assert the exact value without control over the PRNG seed.
  // If this test fails as a one-off, it indicates that the boundary generation is not producing a content-type-safe string,
  // which should NOT be ignored, as this has lead to 5XX errors in production in the past.
  let contentType = body.headers['content-type'];
  assert(
    /boundary="--StarlingMonkeyFormBoundary[a-zA-Z0-9]+"/.test(contentType),
    true,
    'content-type header contains a safe boundary string, DO NOT IGNORE FAILURES',
  );
  return new Response('ok');
});
