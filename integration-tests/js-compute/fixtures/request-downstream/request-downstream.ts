/// <reference types="@fastly/js-compute" />

addEventListener("fetch", (event) => {
  // Get the request from the client
  let downstreamRequest = event.request;
  let downstreamClientIpString = event.client.address;

  // Check params that we aren't sending back down
  let method = "POST";
  if (downstreamRequest.method != method) {
    throw new Error(
      "downstreamRequest.method() did not return " +
        method +
        ", instead returned: " +
        downstreamRequest.method
    );
  }
  let url = new URL("http://example.org/hello");
  if (downstreamRequest.url != url.href) {
    throw new Error(
      "downstreamRequest.url did not return " +
        url.href +
        ", instead returned: " +
        downstreamRequest.url
    );
  }

  let localhostIp = "127.0.0.1";
  if (downstreamClientIpString != localhostIp) {
    throw new Error(
      "Fastly.getClientIpAddressString() did not return " +
        localhostIp +
        ", instead returned: " +
        downstreamClientIpString
    );
  }

  // Build a response
  // let response = new Response(await downstreamRequest.arrayBuffer(), {
  let response = new Response(downstreamRequest.body, {
    headers: downstreamRequest.headers,
  });

  // Send our response back to the client
  event.respondWith(response);
});
