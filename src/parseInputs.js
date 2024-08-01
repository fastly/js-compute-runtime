import { fileURLToPath } from "node:url";
import { dirname, join, isAbsolute } from "node:path";
import { unknownArgument } from "./unknownArgument.js";
import { tooManyEngines } from "./tooManyEngines.js";

export async function parseInputs(cliInputs) {
  const __dirname = dirname(fileURLToPath(import.meta.url));

  let enableExperimentalHighResolutionTimeMethods = false;
  let enableExperimentalTopLevelAwait = false;
  let starlingMonkey = true;
  let enablePBL = false;
  let enableAOT = false;
  let customEngineSet = false;
  let wasmEngine = join(__dirname, "../starling.wasm");
  let aotCache = join(__dirname, "../starling-ics.wevalcache");
  let customInputSet = false;
  let input = join(process.cwd(), "bin/index.js");
  let customOutputSet = false;
  let output = join(process.cwd(), "bin/main.wasm");
  let cliInput;

  // eslint-disable-next-line no-cond-assign
  loop: while ((cliInput = cliInputs.shift())) {
    switch (cliInput) {
      case "--": {
        break loop;
      }
      case "--enable-experimental-high-resolution-time-methods": {
        enableExperimentalHighResolutionTimeMethods = true;
        break;
      }
      case "--enable-experimental-top-level-await": {
        enableExperimentalTopLevelAwait = true;
        break;
      }
      case "--enable-pbl": {
        enablePBL = true;
        break;
      }
      case "--enable-experimental-aot": {
        enableAOT = true;
        break;
      }
      case "-V":
      case "--version": {
        return { version: true };
      }
      case "-h":
      case "--help": {
        return { help: true };
      }
      case "--starlingmonkey": {
        break;
      }
      case "--disable-starlingmonkey": {
        starlingMonkey = false;
        wasmEngine = join(__dirname, "../js-compute-runtime.wasm");
        console.log('Building with the js-compute-runtime.wasm engine');
        break;
      }
      case "--engine-wasm": {
        if (customEngineSet) {
          tooManyEngines();
        }
        const value = cliInputs.shift();
        customEngineSet = true;
        if (isAbsolute(value)) {
          wasmEngine = value;
        } else {
          wasmEngine = join(process.cwd(), value);
        }
        break;
      }
      case "--aot-cache": {
        if (isAbsolute(value)) {
          aotCache = value;
        } else {
          aotCache = join(process.cwd(), value);
        }
        break;
      }
      default: {
        // The reason this is not another `case` and is an `if` using `startsWith`
        // is because previous versions of the CLI allowed an arbitrary amount of
        // = characters to be present. E.G. This is valid --engine-wasm====js.wasm
        if (cliInput.startsWith("--engine-wasm=")) {
          if (customEngineSet) {
            tooManyEngines();
          }
          const value = cliInput.replace(/--engine-wasm=+/, "");
          // This is used to detect if multiple --engine-wasm flags have been set
          // which is not supported.
          customEngineSet = true;
          if (isAbsolute(value)) {
            wasmEngine = value;
          } else {
            wasmEngine = join(process.cwd(), value);
          }
          break;
        } else if (cliInput.startsWith("-")) {
          unknownArgument(cliInput);
        } else {
          if (!customInputSet) {
            customInputSet = true;
            if (isAbsolute(cliInput)) {
              input = cliInput;
            } else {
              input = join(process.cwd(), cliInput);
            }
          } else if (!customOutputSet) {
            customOutputSet = true;
            if (isAbsolute(cliInput)) {
              output = cliInput;
            } else {
              output = join(process.cwd(), cliInput);
            }
          } else {
            unknownArgument(cliInput);
          }
        }
      }
    }
  }

  if (!starlingMonkey && enableAOT) {
    // enableAOT requires StarlingMonkey.
    console.log("AOT option is not compatible with pre-StarlingMonkey engine; please use StarlingMonkey.");
    process.exit(1);
  }
  if (!customEngineSet && enableAOT) {
      wasmEngine = join(__dirname, "../starling-weval.wasm");
  }

  return {
    enableExperimentalHighResolutionTimeMethods,
    enableExperimentalTopLevelAwait,
    enablePBL,
    enableAOT,
    aotCache,
    input,
    output,
    starlingMonkey,
    wasmEngine,
  };
}
