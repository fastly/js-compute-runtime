import { stat } from 'node:fs/promises';

export async function isFileOrDoesNotExist(path) {
  try {
    const stats = await stat(path);
    return stats.isFile();
  } catch (error) {
    if (error instanceof Error && error.code === 'ENOENT') {
      return true;
    }
    throw error;
  }
}
