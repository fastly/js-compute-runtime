## 0.2.4 (2022-02-09)

### Enhancements

* Support streaming upstream request bodies (https://github.com/fastly/js-compute-runtime/pull/67)

## 0.2.3 (2022-02-01)

### Fixes

* Avoid waiting for async tasks that weren't passed to `FetchEvent#waitUntil` (https://github.com/fastly/js-compute-runtime/pull/53)
* Significantly improve spec-compliance of Request and Response builtins (https://github.com/fastly/js-compute-runtime/pull/64)
### Enhancements

* Increase max supported header size from 4096 bytes to 69000 bytes (https://github.com/fastly/js-compute-runtime/pull/58)
* Update to SpiderMonkey 96 beta (https://github.com/fastly/js-compute-runtime/pull/61)
* Add full support for TransformStreams (https://github.com/fastly/js-compute-runtime/pull/61)
* Support directly piping Request/Response bodies to other Requests/Responses instead of manually copying every chunk (https://github.com/fastly/js-compute-runtime/pull/62)
* Add support for the `queueMicrotask` global function (https://github.com/fastly/js-compute-runtime/pull/65)
* Add support for the `structuredClone` global function (https://github.com/fastly/js-compute-runtime/pull/65)
* Add support for the `location` global object as an instance of `WorkerLocation` (https://github.com/fastly/js-compute-runtime/pull/65)
* Support Big{Ui,I}nt64Array in crypto.getRandomValues (https://github.com/fastly/js-compute-runtime/pull/65)
* Enable class static blocks syntax (https://github.com/fastly/js-compute-runtime/pull/65)

### Fixes

* Ensure we're not waiting for async tasks not passed to `FetchEvent#waitUntil` (https://github.com/fastly/js-compute-runtime/pull/53)

## 0.2.2 (2021-11-10)

### Fixes

* Strip leading `?` in `URLSearchParams` constructor (https://github.com/fastly/js-compute-runtime/pull/35)
* Error the ReadableStream when a body read fails in the hostcall (https://github.com/fastly/js-compute-runtime/pull/36)
* Report uncaught exceptions in the request handler to stderr (https://github.com/fastly/js-compute-runtime/pull/44)
* Fix geo-lookup hostcall invocation (https://github.com/fastly/js-compute-runtime/pull/46)

### Enhancements

* Resolve URLs passed to `Request` and `fetch` relative to the client request URL's origin (https://github.com/fastly/js-compute-runtime/pull/38)
* Return null instead of throwing on missing key in `Dictionary#get` (https://github.com/fastly/js-compute-runtime/pull/41)
* Update to SpiderMonkey 94 beta (https://github.com/fastly/js-compute-runtime/pull/45)
* Expose environment variables via the `fastly.env.get` function (https://github.com/fastly/js-compute-runtime/pull/50)

## 0.2.1 (2021-08-27)

### Fixes

- Properly support `base` argument in `URL` constructor
  [#33](https://github.com/fastly/js-compute-runtime/pull/33)

## 0.2.0 (2021-08-24)

### Enhancements

- Implement the WHATWG URL and URLSearchParam classes
  [#4](https://github.com/fastly/js-compute-runtime/pull/4)

- Enable private class fields and methods, plus ergonomic brand checks
  [#13](https://github.com/fastly/js-compute-runtime/pull/13)

- **Breaking change:** Implement FetchEvent#waitUntil
  [#14](https://github.com/fastly/js-compute-runtime/pull/14)

- Mark builtins that rely on hostcalls as request-handler-only
  [#20](https://github.com/fastly/js-compute-runtime/pull/20)

- Add support for including binary files, and for using typed arrays as Response bodies
  [#22](https://github.com/fastly/js-compute-runtime/pull/22)

- Improve handling of hostcall errors
  [#24](https://github.com/fastly/js-compute-runtime/pull/24)

### Fixes

- **Breaking change:** Make FetchEvent handling more spec-compliant
  [#8](https://github.com/fastly/js-compute-runtime/pull/8)

- Normalize HTTP method names when constructing Requests
  [#15](https://github.com/fastly/js-compute-runtime/pull/15)

- Don't trap when trying to delete a non-existent header
  [#16](https://github.com/fastly/js-compute-runtime/pull/16)

- Reject fetch promise on network error
  [#29](https://github.com/fastly/js-compute-runtime/pull/29)

## 0.1.0 (2021-07-29)

- Initial release.
