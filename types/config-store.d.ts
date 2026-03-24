declare module 'fastly:config-store' {
  /**
   * Class for accessing a [Fastly Config Store](https://www.fastly.com/documentation/reference/api/services/resources/config-store/).
   *
   * **Note**: Can only be used when processing requests, not during build-time initialization.
   *
   * @example
   * ```js
   * import { ConfigStore } from "fastly:config-store";
   *
   * async function app(event) {
   *   const config = new ConfigStore("animals");
   *   return new Response(config.get("cat"));
   * }
   * addEventListener("fetch", (event) => event.respondWith(app(event)));
   * ```
   */
  class ConfigStore {
    /**
     * Creates a new ConfigStore object, providing access to the named
     * [Config Store resource](https://www.fastly.com/documentation/reference/api/services/resources/config-store/).
     *
     * @param name The resource link name for the Config Store.
     * @throws `TypeError` if no Config Store exists with the provided name.
     * @throws `TypeError` if the provided name is empty, longer than 255 characters,
     *   does not start with an ASCII alphabetical character, or contains characters
     *   other than ASCII alphanumerics, underscores, and spaces.
     */
    constructor(name: string);
    /**
     * Get a value for a key in the Config Store. If the provided key does not
     * exist in the Config Store then this returns `null`.
     *
     * @param key The key to retrieve.
     * @throws `TypeError` if the provided key is empty or longer than 255 characters.
     */
    get(key: string): string | null;
  }
}
