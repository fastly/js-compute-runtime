
declare module "fastly:cache-override" {
  /**
   * Configures the caching behavior of a {@linkcode "globals".Response}.
   *
   * Normally, the HTTP Headers on a {@linkcode "globals".Response} would control how the {@linkcode "globals".Response} is cached,
   * but `CacheOverride` can be set on a {@linkcode "globals".Request}, to define custom caching behavior.
   * 
   * @example
   * In this example we override the cache for all the requests prefixed /static/ to have a long TTL (Time To Live), 
   * and the home page to have a short TTL and a long SWR (Stale While Revalidate).
   * 
   * View this example on [Fiddle](https://fiddle.fastly.dev/fiddle/aec84add).
   * 
   * ```js
   * /// <reference types="@fastly/js-compute" />
   * import { CacheOverride } from "fastly:cache-override";
   * 
   * // In this example we override the cache for all the requests prefixed /static/ 
   * // to have a long TTL (Time To Live), and the home page to have a short TTL and 
   * // a long SWR (Stale While Revalidate).
   * async function app (event) {
   *   const path = (new URL(event.request.url)).pathname;
   *   let cacheOverride;
   *   if (path == '/') {
   *     cacheOverride = new CacheOverride('override', {ttl: 10, swr: 86_400});
   *   } else if (path.startsWith('/static/')) {
   *     cacheOverride = new CacheOverride('override', {ttl: 86_400});
   *   } else {
   *     cacheOverride = new CacheOverride('none')
   *   }
   *   // For any request, return the fastly homepage -- without defining a backend!
   *   return fetch(event.request.url, {
   *     cacheOverride,
   *     backend: 'origin_0'
   *   });
   * }
   * addEventListener("fetch", event => event.respondWith(app(event)));
   * ```
   */
  class CacheOverride {
    /**
     * @param mode Sets the cache override mode for a request
     * 
     * If set to:
     * - "none": Do not override the behavior specified in the origin response’s cache control headers.
     * - "pass": Do not cache the response to this request, regardless of the origin response’s headers.
     * - "override": Override particular cache control settings using a {@linkcode CacheOverride} object.
     * 
     * @param [init]
     *
     * @param {number} [init.ttl]
     * Override the caching behavior of this request to use the given Time to Live (TTL), in seconds.
     * 
     * @param {number} [init.swr]
     * Override the caching behavior of this request to use the given `stale-while-revalidate` time,
     * in seconds
     *
     * @param {string} [init.surrogateKey]
     * Override the caching behavior of this request to include the given surrogate key, provided as
     * a header value.
     *
     * See the [Fastly surrogate keys guide](https://docs.fastly.com/en/guides/purging-api-cache-with-surrogate-keys)
     * for details.
     * 
     * @param {boolean} [init.pci]
     * Override the caching behavior of this request to enable or disable PCI/HIPAA-compliant
     * non-volatile caching.
     *
     * By default, this is false, which means the request may not be PCI/HIPAA-compliant. Set it to
     * true to enable compliant caching.
     *
     * See the [Fastly PCI-Compliant Caching and Delivery documentation](https://docs.fastly.com/products/pci-compliant-caching-and-delivery)
     * for details.
     */
    constructor(
      mode: "none" | "pass" | "override",
      init?: {
        ttl?: number,
        swr?: number,
        surrogateKey?: string,
        pci?: boolean
      }
    );

    /**
     * Sets the cache override mode for a request
     * 
     * If set to:
     * - "none": Do not override the behavior specified in the origin response’s cache control headers.
     * - "pass": Do not cache the response to this request, regardless of the origin response’s headers.
     * - "override": Override particular cache control settings using a {@linkcode CacheOverride} object.
     */
    public mode: "none" | "pass" | "override";
    /**
     * Override the caching behavior of this request to use the given Time to Live (TTL), in seconds.
     */
    public ttl?: number;
    /**
      * Override the caching behavior of this request to use the given `stale-while-revalidate` time,
     * in seconds
     */
    public swr?: number;
    /**
     * Override the caching behavior of this request to include the given surrogate key, provided as
     * a header value.
     *
     * See the [Fastly surrogate keys guide](https://docs.fastly.com/en/guides/purging-api-cache-with-surrogate-keys)
     * for details.
     */
    public surrogateKey?: string;
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
    public pci?: boolean;
  }
}
