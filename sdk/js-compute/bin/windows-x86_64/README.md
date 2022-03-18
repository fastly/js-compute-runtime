# Fastly Compute@Edge JS Runtime

The JS Compute Runtime for Fastly's [Compute@Edge platform](https://www.fastly.com/products/edge-compute/serverless) provides the environment JavaScript is executed in when using the [Compute@Edge JavaScript SDK](https://www.npmjs.com/package/@fastly/js-compute).

**Note**: If you just want to use JavaScript on the Compute@Edge Platform, we recommend using the JavaScript Starter Kits provided by the Fastly CLI tool. For more information please see the [JavaScript documentation on the Fastly Developer Hub](https://developer.fastly.com/learning/compute/javascript/).

## Working with the JS Compute Runtime source

Note that this repository uses Git submodules, so you will need to run

```
git submodule update --recursive --init
```

to pull down or update submodules.

### Building the JS Compute Runtime

To build from source, you need to ensure that the headers and object files for the [SpiderMonkey JavaScript engine](https://spidermonkey.dev/) are available. It's recommended to download pre-built object files:

```sh
(cd c-dependencies/spidermonkey && sh download-engine.sh)
```

Alternatively, the engine can also be built from source using `c-dependencies/spidermonkey/build-engine.sh`. That should only be required if you want to modify the engine itself, however.

Once that is done, the runtime and the CLI tool for applying it to JS source code can be built using cargo:

```sh
cargo build --release
```

## Testing

The JS Compute Runtime doesn't currently contain automated tests itself. Instead, Fastly runs an automated test suite for an internal repository for the JavaScript SDK using the runtime.

Manual testing is well supported, however. To test your changes, you can follow these steps:

1. Build the runtime's CLI tool, see above
2. Create a C@E service from a JS source file by running the CLI tool
3. Test the service using [Fastly's local testing server](https://developer.fastly.com/learning/compute/testing/#running-a-local-testing-server)

As an example, to turn a file `test.js` with this source code:

```js
addEventListener("fetch", (e) => {
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
