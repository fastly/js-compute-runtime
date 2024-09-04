/* eslint-env serviceworker */
import { pass, assert, assertThrows } from "./assertions.js";
import { routes } from "./routes.js";

let error;
routes.set("/response/json", async () => {
  const APPLICATION_JSON = "application/json";
  const FOO_BAR = "foo/bar";

  const INIT_TESTS = [
    [undefined, 200, "", APPLICATION_JSON, {}],
    [{ status: 400 }, 400, "", APPLICATION_JSON, {}],
    [{ statusText: "foo" }, 200, "foo", APPLICATION_JSON, {}],
    [{ headers: {} }, 200, "", APPLICATION_JSON, {}],
    [{ headers: { "content-type": FOO_BAR } }, 200, "", FOO_BAR, {}],
    [
      { headers: { "x-foo": "bar" } },
      200,
      "",
      APPLICATION_JSON,
      { "x-foo": "bar" },
    ],
  ];

  for (const [
    init,
    expectedStatus,
    expectedStatusText,
    expectedContentType,
    expectedHeaders,
  ] of INIT_TESTS) {
    const response = Response.json("hello world", init);
    error = assert(response.type, "default", "response.type");
    if (error) {
      return error;
    }
    error = assert(response.status, expectedStatus, "response.status");
    if (error) {
      return error;
    }
    error = assert(
      response.statusText,
      expectedStatusText,
      "response.statusText",
    );
    if (error) {
      return error;
    }
    error = assert(
      response.headers.get("content-type"),
      expectedContentType,
      'response.headers.get("content-type")',
    );
    if (error) {
      return error;
    }
    for (const key in expectedHeaders) {
      error = assert(
        response.headers.get(key),
        expectedHeaders[key],
        "response.headers.get(key)",
      );
      if (error) {
        return error;
      }
    }
    const data = await response.json();
    error = assert(data, "hello world", "data");
    if (error) {
      return error;
    }
  }

  const nullBodyStatus = [204, 205, 304];
  for (const status of nullBodyStatus) {
    error = assertThrows(function () {
      Response.json("hello world", { status: status });
    }, TypeError);
    if (error) {
      return error;
    }
  }

  const response = Response.json({ foo: "bar" });
  const data = await response.json();
  error = assert(typeof data, "object", "typeof data");
  if (error) {
    return error;
  }
  error = assert(data.foo, "bar", "data.foo");
  if (error) {
    return error;
  }

  error = assertThrows(function () {
    Response.json(Symbol("foo"));
  }, TypeError);
  if (error) {
    return error;
  }

  const a = { b: 1 };
  a.a = a;
  error = assertThrows(function () {
    Response.json(a);
  }, TypeError);
  if (error) {
    return error;
  }

  class CustomError extends Error {
    name = "CustomError";
  }
  error = assertThrows(function () {
    Response.json({
      get foo() {
        throw new CustomError("bar");
      },
    });
  }, CustomError);
  if (error) {
    return error;
  }
  return pass();
});
