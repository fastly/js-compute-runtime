declare var self: typeof globalThis;
interface EventMap {
  "fetch": FetchEvent;
}

interface EventListenerMap {
  "fetch": FetchEventListener;
}

interface FetchEventListener {
  (this: typeof globalThis, event: FetchEvent): any
}

declare var onfetch: FetchEventListener;

/**
 * This is a fetch specific implementation of [addEventListener](https://developer.mozilla.org/en-US/docs/Web/API/EventTarget/addEventListener), and is very similar to [handling FetchEvent from a Service Worker](https://developer.mozilla.org/en-US/docs/Web/API/FetchEvent/request).
 *
 * For Fastly C@E, this will be the entrypoint in handling your downstream request from your client.
 */
declare function addEventListener<K extends keyof EventMap>(type: K, listener: EventListenerMap[K]): void;

/**
 * @deprecated This has moved to {@link "fastly:backend".BackendConfiguration} - This global variable will be removed in the next major version.
 * @hidden
 */
declare interface BackendConfiguration {
  /**
   * The name of the backend.
   */
  name: string,
  /**
   * A hostname, IPv4, or IPv6 address for the backend as well as an optional port.
   * E.G. "hostname", "ip address", "hostname:port", or "ip address:port"
   */
  target: string,
  /**
   * If set, will force the HTTP Host header on connections to this backend to be the supplied value.
   */
  hostOverride?: string,
  /**
   * Maximum duration in milliseconds to wait for a connection to this backend to be established.
   * If exceeded, the connection is aborted and a 503 response will be presented instead.
   * Defaults to 1,000 milliseconds.
   */
  connectTimeout?: number,
  /**
   * Maximum duration in milliseconds to wait for the server response to begin after a TCP connection is established and the request has been sent.
   * If exceeded, the connection is aborted and a 503 response will be presented instead.
   * Defaults to 15,000 milliseconds.
   */
  firstByteTimeout?: number,
  /**
   * Maximum duration in milliseconds that Fastly will wait while receiving no data on a download from a backend.
   * If exceeded, the response received so far will be considered complete and the fetch will end. 
   * Defaults to 10,000 milliseconds.
   */
  betweenBytesTimeout?: number,
  /**
   * Whether or not to require TLS for connections to this backend.
   */
  useSSL?: boolean,
  /**
   * Minimum allowed TLS version on SSL connections to this backend.
   * If the backend server is not able to negotiate a connection meeting this constraint, a 503 response will be presented instead.
   */
  tlsMinVersion?: number,
  /**
   * Maximum allowed TLS version on SSL connections to this backend.
   * If the backend server is not able to negotiate a connection meeting this constraint, a 503 response will be presented instead.
   */
  tlsMaxVersion?: number,
  /**
   * Define the hostname that the server certificate should declare.
   */
  certificateHostname?: string,
  /**
   * Whether or not to validate the server certificate.
   * Highly recommended to enable this if `useSSL` is set to `true`.
   */
  checkCertificate?: boolean,
  /**
   * The CA certificate to use when checking the validity of the backend.
   */
  caCertificate?: string,
  /**
   * List of OpenSSL ciphers to support for connections to this origin. 
   * If the backend server is not able to negotiate a connection meeting this constraint, a 503 response will be presented instead.
   */
  ciphers?: string,
  /**
   * The SNI hostname to use on connections to this backend.
   */
  sniHostname?: string,
}

/**
* Class for creating new [Fastly Backends](https://developer.fastly.com/reference/api/services/backend/).
*
* **Note**: Can only be used when processing requests, not during build-time initialization.
* @deprecated This has moved to {@link "fastly:backend".Backend} - This global variable will be removed in the next major version.
@hidden
*/
declare class Backend {
  /**
   * Creates a new Backend object
   */
  constructor(configuration: BackendConfiguration);
  /**
   * Returns the name of the Backend, which can be used on {@link RequestInit.backend}
   */
  toString(): string;
}

/**
 * A Fastly C@E specific implementation of [FetchEvent](https://developer.mozilla.org/en-US/docs/Web/API/FetchEvent/FetchEvent).
 */
declare interface FetchEvent {
  /**
   * Information about the downstream client that made the request
   */
  readonly client: ClientInfo;
  /**
   * The downstream request that came from the client
   */
  readonly request: Request;

  /**
   * Send a response back to the client.
   *
   * **Note**: The service will be kept alive until the response has been fully sent.
   *
   * If the response contains a streaming body created by the service itself, then the service
   * will be kept alive until the body {@link ReadableStream} has been closed or errored.
   *
   * However, if the body is a stream originating in a request to a backend, i.e. if a backend
   * response's {@link Response.body} is passed as input to the {@link Response} constructor, the
   * service will not be kept alive until sending the body has finished.
   *
   * **Note**: If `response` is a `Promise`, the service will be kept alive until the
   * response has been resolved or rejected, and the {@link Response} it resolved to has been
   * fully sent.
   *
   * **Note**: Use {@link FetchEvent.waitUntil} to extend the service's lifetime further if
   * necessary.
   *
   * @param response - Response to send back down to the client
   */
  respondWith(response: Response | PromiseLike<Response>): void;

  /**
   * Extend the service's lifetime to ensure asynchronous operations succeed.
   *
   * By default, a service will shut down as soon as the response passed to
   * {@link FetchEvent.respondWith | respondWith} has been sent. `waitUntil` can be used to
   * ensure that the service will stay alive until additional asynchronous operations have
   * completed, such as sending telemetry data to a separate backend after the response has
   * been sent.
   *
   * @param promise - The `Promise` to wait for
   */
  waitUntil(promise: Promise<any>): void;
}

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
 * 
 * @deprecated This has moved to {@link "fastly:cache-override".CacheOverrideMode} - This global type will be removed in the next major version.
 * @hidden
 */
