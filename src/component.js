import { componentEmbed, componentNew, preview1AdapterReactorPath } from '@bytecodealliance/jco';
import { readFile, writeFile } from 'node:fs/promises';

export async function compileComponent (path) {
  const coreComponent = await readFile(path);
  const wit = await readFile(new URL('../fastly.wit', import.meta.url), 'utf8');
  const coreComponentEmbedded = await componentEmbed(coreComponent, wit);
  const generatedComponent = await componentNew(coreComponentEmbedded, [['wasi_snapshot_preview1', await readFile(preview1AdapterReactorPath())]]);
  await writeFile(path, generatedComponent);
}
