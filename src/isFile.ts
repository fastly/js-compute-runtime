import { stat } from 'node:fs/promises';

export async function isFile(path: string) {
  const stats = await stat(path);
  return stats.isFile();
}

export async function isDirectory(path: string) {
  const stats = await stat(path);
  return stats.isDirectory();
}
