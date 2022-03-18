/// <reference types="@fastly/js-compute" />

addEventListener("fetch", (event) => {
  // Build a response
  let response = new Response("Hello JS", {
    status: 200,
  });

  // Send our response back to the client
  event.respondWith(response);
});
