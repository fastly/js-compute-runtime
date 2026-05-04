import { cd, $ as zx, retry, expBackoff } from 'zx';

export async function $(...args) {
  await new Promise((resolve) => setTimeout(resolve, 3000));
  return await retry(10, expBackoff('60s', '10s'), async () => zx(...args));
}
