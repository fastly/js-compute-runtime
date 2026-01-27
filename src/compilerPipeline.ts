import { copyFile, readFile, writeFile } from 'node:fs/promises';
import { basename, resolve } from 'node:path';
import MagicString from 'magic-string';
import { pipeline } from './pipeline.js';

export type SourceMapInfo = {
  f: string; // Filename
  s: string; // Sourcemap filename
};

export class CompilerContext {
  inFilepath: string;
  outFilepath: string;
  tmpDir: string;
  debugIntermediateFilesDir: string | undefined;
  sourceMaps: SourceMapInfo[];
  moduleMode: boolean;
  enableStackTraces: boolean;
  excludeSources: boolean;
  compilerPipelineSteps: CompilerPipelineStep[];

  constructor(
    input: string,
    tmpDir: string,
    debugIntermediateFilesDir: string | undefined,
    moduleMode: boolean,
    enableStackTraces: boolean,
    excludeSources: boolean,
  ) {
    this.inFilepath = input;
    this.outFilepath = ''; // This is filled in by the eventual steps of the compiler
    this.tmpDir = tmpDir;
    this.debugIntermediateFilesDir = debugIntermediateFilesDir;
    this.sourceMaps = [];
    this.moduleMode = moduleMode;
    this.enableStackTraces = enableStackTraces;
    this.excludeSources = excludeSources;
    this.compilerPipelineSteps = [];
  }

  addCompilerPipelineStep(step: CompilerPipelineStep) {
    this.compilerPipelineSteps.push(step);
  }

  async applyCompilerPipeline() {
    await pipeline<CompilerContext>(
      this.compilerPipelineSteps.map(
        (step) => async (args: CompilerContext, index) => {
          await step.fn.call(step, args, index);
          return args;
        },
      ),
      this,
      {
        beforeStep: (args: CompilerContext, index: number) => {
          const step = this.compilerPipelineSteps[index];
          args.outFilepath = resolve(this.tmpDir, step.outFilename);
        },
        afterStep: (args: CompilerContext) => {
          args.inFilepath = args.outFilepath;
        },
      },
    );
  }

  async magicStringWriter(
    filename: string,
    fn: (magicString: MagicString, source: string) => void | Promise<void>,
  ) {
    const source = await readFile(this.inFilepath, { encoding: 'utf8' });
    const magicString = new MagicString(source);

    await fn(magicString, source);

    await writeFile(this.outFilepath, magicString.toString());

    if (this.enableStackTraces != null) {
      const map = magicString.generateMap({
        source: basename(this.inFilepath),
        hires: true,
        includeContent: true,
      });

      await writeFile(this.outFilepath + '.map', map.toString());
      this.sourceMaps.push({ f: filename, s: this.outFilepath + '.map' });
    }
  }

  async maybeWriteDebugIntermediateFile(outFilename: string) {
    if (this.debugIntermediateFilesDir != null) {
      await copyFile(
        this.outFilepath,
        resolve(this.debugIntermediateFilesDir, outFilename),
      );
    }
  }

  async maybeWriteDebugIntermediateSourceMapFile(outFilename: string) {
    if (this.enableStackTraces && this.debugIntermediateFilesDir != null) {
      await copyFile(
        this.outFilepath + '.map',
        resolve(this.debugIntermediateFilesDir, outFilename),
      );
    }
  }

  async maybeWriteDebugIntermediateFiles(outFilename: string) {
    await this.maybeWriteDebugIntermediateFile(outFilename);
    await this.maybeWriteDebugIntermediateSourceMapFile(outFilename + '.map');
  }
}

export type CompilerPipelineStep = {
  outFilename: string;
  fn: (
    this: CompilerPipelineStep,
    args: CompilerContext,
    index: number,
  ) => void | PromiseLike<void>;
};
