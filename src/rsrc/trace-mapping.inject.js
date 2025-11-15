import { TraceMap, originalPositionFor } from "@jridgewell/trace-mapping";

let _traceMap;
function getTraceMap() {
  if (!_traceMap) {
    _traceMap = new TraceMap(globalThis.__FASTLY_SOURCE_MAP);
  }
  return _traceMap;
}

function buildErrorHeading(e) {
  const name = e?.name || 'Error';
  // Prefer e.cause.message if message is missing and cause is informative
  const msg =
    (typeof e?.message === 'string' && e.message) ||
    (e?.cause && typeof e.cause?.message === 'string' && e.cause.message) ||
    '';
  return msg ? `${name}: ${msg}` : name;
}

function getSourceContext(source, line, col, { radius = 3 } = {}) {
  const tm = getTraceMap();
  if (!tm) return null;

  const idx = tm.sources.indexOf(source);
  if (idx < 0) return null;

  const content = tm.sourcesContent?.[idx];
  if (!content) return null; // gracefully skip

  const lines = content.split('\n');

  const start = Math.max(0, line - 1 - radius);
  const end   = Math.min(lines.length - 1, line - 1 + radius);

  const result = [];

  for (let i = start; i <= end; i++) {
    const prefix = i === (line - 1) ? '>' : ' ';
    const num = String(i + 1).padStart(5);
    result.push(`${prefix} ${num} | ${lines[i]}`);
    if (i === line - 1) {
      result.push(`          ${''.padStart(col)}^`);
    }
  }

  return result;
}

function parseStarlingMonkeyFrame(line) {
  line = String(line).trim().replace(/\)?$/, ""); // strip trailing ')'
  const m = line.match(/:(\d+):(\d+)\s*$/);
  if (!m) return null;

  const col = +m[2];
  const lineNo = +m[1];
  const head = line.slice(0, m.index);
  if (head.startsWith('@')) { // no function name
    return { fn: null, file: head.slice(1), line: lineNo, col, };
  }
  const at = head.lastIndexOf("@");
  if (at > 0) { // fn@file
    return { fn: head.slice(0, at), file: head.slice(at + 1), line: lineNo, col, };
  }
  // file only
  return { fn: null, file: head, line: lineNo, col, };
}

function mapStack(raw) {
  const tm = getTraceMap();
  const out = [];
  const lines = String(raw).split(/\r?\n/);

  for (const line of lines) {
    if (line.startsWith('node_modules/@fastly/js-compute/src/rsrc/trace-mapping.inject.js')) {
      // If the line comes from this file, skip it
      continue;
    }
    if (line === '') { out.push({l: line}); continue; }

    const m = parseStarlingMonkeyFrame(line);
    if (!m) { out.push({e:'(frame not parsed)', l:line}); continue; }

    const { fn, file, line: l, col: c } = m;

    const genLine = Number(l);
    const genCol  = Number(c);

    // Only map frames that come from the generated bundle
    if (file !== globalThis.__FASTLY_GEN_FILE) { out.push({e:'(frame not mapped)', l:line}); continue; }

    const pos = originalPositionFor(tm, { line: genLine, column: Math.max(0, genCol - 1) });
    if (!pos?.source) {
      continue;
    }

    out.push({m:{pos,fn}, l: line});
  }

  return out;
}

function mapError(e) {
  const lines = [];

  const raw = e?.stack ?? String(e);

  lines.push(buildErrorHeading(e));
  try {
    const stack = mapStack(raw);
    let contextOutputted = false;
    for (const line of stack) {
      const { e, m, l } = line;
      if (l === '') {
        lines.push('');
        continue;
      }
      if (e != null) {
        lines.push(`${e} ${l}`);
        continue;
      }

      let formatted;
      let ctx;
      if (m == null) {
        formatted = l;
      } else {
        const {pos, fn} = m;

        let name = pos.name;
        if (fn == null || fn === '') {
          name = null;
        } else if (fn[fn.length-1] === '<') {
          name = '(anonymous function)';
        } else {
          name = fn;
        }

        const filePos = `${pos.source}:${pos.line}:${pos.column != null ? pos.column + 1 : 0}`;
        formatted = name ? `${name} (${filePos})` : filePos;
        if (!contextOutputted) {
          ctx = getSourceContext(pos.source, pos.line, pos.column);
          contextOutputted = true;
        }
      }
      lines.push(`  at ${formatted}`);
      if (ctx) {
        lines.push(...ctx);
      }
    }
  } catch {
    lines.push('(Raw error)');
    lines.push(raw);
  }
  return lines;
}

// Monkey patch addEventListener('fetch')
const _orig_addEventListener = globalThis.addEventListener;
globalThis.addEventListener = function (type, listener) {
  if (type !== 'fetch') {
    return _orig_addEventListener.call(this, type, listener);
  }

  const _orig_listener = listener;
  return _orig_addEventListener.call(this, type, (event) => {

    // Patch respondWith on this event instance
    const _orig_respondWith = event.respondWith.bind(event);
    event.respondWith = (value) => {
      const wrappedValue = Promise
        .resolve(value)
        .catch((err) => {
          console.error('Unhandled error while running request handler');
          try { globalThis.__fastlyMapAndLogError(err); } catch { /* swallow */ }
          console.error('Raw error below:');
          throw err;
        });
      try {
        return _orig_respondWith(wrappedValue);
      } catch (err) {
        console.error('Unhandled error sending response');
        try { globalThis.__fastlyMapAndLogError(err); } catch { /* swallow */ }
        console.error('Raw error below:');
        throw err;
      }
    };

    try {
      return _orig_listener.call(this, event);
    } catch (err) {
      console.error('Unhandled error running event listener');
      try { globalThis.__fastlyMapAndLogError(err); } catch { /* swallow */ }
      console.error('Raw error below:');
      throw err;
    }
  });
};

globalThis.__fastlyMapError = mapError
