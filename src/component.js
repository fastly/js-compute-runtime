import { componentNew, preview1AdapterReactorPath } from '@bytecodealliance/jco';
import { readFile, writeFile } from 'node:fs/promises';

export async function compileComponent (path, adapter) {
  const coreComponent = await readFile(path);
  if (!adapter) {
    adapter = preview1AdapterReactorPath();
  }
  const generatedComponent = await componentNew(coreComponent, [['wasi_snapshot_preview1', await readFile(adapter)]]);
  await writeFile(path, generatedComponent);
}
