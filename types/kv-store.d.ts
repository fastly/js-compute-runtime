/// <reference path="globals.d.ts" />

declare module 'fastly:kv-store' {
  /**
   * Class for accessing a [Fastly KV store](https://developer.fastly.com/reference/api/kv-store/).
   *
   * A KV store is a persistent, globally consistent key-value store. See
   * [Data stores for Fastly services](https://developer.fastly.com/learning/concepts/edge-state/data-stores#kv-stores)
   * for initialization and usage details.
   *
   * **Note**: Can only be used when processing requests, not during build-time initialization.
   *
   * @example
   * In this example we connect to a KV store named `'files'`, save an entry to the store
   * under the key `'hello'` and then read back the value and return it to the client.
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
     * Creates a new JavaScript KVStore object which interacts with the Fastly KV store named `name`.
     *
     * @param name Name of the Fastly KV store to interact with. A name cannot be empty, contain
     *   control characters, or be longer than 255 characters.
     * @throws `TypeError` if no KV store exists with the provided name, or the name is empty,
     *   longer than 255 characters, does not start with an ASCII alphabetical character,
     *   or contains control characters (`\u0000-\u001F`).
     */
    constructor(name: string);
    /**
     * Delete the value associated with the key `key` in the KV store.
     *
     * @param key The key to delete from within the KV store. A key cannot:
     * - Be any of the strings "", ".", or ".."
     * - Start with the string ".well-known/acme-challenge/"
     * - Contain any of the characters "#;?^|\n\r"
     * - Be longer than 1024 characters
     * @throws `TypeError` if the key violates any of the above constraints.
     * @version 3.13.0
     */
    delete(key: string): Promise<undefined>;

    /**
     * Gets the value associated with the key `key` in the KV store.
     *
     * When the key is present, a resolved `Promise` containing a `KVStoreEntry` will be returned
     * which contains the associated value. When the key is absent, a resolved `Promise`
     * containing `null` is returned.
     *
     * @param key The key to retrieve from within the KV store. A key cannot:
     * - Be any of the strings "", ".", or ".."
     * - Start with the string ".well-known/acme-challenge/"
     * - Contain any of the characters "#;?^|\n\r"
     * - Be longer than 1024 characters
     * @throws Throws `TypeError` if the key violates any of the above constraints.
     */
    get(key: string): Promise<KVStoreEntry | null>;

    /**
     * Write the value of `value` into the KV store under the key `key`.
     *
     * Note: KV store is eventually consistent, this means that the updated contents associated
     * with the key `key` may not be available to read from all edge locations immediately and
     * some edge locations may continue returning the previous contents associated with the key.
     *
     * @param key The key to associate with the value in the KV store. A key cannot:
     * - Be any of the strings "", ".", or ".."
     * - Start with the string ".well-known/acme-challenge/"
     * - Contain any of the characters "#;?^|\n\r"
     * - Be longer than 1024 characters
     * @param value The value to store within the KV store. Maximum size is 30 MB.
     * @throws `TypeError` if the key violates any of the above constraints or 
     * if `gen` is provided and is not a positive integer.
     */
    put(
      key: string,
      value: BodyInit,
      options?: {
        /**
         * Binary metadata to associate with the entry, up to 1000 bytes.
         *
         * If passing a string, UTF-8 encoding is used.
         *
         * @version 3.26.0
         */
        metadata?: ArrayBufferView | ArrayBuffer | string;
        /**
         * TTL (time-to-live) for the entry, in seconds.
         *
         * @version 3.26.0
         */
        ttl?: number;
        /**
         * Insert mode, defaults to `'overwrite'`.
         * - `'overwrite'`: Replace any existing value for the key.
         * - `'add'`: Only insert if the key does not already exist.
         * - `'append'`: Append to any existing value for the key.
         * - `'prepend'`: Prepend to any existing value for the key.
         *
         * @version 3.26.0
         */
        mode?: 'overwrite' | 'add' | 'append' | 'prepend';
        /**
         * Generation counter for conditional writes. The write only succeeds if the
         * current generation of the entry matches this value. Must be a positive integer.
         *
         * @version 3.31.0
         */
        gen?: number;
      },
    ): Promise<undefined>;

    /**
     * List keys in the KV store, optionally filtered by prefix.
     *
     * @param options Options for filtering and paginating the key list.
     * @returns A `Promise` resolving with the list of keys and a cursor for pagination.
     * @version 3.26.0
     */
    list(options?: {
      /**
       * Do not wait to sync the key list, and instead immediately return the current
       * cached key list. May be faster but possibly out of date.
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
       * The base64 cursor string representing the last listing operation.
       */
      cursor?: string;
    }): Promise<{
      list: string[];
      /**
       * Pass this base64 cursor into a subsequent list call to obtain the next listing.
       *
       * The cursor is `undefined` when the end of the list is reached.
       */
      cursor: string | undefined;
    }>;
  }

  /**
   * Class for interacting with a [Fastly KV store](https://developer.fastly.com/reference/api/kv-store/) entry.
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
     * Binary metadata associated with this entry, up to 1000 bytes.
     *
     * Returns `null` if no metadata is set.
     *
     * @version 3.26.0
     */
    metadata(): ArrayBuffer | null;

    /**
     * Metadata associated with this entry, decoded as a UTF-8 string.
     *
     * Returns `null` if no metadata is set.
     *
     * @throws If the metadata is not valid UTF-8.
     * @version 3.26.0
     */
    metadataText(): string | null;
  }
}
