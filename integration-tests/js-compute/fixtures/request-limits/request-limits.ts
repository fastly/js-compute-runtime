/// <reference types="@fastly/js-compute" />

addEventListener("fetch", (event) => {
  // Get the request from the client
  let downstreamRequest = event.request;

  // Get the number of requests we should make
  let requestsQueryParam = downstreamRequest.url.split("?requests=")[1];
  let numRequestsToMake = parseInt(requestsQueryParam, 10);

  let pendingRequests = [];

  // Loop and add the maximum limit of requests
  for (let i = 0; i < numRequestsToMake; i++) {
    let request = new Request("https://compute-sdk-test-backend.edgecompute.app/", {
      method: "GET"
    });
    let pendingRequest = fetch(request, {
      backend: "TheOrigin",
    });

    pendingRequests.push(pendingRequest);
  }

  // Build a response
  let response = Promise.all(pendingRequests).then(
    (responses) => new Response(responses.length.toString())
  );

  // Send the upstream/origin response back down
  event.respondWith(response);
});
