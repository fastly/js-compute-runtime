import { componentEmbed, componentNew } from '@bytecodealliance/jco';
import { readFile, writeFile } from 'node:fs/promises';

export async function compileComponent (path) {
  const coreComponent = await readFile(path);
  const wit = await readFile(new URL('../xqd.wit', import.meta.url), 'utf8');
  const coreComponentEmbedded = await componentEmbed(coreComponent, wit);
  const generatedComponent = await componentNew(coreComponentEmbedded, [['wasi_snapshot_preview1', await readFile(new URL('../wasi_snapshot_preview1.wasm', import.meta.url))]]);
  await writeFile(path, generatedComponent);
}