declare type CacheOverrideMode = "none" | "pass" | "override";

/**
 * Base class for Cache Override, which is used to configure caching behavior.
 * @deprecated This has moved to {@link "fastly:cache-override".CacheOverrideInit} - This global interface will be removed in the next major version.
 * @hidden
 */
declare interface CacheOverrideInit {
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
 * 
 * @deprecated This has moved to {@link "fastly:cache-override".CacheOverride} - This global interface will be removed in the next major version.
 * @hidden
 */
declare interface CacheOverride extends CacheOverrideInit {
  mode: CacheOverrideMode;
}

/**
 * @deprecated This has moved to {@link "fastly:cache-override".CacheOverride} - This global variable will be removed in the next major version.
 * @hidden
 */
declare var CacheOverride: {
  prototype: CacheOverride;
  new(mode: CacheOverrideMode, init?: CacheOverrideInit): CacheOverride;
};

/**
 * Information about the downstream client making the request to the C@E service.
 */
declare interface ClientInfo {
  /**
   * A string representation of the IPv4 or IPv6 address of the downstream client.
   */
  readonly address: string;
  readonly geo: import('fastly:geolocation').Geolocation;
}

/**
 * Class for accessing [Fastly Edge Dictionaries](https://docs.fastly.com/en/guides/about-edge-dictionaries).
 *
 * **Note**: Can only be used when processing requests, not during build-time initialization.
 *
 * @deprecated This has moved to {@link "fastly:config-store".ConfigStore} - This global class will be removed in the next major version.
 * @hidden
 */
declare class ConfigStore {
  /**
   * Creates a new ConfigStore object
   */
  constructor(name: string);
  /**
   * Get a value for a key in the config-store.
   */
  get(key: string): string;
}


/**
 * Class for accessing [Fastly Edge Dictionaries](https://docs.fastly.com/en/guides/about-edge-dictionaries).
 *
 * **Note**: Can only be used when processing requests, not during build-time initialization.
 * @deprecated This has moved to {@link "fastly:dictionary".Dictionary} - This global class will be removed in the next major version.
 * @hidden
 */
declare class Dictionary {
  /**
   * Creates a new Dictionary object, and opens an edge dictionary to query
   */
  constructor(name: string);
  /**
   * Get a value for a key in the dictionary
   */
  get(key: string): string;
}

/**
 * [Fastly Geolocation](https://developer.fastly.com/reference/vcl/variables/geolocation/)
 * information about an IP address
 *
 * Can be retrieved for the incoming request's client IP address using the
 * {@linkcode ClientInfo#geo} accessor, and for arbitrary addresses using
 * {@linkcode Fastly.getGeolocationForIpAddress}.
 * @deprecated This has moved to {@link "fastly:geolocation".Geolocation} - This global interface will be removed in the next major version.
 * @hidden
 */
declare interface Geolocation {
  /**
   * The name of the organization associated with as_number.
   *
   * For example, fastly is the value given for IP addresses under AS-54113.
   */
  as_name: string | null;

  /**
   * [Autonomous system](https://en.wikipedia.org/wiki/Autonomous_system_(Internet)) (AS) number.
   */
  as_number: number | null;

  /**
   * The telephone area code associated with an IP address.
   *
   * These are only available for IP addresses in the United States, its territories, and Canada.
   */
  area_code: number | null;

  /**
   * City or town name.
   */
  city: string | null;

  /**
   * Connection speed.
   */
  conn_speed: string | null;

  /**
   * Connection type.
   */
  conn_type: string | null;

  /**
   * Continent.
   */
  continent: string | null;

  /**
   * A two-character [ISO 3166-1](https://en.wikipedia.org/wiki/ISO_3166-1) country code for the country associated with an IP address.
   *
   * The US country code is returned for IP addresses associated with overseas United States military bases.
   *
   * These values include subdivisions that are assigned their own country codes in ISO 3166-1. For example, subdivisions NO-21 and NO-22 are presented with the country code SJ for Svalbard and the Jan Mayen Islands.
   */
  country_code: string | null;

  /**
   * A three-character [ISO 3166-1 alpha-3](https://en.wikipedia.org/wiki/ISO_3166-1_alpha-3) country code for the country associated with the IP address.
   *
   * The USA country code is returned for IP addresses associated with overseas United States military bases.
   */
  country_code3: string | null;

  /**
   * Country name.
   *
   * This field is the [ISO 3166-1](https://en.wikipedia.org/wiki/ISO_3166-1) English short name for a country.
   */
  country_name: string | null;

  /**
   * Time zone offset from Greenwich Mean Time (GMT) for `city`.
   */
  gmt_offset: string | null;

  /**
   * Latitude, in units of degrees from the equator.
   *
   * Values range from -90.0 to +90.0 inclusive, and are based on the [WGS 84](https://en.wikipedia.org/wiki/World_Geodetic_System) coordinate reference system.
   */
  latitude: number | null;

  /**
   * Longitude, in units of degrees from the [IERS Reference Meridian](https://en.wikipedia.org/wiki/IERS_Reference_Meridian).
   *
   * Values range from -180.0 to +180.0 inclusive, and are based on the [WGS 84](https://en.wikipedia.org/wiki/World_Geodetic_System) coordinate reference system.
   */
  longitude: number | null;

  /**
   * Metro code, representing designated market areas (DMAs) in the United States.
   */
  metro_code: number | null;

  /**
   * The postal code associated with the IP address.
   *
   * These are available for some IP addresses in Australia, Canada, France, Germany, Italy, Spain, Switzerland, the United Kingdom, and the United States.
   *
   * For Canadian postal codes, this is the first 3 characters. For the United Kingdom, this is the first 2-4 characters (outward code). For countries with alphanumeric postal codes, this field is a lowercase transliteration.
   */
  postal_code: string | null;

  /**
   * Client proxy description.
   */
  proxy_description: string | null;

