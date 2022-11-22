import test from 'brittle';
import { getBinPath } from 'get-bin-path'
import { prepareEnvironment } from '@gmrchk/cli-testing-library';

const cli = await getBinPath()

test('--help should return help on stdout and zero exit code', async function (t) {
    const { execute, cleanup } = await prepareEnvironment();
    t.teardown(async function () {
        await cleanup();
    });
    
    t.is(code, 0);
    t.alike(stdout, [
        'js-compute-runtime 0.5.4',
        'USAGE:',
        'js-compute-runtime [FLAGS] [OPTIONS] [ARGS]',
        'FLAGS:',
        '-h, --help                      Prints help information',
        '-V, --version                   Prints version information',
        'OPTIONS:',
        '--engine-wasm <engine-wasm>    The JS engine Wasm file path',
        'ARGS:',
        '<input>     The input JS script\'s file path [default: bin/index.js]',
        '<output>    The file path to write the output Wasm module to [default: bin/main.wasm]'
    ])
    t.alike(stderr, [])
});

test('-h should return help on stdout and zero exit code', async function (t) {
    const { execute, cleanup } = await prepareEnvironment();
    t.teardown(async function () {
        await cleanup();
    });
    const { code, stdout, stderr } = await execute('node',`${cli} -h`);
    
    t.is(code, 0);
    t.alike(stdout, [
        'js-compute-runtime 0.5.4',
        'USAGE:',
        'js-compute-runtime [FLAGS] [OPTIONS] [ARGS]',
        'FLAGS:',
        '-h, --help                      Prints help information',
        '-V, --version                   Prints version information',
        'OPTIONS:',
        '--engine-wasm <engine-wasm>    The JS engine Wasm file path',
        'ARGS:',
        '<input>     The input JS script\'s file path [default: bin/index.js]',
        '<output>    The file path to write the output Wasm module to [default: bin/main.wasm]'
    ])
    t.alike(stderr, [])
});
