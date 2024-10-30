declare module 'fastly:secret-store' {
  class SecretStore {
    /**
     * Creates a new SecretStore object, and opens a Secret Store to query
     *
     * @throws {Error} Throws an `Error` if no Secret Store exists with the provided name
     */
    constructor(name: string);
    /**
     * Get a value for a key in the Secret Store. If the provided key does not exist in the Secret Store then this resolves with `null`.
     *
     * @throws {TypeError} Throws an `TypeError` if the provided key is longer than 256 characters.
     */
    get(key: string): Promise<SecretStoreEntry | null>;

    /**
     * Constructs a local in-memory SecretStoreEntry from the provided bytes,
     * useful for passing bytes to APIs that only take a SecretStoreEntry.
     *
     * Note: This is not the recommended way to obtain secrets, instead use {@link get}.
     */
    static fromBytes(bytes: ArrayBufferView | ArrayBuffer): SecretStoreEntry;
  }

  class SecretStoreEntry {
    constructor(name: string);
    /**
     * Get the plaintext value of the SecretStoreEntry as a UTF8 string.
     *
     * Note: Using this method will bring the secret into user memory,
     *       always avoid using this method when possible, instead passing
     *       the secret directly.
     */
    plaintext(): string;
    /**
     * Get the raw byte value of the SecretStoreEntry as a Uint8Array.
     *
     * Note: Using this method will bring the secret into user memory,
     *       always avoid using this method when possible, instead passing
     *       the secret directly.
     */
    rawBytes(): Uint8Array;
  }
}
