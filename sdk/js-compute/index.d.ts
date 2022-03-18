/**
 * This is a fetch specific implementation of [addEventListener](https://developer.mozilla.org/en-US/docs/Web/API/EventTarget/addEventListener), and is very similar to [handling FetchEvent from a Service Worker](https://developer.mozilla.org/en-US/docs/Web/API/FetchEvent/request).
 *
 * For Fastly C@E, this will be the entrypoint in handling your downstream request from your client.
 */
declare function addEventListener(
  type: "fetch",
  listener: (event: FetchEvent) => void
);

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
  respondWith(response: Response | Promise<Response>): void;

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
 */
declare type CacheOverrideMode = "none" | "pass" | "override";

/**
 * Base class for Cache Override, which is used to configure caching behavior.
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
 */
declare interface CacheOverride extends CacheOverrideInit {
  mode: CacheOverrideMode;
}

declare var CacheOverride: {
  prototype: CacheOverride;
  new (mode: CacheOverrideMode, init?: CacheOverrideInit): CacheOverride;
};

/**
 * Information about the downstream client making the request to the C@E service.
 */
declare interface ClientInfo {
  /**
   * A string representation of the IPv4 or IPv6 address of the downstream client.
   */
  readonly address: string;
  readonly geo: Geolocation;
}

/**
 * Class for accessing [Fastly Edge Dictionaries](https://docs.fastly.com/en/guides/about-edge-dictionaries).
 *
 * **Note**: Can only be used when processing requests, not during build-time initialization.
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
 * {@linkcode fastly.getGeolocationForIpAddress}.
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
 * Class to handle environment variables for the C@E service.
 *
 * For additional references, see the [Fastly Developer Hub for C@E Environment Variables](https://developer.fastly.com/reference/compute/ecp-env/)
 */
declare class Env {
  constructor();

  /**
   * Function to get the environment variable value, for the provided environment variable name.
   *
   * @param name The name of the environment variable
   * @returns the value of the environemnt variable
   */
  get(name: string): string;
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
 * **Note**: relative to the `Console` builtin in browsers, this implementation is fairly basic for
 * now. It only supports a single argument, and does a basic `toString` operation on it. Use e.g.
 * `JSON.stringify` on the input to retain more information on internal object structure.
 *
 * **Note**: Messages are prefixed with the respective logel level, starting with an upper-case letter, e.g. `"Log: "`.
 */
declare interface Console {
  log(message: any);
  trace(message: any);
  info(message: any);
  warn(message: any);
  error(message: any);
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
 * Instances of Logger for specific endpoints can be created using {@linkcode fastly.getLogger}.
 */
declare interface Logger {
  /**
   * Send the given message, converted to a string, to this Logger instance's endpoint
   */
  log(message: any);
}

/**
 * Fastly-specific APIs available to Compute@Edge JS services
 */
declare interface Fastly {
  /**
   * Property to access the environment variables for the C@E service.
   */
  env: Env;

  /**
   * Creates a new {@linkcode Logger} instance for the given
   * [named log endpoint](https://developer.fastly.com/learning/integrations/logging).
   *
   * **Note**: Can only be used when processing requests, not during build-time initialization.
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
  enableDebugLogging(enabled: boolean);

  /**
   * Retrieve geolocation information about the given IP address.
   *
   * @param address - the IPv4 or IPv6 address to query
   *
   * **Note**: Can only be used when processing requests, not during build-time initialization.
   */
  getGeolocationForIpAddress(address: string): Geolocation;

