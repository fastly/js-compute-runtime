import Parser, { Query } from "tree-sitter";
import JavaScript from "tree-sitter-javascript";

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
    regexLiterals.push({
      pattern: m.captures[0].node.text,
      flags: m.captures[1]?.node.text || "",
    });
  }
  return regexLiterals;
}

const PREAMBLE = `;{
  // Precompiled regular expressions
  const precompile = (r) => { r.exec('a'); r.exec('\\u1000'); };`;
const POSTAMBLE = "}";

// TODO: This should also detect and update sourcemaps if they are present, otherwise the sourcemaps would be incorrect.
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

  return (
    PREAMBLE +
    lits
      .map((regex) => {
        return `precompile(/${regex.pattern}/${regex.flags});`;
      })
      .join("\n") +
    POSTAMBLE + inputApplication
  );
}
