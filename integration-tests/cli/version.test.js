import test from 'brittle';
import { getBinPath } from 'get-bin-path'
import { prepareEnvironment } from '@gmrchk/cli-testing-library';

const cli = await getBinPath()

test('--version should return version number on stdout and zero exit code', async function (t) {
    const { execute, cleanup } = await prepareEnvironment();
    t.teardown(async function () {
        await cleanup();
    });
    const { code, stdout, stderr } = await execute('node', `${cli} --version`);

    t.is(code, 0);
    t.alike(stdout, ['js-compute-runtime 0.5.4'])
    t.alike(stderr, [])
});

test('-V should return version number on stdout and zero exit code', async function (t) {
    const { execute, cleanup } = await prepareEnvironment();
    t.teardown(async function () {
        await cleanup();
    });
    const { code, stdout, stderr } = await execute('node', `${cli} -V`);

    t.is(code, 0);
    t.alike(stdout, ['js-compute-runtime 0.5.4'])
    t.alike(stderr, [])
});
