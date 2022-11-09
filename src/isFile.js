import { stat } from "node:fs/promises";

export async function isFile(path) {
  const stats = await stat(path);
  return stats.isFile();
}
