import { dirname, resolve, sep, normalize } from "node:path";
import { tmpdir } from "node:os";
import { spawnSync } from "node:child_process";
import { mkdir, readFile, mkdtemp, writeFile } from "node:fs/promises";
import { rmSync } from "node:fs";
import { isFile } from "./isFile.js";
import { isFileOrDoesNotExist } from "./isFileOrDoesNotExist.js";
import wizer from "@bytecodealliance/wizer";
import weval from "@cfallin/weval";
import { precompile } from "./precompile.js";
import { bundle } from "./bundle.js";

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
  enableAOT = false,
  aotCache = '',
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
      `Error: The \`input\` path points to a non-existent file: ${input}`
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
      `Error: The \`wasmEngine\` path points to a non-existent file: ${wasmEngine}`
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

  let wizerInput, cleanup = () => {};

  let contents;
  try {
    contents = await bundle(input, enableExperimentalTopLevelAwait);
  } catch (error) {
    console.error(
      `Error:`,
      error.message
    );
    process.exit(1);
  }

  wizerInput = precompile(
    contents.outputFiles[0].text,
    undefined,
    enableExperimentalTopLevelAwait
  );

  // for StarlingMonkey, we need to write to a tmpdir pending streaming source hooks or similar
  const tmpDir = await getTmpDir();
  const outPath = resolve(tmpDir, 'input.js');
  await writeFile(outPath, wizerInput);
  wizerInput = outPath;
  cleanup = () => {
    rmSync(tmpDir, { recursive: true });
  };

  try {
    if (enableAOT) {
      const wevalBin = await weval();

      let wevalProcess = spawnSync(
        `"${wevalBin}"`,
        [
          "weval",
          ...aotCache ? [`--cache-ro ${aotCache}`] : [],
          "--dir .",
          `--dir ${dirname(wizerInput)}`,
          "-w",
          `-i "${wasmEngine}"`,
          `-o "${output}"`,
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
      if (wevalProcess.status !== 0) {
        throw new Error(`Weval initialization failure`);
      }
      process.exitCode = wevalProcess.status;
    } else {
      let wizerProcess = spawnSync(
        `"${wizer}"`,
        [
          "--inherit-env=true",
          "--allow-wasi",
          "--dir=.",
          `--dir=${dirname(wizerInput)}`,
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
    }
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
