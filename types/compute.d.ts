declare module 'fastly:compute' {
  /**
   * Get the current elapsed vCPU time in milliseconds for the current request handler
   */
  export function vCpuTime(): number;

  /**
   * Purge the HTTP & Core cache via a surrogate key
   * @param surrogateKey - The surrogate key string to purge.
   * @param soft - Enable to perform a soft purge, retaining stale cache entries to
   *               reduce load on the origin server, defaults to a hard purge.
   */
  export function purgeSurrogateKey(
    surrogateKey: string,
    soft?: boolean,
  ): boolean;
}
