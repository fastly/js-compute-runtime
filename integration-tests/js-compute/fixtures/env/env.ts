/// <reference types="@fastly/js-compute" />

addEventListener("fetch", (event) => {
  // Get the request from the client
  let downstreamRequest = event.request;
  let downstreamUrl = new URL(downstreamRequest.url);

  let responseBody = "";
  let status = 200;

  if (downstreamRequest.method == "GET" && downstreamUrl.pathname == "/") {
    responseBody = fastly.env.get("FASTLY_HOSTNAME");
  } else {
    responseBody = "Bad Request";
    status = 400;
  }

  // Build a response
  let response = new Response(responseBody, {
    status,
  });

  // Send our response back to the client
  event.respondWith(response);
});
