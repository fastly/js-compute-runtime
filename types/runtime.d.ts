declare module 'fastly:runtime' {
  /**
   * JavaScript SDK version string for the JS runtime engine build.
   *
   * @hidden
   */
  export const sdkVersion: string;
  /**
   * @hidden
   */
  export function setBaseURL(base: URL | null | undefined): void;

  /**
   * Embed a file as a Uint8Array.
   *
   * **Note**: Can only be used during build-time initialization, not when processing requests.
   *
   * @param path The path to include, relative to the project's top-level directory.
   */
  export function includeBytes(path: string): Uint8Array<ArrayBuffer>;

  /**
   * @version 4.3.0
   */
  export interface ReusableSandboxOptions {
    /**
     * The maximum number of requests to handle with a single sandbox instance
     * before it is recycled. Default is 1. A value of 0 means no maximum.
     */
    maxRequests?: number;
    /**
     * The maximum time in milliseconds to wait for the next request before
     * recycling the sandbox. Default is up to the platform.
     */
    betweenRequestTimeoutMs?: number;
    /**
     * The maximum memory in MiB that the sandbox is allowed to use. If
     * exceeded, the sandbox will be recycled before the next request.
     * Default is no limit.
     */
    maxMemoryMiB?: number;
    /**
     * The maximum time in milliseconds that a sandbox is allowed to run
     * before it is recycled after handling the current request. Default is
     * no timeout.
     */
    sandboxTimeoutMs?: number;
  }
  /**
   * Configure reuse of the same underlying sandbox for multiple requests,
   * which can improve performance by avoiding the overhead of initializing a
   * new sandbox for each request.
   *
   * **Note**: Can only be used during build-time initialization, not when
   * processing requests.
   *
   * @param options Configuration options for sandbox reuse.
   * @version 4.3.0
   */
  export function setReusableSandboxOptions(
    options: ReusableSandboxOptions,
  ): void;
}