## Unreleased

### Changed

-

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
