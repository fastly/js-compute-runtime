// This test verifies that a reusable sandbox is retired successfully after its idle timeout.

import assert from 'node:assert/strict';
import { execFileSync, spawnSync } from 'node:child_process';
import { mkdtemp, rm } from 'node:fs/promises';
import { tmpdir } from 'node:os';
import { join } from 'node:path';
import { fileURLToPath } from 'node:url';

const directory = await mkdtemp(join(tmpdir(), 'js-compute-idle-'));

try {
  const wasm = join(directory, 'main.wasm');
  const cli = fileURLToPath(
    new URL('../../dist/cli/js-compute-runtime-cli.js', import.meta.url),
  );
  const flags = [
    ...(process.argv.includes('--debug-build') ? ['--debug-build'] : []),
    ...(process.argv.includes('--aot') ? [] : ['--disable-aot']),
  ];
  const testFile = fileURLToPath(
    new URL('./fixtures/reusable-sandboxes/idle-timeout.js', import.meta.url),
  );
  execFileSync(process.execPath, [cli, ...flags, testFile, wasm], {
    stdio: 'inherit',
  });

  const run = spawnSync('viceroy', ['run', wasm], {
    encoding: 'utf8',
    timeout: 30_000,
  });
  assert.ifError(run.error);
  assert.equal(
    run.status,
    0,
    `Viceroy run failed:\n${run.stdout}\n${run.stderr}`,
  );
} finally {
  await rm(directory, { recursive: true, force: true });
}
