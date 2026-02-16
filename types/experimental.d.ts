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
   *
   * Set the default backend to use when dynamic backends are disabled.
   *
   * For backwards compatibility, unless allowDynamicBackends has been explicitly
   * called before invoking this function, it will disable dynamic backends when it is called
   * in order to apply, as if calling allowDynamicBackends(false).
   *
   * @experimental
   * @deprecated use {@link enforceExplicitBackends} instead.
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
   * @param path - The path to include, relative to the project's top-level directory
   *
   * **Note**: Can only be used during build-time initialization, not when processing requests.
   * @experimental
   */
  export function includeBytes(path: string): Uint8Array<ArrayBuffer>;

  /**
   * Whether or not to allow Dynamic Backends.
   *
   * By default, Dynamic Backends are disabled within a JavaScript application as it can be a potential
   * avenue for third-party JavaScript code to send requests, potentially including sensitive/secret data,
   * off to destinations that the JavaScript project was not intending, which could be a security issue.
   *
   * @note
   * This feature is in disabled by default for Fastly Services. Please contact [Fastly Support](https://support.fastly.com/hc/en-us/requests/new?ticket_form_id=360000269711) to request the feature be enabled on the Fastly Services which require Dynamic Backends.
   *
   * @experimental
   * @deprecated, use {@link enforceExplicitBackends} instead.
   */
  export function allowDynamicBackends(enabled: boolean): void;
  export function allowDynamicBackends(defaultConfig: {
    connectTimeout?: number;
    firstByteTimeout?: number;
    betweenBytesTimeout?: number;
  }): void;

  /**
   * Get information about an error as a ready-to-print array of strings.
   * This includes the error name, message, and a call stack.
   * If --enable-stack-traces is specified during the build, the call stack
   * will be mapped using source maps.
   * If --enable-stack-traces is specified and --exclude-sources is not specified,
   * then this will also include a code dump of neighboring lines of user code.
   * @param error
   */
  export function mapError(error: Error | string): (Error | string)[];

  /**
   * Calls mapError(error) and outputs the results to stderr output.
   * @param error
   */
  export function mapAndLogError(error: Error | string): void;

  export interface ReusableSandboxOptions {
    /**
     * The maximum number of requests to handle with a single sandbox instance before it is recycled.
     * Default is 1. A value of 0 means there is no maximum, and the sandbox may be reused indefinitely until it is recycled for another reason (e.g., timeout or memory limit).
     */
    maxRequests?: number;
    /**
     * The maximum amount of time in milliseconds to wait for the next request before recycling the sandbox. Default is up to the platform and not specified.
     */
    betweenRequestTimeoutMs?: number;
    /**
     * The maximum amount of memory in MiB that the sandbox is allowed to use. If the sandbox exceeds this limit, it will be recycled before handling the next request. Default is no limit.
     */
    maxMemoryMiB?: number;
    /**
     * The maximum amount of time in milliseconds that a sandbox is allowed to run before it is recycled after handling the current request. Default is no timeout.
     */
    sandboxTimeoutMs?: number;
  }
  /**
   * Configure reuse of the same underlying sandbox for multiple requests, 
   * which can improve performance by avoiding the overhead of initializing a 
   * new sandbox for each request.
   * @experimental
   * @param options - Configuration options for sandbox reuse
   */
  export function setReusableSandboxOptions(options: ReusableSandboxOptions): void;
}
