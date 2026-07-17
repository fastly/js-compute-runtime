declare module 'fastly:security' {
  /**
   * Inspect a request using the [Fastly Next-Gen WAF](https://docs.fastly.com/en/ngwaf/).
   *
   * **Note**: Can only be used when processing requests, not during build-time initialization.
   *
   * @param request The request to inspect.
   * @param config Configuration for the inspection.
   * @throws {TypeError} If `request` is not a Request instance.
   * @throws {TypeError} If `overrideClientIp` is not a valid IPv4 or IPv6 address.
   *
   * @example
   * ```js
   * /// <reference types="@fastly/js-compute" />
   *
   * import { inspect } from "fastly:security";
   *
   * async function app(event) {
   *   const res = inspect(event.request, {
   *     corp: "my-corp",
   *     workspace: "my-workspace"
   *   });
   *   switch (res.verdict) {
   *     case "allow":
   *       return await fetch(event.request);
   *     case "block":
   *       return new Response("Request Blocked", { status: 400 });
   *     case "unauthorized":
   *       return new Response("Unauthorized", { status: 401 });
   *     default:
   *       return new Response("Unexpected verdict", { status: 500 });
   *   }
   * }
   *
   * addEventListener("fetch", (event) => event.respondWith(app(event)));
   * ```
   *
   * @version 3.38.0
   */
  export function inspect(
    request: Request,
    config: InspectConfig,
  ): InspectResponse;

  /**
   * Configuration for {@link inspect}.
   *
   * @version 3.38.3
   */
  export interface InspectConfig {
    /**
     * Set a corp name for the configuration.
     *
     * This is currently required, but will be made optional in the future.
     */
    corp: string;
    /**
     * Set a workspace name for the configuration.
     *
     * This is currently required, but will be made optional in the future.
     */
    workspace: string;
    /**
     * Specify an explicit client IP address to inspect. Must be a valid IPv4 or IPv6 address.
     *
     * By default, `inspect` will use the IP address that made the request to the
     * running Compute service, but you may want to use a different IP when
     * service chaining or if requests are proxied from outside of Fastly's
     * network.
     */
    overrideClientIp?: string;
  }

  /**
   * Results of inspecting a request with the Fastly Next-Gen WAF.
   */
  export interface InspectResponse {
    /** WAF response status code. */
    waf_response: number;
    /** A redirect URL returned from the WAF, if applicable. */
    redirect_url?: string;
    /** Tags returned by the WAF. */
    tags: string[];
    /**
     * The WAF's verdict on how to handle this request.
     *
     * Known verdicts are defined in {@link InspectVerdict}, but other values
     * may be returned.
     */
    verdict: InspectVerdict | string;
    /** How long the WAF spent determining its verdict, in milliseconds. */
    decision_ms: number;
  }

  /**
   * Known verdict values returned by {@link inspect}.
   *
   * @version 3.38.0
   */
  export enum InspectVerdict {
    /** The request is allowed. */
    Allow = 'allow',
    /** The request should be blocked. */
    Block = 'block',
    /** This service is not authorized to inspect requests. */
    Unauthorized = 'unauthorized',
  }
}
