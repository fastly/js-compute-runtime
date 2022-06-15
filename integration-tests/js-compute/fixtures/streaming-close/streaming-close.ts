/// <reference types="@fastly/js-compute" />

addEventListener("fetch", (event) => {
  // Build a response
  let headers = new Headers();
  headers.set("JSHeader", "JSValue");

  let streamController!: ReadableStreamDefaultController;
  let stream = new ReadableStream({
    start: (controller: ReadableStreamDefaultController) => {
      streamController = controller;
    },
  });
  let response = new Response(stream, {
    headers,
  });

  // Send downstream (back to the client)
  event.respondWith(response);

  // Make a request upstream
  let upstreamRequest = new Request("https://compute-sdk-test-backend.edgecompute.app/streaming_close");
  let upstreamCacheOverride = new CacheOverride("pass");
  upstreamRequest.setCacheOverride(upstreamCacheOverride);
  fetch(upstreamRequest, {
    backend: "TheOrigin",
  }).then(async (upstreamResponse) => {
    // Get our response, and make our stream
    let readableStream = upstreamResponse.body;
    let streamReader = readableStream.getReader();

    // Get our array of utf8 vowels
    let vowelsTypedArray = new TextEncoder().encode("aeiou");
    let vowelsRemoved = 0;

    let isDone = false;
    while (!isDone) {
      let response = await streamReader.read();

      // Check if we are done
      if (response.done) {
        isDone = true;
        continue;
      }

      if (response.value.byteLength == 0) {
        continue;
      }

      // Otherwise get the byte and if it is a vowel skip it
      let upstreamBytes = response.value;
      let downstreamBytesArray = [];
      for (let i = 0; i < upstreamBytes.length; i++) {
        let byte = upstreamBytes[i];
        if (vowelsTypedArray.includes(byte)) {
          vowelsRemoved++;
        } else {
          downstreamBytesArray.push(byte);
        }
      }

      // Create a downstream buffer from our array
      let downstreamBytesTypedArray = new Uint8Array(downstreamBytesArray);
      for (let i = 0; i < downstreamBytesArray.length; i++) {
        downstreamBytesTypedArray[i] = downstreamBytesArray[i];
      }

      streamController.enqueue(downstreamBytesTypedArray);
    }

    // Close the stream
    streamController.close();

    // Send the number of vowels removed to the telemetry server
    let teleHeaders = new Headers();
    teleHeaders.set("Vowels-Removed", vowelsRemoved.toString());
    let teleRequest = new Request("https://telemetry-server.com/example", {
      method: "POST",
      headers: teleHeaders
    });
    await fetch(teleRequest, {
      backend: "TelemetryServer",
    });
  });
});
