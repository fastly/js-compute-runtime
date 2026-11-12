import { fileURLToPath } from 'node:url';
import { dirname, join } from 'node:path';

const __dirname = dirname(fileURLToPath(import.meta.url));

export default async function wasmtime() {
  const binary = process.platform === 'win32' ? 'wasmtime.exe' : 'wasmtime';
  const binPath = join(__dirname, 'bin', binary);
  return binPath;
}
