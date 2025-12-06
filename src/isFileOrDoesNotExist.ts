import { stat } from 'node:fs/promises';

export async function isFileOrDoesNotExist(path: string) {
  try {
    const stats = await stat(path);
    return stats.isFile();
  } catch (error: unknown) {
    if (error instanceof Error && 'code' in error && error.code === 'ENOENT') {
      return true;
    }
    throw error;
  }
}
