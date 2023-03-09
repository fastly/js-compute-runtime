import Parser, { Query } from "tree-sitter";
import JavaScript from "tree-sitter-javascript";
import regexpuc from 'regexpu-core';

function findRegexLiterals(source) {
  const parser = new Parser();
  parser.setLanguage(JavaScript);

  const tree = parser.parse(source);
  const query = new Query(
    JavaScript,
    "(regex pattern: (regex_pattern) @pattern flags: (regex_flags)? @flags)"
  );
  const regexLiterals = [];
  for (const m of query.matches(tree.rootNode)) {
    const pattern = m.captures[0].node.text;
    const flags = m.captures[1]?.node.text || "";
    // transpile unicode property escapes
    const patternTranspiled = regexpuc(pattern, flags, { unicodePropertyEscapes: 'transform' });
    regexLiterals.push({
      patternStart: m.captures[0].node.startIndex,
      patternEnd: m.captures[0].node.endIndex,
      pattern,
      patternTranspiled,
      flags,
      flagsStart: m.captures[1]?.node.startIndex,
      flagsEnd: m.captures[1]?.node.endIndex,
    });
  }
  return regexLiterals;
}

const PREAMBLE = `;{
  // Precompiled regular expressions
  const precompile = (r) => { r.exec('a'); r.exec('\\u1000'); };`;
const POSTAMBLE = "}";

// TODO: This should also detect and update sourcemaps if they are present, otherwise the sourcemaps would be incorrect.
//       We could use https://github.com/rich-harris/magic-string to create and/or update sourcemaps
// 
/// Emit a block of javascript that will pre-compile the regular expressions given. As spidermonkey
/// will intern regular expressions, duplicating them at the top level and testing them with both
/// an ascii and utf8 string should ensure that they won't be re-compiled when run in the fetch
/// handler.
export function precompile(inputApplication) {
  let lits = findRegexLiterals(inputApplication);

  if (lits.length === 0) {
    return inputApplication;
  }

  let offset = 0;
  for (const lit of lits) {
    if (lit.pattern === lit.patternTranspiled)
      continue;
    inputApplication = inputApplication.slice(0, lit.patternStart + offset) + lit.patternTranspiled + inputApplication.slice(lit.patternEnd + offset);
    offset += lit.patternTranspiled.length - lit.pattern.length;
  }

  return (
    PREAMBLE +
    lits
      .map((regex) => {
        return `precompile(/${regex.patternTranspiled}/${regex.flags});`;
      })
      .join("\n") +
    POSTAMBLE + inputApplication
  );
}
