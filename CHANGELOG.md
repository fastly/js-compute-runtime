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
