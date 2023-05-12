import { fileURLToPath } from "node:url";
import { dirname, join, isAbsolute } from "node:path";
import { unknownArgument } from "./unknownArgument.js";
import { tooManyEngines } from "./tooManyEngines.js";

export async function parseInputs(cliInputs) {
  const __dirname = dirname(fileURLToPath(import.meta.url));

  let component = false;
  let enableExperimentalHighResolutionTimeMethods = false;
  let enableWeval = false;
  let customEngineSet = false;
  let wasmEngineBase = join(__dirname, "../js-compute-runtime");
  let wasmEngine;
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
      case "--enable-weval": {
        enableWeval = true;
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
      case "--component": {
        component = true;
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

  if (!wasmEngine) {
    wasmEngine = wasmEngineBase;
    if (component) {
      wasmEngine += "-component";
    }
    if (enableWeval) {
      wasmEngine += "-weval.wasm";
    } else {
      wasmEngine += "-normal.wasm";
    }
  }
  return { wasmEngine, component, input, output, enableExperimentalHighResolutionTimeMethods, enableWeval };
}
