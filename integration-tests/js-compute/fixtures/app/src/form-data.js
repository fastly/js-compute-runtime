import { assert, assertDoesNotThrow } from './assertions.js';
import { routes } from './routes.js';

routes.set('/form-data/quoted-boundary', async () => {
  const data = new FormData();
  data.append('test', 'field');
  let response = await fetch('https://http-me.fastly.dev/anything', {
    method: 'POST',
    body: data,
  });
  assert(response.ok, true, 'response.ok === true');
  let body = await response.json();

  // Ensure that the boundary is quoted in the content-type header, as per RFC 2046, section 5.1.1.
  // StarlingMonkey's implementation returns random Base64 prefixed with --StarlingMonkeyFormBoundary,
  // so we can't assert the exact value, but we check that it is quoted and the Base64 part is valid.
  let contentType = body.headers['content-type'];
  let match = contentType.match(
    /boundary="--StarlingMonkeyFormBoundary([^"]+)"/,
  );
  assert(match !== null, true, 'content-type header has quoted boundary');
  assertDoesNotThrow(
    () => atob(match[1]),
    'boundary contains valid Base64 characters',
  );
  return new Response('ok');
});
