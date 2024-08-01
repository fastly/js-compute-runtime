import test from 'brittle';
import { getBinPath } from 'get-bin-path'
import { prepareEnvironment } from '@jakechampion/cli-testing-library';

const cli = await getBinPath({name:"js-compute"})

test('should create output directory, wasm file and return zero exit code', async function (t) {
    const { execute, cleanup, writeFile, exists, path } = await prepareEnvironment();
    t.teardown(async function () {
        await cleanup();
    });

    await writeFile('./index.js', `addEventListener('fetch', function(){})`)
    
    t.is(await exists('./my/cool/app.wasm'), false)

    const { code, stdout, stderr } = await execute(process.execPath, `${cli} ${path}/index.js ${path}/my/cool/app.wasm`);

    t.is(await exists('./my/cool/app.wasm'), true)
    t.alike(stdout, []);
    t.alike(stderr, []);
    t.is(code, 0);
});
