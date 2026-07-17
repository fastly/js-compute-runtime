/* eslint-env serviceworker */
import { routes } from './routes.js';
import { assert } from './assertions.js';

routes.set('/tee', async function (event) {
  const req = event.request;
  // eslint-disable-next-line no-unused-vars
  let [body1, _body2] = req.body.tee();

  // Regression test for making requests whose bodies are streams that result
  // from a `tee`. The bug this is guarding against is where we were eagerly
  // placing the request into the pending queue, even though its body stream had
  // not been closed yet. This resulted in deadlock when we called
  // `pending_req_select`, as we were waiting on an http request whose body had
  // not been closed.
  let res = await fetch(
    new Request('/anything', {
      body: body1,
      headers: req.headers,
      method: req.method,
      backend: 'httpme',
    }),
  );
  let body = await res.json();
  assert(body['body'], 'hello world!');
});

routes.set('/tee/error', async function (event) {
  const req = event.request;
  let res = fetch('/anything', {
    method: 'POST',
    body: new ReadableStream({
      start: (controller) => {
        controller.enqueue('Test');
        controller.close();
      },
    }),
    backend: 'httpme',
  });

  return res
    .then(() => new Response("Error wasn't raised"))
    .catch((err) => {
      return new Response(err.toString());
    });
});
