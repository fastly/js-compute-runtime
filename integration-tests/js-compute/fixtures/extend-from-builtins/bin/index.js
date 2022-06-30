const builtins = [
  TransformStream,
  CompressionStream,
  Request,
  Response,
  Dictionary,
  Headers,
  CacheOverride,
  TextEncoder,
  TextDecoder,
  URL,
  URLSearchParams,
  ObjectStore,
];

addEventListener("fetch", (event) => {
  for (const builtin of builtins) {
    class customClass extends builtin {
      constructor() {
        switch (builtin.name) {
          case "CacheOverride": {
            super("none");
            break;
          }
          case "CompressionStream": {
            super("gzip");
            break;
          }
          case "Dictionary":
          case "ObjectStore": {
            super("example");
            break;
          }
          case "Request":
          case "URL": {
            super("http://example.com");
            break;
          }
          default: {
            super();
          }
        }
      }
      shrimp() {
        return "shrimp";
      }
    }
    const instance = new customClass();
    if (Reflect.getPrototypeOf(instance) !== customClass.prototype) {
      throw new Error(
        "Extending from `" +
          builtin.name +
          "`: Expected `Reflect.getPrototypeOf(instance) === customClass.prototype` to be `true`, instead found: `false`"
      );
    }
    if (instance instanceof customClass !== true) {
      throw new Error(
        "Extending from `" +
          builtin.name +
          "`: Expected `instance instanceof customClass` to be `true`, instead found: `false`"
      );
    }
    if (instance.name === customClass.name) {
      throw new Error(
        "Extending from `" +
          builtin.name +
          "`: Expected `instance.name === customClass.name` to be `true`, instead found: `false`"
      );
    }
    if (Reflect.has(instance, "shrimp") !== true) {
      throw new Error(
        "Extending from `" +
          builtin.name +
          '`: Expected `Reflect.has(instance, "shrimp")` to be `true`, instead found: `false`'
      );
    }
    if (typeof instance.shrimp !== "function") {
      throw new Error(
        "Extending from `" +
          builtin.name +
          "`: Expected `typeof instance.shrimp` to be `function`, instead found: `" +
          typeof instance.shrimp +
          "`"
      );
    }

    console.log(`Tests for extending from ${builtin.name} all passed`);
  }
  event.respondWith(new Response());
});
