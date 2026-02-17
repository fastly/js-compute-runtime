#!/usr/bin/env node

import { parseInputs } from '../parseInputs.js';
import { printHelp, printVersion } from '../printHelp.js';
import { addSdkMetadataField } from '../addSdkMetadataField.js';

const parsedInputs = await parseInputs(process.argv.slice(2));

if (parsedInputs === 'version') {
  await printVersion();
} else if (parsedInputs === 'help') {
  await printHelp();
} else {
  const {
    enableAOT,
    aotCache,
    enableHttpCache,
    enableExperimentalHighResolutionTimeMethods,
    moduleMode,
    bundle,
    enableStackTraces,
    excludeSources,
    debugIntermediateFilesDir,
    wasmEngine,
    wevalBin,
    input,
    output,
    env,
  } = parsedInputs;

  // This is a dynamic import because this import will throw an error
  // if it does not have a pre-compiled version of Wizer available in the platform
  // running the CLI. In that situation, we would still like the
  // js-compute-runtime cli's --version and --help flags to work as
  // it could be that the user is using an older version of js-compute-runtime
  // and a newer version does not support the platform they are using.
  const { compileApplicationToWasm } = await import(
    '../compileApplicationToWasm.js'
  );
  await compileApplicationToWasm({
    input,
    output,
    wasmEngine,
    enableHttpCache,
    enableExperimentalHighResolutionTimeMethods,
    enableAOT,
    aotCache,
    enableStackTraces,
    excludeSources,
    debugIntermediateFilesDir,
    wevalBin,
    moduleMode,
    doBundle: bundle,
    env,
  });
  await addSdkMetadataField(output, enableAOT);
}
