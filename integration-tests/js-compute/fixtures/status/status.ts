/// <reference types="@fastly/js-compute" />

addEventListener("fetch", (event) => {
  // Build a response
  let response = new Response("Unauthorized", {
    status: 401,
  });

  // Send our response back to the client
  event.respondWith(response);
});
