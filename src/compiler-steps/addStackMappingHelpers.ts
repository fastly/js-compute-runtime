import { CompilerPipelineStep } from '../compilerPipeline.js';

// Compiler Step - Add Stack Mapping Helpers
// This step runs only when stack tracing is enabled, and
// any time after bundling

export const addStackMappingHelpersStep: CompilerPipelineStep = {
  outFilename: '__stack_mapping_helpers.js',
  async fn(ctx, index) {
    await ctx.magicStringWriter(this.outFilename, async (magicString) => {
      // INSERT init guard
      let initGuardPre, initGuardPost;
      if (ctx.moduleMode) {
        initGuardPre = `\
await(async function __fastly_init_guard__() {
`;
        initGuardPost = `\
})().catch(e => {
console.error('Unhandled error while running top level module code');
try { globalThis.__fastlyMapAndLogError(e); } catch { /* swallow */ }
console.error('Raw error below:');
throw e;
});
`;
      } else {
        initGuardPre = `\
(function __fastly_init_guard__() { try {
`;
        initGuardPost = `\
} catch (e) {
console.error('Unhandled error while running top level script');
try { globalThis.__fastlyMapAndLogError(e); } catch { /* swallow */ }
console.error('Raw error below:');
throw e;
}
})();
`;
      }

      magicString.prepend(initGuardPre);
      magicString.append(initGuardPost);

      // SOURCE MAPPING HEADER
      const STACK_MAPPING_HEADER = `\
globalThis.__FASTLY_SOURCE_MAP = JSON.parse(__FINAL_SOURCE_MAP__);
`;
      magicString.prepend(STACK_MAPPING_HEADER);
    });

    await ctx.maybeWriteDebugIntermediateFiles(
      `__${index + 1}_stack_mapping_helpers.js`,
    );
  },
};
