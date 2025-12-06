import { dirname, isAbsolute, resolve } from 'node:path';
import { type Plugin } from 'esbuild';

export type SwallowTopLevelExportsPluginParams = {
  entry?: string;
};

export function swallowTopLevelExportsPlugin(opts?: SwallowTopLevelExportsPluginParams) {
  const { entry } = opts ?? {};

  const name = 'swallow-top-level-exports';
  const namespace = 'swallow-top-level';
  if (!entry) throw new Error(`[${name}] You must provide opts.entry`);

  // Normalize once so our onResolve comparison is exact.
  const normalizedEntry = resolve(entry);

  return {
    name,
    setup(build) {
      build.onResolve({ filter: /.*/ }, (args) => {
        const maybeEntry = isAbsolute(args.path)
          ? args.path
          : resolve(args.resolveDir || process.cwd(), args.path);
        if (args.kind === 'entry-point' && maybeEntry === normalizedEntry) {
          return { path: normalizedEntry, namespace };
        }
        return null;
      });
      build.onLoad({ filter: /.*/, namespace }, (args) => {
        // Generate a JS wrapper that imports the real entry
        // This swallows the top level exports for the entry file
        // and runs any side effects, such as addEventListener().
        return {
          contents: `import ${JSON.stringify(args.path)};`,
          loader: 'js',
          resolveDir: dirname(args.path),
        };
      });
    },
  } satisfies Plugin;
}
