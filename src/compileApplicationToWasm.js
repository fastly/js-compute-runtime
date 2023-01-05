import { dirname, parse, join } from "node:path";
import { spawnSync } from "node:child_process";
import { mkdir, readFile, writeFile } from "node:fs/promises";
import { isFile } from "./isFile.js";
import { isFileOrDoesNotExist } from "./isFileOrDoesNotExist.js";
import wizer from "@jakechampion/wizer";
import { precompile } from "./precompile.js";
import { bundle } from "./bundle.js";
import { containsSyntaxErrors } from "./containsSyntaxErrors.js";

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

  try {
    await readFile(input, { encoding: "utf-8" });
  } catch (error) {
    console.error(
      "Error: Failed to open the `input` (${input})",
      error.message
    );
    process.exit(1);
  }

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

  const outputDir = dirname(output);
  try {
    await mkdir(dirname(outputDir), {
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

  if (containsSyntaxErrors(input)) {
    process.exit(1);
  }
  let inputFileName = parse(input).base;
  let applicationPath = join(outputDir, inputFileName);

  let contents = await bundle(input, applicationPath);
  if (contents.errors.length) {
    for (const error of contents.errors) {
      console.error(error);
    }
    process.exit(1);
  }
  
  let index = contents.outputFiles.findIndex(a => a.path.endsWith(inputFileName))
  let app = contents.outputFiles[index];

  let application = precompile(app.text);
  delete app.text;
  app.text = application;
  for (const file of contents.outputFiles) {
    try {
      await writeFile(file.path, file.text, {
        encoding: 'utf-8'
      });
    } catch (error) {
      console.error(
        `Error: Failed to write to (${file.path})`,
        error.message
      );
      process.exit(1);
    }
  }

  try {
    let wizerProcess = spawnSync(
      wizer,
      [
        "--allow-wasi",
        `--dir=${process.cwd()}`,
        `--wasm-bulk-memory=true`,
        "-r _start=wizer.resume",
        `-o=${output}`,
        wasmEngine,
      ],
      {
        stdio: [null, process.stdout, process.stderr],
        input: applicationPath,
        shell: true,
        encoding: "utf-8",
      }
    );
    process.exitCode = wizerProcess.status;
  } catch (error) {
    console.error(`Error: Failed to compile JavaScript to Wasm`, error.message);
    process.exit(1);
  }
}
