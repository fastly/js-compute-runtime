declare module 'fastly:debug' {
  /**
   * Causes the Fastly Compute JS runtime environment to log debug information to stdout.
   *
   * **Note**: This is mostly for internal debugging purposes and will generate highly unstable
   * output.
   * @hidden
   */
   export function enableDebugLogging(enabled: boolean): void;
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
   * @version 4.3.0
   */
  export function mapError(error: Error | string): (Error | string)[];

  /**
   * Calls {@link mapError} and outputs the results to stderr.
   *
   * @param error The error to map and log. If a string is provided, it is
   *   first converted to an `Error`.
   * @version 4.3.0
   */
  export function mapAndLogError(error: Error | string): void;
}