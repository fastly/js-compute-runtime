/// <reference types="@fastly/js-compute" />

addEventListener("fetch", (event) => {
  // Make a Request upstream to our origin
  let headers = new Headers();
  headers.set("UpstreamHeader", "UpstreamValue");
  let upstreamRequest = new Request("https://compute-sdk-test-backend.edgecompute.app/request_upstream", {
    headers,
  });
  let upstreamResponse = fetch(upstreamRequest, {
    backend: "TheOrigin",
  });

  // Send the upstream/origin response back down
  event.respondWith(upstreamResponse);
});
