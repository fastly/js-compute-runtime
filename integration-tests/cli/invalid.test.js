import test from 'brittle';
import { getBinPath } from 'get-bin-path';
import { prepareEnvironment } from '@jakechampion/cli-testing-library';
import { strictEqual } from 'node:assert';

const cli = await getBinPath({ name: 'js-compute' });

test('should return non-zero exit code on syntax errors', async function (t) {
  const { execute, cleanup, writeFile } = await prepareEnvironment();
  t.teardown(async function () {
    await cleanup();
  });
  await writeFile('./bin/index.js', '\n\n\n"hello";@');
  const { code, stdout, stderr } = await execute(process.execPath, cli);
  t.alike(stdout, []);
  strictEqual(
    stderr.join('\n').toString().replace(/\r\n/g, '\n'),
    `✘ [ERROR] Expected identifier but found end of file
bin/index.js:4:9:
4 │ "hello";@
╵          ^
Error: Build failed with 1 error:
bin/index.js:4:9: ERROR: Expected identifier but found end of file`.replace(
      /\r\n/g,
      '\n',
    ),
  );

  t.is(code, 1);
});
