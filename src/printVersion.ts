import { readFile } from 'node:fs/promises';
import { basename, dirname, join } from 'node:path';
import { argv } from 'node:process';
import { fileURLToPath } from 'node:url';
const __dirname = dirname(fileURLToPath(import.meta.url));

export async function printVersion() {
  const packageJson = await readFile(join(__dirname, '../package.json'), {
    encoding: 'utf-8',
  });
  const version = (JSON.parse(packageJson) as { version: string }).version;
  console.log(`${basename(argv[1])} ${version}`);
}
