import test from 'brittle';
import { getBinPath } from 'get-bin-path'
import { prepareEnvironment } from '@jakechampion/cli-testing-library';

const cli = await getBinPath()

test('should return non-zero exit code', async function (t) {
    const { execute, cleanup} = await prepareEnvironment();
    t.teardown(async function () {
        await cleanup();
    });

    const { code, stdout, stderr } = await execute(process.execPath, cli);

    t.alike(stdout, []);
    t.alike(stderr, ['Error: The `input` path points to a non-existant file: {{base}}/bin/index.js']);
    t.is(code, 1);
});
