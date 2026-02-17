import { readFile } from 'node:fs/promises';
import { basename, dirname, join } from 'node:path';
import { argv } from 'node:process';
import { fileURLToPath } from 'node:url';
const __dirname = dirname(fileURLToPath(import.meta.url));

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
    --weval-bin <weval-bin>                                 Path to the weval binary to use for AOT compilation             

ARGS:
    <input>     The input JS script's file path [default: bin/index.js]
    <output>    The file path to write the output Wasm module to [default: bin/main.wasm]
`);
}

export async function printVersion() {
  const packageJson = await readFile(join(__dirname, '../package.json'), {
    encoding: 'utf-8',
  });
  const version = (JSON.parse(packageJson) as { version: string }).version;
  console.log(`${basename(argv[1])} ${version}`);
}

export function tooManyEngines() {
  console.error(`error: The argument '--engine-wasm <engine-wasm>' was provided more than once, but cannot be used multiple times
  
USAGE:
    js-compute-runtime --engine-wasm <engine-wasm>
      
For more information try --help`);
  process.exit(1);
}

export function unknownArgument(cliInput: string) {
  console.error(`error: Found argument '${cliInput}' which wasn't expected, or isn't valid in this context

USAGE:
    js-compute-runtime [FLAGS] [OPTIONS] [ARGS]

For more information try --help`);
  process.exit(1);
}
