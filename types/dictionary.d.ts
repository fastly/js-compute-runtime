declare module 'fastly:dictionary' {
  /**
   * Class for accessing [Fastly Edge Dictionaries](https://docs.fastly.com/en/guides/about-edge-dictionaries).
   *
   * **Note**: Can only be used when processing requests, not during build-time initialization.
   *
   * @deprecated Use {@link config-store!ConfigStore | ConfigStore} from `'fastly:config-store'` instead.
   */
  class Dictionary {
    /**
     * Creates a new Dictionary object, providing access to the named
     * [Edge Dictionary](https://docs.fastly.com/en/guides/about-edge-dictionaries).
     *
     * @param name The name of the Edge Dictionary.
     * @throws `TypeError` if no Dictionary exists with the provided name.
     * @throws `TypeError` if the provided name is empty, longer than 255 characters,
     *   does not start with an ASCII alphabetical character, or contains characters
     *   other than ASCII alphanumerics, underscores, and spaces.
     * @deprecated Use {@link config-store!ConfigStore | ConfigStore} from `'fastly:config-store'` instead.
     */
    constructor(name: string);
    /**
     * Get a value for a key in the Dictionary. If the provided key does not
     * exist in the Dictionary then this returns `null`.
     *
     * @param key The key to retrieve.
     * @throws `TypeError` if the provided key is empty or longer than 255 characters.
     * @deprecated Use {@link config-store!ConfigStore | ConfigStore} from `'fastly:config-store'` instead.
     */
    get(key: string): string | null;
  }
}
