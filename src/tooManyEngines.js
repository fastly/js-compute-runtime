export function tooManyEngines() {
  console.error(`error: The argument '--engine-wasm <engine-wasm>' was provided more than once, but cannot be used multiple times
  
USAGE:
    js-compute-runtime --engine-wasm <engine-wasm>
      
For more information try --help`);
  process.exit(1);
}
