/// <reference path="../types/secret-store.d.ts" />

declare module 'fastly:backend' {
  export function setDefaultDynamicBackendConfig(
    defaultDynamicBackendConfiguration: DefaultBackendConfiguration,
  ): void;

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
     */
    useSSL?: boolean;
    /**
     * Determine whether or not connections to the same backend should be pooled across different sessions.
     * Fastly considers two backends “the same” if they’re registered with the same name and the exact same settings.
     * In those cases, when pooling is enabled, if Session 1 opens a connection to this backend it will be left open, and can be re-used by Session 2.
     * This can help improve backend latency, by removing the need for the initial network / TLS handshake(s).
     * By default, pooling is enabled for dynamic backends.
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
     * The CA certificate to use when checking the validity of the backend.
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
     */
    clientCertificate?: {
      certificate: string;
      key: import('fastly:secret-store').SecretStoreEntry;
    };

    /**
     * Enables and sets the HTTP keepalive time in milliseconds for the backend.
     */
    httpKeepalive?: number;

    /**
     * Enables and sets the TCP keep alive options for the backend.
     * Setting to boolean true enables keepalive with the default options.
     */
    tcpKeepalive?: boolean | {
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
     * @throws {TypeError} Throws a TypeError if the value is not valid. I.E. The value is null, undefined, an empty string or a string with more than 254 characters.
     */
    name: string;
    /**
     * A hostname, IPv4, or IPv6 address for the backend as well as an optional port.
     * If set, the target has to be at-least 1 character.
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
     * @throws {TypeError} Throws a TypeError if the value is not valid. I.E. Is null, undefined, an empty string, not a valid IP address or host, or is the string `::`
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
     * Define the hostname that the server certificate should declare.
     *
     * @throws {TypeError} Throws a TypeError if the value is an empty string.
     */
    certificateHostname?: string;
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
     */
    grpc?: boolean;
  }

