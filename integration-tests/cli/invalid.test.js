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

  // TODO: do not even try to fix the mess below if something breaks with it again. Just remove
  // cli-testing-library dependency entirely and check stderr directly.
  try {
    strictEqual(
      stderr.join('').toString().replace(/\r?\n/g, '').slice(1),
      ` [ERROR] Expected identifier but found end of file
bin/index.js:4:9:
4 │ "hello";@
╵          ^
Error: Build failed with 1 error:
bin/index.js:4:9: ERROR: Expected identifier but found end of file`.replace(
        /\r?\n/g,
        '',
      ),
    );
  } catch {
    strictEqual(
      stderr.join('').toString().replace(/\r?\n/g, '').slice(1),
      ` [ERROR] Expected identifier but found end of file
bin/index.js:4:9:
4 │ "hello";@
╵^
Error: Build failed with 1 error:
bin/index.js:4:9: ERROR: Expected identifier but found end of file`.replace(
        /\r?\n/g,
        '',
      ),
    );
  }

  t.is(code, 1);
});
