import test from 'brittle';
import { getBinPath } from 'get-bin-path'
import { prepareEnvironment } from '@jakechampion/cli-testing-library';

const cli = await getBinPath({name:"js-compute"})

test('should return non-zero exit code', async function (t) {
    const { execute, cleanup, makeDir, writeFile, path} = await prepareEnvironment();
    t.teardown(async function () {
        await cleanup();
    });
    await makeDir('./engine.wasm')
    await writeFile('./bin/index.js', `addEventListener('fetch', function(){})`)
    const { code, stdout, stderr } = await execute(process.execPath, `${cli} --engine-wasm ${path}/engine.wasm`);

    t.alike(stdout, []);
    t.alike(stderr, ['Error: The `wasmEngine` path does not point to a file: {{base}}/engine.wasm']);
    t.is(code, 1);
});
