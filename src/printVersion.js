import { readFile } from "node:fs/promises";
import { fileURLToPath } from 'node:url';
import { dirname, join } from 'node:path';
const __dirname = dirname(fileURLToPath(import.meta.url));

export async function printVersion() {
  const packageJson = await readFile(join(__dirname, '/package.json'), { encoding: 'utf-8' });
  const version = JSON.parse(packageJson).version;
  console.log(`js-compute-runtime ${version}`);
}
