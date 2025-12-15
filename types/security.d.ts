declare module 'fastly:security' {
  /**
   * Inspect a Request using the [Fastly Next-Gen WAF](https://docs.fastly.com/en/ngwaf/).
   */
  export function inspect(request: Request): InspectResponse;

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
