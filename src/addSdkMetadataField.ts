import { metadataAdd } from '@bytecodealliance/jco';
import { readFile, writeFile } from 'node:fs/promises';
import { dirname, join } from 'node:path';
import { fileURLToPath } from 'node:url';
const __dirname = dirname(fileURLToPath(import.meta.url));

export async function addSdkMetadataField(wasmPath: string, usingAOT: boolean) {
  const packageJson = await readFile(join(__dirname, '../package.json'), {
    encoding: 'utf-8',
  });

  const { name, version } = JSON.parse(packageJson) as { name: string, version: string, };

  let sdkName: string;
  if (usingAOT) {
    sdkName = name + ' (StarlingMonkey with Weval)';
  } else {
    sdkName = name + ' (StarlingMonkey)';
  }

  const metadata = [['sdk', [[sdkName, version]]]];
  const wasm = await readFile(wasmPath);
  // eslint-disable-next-line @typescript-eslint/no-unsafe-assignment
  const newWasm: Uint8Array = await metadataAdd(wasm, metadata);
  await writeFile(wasmPath, newWasm);
}
