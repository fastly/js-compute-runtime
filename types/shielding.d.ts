/// <reference path="../types/backend.d.ts" />

declare module 'fastly:shielding' {
  /**
   * Configuration options for shield backends returned by
   * {@link Shield.encryptedBackend} and {@link Shield.unencryptedBackend}.
   */
  interface ShieldBackendConfiguration {
    /**
     * First byte timeout for the returned backend, in milliseconds.
     */
    firstByteTimeout?: number;
  }

  /**
   * Represents a [Fastly shield](https://www.fastly.com/documentation/guides/concepts/shielding/#shield-locations)
   * location, used to route requests through a designated shield POP.
   *
   * **Note**: Can only be used when processing requests, not during build-time initialization.
   *
   * @example
   * ```js
   * /// <reference types="@fastly/js-compute" />
   *
   * import { Shield } from "fastly:shielding";
   *
   * async function app(event) {
   *     const shield = new Shield('wsi-australia-au');
   *     // If running on the shield POP, fetch from the origin directly
   *     if (shield.runningOn()) {
   *         return await fetch('https://my-site.com/anything', { backend: 'my-backend' });
   *     }
   *     // Otherwise, route the request through the shield using an encrypted connection
   *     return await fetch(event.request, { backend: shield.encryptedBackend() });
   * }
   *
   * addEventListener("fetch", (event) => event.respondWith(app(event)))
   * ```
   *
   * @version 3.37.0
   */
  export class Shield {
    /**
     * Load information about the given shield.
     *
     * Shield names are shield codes as listed in the
     * [shield locations](https://www.fastly.com/documentation/guides/concepts/shielding/#shield-locations)
     * documentation (e.g. `"pdx-or-us"` for Portland, OR, USA or `"paris-fr"` for Paris, France).
     *
     * @param name The shield code identifying the shield POP
     * @throws Throws an `Error` if no shield exists with the provided name
     */
    constructor(name: string);

    /**
     * Returns whether the code is currently running on (or effectively on) the given shield POP.
     *
     * This may also return `true` when Fastly is routing traffic from the target shield POP to the
     * current POP for load balancing or performance reasons, since the result would be approximately
     * identical to running on the target shield itself.
     */
    runningOn(): boolean;

    /**
     * Return a {@link backend!Backend | Backend} representing an unencrypted
     * connection to this shield POP. Prefer {@link encryptedBackend} unless the data is already
     * encrypted, as data sent over this backend travels unencrypted over the open internet.
     *
     * @param configuration Optional backend configuration
     */
    unencryptedBackend(
      configuration?: ShieldBackendConfiguration,
    ): import('fastly:backend').Backend;

    /**
     * Return a {@link backend!Backend | Backend} representing an encrypted
     * connection to this shield POP. This is almost always the backend you want to use.
     *
     * @param configuration Optional backend configuration
     */
    encryptedBackend(
      configuration?: ShieldBackendConfiguration,
    ): import('fastly:backend').Backend;
  }
}
