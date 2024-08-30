/* eslint-env serviceworker */

import { routes } from "./routes.js";
import { pass, assert, assertThrows } from "./assertions.js";

routes.set("/response/stall", async (event) => {
  return new Response(
    new ReadableStream({
      start(controller) {
        // stall
      },
    })
  );
});

routes.set("/response/text/guest-backed-stream", async () => {
  let contents = new Array(10).fill(new Uint8Array(500).fill(65));
  contents.push(new Uint8Array([0, 66]));
  contents.push(new Uint8Array([1, 1, 2, 65]));
  let res = new Response(iteratableToStream(contents));
  let text = await res.text();

  let error = assert(
    text,
    "A".repeat(5000) + "\x00B\x01\x01\x02A",
    `await res.text() === "a".repeat(5000)`
  );
  if (error) {
    return error;
  }
  return pass();
});
routes.set("/response/json/guest-backed-stream", async () => {
  let obj = { a: 1, b: 2, c: { d: 3 } };
  let encoder = new TextEncoder();
  let contents = encoder.encode(JSON.stringify(obj));
  let res = new Response(iteratableToStream([contents]));
  let json = await res.json();

  let error = assert(json, obj, `await res.json() === obj`);
  if (error) {
    return error;
  }
  return pass();
});
routes.set("/response/arrayBuffer/guest-backed-stream", async () => {
  let obj = { a: 1, b: 2, c: { d: 3 } };
  let encoder = new TextEncoder();
  let contents = encoder.encode(JSON.stringify(obj));
  let res = new Response(iteratableToStream([contents]));
  let json = await res.arrayBuffer();

  let error = assert(
    json,
    contents.buffer,
    `await res.json() === contents.buffer`
  );
  if (error) {
    return error;
  }
  return pass();
});
routes.set("/response/ip-port-undefined", async () => {
  let res = new Response();
  let error = assert(res.ip, undefined);
  if (error) {
    return error;
  }
  error = assert(res.port, undefined);
  if (error) {
    return error;
  }
  return pass();
});

function iteratableToStream(iterable) {
  return new ReadableStream({
    async pull(controller) {
      for await (const value of iterable) {
        controller.enqueue(value);
      }
      controller.close();
    },
  });
}