  /**
   * Client proxy type.
   */
  proxy_type: string | null;

  /**
   * [ISO 3166-2](https://en.wikipedia.org/wiki/ISO_3166-2) country subdivision code.
   *
   * For countries with multiple levels of subdivision (for example, nations within the United Kingdom), this variable gives the more specific subdivision.
   *
   * This field can be None for countries that do not have ISO country subdivision codes. For example, None is given for IP addresses assigned to the Åland Islands (country code AX, illustrated below).
   */
  region: string | null;

  /**
   * Time zone offset from coordinated universal time (UTC) for `city`.
   */
  utc_offset: number | null;
}

/**
 * Class for accessing a [Fastly Object-store](https://developer.fastly.com/reference/api/object-store/).
 *
 * An object store is a persistent, globally consistent key-value store.
 * 
 * **Note**: Can only be used when processing requests, not during build-time initialization.
 * @deprecated This has moved to {@link "fastly:object-store".ObjectStore} - This global class will be removed in the next major version.
 * @hidden
 */
declare class ObjectStore {
  /**
   * Creates a new JavaScript ObjectStore object which interacts with the Fastly Object-store named `name`.
   * 
   * @param name Name of the Fastly Object-store to interact with. A name cannot be empty, contain Control characters, or be longer than 255 characters.
   */
  constructor(name: string);
  /**
   * Gets the value associated with the key `key` in the Object-store.
   * When the key is present, a resolved Promise containing an ObjectStoreEntry will be returned which contains the associated value.
   * When the key is absent, a resolved Promise containing null is returned.
   * @param key The key to retrieve from within the Object-store. A key cannot:
   * - Be any of the strings "", ".", or ".."
   * - Start with the string ".well-known/acme-challenge/""
   * - Contain any of the characters "#?*[]\n\r"
   * - Be longer than 1024 characters
   */
  get(key: string): Promise<ObjectStoreEntry | null>;

  /**
   * Write the value of `value` into the Object-store under the key `key`.
   * 
   * Note: Object-store is eventually consistent, this means that the updated contents associated with the key `key` may not be available to read from all
   * edge locations immediately and some edge locations may continue returning the previous contents associated with the key.
   * 
   * @param key The key to associate with the value. A key cannot:
   * - Be any of the strings "", ".", or ".."
   * - Start with the string ".well-known/acme-challenge/""
   * - Contain any of the characters "#?*[]\n\r"
   * - Be longer than 1024 characters
   * @param value The value to store within the Object-store.
   */
  put(key: string, value: BodyInit): Promise<undefined>;
}

/**
 * Class for interacting with a [Fastly Object-store](https://developer.fastly.com/reference/api/object-store/) entry.
 * 
 * @deprecated This has moved to {@link "fastly:object-store".ObjectStoreEntry} - This global interface will be removed in the next major version.
 * @hidden
 */
declare interface ObjectStoreEntry {
  /**
   * A ReadableStream with the contents of the entry. 
   */
  get body(): ReadableStream;
  /**
   * A boolean value that indicates whether the body has been read from already.
   */
  get bodyUsed(): boolean;
  /**
   * Reads the body and returns it as a promise that resolves with a string. 
   * The response is always decoded using UTF-8. 
   */
  text(): Promise<string>;
  /**
   * Reads the body and returns it as a promise that resolves with the result of parsing the body text as JSON.
   */
  json(): Promise<object>;
  /**
   * Reads the body and returns it as a promise that resolves with an ArrayBuffer. 
   */
  arrayBuffer(): Promise<ArrayBuffer>;
  // And eventually formData and blob once we support them on Request and Response, too.
}

/**
 * The URL class as [specified by WHATWG](https://url.spec.whatwg.org/#url-class)
 *
 * @see {@link https://developer.mozilla.org/en-US/docs/Web/API/URL | URL on MDN}
 */
declare class URL {
  constructor(url: string, base?: string | URL);

  get href(): string;
  set href(V: string);

  get origin(): string;

  get protocol(): string;
  set protocol(V: string);

  get username(): string;
  set username(V: string);

  get password(): string;
  set password(V: string);

  get host(): string;
  set host(V: string);

  get hostname(): string;
  set hostname(V: string);

  get port(): string;
  set port(V: string);

  get pathname(): string;
  set pathname(V: string);

  get search(): string;
  set search(V: string);

  get searchParams(): URLSearchParams;

  get hash(): string;
  set hash(V: string);

  toJSON(): string;

  readonly [Symbol.toStringTag]: "URL";
}

/**
 * The URLSearchParams class as [specified by WHATWG](https://url.spec.whatwg.org/#interface-urlsearchparams)
 *
 * @see {@link https://developer.mozilla.org/en-US/docs/Web/API/URLSearchParams | URLSearchParams on MDN}
 */

declare class URLSearchParams {
  constructor(
    init?:
      | ReadonlyArray<readonly [name: string, value: string]>
      | Iterable<readonly [name: string, value: string]>
      | { readonly [name: string]: string }
      | string
  );

  append(name: string, value: string): void;
  delete(name: string): void;
  get(name: string): string | null;
  getAll(name: string): string[];
  has(name: string): boolean;
  set(name: string, value: string): void;
  sort(): void;

  keys(): IterableIterator<string>;
  values(): IterableIterator<string>;
  entries(): IterableIterator<[name: string, value: string]>;
  forEach<THIS_ARG = void>(
    callback: (
      this: THIS_ARG,
      value: string,
      name: string,
      searchParams: this
    ) => void,
    thisArg?: THIS_ARG
  ): void;

