/// <reference types="@fastly/js-compute" />

addEventListener("fetch", (event) => {
  // Build a response
  let headers = new Headers();
  headers.set("JSHeader", "JSValue");
  let response = new Response("Hello JS", {
    status: 200,
    headers,
  });

  // Send our response back to the client
  event.respondWith(response);
});
