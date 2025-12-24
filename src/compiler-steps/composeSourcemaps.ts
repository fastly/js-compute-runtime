import { readFile, writeFile } from 'node:fs/promises';
import { resolve } from 'node:path';
import remapping, {
  SourceMap,
  type SourceMapInput,
  type SourceMapLoader,
} from '@jridgewell/remapping';
import { TraceMap } from '@jridgewell/trace-mapping';
import picomatch from 'picomatch';

import { CompilerPipelineStep, SourceMapInfo } from '../compilerPipeline.js';

export type ExcludePattern = string | ((file: string) => boolean);

// Compiler Step - Compose Sourcemaps
// This step usually runs at the end.

// This step composes all the source maps up to this point into a single
// source map, and injects it into the bundle.

// Be careful: Do not run any steps after this step. Such steps will not be
// reflected in downstream source maps.

export const composeSourcemapsStep: CompilerPipelineStep = {
  outFilename: '__fastly_bundle_with_sourcemaps.js',
  async fn(ctx) {
    // Compose source maps
    const replaceSourceMapToken = '__FINAL_SOURCE_MAP__';
    let excludePatterns: ExcludePattern[] = [
      'forbid-entry:/**',
      'node_modules/**',
    ];
    if (ctx.excludeSources) {
      excludePatterns = [() => true];
    }
    const composed = await composeSourcemaps(ctx.sourceMaps, excludePatterns);

    const postBundleContent = await readFile(ctx.inFilepath, {
      encoding: 'utf-8',
    });
    const outputWithSourcemapsContent = postBundleContent.replace(
      replaceSourceMapToken,
      () => JSON.stringify(composed),
    );
    await writeFile(ctx.outFilepath, outputWithSourcemapsContent);

    if (ctx.debugIntermediateFilesDir != null) {
      await writeFile(
        resolve(ctx.debugIntermediateFilesDir, 'fastly_sourcemaps.json'),
        composed,
      );
    }
  },
};

async function readSourcemap(e: SourceMapInfo) {
  const sourceMapJson = await readFile(e.s, { encoding: 'utf-8' });
  return JSON.parse(sourceMapJson) as SourceMapInput;
}

export async function composeSourcemaps(
  sourceMaps: SourceMapInfo[],
  excludePatterns: ExcludePattern[] = [],
) {
  const topSourceMap = sourceMaps.pop();
  if (topSourceMap == null) {
    throw new Error(
      'Unexpected: composeSourcemaps received empty sourceMaps array.',
    );
  }

  const top = new TraceMap(await readSourcemap(topSourceMap));

  const priors: Record<string, SourceMapInput> = {};
  for (const sourceMap of sourceMaps) {
    priors[sourceMap.f] = await readSourcemap(sourceMap);
  }

  // Loader: given a source name from mapXZ, return a TraceMap for that source (if any).
  const loader: SourceMapLoader = (source) => {
    const m = priors[source];
    if (!m) return null; // no earlier map for this source
    return new TraceMap(m);
  };

  const raw = JSON.parse(remapping(top, loader, false).toString()) as SourceMap;

  return JSON.stringify(stripSourcesContent(raw, excludePatterns));
}

function stripSourcesContent(map: SourceMap, excludes: ExcludePattern[]) {
  if (map.sourcesContent == null) {
    return map;
  }

  const matchers = excludes.map((p) =>
    typeof p === 'string' ? picomatch(p) : p,
  );

  for (let i = 0; i < map.sources.length; i++) {
    const src = map.sources[i];

    // [Windows] normalize slashes
    const normalized = src?.replace(/\\/g, '/');

    if (normalized == null || matchers.some((fn) => fn(normalized))) {
      map.sourcesContent[i] = null;
    }
  }

  return map;
}
