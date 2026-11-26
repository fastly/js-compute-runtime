import test from 'brittle';
import assert from 'node:assert/strict';
import { spawnSync } from 'node:child_process';
import { mkdtemp, readdir, rm, stat, writeFile } from 'node:fs/promises';
import { tmpdir } from 'node:os';
import { join } from 'node:path';
import { fileURLToPath } from 'node:url';
import { installWasmtime } from '../../scripts/install-wasmtime.js';

const testUnix = process.platform === 'win32' ? test.skip : test;

testUnix(
  'existing Wasmtime binary is made executable without downloading',
  async function (t) {
    const destination = await mkdtemp(join(tmpdir(), 'wasmtime-test-'));
    t.teardown(() => rm(destination, { recursive: true, force: true }));
    const binary = join(destination, 'wasmtime');
    await writeFile(binary, 'existing binary', { mode: 0o600 });

    await installWasmtime({ destination, platform: 'linux', arch: 'x64' });

    t.is((await stat(binary)).mode & 0o777, 0o755);
  },
);

test('rejects a bad checksum before extracting or installing', async function (t) {
  const destination = await mkdtemp(join(tmpdir(), 'wasmtime-test-'));
  t.teardown(() => rm(destination, { recursive: true, force: true }));
  const originalFetch = globalThis.fetch;
  globalThis.fetch = async () => new Response('not a valid release archive');
  try {
    await assert.rejects(
      installWasmtime({ destination, platform: 'linux', arch: 'x64' }),
      /SHA-256 checksum mismatch/,
    );
    t.alike(await readdir(destination), []);
  } finally {
    globalThis.fetch = originalFetch;
  }
});

test('postinstall can be skipped without downloading', function (t) {
  const script = fileURLToPath(
    new URL('../../scripts/install-wasmtime.js', import.meta.url),
  );
  const result = spawnSync(process.execPath, [script], {
    encoding: 'utf8',
    env: { ...process.env, FASTLY_SKIP_WASMTIME_INSTALL: '1' },
  });
  t.is(result.status, 0);
  t.ok(result.stdout.includes('Skipping Wasmtime installation'));
});
