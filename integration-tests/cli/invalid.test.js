import test from 'brittle';
import { getBinPath } from 'get-bin-path'
import { prepareEnvironment } from '@gmrchk/cli-testing-library';

const cli = await getBinPath()

test('should bail on syntax errors', async function (t) {
    const { execute, cleanup, writeFile } = await prepareEnvironment();
    await writeFile('./bin/index.js', '@')
    const { code } = await execute(
        'node',
        cli
    );

    t.is(code, 1);

    await cleanup();
});