  /**
   * Embed a file as a Uint8Array.
   *
   * @param path - The path to include, relative to the project's top-level directory
   *
   * **Note**: Can only be used during build-time initialization, not when processing requests.
   *
   * @experimental
   */
  includeBytes(path: String): Uint8Array;
}

/**
 * The global instance of the {@linkcode Fastly} builtin
 */
declare var fastly: Fastly;

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
 * Constructor parameter for the
 * [Fetch API Body](https://developer.mozilla.org/en-US/docs/Web/API/Body) and its implementations
 * ({@linkcode Request}, and {@linkcode Response})
 */
declare type BodyInit = ArrayBufferView | ArrayBuffer | ReadableStream | string | URLSearchParams | null;

/**
 * Body for Fetch HTTP Requests and Responses
 *
 * @see {@link https://developer.mozilla.org/en-US/docs/Web/API/Fetch_API/Using_Fetch#Body | Body on MDN}
 */
declare interface Body {
  readonly body: ReadableStream<Uint8Array> | null;
  readonly bodyUsed: boolean;

  arrayBuffer(): Promise<ArrayBuffer>;
  json(): Promise<any>;
  text(): Promise<string>;
}

/**
 * Constructor parameter for the
 * [Fetch API Request](https://developer.mozilla.org/en-US/docs/Web/API/Request)
 *
 * Usually this an URL to the resource you are requesting.
 */
declare type RequestInfo = string | Request;

/**
 * Constructor parameter for the
 * [Fetch API Request](https://developer.mozilla.org/en-US/docs/Web/API/Request)
 *
 * This contains information to send along with the request (Headers, body, etc...), as well as
 * Fastly specific information.
 */
declare interface RequestInit {
  method?: string;
  headers?: HeadersInit;
  body?: BodyInit | null;

  /**
   * The Fastly configured backend the request should be sent to.
   */
  backend?: string;
  cacheOverride?: CacheOverride;
}

/**
 * The Request class as [specified by WHATWG](https://fetch.spec.whatwg.org/#ref-for-dom-request%E2%91%A0)
 *
 * **Note**: Can only be used when processing requests, not during build-time initialization.
 *
 * @see {@link https://developer.mozilla.org/en-US/docs/Web/API/Request | Request on MDN}
 */
declare class Request implements Body {
  constructor(input: RequestInfo, init?: RequestInit);
  headers: Headers;
  method: string;
  url: string;

  body: ReadableStream<any>;
  bodyUsed: boolean;
  arrayBuffer(): Promise<ArrayBuffer>;
  json(): Promise<any>;
  text(): Promise<string>;

  // Fastly extensions
  backend: string;
  setCacheOverride(override: CacheOverride);
}

/**
 * Constructor parameter for the [Fetch API Response](https://developer.mozilla.org/en-US/docs/Web/API/Response)
 * This contains information to send along with the response.
 */
declare interface ResponseInit {
  headers?: HeadersInit;
  status?: number;
}

/**
 * The Response class as [specified by WHATWG](https://fetch.spec.whatwg.org/#ref-for-dom-response%E2%91%A0)
 *
 * **Note**: Can only be used when processing requests, not during build-time initialization.
 *
 * @see {@link https://developer.mozilla.org/en-US/docs/Web/API/Response | Response on MDN}
 */
declare class Response implements Body {
  constructor(body?: BodyInit, init?: ResponseInit);

  // These methods aren't supported yet.
  // static error(): Response;
  // static redirect(url: string, status: number): Response;
  // clone(): Response;

  headers: Headers;
  ok: boolean;
  redirected: boolean;
  status: number;
  url: string;

  body: ReadableStream<any>;
  bodyUsed: boolean;
  arrayBuffer(): Promise<ArrayBuffer>;
  json(): Promise<any>;
  text(): Promise<string>;
}

type ReadableStreamReader<T> = ReadableStreamDefaultReader<T>;
type ReadableStreamController<T> = ReadableStreamDefaultController<T>;

interface UnderlyingSinkAbortCallback {
  (reason: any): void | PromiseLike<void>;
}

interface UnderlyingSinkCloseCallback {
  (): void | PromiseLike<void>;
}

interface UnderlyingSinkStartCallback {
  (controller: WritableStreamDefaultController): void | PromiseLike<void>;
}

interface UnderlyingSinkWriteCallback<W> {
  (
    chunk: W,
    controller: WritableStreamDefaultController
  ): void | PromiseLike<void>;
}

interface UnderlyingSourceCancelCallback {
  (reason: any): void | PromiseLike<void>;
}

interface UnderlyingSourcePullCallback<R> {
  (controller: ReadableStreamController<R>): void | PromiseLike<void>;
}

interface UnderlyingSourceStartCallback<R> {
  (controller: ReadableStreamController<R>): void | PromiseLike<void>;
}

interface UnderlyingSink<W = any> {
  abort?: UnderlyingSinkAbortCallback;
  close?: UnderlyingSinkCloseCallback;
  start?: UnderlyingSinkStartCallback;
  type?: undefined;
  write?: UnderlyingSinkWriteCallback<W>;
}

interface UnderlyingSource<R = any> {
  cancel?: UnderlyingSourceCancelCallback;
  pull?: UnderlyingSourcePullCallback<R>;
  start?: UnderlyingSourceStartCallback<R>;
  type?: undefined;
}

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
   */
  preventClose?: boolean;
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

type ReadableStreamDefaultReadResult<T> =
  | ReadableStreamDefaultReadValueResult<T>
  | ReadableStreamDefaultReadDoneResult;

/** This Streams API interface represents a readable stream of byte data. The Fetch API offers a concrete instance of a ReadableStream through the body property of a Response object. */
interface ReadableStream<R = any> {
  readonly locked: boolean;
  cancel(reason?: any): Promise<void>;
  getReader(): ReadableStreamDefaultReader<R>;
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
  new <R = any>(
    underlyingSource?: UnderlyingSource<R>,
    strategy?: QueuingStrategy<R>
  ): ReadableStream<R>;
};

interface ReadableStreamDefaultController<R = any> {
  readonly desiredSize: number | null;
  close(): void;
  enqueue(chunk: R): void;
  error(e?: any): void;
}

declare var ReadableStreamDefaultController: {
  prototype: ReadableStreamDefaultController;
  new (): ReadableStreamDefaultController;
};

interface ReadableStreamDefaultReader<R = any>
  extends ReadableStreamGenericReader {
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
  new <W = any>(
    underlyingSink?: UnderlyingSink<W>,
    strategy?: QueuingStrategy<W>
  ): WritableStream<W>;
};

/** This Streams API interface represents a controller allowing control of a WritableStream's state. When constructing a WritableStream, the underlying sink is given a corresponding WritableStreamDefaultController instance to manipulate. */
interface WritableStreamDefaultController {
  error(e?: any): void;
}

declare var WritableStreamDefaultController: {
  prototype: WritableStreamDefaultController;
  new (): WritableStreamDefaultController;
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
  new<I = any, O = any>(transformer?: Transformer<I, O>, writableStrategy?: QueuingStrategy<I>, readableStrategy?: QueuingStrategy<O>): TransformStream<I, O>;
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
    (controller: TransformStreamDefaultController<O>): any;
}

interface TransformerTransformCallback<I, O> {
    (chunk: I, controller: TransformStreamDefaultController<O>): void | PromiseLike<void>;
}

declare type HeadersInit = Headers | string[][] | { [key: string]: string };

/**
 * The Headers class as [specified by WHATWG](https://fetch.spec.whatwg.org/#headers-class)
 *
 * @see {@link https://developer.mozilla.org/en-US/docs/Web/API/Headers | Headers on MDN}
 */
declare class Headers implements Iterable<[string, string]> {
  constructor(init?: HeadersInit);

  forEach(callback: (value: string, name: string) => void): void;
  append(name: string, value: string): void;
  delete(name: string): void;
  get(name: string): string | null;
  has(name: string): boolean;
  set(name: string, value: string): void;

  // Iterable methods
  entries(): IterableIterator<[string, string]>;
  keys(): IterableIterator<string>;
  values(): IterableIterator<[string]>;
  [Symbol.iterator](): Iterator<[string, string]>;
}

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
declare function fetch(
  resource: RequestInfo,
  init?: RequestInit
): Promise<Response>;

interface VoidFunction {
  (): void;
}

declare function queueMicrotask(callback: VoidFunction): void;

declare function structuredClone(value: any, options?: StructuredSerializeOptions): any;

interface StructuredSerializeOptions {
  transfer?: Transferable[];
}

type Transferable = ArrayBuffer;

interface WorkerLocation {
  readonly href: string;
  readonly protocol: string;
  readonly host: string;
  readonly hostname: string;
  readonly origin: string;
  readonly port: string;
  readonly pathname: string;
  readonly search: string;
  readonly hash: string;
  toString(): string;
}

declare var WorkerLocation: {
  prototype: WorkerLocation;
  new(): WorkerLocation;
};

declare var location: WorkerLocation;

interface Crypto {
  getRandomValues<T extends ArrayBufferView | null>(array: T): T;
}

declare var Crypto: {
  prototype: Crypto;
  new(): Crypto;
};

declare var crypto: Crypto;