import { readFile } from 'node:fs/promises';
import remapping from '@jridgewell/remapping';
import { TraceMap } from '@jridgewell/trace-mapping';
import picomatch from "picomatch";

async function readSourcemap(e) {
  const sourceMapJson = await readFile(e.s, { encoding: 'utf-8' });
  return JSON.parse(sourceMapJson);
}

export async function composeSourcemaps(
  sourceMaps,
  excludePatterns = [],
) {

  const top = new TraceMap(await readSourcemap(sourceMaps.pop()));

  const priors = {};
  for (const sourceMap of sourceMaps) {
     priors[sourceMap.f] = await readSourcemap(sourceMap);
  }

  // Loader: given a source name from mapXZ, return a TraceMap for that source (if any).
  const loader = (source) => {
    const m = priors[source];
    if (!m) return null; // no earlier map for this source
    return new TraceMap(m);
  };

  const raw = JSON.parse(
    remapping(top, loader, false).toString()
  );

  return JSON.stringify(
    stripSourcesContent(raw, excludePatterns)
  );
}

function stripSourcesContent(map, excludes) {
  const matchers = excludes.map(p => typeof p === 'string' ? picomatch(p) : p);

  for (let i = 0; i < map.sources.length; i++) {
    const src = map.sources[i];

    // [Windows] normalize slashes
    const normalized = src.replace(/\\/g, "/");

    if (matchers.some(fn => fn(normalized))) {
      map.sourcesContent[i] = null;
    }
  }

  return map;
}
