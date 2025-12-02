declare module 'fastly:env' {
  /**
   * Function to get the value for the provided environment variable name.
   *
   * For a list of available environment variables, see the [Fastly Developer Hub for Compute Environment Variables](https://developer.fastly.com/reference/compute/ecp-env/)
   *
   * **Note**: The environment variables can only be retrieved when processing requests, not during build-time initialization.
   *
   * @param name The name of the environment variable
   *
   * @example
   * <script async defer src="https://fiddle.fastly.dev/embed.js"></script>
   * In this example we log to stdout the environment variables `FASTLY_HOSTNAME` and `FASTLY_TRACE_ID`.
   *
   * <script type="application/json+fiddle">
   * {
   *   "type": "javascript",
   *   "title": "Environment Variable Example",
   *   "origins": [
   *     "https://http-me.fastly.dev"
   *   ],
   *   "src": {
   *     "deps": "{\n  \"@fastly/js-compute\": \"^0.7.0\"\n}",
   *     "main": "/// <reference types=\"@fastly/js-compute\" />\nimport { env } from \"fastly:env\";\n\nfunction app(event) {\n  console.log(\"FASTLY_HOSTNAME:\", env(\"FASTLY_HOSTNAME\"));\n  console.log(\"FASTLY_TRACE_ID:\", env(\"FASTLY_TRACE_ID\"));\n\n  return new Response(\"\", {\n    status: 200\n  });\n}\n\naddEventListener(\"fetch\", event => event.respondWith(app(event)));\n"
   *   },
   *   "requests": [
   *     {
   *       "enableCluster": true,
   *       "enableShield": false,
   *       "enableWAF": false,
   *       "method": "GET",
   *       "path": "/status=200",
   *       "useFreshCache": false,
   *       "followRedirects": false,
   *       "tests": "",
   *       "delay": 0
   *     }
   *   ],
   *   "srcVersion": 26
   * }
   * </script>
   * <noscript>
   * ```js
   * /// <reference types="@fastly/js-compute" />
   * import { env } from "fastly:env";
   *
   * function app(event) {
   *   console.log("FASTLY_HOSTNAME:", env("FASTLY_HOSTNAME"));
   *   console.log("FASTLY_TRACE_ID:", env("FASTLY_TRACE_ID"));
   *
   *   return new Response("", {
   *     status: 200
   *   });
   * }
   *
   * addEventListener("fetch", event => event.respondWith(app(event)));
   * ```
   * </noscript>
   */
  function env(name: string): string;
}
