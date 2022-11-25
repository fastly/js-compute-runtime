
declare module "fastly:config-store" {
  /**
    * Class for accessing [Fastly Edge Dictionaries](https://docs.fastly.com/en/guides/about-edge-dictionaries).
    *
    * **Note**: Can only be used when processing requests, not during build-time initialization.
    */
  class ConfigStore {
    /**
     * Creates a new ConfigStore object
     */
    constructor(name: string);
    /**
     * Get a value for a key in the config-store.
     */
    get(key: string): string;
  }

}
