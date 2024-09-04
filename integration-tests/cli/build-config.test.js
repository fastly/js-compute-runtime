import test from 'brittle';
import { getBinPath } from 'get-bin-path';
import { prepareEnvironment } from '@jakechampion/cli-testing-library';

const cli = await getBinPath({ name: 'js-compute' });

test('should build the fastly condition', async function (t) {
  const { execute, cleanup, writeFile, exists, path } =
    await prepareEnvironment();
  t.teardown(async function () {
    await cleanup();
  });

  await writeFile('./index.js', `import '#test';`);
  await writeFile('./test.js', `addEventListener('fetch', function(){})`);
  await writeFile(
    './package.json',
    `{ "type": "module", "imports": { "#test": { "fastly": "./test.js" } } }`,
  );

  t.is(await exists('./app.wasm'), false);

  const { code, stdout, stderr } = await execute(
    process.execPath,
    `${cli} ${path}/index.js ${path}/app.wasm`,
  );

  t.is(await exists('./app.wasm'), true);
  t.alike(stdout, []);
  t.alike(stderr, []);
  t.is(code, 0);
});
