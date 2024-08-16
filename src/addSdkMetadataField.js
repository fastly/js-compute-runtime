import { metadataAdd } from '@bytecodealliance/jco';
import { readFile, writeFile } from 'node:fs/promises';
import { dirname, join } from "node:path";
import { fileURLToPath } from "node:url";
const __dirname = dirname(fileURLToPath(import.meta.url));

export async function addSdkMetadataField(wasmPath, usingPBL) {
  const packageJson = await readFile(join(__dirname, "../package.json"), {
    encoding: "utf-8",
  });

  let { name, version } = JSON.parse(packageJson);

  if (usingPBL) {
    name += ' (StarlingMonkey with PBL)'
  } else {
    name += ' (StarlingMonkey)'
  }

  const metadata = [
    [
      "sdk",
      [
        [`${name}`, version],
      ],
    ],
  ];
  const wasm = await readFile(wasmPath);
  const newWasm = await metadataAdd(wasm, metadata);
  await writeFile(wasmPath, newWasm);
}
