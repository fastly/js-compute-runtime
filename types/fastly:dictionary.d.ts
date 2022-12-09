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
   * <a href='https://fiddle.fastly.dev/fiddle/8f5a5052/embedded'>View this example on Fastly Fiddle</a>
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
