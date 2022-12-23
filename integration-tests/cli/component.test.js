import test from 'brittle';
import { getBinPath } from 'get-bin-path'
import { prepareEnvironment } from '@jakechampion/cli-testing-library';
import { print } from 'js-component-tools';
import { resolve } from 'node:path';
import { readFile } from 'node:fs/promises';

const cli = await getBinPath()

test('should create wasm file and return zero exit code', async function (t) {
    const { execute, cleanup, path, writeFile, exists } = await prepareEnvironment();
    t.teardown(async function () {
        await cleanup();
    });

    await writeFile('./bin/index.js', `addEventListener('fetch', function(){})`)
    
    t.is(await exists('./bin/main.wasm'), false)

    const { code, stdout, stderr } = await execute(process.execPath, cli + ' --component');

    t.is(await exists('./bin/main.wasm'), true);

    // (necessary because readFile gives a string)
    const wasmBuffer = await readFile(resolve(path, './bin/main.wasm'));

    const wat = print(wasmBuffer);

    t.is(wat.slice(0, 10), '(component');
    t.alike(stdout, []);
    t.alike(stderr, []);
    t.is(code, 0);
});
