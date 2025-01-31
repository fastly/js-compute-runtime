# Fastly Compute JS Runtime

The JS Compute Runtime for Fastly's [Compute platform](https://www.fastly.com/products/edge-compute/serverless) provides the environment JavaScript is executed in when using the [Fastly Compute JavaScript SDK](https://www.npmjs.com/package/@fastly/js-compute).

**Note**: If you just want to use JavaScript on the Fastly Compute Platform, we recommend using the JavaScript Starter Kits provided by the Fastly CLI tool. For more information please see the [JavaScript documentation on the Fastly Developer Hub](https://developer.fastly.com/learning/compute/javascript/).

## Working with the JS Compute Runtime source

Note that this repository uses Git submodules, so you will need to run

```sh
git submodule update --recursive --init
```

to pull down or update submodules.

### Building the JS Compute Runtime

To build from source, you need to have the following tools installed to successfully build, depending on your system.

#### Linux

- Build tools
  ```sh
  sudo apt install build-essential
  ```
- Rust
  ```
  curl -so rust.sh https://sh.rustup.rs && sh rust.sh -y
  restart shell or run source $HOME/.cargo/env
  ```
- binaryen
  ```sh
  sudo apt install binaryen
  ```
- rust target wasm32-wasi
  ```sh
  rustup target add wasm32-wasi
  ```
- [cbindgen](https://github.com/eqrion/cbindgen#quick-start)
  ```sh
  cargo install cbindgen
  ```
- [wasi-sdk, version 20](https://github.com/WebAssembly/wasi-sdk/releases/tag/wasi-sdk-20),
  with alternate [install instructions](https://github.com/WebAssembly/wasi-sdk#install)
  ```sh
  curl -sS -L -O https://github.com/WebAssembly/wasi-sdk/releases/download/wasi-sdk-20/wasi-sdk-20.0-linux.tar.gz
  tar xf wasi-sdk-20.0-linux.tar.gz
  sudo mkdir -p /opt/wasi-sdk
  sudo mv wasi-sdk-20.0/* /opt/wasi-sdk/
  ```

Build the runtime using npm:

```sh
npm run build
```

#### macOS (Apple silicon)

- Build tools
  ```sh
  # First, install Xcode from the Mac App Store. Then:
  sudo xcode-select --switch /Applications/Xcode.app
  sudo xcodebuild -license
  ```
- Homebrew
  ```sh
  # From homebrew.sh
  /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
  ```
- wget
  ```sh
  brew install wget
  ```
- binaryen
  ```sh
  brew install binaryen
  ```
- Python
  ```sh
  brew install python@3
  ```
- Rust
  ```sh
  curl -so rust.sh https://sh.rustup.rs && sh rust.sh -y
  # then, restart shell or run:
  source $HOME/.cargo/env
  ```
- rust target wasm32-wasi
  ```sh
  rustup target add wasm32-wasi
  ```
- [cbindgen](https://github.com/eqrion/cbindgen#quick-start)
  ```sh
  cargo install cbindgen
  ```
- [wasi-sdk, version 20](https://github.com/WebAssembly/wasi-sdk/releases/tag/wasi-sdk-20),
  with alternate [install instructions](https://github.com/WebAssembly/wasi-sdk#install)
  ```sh
  curl -sS -L -O https://github.com/WebAssembly/wasi-sdk/releases/download/wasi-sdk-20/wasi-sdk-20.0-macos.tar.gz
  tar xf wasi-sdk-20.0-macos.tar.gz
  sudo mkdir -p /opt/wasi-sdk
  sudo mv wasi-sdk-20.0/* /opt/wasi-sdk/
  ```

Build the runtime using npm:

```sh
npm run build
```

## Testing a Local build in a Compute application

:warning: **You should not use this for production workloads!!!!!!!!**

You can test a local build of the JS Compute runtime by installing it in your JavaScript Compute application and running that locally or by uploading it to your Fastly service.

1. First, follow the directions in [Building the JS Compute Runtime](#building-the-js-compute-runtime) for your platform to obtain a local build. The build outputs are the following files:

   - `fastly.wasm`
   - `fastly.debug.wasm`
   - `fastly-weval.wasm`
   - `fastly-ics.wevalcache`

2. Create a local tarball using npm.

   ```shell
   npm pack
   ```

   The resulting tarball will have a filename such as `fastly-js-compute-<version>.tgz`.

3. In your Compute application, install the tarball using `npm`:

   ```shell
   npm install /path/to/fastly-js-compute-<version>.tgz
   ```

4. Build and test or deploy your application as usual, using `fastly compute serve` or `fastly compute publish`, or an appropriate npm script.

## Testing a Dev Release in a Compute application

:warning: **You should not use this for production workloads!!!!!!!!**

Dev builds are released before production releases to allow for further testing. These are not released upstream to NPM, however you can acquire them from the [Releases](https://github.com/fastly/js-compute-runtime/releases/) section. Download the runtime for your platform, extract the executable and place it in the /node_modules/@fastly/js-compute/bin/PLATFORM folder of your Fastly Compute project. Then you can use the normal [Fastly CLI](https://github.com/fastly/cli) to build your service.

Please submit an [issue](https://github.com/fastly/js-compute-runtime/issues) if you find any problems during testing.

## Tests

All tests are automatically run on pull requests via CI.

### Unit Testing

Unit tests are run via `npm run test`, currently including:

- CLI tests (`npm run test:cli`)
- Typing tests (`npm run test:types`)

### Integration Tests

Complete test applications are tested from the `./integration-tests/js-compute/fixtures/app/src` and `./integration-tests/js-compute/fixtures/module-mode/src` directories.

Tests themselves are listed in the `./integration-tests/js-compute/fixtures/app/tests.json` and `./integration-tests/js-compute/fixtures/module-mode/tests.json` files.

Integration tests can be run via `npm run test:integration`, which defaults to the release build.

In addition the following flags can be added after the command (passed via `npm run test:debug -- ...` after the `--`):

- `--local`: Test locally using Viceroy, instead of publishing to a staging Compute service.
- `--bail`: Immediately stop testing on the first failure, and report the failure.
- `--verbose`: Adds verbose logging to `fastly compute publish` and Viceroy (which provides hostcall logging as well).
- `--debug-build`: Use the debug build
- `--debug-log`: Enable debug logging for the tests (engine-level DEBUG_LOG)
- `--module-mode`: Run the module mode test suite (`fixtures/module-mode` instead of `fixtures/app`).
- `--http-cache`: Run the HTTP cache test suite
- `[...args]`: Additional arguments allow for filtering tests

A typical development test command is therefore something like:

```
npm run build:debug && npm run test:integration -- --debug-build --debug-log --local --bail /crypto
```

Which would run a debug build, enable debugging logging, and then that build against all the crypto tests locally on Viceroy, throwing an error as soon as one is found.

Some tests can only be run on Compute and not Viceroy and will be automatically skipped. A green tick is always shown for a test that ran successfully - if it is missing that means it did not run.

### Web Platform Tests

The Web Platform tests are included as a submodule, and can be run via `npm run test:wpt` or `npm run test:wpt:debug`.

The WPT test runner supports the following options (passed via `npm run test:wpt -- ...` after the `--`):

- `--update-expectations`: Update the WPT test expectations JSON files based on the current PASS/FAIL test statuses, instead of throwing an error when the current PASS/FAIL lists are not matched.
- `[...args]`: Filter to apply to WPT tests to run

Run `./tests/wpt-harness/run-wpt.mjs --help` for further options information.
