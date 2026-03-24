/// <reference path="../types/secret-store.d.ts" />
/// <reference path="../types/globals.d.ts" />

declare module 'fastly:backend' {
  /**
   * Set the default backend configuration options for dynamic backends.
   *
   * Applies to backends created via {@link Backend | new Backend(...)} as well as for dynamic backends
   * implicitly created when using {@link fetch | fetch()}.
   *
   * @param defaultDynamicBackendConfiguration default backend configuration options
   * @version 3.24.0 
   */
  export function setDefaultDynamicBackendConfig(
    defaultDynamicBackendConfiguration: DefaultBackendConfiguration,
  ): void;

  /**
   * Call this function to enforce the security property of explicitly-defined backends, even
   * when dynamic backends are enabled at the Fastly service level.
   *
   * By default, if dynamic backends are supported for the Fastly service, they will be automatically
   * used when creating a new `fetch()` request. This default behaviour for dynamic backends can be a
   * potential security concern since third-party JavaScript code may send arbitrary requests,
   * including sensitive/secret data, off to destinations that the JavaScript project was not
   * intending.
   *
   * When calling this function, an optional default backend name can be provided.
   *
   * @note
   * This is a separate option to the service-level dynamic backend support for Fastly services.
   * By default, dynamic backends are disabled for Fastly Services, so that even if not using this
   * function, the service-level security configuration will apply.
   *
   * @param defaultBackend the name of the default backend to use, when using {@link fetch | fetch()}.
   *
   * @version 3.24.0
   * @experimental
   */
  export function enforceExplicitBackends(defaultBackend?: string): void;

  interface DefaultBackendConfiguration {
    /**
     * Maximum duration in milliseconds to wait for a connection to this backend to be established.
     * If exceeded, the connection is aborted and a 503 response will be presented instead.
     * @defaultValue 1_000
     * @throws {RangeError} Throws a RangeError if the value is negative or greater than or equal to 2^32
     */
    connectTimeout?: number;
    /**
     * Maximum duration in milliseconds to wait for the server response to begin after a TCP connection is established and the request has been sent.
     * If exceeded, the connection is aborted and a 503 response will be presented instead.
     * @defaultValue 15_000
     * @throws {RangeError} Throws a RangeError if the value is negative or greater than or equal to 2^32
     */
    firstByteTimeout?: number;
    /**
     * Maximum duration in milliseconds that Fastly will wait while receiving no data on a download from a backend.
     * If exceeded, the response received so far will be considered complete and the fetch will end.
     * @defaultValue 10_000
     * @throws {RangeError} Throws a RangeError if the value is negative or greater than or equal to 2^32
     */
    betweenBytesTimeout?: number;
    /**
     * Whether or not to require TLS for connections to this backend.
     *
     * When using TLS, Fastly checks the validity of the backend's certificate, and fails the connection if the certificate is invalid.
     * This check is not optional: an invalid certificate will cause the backend connection to fail (but read on).
     *
     * By default, the validity check does not require that the certificate hostname matches the hostname of your request.
     * You can use {@link BackendConfiguration.certificateHostname} to request a check of the certificate hostname.
     *
     * By default, certificate validity uses a set of public certificate authorities.
     * You can specify an alternative CA using {@link caCertificate}.
     */
    useSSL?: boolean;
    /**
     * Determine whether or not connections to the same backend should be pooled across different sessions.
     * Fastly considers two backends “the same” if they’re registered with the same name and the exact same settings.
     * In those cases, when pooling is enabled, if Session 1 opens a connection to this backend it will be left open, and can be re-used by Session 2.
     * This can help improve backend latency, by removing the need for the initial network / TLS handshake(s).
     * By default, pooling is enabled for dynamic backends.
     * @version 3.1.0
     */
    dontPool?: boolean;
    /**
     * Minimum allowed TLS version on SSL connections to this backend.
     * If the backend server is not able to negotiate a connection meeting this constraint, a 503 response will be presented instead.
     *
     * @throws {RangeError} Throws a RangeError if the value is not 1, 1.1, 1.2, or 1.3
     */
    tlsMinVersion?: 1 | 1.1 | 1.2 | 1.3;
    /**
     * Maximum allowed TLS version on SSL connections to this backend.
     * If the backend server is not able to negotiate a connection meeting this constraint, a 503 response will be presented instead.
     *
     * @throws {RangeError} Throws a RangeError if the value is not 1, 1.1, 1.2, or 1.3
     */
    tlsMaxVersion?: 1 | 1.1 | 1.2 | 1.3;
    /**
     * Define the hostname that the server certificate should declare.
     *
     * If not set (default), the server certificate may present any hostname.
     *
     * @throws {TypeError} Throws a TypeError if the value is an empty string.
     */
    certificateHostname?: string;
    /**
     * The CA certificate to use when checking the validity of the backend.
     *
     * If not provided (default), the backend's certificate is validated using a set of public root CAs.
     *
     * @throws {TypeError} Throws a TypeError if the value is an empty string.
     */
    caCertificate?: string;
    /**
     * List of OpenSSL ciphers to support for connections to this origin.
     * If the backend server is not able to negotiate a connection meeting this constraint, a 503 response will be presented instead.
     *
     * [List of ciphers supported by Fastly](https://developer.fastly.com/learning/concepts/routing-traffic-to-fastly/#use-a-tls-configuration).
     *
     * @throws {TypeError} Throws a TypeError if the value is an empty string.
     */
    ciphers?: string;
    /**
     * Set the client certificate to be provided to the server during the initial TLS handshake.
     *
     * @throws {TypeError} Throws a TypeError if the value is not an object of the correct type.
     *
     * @version 3.15.0
     */
    clientCertificate?: {
      certificate: string;
      key: import('fastly:secret-store').SecretStoreEntry;
    };

