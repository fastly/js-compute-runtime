
declare module "fastly:config-store" {
  /**
   * Class for accessing [Fastly Edge Dictionaries](https://docs.fastly.com/en/guides/about-edge-dictionaries).
   *
   * **Note**: Can only be used when processing requests, not during build-time initialization.
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
