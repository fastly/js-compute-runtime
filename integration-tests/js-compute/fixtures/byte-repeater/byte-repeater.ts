/// <reference types="@fastly/js-compute" />

addEventListener("fetch", (event) => {
  let headers = new Headers();
  headers.set("AssemblyScriptHeader", "AssemblyScriptValue");

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
  let upstreamRequest = new Request("http://provider.org/TheURL");
  upstreamRequest.setCacheOverride(new CacheOverride("pass"));
  fetch(upstreamRequest, {
    backend: "TheOrigin",
  }).then(async (upstreamResponse) => {
    let body = upstreamResponse.body;
    let streamReader = body.getReader();

    while (true) {
      let chunk = await streamReader.read();

      // Check if we are done
      if (chunk.done) {
        break;
      }

      if (chunk.value.byteLength == 0) {
        continue;
      }

      // Otherwise get the byte and repeat them
      let upstreamBytes: Uint8Array = chunk.value;
      let downstreamBytes = new Uint8Array(upstreamBytes.length * 2);
      for (let i = 0; i < upstreamBytes.length; i++) {
        let downstreamIndex = i * 2;
        downstreamBytes[downstreamIndex] = upstreamBytes[i];
        downstreamBytes[downstreamIndex + 1] = upstreamBytes[i];
      }

      streamController.enqueue(downstreamBytes);
    }
    streamController.close();
  });
});
