/**
 * @experimental
 */
declare module 'fastly:experimental' {
  import type { enforceExplicitBackends } from 'fastly:backend';

  /**
   * JavaScript SDK version string for the JS runtime engine build.
   *
   * @experimental
   * @hidden
   */
  export const sdkVersion: string;
  /**
   * @experimental
   * @hidden
   */
  export function setBaseURL(base: URL | null | undefined): void;
  /**
   * Set the default backend to use when dynamic backends are disabled.
   *
   * For backwards compatibility, unless `allowDynamicBackends` has been explicitly
   * called before invoking this function, it will disable dynamic backends when
   * it is called, as if calling `allowDynamicBackends(false)`.
   *
   * @param backend The name of the default backend.
   * @experimental
   * @deprecated Use {@link enforceExplicitBackends} instead.
   */
  export function setDefaultBackend(backend: string): void;
  /**
   * Causes the Fastly Compute JS runtime environment to log debug information to stdout.
   *
   * **Note**: This is mostly for internal debugging purposes and will generate highly unstable
   * output.
   * @experimental
   * @hidden
   */
  export function enableDebugLogging(enabled: boolean): void;

  /**
   * Embed a file as a Uint8Array.
   *
   * **Note**: Can only be used during build-time initialization, not when processing requests.
   *
   * @param path The path to include, relative to the project's top-level directory.
   * @experimental
   */
  export function includeBytes(path: string): Uint8Array<ArrayBuffer>;

  /**
   * Control whether or not Dynamic Backends are allowed within this Fastly
   * Compute service.
   *
   * By default, Dynamic Backends are disabled within a JavaScript application
   * as it can be a potential avenue for third-party JavaScript code to send
   * requests, potentially including sensitive/secret data, off to destinations
   * that the JavaScript project was not intending, which could be a security
   * issue.
   *
   * **Note**: This feature is disabled by default for Fastly Services. Please
   * contact [Fastly Support](https://support.fastly.com/hc/en-us/requests/new?ticket_form_id=360000269711)
   * to request the feature be enabled on the Fastly Services which require
   * Dynamic Backends.
   *
   * @param enabled Whether or not to allow Dynamic Backends.
   * @experimental
   * @deprecated Use {@link enforceExplicitBackends} instead.
   */
  export function allowDynamicBackends(enabled: boolean): void;
  /**
   * Enable Dynamic Backends with default timeout configuration.
   *
   * @param defaultConfig Default timeout configuration for dynamic backends.
   * @throws `RangeError` if any timeout value is negative or greater than or
   *   equal to 2^32.
   * @experimental
   * @deprecated Use {@link enforceExplicitBackends} instead.
   */
  export function allowDynamicBackends(defaultConfig: {
    /** Maximum duration in milliseconds to wait for a connection to be established. */
    connectTimeout?: number;
    /** Maximum duration in milliseconds to wait for the server response to begin after a TCP connection is established and the request has been sent. */
    firstByteTimeout?: number;
    /** Maximum duration in milliseconds that Fastly will wait while receiving no data on a download from a backend. */
    betweenBytesTimeout?: number;
  }): void;

  /**
   * Get information about an error as a ready-to-print array.
   * This includes the error name, message, and a call stack.
   *
   * If `--enable-stack-traces` is specified during the build, the call stack
   * will be mapped using source maps. If `--enable-stack-traces` is specified
   * and `--exclude-sources` is not, this will also include a code dump of
   * neighboring lines of user code.
   *
   * @param error The error to retrieve information about. If a string is
   *   provided, it is first converted to an `Error`.
   * @version 3.37.0
   */
  export function mapError(error: Error | string): (Error | string)[];

  /**
   * Calls {@link mapError} and outputs the results to stderr.
   *
   * @param error The error to map and log. If a string is provided, it is
   *   first converted to an `Error`.
   * @version 3.37.0
   */
  export function mapAndLogError(error: Error | string): void;

  /**
   * @version 3.40.0
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
   * @experimental
   * @version 3.40.0
   */
  export function setReusableSandboxOptions(
    options: ReusableSandboxOptions,
  ): void;
}
