async function handleRequest(event) {
  // Get the client request from the event
  let req = event.request;
  let method = req.method;
  let url = new URL(event.request.url);

  // Default path response

  if (method == "GET" && url.pathname == "/") {
    return new Response("Compute SDK Test Backend");
  }

  // async_select

  if (method == "GET" && url.pathname == "/async_select_1") {
    return new Response("the world of tomorrow!", {
      headers: {
        FooName: "FooValue",
      },
    });
  }
  if (method == "GET" && url.pathname == "/async_select_2") {
    return new Response("the world of tomorrow!", {
      headers: {
        BarName: "BarValue",
      },
    });
  }

  // byte_repeater

  if (method == "GET" && url.pathname == "/byte_repeater") {
    let streamController;
    let stream = new ReadableStream({
      start: (controller) => {
        streamController = controller;
      },
    });

    let response = new Response(stream);

    streamController.enqueue(new TextEncoder().encode("1234"));

    let delayAndStreamResponseTask = async () => {
      let cacheOverride = new CacheOverride("pass", { ttl: 0 });
      let upstreamRequest = new Request("https://httpbin.org/delay/1", {});
      await fetch(upstreamRequest, {
        backend: "httpbin",
        cacheOverride,
      });
      streamController.enqueue(new TextEncoder().encode("56789012\n"));
      streamController.close();
    };
    delayAndStreamResponseTask();
    return response;
  }

  // request_upstream

  if (method == "GET" && url.pathname == "/request_upstream") {
    let headers = new Headers();
    headers.set("OriginHeader", "OriginValue");
    headers.append("x-cat", "meow");
    headers.append("x-cat", "nyan");
    headers.append("x-cat", "mrrow");
    headers.append("x-cat", "miau");

    return new Response("Hello from Origin", {
      headers,
    });
  }

  // streaming_close

  if (method == "GET" && url.pathname == "/streaming_close") {
    let streamController;
    let stream = new ReadableStream({
      start: (controller) => {
        streamController = controller;
      },
    });

    let response = new Response(stream);

    streamController.enqueue(new TextEncoder().encode("will"));

    let delayAndStreamResponseTask = async () => {
      // Make a delay response
      let cacheOverride = new CacheOverride("pass", { ttl: 0 });
      let upstreamRequest = new Request("https://httpbin.org/delay/1", {});
      await fetch(upstreamRequest, {
        backend: "httpbin",
        cacheOverride,
      });
      streamController.enqueue(new TextEncoder().encode(" smith\n"));
      streamController.close();
    };
    delayAndStreamResponseTask();
    return response;
  }

  // logs

  if (method == "GET" && url.pathname == "/logs") {
    console.log("ComputeLog :: Hello!");
    return new Response("Compute SDK Test Backend");
  }

  // tee

  if (method == "POST" && url.pathname == "/tee") {
    const text = await event.request.text();
    return new Response(text);
  }

  // local_upstream_request

  if (method == "GET" && url.pathname == "/local_upstream_request") {
    let delayAndRequestTask = async () => {
      // Make a delay response
      let cacheOverride = new CacheOverride("pass", { ttl: 0 });
      let upstreamRequest = new Request("https://httpbin.org/delay/1", {});
      await fetch(upstreamRequest, {
        backend: "httpbin",
        cacheOverride,
      });

      // Make a request upstream
      await fetch("http://localhost:8081/test", {
        backend: "localserver",
        cacheOverride,
      });
    };

    delayAndRequestTask();
    return new Response("Compute SDK Test Backend");
  }

  // Catch all other requests and return a 404.
  return new Response("The page you requested could not be found", {
    status: 404,
  });
}

addEventListener("fetch", (event) => event.respondWith(handleRequest(event)));
