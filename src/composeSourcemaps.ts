import { readFile } from 'node:fs/promises';
import remapping, {
  SourceMap,
  type SourceMapInput,
  type SourceMapLoader,
} from '@jridgewell/remapping';
import { TraceMap } from '@jridgewell/trace-mapping';
import picomatch from 'picomatch';

export type ExcludePattern = string | ((file: string) => boolean);

export type SourceMapInfo = {
  f: string, // Filename
  s: string, // Sourcemap filename
};

async function readSourcemap(e: SourceMapInfo) {
  const sourceMapJson = await readFile(e.s, { encoding: 'utf-8' });
  return JSON.parse(sourceMapJson) as SourceMapInput;
}

export async function composeSourcemaps(sourceMaps: SourceMapInfo[], excludePatterns: ExcludePattern[] = []) {
  const topSourceMap = sourceMaps.pop();
  if (topSourceMap == null) {
    throw new Error('Unexpected: composeSourcemaps received empty sourceMaps array.');
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
