import { dirname, resolve, sep, normalize } from "node:path";
import { tmpdir } from "node:os";
import { spawnSync } from "node:child_process";
import { mkdir, readFile, mkdtemp, writeFile } from "node:fs/promises";
import { rmSync } from "node:fs";
import { isFile } from "./isFile.js";
import { isFileOrDoesNotExist } from "./isFileOrDoesNotExist.js";
import wizer from "@bytecodealliance/wizer";
import { precompile } from "./precompile.js";
import { enableTopLevelAwait } from "./enableTopLevelAwait.js";
import { bundle } from "./bundle.js";
import { containsSyntaxErrors } from "./containsSyntaxErrors.js";

async function getTmpDir () {
  return await mkdtemp(normalize(tmpdir() + sep));
}

export async function compileApplicationToWasm(
  input,
  output,
  wasmEngine,
  enableExperimentalHighResolutionTimeMethods = false,
  enablePBL = false,
  enableExperimentalTopLevelAwait = false,
  starlingMonkey = false
) {
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
      `Error: Failed to open the \`input\` (${input})`,
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

  if (containsSyntaxErrors(input)) {
    process.exit(1);
  }

  let wizerInput, cleanup = () => {};

  let contents = await bundle(input, enableExperimentalTopLevelAwait);
  wizerInput = precompile(
    contents.outputFiles[0].text,
    undefined,
    enableExperimentalTopLevelAwait
  );
  if (enableExperimentalTopLevelAwait && !starlingMonkey) {
    wizerInput = enableTopLevelAwait(wizerInput);
  }

  // for StarlingMonkey, we need to write to a tmpdir
  if (starlingMonkey) {
    const tmpDir = await getTmpDir();
    const outPath = resolve(tmpDir, 'input.js');
    await writeFile(outPath, wizerInput);
    wizerInput = outPath;
    cleanup = () => {
      rmSync(tmpDir, { recursive: true });
    };
  }

  try {
    let wizerProcess = spawnSync(
      `"${wizer}"`,
      [
        "--inherit-env=true",
        "--allow-wasi",
        "--dir=.",
        ...starlingMonkey ? [`--dir=${dirname(wizerInput)}`] : [],
        `--wasm-bulk-memory=true`,
        "-r _start=wizer.resume",
        `-o="${output}"`,
        `"${wasmEngine}"`,
      ],
      {
        stdio: [null, process.stdout, process.stderr],
        input: wizerInput,
        shell: true,
        encoding: "utf-8",
        env: {
          ENABLE_EXPERIMENTAL_HIGH_RESOLUTION_TIME_METHODS:
            enableExperimentalHighResolutionTimeMethods ? "1" : "0",
          ENABLE_PBL: enablePBL ? "1" : "0",
          ...process.env,
        },
      }
    );
    if (wizerProcess.status !== 0) {
      throw new Error(`Wizer initialization failure`);
    }
    process.exitCode = wizerProcess.status;
  } catch (error) {
    console.error(
      `Error: Failed to compile JavaScript to Wasm: `,
      error.message
    );
    process.exit(1);
  } finally {
    cleanup();
  }
}
