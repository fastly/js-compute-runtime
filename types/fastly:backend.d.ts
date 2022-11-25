declare module 'fastly:backend' {
  interface BackendConfiguration {
    /**
     * The name of the backend.
     */
    name: string;
    /**
     * A hostname, IPv4, or IPv6 address for the backend as well as an optional port.
     * E.G. "hostname", "ip address", "hostname:port", or "ip address:port"
     */
    target: string;
    /**
     * If set, will force the HTTP Host header on connections to this backend to be the supplied value.
     */
    hostOverride?: string;
    /**
     * Maximum duration in milliseconds to wait for a connection to this backend to be established.
     * If exceeded, the connection is aborted and a 503 response will be presented instead.
     * Defaults to 1,000 milliseconds.
     */
    connectTimeout?: number;
    /**
     * Maximum duration in milliseconds to wait for the server response to begin after a TCP connection is established and the request has been sent.
     * If exceeded, the connection is aborted and a 503 response will be presented instead.
     * Defaults to 15,000 milliseconds.
     */
    firstByteTimeout?: number;
    /**
     * Maximum duration in milliseconds that Fastly will wait while receiving no data on a download from a backend.
     * If exceeded, the response received so far will be considered complete and the fetch will end.
     * Defaults to 10,000 milliseconds.
     */
    betweenBytesTimeout?: number;
    /**
     * Whether or not to require TLS for connections to this backend.
     */
    useSSL?: boolean;
    /**
     * Minimum allowed TLS version on SSL connections to this backend.
     * If the backend server is not able to negotiate a connection meeting this constraint, a 503 response will be presented instead.
     */
    tlsMinVersion?: number;
    /**
     * Maximum allowed TLS version on SSL connections to this backend.
     * If the backend server is not able to negotiate a connection meeting this constraint, a 503 response will be presented instead.
     */
    tlsMaxVersion?: number;
    /**
     * Define the hostname that the server certificate should declare.
     */
    certificateHostname?: string;
    /**
     * Whether or not to validate the server certificate.
     * Highly recommended to enable this if `useSSL` is set to `true`.
     */
    checkCertificate?: boolean;
    /**
     * The CA certificate to use when checking the validity of the backend.
     */
    caCertificate?: string;
    /**
     * List of OpenSSL ciphers to support for connections to this origin.
     * If the backend server is not able to negotiate a connection meeting this constraint, a 503 response will be presented instead.
     */
    ciphers?: string;
    /**
     * The SNI hostname to use on connections to this backend.
     */
    sniHostname?: string;
  }

  /**
   * Class for creating new [Fastly Backends](https://developer.fastly.com/reference/api/services/backend/).
   *
   * **Note**: Can only be used when processing requests, not during build-time initialization.
   *
   * @example
   * Construct a new backend with all properties set to their default values:
   * ```js
   * const myBackend = new Backend({ name: 'fastly', target: 'www.fastly.com'});
   * ```
   */
  class Backend {
    /**
     * Creates a new Backend object
     */
    constructor(configuration: BackendConfiguration);
    /**
     * Returns the name of the Backend, which can be used on {@link RequestInit.backend}
     */
    toString(): string;
  }
}
