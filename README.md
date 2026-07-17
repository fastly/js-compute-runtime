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

### CLI Flags and configuration

The CLI is typically invoked by the `build` script defined in your project's `package.json` scripts.

```json
{
  "scripts": {
    "build": "js-compute-runtime src/index.js bin/main.wasm"
  }
}
```

The CLI is invoked as follows:

```sh
js-compute-runtime [OPTIONS] <input-js-file> <output-wasm-file>
```

Options can be specified on the command line or added to a persistent configuration file for the project.  The CLI will search for a configuration starting from the current directory in the following order:

* A `fastlycompute` property in `package.json`
* `.fastlycomputerc.json`
* `fastlycompute.config.js`

<details>
<summary>Click here to see full list</summary>

The CLI will search for a configuration starting from the current directory in the following order:

- A `fastlycompute` property in `package.json`
- `.fastlycomputerc` (JSON or YAML)
- `.fastlycomputerc.json`, `.fastlycomputerc.yaml`, `.fastlycomputerc.yml`
- `.fastlycomputerc.js`, `.fastlycomputerc.mjs`
- `fastlycompute.config.js`, `fastlycompute.config.mjs`

</details>

If an option is defined in both the command line and the configuration file, the command line option takes precedence.

#### Supported Options

| Config Key                                    | CLI Flag                                             | Type                        | Description                                                                                                                                               | 
|:----------------------------------------------|:-----------------------------------------------------|:----------------------------|:----------------------------------------------------------------------------------------------------------------------------------------------------------|
| `enableAOT`                                   | `--enable-aot`                                       | `boolean`                   | Enable AOT compilation for performance                                                                                                                    |
| `aotCache`                                    | `--aot-cache`                                        | `string` (path)             | Specify a path to the AOT cache file                                                                                                                      |
| `enableHttpCache`                             | `--enable-http-cache`                                | `boolean`                   | Enable the [HTTP cache hook API](https://www.fastly.com/documentation/guides/concepts/cache/#modifying-a-request-as-it-is-forwarded-to-a-backend)         |
| `enableExperimentalHighResolutionTimeMethods` | `--enable-experimental-high-resolution-time-methods` | `boolean`                   | Enable experimental fastly.now() method                                                                                                                   |
| `enableExperimentalTopLevelAwait`             | `--enable-experimental-top-level-await`              | `boolean`                   | Enable experimental top level await                                                                                                                       |
| `enableStackTraces`                           | `--enable-stack-traces`                              | `boolean`                   | Enable stack traces                                                                                                                                       |
| `excludeSources`                              | `--exclude-sources`                                  | `boolean`                   | Don't include sources in stack traces                                                                                                                     |
| `debugIntermediateFiles`                      | `--debug-intermediate-files`                         | `string` (path)             | Output intermediate files in directory                                                                                                                    |
| `debugBuild`                                  | `--debug-build`                                      | `boolean`                   | Use debug build of the SDK runtime                                                                                                                        |
| `engineWasm`                                  | `--engine-wasm`                                      | `string` (path)             | Specify a custom Wasm engine (advanced)                                                                                                                   |
| `wevalBin`                                    | `--weval-bin`                                        | `string` (path)             | Specify a custom weval binary (advanced)                                                                                                                  |
| `env`                                         | `--env`                                              | `string \| object \| array` | Set environment variables, possibly inheriting from the current environment. Multiple variables can be comma-separated (e.g., --env ENV_VAR,OVERRIDE=val) |

NOTE: The `env` field is additive. Values defined on the command-line values append to, rather than replace, the values defined in any configuration file. 

#### Example command-line options

```sh
js-compute-runtime --enable-aot --enable-stack-traces --enable-top-level-await --env=LOG_LEVEL=debug ./src/index.js ./bin/main.wasm
```

#### Example `.fastlycomputerc.json`

```json
{
  "enableAOT": true,
  "enableStackTraces": true,
  "enableTopLevelAwait": true,
  "env": {
    "LOG_LEVEL": "debug"
  }
}
```

#### Example `package.json`

```json
{
  "name": "my-fastly-service",
  "type": "module",
  "dependencies": {
    "@fastly/cli": "^13.0.0"
  },
  "scripts": {
    "build": "js-compute-runtime ./src/index.js ./bin/main.wasm",
    "dev": "fastly compute serve",
    "deploy": "fastly compute publish"
  },
  "fastlycompute": {
    "enableAOT": true,
    "enableStackTraces": true,
    "enableTopLevelAwait": true,
    "env": {
      "LOG_LEVEL": "debug"
    }
  }
}
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