  readonly [Symbol.toStringTag]: "URLSearchParams";
  [Symbol.iterator](): IterableIterator<[name: string, value: string]>;
}

/**
 * Interface for logging to stdout for
 * [live log monitoring](https://developer.fastly.com/learning/compute/testing/#live-log-monitoring-in-your-console).
 *
 * **Note**: This implementation accepts any number of arguments. String representations of each object are appended
 * together in the order listed and output. Unlike the `Console` built-in in browsers and Node.js, this implementation
 * does not perform string substitution.
 *
 * **Note**: Messages are prefixed with the respective log level, starting with an upper-case letter, e.g. `"Log: "`.
 */
declare interface Console {
  log(...objects: any[]): void;
  debug(...objects: any[]): void;
  info(...objects: any[]): void;
  warn(...objects: any[]): void;
  error(...objects: any[]): void;
}

/**
 * The global {@linkcode Console} instance
 */
declare var console: Console;

/**
 * TextEncoder takes a stream of code points as input and emits a stream of UTF-8 bytes
 *
 * @see {@link https://developer.mozilla.org/en-US/docs/Web/API/TextEncoder | TextEncoder on MDN}
 */
declare class TextEncoder {
  constructor();
  encode(input?: string): Uint8Array;
  get encoding(): string;
}

/**
 * TextDecoder takes a stream UTF-8 bytes as input and emits a stream of code points
 *
 * @see {@link https://developer.mozilla.org/en-US/docs/Web/API/TextDecoder | TextDecoder on MDN}
 *
 * **Note**: On Compute@Edge, TextDecoder only supports UTF-8 bytes as input, and always operates
 * in non-fatal mode.
 */
declare class TextDecoder {
  constructor();
  decode(input?: ArrayBuffer | ArrayBufferView): string;
  get encoding(): string;
}

/**
 * Simple interface for logging to
 * [third party logging providers](https://developer.fastly.com/learning/integrations/logging)
 *
 * Instances of Logger for specific endpoints can be created using {@linkcode Fastly.getLogger}.
 * @deprecated This has moved to {@link "fastly:logger".Logger} - This global class will be removed in the next major version.
 * @hidden
 */
declare interface Logger {
  /**
   * Send the given message, converted to a string, to this Logger instance's endpoint
   */
  log(message: any): void;
}

/**
 * Fastly-specific APIs available to Compute@Edge JS services
 */
declare interface Fastly {
  set baseURL(base: URL | null | undefined);
  get baseURL(): URL | null;
  set defaultBackend(backend: string);
  get defaultBackend(): string;
  /**
   * Property to access the environment variables for the C@E service.
   * @hidden
   */
  env: {
    /**
     * Function to get the environment variable value, for the provided environment variable name.
     *
     * For additional references, see the [Fastly Developer Hub for C@E Environment Variables](https://developer.fastly.com/reference/compute/ecp-env/)
     *
     * @param name The name of the environment variable
     * @returns the value of the environemnt variable
     * @deprecated This has moved to {@link "fastly:env".env} - This function will be removed in the next major version.
     * @hidden
     */
    get(name: string): string;
  };

  /**
   * Creates a new {@linkcode Logger} instance for the given
   * [named log endpoint](https://developer.fastly.com/learning/integrations/logging).
   *
   * **Note**: Can only be used when processing requests, not during build-time initialization.
   * @deprecated This function will be removed in the next major version. Use of this function can be replaced with the fastly logger class {@link "fastly:logger".Logger} 
   * @hidden
   */
  getLogger(endpoint: string): Logger;

  /**
   * Causes the Compute@Edge JS runtime environment to log debug information to stdout.
   *
   * **Note**: This is mostly for internal debugging purposes and will generate highly unstable
   * output.
   *
   * @experimental
   */
  enableDebugLogging(enabled: boolean): void;

  /**
   * Retrieve geolocation information about the given IP address.
   *
   * @param address - the IPv4 or IPv6 address to query
   *
   * **Note**: Can only be used when processing requests, not during build-time initialization.
   */
  getGeolocationForIpAddress(address: string): import('fastly:geolocation').Geolocation;