    /**
     * Enables and sets the HTTP keepalive time in milliseconds for the backend.
     *
     * @version 3.24.0
     */
    httpKeepalive?: number;

    /**
     * Enables and sets the TCP keep alive options for the backend.
     * Setting to boolean true enables keepalive with the default options.
     *
     * @version 3.24.0
     */
    tcpKeepalive?:
      | boolean
      | {
          /**
           * Configure how long to wait after the last sent data over the TCP connection before
           * starting to send TCP keepalive probes.
           */
          timeSecs?: number;

          /**
           * Configure how long to wait between each TCP keepalive probe sent to the backend to determine if it is still active.
           */
          intervalSecs?: number;

          /**
           * Number of probes to send to the backend before it is considered dead.
           */
          probes?: number;
        };
  }

  interface BackendConfiguration extends DefaultBackendConfiguration {
    /**
     * The name of the backend.
     * The name has to be between 1 and 254 characters inclusive.
     * The name can be whatever you would like, as long as it does not match the name of any of the static service backends nor match any other dynamic backends built during a single execution of the application.
     *
     * If no name is provided, an autogenerated internal name will be used.
     *
     * @throws {TypeError} Throws a TypeError if the backend already exists or the name value is not valid, e.g., an empty string or a string with more than 254 characters.
     *
     * @deprecated to avoid name collisions it is recommended to use auto-generated names by omitting this property.
     */
    name?: string;
    /**
     * A hostname, IPv4, or IPv6 address for the backend as well as an optional port.
     * If set, the target has to be at least 1 character.
     *
     * @example hostname
     * "example.com"
     * @example hostname and port
     * "example.com:443"
     * @example ip address
     * "1.2.3.4"
     * @example ip address and port
     * "1.2.3.4:8080"
     *
     * @throws {TypeError} Throws a TypeError if the value is not valid, i.e. is null, undefined, an empty string, not a valid IP address or host, or is the string `::`
     */
    target: string;
    /**
     * If set, will force the HTTP Host header on connections to this backend to be the supplied value.
     *
     * @example
     * "example.com:443"
     *
     * @throws {TypeError} Throws a TypeError if the value is an empty string.
     */
    hostOverride?: string;

    /**
     * The SNI hostname to use on connections to this backend.
     *
     * @throws {TypeError} Throws a TypeError if the value is an empty string.
     */
    sniHostname?: string;
    /**
     * @experimental
     *
     * When enabled, sets that this backend is to be used for gRPC traffic.
     *
     * Warning: When using this experimental feature, no guarantees are provided for behaviours for
     * backends that do not provide gRPC traffic.
     * @version 3.23.0
     */
    grpc?: boolean;
  }

