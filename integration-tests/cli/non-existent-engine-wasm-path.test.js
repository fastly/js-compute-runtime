import test from 'brittle';
import { getBinPath } from 'get-bin-path';
import { prepareEnvironment } from '@jakechampion/cli-testing-library';
import { ok } from 'node:assert';

const cli = await getBinPath({ name: 'js-compute' });

test('should return non-zero exit code', async function (t) {
  const { execute, cleanup, writeFile, path } = await prepareEnvironment();
  t.teardown(async function () {
    await cleanup();
  });

  await writeFile('./bin/index.js', `addEventListener('fetch', function(){})`);
  const { code, stdout, stderr } = await execute(
    process.execPath,
    `${cli} --engine-wasm ${path}/engine.wasm`,
  );

  t.alike(stdout, []);
  ok(
    stderr
      .toString()
      .startsWith(
        'Error: The `wasmEngine` path points to a non-existent file:',
      ),
  );
  ok(stderr.toString().endsWith('engine.wasm'));
  t.is(code, 1);
});