  /**
   * Embed a file as a Uint8Array.
   *
   * @param path - The path to include, relative to the project's top-level directory
   *
   * **Note**: Can only be used during build-time initialization, not when processing requests.
   *
   * @experimental
   */
  includeBytes(path: string): Uint8Array;
}

/**
 * The global instance of the {@linkcode Fastly} builtin
 */
declare var fastly: Fastly;

type CompressionStreamFormat = "deflate" | "deflate-raw" | "gzip"

interface CompressionStream {
  readonly readable: ReadableStream<Uint8Array>;
  readonly writable: WritableStream<Uint8Array>;
}

declare var CompressionStream: {
  prototype: CompressionStream;
  new(format: CompressionStreamFormat): CompressionStream;
};

type DecompressionStreamFormat = "deflate" | "deflate-raw" | "gzip"

interface DecompressionStream {
  readonly readable: ReadableStream<Uint8Array>;
  readonly writable: WritableStream<Uint8Array>;
}

declare var DecompressionStream: {
  prototype: DecompressionStream;
  new(format: DecompressionStreamFormat): DecompressionStream;
};

// Note: the contents below here are, partially modified, copies of content from TypeScript's
// `lib.dom.d.ts` file.
// We used to keep them in a separate file, referenced using a `/// reference path="..."`
// directive, but that causes the defined types to be absent from TypeDoc output.

/*! *****************************************************************************
Copyright (c) Microsoft Corporation. All rights reserved.
Licensed under the Apache License, Version 2.0 (the "License"); you may not use
this file except in compliance with the License. You may obtain a copy of the
License at http://www.apache.org/licenses/LICENSE-2.0

THIS CODE IS PROVIDED ON AN *AS IS* BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION ANY IMPLIED
WARRANTIES OR CONDITIONS OF TITLE, FITNESS FOR A PARTICULAR PURPOSE,
MERCHANTABLITY OR NON-INFRINGEMENT.

See the Apache Version 2.0 License for specific language governing permissions
and limitations under the License.
***************************************************************************** */

/*!
 * This file is largely a subset of TypeScript's lib.dom.d.ts file, with some
 * C@E-specific additions and modifications. Those modifications are
 * Copyright (c) Fastly Corporation, under the same license as the rest of the file.
 */

/**
 * Used within the
 * [Request](https://developer.mozilla.org/en-US/docs/Web/API/Request/Request) and
 * [Response](https://developer.mozilla.org/en-US/docs/Web/API/Response/Response) constructors.
 * ({@linkcode Request}, and {@linkcode Response})
 */
declare type BodyInit = ReadableStream | ArrayBufferView | ArrayBuffer | URLSearchParams | string;

/**
 * Body for Fetch HTTP Requests and Responses
 *
 * @see {@link https://developer.mozilla.org/en-US/docs/Web/API/Fetch_API/Using_Fetch#Body | Body on MDN}
 */
declare interface Body {
  readonly body: ReadableStream<Uint8Array> | null;
  readonly bodyUsed: boolean;
  arrayBuffer(): Promise<ArrayBuffer>;
  // blob(): Promise<Blob>;
  // formData(): Promise<FormData>;
  json(): Promise<any>;
  text(): Promise<string>;
}

/**
 * Constructor parameter for
 * [Request](https://developer.mozilla.org/en-US/docs/Web/API/Request)
 *
 * Usually this a URL to the resource you are requesting.
 */
declare type RequestInfo = Request | string;

/**
 * Constructor parameter for
 * [Request](https://developer.mozilla.org/en-US/docs/Web/API/Request)
 *
 * This contains information to send along with the request (Headers, body, etc...), as well as
 * Fastly specific information.
 */
declare interface RequestInit {
  /** A BodyInit object or null to set request's body. */
  body?: BodyInit | null;
  // /** A string indicating how the request will interact with the browser's cache to set request's cache. */
  // cache?: RequestCache;
  // /** A string indicating whether credentials will be sent with the request always, never, or only when sent to a same-origin URL. Sets request's credentials. */
  // credentials?: RequestCredentials;
  /** A Headers object, an object literal, or an array of two-item arrays to set request's headers. */
  headers?: HeadersInit;
  // /** A cryptographic hash of the resource to be fetched by request. Sets request's integrity. */
  // integrity?: string;
  // /** A boolean to set request's keepalive. */
  // keepalive?: boolean;
  /** A string to set request's method. */
  method?: string;
  // /** A string to indicate whether the request will use CORS, or will be restricted to same-origin URLs. Sets request's mode. */
  // mode?: RequestMode;
  // /** A string indicating whether request follows redirects, results in an error upon encountering a redirect, or returns the redirect (in an opaque fashion). Sets request's redirect. */
  // redirect?: RequestRedirect;
  // /** A string whose value is a same-origin URL, "about:client", or the empty string, to set request's referrer. */
  // referrer?: string;
  // /** A referrer policy to set request's referrerPolicy. */
  // referrerPolicy?: ReferrerPolicy;
  // /** An AbortSignal to set request's signal. */
  // signal?: AbortSignal | null;
  // /** Can only be null. Used to disassociate request from any Window. */
  // window?: null;

  /** The Fastly configured backend the request should be sent to. */
  backend?: string;
  cacheOverride?: import('fastly:cache-override').CacheOverride;
  cacheKey?: string;
}

/**
 * The Request class as [specified by WHATWG](https://fetch.spec.whatwg.org/#ref-for-dom-request%E2%91%A0)
 *
 * **Note**: Can only be used when processing requests, not during build-time initialization.
 *
 * @see {@link https://developer.mozilla.org/en-US/docs/Web/API/Request | Request on MDN}
 */
interface Request extends Body {
  // /** Returns the cache mode associated with request, which is a string indicating how the request will interact with the browser's cache when fetching. */
  // readonly cache: RequestCache;
  // /** Returns the credentials mode associated with request, which is a string indicating whether credentials will be sent with the request always, never, or only when sent to a same-origin URL. */
  // readonly credentials: RequestCredentials;
  // /** Returns the kind of resource requested by request, e.g., "document" or "script". */
  // readonly destination: RequestDestination;
  /** Returns a Headers object consisting of the headers associated with request. */
  readonly headers: Headers;
  // /** Returns request's subresource integrity metadata, which is a cryptographic hash of the resource being fetched. Its value consists of multiple hashes separated by whitespace. [SRI] */
  // readonly integrity: string;
  // /** Returns a boolean indicating whether or not request can outlive the global in which it was created. */
  // readonly keepalive: boolean;
  /** Returns request's HTTP method, which is "GET" by default. */
  readonly method: string;
  // /** Returns the mode associated with request, which is a string indicating whether the request will use CORS, or will be restricted to same-origin URLs. */
  // readonly mode: RequestMode;
  // /** Returns the redirect mode associated with request, which is a string indicating how redirects for the request will be handled during fetching. A request will follow redirects by default. */
  // readonly redirect: RequestRedirect;
  // /** Returns the referrer of request. Its value can be a same-origin URL if explicitly set in init, the empty string to indicate no referrer, and "about:client" when defaulting to the global's default. This is used during fetching to determine the value of the `Referer` header of the request being made. */
  // readonly referrer: string;
  // /** Returns the referrer policy associated with request. This is used during fetching to compute the value of the request's referrer. */
  // readonly referrerPolicy: ReferrerPolicy;
  // /** Returns the signal associated with request, which is an AbortSignal object indicating whether or not request has been aborted, and its abort event handler. */
  // readonly signal: AbortSignal;
  /** Returns the URL of request as a string. */
  readonly url: string;
  // clone(): Request;

