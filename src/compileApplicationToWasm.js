import { dirname } from "node:path";
import { spawnSync } from "node:child_process";
import { mkdir, readFile } from "node:fs/promises";
import { isFile } from "./isFile.js";
import { isFileOrDoesNotExist } from "./isFileOrDoesNotExist.js";
import wizer from "@bytecode-alliance/wizer";
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

/// Emit a block of javascript that will pre-compile the regular expressions given. As spidermonkey
/// will intern regular expressions, duplicating them at the top level and testing them with both
/// an ascii and utf8 string should ensure that they won't be re-compiled when run in the fetch
/// handler.
function precompile(literals) {
  return (
    PREAMBLE +
    literals
      .map((regex) => {
        return `precompile(/${regex.pattern}/${regex.flags});`;
      })
      .join("\n") +
    POSTAMBLE
  );
}

export async function compileApplicationToWasm(input, output, wasmEngine) {
  try {
    if (!(await isFile(input))) {
      console.error(
        `Error: The \`input\` path does not point to a file: ${input}`
      );
      process.exit(1);
    }
  } catch (error) {
    console.error(
      `Error: The \`input\` path points to a non-existant file: ${input}`
    );
    process.exit(1);
  }

  let inputContents;
  try {
    inputContents = await readFile(input, { encoding: "utf-8" });
  } catch (error) {
    console.error(
      "Error: Failed to open the `input` (${input})",
      error.message
    );
    process.exit(1);
  }
  // TODO: Enhancement - Check that input is valid JavaScript syntax before Wizening
  try {
    if (!(await isFile(wasmEngine))) {
      console.error(
        `Error: The \`wasmEngine\` path does not point to a file: ${wasmEngine}`
      );
      process.exit(1);
    }
  } catch (error) {
    console.error(
      `Error: The \`wasmEngine\` path points to a non-existant file: ${wasmEngine}`
    );
    process.exit(1);
  }
  try {
    await mkdir(dirname(output), {
      recursive: true,
    });
  } catch (error) {
    console.error(
      `Error: Failed to create the \`output\` (${output}) directory`,
      error.message
    );
    process.exit(1);
  }

  try {
    if (!(await isFileOrDoesNotExist(output))) {
      console.error(
        `Error: The \`output\` path does not point to a file: ${output}`
      );
      process.exit(1);
    }
  } catch (error) {
    console.error(
      `Error: Failed to check whether the \`output\` (${output}) is a file path`,
      error.message
    );
    process.exit(1);
  }

  let lits = findRegexLiterals(inputContents);

  if (lits.length != 0) {
    inputContents += precompile(lits);
  }

  try {
    let wizerProcess = spawnSync(
      wizer,
      [
        "--allow-wasi",
        `--dir=${process.cwd()}`,
        "-r _start=wizer.resume",
        `-o=${output}`,
        wasmEngine,
      ],
      {
        stdio: [null, process.stdout, process.stderr],
        input: inputContents,
        encoding: "utf-8",
      }
    );
    process.exitCode = wizerProcess.status;
  } catch (error) {
    console.error(`Error: Failed to compile JavaScript to Wasm`, error.message);
    process.exit(1);
  }
}
