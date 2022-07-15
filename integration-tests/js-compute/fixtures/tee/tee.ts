addEventListener("fetch", event => event.respondWith(handleRequest(event.request)));

async function handleRequest(req: Request) {
  let [body1, _body2] = req.body.tee();

  // Regression test for making requests whose bodies are streams that result
  // from a `tee`. The bug this is guarding against is where we were eagerly
  // placing the request into the pending queue, even though its body stream had
  // not been closed yet. This resulted in deadlock when we called
  // `pending_req_select`, as we were waiting on an http request whose body had
  // not been closed.
  let res = fetch(new Request(req.url, {
    body: body1,
    headers: req.headers,
    method: req.method,
    backend: "TheOrigin"
  }));

  return res;
}
