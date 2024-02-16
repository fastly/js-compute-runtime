declare module "fastly:secret-store" {
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
  }
  
  class SecretStoreEntry {
    constructor(name: string);
    /**
     * Get the plaintext value of the SecretStoreEntry.
     */
    plaintext(): string;
  }
}
