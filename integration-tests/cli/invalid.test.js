import test from 'brittle';
import { getBinPath } from 'get-bin-path'
import { prepareEnvironment } from '@gmrchk/cli-testing-library';

const cli = await getBinPath()

test('should return non-zero exit code on syntax errors', async function (t) {
    const { execute, cleanup, writeFile } = await prepareEnvironment();
    t.teardown(async function () {
        await cleanup();
    });
    await writeFile('./bin/index.js', '@')
    const { code } = await execute('node', cli);
    
    t.is(code, 1);
});
