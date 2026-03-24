declare module 'fastly:cache' {
  interface PurgeOptions {
    /**
     * Where to purge the content from.
     *
     * "pop" - This will remove the content from the POP that contains the currently executing instance.
     * "global" - This will remove the content from all of Fastly.
     */
    scope: 'pop' | 'global';
  }

  /**
   * The `SimpleCache` class provides a simplified interface to inserting and retrieving entries
   * from Fastly's Cache. All methods on the class are static methods.
   *
   * For more advanced use cases requiring transactional lookups, request collapsing, or
   * revalidation, use {@link CoreCache} instead.
   *
   * @example
   * ```js
   * import { SimpleCache } from 'fastly:cache';
   *
   * addEventListener('fetch', event => event.respondWith(app(event)));
   *
   * async function app(event) {
   *   const path = new URL(event.request.url).pathname;
   *   let page = await SimpleCache.getOrSet(path, async () => {
   *     return {
   *       value: await render(path),
   *       // Store the page in the cache for 1 minute
   *       ttl: 60 // seconds
   *     }
   *   });
   *   return new Response(await page.text(), {
   *     headers: { 'content-type': 'text/plain;charset=UTF-8' }
   *   });
   * }
   *
   * async function render(path) {
   *   // expensive/slow function which constructs and returns the contents for a given path
   *   await new Promise(resolve => setTimeout(resolve, 10_000));
   *   return path;
   * }
   * ```
   */
  export class SimpleCache {
    /**
     * Gets the entry associated with the key `key` from the cache.
     *
     * @param key The key to retrieve from within the cache (up to 8,135 characters).
     * @returns The cached entry, or `null` if the key does not exist in the cache.
     * @throws `TypeError` if the provided `key` is an empty string, cannot be coerced to a string, or is longer than 8,135 characters.
     */
    static get(key: string): SimpleCacheEntry | null;
    /**
     * Inserts a new entry or overwrites an existing entry in the cache.
     *
     * @deprecated Use {@link SimpleCache.getOrSet} instead.
     * @param key The key to store the entry under (up to 8,135 characters).
     * @param value The value to store in the cache.
     * @param ttl The time-to-live for this entry, in seconds.
     * @throws `TypeError` if the provided `key` is an empty string, cannot be coerced to a string, or is longer than 8,135 characters.
     */
    static set(key: string, value: BodyInit, ttl: number): undefined;
    /**
     * Inserts a new entry or overwrites an existing entry in the cache using a `ReadableStream`.
     *
     * @deprecated Use {@link SimpleCache.getOrSet} instead.
     * @param key The key to store the entry under (up to 8,135 characters).
     * @param value A `ReadableStream` to store in the cache.
     * @param ttl The time-to-live for this entry, in seconds.
     * @param length The length of the `ReadableStream` value, in bytes.
     * @throws `TypeError` if the provided `key` is an empty string, cannot be coerced to a string, or is longer than 8,135 characters.
     */
    static set(
      key: string,
      value: ReadableStream,
      ttl: number,
      length: number,
    ): undefined;
    /**
     * Attempts to get an entry from the cache for the supplied `key`. If no entry is found
     * (or has expired), the supplied `set` function is executed and its result is inserted
     * into the cache under the supplied `key`.
     *
     * @param key The key to lookup and/or store the entry under (up to 8,135 characters).
     * @param set A function to execute if the cache does not have a usable entry. Should return a Promise resolving with `value` and `ttl` (in seconds).
     * @throws `TypeError` if the provided `key` is an empty string, cannot be coerced to a string, or is longer than 8,135 characters.
     * @throws `TypeError` if the provided `ttl` cannot be coerced to a number, is negative, `NaN`, or `Infinity`.
     */
    static getOrSet(
      key: string,
      set: () => Promise<{ value: BodyInit; ttl: number }>,
    ): Promise<SimpleCacheEntry>;
    /**
     * Attempts to get an entry from the cache for the supplied `key`. If no entry is found
     * (or has expired), the supplied `set` function is executed and its result is inserted
     * into the cache under the supplied `key`.
     *
     * When the value is a `ReadableStream`, the `length` property must be provided.
     *
     * @param key The key to lookup and/or store the entry under (up to 8,135 characters).
     * @param set A function to execute if the cache does not have a usable entry. Should return a Promise resolving with `value`, `ttl` (in seconds), and `length` (in bytes).
     * @throws `TypeError` if the provided `key` is an empty string, cannot be coerced to a string, or is longer than 8,135 characters.
     * @throws `TypeError` if the provided `ttl` cannot be coerced to a number, is negative, `NaN`, or `Infinity`.
     */
    static getOrSet(
      key: string,
      set: () => Promise<{
        value: ReadableStream;
        ttl: number;
        length: number;
      }>,
    ): Promise<SimpleCacheEntry>;
    /**
     * Purges the entry associated with the key `key` from the cache.
     *
     * @param key The key to purge from within the cache (up to 8,135 characters).
     * @param options Options controlling the scope of the purge.
     * @throws `TypeError` if the provided `key` is an empty string, cannot be coerced to a string, or is longer than 8,135 characters.
     */
    static purge(key: string, options: PurgeOptions): undefined;
  }

  /**
   * Represents an entry retrieved from the {@link SimpleCache}.
   */
  export interface SimpleCacheEntry {
    /** Provides access to the entry's contents as a `ReadableStream`. */
    get body(): ReadableStream;
    /** Indicates whether the body has been read yet. */
    get bodyUsed(): boolean;
    /** Reads the entry's stream to completion and returns the result as a string, decoded using UTF-8. */
    text(): Promise<string>;
    /** Reads the entry's stream to completion and parses the result as JSON. */
    json(): Promise<object>;
    /** Reads the entry's stream to completion and returns the result as an `ArrayBuffer`. */
    arrayBuffer(): Promise<ArrayBuffer>;
  }

  /**
   * Options for cache lookups.
   *
   * @version 3.9.0
   */
  export interface LookupOptions {
    /**
     * Note: These headers are narrowly useful for implementing cache lookups incorporating the semantics of the HTTP Vary header.
     * The headers act as additional factors in object selection, and the choice of which headers to factor in is determined during insertion, via the `vary` property within `TransactionInsertOptions`.
     * A lookup will succeed when there is at least one cached item that matches lookup's `key`, and all of the lookup's headers included in the cache items' `vary` list match the corresponding headers in that cached item.
     * A typical example is a cached HTTP response, where the request had an "Accept-Encoding" header. In that case, the origin server may or may not decide on a given encoding, and whether that same response is suitable for a request with a different (or missing) Accept-Encoding header is determined by whether Accept-Encoding is listed in Vary header in the origin's response.
     */
    headers?: HeadersInit;
  }

  /**
   * Options for non-transactional cache insertions via {@link CoreCache.insert}.
   *
   * @version 3.9.0
   */
  export interface InsertOptions extends TransactionInsertOptions {
    /**
     * Note: These headers are narrowly useful for implementing cache lookups incorporating the semantics of the HTTP Vary header.
     * The headers act as additional factors in object selection, and the choice of which headers to factor in is determined during insertion, via the `vary` property within `TransactionInsertOptions`.
     * A lookup will succeed when there is at least one cached item that matches lookup's `key`, and all of the lookup's headers included in the cache items' `vary` list match the corresponding headers in that cached item.
     * A typical example is a cached HTTP response, where the request had an "Accept-Encoding" header. In that case, the origin server may or may not decide on a given encoding, and whether that same response is suitable for a request with a different (or missing) Accept-Encoding header is determined by whether Accept-Encoding is listed in Vary header in the origin's response.
     */
    headers?: HeadersInit;
  }

  /**
   * Options for transactional cache insertions.
   *
   * @version 3.9.0
   */
  export interface TransactionInsertOptions {
    /**
     * Sets the "time to live" for the cache item in milliseconds: The time for which the item will be considered fresh.
     */
    maxAge: number;
    /**
     * Sets the list of headers that must match when looking up this cached item.
     */
    vary?: Array<string>;
    /**
     * Sets the initial age of the cached item, in milliseconds, to be used in freshness calculations.
     *
     * The initial age is 0 by default.
     */
    initialAge?: number;
    /**
     * Sets the stale-while-revalidate period, in milliseconds for the cached item, which is the time for which the item can be safely used despite being considered stale.
     * Having a stale-while-revalidate period provides a signal that the cache should be updated (or its contents otherwise revalidated for freshness) asynchronously, while the stale cached item continues to be used, rather than blocking on updating the cached item.
     * The methods `CacheState.prototype.usable` and `CacheState.prototype.stale` can be used to determine the current state of an item.
     *
     * The stale-while-revalidate period is 0 by default.
     */
    staleWhileRevalidate?: number;
    /**
     * Sets the surrogate keys that can be used for purging this cached item.
     * Surrogate key purges are the only means to purge specific items from the cache. At least one surrogate key must be set in order to remove an item without performing a purge-all, waiting for the item's TTL to elapse, or overwriting the item with insert().
     * Surrogate keys must contain only printable ASCII characters (those between 0x21 and 0x7E, inclusive). Any invalid keys will be ignored.
     *
     * See the [Fastly surrogate keys guide](https://docs.fastly.com/en/guides/working-with-surrogate-keys) for details.
     */
    surrogateKeys?: Array<string>;
    /**
     * Sets the size of the cached item, in bytes, when known prior to actually providing the bytes.
     * It is preferable to provide a length, if possible.
     * Clients that begin streaming the item's contents before it is completely provided will see the promised length which allows them to, for example, use content-length instead of transfer-encoding: chunked if the item is used as the body of a Request or Response.
     */
    length?: number;
    /**
     * Sets the user-defined metadata to associate with the cached item.
     */
    userMetadata?: ArrayBufferView | ArrayBuffer | URLSearchParams | string;
    /**
     * Enable or disable PCI/HIPAA-compliant non-volatile caching.
     * By default, this is false.
     *
     * See the [Fastly PCI-Compliant Caching and Delivery documentation](https://docs.fastly.com/products/pci-compliant-caching-and-delivery) for details.
     */
    sensitive?: boolean;
  }

  /**
   * The CoreCache class exposes the Compute Core Cache API, the same set of primitive operations used to build Fastly services.
   * The Core Cache API puts the highest level of power in the hands of the user, but requires manual serialization of cache contents and explicit handling of request collapsing and revalidation control flow.
   *
   * @version 3.9.0
   */
  export class CoreCache {
    /**
     * Perform a non-transactional lookup into the cache, returning a CacheEntry if a usable cached item was found.
     * A cached item is usable if its age is less than the sum of its TTL and its stale-while-revalidate period. Items beyond that age are unusably stale.
     *
     * Note: A non-transactional lookup will not attempt to coordinate with any concurrent cache lookups.
     * If two instances of the service perform a lookup at the same time for the same cache key, and the item is not yet cached, they will both return `null`.
     * Without further coordination, they may both end up performing the work needed to insert() the item (which usually involves origin requests and/or computation) and racing with each other to insert.
     * To resolve such races between concurrent lookups, use `CoreCache.transactionLookup()` instead.
     *
     * @param key A cache key which is a string with a length of up to 8,135 that identify a cached item. The cache key may not uniquely identify an item; headers can be used to augment the key when multiple items are associated with the same key.
     * @param options A set of options to used whilst performing this lookup into the cache.
     * @throws `TypeError` if the provided `key` is an empty string, cannot be coerced to a string, or is longer than 8,135 characters.
     */
    static lookup(key: string, options?: LookupOptions): CacheEntry | null;

    /**
     * Perform a non-transactional insertion into the cache, returning a `FastlyBody` instance for providing the cached object itself.
     * For the insertion to complete successfully, the object must be written into the returned `FastlyBody` instance, and then `FastlyBody.prototype.close` must be called.
     * If `FastlyBody.prototype.close` does not get called, the insertion is considered incomplete, and any concurrent lookups that may be reading from the object as it is streamed into the cache may encounter a streaming error.
     *
     * Note: Like `CoreCache.lookup()`, `CoreCache.insert()` may race with concurrent lookups or insertions, and will unconditionally overwrite existing cached items rather than allowing for revalidation of an existing object.
     * The transactional equivalent of this function is `TransactionCacheEntry.insert()`, which may only be called following a `CoreCache.transactionLookup()` call and the returned `CacheEntry` when has a state where `CacheState.mustInsertOrUpdate()` returns true.
     *
     * @param key A cache key which is a string with a length of up to 8,135 that identify a cached item. The cache key may not uniquely identify an item; headers can be used to augment the key when multiple items are associated with the same key.
     * @param options A set of options to used whilst performing this insertion into the cache.
     * @throws `TypeError` if the provided `key` is an empty string, cannot be coerced to a string, or is longer than 8,135 characters.
     */
    static insert(
      key: string,
      options: InsertOptions,
    ): import('fastly:body').FastlyBody;

    /**
     * Perform a transactional lookup into the cache, returning a `TransactionCacheEntry` instance.
     *
     * Transactions coordinate between concurrent actions on the same cache key, incorporating concepts of [request collapsing](https://developer.fastly.com/learning/concepts/request-collapsing/) and [revalidation](https://developer.fastly.com/learning/concepts/stale/), though at a lower level that does not automatically interpret HTTP semantics.
     *
     * Request Collapsing:
     * If there are multiple concurrent calls to `CoreCache.transactionLookup()` for the same item and that item is not present,
     * just one of the callers will be instructed to insert the item into the cache as part of the transaction.
     * The other callers will block until the metadata for the item has been inserted, and can then begin streaming its contents out of the cache at the same time that the inserting caller streams them into the cache.
     *
     * Revalidation:
     * Similarly, if an item is usable but stale, and multiple callers attempt a `CoreCache.transactionLookup()` concurrently, they will all be given access to the stale item, but only one will be designated to perform an asynchronous update (or insertion) to freshen the item in the cache.
     *
     * @param key A cache key which is a string with a length of up to 8,135 that identify a cached item. The cache key may not uniquely identify an item; headers can be used to augment the key when multiple items are associated with the same key.
     * @param options A set of options to used whilst performing this lookup into the cache.
     * @throws `TypeError` if the provided `key` is an empty string, cannot be coerced to a string, or is longer than 8,135 characters.
     */
    static transactionLookup(
      key: string,
      options?: LookupOptions,
    ): TransactionCacheEntry;
  }

  /**
   * The status of this lookup (and potential transaction).
   *
   * @version 3.9.0
   */
  export class CacheState {
    /**
     * Returns `true` if a cached item was located.
     *
     * Even if a cached item is found, the cached item might be stale and require updating. Use `mustInsertOrUpdate()` to determine whether this transaction client is expected to update the cached item.
     */
    found(): boolean;
    /**
     * Returns `true` if the cached item is usable.
     *
     * A cached item is usable if its age is less than the sum of the `maxAge` and `staleWhileRevalidate` periods.
     */
    usable(): boolean;
    /**
     * Returns `true` if the cached item is stale.
     *
     * A cached item is stale if its age is greater than its `maxAge` period.
     */
    stale(): boolean;
    /**
     * Returns `true` if a fresh cache item was not found, and this transaction client is expected to insert a new item or update a stale item.
     */
    mustInsertOrUpdate(): boolean;
  }

  /**
   * Options for requesting a byte range within a cached item.
   *
   * @version 3.9.0
   */
  export interface CacheBodyOptions {
    /**
     * The offset from which to start the range.
     */
    start: number;
    /**
     * How long the range should be.
     */
    end: number;
  }

  /**
   * Represents a cached item retrieved via {@link CoreCache.lookup} or as part of a
   * {@link TransactionCacheEntry.insertAndStreamBack} operation.
   *
   * @version 3.9.0
   */
  export class CacheEntry {
    /**
     * Closes the connection to the cache for this `CacheEntry`.
     */
    close(): void;

    /**
     * Returns a `CacheState` instance which reflects the current state of this `CacheEntry` instance.
     */
    state(): CacheState;

    /**
     * The user-controlled metadata associated with the cached item.
     */
    userMetadata(): ArrayBuffer;

    /**
     * Retrieves the cached item as a `ReadableStream`.
     *
     * Only one stream can be active at a time for a given `CacheEntry`. An error will be thrown if a stream is already active for this CacheEntry.
     *
     * @param options Optional property used to request a range of bytes within the cached item.
     */
    body(options?: CacheBodyOptions): ReadableStream;

    /**
     * The size in bytes of the cached item, if known.
     *
     * @returns The length in bytes, or `null` if the length is currently unknown. The length may be unknown if the item is currently being streamed into the cache without a fixed length.
     */
    length(): number | null;

    /**
     * The time in milliseconds for which the cached item is considered fresh.
     */
    maxAge(): number;

    /**
     * The time in milliseconds for which a cached item can safely be used despite being considered stale.
     */
    staleWhileRevalidate(): number;

    /**
     * The current age in milliseconds of the cached item.
     */
    age(): number;

    /**
     * Determines the number of cache hits to this cached item.
     *
     * Note: this hit count only reflects the view of the server that supplied the cached item. Due to clustering, this count may vary between potentially many servers within the data center where the item is cached. See the [clustering documentation](https://developer.fastly.com/learning/vcl/clustering/) for details, though note that the exact caching architecture of Compute is different from VCL services.
     */
    hits(): number;
  }

  /**
   * Options for updating a cached item's metadata via {@link TransactionCacheEntry.update}.
   *
   * @version 3.9.0
   */
  export interface TransactionUpdateOptions {
    /**
     * Sets the "time to live" for the cache item in milliseconds: The time for which the item will be considered fresh.
     */
    maxAge: number;
    /**
     * Sets the list of headers that must match when looking up this cached item.
     */
    vary?: Array<string>;
    /**
     * Sets the initial age of the cached item, in milliseconds, to be used in freshness calculations.
     *
     * The initial age is 0 by default.
     */
    initialAge?: number;
    /**
     * Sets the stale-while-revalidate period, in milliseconds for the cached item, which is the time for which the item can be safely used despite being considered stale.
     * Having a stale-while-revalidate period provides a signal that the cache should be updated (or its contents otherwise revalidated for freshness) asynchronously, while the stale cached item continues to be used, rather than blocking on updating the cached item.
     * The methods `CacheState.prototype.usable` and `CacheState.prototype.stale` can be used to determine the current state of an item.
     *
     * The stale-while-revalidate period is 0 by default.
     */
    staleWhileRevalidate?: number;
    /**
     * Sets the surrogate keys that can be used for purging this cached item.
     * Surrogate key purges are the only means to purge specific items from the cache. At least one surrogate key must be set in order to remove an item without performing a purge-all, waiting for the item's TTL to elapse, or overwriting the item with insert().
     * Surrogate keys must contain only printable ASCII characters (those between `0x21` and `0x7E`, inclusive). Any invalid keys will be ignored.
     *
     * See the [Fastly surrogate keys guide](https://docs.fastly.com/en/guides/working-with-surrogate-keys) for details.
     */
    surrogateKeys?: Array<string>;
    /**
     * Sets the size of the cached item, in bytes, when known prior to actually providing the bytes.
     * It is preferable to provide a length, if possible.
     * Clients that begin streaming the item's contents before it is completely provided will see the promised length which allows them to, for example, use content-length instead of transfer-encoding: chunked if the item is used as the body of a Request or Response.
     */
    length?: number;
    /**
     * Sets the user-defined metadata to associate with the cached item.
     */
    userMetadata?: ArrayBufferView | ArrayBuffer | URLSearchParams | string;
  }

  /**
   * Represents a cached item retrieved via a transactional lookup ({@link CoreCache.transactionLookup}).
   * Extends {@link CacheEntry} with methods for inserting, updating, and cancelling cache transactions.
   *
   * @version 3.9.0
   */
  export class TransactionCacheEntry extends CacheEntry {
    /**
     * Perform a transactional cache insertion, returning a `FastlyBody` instance for providing the cached object itself.
     *
     * This method should only be called when `TransactionCacheEntry.state().mustInsertOrUpdate()` is true; otherwise, an error will be thrown when attempting to perform the insertion.
     */
    insert(options: TransactionInsertOptions): import('fastly:body').FastlyBody;

    /**
     * Perform a transactional cache insertion, returning a `FastlyBody` instance for providing the cached object itself, and a `CacheEntry` instance which can be used to stream out the newly-inserted cache item.
     *
     * For the insertion to complete successfully, the object must be written into the returned `FastlyBody` instance, and then `FastlyBody.prototype.close` must be called.
     * If `FastlyBody.prototype.close` does not get called, the insertion is considered incomplete, and any concurrent lookups that may be reading from the object as it is streamed into the cache may encounter a streaming error.
     */
    insertAndStreamBack(
      options: TransactionInsertOptions,
    ): [import('fastly:body').FastlyBody, CacheEntry];
    /**
     * Perform an update of the cache item's metadata.
     */
    update(options: TransactionUpdateOptions): void;
    /**
     * Cancel an obligation to provide an object to the cache.
     */
    cancel(): void;
  }
}
