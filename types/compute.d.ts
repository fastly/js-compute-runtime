declare module 'fastly:compute' {
  /**
   * Get the current elapsed vCPU time in milliseconds for the current request handler.
   *
   * @version 3.22.0
   */
  export function vCpuTime(): number;

  /**
   * Purge the given surrogate key from Fastly's HTTP and Core caches.
   *
   * There are two purge modes: hard purge (the default) clears all matching items from
   * the cache immediately, while soft purge retains stale entries in the cache, reducing
   * origin load while enabling stale revalidations.
   *
   * See the [Fastly Purge documentation](https://www.fastly.com/documentation/guides/concepts/edge-state/cache/purging/#surrogate-key-purge)
   * for more information on caching and purge operations.
   *
   * @param surrogateKey The surrogate key string to purge.
   * @param soft Enable to perform a soft purge, retaining stale cache entries to
   *   reduce load on the origin server. Defaults to a hard purge.
   * @version 3.22.0
   */
  export function purgeSurrogateKey(
    surrogateKey: string,
    soft?: boolean,
  ): void;
}
