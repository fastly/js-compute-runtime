
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
    * <a href='https://fiddle.fastly.dev/fiddle/045e1ffe/embedded'>View this example on Fastly Fiddle</a>
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
