declare module "fastly:object-store" {
  /**
   * Class for accessing a [Fastly Object-store](https://developer.fastly.com/reference/api/object-store/).
   *
   * An object store is a persistent, globally consistent key-value store.
   *
   * **Note**: Can only be used when processing requests, not during build-time initialization.
   */
  export class ObjectStore {
    /**
     * Creates a new JavaScript ObjectStore object which interacts with the Fastly Object-store named `name`.
     *
     * @param name Name of the Fastly Object-store to interact with. A name cannot be empty, contain Control characters, or be longer than 255 characters.
     */
    constructor(name: string);
    /**
     * Gets the value associated with the key `key` in the Object-store.
     * When the key is present, a resolved Promise containing an ObjectStoreEntry will be returned which contains the associated value.
     * When the key is absent, a resolved Promise containing null is returned.
     * @param key The key to retrieve from within the Object-store. A key cannot:
     * - Be any of the strings "", ".", or ".."
     * - Start with the string ".well-known/acme-challenge/""
     * - Contain any of the characters "#?*[]\n\r"
     * - Be longer than 1024 characters
     */
    get(key: string): Promise<ObjectStoreEntry | null>;

    /**
     * Write the value of `value` into the Object-store under the key `key`.
     *
     * Note: Object-store is eventually consistent, this means that the updated contents associated with the key `key` may not be available to read from all
     * edge locations immediately and some edge locations may continue returning the previous contents associated with the key.
     *
     * @param key The key to associate with the value. A key cannot:
     * - Be any of the strings "", ".", or ".."
     * - Start with the string ".well-known/acme-challenge/""
     * - Contain any of the characters "#?*[]\n\r"
     * - Be longer than 1024 characters
     * @param value The value to store within the Object-store.
     */
    put(key: string, value: BodyInit): Promise<undefined>;
  }

  /**
   * Class for interacting with a [Fastly Object-store](https://developer.fastly.com/reference/api/object-store/) entry.
   */
  interface ObjectStoreEntry {
    /**
     * A ReadableStream with the contents of the entry.
     */
    get body(): ReadableStream;
    /**
     * A boolean value that indicates whether the body has been read from already.
     */
    get bodyUsed(): boolean;
    /**
     * Reads the body and returns it as a promise that resolves with a string.
     * The response is always decoded using UTF-8.
     */
    text(): Promise<string>;
    /**
     * Reads the body and returns it as a promise that resolves with the result of parsing the body text as JSON.
     */
    json(): Promise<object>;
    /**
     * Reads the body and returns it as a promise that resolves with an ArrayBuffer.
     */
    arrayBuffer(): Promise<ArrayBuffer>;
  }
}
