#!/usr/bin/env node

import { parseInputs } from './src/parseInputs.js'
import { printVersion } from "./src/printVersion.js";
import { printHelp } from "./src/printHelp.js";
import { addSdkMetadataField } from "./src/addSdkMetadataField.js";

const {wasmEngine, input, component, output, version, help} = await parseInputs(process.argv.slice(2))

if (version) {
  await printVersion();
} else if (help) {
  await printHelp();
} else {
  // This is a dynamic import because this import will throw an error
  // if if does not have a pre-compiled version of Wizer available the platform 
  // running the CLI. In that situation, we would still like the 
  // js-compute-runtime cli's --version and --help flags to work as
  // it could be that the user is using an older version of js-compute-runtime
  // and a newer version does support the platform they are using.
  const {compileApplicationToWasm} = await import('./src/compileApplicationToWasm.js')
  await compileApplicationToWasm(input, output, wasmEngine);
  if (component) {
    const {compileComponent} = await import('./src/component.js');
    await compileComponent(output);
  }
  await addSdkMetadataField(output);
}
