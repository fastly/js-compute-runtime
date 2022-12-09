declare module "fastly:env" {
  /**
   * Function to get the value for the provided environment variable name.
   *
   * For a list of available environment variables, see the [Fastly Developer Hub for C@E Environment Variables](https://developer.fastly.com/reference/compute/ecp-env/)
   * 
   * **Note**: The environment variables can only be retrieved when processing requests, not during build-time initialization.
   *
   * @param name The name of the environment variable
   * 
   * @example
   * <script async defer src="https://fiddle.fastly.dev/embed.js"></script>
   * In this example we log to stdout the environment variables `FASTLY_HOSTNAME` and `FASTLY_TRACE_ID`.
   * 
   * <a href='https://fiddle.fastly.dev/fiddle/276f3cdd/embedded'>View this example on Fastly Fiddle</a>
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
