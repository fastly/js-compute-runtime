declare module 'fastly:secret-store' {
  /**
   * Class for accessing a [Fastly secret store](https://developer.fastly.com/reference/api/services/resources/secret-store/).
   *
   * A secret store is a persistent, globally distributed store for secrets accessible to
   * Fastly Compute services during request processing.
   *
   * **Note**: Can only be used when processing requests, not during build-time initialization.
   *
   * @example
   * In this example we connect to a secret store named `'secrets'` and retrieve a secret
   * named `'cat-api-key'` to use in a request header.
   *
   * ```js
   * /// <reference types="@fastly/js-compute" />
   *
   * import { SecretStore } from "fastly:secret-store";
   *
   * async function app(event) {
   *   const secrets = new SecretStore('secrets')
   *
   *   const catApiKey = await secrets.get('cat-api-key')
   *
   *   return fetch('/api/cat', {
   *     headers: {
   *       'cat-api-key': catApiKey.plaintext()
   *     }
   *   })
   * }
   *
   * addEventListener("fetch", (event) => event.respondWith(app(event)))
   * ```
   */
  class SecretStore {
    /**
     * Creates a new `SecretStore` object which interacts with the Fastly secret store named `name`.
     *
     * @param name Name of the Fastly secret store to interact with. A name cannot be empty,
     *   longer than 255 characters, or contain characters other than letters, numbers, dashes
     *   (`-`), underscores (`_`), and periods (`.`).
     * @throws Throws `TypeError` if no secret store exists with the provided name, the name is empty,
     *   longer than 255 characters, or contains invalid characters.
     */
    constructor(name: string);
    /**
     * Get the value associated with the key `key` in the secret store. If the key does not
     * exist, the returned `Promise` resolves with `null`.
     *
     * @param key The key to retrieve. A key cannot be empty, longer than 255 characters, or
     *   contain characters other than letters, numbers, dashes (`-`), underscores (`_`), and
     *   periods (`.`).
     * @throws Throws `TypeError` if the key is empty, longer than 255 characters, or contains
     *   invalid characters.
     */
    get(key: string): Promise<SecretStoreEntry | null>;

    /**
     * Construct a local in-memory `SecretStoreEntry` from the provided bytes, useful for
     * passing bytes to APIs that only accept a `SecretStoreEntry`.
     *
     * **Note**: This is not the recommended way to obtain secrets — use {@link get} instead.
     *
     * @param bytes The raw bytes to wrap as a `SecretStoreEntry`.
     * @version 3.15.0
     */
    static fromBytes(bytes: ArrayBufferView | ArrayBuffer): SecretStoreEntry;
  }

  class SecretStoreEntry {
    /**
     * Get the plaintext value of the secret as a UTF-8 string.
     *
     * **Note**: Using this method will bring the secret into user memory — avoid using it
     * when possible, instead passing the `SecretStoreEntry` directly.
     */
    plaintext(): string;
    /**
     * Get the raw byte value of the secret as a Uint8Array.
     *
     * **Note**: Using this method will bring the secret into user memory — avoid using it
     * when possible, instead passing the `SecretStoreEntry` directly.
     *
     * @version 3.15.0
     */
    rawBytes(): Uint8Array<ArrayBuffer>;
  }
}