  /**
   * Class for dynamically creating new [Fastly Backends](https://developer.fastly.com/reference/api/services/backend/).
   *
   * To disable the usage of dynamic backends, see {@link enforceExplicitBackends}.
   *
   * @example
   * In this example an explicit Dynamic Backend is created and supplied to the fetch request, the response is then returned to the client.
   * ```js
   * /// <reference types="@fastly/js-compute" />
   * import { Backend } from "fastly:backend";
   * async function app() {
   *   // For any request, return the fastly homepage -- without defining a backend!
   *   const backend = new Backend({
   *     name: 'fastly',
   *     target: 'fastly.com',
   *     hostOverride: "www.fastly.com",
   *     connectTimeout: 1000,
   *     firstByteTimeout: 15000,
   *     betweenBytesTimeout: 10000,
   *     useSSL: true,
   *     tlsMinVersion: 1.3,
   *     tlsMaxVersion: 1.3,
   *   });
   *   return fetch('https://www.fastly.com/', {
   *     backend // Here we are configuring this request to use the backend from above.
   *   });
   * }
   * addEventListener("fetch", event => event.respondWith(app(event)));
   * ```
   * @fiddle meta
   * {
   *   "title": "Explicit Dynamic Backend Example",
   *   "request": "/status=200"
   * }
   */
  class Backend {
    /**
     * Creates a new Backend instance
     *
     * @example
     * Construct a new backend with all properties set to their default values:
     * ```js
     * const myBackend = new Backend({ name: 'fastly', target: 'www.fastly.com'});
     * ```
     */
    constructor(configuration: BackendConfiguration);

    /**
     * The name of the backend
     * @version 3.24.0
     */
    readonly name: string;

    /**
     * Whether this backend was dynamically created by the running service.
     * @version 3.24.0 
     */
    readonly isDynamic: boolean;
    /**
     * The host target for the backend
     * @version 3.24.0
     */
    readonly target: string;
    /**
     * The host header override defined for the backend.
     *
     * See https://docs.fastly.com/en/guides/specifying-an-override-host for more information.
     * @version 3.24.0
     */
    readonly hostOverride: string;
    /**
     * The backend port
     * @version 3.24.0
     */
    readonly port: number;
    /**
     * The connect timeout for the backend in milliseconds, if available.
     * @version 3.24.0
     */
    readonly connectTimeout: number | null;
    /**
     * The first byte timeout for the backend in milliseconds, if available.
     * @version 3.24.0
     */
    readonly firstByteTimeout: number | null;
    /**
     * The between bytes timeout for the backend in milliseconds, if available.
     * @version 3.24.0
     */
    readonly betweenBytesTimeout: number | null;
    /**
     * The HTTP keepalive time for the backend in milliseconds.
     * @version 3.24.0
     */
    readonly httpKeepaliveTime: number;
    /**
     * The TCP keepalive configuration, if TCP keepalive is enabled.
     * @version 3.24.0
     */
    readonly tcpKeepalive: null | {
      /**
       * The keepalive time in seconds.
       */
      timeSecs: number;
      /**
       * The interval in seconds between probes.
       */
      intervalSecs: number;
      /**
       * The number of probes to send before terminating the keepalive.
       */
      probes: number;
    };
    /**
     * Whether the backend is configured to use SSL.
     * @version 3.24.0
     */
    readonly isSSL: boolean;
    /**
     * The minimum SSL version number this backend will use, if available.
     * @version 3.24.0
     */
    readonly tlsMinVersion: 1 | 1.1 | 1.2 | 1.3 | null;
    /**
     * The maximum SSL version number this backend will use, if available.
     * @version 3.24.0
     */
    readonly tlsMaxversion: 1 | 1.1 | 1.2 | 1.3 | null;

    /**
     * Get the health of this backend.
     * @version 3.24.0
     */
    health(): 'healthy' | 'unhealthy' | 'unknown';

    /**
     * Returns the name associated with the Backend instance.
     * @deprecated Use `backend.name` instead.
     * @version 3.7.0
     */
    toName(): string;

    /**
     * Returns the name associated with the Backend instance.
     *
     * @deprecated Use `backend.name` instead.
     */
    toString(): string;

    /**
     * Returns a boolean indicating if a Backend with the given name exists or not.
     * @version 3.7.0
     */
    static exists(name: string): boolean;

    /**
     * Returns the Backend instance with the given name, if one exists. If one does not exist, an error is thrown.
     * @version 3.7.0
     */ 
    static fromName(name: string): Backend;

    /**
     * Returns a string representing the health of the given Backend instance.
     * Possible values are:
     *
     * "healthy" - The backend's health check has succeeded, indicating the backend is working as expected and should receive requests.
     * "unhealthy" - The backend's health check has failed, indicating the backend is not working as expected and should not receive requests.
     * "unknown" - The backend does not have a health check configured.
     *
     * @deprecated Use `backend.health()` ({@link Backend.prototype.health}) instead.
     * @version 3.7.0 
     */
    static health(backend: Backend): 'healthy' | 'unhealthy' | 'unknown';
  }
}
