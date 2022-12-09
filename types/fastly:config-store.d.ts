
declare module "fastly:config-store" {
  /**
   * Class for accessing [Fastly Edge Dictionaries](https://docs.fastly.com/en/guides/about-edge-dictionaries).
   *
   * **Note**: Can only be used when processing requests, not during build-time initialization.
   * 
   * @example
   * <script async defer src="https://fiddle.fastly.dev/embed.js"></script>
   * In this example we have an Edge Dictionary named 'animals' and we return the 'cat'
   * entry as the response body to the client.
   * 
   * <script type="application/json+fiddle">
   * {
   *   "type": "javascript",
   *   "title": "ConfigStore Example",
   *   "origins": [
   *     "https://http-me.glitch.me"
   *   ],
   *   "src": {
   *     "deps": "{\n  \"@fastly/js-compute\": \"^0.5.15\"\n}",
   *     "main": "/// <reference types=\"@fastly/js-compute\" />\nimport { ConfigStore } from \"fastly:config-store\";\n\nasync function app (event) {\n  const config = new ConfigStore('animals');\n  return new Response(config.get('cat'));\n}\n\naddEventListener(\"fetch\", event => event.respondWith(app(event)));\n"
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
   * import { ConfigStore } from "fastly:config-store";
   *
   * async function app (event) {
   *   const config = new ConfigStore('animals');
   *   return new Response(config.get('cat'));
   * }
   * addEventListener("fetch", event => event.respondWith(app(event)));
   * ```
   * </noscript>
   */
  class ConfigStore {
    /**
     * Creates a new ConfigStore object
     * 
     * @throws {Error} Throws an `Error` if no Config Store exists with the provided name
     */
    constructor(name: string);
    /**
     * Get a value for a key in the config-store. If the provided key does not exist in the Config Store then this returns `null`.
     * 
     * @throws {TypeError} Throws an `TypeError` if the provided key is longer than 256 characters.
     */
    get(key: string): string | null;
  }

}
