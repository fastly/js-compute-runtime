import test from 'brittle';
import { getBinPath } from 'get-bin-path';
import { prepareEnvironment } from '@jakechampion/cli-testing-library';
import { chmodSync } from 'node:fs';

const cli = await getBinPath({ name: 'js-compute' });

test('should use WEVAL_BIN when set and AOT is enabled', async function (t) {
  const { execute, cleanup, writeFile, exists, path } =
    await prepareEnvironment();
  t.teardown(async function () {
    delete process.env.WEVAL_BIN;
    await cleanup();
  });

  await writeFile('./index.js', `addEventListener('fetch', function(){})`);
  await writeFile('./dummy.wasm', '');

  const markerPath = `${path}/weval-bin-invoked`;
  const wrapperPath = `${path}/weval-wrapper.sh`;
  await writeFile(
    './weval-wrapper.sh',
    `#!/bin/sh
echo "invoked" > "${markerPath}"
exit 1
`,
  );
  chmodSync(wrapperPath, 0o755);

  process.env.WEVAL_BIN = wrapperPath;

  const { code } = await execute(
    process.execPath,
    `${cli} ${path}/index.js ${path}/out.wasm --enable-aot --engine-wasm ${path}/dummy.wasm`,
  );

  t.is(await exists('./weval-bin-invoked'), true);
  t.is(code, 1);
});
