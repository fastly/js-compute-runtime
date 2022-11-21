import test from 'brittle';
import { getBinPath } from 'get-bin-path'
import { prepareEnvironment } from '@gmrchk/cli-testing-library';

const cli = await getBinPath()

test('should create wasm file and return zero exit code', async function (t) {
    const { execute, cleanup, writeFile, exists } = await prepareEnvironment();
    t.teardown(async function () {
        await cleanup();
    });

    await writeFile('./bin/index.js', `addEventListener('fetch', function(){})`)
    
    t.is(await exists('./bin/main.wasm'), false)

    const { code, stdout, stderr } = await execute('node', cli);

    t.is(await exists('./bin/main.wasm'), true)
    t.alike(stdout, []);
    t.alike(stderr, []);
    t.is(code, 0);
});
