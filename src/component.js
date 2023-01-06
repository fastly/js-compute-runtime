import { componentNew } from 'js-component-tools';
import { readFile, writeFile } from 'node:fs/promises';

export async function compileComponent (path) {
  const coreComponent = await readFile(path);
  const wit = await readFile(new URL('../xqd.wit', import.meta.url), 'utf8');
  const generatedComponent = await componentNew(coreComponent, {
    adapters: [['wasi_snapshot_preview1', await readFile(new URL('../wasi_snapshot_preview1.wasm', import.meta.url))]],
    wit
  });
  await writeFile(path, generatedComponent);
}
