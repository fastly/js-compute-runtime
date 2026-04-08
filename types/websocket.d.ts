/// <reference path="globals.d.ts" />

declare module 'fastly:websocket' {
  /**
   * Create a {@link Response} that instructs Fastly to pass the original request through as a
   * WebSocket connection to the specified backend.
   *
   * **Note**: Can only be used when processing requests, not during build-time initialization.
   *
   * @example
   * ```js
   * import { createWebsocketHandoff } from "fastly:websocket";
   *
   * async function handleRequest(event) {
   *     const url = new URL(event.request.url);
   *     if (url.pathname === '/stream') {
   *         return createWebsocketHandoff(event.request, 'websocket-backend');
   *     }
   *     return new Response('Not found', { status: 404 });
   * }
   *
   * addEventListener("fetch", (event) => event.respondWith(handleRequest(event)));
   * ```
   *
   * @param request The request to pass through as a WebSocket connection
   * @param backend The name of the backend to send the request to (1–254 characters)
   * @throws Throws an `Error` if `request` is not a {@link Request} instance, or if `backend` is empty or longer than 254 characters
   * @version 3.34.0
   */
  function createWebsocketHandoff(request: Request, backend: string): Response;
}