  // Fastly extensions
  backend: string;
  setCacheOverride(override: import('fastly:cache-override').CacheOverride): void;
  setCacheKey(key: string): void;
}

declare var Request: {
  prototype: Request;
  new(input: RequestInfo | URL, init?: RequestInit): Request;
};

/**
 * Constructor parameter for the [Fetch API Response](https://developer.mozilla.org/en-US/docs/Web/API/Response)
 * This contains information to send along with the response.
 */
declare interface ResponseInit {
  headers?: HeadersInit;
  status?: number;
  statusText?: string;
}

/**
 * The Response class as [specified by WHATWG](https://fetch.spec.whatwg.org/#ref-for-dom-response%E2%91%A0)
 *
 * **Note**: Can only be used when processing requests, not during build-time initialization.
 *
 * @see {@link https://developer.mozilla.org/en-US/docs/Web/API/Response | Response on MDN}
 */
interface Response extends Body {
  readonly headers: Headers;
  readonly ok: boolean;
  // readonly redirected: boolean;
  readonly status: number;
  readonly statusText: string;
  // readonly type: ResponseType;
  readonly url: string;
  // clone(): Response;
}

declare var Response: {
  prototype: Response;
  new(body?: BodyInit | null, init?: ResponseInit): Response;
  // error(): Response;
  // redirect(url: string | URL, status?: number): Response;
};

type ReadableStreamReader<T> = ReadableStreamDefaultReader<T>;
// type ReadableStreamReader<T> = ReadableStreamDefaultReader<T> | ReadableStreamBYOBReader;
type ReadableStreamController<T> = ReadableStreamDefaultController<T>;
// type ReadableStreamController<T> = ReadableStreamDefaultController<T> | ReadableByteStreamController;

interface UnderlyingSinkAbortCallback {
  (reason?: any): void | PromiseLike<void>;
}

interface UnderlyingSinkCloseCallback {
  (): void | PromiseLike<void>;
}

interface UnderlyingSinkStartCallback {
  (controller: WritableStreamDefaultController): any;
}

interface UnderlyingSinkWriteCallback<W> {
  (chunk: W, controller: WritableStreamDefaultController): void | PromiseLike<void>;
}

interface UnderlyingSourceCancelCallback {
  (reason?: any): void | PromiseLike<void>;
}

interface UnderlyingSourcePullCallback<R> {
  (controller: ReadableStreamController<R>): void | PromiseLike<void>;
}

interface UnderlyingSourceStartCallback<R> {
  (controller: ReadableStreamController<R>): any;
}

interface UnderlyingSink<W = any> {
  abort?: UnderlyingSinkAbortCallback;
  close?: UnderlyingSinkCloseCallback;
  start?: UnderlyingSinkStartCallback;
  type?: undefined;
  write?: UnderlyingSinkWriteCallback<W>;
}

interface UnderlyingSource<R = any> {
  autoAllocateChunkSize?: number;
  cancel?: UnderlyingSourceCancelCallback;
  pull?: UnderlyingSourcePullCallback<R>;
  start?: UnderlyingSourceStartCallback<R>;
  type?: ReadableStreamType;
}

type ReadableStreamType = "bytes";

interface StreamPipeOptions {
  preventAbort?: boolean;
  preventCancel?: boolean;
  /**
   * Pipes this readable stream to a given writable stream destination. The way in which the piping process behaves under various error conditions can be customized with a number of passed options. It returns a promise that fulfills when the piping process completes successfully, or rejects if any errors were encountered.
   *
   * Piping a stream will lock it for the duration of the pipe, preventing any other consumer from acquiring a reader.
   *
   * Errors and closures of the source and destination streams propagate as follows:
   *
   * An error in this source readable stream will abort destination, unless preventAbort is truthy. The returned promise will be rejected with the source's error, or with any error that occurs during aborting the destination.
   *
   * An error in destination will cancel this source readable stream, unless preventCancel is truthy. The returned promise will be rejected with the destination's error, or with any error that occurs during canceling the source.
   *
   * When this source readable stream closes, destination will be closed, unless preventClose is truthy. The returned promise will be fulfilled once this process completes, unless an error is encountered while closing the destination, in which case it will be rejected with that error.
   *
   * If destination starts out closed or closing, this source readable stream will be canceled, unless preventCancel is true. The returned promise will be rejected with an error indicating piping to a closed stream failed, or with any error that occurs during canceling the source.
   *
   * The signal option can be set to an AbortSignal to allow aborting an ongoing pipe operation via the corresponding AbortController. In this case, this source readable stream will be canceled, and destination aborted, unless the respective options preventCancel or preventAbort are set.
   */
  preventClose?: boolean;
  // signal?: AbortSignal;
}

interface QueuingStrategySize<T = any> {
  (chunk: T): number;
}

interface QueuingStrategy<T = any> {
  highWaterMark?: number;
  size?: QueuingStrategySize<T>;
}

interface QueuingStrategyInit {
  /**
   * Creates a new ByteLengthQueuingStrategy with the provided high water mark.
   *
   * Note that the provided high water mark will not be validated ahead of time. Instead, if it is negative, NaN, or not a number, the resulting ByteLengthQueuingStrategy will cause the corresponding stream constructor to throw.
   */
  highWaterMark: number;
}

interface ReadableStreamDefaultReadDoneResult {
  done: true;
  value?: undefined;
}

interface ReadableStreamDefaultReadValueResult<T> {
  done: false;
  value: T;
}

type ReadableStreamDefaultReadResult<T> = ReadableStreamDefaultReadValueResult<T> | ReadableStreamDefaultReadDoneResult;

interface ReadableWritablePair<R = any, W = any> {
  readable: ReadableStream<R>;
  /**
   * Provides a convenient, chainable way of piping this readable stream through a transform stream (or any other \{ writable, readable \} pair). It simply pipes the stream into the writable side of the supplied pair, and returns the readable side for further use.
   *
   * Piping a stream will lock it for the duration of the pipe, preventing any other consumer from acquiring a reader.
   */
  writable: WritableStream<W>;
}

/** This Streams API interface represents a readable stream of byte data. The Fetch API offers a concrete instance of a ReadableStream through the body property of a Response object. */
interface ReadableStream<R = any> {
  readonly locked: boolean;
  cancel(reason?: any): Promise<void>;
  getReader(): ReadableStreamDefaultReader<R>;
  pipeThrough<T>(transform: ReadableWritablePair<T, R>, options?: StreamPipeOptions): ReadableStream<T>;
  pipeTo(dest: WritableStream<R>, options?: StreamPipeOptions): Promise<void>;
  tee(): [ReadableStream<R>, ReadableStream<R>];
}

/**
 * The ReadableStream class as [specified by WHATWG](https://streams.spec.whatwg.org/#rs-class)
 *
 * @see {@link https://developer.mozilla.org/en-US/docs/Web/API/ReadableStream | ReadableStream on MDN}
 */
declare var ReadableStream: {
  prototype: ReadableStream;
  new <R = any>(underlyingSource?: UnderlyingSource<R>, strategy?: QueuingStrategy<R>): ReadableStream<R>;
};

interface ReadableStreamDefaultController<R = any> {
  readonly desiredSize: number | null;
  close(): void;
  enqueue(chunk: R): void;
  error(e?: any): void;
}

declare var ReadableStreamDefaultController: {
  prototype: ReadableStreamDefaultController;
  new(): ReadableStreamDefaultController;
};

interface ReadableStreamDefaultReader<R = any> extends ReadableStreamGenericReader {
  read(): Promise<ReadableStreamDefaultReadResult<R>>;
  releaseLock(): void;
}

declare var ReadableStreamDefaultReader: {
  prototype: ReadableStreamDefaultReader;
  new <R = any>(stream: ReadableStream<R>): ReadableStreamDefaultReader<R>;
};

interface ReadableStreamGenericReader {
  readonly closed: Promise<undefined>;
  cancel(reason?: any): Promise<void>;
}

/** This Streams API interface provides a standard abstraction for writing streaming data to a destination, known as a sink. This object comes with built-in backpressure and queuing. */
interface WritableStream<W = any> {
  readonly locked: boolean;
  abort(reason?: any): Promise<void>;
  getWriter(): WritableStreamDefaultWriter<W>;
}

/**
 * The WritableStream class as [specified by WHATWG](https://streams.spec.whatwg.org/#ws-class)
 *
 * @see {@link https://developer.mozilla.org/en-US/docs/Web/API/WritableStream | WritableStream on MDN}
 */
declare var WritableStream: {
  prototype: WritableStream;
  new <W = any>(underlyingSink?: UnderlyingSink<W>, strategy?: QueuingStrategy<W>): WritableStream<W>;
};

/** This Streams API interface represents a controller allowing control of a WritableStream's state. When constructing a WritableStream, the underlying sink is given a corresponding WritableStreamDefaultController instance to manipulate. */
interface WritableStreamDefaultController {
  error(e?: any): void;
}

declare var WritableStreamDefaultController: {
  prototype: WritableStreamDefaultController;
  new(): WritableStreamDefaultController;
};

/** This Streams API interface is the object returned by WritableStream.getWriter() and once created locks the < writer to the WritableStream ensuring that no other streams can write to the underlying sink. */
interface WritableStreamDefaultWriter<W = any> {
  readonly closed: Promise<undefined>;
  readonly desiredSize: number | null;
  readonly ready: Promise<undefined>;
  abort(reason?: any): Promise<void>;
  close(): Promise<void>;
  releaseLock(): void;
  write(chunk: W): Promise<void>;
}

declare var WritableStreamDefaultWriter: {
  prototype: WritableStreamDefaultWriter;
  new <W = any>(stream: WritableStream<W>): WritableStreamDefaultWriter<W>;
};

interface TransformStream<I = any, O = any> {
  readonly readable: ReadableStream<O>;
  readonly writable: WritableStream<I>;
}

/**
 * The TransformStream class as [specified by WHATWG](https://streams.spec.whatwg.org/#ts-class)
 *
 * @see {@link https://developer.mozilla.org/en-US/docs/Web/API/TransformStream | TransformStream on MDN}
 */

declare var TransformStream: {
  prototype: TransformStream;
  new <I = any, O = any>(transformer?: Transformer<I, O>, writableStrategy?: QueuingStrategy<I>, readableStrategy?: QueuingStrategy<O>): TransformStream<I, O>;
};

interface TransformStreamDefaultController<O = any> {
  readonly desiredSize: number | null;
  enqueue(chunk?: O): void;
  error(reason?: any): void;
  terminate(): void;
}

declare var TransformStreamDefaultController: {
  prototype: TransformStreamDefaultController;
  new(): TransformStreamDefaultController;
};

interface Transformer<I = any, O = any> {
  flush?: TransformerFlushCallback<O>;
  readableType?: undefined;
  start?: TransformerStartCallback<O>;
  transform?: TransformerTransformCallback<I, O>;
  writableType?: undefined;
}

interface TransformerFlushCallback<O> {
  (controller: TransformStreamDefaultController<O>): void | PromiseLike<void>;
}

interface TransformerStartCallback<O> {
  (controller: TransformStreamDefaultController<O>): void | PromiseLike<void>;
}

interface TransformerTransformCallback<I, O> {
  (chunk: I, controller: TransformStreamDefaultController<O>): void | PromiseLike<void>;
}

type HeadersInit = Headers | string[][] | Record<string, string>;

/**
 * The Headers class as [specified by WHATWG](https://fetch.spec.whatwg.org/#headers-class)
 *
 * @see {@link https://developer.mozilla.org/en-US/docs/Web/API/Headers | Headers on MDN}
 */
interface Headers {
  append(name: string, value: string): void;
  delete(name: string): void;
  get(name: string): string | null;
  has(name: string): boolean;
  set(name: string, value: string): void;
  forEach(callbackfn: (value: string, key: string, parent: Headers) => void, thisArg?: any): void;
  // Iterable methods
  entries(): IterableIterator<[string, string]>;
  keys(): IterableIterator<string>;
  values(): IterableIterator<[string]>;
  [Symbol.iterator](): Iterator<[string, string]>;
}

declare var Headers: {
  prototype: Headers;
  new(init?: HeadersInit): Headers;
};

/**
 * The atob() function decodes a string of data which has been encoded using Base64 encoding.
 * 
 * @param data A binary string (i.e., a string in which each character in the string is treated as a byte of binary data) containing base64-encoded data.
 * @returns An ASCII string containing decoded data from `data`.
 * 
 * @see {@link https://developer.mozilla.org/en-US/docs/Web/API/atob | atob on MDN}
 */
declare function atob(data: string): string;

/**
 *  The btoa() method creates a Base64-encoded ASCII string from a binary string (i.e., a string in which each character in the string is treated as a byte of binary data). 
 * @param data The binary string to encode.
 * @returns  An ASCII string containing the Base64 representation of `data`. 
 * 
 * @see {@link https://developer.mozilla.org/en-US/docs/Web/API/btoa | btoa on MDN}
 */
declare function btoa(data: string): string;

/**
 * The setTimeout() method sets a timer which calls a function once the timer expires.
 *
 * @param callback A function to be called after the timer expires.
 * @param delay The time, in milliseconds, that the timer should wait before calling the specified function. Defaults to 0 if not specified.
 * @param args Additional arguments which are passed through to the callback function.
 * @returns A numeric, non-zero value which identifies the timer created; this value can be passed to clearTimeout() to cancel the timeout.
 *
 * @see {@link https://developer.mozilla.org/en-US/docs/Web/API/setTimeout | setTimeout on MDN}
 */
declare function setTimeout<TArgs extends any[]>(callback: (...args: TArgs) => void, delay?: number, ...args: TArgs): number;

/**
 * The clearTimeout() method cancels a timeout previously established by calling setTimeout(). If the parameter provided does not identify a previously established action, this method does nothing.
 * @param timeoutID The identifier of the timeout you want to cancel. This ID was returned by the corresponding call to setTimeout().
 *
 * @see {@link https://developer.mozilla.org/en-US/docs/Web/API/clearTimeout | clearTimeout on MDN}
 */
declare function clearTimeout(timeoutID?: number): void;

/**
 * The setInterval() method repeatedly calls a function, with a fixed time delay between each call.
 * This method returns an interval ID which uniquely identifies the interval, so you can remove it later by calling clearInterval().
 *
 * @param callback A function to be called every delay milliseconds. The first call happens after delay milliseconds.
 * @param delay The time, in milliseconds, that the timer should delay in between calls of the specified function. Defaults to 0 if not specified.
 * @param args Additional arguments which are passed through to the callback function.
 * @returns A numeric, non-zero value which identifies the timer created; this value can be passed to clearInterval() to cancel the interval.
 *
 * @see {@link https://developer.mozilla.org/en-US/docs/Web/API/setInterval | setInterval on MDN}
 */
declare function setInterval<TArgs extends any[]>(callback: (...args: TArgs) => void, delay?: number, ...args: TArgs): number;

/**
 * The clearInterval() method cancels a timed, repeating action which was previously established by a call to setInterval(). If the parameter provided does not identify a previously established action, this method does nothing.
 * @param intervalID The identifier of the repeated action you want to cancel. This ID was returned by the corresponding call to setInterval().
 *
 * @see {@link https://developer.mozilla.org/en-US/docs/Web/API/clearInterval | clearInterval on MDN}
 */
declare function clearInterval(intervalID?: number): void;

/**
 * Fetch resources from backends.
 *
 * **Note**: Compute@Edge requires all outgoing requests to go to a predefined
 * {@link https://developer.fastly.com/reference/glossary#term-backend | backend}, passed in
 * via the {@link RequestInit.backend | backend} property on the `init` object.
 *
 * **Note**: Can only be used when processing requests, not during build-time initialization.
 *
 * @see {@link https://developer.mozilla.org/en-US/docs/Web/API/WindowOrWorkerGlobalScope/fetch | fetch on MDN}
 *
 * @param resource - The resource to fetch, either a URL string or a {@link Request} object
 * @param init - An object containing settings to apply to the request
 */
declare function fetch(input: RequestInfo, init?: RequestInit): Promise<Response>;

interface VoidFunction {
  (): void;
}

declare function queueMicrotask(callback: VoidFunction): void;

declare function structuredClone(value: any, options?: StructuredSerializeOptions): any;

interface StructuredSerializeOptions {
  transfer?: Transferable[];
}

type Transferable = ArrayBuffer;
// type Transferable = ArrayBuffer | MessagePort | ImageBitmap;

interface WorkerLocation {
  readonly hash: string;
  readonly host: string;
  readonly hostname: string;
  readonly href: string;
  toString(): string;
  readonly origin: string;
  readonly pathname: string;
  readonly port: string;
  readonly protocol: string;
  readonly search: string;
}

declare var WorkerLocation: {
  prototype: WorkerLocation;
  new(): WorkerLocation;
};

declare var location: WorkerLocation;

/** Basic cryptography features available in the current context. It allows access to a cryptographically strong random number generator and to cryptographic primitives. */
interface Crypto {
  // /** Available only in secure contexts. */
  // readonly subtle: SubtleCrypto;
  getRandomValues<T extends ArrayBufferView | null>(array: T): T;
  // /** Available only in secure contexts. */
  // randomUUID(): string;
}

declare var Crypto: {
  prototype: Crypto;
  new(): Crypto;
};

declare var crypto: Crypto;

