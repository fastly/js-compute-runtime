import { parse } from "acorn";
import MagicString from "magic-string";
import { simple as simpleWalk } from "acorn-walk";

/// Emit a block of javascript that enables top level awaits
/// * removes any top-level exports
/// * wraps source with async function and executes it
export function enableTopLevelAwait(source, filename = "<input>") {
  const magicString = new MagicString(source, {
    filename,
  });

  const ast = parse(source, {
    ecmaVersion: "latest",
    sourceType: "module",
  });

  function replaceExportWithDeclaration(node) {
    let body = '';
    if (node.declaration != null) {
      // The following may have declarations:
      // ExportNamedDeclaration, e.g.:
      //   export var i = 0;
      //   export const y = foo();
      // ExportDefaultDeclaration, e.g.:
      //   export default 1;
      //   export default foo();
      body = magicString
        .snip(node.declaration.start, node.declaration.end)
        .toString();
    }
    magicString.overwrite(node.start, node.end, body);
  }

  simpleWalk(ast, {
    ExportNamedDeclaration(node) {
      replaceExportWithDeclaration(node);
    },
    ExportSpecifier(node) {
      replaceExportWithDeclaration(node);
    },
    ExportDefaultDeclaration(node) {
      replaceExportWithDeclaration(node);
    },
    ExportAllDeclaration(node) {
      replaceExportWithDeclaration(node);
    },
  });

  magicString.prepend(';((async()=>{');
  magicString.append('})())');

  // When we're ready to pipe in source maps:
  // const map = magicString.generateMap({
  //   source: 'source.js',
  //   file: 'converted.js.map',
  //   includeContent: true
  // });

  return magicString.toString();

}

