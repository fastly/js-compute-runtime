/// <reference path="../types/globals.d.ts" />

declare module 'fastly:body' {
  /**
   * A low-level API for constructing and manipulating HTTP message bodies outside of the standard Fetch API.
   *
   * **Note**: Can only be used when processing requests, not during build-time initialization.
   *
   * @example
   * ```js
   * import { FastlyBody } from 'fastly:body';
   *
   * addEventListener('fetch', (event) => {
   *   const body = new FastlyBody();
   *   body.append('Hello, ');
   *   body.append('world!');
   *   const chunk = body.read(1024);
   *   event.respondWith(new Response(chunk, { status: 200 }));
   * });
   * ```
   *
   * @version 3.9.0
   */
  export class FastlyBody {
    /**
     * Creates a new, empty `FastlyBody` instance.
     */
    constructor();

    /**
     * Appends the contents of another `FastlyBody` to this body.
     * The `dest` body is consumed by this operation and should not be reused.
     *
     * @param dest The `FastlyBody` to append to this body.
     * @throws `Error` if `dest` is not a `FastlyBody` instance.
     */
    concat(dest: FastlyBody): void;

    /**
     * Reads up to `chunkSize` bytes from the body, advancing the read position.
     * Returns an empty `ArrayBuffer` when no more data is available.
     *
     * @param chunkSize The maximum number of bytes to read. Must be a positive number.
     * @throws `Error` if `chunkSize` is not a positive number, is `NaN`, or is `Infinity`.
     */
    read(chunkSize: number): ArrayBuffer;

    /**
     * Appends data to the end of this body.
     *
     * @param data The data to append.
     * @throws `TypeError` if `data` is a guest-backed `ReadableStream` (not yet supported),
     *   or if the `ReadableStream` is unusable (locked or already read from).
     */
    append(data: BodyInit): void;

    /**
     * Prepends data to the beginning of this body.
     *
     * @param data The data to prepend.
     * @throws `TypeError` if `data` is a guest-backed `ReadableStream` (not yet supported),
     *   or if the `ReadableStream` is unusable (locked or already read from).
     */
    prepend(data: BodyInit): void;

    /**
     * Closes the body, signaling that no more data will be written.
     */
    close(): void;

    /**
     * Abandons the body, discarding its contents without finalizing.
     * Unlike {@link close}, this signals that the body is being discarded rather than completed.
     *
     * @version 3.30.0
     */
    abandon(): void;
  }
}
