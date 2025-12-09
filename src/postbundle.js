import { readFile, writeFile } from 'node:fs/promises';
import { basename } from 'node:path';
import regexpuc from 'regexpu-core';
import { parse } from 'acorn';
import MagicString from 'magic-string';
import { simple as simpleWalk } from 'acorn-walk';

export async function postbundle(
  input,
  outfile,
  { moduleMode = false, enableStackTraces = false },
) {
  const source = await readFile(input, { encoding: 'utf8' });
  const magicString = new MagicString(source);

  // PRECOMPILE REGEXES
  // Emit a block of javascript that will pre-compile the regular expressions given. As spidermonkey
  // will intern regular expressions, duplicating them at the top level and testing them with both
  // an ascii and utf8 string should ensure that they won't be re-compiled when run in the fetch
  // handler.
  const PREAMBLE = `(function(){
  // Precompiled regular expressions
  const precompile = (r) => { r.exec('a'); r.exec('\\u1000'); };`;
  const POSTAMBLE = '})();';

  const ast = parse(source, {
    ecmaVersion: 'latest',
    sourceType: moduleMode ? 'module' : 'script',
  });

  const precompileCalls = [];
  simpleWalk(ast, {
    Literal(node) {
      if (!node.regex) return;
      let transpiledPattern;
      try {
        transpiledPattern = regexpuc(node.regex.pattern, node.regex.flags, {
          unicodePropertyEscapes: 'transform',
        });
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
    magicString.prepend(`${PREAMBLE}${precompileCalls.join('')}${POSTAMBLE}\n`);
  }

  if (enableStackTraces) {
    // INSERT init guard
    let initGuardPre, initGuardPost;
    if (moduleMode) {
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
  }

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

  await writeFile(outfile, magicString.toString());

  if (enableStackTraces) {
    const map = magicString.generateMap({
      source: basename(input),
      hires: true,
      includeContent: true,
    });

    await writeFile(outfile + '.map', map.toString());
  }
}
