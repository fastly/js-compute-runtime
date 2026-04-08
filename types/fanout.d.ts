/// <reference path="globals.d.ts" />

declare module 'fastly:fanout' {
  /**
   * Creates a {@link Response} that instructs Fastly to pass the original
   * request through [Fanout](https://www.fastly.com/documentation/guides/concepts/real-time-messaging/fanout/)
   * to the declared backend.
   *
   * **Note**: Can only be used when processing requests, not during build-time
   * initialization.
   *
   * @example
   * ```js
   * import { createFanoutHandoff } from "fastly:fanout";
   *
   * async function handleRequest(event) {
   *   const url = new URL(event.request.url);
   *   if (url.pathname === "/stream") {
   *     return createFanoutHandoff(event.request, "my-backend");
   *   }
   *   return new Response("Not found", { status: 404 });
   * }
   *
   * addEventListener("fetch", (event) => event.respondWith(handleRequest(event)));
   * ```
   *
   * @param request The request to pass through Fanout.
   * @param backend The name of the backend that Fanout should send the request
   * to. The name must be between 1 and 254 characters inclusive.
   * @returns A {@link Response} that can be passed to `event.respondWith`.
   * @throws `Error` if `request` is not a {@link Request} instance, or
   * if `backend` is an empty string or longer than 254 characters.
   */
  function createFanoutHandoff(request: Request, backend: string): Response;
}
