import { dirname, basename, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { build } from 'esbuild';

import { moveFile } from '../files.js';
import { CompilerPipelineStep } from '../compilerPipeline.js';
import { fastlyPlugin } from '../esbuild-plugins/fastlyPlugin.js';
import { swallowTopLevelExportsPlugin } from '../esbuild-plugins/swallowTopLevelExportsPlugin.js';

const __filename = fileURLToPath(import.meta.url);
const __dirname = dirname(__filename);

// Compiler Step - bundle
// This step usually runs first.

// Runs esbuild:
// - bundles imported modules
// - applies the Fastly plugin, which resolves fastly:* imports
// - applies the Top level exports plugin, allowing top level file to contain any exports.
// - sets the named condition 'fastly'

// If stack traces are enabled:
// - injects 'trace-mapping.inject.js', which contains error mapping code
// - enables source maps and writes it as an external file

export const bundleStep: CompilerPipelineStep = {
  outFilename: '__input_bundled.js',
  async fn(ctx, index) {
    // esbuild respects input source map, works if it's linked via sourceMappingURL
    // either inline or as separate file
    try {
      const bundleFilename = basename(ctx.outFilepath);

      // Put build() output in cwd to build bundle and sourcemap with correct paths
      const outfile = resolve(bundleFilename);

      const plugins = [fastlyPlugin];
      if (ctx.moduleMode) {
        plugins.push(swallowTopLevelExportsPlugin({ entry: ctx.inFilepath }));
      }

      const inject = [];
      if (ctx.enableStackTraces) {
        inject.push(resolve(__dirname, '../../rsrc/trace-mapping.inject.js'));
      }

      await build({
        conditions: ['fastly'],
        entryPoints: [ctx.inFilepath],
        bundle: true,
        write: true,
        outfile,
        sourcemap: ctx.enableStackTraces ? 'external' : undefined,
        sourcesContent: ctx.enableStackTraces ? true : undefined,
        format: ctx.moduleMode ? 'esm' : 'iife',
        tsconfig: undefined,
        plugins,
        inject,
      });

      // Move build() output to outFilepath
      await moveFile(outfile, ctx.outFilepath);
      if (ctx.enableStackTraces) {
        await moveFile(outfile + '.map', ctx.outFilepath + '.map');
        ctx.sourceMaps.push({
          f: this.outFilename,
          s: ctx.outFilepath + '.map',
        });
      }
    } catch (maybeError: unknown) {
      const error =
        maybeError instanceof Error
          ? maybeError
          : new Error(String(maybeError));
      console.error(`Error:`, error.message);
      process.exit(1);
    }

    await ctx.maybeWriteDebugIntermediateFiles(`__${index + 1}_bundled.js`);
  },
};
