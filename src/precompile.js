import regexpuc from "regexpu-core";
import { parse } from "acorn";
import MagicString from "magic-string";
import { simple as simpleWalk } from "acorn-walk";

const PREAMBLE = `;{
  // Precompiled regular expressions
  const precompile = (r) => { r.exec('a'); r.exec('\\u1000'); };`;
const POSTAMBLE = "}";

/// Emit a block of javascript that will pre-compile the regular expressions given. As spidermonkey
/// will intern regular expressions, duplicating them at the top level and testing them with both
/// an ascii and utf8 string should ensure that they won't be re-compiled when run in the fetch
/// handler.
export function precompile(source, filename = "<input>") {
  const magicString = new MagicString(source, {
    filename,
  });

  const ast = parse(source, {
    ecmaVersion: "latest",
    sourceType: "module",
  });

  const precompileCalls = [];
  simpleWalk(ast, {
    Literal(node) {
      if (!node.regex) return;
      let transpiledPattern;
      try {
        transpiledPattern = regexpuc(node.regex.pattern, node.regex.flags, {
          unicodePropertyEscapes: "transform",
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

  if (!precompileCalls.length) return source;

  magicString.prepend(`${PREAMBLE}${precompileCalls.join("\n")}${POSTAMBLE}`);

  // When we're ready to pipe in source maps:
  // const map = magicString.generateMap({
  //   source: 'source.js',
  //   file: 'converted.js.map',
  //   includeContent: true
  // });

  return magicString.toString();
}
