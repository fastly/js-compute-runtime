declare module 'fastly:logger' {
  /**
   * Class for connecting to [Fastly named log endpoints](https://developer.fastly.com/learning/integrations/logging/).
   *
   * **Note**: Can only be used when processing requests, not during build-time initialization.
   *
   * @example
   * In this example we create a logger named `"splunk"` and log the incoming request method
   * and destination.
   *
   * ```js
   * /// <reference types="@fastly/js-compute" />
   * import { Logger } from "fastly:logger";
   *
   * async function app(event) {
   *   let logger = new Logger("splunk");
   *   logger.log(JSON.stringify({
   *     method: event.request.method,
   *     url: event.request.url
   *   }));
   *
   *   return new Response('OK');
   * }
   *
   * addEventListener("fetch", event => event.respondWith(app(event)));
   * ```
   */
  export class Logger {
    /**
     * Creates a new Logger instance for the given
     * [named log endpoint](https://developer.fastly.com/learning/integrations/logging).
     *
     * @param name The name of the Fastly log endpoint to associate with this Logger instance.
     */
    constructor(name: string);
    /**
     * Send the given message, converted to a string, to this Logger instance's endpoint.
     */
    log(message: any): void;
  }

  interface ConsoleLoggingOptions {
    /**
     * Whether to output string prefixes "Log: " | "Debug: " | "Info: " | "Warn: " | "Error: "
     * before messages.
     *
     * Defaults to false.
     */
    prefixing?: boolean;
    /**
     * Whether to use stderr for `console.warn` and `console.error` messages.
     *
     * Defaults to false.
     */
    stderr?: boolean;
  }

  /**
   * Configure the behaviour of `console.log` and related console logging functions.
   * @version 3.28.0
   */
  export function configureConsole(loggingOptions: ConsoleLoggingOptions): void;
}
