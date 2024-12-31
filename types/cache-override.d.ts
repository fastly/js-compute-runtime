declare module 'fastly:cache-override' {
  /**
   * Cache customization options for responses, provided through the afterSend hook
   *
   * For customizing the response status, headers, and other cache options, these
   * can be modified directly on the response.
   */
  interface CacheOptions {
    /**
     * Whether to cache this response.
     *
     * By default, leaving this field empty, responses will be cached based on their cache header
     * information.
     *
     * Setting this to true or false will override this default cache behaviour, setting in the cache
     * or not setting in the cache, even if the default behaviour would have been otherwise.
     *
     * Setting to 'record-uncacheable' the response will not only not be cached, but the cache will
     * record that the originating request led to an uncacheable response, so that future cache lookups
     * will result in immediately going to the backend, rather than attempting to coordinate concurrent
     * requests to reduce backend traffic.
     *
     * See the [Fastly request collapsing guide](https://www.fastly.com/documentation/guides/concepts/edge-state/cache/request-collapsing/)
     * for more details on the mechanism that `recordUncacheable` disables.
     */
    cache?: boolean | 'uncacheable';
    /**
     * Get or set a [TransformStream](https://developer.mozilla.org/en-US/docs/Web/API/TransformStream) to be used for
     * transforming the response body prior to caching.
     *
     * Body transformations are performed by specifying a transform, rather than by directly working with the body
     * during the onAfterSend callback function, because not every response contains a fresh body:
     * 304 Not Modified responses, which are used to revalidate a stale cached response, are valuable precisely because
     * they do not retransmit the body.
     *
     * For any other response status, the backend response will contain a relevant body, and the `bodyTransform` will
     * be applied to it. The original backend body is piped to the [`writeable`](https://developer.mozilla.org/en-US/docs/Web/API/TransformStream/writable)
     * end of the transform, and transform is responsible for writing the new body, which will be read out from the
     * [`readable`](https://developer.mozilla.org/en-US/docs/Web/API/TransformStream/readable) end of the transform.
     * This setup allows the transform to work with streamed chunks of the backend body, rather
     * than necessarily reading it entirely into memory.
     */
    bodyTransform?: TransformStream<Uint8Array, Uint8Array>;
  }

  /**
   * Configures the caching behavior of a {@linkcode "globals".Response}.
   *
   * Normally, the HTTP Headers on a {@linkcode "globals".Response} would control how the {@linkcode "globals".Response} is cached,
   * but `CacheOverride` can be set on a {@linkcode "globals".Request}, to define custom caching behavior.
   *
   * @example
   * <script async defer src="https://fiddle.fastly.dev/embed.js"></script>
   * In this example we override the cache for all the requests prefixed /static/ to have a long TTL (Time To Live),
   * and the home page to have a short TTL and a long SWR (Stale While Revalidate).
   *
   * <script type="application/json+fiddle">
   * {
   *   "type": "javascript",
   *   "title": "CacheOverride Example",
   *   "origins": [
   *     "https://http-me.glitch.me"
   *   ],
   *   "src": {
   *     "deps": "{\n  \"@fastly/js-compute\": \"^0.7.0\"\n}",
   *     "main": "/// <reference types=\"@fastly/js-compute\" />\nimport { CacheOverride } from \"fastly:cache-override\";\n\n// In this example we override the cache for all the requests prefixed /static/ \n// to have a long TTL (Time To Live), and the home page to have a short TTL and \n// a long SWR (Stale While Revalidate).\nasync function app (event) {\n  const path = (new URL(event.request.url)).pathname;\n  let cacheOverride;\n  if (path == '/') {\n    cacheOverride = new CacheOverride('override', {ttl: 10, swr: 86_400});\n  } else if (path.startsWith('/static/')) {\n    cacheOverride = new CacheOverride('override', {ttl: 86_400});\n  } else {\n    cacheOverride = new CacheOverride('none')\n  }\n  return fetch(event.request.url, {\n    cacheOverride,\n    backend: 'origin_0'\n  });\n}\naddEventListener(\"fetch\", event => event.respondWith(app(event)));\n"
   *   },
   *   "requests": [
   *     {
   *       "enableCluster": true,
   *       "enableShield": false,
   *       "enableWAF": false,
   *       "method": "GET",
   *       "path": "/status=200",
   *       "useFreshCache": false,
   *       "followRedirects": false,
   *       "tests": "",
   *       "delay": 0
   *     }
   *   ],
   *   "srcVersion": 26
   * }
   * </script>
   * <noscript>
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
   *   return fetch(event.request.url, {
   *     cacheOverride,
   *     backend: 'origin_0'
   *   });
   * }
   * addEventListener("fetch", event => event.respondWith(app(event)));
   * ```
   * </noscript>
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
     *
     * @param {void} [init.beforeSend]
     * Set a [callback function](https://developer.mozilla.org/en-US/docs/Glossary/Callback_function) to be invoked if a
     * request is going all the way to a backend, allowing the request to be modified beforehand.
     *
     * See [Modifying a request as it is forwarded to a backend](https://www.fastly.com/documentation/guides/concepts/edge-state/cache/#modifying-a-request-as-it-is-forwarded-to-a-backend)
     * in the Fastly cache interfaces documentation for details.
     *
     * @param {void} [init.afterSend]
     * Set a [callback function](https://developer.mozilla.org/en-US/docs/Glossary/Callback_function) to be invoked after
     * a response has been sent, but before it is stored into the cache.
     *
     * See [Controlling cache behavior based on backend response](https://www.fastly.com/documentation/guides/concepts/edge-state/cache/#controlling-cache-behavior-based-on-backend-response)
     * in the Fastly cache interfaces documentation for details.
     */
    constructor(
      mode: 'none' | 'pass' | 'override',
      init?: {
        ttl?: number;
        swr?: number;
        surrogateKey?: string;
        pci?: boolean;
        beforeSend?: (request: Request) => void | PromiseLike<void>;
        afterSend?: (
          response: Response,
        ) => void | CacheOptions | PromiseLike<void | CacheOptions>;
      },
    );
    constructor(overrideInit?: {
      ttl?: number;
      swr?: number;
      surrogateKey?: string;
      pci?: boolean;
      beforeSend?: (request: Request) => void | PromiseLike<void>;
      afterSend?: (
        response: Response,
      ) => void | CacheOptions | PromiseLike<void | CacheOptions>;
    });

    /**
     * Sets the cache override mode for a request
     *
     * If set to:
     * - "none": Do not override the behavior specified in the origin response’s cache control headers.
     * - "pass": Do not cache the response to this request, regardless of the origin response’s headers.
     * - "override": Override particular cache control settings using a {@linkcode CacheOverride} object.
     */
    public mode: 'none' | 'pass' | 'override';
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

    /**
     * Callback to be invoked if a request is going all the way to a backend, allowing the request to be modified beforehand.
     *
     * See [Modifying a request as it is forwarded to a backend](https://www.fastly.com/documentation/guides/concepts/edge-state/cache/#modifying-a-request-as-it-is-forwarded-to-a-backend)
     * in the Fastly cache interfaces documentation for details.
     *
     */
    public beforeSend?: (request: Request) => void | PromiseLike<void>;

    /**
     * Callback to be invoked after a response has been sent, but before it is stored into the cache.
     *
     * See [Controlling cache behavior based on backend response](https://www.fastly.com/documentation/guides/concepts/edge-state/cache/#controlling-cache-behavior-based-on-backend-response)
     * in the Fastly cache interfaces documentation for details.
     */
    public afterSend?: (
      response: Response,
    ) => void | CacheOptions | PromiseLike<void | CacheOptions>;
  }
}
