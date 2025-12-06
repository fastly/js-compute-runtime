import { dirname, resolve, sep, normalize } from 'node:path';
import { tmpdir, freemem } from 'node:os';
import { spawnSync, type SpawnSyncOptionsWithStringEncoding } from 'node:child_process';
import {
  mkdir,
  readFile,
  mkdtemp,
  writeFile,
  copyFile,
} from 'node:fs/promises';
import { rmSync } from 'node:fs';
import weval from '@bytecodealliance/weval';
import wizer from '@bytecodealliance/wizer';

import { isDirectory, isFile } from './isFile.js';
import { postbundle } from './postbundle.js';
import { bundle } from './bundle.js';
import { composeSourcemaps, ExcludePattern } from './composeSourcemaps.js';

const maybeWindowsPath =
  process.platform === 'win32'
    ? (path: string) => {
        return '//?/' + path.replace(/\\/g, '/');
      }
    : (path: string) => path;

async function getTmpDir() {
  return await mkdtemp(normalize(tmpdir() + sep));
}

export type CompileApplicationToWasmParams = {
  input: string,
  output: string,
  wasmEngine: string,
  enableHttpCache: boolean,
  enableExperimentalHighResolutionTimeMethods: boolean,
  enableAOT: boolean,
  aotCache: string,
  enableStackTraces: boolean,
  excludeSources: boolean,
  debugIntermediateFilesDir: string | undefined,
  moduleMode: boolean,
  doBundle: boolean,
  env: Record<string, string>,
};

