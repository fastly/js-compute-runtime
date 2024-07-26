import test from 'brittle';
import { getBinPath } from 'get-bin-path'
import { prepareEnvironment } from '@jakechampion/cli-testing-library';

const cli = await getBinPath()

test('should create wasm file and return zero exit code for StarlingMonkey', async function (t) {
    const { execute, cleanup, writeFile, exists } = await prepareEnvironment();
    t.teardown(async function () {
        await cleanup();
    });

    await writeFile('./bin/index.js', `addEventListener('fetch', function(){})`)
    
    t.is(await exists('./bin/main.wasm'), false)

    const { code, stdout, stderr } = await execute(process.execPath, cli + ' --disable-starlingmonkey');

    t.is(await exists('./bin/main.wasm'), true)
    t.alike(stdout, ['Using the js-compute-runtime.wasm engine']);
    t.alike(stderr, []);
    t.is(code, 0);
});
