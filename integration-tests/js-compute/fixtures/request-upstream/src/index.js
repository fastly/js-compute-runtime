/* eslint-env serviceworker */
addEventListener("fetch", (event) => {
  let headers = new Headers();
  headers.set("UpstreamHeader", "UpstreamValue");

  let upstreamRequest = new Request(event.request, {
    headers,
  });
  
  // Create a new Request object with an updated URL to our origin
  upstreamRequest = new Request(
    "https://compute-sdk-test-backend.edgecompute.app/request_upstream",
    {
      headers,
    }
  );
  let upstreamResponse = fetch(upstreamRequest, {
    backend: "TheOrigin",
  });

  // Send the upstream/origin response back down
  event.respondWith(upstreamResponse);
});