  /**
   * Class for dynamically creating new [Fastly Backends](https://developer.fastly.com/reference/api/services/backend/).
   *
   * @note
   * This feature is in disabled by default for Fastly Services. Please contact [Fastly Support](https://support.fastly.com/hc/en-us/requests/new?ticket_form_id=360000269711) to request the feature be enabled on the Fastly Services which require Dynamic Backends.
   *
   * By default, Dynamic Backends are disabled within a JavaScript application as it can be a potential
   * avenue for third-party JavaScript code to send requests, potentially including sensitive/secret data,
   * off to destinations that the JavaScript project was not intending, which could be a security issue.
   *
   * To enable Dynamic Backends the application will need to explicitly allow Dynamic Backends via:
   * ```js
   * import { allowDynamicBackends } from "fastly:experimental";
   * allowDynamicBackends(true);
   * ```
   *
   * **Note**: Can only be used when processing requests, not during build-time initialization.
   *
   * @example
   * <script async defer src="https://fiddle.fastly.dev/embed.js"></script>
   * In this example an implicit Dynamic Backend is created when making the fetch request to https://www.fastly.com/ and the response is then returned to the client.
   *
   * <script type="application/json+fiddle">
   * {
   *   "type": "javascript",
   *   "title": "Implicit Dynamic Backend Example",
   *   "origins": [
   *     "https://http-me.glitch.me"
   *   ],
   *   "src": {
   *     "deps": "{\n  \"@fastly/js-compute\": \"^0.7.0\"\n}",
   *     "main": "/// <reference types=\"@fastly/js-compute\" />\nimport { allowDynamicBackends } from \"fastly:experimental\";\nallowDynamicBackends(true);\nasync function app() {\n  // For any request, return the fastly homepage -- without defining a backend!\n  return fetch('https://www.fastly.com/');\n}\naddEventListener(\"fetch\", event => event.respondWith(app(event)));\n"
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
   * import { allowDynamicBackends } from "fastly:experimental";
   * allowDynamicBackends(true);
   * async function app() {
   *   // For any request, return the fastly homepage -- without defining a backend!
   *   return fetch('https://www.fastly.com/');
   * }
   * addEventListener("fetch", event => event.respondWith(app(event)));
   * ```
   * </noscript>
   *
   * @example
   * In this example an explicit Dynamic Backend is created and supplied to the fetch request, the response is then returned to the client.
   *
   * <script type="application/json+fiddle">
   * {
   *   "type": "javascript",
   *   "title": "Explicit Dynamic Backend Example",
   *   "origins": [
   *     "https://http-me.glitch.me"
   *   ],
   *   "src": {
   *     "deps": "{\n  \"@fastly/js-compute\": \"^0.7.0\"\n}",
   *     "main": "/// <reference types=\"@fastly/js-compute\" />\nimport { allowDynamicBackends } from \"fastly:experimental\";\nimport { Backend } from \"fastly:backend\";\nallowDynamicBackends(true);\nasync function app() {\n  // For any request, return the fastly homepage -- without defining a backend!\n  const backend = new Backend({\n    name: 'fastly',\n    target: 'fastly.com',\n    hostOverride: \"www.fastly.com\",\n    connectTimeout: 1000,\n    firstByteTimeout: 15000,\n    betweenBytesTimeout: 10000,\n    useSSL: true,\n    sslMinVersion: 1.3,\n    sslMaxVersion: 1.3,\n  });\n  return fetch('https://www.fastly.com/', {\n    backend // Here we are configuring this request to use the backend from above.\n  });\n}\naddEventListener(\"fetch\", event => event.respondWith(app(event)));\n"
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
   * import { allowDynamicBackends } from "fastly:experimental";
   * import { Backend } from "fastly:backend";
   * allowDynamicBackends(true);
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
   *     sslMinVersion: 1.3,
   *     sslMaxVersion: 1.3,
   *   });
   *   return fetch('https://www.fastly.com/', {
   *     backend // Here we are configuring this request to use the backend from above.
   *   });
   * }
   * addEventListener("fetch", event => event.respondWith(app(event)));
   * ```
   * </noscript>
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
     * Whether this backend was dynamically created by the running service.
     */
    readonly isDynamic: boolean;
    /**
     * The host target for the backend
     */
    readonly target: string;
    /**
     * The host header override defined for the backend.
     * 
     * See https://docs.fastly.com/en/guides/specifying-an-override-host for more information.
     */
    readonly hostOverride: string;
    /**
     * The backend port
     */
    readonly port: number;
    /**
     * The connect timeout for the backend in milliseconds, if available.
     */
    readonly connectTimeout: number | null;
    /**
     * The first byte timeout for the backend in milliseconds, if available.
     */
    readonly firstByteTimeout: number | null;
    /**
     * The between bytes timeout for the backend in milliseconds, if available.
     */
    readonly betweenBytesTimeout: number | null;
    /**
     * The HTTP keepalive time for the backend in milliseconds.
     */
    readonly httpKeepaliveTime: number;
    /**
     * The TCP keepalive configuration, if TCP keepalive is enabled.
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
     */
    readonly isSSL: boolean;
    /**
     * The minimum SSL version number this backend will use, if available.
     */
    readonly tlsMinVersion: 1 | 1.1 | 1.2 | 1.3 | null;
    /**
     * The maximum SSL version number this backend will use, if available.
     */
    readonly tlsMaxversion: 1 | 1.1 | 1.2 | 1.3 | null;

    /**
     * Get the health of this backend.
     */
    health(): 'healthy' | 'unhealthy' | 'unknown';

    /**
     * Returns the name associated with the Backend instance.
     */
    toName(): string;

    /**
     * Returns the name of the Backend, which can be used on {@link "globals".RequestInit.backend}
     */
    toString(): string;

    /**
     * Returns a boolean indicating if a Backend with the given name exists or not.
     */
    static exists(name: string): boolean;

    /**
     * Returns the Backend instance with the given name, if one exists. If one does not exist, an error is thrown.
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
     */
    static health(backend: Backend): 'healthy' | 'unhealthy' | 'unknown';
  }
}
