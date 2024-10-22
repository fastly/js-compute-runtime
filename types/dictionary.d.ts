declare module 'fastly:dictionary' {
  /**
   * Class for accessing [Fastly Edge Dictionaries](https://docs.fastly.com/en/guides/about-edge-dictionaries).
   *
   * **Note**: Can only be used when processing requests, not during build-time initialization.
   *
   * @deprecated This Class is deprecated, it has been renamed to ConfigStore and can be imported via `import { ConfigStore } from 'fastly:config-store'`
   */
  class Dictionary {
    /**
     * Creates a new Dictionary object, and opens an edge dictionary to query
     *
     * @throws {Error} Throws an `Error` if no Dictionary exists with the provided name
     *
     * @deprecated This constructor is deprecated, it has been renamed to ConfigStore and can be imported via `import { ConfigStore } from 'fastly:config-store'`
     */
    constructor(name: string);
    /**
     * Get a value for a key in the dictionary. If the provided key does not exist in the Dictionary then this returns `null`.
     *
     * @throws {TypeError} Throws an `TypeError` if the provided key is longer than 256 characters.
     *
     * @deprecated This Class is deprecated, the Class has been renamed to ConfigStore and can be imported via `import { ConfigStore } from 'fastly:config-store'`
     */
    get(key: string): string | null;
  }
}
