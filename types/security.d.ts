declare module 'fastly:security' {
  /**
   * Inspect a Request using the [Fastly Next-Gen WAF](https://docs.fastly.com/en/ngwaf/).
   */
  export function inspect(
    request: Request,
    config: InspectConfig,
  ): InspectResponse;

  /**
   * Results of asking Security to inspect a Request.
   */
  export interface InspectResponse {
    /** Security status code. */
    waf_response: number;
    /** Security status code. */
    redirect_url?: string;
    /** Tags returned by Security. */
    tags: string[];
    /** Get Security’s verdict on how to handle this request. */
    verdict: InspectVerdict | string;
    /** How long Security spent determining its verdict, in milliseconds. */
    decision_ms: number;
  }

  export enum InspectVerdict {
    /** Security indicated that this request is allowed. */
    Allow = 'allow',
    /** Security indicated that this request should be blocked. */
    Block = 'block',
    /** Security indicated that this service is not authorized to inspect a request. */
    Unauthorized = 'unauthorized',
  }
}

/**
 * Configuration object for `inspect`.
 */
export interface InspectConfig {
  /**
   * Set a corp name for the configuration.
   *
   * This is currently required but will be made optional in the future.
   */
  corp: string;
  /**
   * Set a workspace name for the configuration.
   *
   * This is currently required but will be made optional in the future.
   */
  workspace: string;
  /**
   * Specify an explicit client IP address to inspect.
   *
   * By default, `inspect` will use the IP address that made the request to the
   * running Compute service, but you may want to use a different IP when
   * service chaining or if requests are proxied from outside of Fastly’s
   * network.
   */
  overrideClientIp?: string;
}
