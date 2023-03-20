addEventListener("fetch", (event) =>
  event.respondWith(handleRequest(event.request))
);

async function handleRequest(req) {
  if (req.url.endsWith("/tee")) {
    // eslint-disable-next-line no-unused-vars
    let [body1, _body2] = req.body.tee();

    // Regression test for making requests whose bodies are streams that result
    // from a `tee`. The bug this is guarding against is where we were eagerly
    // placing the request into the pending queue, even though its body stream had
    // not been closed yet. This resulted in deadlock when we called
    // `pending_req_select`, as we were waiting on an http request whose body had
    // not been closed.
    let res = fetch(
      new Request(req.url, {
        body: body1,
        headers: req.headers,
        method: req.method,
        backend: "TheOrigin",
      })
    );

    return res;
  }

  if (req.url.endsWith("/error")) {
    console.log(req.method);
    let res = fetch(req.url, {
      method: "POST",
      body: new ReadableStream({
        start: (controller) => {
          controller.enqueue("Test");
          controller.close();
        },
      }),
      backend: "TheOrigin",
    });

    return res
      .then(() => new Response("Error wasn't raised"))
      .catch((err) => {
        console.log(err.toString());
        return new Response(err.toString());
      });
  }

  return new Response("");
}
