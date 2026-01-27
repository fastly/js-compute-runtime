import { parse } from 'acorn';
import { simple as simpleWalk } from 'acorn-walk';
import regexpuc from 'regexpu-core';

import { CompilerPipelineStep } from '../compilerPipeline.js';

// Compiler Step - Precompile regexes
// This step runs any time after bundling

export const precompileRegexesStep: CompilerPipelineStep = {
  outFilename: '__fastly_precompiled_regexes.js',
  async fn(ctx, index) {
    await ctx.magicStringWriter(
      this.outFilename,
      async (magicString, source) => {
        // PRECOMPILE REGEXES
        // Emit a block of JavaScript that will pre-compile the regular expressions given.
        // As SpiderMonkey will intern regular expressions, duplicating them at the top
        // level and testing them with both an ascii and utf8 string should ensure that
        // they won't be re-compiled when run in the fetch handler.
        const PREAMBLE = `(function(){
  // Precompiled regular expressions
  const precompile = (r) => { r.exec('a'); r.exec('\\u1000'); };`;
        const POSTAMBLE = '})();';

        const ast = parse(source, {
          ecmaVersion: 'latest',
          sourceType: ctx.moduleMode ? 'module' : 'script',
        });

        const precompileCalls: string[] = [];
        simpleWalk(ast, {
          Literal(node) {
            if (!node.regex) return;
            let transpiledPattern;
            try {
              transpiledPattern = regexpuc(
                node.regex.pattern,
                node.regex.flags,
                {
                  unicodePropertyEscapes: 'transform',
                },
              );
            } catch {
              // swallow regex parse errors here to instead throw them at the engine level
              // this then also avoids regex parser bugs being thrown unnecessarily
              transpiledPattern = node.regex.pattern;
            }
            const transpiledRegex = `/${transpiledPattern}/${node.regex.flags}`;
            precompileCalls.push(`precompile(${transpiledRegex});`);
            magicString.overwrite(node.start, node.end, transpiledRegex);
          },
        });

        if (precompileCalls.length) {
          magicString.prepend(
            `${PREAMBLE}${precompileCalls.join('')}${POSTAMBLE}\n`,
          );
        }
      },
    );

    await ctx.maybeWriteDebugIntermediateFiles(
      `__${index + 1}_precompiled_regexes.js`,
    );
  },
};
