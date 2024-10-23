import { basename } from 'node:path';
import { argv } from 'node:process';
import { printVersion } from './printVersion.js';

export async function printHelp() {
  await printVersion();
  console.log(`
USAGE:
    ${basename(argv[1])} [FLAGS] [OPTIONS] [ARGS]

FLAGS:
    -h, --help                                              Prints help information
    -V, --version                                           Prints version information

OPTIONS:
    --engine-wasm <engine-wasm>                             The JS engine Wasm file path
    --module-mode                            [experimental] Run all sources as native modules,
                                                            with full error stack support.
    --bundle                                                Runs the esbuild bundler (enabled by default, unless using module mode).
    --enable-experimental-high-resolution-time-methods      Enable experimental high-resolution fastly.now() method

ARGS:
    <input>     The input JS script's file path [default: bin/index.js]
    <output>    The file path to write the output Wasm module to [default: bin/main.wasm]
`);
}
