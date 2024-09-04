/* eslint-env serviceworker */
import { routes } from "./routes.js";

routes.set("/tee", async function (event) {
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
    new Request("/post", {
      body: body1,
      headers: req.headers,
      method: req.method,
      backend: "httpbin",
    }),
  );
  let body = await res.json();

  return new Response(body.data);
});

routes.set("/tee/error", async function (event) {
  const req = event.request;
  let res = fetch("/post", {
    method: "POST",
    body: new ReadableStream({
      start: (controller) => {
        controller.enqueue("Test");
        controller.close();
      },
    }),
    backend: "httpbin",
  });

  return res
    .then(() => new Response("Error wasn't raised"))
    .catch((err) => {
      return new Response(err.toString());
    });
});
