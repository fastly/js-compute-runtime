// This test verifies that a reusable sandbox is retired successfully after its idle timeout.

import assert from 'node:assert/strict';
import { execFileSync, spawn } from 'node:child_process';
import { mkdtemp, rm } from 'node:fs/promises';
import { tmpdir } from 'node:os';
import { join } from 'node:path';
import { setTimeout as sleep } from 'node:timers/promises';
import { fileURLToPath } from 'node:url';

const directory = await mkdtemp(join(tmpdir(), 'js-compute-idle-'));
let server;
let log = '';
let serverError;

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

  server = spawn('viceroy', ['serve', '-v', '--addr', '127.0.0.1:0', wasm], {
    env: { ...process.env, RUST_LOG: 'info' },
  });
  server.stdout.on('data', (chunk) => (log += chunk.toString()));
  server.stderr.on('data', (chunk) => (log += chunk.toString()));
  server.on('error', (error) => (serverError = error));

  async function waitFor(pattern) {
    const deadline = Date.now() + 30_000;
    while (!pattern.test(log)) {
      if (serverError) throw serverError;
      if (
        server.exitCode !== null ||
        server.signalCode !== null ||
        /Exited with i32 exit status [1-9]\d*/.test(log) ||
        Date.now() >= deadline
      ) {
        throw new Error(`Viceroy did not complete the test:\n${log}`);
      }
      await sleep(100);
    }
    return log.match(pattern);
  }

  const [, url] = await waitFor(/Listening on (http:\/\/127\.0\.0\.1:\d+)/);
  for (let i = 0; i < 2; i++) {
    const response = await fetch(url, { signal: AbortSignal.timeout(10_000) });
    assert.equal(response.status, 200);
    assert.equal(await response.text(), 'ok\n');
  }
  await waitFor(/Exiting process after handling 2 requests\./);
  await waitFor(/request completed in /);
  assert.doesNotMatch(log, /Exited with i32 exit status [1-9]\d*/);
  console.log('Reusable sandbox retired successfully after its idle timeout.');
} finally {
  if (server?.pid && server.exitCode === null && server.signalCode === null) {
    const exited = new Promise((resolve) => server.once('close', resolve));
    server.kill();
    await exited;
  }
  await rm(directory, { recursive: true, force: true });
}
