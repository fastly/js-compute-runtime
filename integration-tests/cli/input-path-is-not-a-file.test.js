import test from 'brittle';
import { getBinPath } from 'get-bin-path';
import { prepareEnvironment } from '@jakechampion/cli-testing-library';
import { ok } from 'node:assert';

const cli = await getBinPath({ name: 'js-compute' });

test('should return non-zero exit code', async function (t) {
  const { execute, cleanup, makeDir } = await prepareEnvironment();
  t.teardown(async function () {
    await cleanup();
  });
  await makeDir('./bin/index.js');
  const { code, stdout, stderr } = await execute(process.execPath, cli);

  t.alike(stdout, []);
  ok(
    stderr
      .toString()
      .startsWith('Error: The `input` path does not point to a file:'),
  );
  ok(stderr.toString().endsWith('index.js'));
  t.is(code, 1);
});
