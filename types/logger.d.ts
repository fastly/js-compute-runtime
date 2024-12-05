declare module 'fastly:logger' {
  /**
   * Class for creating [Fastly Named Loggers](https://developer.fastly.com/learning/integrations/logging/).
   *
   * **Note**: Can only be used when processing requests, not during build-time initialization.
   *
   * @example
   * <script async defer src="https://fiddle.fastly.dev/embed.js"></script>
   * In this example we have a create a logger named 'splunk' and logs the incoming request method and destination.
   *
   * <script type="application/json+fiddle">
   * {
   *   "type": "javascript",
   *   "title": "Logger Example",
   *   "origins": [
   *     "https://http-me.glitch.me"
   *   ],
   *   "src": {
   *     "deps": "{\n  \"@fastly/js-compute\": \"^0.7.0\"\n}",
   *     "main": "/// <reference types=\"@fastly/js-compute\" />\nimport { Logger } from \"fastly:logger\";\n\nasync function app (event) {\n  let logger = new Logger(\"splunk\");\n  logger.log(JSON.stringify({\n    method: event.request.method,\n    url: event.request.url\n  }));\n\n  return new Response('OK');\n}\n\naddEventListener(\"fetch\", event => event.respondWith(app(event)));\n"
   *   },
   *   "requests": [
   *     {
   *       "enableCluster": true,
   *       "enableShield": false,
   *       "enableWAF": false,
   *       "data": {
   *         "dictionaries": {
   *           "animals": {
   *             "cat": "meow"
   *           }
   *         }
   *       },
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
   * import { Logger } from "fastly:logger";
   *
   * async function app (event) {
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
   * </noscript>
   */
  export class Logger {
    /**
     * Creates a new Logger instance for the given [named log endpoint](https://developer.fastly.com/learning/integrations/logging).
     *
     * **Note**: Can only be used when processing requests, not during build-time initialization.
     */
    constructor(name: string);
    /**
     * Send the given message, converted to a string, to this Logger instance's endpoint
     */
    log(message: any): void;
  }

  interface ConsoleLoggingOptions {
    /**
     * Whether to output string prefixes "Log: " | "Debug: " | "Info: " | "Warn: " | "Error: "
     * before messages.
     *
     * Defaults to true.
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
   *
   * Currently only supports customizing prefixing and stdio output.
   *
   * @param loggingOptions The console logging options
   */
  export function configureConsole(loggingOptions: ConsoleLoggingOptions): void;
}