export async function compileApplicationToWasm(params: CompileApplicationToWasmParams) {

  const {
    output,
    wasmEngine,
    enableHttpCache = false,
    enableExperimentalHighResolutionTimeMethods = false,
    enableAOT = false,
    aotCache = '',
    enableStackTraces,
    excludeSources,
    debugIntermediateFilesDir,
    moduleMode = false,
    doBundle = false,
    env,
  } = params;

  let input = params.input;

  try {
    if (!(await isFile(input))) {
      console.error(
        `Error: The \`input\` path does not point to a file: ${input}`,
      );
      process.exit(1);
    }
  } catch {
    console.error(
      `Error: The \`input\` path points to a non-existent file: ${input}`,
    );
    process.exit(1);
  }

  try {
    await readFile(input, { encoding: 'utf-8' });
  } catch (maybeError: unknown) {
    const error = maybeError instanceof Error ? maybeError : new Error(String(maybeError));
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
  } catch {
    console.error(
      `Error: The \`wasmEngine\` path points to a non-existent file: ${wasmEngine}`,
    );
    process.exit(1);
  }

  // If output exists already, make sure it's not a directory
  // (we'll try to overwrite it if it's a file)
  try {
    if (await isDirectory(output)) {
      console.error(
        `Error: The \`output\` path points to a directory: ${output}`,
      );
      process.exit(1);
    }
  } catch {
    // Output doesn't exist
  }

  try {
    await mkdir(dirname(output), {
      recursive: true,
    });
  } catch (maybeError: unknown) {
    const error =
      maybeError instanceof Error ? maybeError : new Error(String(maybeError));
    console.error(
      `Error: Failed to create the \`output\` (${dirname(output)}) directory: ${output}`,
      error.message,
    );
    process.exit(1);
  }

  if (debugIntermediateFilesDir != null) {
    try {
      console.log(
        `Preparing \`debug-intermediate-files\` directory: ${debugIntermediateFilesDir}`,
      );
      await mkdir(debugIntermediateFilesDir, {
        recursive: true,
      });
    } catch (maybeError: unknown) {
      const error = maybeError instanceof Error ? maybeError : new Error(String(maybeError));
      console.error(
        `Error: Failed to create the \`debug-intermediate-files\` (${debugIntermediateFilesDir}) directory`,
        error.message,
      );
      process.exit(1);
    }
  }

  let tmpDir;
  if (doBundle) {
    tmpDir = await getTmpDir();

    const sourceMaps = [];

    const bundleFilename = '__input_bundled.js';
    const bundleOutputFilePath = resolve(tmpDir, bundleFilename);

    // esbuild respects input source map, works if it's linked via sourceMappingURL
    // either inline or as separate file
    try {
      await bundle(input, bundleOutputFilePath, {
        moduleMode,
        enableStackTraces,
      });
    } catch (maybeError: unknown) {
      const error = maybeError instanceof Error ? maybeError : new Error(String(maybeError));
      console.error(`Error:`, error.message);
      process.exit(1);
    }

    if (debugIntermediateFilesDir != null) {
      await copyFile(
        bundleOutputFilePath,
        resolve(debugIntermediateFilesDir, '__1_bundled.js'),
      );
      if (enableStackTraces) {
        await copyFile(
          bundleOutputFilePath + '.map',
          resolve(debugIntermediateFilesDir, '__1_bundled.js.map'),
        );
      }
    }
    if (enableStackTraces) {
      sourceMaps.push({ f: bundleFilename, s: bundleOutputFilePath + '.map' });
    }

    const postbundleFilename = '__fastly_post_bundle.js';
    const postbundleOutputFilepath = resolve(tmpDir, postbundleFilename);

    await postbundle(bundleOutputFilePath, postbundleOutputFilepath, {
      moduleMode,
      enableStackTraces,
    });

    if (debugIntermediateFilesDir != null) {
      await copyFile(
        postbundleOutputFilepath,
        resolve(debugIntermediateFilesDir, '__2_postbundled.js'),
      );
      if (enableStackTraces) {
        await copyFile(
          postbundleOutputFilepath + '.map',
          resolve(debugIntermediateFilesDir, '__2_postbundled.js.map'),
        );
      }
    }
    if (enableStackTraces) {
      sourceMaps.push({
        f: postbundleFilename,
        s: postbundleOutputFilepath + '.map',
      });
    }

    if (enableStackTraces) {
      // Compose source maps
      const replaceSourceMapToken = '__FINAL_SOURCE_MAP__';
      let excludePatterns: ExcludePattern[] = ['forbid-entry:/**', 'node_modules/**'];
      if (excludeSources) {
        excludePatterns = [() => true];
      }
      const composed = await composeSourcemaps(sourceMaps, excludePatterns);

      const outputWithSourcemaps = '__fastly_bundle_with_sourcemaps.js';
      const outputWithSourcemapsFilePath = resolve(
        tmpDir,
        outputWithSourcemaps,
      );

      const postBundleContent = await readFile(postbundleOutputFilepath, {
        encoding: 'utf-8',
      });
      const outputWithSourcemapsContent = postBundleContent.replace(
        replaceSourceMapToken,
        () => JSON.stringify(composed),
      );
      await writeFile(
        outputWithSourcemapsFilePath,
        outputWithSourcemapsContent,
      );

      if (debugIntermediateFilesDir != null) {
        await copyFile(
          outputWithSourcemapsFilePath,
          resolve(debugIntermediateFilesDir, 'fastly_bundle.js'),
        );
        await writeFile(
          resolve(debugIntermediateFilesDir, 'fastly_sourcemaps.json'),
          composed,
        );
      }

      // the output with sourcemaps is now the Wizer input
      input = outputWithSourcemapsFilePath;
    } else {
      // the bundled output is now the Wizer input
      input = postbundleOutputFilepath;
      if (debugIntermediateFilesDir != null) {
        await copyFile(
          postbundleOutputFilepath,
          resolve(debugIntermediateFilesDir, 'fastly_bundle.js'),
        );
      }
    }
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
      RUST_MIN_STACK: String(Math.max(8 * 1024 * 1024, Math.floor(freemem() * 0.1))),
    },
  } satisfies SpawnSyncOptionsWithStringEncoding;

  try {
    if (!doBundle) {
      if (enableAOT) {
        const wevalBin = await weval();

        const wevalProcess = spawnSync(
          `"${wevalBin}"`,
          [
            'weval',
            '-v',
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
        const wizerProcess = spawnSync(
          `"${wizer}"`,
          [
            '--allow-wasi',
            `--wasm-bulk-memory=true`,
            `--dir="${maybeWindowsPath(process.cwd())}"`,
            '--inherit-env=true',
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

        const wevalProcess = spawnSync(
          `"${wevalBin}"`,
          [
            'weval',
            '-v',
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
        const wizerProcess = spawnSync(
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
  } catch (maybeError: unknown) {
    const error = maybeError instanceof Error ? maybeError : new Error(String(maybeError));
    throw new Error(
      `Error: Failed to compile JavaScript to Wasm:\n${error.message}`,
    );
  } finally {
    if (doBundle && tmpDir != null) {
      rmSync(tmpDir, { recursive: true });
    }
  }
}
