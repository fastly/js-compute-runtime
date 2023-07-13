import { componentNew, preview1AdapterReactorPath } from '@bytecodealliance/jco';
import { readFile, writeFile } from 'node:fs/promises';

export async function compileComponent (path) {
  const coreComponent = await readFile(path);
  const generatedComponent = await componentNew(coreComponent, [['wasi_snapshot_preview1', await readFile(preview1AdapterReactorPath())]]);
  await writeFile(path, generatedComponent);
}
