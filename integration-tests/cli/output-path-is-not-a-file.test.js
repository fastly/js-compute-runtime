import test from 'brittle';
import { getBinPath } from 'get-bin-path';
import { prepareEnvironment } from '@jakechampion/cli-testing-library';
import { ok } from 'node:assert';

const cli = await getBinPath({ name: 'js-compute' });

test('should return non-zero exit code', async function (t) {
  const { execute, cleanup, makeDir, writeFile } = await prepareEnvironment();
  t.teardown(async function () {
    await cleanup();
  });
  await makeDir('./bin/main.wasm');
  await writeFile('./bin/index.js', `addEventListener('fetch', function(){})`);
  const { code, stdout, stderr } = await execute(process.execPath, cli);

  t.alike(stdout, []);
  ok(
    stderr
      .toString()
      .startsWith('Error: The `output` path does not point to a file:'),
  );
  ok(stderr.toString().endsWith('main.wasm'));
  t.is(code, 1);
});
