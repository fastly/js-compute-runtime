/// <reference types="@fastly/js-compute" />

class MyRequest extends Request {
  constructor(input: RequestInfo, init?: RequestInit) {
    super(input, init);
  }
  bar() {
    return "bar";
  }
}

addEventListener("fetch", (event) => {
  const request = new MyRequest("https://www.google.com/");
  if (Reflect.getPrototypeOf(request) !== MyRequest.prototype) {
    throw new Error(
      "Expected `Reflect.getPrototypeOf(request) === MyRequest.prototype` to be `true`, instead found: `false`"
    );
  }
  if (request instanceof MyRequest !== true) {
    throw new Error(
      "Expected `request instanceof MyRequest` to be `true`, instead found: `false`"
    );
  }
  if (Reflect.has(request, "bar") !== true) {
    throw new Error(
      'Expected `Reflect.has(request, "bar")` to be `true`, instead found: `false`'
    );
  }
  if (typeof request.bar !== "function") {
    throw new Error(
      "Expected `typeof request.bar` to be `function`, instead found: `" +
        typeof request.bar +
        "`"
    );
  }

  let response = new Response("");
  event.respondWith(response);
});
