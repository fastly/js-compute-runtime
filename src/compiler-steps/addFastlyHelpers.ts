import { CompilerPipelineStep } from '../compilerPipeline.js';

// Compiler Step - Add Fastly Helpers
// This step usually runs last before composing sourcemaps.

export const addFastlyHelpersStep: CompilerPipelineStep = {
  outFilename: '__fastly_helpers.js',
  async fn(ctx, index) {
    await ctx.magicStringWriter(this.outFilename, async (magicString) => {
      // MISC HEADER
      const SOURCE_FILE_NAME = 'fastly:app.js';
      const STACK_MAPPING_HEADER = `\
//# sourceURL=${SOURCE_FILE_NAME}
globalThis.__FASTLY_GEN_FILE = "${SOURCE_FILE_NAME}";
globalThis.__orig_console_error = console.error.bind(console);
globalThis.__fastlyMapAndLogError = (e) => {
  for (const line of globalThis.__fastlyMapError(e)) {
    globalThis.__orig_console_error(line);
  }
};
globalThis.__fastlyMapError = (e) => {
  return [
    '(Raw error) - build with --enable-stack-traces for mapped stack information.',
    e,
  ];   
};
`;
      magicString.prepend(STACK_MAPPING_HEADER);
    });

    await ctx.maybeWriteDebugIntermediateFiles(
      `__${index + 1}_fastly_helpers.js`,
    );
  },
};
