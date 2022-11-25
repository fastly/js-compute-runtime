
declare module "fastly:cache-override" {
  /**
 * Set the cache override mode on a request
 *
 * None
 *   Do not override the behavior specified in the origin response’s cache control headers.
 * Pass
 *   Do not cache the response to this request, regardless of the origin response’s headers.
 * Override
 *   Override particular cache control settings using a {@linkcode CacheOverride} object.
 *
 * The origin response’s cache control headers will be used for ttl and stale_while_revalidate if None.
 */
  type CacheOverrideMode = "none" | "pass" | "override";

  /**
   * Base class for Cache Override, which is used to configure caching behavior.
   */
  interface CacheOverrideInit {
    /**
     * Override the caching behavior of this request to use the given Time to Live (TTL), in seconds.
     */
    ttl?: number;
    /**
     * Override the caching behavior of this request to use the given `stale-while-revalidate` time,
     * in seconds
     */
    swr?: number;
    /**
     * Override the caching behavior of this request to include the given surrogate key, provided as
     * a header value.
     *
     * See the [Fastly surrogate keys guide](https://docs.fastly.com/en/guides/purging-api-cache-with-surrogate-keys)
     * for details.
     */
    surrogateKey?: string;
    /**
     * Override the caching behavior of this request to enable or disable PCI/HIPAA-compliant
     * non-volatile caching.
     *
     * By default, this is false, which means the request may not be PCI/HIPAA-compliant. Set it to
     * true to enable compliant caching.
     *
     * See the [Fastly PCI-Compliant Caching and Delivery documentation](https://docs.fastly.com/products/pci-compliant-caching-and-delivery)
     * for details.
     */
    pci?: boolean;
  }
  /**
   * Configures the caching behavior of a {@linkcode Response}.
   *
   * Normally, the HTTP Headers on a Response would control how the Response is cached,
   * but CacheOverride can be set on a {@linkcode Request}, to define custom caching behavior.
   */
  interface CacheOverride extends CacheOverrideInit {
    mode: CacheOverrideMode;
  }

  var CacheOverride: {
    prototype: CacheOverride;
    new(mode: CacheOverrideMode, init?: CacheOverrideInit): CacheOverride;
  };
}
