import { stat, rename, copyFile, unlink } from 'node:fs/promises';
import { resolve } from 'node:path';

export async function isFile(path: string) {
  const stats = await stat(path);
  return stats.isFile();
}

export async function isDirectory(path: string) {
  const stats = await stat(path);
  return stats.isDirectory();
}

export async function moveFile(src: string, dest: string): Promise<void> {
  try {
    await rename(src, dest);
  } catch (err: unknown) {
    if (!isErrnoException(err) || err.code !== 'EXDEV') {
      throw err;
    }

    // Cross-device move: copy + delete
    await copyFile(src, dest);
    await unlink(src);
  }
}

function isErrnoException(err: unknown): err is NodeJS.ErrnoException {
  return (
    typeof err === 'object' &&
    err !== null &&
    'code' in err &&
    typeof err.code === 'string'
  );
}
