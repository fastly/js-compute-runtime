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
    --env <KEY=VALUE>                                       Set environment variables, possibly inheriting
                                                           from the current environment. Multiple
                                                           variables can be comma-separated 
                                                           (e.g., --env ENV_VAR,OVERRIDE=val)
    --module-mode                            [experimental] Run all sources as native modules,
                                                           with full error stack support.
    --engine-wasm <engine-wasm>                             The JS engine Wasm file path
                                                            with full error stack support
    --enable-http-cache                                     Enable the HTTP cache hook API
    --enable-aot                                            Enable AOT compilation for performance
    --enable-experimental-high-resolution-time-methods      Enable experimental fastly.now() method
    --enable-experimental-top-level-await                   Enable experimental top level await
    --enable-stack-traces                                   Enable stack traces
    --exclude-sources                                       Don't include sources in stack traces                
    --debug-intermediate-files <dir>                        Output intermediate files in directory                

ARGS:
    <input>     The input JS script's file path [default: bin/index.js]
    <output>    The file path to write the output Wasm module to [default: bin/main.wasm]
`);
}
