/// <reference path="globals.d.ts" />

declare module 'fastly:kv-store' {
  /**
   * Class for accessing a [Fastly KV-store](https://developer.fastly.com/reference/api/kv-store/).
   *
   * A kv store is a persistent, globally consistent key-value store.
   *
   * **Note**: Can only be used when processing requests, not during build-time initialization.
   *
   * @example
   * <script async defer src="https://fiddle.fastly.dev/embed.js"></script>
   * In this example we connect to an KV Store named `'files'` and save an entry to the store under the key `'hello'` and then read back the value and return it to the client.
   *
   * ```js
   * /// <reference types="@fastly/js-compute" />
   *
   * import { KVStore } from "fastly:kv-store";
   *
   * async function app(event) {
   *   const files = new KVStore('files')
   *
   *   await files.put('hello', 'world')
   *
   *   const entry = await files.get('hello')
   *
   *   return new Response(await entry.text())
   * }
   *
   * addEventListener("fetch", (event) => event.respondWith(app(event)))
   *
   * ```
   */
  export class KVStore {
    /**
     * Creates a new JavaScript KVStore object which interacts with the Fastly KV Store named `name`.
     *
     * @param name Name of the Fastly KV Store to interact with. A name cannot be empty, contain Control characters, or be longer than 255 characters.
     */
    constructor(name: string);
    /**
     * Delete the value associated with the key `key` in the KV Store.
     * @param key The key to retrieve from within the KV Store. A key cannot:
     * - Be any of the strings "", ".", or ".."
     * - Start with the string ".well-known/acme-challenge/""
     * - Contain any of the characters "#;?^|\n\r"
     * - Be longer than 1024 characters
     */
    delete(key: string): Promise<undefined>;

    /**
     * Gets the value associated with the key `key` in the KV Store.
     * When the key is present, a resolved Promise containing an KVStoreEntry will be returned which contains the associated value.
     * When the key is absent, a resolved Promise containing null is returned.
     * @param key The key to retrieve from within the KV Store. A key cannot:
     * - Be any of the strings "", ".", or ".."
     * - Start with the string ".well-known/acme-challenge/""
     * - Contain any of the characters "#;?^|\n\r"
     * - Be longer than 1024 characters
     */
    get(key: string): Promise<KVStoreEntry | null>;

    /**
     * Write the value of `value` into the KV Store under the key `key`.
     *
     * Note: KV Store is eventually consistent, this means that the updated contents associated with the key `key` may not be available to read from all
     * edge locations immediately and some edge locations may continue returning the previous contents associated with the key.
     *
     * @param key The key to associate with the value. A key cannot:
     * - Be any of the strings "", ".", or ".."
     * - Start with the string ".well-known/acme-challenge/""
     * - Contain any of the characters "#;?^|\n\r"
     * - Be longer than 1024 characters
     * @param value The value to store within the KV Store.
     */
    put(
      key: string,
      value: BodyInit,
      options?: {
        /**
         * Optional metadata to be associated with the entry.
         *
         * If passing a string, UTF-8 encoding is used
         */
        metadata?: ArrayBufferView | ArrayBuffer | string;
        /**
         * TTL for the entry.
         */
        ttl?: number;
        /**
         * Insert mode, defaults to 'overwrite'.
         */
        mode?: 'overwrite' | 'add' | 'append' | 'prepend';
        /**
         * If generation match integer.
         */
        gen?: number;
      },
    ): Promise<undefined>;

    /**
     * Returns an async iterator for the values of the KV Store
     * optionally taking a prefix and limit
     */
    list(options?: {
      /**
       * Do not wait to sync the key list, and instead immediately return the current cached key list.
       */
      noSync?: boolean;
      /**
       * String prefix for keys to list.
       */
      prefix?: string;
      /**
       * Limit the number of keys provided per listing.
       */
      limit?: number;
      /**
       * Cursor
       *
       * The base64 cursor string representing the last listing operation
       */
      cursor?: string;
    }): {
      list: string[];
      /**
       * Pass this base64 cursor into a subsequent list call to obtain the next listing.
       *
       * The cursor is *undefined* when the end of the list is reached.
       */
      cursor: string | undefined;
    };
  }

  /**
   * Class for interacting with a [Fastly KV Store](https://developer.fastly.com/reference/api/kv-store/) entry.
   */
  interface KVStoreEntry {
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

    /**
     * Metadata associatd with this entry
     */
    metadata(): ArrayBuffer | null;

    /**
     * Metadata string associated with this entry
     * Throws an error for invalid UTF-8
     */
    metadataText(): string | null;
  }
}
