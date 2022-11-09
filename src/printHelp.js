import { printVersion } from "./printVersion.js";

export async function printHelp() {
  await printVersion();
  console.log(`
USAGE:
    js-compute-runtime [FLAGS] [OPTIONS] [ARGS]

FLAGS:
    -h, --help                      Prints help information
    -V, --version                   Prints version information

OPTIONS:
        --engine-wasm <engine-wasm>    The JS engine Wasm file path

ARGS:
    <input>     The input JS script's file path [default: bin/index.js]
    <output>    The file path to write the output Wasm module to [default: bin/main.wasm]
`);
}
