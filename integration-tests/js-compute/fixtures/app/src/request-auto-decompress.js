/* eslint-env serviceworker */
import { assert, assertRejects } from './assertions.js';
import { routes } from './routes.js';

// Request.fastly.decompressGzip option -- automatic gzip decompression of responses
routes.set('/request/constructor/fastly/decompressGzip/true', async () => {
  const request = new Request('https://httpbin.org/gzip', {
    headers: {
      accept: 'application/json',
    },
    backend: 'httpbin',
    fastly: {
      decompressGzip: true,
    },
  });
  const response = await fetch(request);
  // This should work because the response will be decompressed and valid json.
  const body = await response.json();
  assert(body.gzipped, true, `body.gzipped`);
});

routes.set('/request/constructor/fastly/decompressGzip/false', async () => {
  const request = new Request('https://httpbin.org/gzip', {
    headers: {
      accept: 'application/json',
    },
    backend: 'httpbin',
    fastly: {
      decompressGzip: false,
    },
  });
  const response = await fetch(request);
  await assertRejects(async function () {
    // This should throw because the response will be gzipped compressed, which we can not parse as json.
    await response.json();
  });
});

routes.set('/fetch/requestinit/fastly/decompressGzip/true', async () => {
  const response = await fetch('https://httpbin.org/gzip', {
    headers: {
      accept: 'application/json',
    },
    backend: 'httpbin',
    fastly: {
      decompressGzip: true,
    },
  });
  // This should work because the response will be decompressed and valid json.
  const body = await response.json();
  assert(body.gzipped, true, `body.gzipped`);
});

routes.set('/fetch/requestinit/fastly/decompressGzip/false', async () => {
  const response = await fetch('https://httpbin.org/gzip', {
    headers: {
      accept: 'application/json',
    },
    backend: 'httpbin',
    fastly: {
      decompressGzip: false,
    },
  });
  await assertRejects(async function () {
    // This should throw because the response will be gzipped compressed, which we can not parse as json.
    await response.json();
  });
});
