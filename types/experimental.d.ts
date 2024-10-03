/// <reference path="globals.d.ts" />

/**
 * @experimental
 */
declare module "fastly:experimental" {
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
  export function includeBytes(path: string): Uint8Array;

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
   */
  export function allowDynamicBackends(enabled: boolean): void;
  export function allowDynamicBackends(defaultConfig: {
    connectTimeout?: number;
    firstByteTimeout?: number;
    betweenBytesTimeout?: number;
  }): void;
}
