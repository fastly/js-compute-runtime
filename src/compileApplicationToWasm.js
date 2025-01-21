import { dirname, resolve, sep, normalize } from 'node:path';
import { tmpdir } from 'node:os';
import { spawnSync } from 'node:child_process';
import { mkdir, readFile, mkdtemp, writeFile } from 'node:fs/promises';
import { rmSync } from 'node:fs';
import { isFile } from './isFile.js';
import { isFileOrDoesNotExist } from './isFileOrDoesNotExist.js';
import wizer from '@bytecodealliance/wizer';
import weval from '@bytecodealliance/weval';
import { precompile } from './precompile.js';
import { bundle } from './bundle.js';

const maybeWindowsPath =
  process.platform === 'win32'
    ? (path) => {
        return '//?/' + path.replace(/\\/g, '/');
      }
    : (path) => path;

async function getTmpDir() {
  return await mkdtemp(normalize(tmpdir() + sep));
}

export async function compileApplicationToWasm(
  input,
  output,
  wasmEngine,
  enableHttpCache = false,
  enableExperimentalHighResolutionTimeMethods = false,
  enableAOT = false,
  aotCache = '',
  moduleMode = false,
  doBundle = false,
  env,
) {
  try {
    if (!(await isFile(input))) {
      console.error(
        `Error: The \`input\` path does not point to a file: ${input}`,
      );
      process.exit(1);
    }
  } catch (error) {
    console.error(
      `Error: The \`input\` path points to a non-existent file: ${input}`,
    );
    process.exit(1);
  }

  try {
    await readFile(input, { encoding: 'utf-8' });
  } catch (error) {
    console.error(
      `Error: Failed to open the \`input\` (${input})`,
      error.message,
    );
    process.exit(1);
  }

  try {
    if (!(await isFile(wasmEngine))) {
      console.error(
        `Error: The \`wasmEngine\` path does not point to a file: ${wasmEngine}`,
      );
      process.exit(1);
    }
  } catch (error) {
    console.error(
      `Error: The \`wasmEngine\` path points to a non-existent file: ${wasmEngine}`,
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
      error.message,
    );
    process.exit(1);
  }

  try {
    if (!(await isFileOrDoesNotExist(output))) {
      console.error(
        `Error: The \`output\` path does not point to a file: ${output}`,
      );
      process.exit(1);
    }
  } catch (error) {
    console.error(
      `Error: Failed to check whether the \`output\` (${output}) is a file path`,
      error.message,
    );
    process.exit(1);
  }

  let tmpDir;
  if (doBundle) {
    let contents;
    try {
      contents = await bundle(input, moduleMode);
    } catch (error) {
      console.error(`Error:`, error.message);
      process.exit(1);
    }

    const precompiled = precompile(
      contents.outputFiles[0].text,
      undefined,
      moduleMode,
    );

    tmpDir = await getTmpDir();
    const outPath = resolve(tmpDir, 'input.js');
    await writeFile(outPath, precompiled);

    // the bundled output is now the Wizer input
    input = outPath;
  }

  const spawnOpts = {
    stdio: [null, process.stdout, process.stderr],
    input: maybeWindowsPath(input),
    shell: true,
    encoding: 'utf-8',
    env: {
      ...env,
      ENABLE_EXPERIMENTAL_HIGH_RESOLUTION_TIME_METHODS:
        enableExperimentalHighResolutionTimeMethods ? '1' : '0',
      ENABLE_EXPERIMENTAL_HTTP_CACHE: enableHttpCache ? '1' : '0',
    },
  };

  try {
    if (!doBundle) {
      // assert(moduleMode);
      if (enableAOT) {
        const wevalBin = await weval();

        let wevalProcess = spawnSync(
          `"${wevalBin}"`,
          [
            'weval',
            ...(aotCache ? [`--cache-ro ${aotCache}`] : []),
            `--dir="${maybeWindowsPath(process.cwd())}"`,
            '-w',
            `-i "${wasmEngine}"`,
            `-o "${output}"`,
          ],
          spawnOpts,
        );
        if (wevalProcess.status !== 0) {
          throw new Error(`Weval initialization failure`);
        }
        process.exitCode = wevalProcess.status;
      } else {
        let wizerProcess = spawnSync(
          `"${wizer}"`,
          [
            '--allow-wasi',
            `--wasm-bulk-memory=true`,
            `--dir="${maybeWindowsPath(process.cwd())}"`,
            '-r _start=wizer.resume',
            `-o="${output}"`,
            `"${wasmEngine}"`,
          ],
          spawnOpts,
        );
        if (wizerProcess.status !== 0) {
          throw new Error(`Wizer initialization failure`);
        }
        process.exitCode = wizerProcess.status;
      }
    } else {
      spawnOpts.input = `${maybeWindowsPath(input)}${moduleMode ? '' : ' --legacy-script'}`;
      if (enableAOT) {
        const wevalBin = await weval();

        let wevalProcess = spawnSync(
          `"${wevalBin}"`,
          [
            'weval',
            ...(aotCache ? [`--cache-ro ${aotCache}`] : []),
            '--dir .',
            `--dir ${maybeWindowsPath(dirname(input))}`,
            '-w',
            `-i "${wasmEngine}"`,
            `-o "${output}"`,
          ],
          spawnOpts,
        );
        if (wevalProcess.status !== 0) {
          throw new Error(`Weval initialization failure`);
        }
        process.exitCode = wevalProcess.status;
      } else {
        let wizerProcess = spawnSync(
          `"${wizer}"`,
          [
            '--inherit-env=true',
            '--allow-wasi',
            '--dir=.',
            `--dir=${maybeWindowsPath(dirname(input))}`,
            '-r _start=wizer.resume',
            `--wasm-bulk-memory=true`,
            `-o="${output}"`,
            `"${wasmEngine}"`,
          ],
          spawnOpts,
        );
        if (wizerProcess.status !== 0) {
          throw new Error(`Wizer initialization failure`);
        }
        process.exitCode = wizerProcess.status;
      }
    }
  } catch (error) {
    console.error(
      `Error: Failed to compile JavaScript to Wasm: `,
      error.message,
    );
    process.exit(1);
  } finally {
    if (doBundle) {
      rmSync(tmpDir, { recursive: true });
    }
  }
}
