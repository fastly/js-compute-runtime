# @fastly/js-compute

JavaScript SDK and CLI for building JavaScript applications on [Fastly Compute](https://www.fastly.com/products/edge-compute/serverless).

![npm version](https://img.shields.io/npm/v/@fastly/js-compute) ![npm downloads per month](https://img.shields.io/npm/dm/@fastly/js-compute)

## Getting Started

We recommend using the [Fastly CLI](https://github.com/fastly/cli) to create, build, and deploy JavaScript Fastly Compute services, as [described on the Fastly Developer Hub](https://developer.fastly.com/learning/compute/).

[Detailed documentation for JavaScript Fastly Compute services](https://developer.fastly.com/learning/compute/javascript/) is also available on Fastly Developer Hub.

## Usage

### JavaScript Examples

The Fastly Developer Hub has a collection of [example JavaScript applications](https://developer.fastly.com/solutions/examples/javascript/).

Here is a small example application:
```js
/// <reference types="@fastly/js-compute" />

async function app(event) {
    const request = event.request;
    return new Response(`You made a request to ${request.url}`)
}

addEventListener("fetch", event => {
  event.respondWith(app(event));
});
```

### API documentation

The API documentation for the JavaScript SDK is located at [https://js-compute-reference-docs.edgecompute.app](https://js-compute-reference-docs.edgecompute.app).

## Security

If you find any security issues, see the [Fastly Security Reporting Page](https://www.fastly.com/security/report-security-issue) or send an email to: `security@fastly.com`

We plan to disclose any found security vulnerabilities per the [GitHub security vulnerability guidance](https://docs.github.com/en/code-security/security-advisories/guidance-on-reporting-and-writing/about-coordinated-disclosure-of-security-vulnerabilities#best-practices-for-maintainers). Note that communications related to security issues in Fastly-maintained OSS as described here are distinct from [Fastly security advisories](https://www.fastly.com/security-advisories).

## Changelog

The changelog can be found [here](https://github.com/fastly/js-compute-runtime/blob/main/CHANGELOG.md).
## License

[Apache-2.0 WITH LLVM-exception](./LICENSE)
