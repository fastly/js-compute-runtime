# Fastly Compute@Edge JS Runtime

The JS Compute Runtime for Fastly's [Compute@Edge platform](https://www.fastly.com/products/edge-compute/serverless) provides the environment JavaScript is executed in when using the [Compute@Edge JavaScript SDK](https://www.npmjs.com/package/@fastly/js-compute).

**Note**: If you just want to use JavaScript on the Compute@Edge Platform, we recommend using the JavaScript Starter Kits provided by the Fastly CLI tool. For more information please see the [JavaScript documentation on the Fastly Developer Hub](https://developer.fastly.com/learning/compute/javascript/).

## Working with the JS Compute Runtime source

Note that this repository uses Git submodules, so you will need to run

```sh
git submodule update --recursive --init
```

to pull down or update submodules.

### Building the JS Compute Runtime

To build from source, you need to ensure that the headers and object files for the [SpiderMonkey JavaScript engine](https://spidermonkey.dev/) are available. It's recommended to download pre-built object files:
```sh
(cd c-dependencies/spidermonkey && sh download-engine.sh)
```


Alternatively, the engine can also be built from source using `c-dependencies/spidermonkey/build-engine.sh`. That should only be required if you want to modify the engine itself, however.

In addition you need to have the following tools installed to successfully build, and build from a linux based system.

- Rust 
  ```
  curl -so rust.sh https://sh.rustup.rs && sh rust.sh -y
  restart shell or run source $HOME/.cargo/env
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
- [wasi-sdk, version 12](https://github.com/WebAssembly/wasi-sdk/releases/tag/wasi-sdk-12),
  with alternate [install instructions](https://github.com/WebAssembly/wasi-sdk#install)
  ```sh
  curl -sS -L -O https://github.com/WebAssembly/wasi-sdk/releases/download/wasi-sdk-12/wasi-sdk-12.0-linux.tar.gz
  tar xf wasi-sdk-12.0-linux.tar.gz
  sudo mkdir -p /opt/wasi-sdk
  sudo mv wasi-sdk-12.0/* /opt/wasi-sdk/
  ```

- rust version - the rust version may need to be pinned due to the WASI spidermonkey version, the current version can be found at [main.yml](.github/workflows/main.yml) under "Install pinned Rust version and wasm32-wasi target"
  ```
   rustup update 1.57.0 --no-self-update

   cd /location/of/js-compute-runtime
   rustup override set 1.57.0
  ```

Once that is done, the runtime and the CLI tool for applying it to JS source code can be built using cargo:
```sh
cargo build --release
```

#### Build a windows executable
To build for windows on a linux system you need to install the following modules:

```
sudo apt-get install gcc-mingw-w64
rustup target add x86_64-pc-windows-gnu
```

then you can run the following
```sh
cargo build --target x86_64-pc-windows-gnu --release
```

## Testing

The JS Compute Runtime doesn't currently contain automated tests itself. Instead, Fastly runs an automated test suite for an internal repository for the JavaScript SDK using the runtime.

Manual testing is well supported, however. To test your changes, you can follow these steps:
1. Build the runtime's CLI tool, see above
2. Create a C@E service from a JS source file by running the CLI tool
3. Test the service using [Fastly's local testing server](https://developer.fastly.com/learning/compute/testing/#running-a-local-testing-server)

As an example, to turn a file `test.js` with this source code:
```js
addEventListener('fetch', e => {
  console.log("Hello World!");
});
```

into a C@E service with your build of the CLI tool, run the following command:
```sh
cargo run --release -- test.js test.wasm
```

Then test your service in Viceroy:
```sh
viceroy test.wasm
```

## Testing a Dev Release
:warning:	**You should not use this for production workloads!!!!!!!!**

Dev builds are released before production releases to allow for further testing. These are not released upstream to NPM, however you can acquire them from the [Releases](https://github.com/fastly/js-compute-runtime/releases/) section. Download the runtime for your platform, extract the executable and place it in the /node_modules/@fastly/js-compute/bin/PLATFORM folder of your Compute@Edge project. Then you can use the normal [Fastly CLI](https://github.com/fastly/cli) to build your service. 

Please submit an [issue](https://github.com/fastly/js-compute-runtime/issues) if you find any problems during testing.
