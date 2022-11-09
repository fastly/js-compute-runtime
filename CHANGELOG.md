## 0.5.4 (2022-09-28)

#### Dynamic Backend support

Note: This feature is disabled by default for Fastly Services. Please contact [Fastly Support](https://support.fastly.com/hc/en-us/requests/new?ticket_form_id=360000269711) to request the feature be enabled on the Fastly Services which require Dynamic Backends.

This feature makes it possible to use the standard `fetch` within JavaScript applications.

Dynamic Backends is a new feature which enables JavaScript applications to dynamically create new backend server definitions without having to deploy a new version of their Fastly Service. These backends function exactly the same as existing backends, and can be configured in all the same ways as existing backends can via the Fastly Service configuration.

By default, Dynamic Backends are disabled within a JavaScript application as it can be a potential avenue for third-party JavaScript code to send requests, potentially including sensitive/secret data, off to destinations that the JavaScript project was not intending, which could be a security issue. To enable Dynamic Backends the application will need to set `fastly.allowDynamicBackends` is to `true`.

There are two ways to make use of Dynamic Backends within JavaScript projects:

The first way is by omitting the `backend` property definition on the Request instance. The JavaScript Runtime will then create a Dynamic Backend definition using default configuration options. This approach is useful for JavaScript applications as it means that a standard `fetch` call will now be possible, which means libraries that use the standard `fetch` will begin to work for applications deployed to Fastly.

Below is as an example JavaScript application using the default Dynamic Backend option:
```js
// Enable dynamic backends -- warning, this is potentially dangerous as third-party dependencies could make requests to their own backends, potentially including your sensitive/secret data
fastly.allowDynamicBackends = true;

// For any request, return the fastly homepage -- without defining a backend!
addEventListener("fetch", event => {
  event.respondWith(fetch('https://www.fastly.com/'));
});
```

The second way is by creating a new Dynamic Backend using the new `Backend` class. This approach is useful for JavaScript applications that want to have full control over the configuration of the new backend defintion, such as only allowing TLS 1.3 and disallowing older versions of TLS for requests made to the new backend.

E.G.
```js
// Enable dynamic backends -- warning, this is potentially dangerous as third-party dependencies could make requests to their own backends, potentially including your sensitive/secret data
fastly.allowDynamicBackends = true;

// For any request, return the fastly homepage -- without defining a backend!
addEventListener("fetch", event => {
  // We are not defining all the possible fields here, the ones which are not defined will use their default value instead.
  const backend = new Backend({
    name: 'fastly',
    target: 'fastly.com',
    hostOverride: "www.fastly.com",
    sslMinVersion: 1.3,
    sslMaxVersion: 1.3,
    sniHostname: "www.fastly.com",
  });
  event.respondWith(fetch('https://www.fastly.com/', {
    backend // Here we are configuring this request to use the newly defined backend from above.
  }));
});
```

#### Config-store support and Dictionary deprecated

We have renamed the `Dictionary` class to `ConfigStore`, the old name `Dictionary` still exists but is now deprecated. We recommend replacing `Dictionary` with `ConfigStore` in your code to avoid having to migrate in the future when `Dictionary` is fully removed.

Below is an example application using the `ConfigStore` class:
```js
async function app(event) {
  const store = new ConfigStore('example')
  
  // Retrieve the contents of the 'hello' key
  const hello = await store.get('hello')
  
  return new Response(hello)
}

addEventListener("fetch", event => {
  event.respondWith(app(event))
})
```


### Added

* Add ConfigStore class ([#270](https://github.com/fastly/js-compute-runtime/pull/270))
* Add Dynamic Backends support ([#250](https://github.com/fastly/js-compute-runtime/issues/250))
* Improved performance when constructing a ObjectStore instance ([#272](https://github.com/fastly/js-compute-runtime/pull/272)

## 0.5.3 (2022-09-16)

### Security Fixes

* [CVE-2022-39218](https://github.com/fastly/js-compute-runtime/security/advisories/GHSA-cmr8-5w4c-44v8): 
  Fixed `Math.random` and `crypto.getRandomValues` methods to always use sufficiently random values. The previous versions would use a PRNG (pseudorandom number generator) which we would seed with a random value however due to our use of [Wizer](https://github.com/bytecodealliance/wizer), the initial value to seed the PRNG was baked-in to the final WebAssembly module meaning the sequence of numbers generated was predictable for that specific WebAssembly module. The new implementations of both `Math.random` and `crypto.getRandomValues` do not use a PRNG and instead pull random values from WASI (WebAssembly System Interface) libc’s `random_get` function, which is always a sufficiently random value.
  
  An attacker with access to the same WebAssembly module that calls the affected methods could use the fixed seed to predict random numbers generated by these functions. This information could be used to bypass cryptographic security controls, for example to disclose sensitive data encrypted by functions that use these generators.

  Developers should update affected modules after applying this patch. Any secrets generated using affected versions should be rotated. Any sensitive ciphertext generated using affected versions should be considered unsafe, e.g. and be deleted or re-generated.
  
### Fixed

- Updated the Typescript definitions for the `console` methods to indicate that they now accept any number of objects. ([#258](https://github.com/fastly/js-compute-runtime/pull/258))

- Store the Object-Store key string into a native object to avoid it becoming garbage collected before being used within `ObjectStore.prototype.get` or `ObjectStore.prototype.put` (([381242](https://github.com/fastly/js-compute-runtime/commit/3812425a955e52c2fd7229e762ef3e691cb78745))


## 0.5.2 (2022-09-02)

### Fixed

- Explicitly declare void as the return type for functions which return nothing - this allows our package to work with typescript's `strict:true` option ([#253](https://github.com/fastly/js-compute-runtime/pull/253))

- Declare ambient types for our npm package instead of exports as we do not yet export anything from the package ([#252](https://github.com/fastly/js-compute-runtime/pull/252))


## 0.5.1 (2022-08-31)

### Fixed

- Removed `type: "module"` from the @fastly/js-compute package.json file as the package still uses `require`

## 0.5.0 (2022-08-30)

### Features

#### Object-store support

This release adds support for Fastly [Object-store](https://developer.fastly.com/reference/api/object-store/), which is globally consistent key-value storage accessible across the Fastly Network. This makes it possible for your Compute@Edge application to read and write from Object-stores.

We've added two classes, `ObjectStore`, and `ObjectStoreEntry`. `ObjectStore` is used to interact with a particular Object-store and `ObjectStoreEntry` is a particular value within an Object-store. We've made `ObjectStoreEntry` have a similar API as `Response` to make it simpler to read and write from Object-stores. I.e. `ObjectStoreEntry` has a `body` property which is a `ReadableStream` and has `arrayBuffer`/`json`/`text` methods - just like `Response`.

The way to use these classes is best shown with an example:
```js
async function app(event) {
  // Create a connection the the Object-store named 'example-store'
  const store = new ObjectStore('example-store')
  
  // Create or update the 'hello' key with the contents 'world'
  await store.put('hello', 'world')
  
  // Retrieve the contents of the 'hello' key
  // Note: Object-stores are eventually consistent, this means that the updated contents associated may not be available to read from all
  // Fastly edge locations immediately and some edge locations may continue returning the previous contents associated with the key.
  const hello = await store.get('hello')
  
  // Read the contents of the `hello` key into a string
  const hellotext = await hello.text()
  return new Response(hellotext)
}

addEventListener("fetch", event => {
  event.respondWith(app(event))
})
```

#### Added `btoa` and `atob` global functions

These two functions enable you to encode to ([btoa](https://developer.mozilla.org/en-US/docs/Web/API/btoa)) and decode from ([atob](https://developer.mozilla.org/en-US/docs/Web/API/atob)) Base64 strings. They follow the same specification as the `atob` and `btoa` functions that exist in web-browsers.

```js
addEventListener("fetch", event => {
  event.respondWith(new Response(atob(btoa('hello from fastly'))))
})
```


#### Improved Console Support

Previously our console methods only supported a single argument and would convert the argument to a string via `String(argument)`, this unfortunately made it difficult to log out complex objects such as Request objects or similar.

We've updated our console methods and they now support any number of arguments. As well as supporting any number of arguments, we've also changed the implementation to have better support for logging out complex objects.

This is a before and after example of what happens when logging a Request with our console methods.

Before:
```js
const request = new Request('https://www.fastly.com', {body:'I am the body', method: 'POST'});
console.log(request); // outputs `[object Object]`.
```

After:
```js
const request = new Request('https://www.fastly.com', {body:'I am the body', method: 'POST'});
console.log(request); // outputs `Request: {method: POST, url: https://www.fastly.com/, version: 2, headers: {}, body: null, bodyUsed: false}`.
```


### Added

* Implemented ObjectStore and ObjectStoreEntry classes for interacting with Fastly ObjectStore ([#110](https://github.com/fastly/js-compute-runtime/issues/110))
* add btoa and atob native implementations ([#227](https://github.com/fastly/js-compute-runtime/issues/227)) ([8b8c31f](https://github.com/fastly/js-compute-runtime/commit/8b8c31fa9ad70337b1060a3242b8e3495ae47df3))

### Changed

* Improved console output for all types ([#204](https://github.com/fastly/js-compute-runtime/issues/204))

## 0.4.0 (2022-07-28)

### Added

- Implement the DecompressionStream builtin [`#160`](https://github.com/fastly/js-compute-runtime/pull/160)
- Improve performace of Regular Expression literals via precompilation [`#146`](https://github.com/fastly/js-compute-runtime/pull/146)

### Fixed

- Calling `tee` on the client request no longer causes the application to hang [`#156`](https://github.com/fastly/js-compute-runtime/pull/156)

## 0.3.0 (2022-06-29)

### Added

- Implement the CompressionStream builtin
  [#84](https://github.com/fastly/js-compute-runtime/pull/84)

### Changed

- Removed the requirement for a fastly.toml file to be present when using js-compute-runtimes CLI to compile a WASM file
- **Breaking change:** Removed --skip-pkg argument from js-compute-runtime's CLI
  [#108](https://github.com/fastly/js-compute-runtime/pull/108)
- **Breaking change:** Removed `console.trace` method

### Fixed

- Fix the response error message text
- Throw an error if constructors are called as plain functions
- Fix the behavior of `console.debug`
- Allow builtin classes to be extended

## 0.2.5  (2022-04-20)

### Changed

- Updated the js-compute-runtime to 0.2.5 : Increased max uri length to 8k, and properly forwards http headers to upstream requests even if the headers aren't ever read from

## 0.2.4 (2022-02-09)

### Changed

- Support streaming upstream request bodies

## 0.2.2 (2022-02-03)

### Added

- Add full support for TransformStreams
- Support directly piping Request/Response bodies to other Requests/Responses instead of manually copying every chunk
- Add support for the `queueMicrotask` global function
- Add support for the `structuredClone` global function
- Add support for the `location` global object as an instance of `WorkerLocation`
- Support Big{Ui,I}nt64Array in crypto.getRandomValues
- Enable class static blocks syntax
- Returned the exit code from the JS Compute Runtime, by passing it up through our CLI

### Changed

- Increase max supported header size from 4096 bytes to 69000 bytes
- Update to SpiderMonkey 96 beta

### Fixed

- Avoid waiting for async tasks that weren't passed to `FetchEvent#waitUntil`
- Significantly improve spec-compliance of Request and Response builtins

### Changed

## 0.2.1 (2021-11-10)

### Added

- Updated the js-compute-runtime to `0.2.2` (Which includes fixes to geoip, a way to get environment variables, improves debugging of exceptions in the request handler, and other updates)
- Added the `Env` namespace for accessing Fastly C@E environment variables.

## 0.2.0 (2021-08-31)

### Added

- Implement the WHATWG URL and URLSearchParam classes
- Enable private class fields and methods, plus ergonomic brand checks
- Breaking change: Implement FetchEvent#waitUntil
- Mark builtins that rely on hostcalls as request-handler-only
- Add support for including binary files, and for using typed arrays as Response bodies
- Improve handling of hostcall errors

### Fixed

- Breaking change: Make FetchEvent handling more spec-compliant
- Normalize HTTP method names when constructing Requests
- Don't trap when trying to delete a non-existent header
- Properly support `base` argument in `URL` constructor

## 0.1.0 (2021-07-28)

- Initial Release
- Includes TypeScript type definitions for Compute@Edge flavored ServiceWorkers APIs
- Also includes the `js-compute-runtime` CLI for bundling JavaScript applications
