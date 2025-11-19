import { fileURLToPath } from 'node:url';
import { dirname, join, isAbsolute } from 'node:path';
import { unknownArgument } from './unknownArgument.js';
import { tooManyEngines } from './tooManyEngines.js';
import { EnvParser } from './env.js';

export async function parseInputs(cliInputs) {
  const __dirname = dirname(fileURLToPath(import.meta.url));

  let enableHttpCache = false;
  let enableExperimentalHighResolutionTimeMethods = false;
  let enableAOT = false;
  let customEngineSet = false;
  let moduleMode = false;
  let bundle = true;
  let wasmEngine = join(__dirname, '../fastly.wasm');
  let aotCache = join(__dirname, '../fastly-ics.wevalcache');
  let customInputSet = false;
  let input = join(process.cwd(), 'bin/index.js');
  let customOutputSet = false;
  let output = join(process.cwd(), 'bin/main.wasm');
  let enableStackTraces = false;
  let excludeSources = false;
  let debugIntermediateFilesDir = undefined;
  let cliInput;

  const envParser = new EnvParser();

  // eslint-disable-next-line no-cond-assign
  loop: while ((cliInput = cliInputs.shift())) {
    switch (cliInput) {
      case '--': {
        break loop;
      }
      case '--env': {
        const value = cliInputs.shift();
        if (!value) {
          console.error('Error: --env requires a KEY=VALUE pair');
          process.exit(1);
        }
        // If value ends with comma, it's a continuation
        while (
          value.endsWith(',') &&
          cliInputs.length > 0 &&
          !cliInputs[0].startsWith('-')
        ) {
          value = value + cliInputs.shift();
        }
        envParser.parse(value);
        break;
      }
      case '--enable-experimental-high-resolution-time-methods': {
        enableExperimentalHighResolutionTimeMethods = true;
        break;
      }
      case '--module-mode': {
        moduleMode = true;
        bundle = false;
        break;
      }
      case '--enable-http-cache': {
        enableHttpCache = true;
        break;
      }
      case '--enable-experimental-top-level-await': {
        moduleMode = true;
        bundle = true;
        break;
      }
      case '--enable-aot': {
        enableAOT = true;
        break;
      }
      case '--enable-experimental-aot': {
        console.error(
          'Warning: --enable-experimental-aot flag is now --enable-aot. The old flag continues\n' +
            'to work for now, but please update your build invocation!',
        );
        enableAOT = true;
        break;
      }
      case '-V':
      case '--version': {
        return { version: true };
      }
      case '-h':
      case '--help': {
        return { help: true };
      }
      case '--starlingmonkey': {
        break;
      }
      case '--debug-build': {
        wasmEngine = join(__dirname, '../fastly.debug.wasm');
        console.log('Building with the debug engine');
        break;
      }
      case '--disable-starlingmonkey': {
        console.error(
          'The legacy js-compute-runtime.wasm engine requires an older version of the JS SDK',
        );
        process.exit(1);
      }
      case '--engine-wasm': {
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
      case '--aot-cache': {
        const value = cliInputs.shift();
        if (isAbsolute(value)) {
          aotCache = value;
        } else {
          aotCache = join(process.cwd(), value);
        }
        break;
      }
      case '--enable-stack-traces': {
        enableStackTraces = true;
        break;
      }
      case '--exclude-sources': {
        excludeSources = true;
        break;
      }
      case '--debug-intermediate-files': {
        const value = cliInputs.shift();
        if (isAbsolute(value)) {
          debugIntermediateFilesDir = value;
        } else {
          debugIntermediateFilesDir = join(process.cwd(), value);
        }
        break;
      }
      default: {
        if (cliInput.startsWith('--engine-wasm=')) {
          if (customEngineSet) {
            tooManyEngines();
          }
          const value = cliInput.replace(/--engine-wasm=+/, '');
          customEngineSet = true;
          if (isAbsolute(value)) {
            wasmEngine = value;
          } else {
            wasmEngine = join(process.cwd(), value);
          }
          break;
        } else if (cliInput.startsWith('--env=')) {
          const value = cliInput.replace(/--env=/, '');
          envParser.parse(value);
          break;
        } else if (cliInput.startsWith('-')) {
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

  if (!customEngineSet && enableAOT) {
    wasmEngine = join(__dirname, '../fastly-weval.wasm');
  }

  return {
    enableExperimentalHighResolutionTimeMethods,
    enableHttpCache,
    moduleMode,
    bundle,
    enableAOT,
    aotCache,
    enableStackTraces,
    excludeSources,
    debugIntermediateFilesDir,
    input,
    output,
    wasmEngine,
    env: envParser.getEnv(),
  };
}
