declare module "fastly:dictionary" {
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
   *   "title": "Dictionary Example",
   *   "type": "javascript",
   *   "origins": [
   *     "https://http-me.glitch.me"
   *   ],
   *   "src": {
   *     "deps": "{\n  \"@fastly/js-compute\": \"^0.7.0\"\n}",
   *     "main": "/// <reference types=\"@fastly/js-compute\" />\nimport { Dictionary } from \"fastly:dictionary\";\n\nasync function app (event) {\n  const animals = new Dictionary('animals');\n  return new Response(animals.get('cat'));\n}\n\naddEventListener(\"fetch\", event => event.respondWith(app(event)));\n"
   *   },
   *   "srcVersion": 7,
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
   *      }
   *   ]
   * }
   * </script>
   * <noscript>
   * ```js
   * /// <reference types="@fastly/js-compute" />
   * import { Dictionary } from "fastly:dictionary";
   * 
   * async function app (event) {
   *   const animals = new Dictionary('animals');
   *   return new Response(animals.get('cat'));
   * }
   * 
   * addEventListener("fetch", event => event.respondWith(app(event)));
   * ```
   * </noscript>
   */
  class Dictionary {
    /**
     * Creates a new Dictionary object, and opens an edge dictionary to query
     * 
     * @throws {Error} Throws an `Error` if no Dictionary exists with the provided name
     */
    constructor(name: string);
    /**
     * Get a value for a key in the dictionary. If the provided key does not exist in the Dictionary then this returns `null`.
     * 
     * @throws {TypeError} Throws an `TypeError` if the provided key is longer than 256 characters.
     */
    get(key: string): string | null;
  }
}
