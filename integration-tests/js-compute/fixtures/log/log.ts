/// <reference types="@fastly/js-compute" />

addEventListener("fetch", (event) => {
  let logger = fastly.getLogger("JavaScriptLog");
  logger.log("Hello!");

  // Build a response
  let response = new Response();

  // Send our response back to the client
  event.respondWith(response);
});
