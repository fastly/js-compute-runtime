declare module "fastly:dictionary" {
  /**
   * Class for accessing [Fastly Edge Dictionaries](https://docs.fastly.com/en/guides/about-edge-dictionaries).
   *
   * **Note**: Can only be used when processing requests, not during build-time initialization.
   */
  class Dictionary {
    /**
     * Creates a new Dictionary object, and opens an edge dictionary to query
     */
    constructor(name: string);
    /**
     * Get a value for a key in the dictionary
     */
    get(key: string): string;
  }
}
