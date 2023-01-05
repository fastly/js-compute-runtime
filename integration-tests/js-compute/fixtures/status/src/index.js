/* eslint-env serviceworker */
addEventListener("fetch", (event) => {
  // Get the request from the client
  if (event.request.method == "POST" && new URL(event.request.url).pathname == "/hello") {
    // Build a response
    let response = new Response("Unauthorized", {
      status: 401,
    });

    // Send our response back to the client
    event.respondWith(response);
  }
});
