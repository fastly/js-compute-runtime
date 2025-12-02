/// <reference path="../../../../../types/index.d.ts" />
/* eslint-env serviceworker */

import { CacheOverride } from 'fastly:cache-override';
import { assert, assertRejects } from './assertions.js';
import { routes, isRunningLocally } from './routes.js';

async function requestInitObjectLiteral(manualFramingHeaders) {
  let request = new Request('https://http-me.fastly.dev/anything', {
    backend: 'httpme',
    method: 'POST',
    body: 'meow',
    manualFramingHeaders,
    headers: {
      'content-length': '1',
    },
  });
  let response = await fetch(request);
  let body = await response.json();
  return body?.headers?.['content-length'];
}

async function requestMethod(manualFramingHeaders) {
  let request = new Request('https://http-me.fastly.dev/anything', {
    backend: 'httpme',
    method: 'POST',
    body: 'meow',
    headers: {
      'content-length': '1',
    },
  });
  request.setManualFramingHeaders(manualFramingHeaders);
  let response = await fetch(request);
  let body = await response.json();
  return body?.headers?.['content-length'];
}

async function requestClone(manualFramingHeaders) {
  let request = new Request('https://http-me.fastly.dev/anything', {
    backend: 'httpme',
    method: 'POST',
    body: 'meow',
    manualFramingHeaders,
    headers: {
      'content-length': '1',
    },
  });
  let response = await fetch(request.clone());
  let body = await response.json();
  return body?.headers?.['content-length'];
}

async function fetchInitObjectLiteral(manualFramingHeaders) {
  let response = await fetch('https://http-me.fastly.dev/anything', {
    backend: 'httpme',
    method: 'POST',
    body: 'meow',
    manualFramingHeaders,
    headers: {
      'content-length': '1',
    },
  });
  let body = await response.json();
  return body?.headers?.['content-length'];
}

routes.set(
  '/override-content-length/request/init/object-literal/true',
  async () => {
    if (isRunningLocally()) {
      await assertRejects(() => requestInitObjectLiteral(true));
    } else {
      let actual = await requestInitObjectLiteral(true);
      let expected = '1';
      assert(actual, expected, `await requestInitObjectLiteral(true)`);
    }
  },
);

routes.set(
  '/override-content-length/request/init/object-literal/false',
  async () => {
    let actual = await requestInitObjectLiteral(false);
    let expected = '4';
    assert(actual, expected, `await requestInitObjectLiteral(false)`);
  },
);

routes.set(
  '/override-content-length/request/method/object-literal/true',
  async () => {
    if (isRunningLocally()) {
      await assertRejects(() => requestMethod(true));
    } else {
      let actual = await requestMethod(true);
      let expected = '1';
      assert(actual, expected, `await requestMethod(true)`);
    }
  },
);

routes.set(
  '/override-content-length/request/method/object-literal/false',
  async () => {
    let actual = await requestMethod(false);
    let expected = '4';
    assert(actual, expected, `await requestMethod(false)`);
  },
);

routes.set('/override-content-length/request/clone/true', async () => {
  if (isRunningLocally()) {
    await assertRejects(() => requestClone(true));
  } else {
    let actual = await requestClone(true);
    let expected = '1';
    assert(actual, expected, `await requestClone(true)`);
  }
});

routes.set('/override-content-length/request/clone/false', async () => {
  let actual = await requestClone(false);
  let expected = '4';
  assert(actual, expected, `await requestClone(false)`);
});

routes.set(
  '/override-content-length/fetch/init/object-literal/true',
  async () => {
    if (isRunningLocally()) {
      await assertRejects(() => fetchInitObjectLiteral(true));
    } else {
      let actual = await fetchInitObjectLiteral(true);
      let expected = '1';
      assert(actual, expected, `await fetchInitObjectLiteral(true)`);
    }
  },
);

routes.set(
  '/override-content-length/fetch/init/object-literal/false',
  async () => {
    let actual = await fetchInitObjectLiteral(false);
    let expected = '4';
    assert(actual, expected, `await fetchInitObjectLiteral(false)`);
  },
);

async function responseInitObjectLiteral(manualFramingHeaders) {
  let response = new Response(
    new ReadableStream({
      start(controller) {
        controller.enqueue(new TextEncoder().encode('meow'));
        controller.close();
      },
    }),
    {
      manualFramingHeaders,
      headers: {
        'content-length': '4',
      },
    },
  );
  return response;
}

async function responseInitresponseInstance(manualFramingHeaders) {
  let response = new Response(
    new ReadableStream({
      start(controller) {
        controller.enqueue(new TextEncoder().encode('meow'));
        controller.close();
      },
    }),
    {
      manualFramingHeaders,
      headers: {
        'content-length': '4',
      },
    },
  );
  response = new Response(
    new ReadableStream({
      start(controller) {
        controller.enqueue(new TextEncoder().encode('meow'));
        controller.close();
      },
    }),
    response,
  );
  return response;
}

routes.set(
  '/override-content-length/response/init/object-literal/true',
  async () => {
    return responseInitObjectLiteral(true);
  },
);

routes.set(
  '/override-content-length/response/init/object-literal/false',
  async () => {
    return responseInitObjectLiteral(false);
  },
);

routes.set(
  '/override-content-length/response/init/response-instance/true',
  async () => {
    return responseInitresponseInstance(true);
  },
);

routes.set(
  '/override-content-length/response/init/response-instance/false',
  async () => {
    return responseInitresponseInstance(false);
  },
);

async function responseMethod(setManualFramingHeaders) {
  const response = await fetch('https://httpbin.org/stream-bytes/11', {
    backend: 'httpbin',
    cacheOverride: new CacheOverride('pass'),
  });
  response.setManualFramingHeaders(setManualFramingHeaders);
  response.headers.set('content-length', '11');
  response.headers.delete('transfer-encoding');
  return response;
}

routes.set('/override-content-length/response/method/false', async () => {
  return responseMethod(false);
});

routes.set('/override-content-length/response/method/true', async () => {
  return responseMethod(true);
});

async function responseClone(setManualFramingHeaders) {
  let response = new Response('meow', {
    backend: 'httpme',
    method: 'POST',
    body: 'meow',
    setManualFramingHeaders,
    headers: {
      'transfer-encoding': 'chunked',
    },
  });
  response = response.clone();
  return response;
}

// TODO: implement Response.clone
routes.set('/override-content-length/response/clone/true', async () => {
  return responseClone(true);
});

// TODO: implement Response.clone
routes.set('/override-content-length/response/clone/false', async () => {
  return responseClone(false);
});
