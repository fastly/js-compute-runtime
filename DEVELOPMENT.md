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

- Rust
  ```sh
  curl -so rust.sh https://sh.rustup.rs && sh rust.sh -y
  # then, restart shell or run:
  source $HOME/.cargo/env
  ```
- Build tools
  ```sh
  sudo apt install build-essential
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

Once you have those installed, you will need to compile SpiderMonkey:
```
(cd runtime/spidermonkey && bash build-engine.sh)
```

Once that is done, the runtime and the CLI tool for applying it to JS source code can be built using npm:
```sh
npm run build && npm run build:starlingmonkey
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
- Python 3.11 (Spidermonkey build requires a version before 3.12)
  ```sh
  brew install python@3.11
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

Once you have those installed, you will need to compile SpiderMonkey:
```sh
export PATH="/opt/homebrew/opt/python@3.11/libexec/bin:$PATH"
(cd runtime/spidermonkey && bash build-engine.sh)
```

Once that is done, the runtime and the CLI tool for applying it to JS source code can be built using npm:
```sh
npm run build && npm run build:starlingmonkey
```

## Testing

The JS Compute Runtime has automated tests which run on all pull-requests. The test applications are located within <./integration-tests/js-compute>.

To run an end-to-end test which builds and deploys an application to fastly:
- Build the runtime and cli: `npm run build` in the root of the project
- Change to the test directory for the runtime: `cd integration-tests/js-compute/`
- Install the test dependencies: `npm install`
- Get a list of all the applications to test: `node test.js`
- Test a single application via: `node test.js <name>` or test all via `node test.js --all`

## Testing a Dev Release
:warning:	**You should not use this for production workloads!!!!!!!!**

Dev builds are released before production releases to allow for further testing. These are not released upstream to NPM, however you can acquire them from the [Releases](https://github.com/fastly/js-compute-runtime/releases/) section. Download the runtime for your platform, extract the executable and place it in the /node_modules/@fastly/js-compute/bin/PLATFORM folder of your Fastly Compute project. Then you can use the normal [Fastly CLI](https://github.com/fastly/cli) to build your service. 

Please submit an [issue](https://github.com/fastly/js-compute-runtime/issues) if you find any problems during testing.
