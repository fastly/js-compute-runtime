import test from 'brittle';
import { getBinPath } from 'get-bin-path'
import { prepareEnvironment } from '@gmrchk/cli-testing-library';

const cli = await getBinPath()

test('should return non-zero exit code on syntax errors', async function (t) {
    const { execute, cleanup, writeFile } = await prepareEnvironment();
    t.teardown(async function () {
        await cleanup();
    });
    await writeFile('./bin/index.js', '\n\n\n"hello";@')
    const { code, stdout, stderr } = await execute(process.execPath, cli);
    t.alike(stdout, []);
    t.alike(stderr, [
        '{{base}}/bin/index.js:4',
        '"hello";@',
        '^',
        'SyntaxError: Invalid or unexpected token'
    ]);

    t.is(code, 1);
});
