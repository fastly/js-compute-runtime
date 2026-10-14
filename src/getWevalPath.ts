import { createRequire } from 'node:module';
import weval from '@bytecodealliance/weval';
import lockfile from 'proper-lockfile';

const wevalModulePath = createRequire(import.meta.url).resolve(
  '@bytecodealliance/weval',
);

export async function getWevalPath(
  install: () => Promise<string> = weval,
  installationPath: string = wevalModulePath,
): Promise<string> {
  const release = await lockfile.lock(installationPath, {
    retries: { retries: 120, minTimeout: 1000, maxTimeout: 1000 },
  });

  try {
    return await install();
  } finally {
    await release();
  }
}
