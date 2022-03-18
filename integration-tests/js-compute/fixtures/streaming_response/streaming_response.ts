/// <reference types="@fastly/js-compute" />

addEventListener("fetch", (event) => {
  // Build a response
  let headers = new Headers();
  headers.set("JSHeader", "JSValue");

  let streamController!: ReadableStreamDefaultController;
  let stream = new ReadableStream({
    start: (controller: ReadableStreamDefaultController) => {
      streamController = controller;
    },
  });
  let response = new Response(stream, {
    headers,
  });

  streamController.enqueue(new TextEncoder().encode("11"));

  // Send downstream (back to the client)
  // Get our stream to keep writing
  event.respondWith(response);

  // Unblock the server
  let upstreamRequest = new Request("http://provider.org/TheURL");
  fetch(upstreamRequest, {
    backend: "TheOrigin",
  }).then((_) => {
    // Continue writing
    streamController.enqueue(new TextEncoder().encode("24"));
  });
});
