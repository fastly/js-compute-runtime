import { rename } from 'node:fs/promises';
import { dirname, basename, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { build } from 'esbuild';

import { swallowTopLevelExportsPlugin } from './swallowTopLevelExportsPlugin.js';

const __filename = fileURLToPath(import.meta.url);
const __dirname = dirname(__filename);

const fastlyPlugin = {
  name: 'fastly',
  setup(build) {
    build.onResolve({ filter: /^fastly:.*/ }, (args) => {
      return {
        path: args.path.replace('fastly:', ''),
        namespace: 'fastly',
      };
    });
    build.onLoad({ filter: /^.*/, namespace: 'fastly' }, async (args) => {
      switch (args.path) {
        case 'acl': {
          return {
            contents: `export const Acl = globalThis.Acl;`,
          };
        }
        case 'backend': {
          return {
            contents: `
export const Backend = globalThis.Backend;
export const setDefaultDynamicBackendConfig = Object.getOwnPropertyDescriptor(globalThis.fastly, 'allowDynamicBackends').set;
const allowDynamicBackends = Object.getOwnPropertyDescriptor(globalThis.fastly, 'allowDynamicBackends').set;
export const setDefaultBackend = Object.getOwnPropertyDescriptor(globalThis.fastly, 'defaultBackend').set;
export function enforceExplicitBackends (defaultBackend) {
  allowDynamicBackends(false);
  if (defaultBackend) setDefaultBackend(defaultBackend);
}
`,
          };
        }
        case 'body': {
          return {
            contents: `export const FastlyBody = globalThis.FastlyBody;`,
          };
        }
        case 'cache-override': {
          return {
            contents: `export const CacheOverride = globalThis.CacheOverride;`,
          };
        }
        case 'config-store': {
          return {
            contents: `export const ConfigStore = globalThis.ConfigStore;`,
          };
        }
        case 'dictionary': {
          return {
            contents: `export const Dictionary = globalThis.Dictionary;`,
          };
        }
        case 'device': {
          return { contents: `export const Device = globalThis.Device;` };
        }
        case 'edge-rate-limiter': {
          return {
            contents: `
export const RateCounter = globalThis.RateCounter;
export const PenaltyBox = globalThis.PenaltyBox;
export const EdgeRateLimiter = globalThis.EdgeRateLimiter;
`,
          };
        }
        case 'env': {
          return { contents: `export const env = globalThis.fastly.env.get;` };
        }
        case 'experimental': {
          return {
            contents: `
export const includeBytes = globalThis.fastly.includeBytes;
export const enableDebugLogging = globalThis.fastly.enableDebugLogging;
export const setBaseURL = Object.getOwnPropertyDescriptor(globalThis.fastly, 'baseURL').set;
export const setDefaultBackend = Object.getOwnPropertyDescriptor(globalThis.fastly, 'defaultBackend').set;
export const allowDynamicBackends = Object.getOwnPropertyDescriptor(globalThis.fastly, 'allowDynamicBackends').set;
export const sdkVersion = globalThis.fastly.sdkVersion;
export const mapAndLogError = (e) => globalThis.__fastlyMapAndLogError(e);
export const mapError = (e) => globalThis.__fastlyMapError(e);
`,
          };
        }
        case 'fanout': {
          return {
            contents: `export const createFanoutHandoff = globalThis.fastly.createFanoutHandoff;`,
          };
        }
        case 'websocket': {
          return {
            contents: `export const createWebsocketHandoff = globalThis.fastly.createWebsocketHandoff;`,
          };
        }
        case 'geolocation': {
          return {
            contents: `export const getGeolocationForIpAddress = globalThis.fastly.getGeolocationForIpAddress;`,
          };
        }
        case 'logger': {
          return {
            contents: `export const Logger = globalThis.Logger;
export const configureConsole = Logger.configureConsole;
delete globalThis.Logger.configureConsole;
`,
          };
        }
        case 'kv-store': {
          return { contents: `export const KVStore = globalThis.KVStore;` };
        }
        case 'secret-store': {
          return {
            contents: `export const SecretStore = globalThis.SecretStore;export const SecretStoreEntry = globalThis.SecretStoreEntry;`,
          };
        }
        case 'cache': {
          return {
            contents: `
export const CacheEntry = globalThis.CacheEntry;
export const CacheState = globalThis.CacheState;
export const CoreCache = globalThis.CoreCache;
export const SimpleCache = globalThis.SimpleCache;
export const SimpleCacheEntry = globalThis.SimpleCacheEntry;
export const TransactionCacheEntry = globalThis.TransactionCacheEntry;
`,
          };
        }
        case 'compute': {
          return {
            contents: `export const { purgeSurrogateKey, vCpuTime } = globalThis.fastly;`,
          };
        }
        case 'html-rewriter': {
          return {
            contents: `export const HTMLRewritingStream = globalThis.HTMLRewritingStream;`,
          };
        }
      }
    });
  },
};

export async function bundle(input, outfile, {
  moduleMode = false,
  enableStackTraces = false,
}) {

  // Build output file in cwd first to build sourcemap with correct paths
  const bundle = resolve(basename(outfile));

  const plugins = [
    fastlyPlugin,
  ];
  if (moduleMode) {
    plugins.push(swallowTopLevelExportsPlugin({entry: input}));
  }

  const inject = [];
  if (enableStackTraces) {
    inject.push(resolve(__dirname, './rsrc/trace-mapping.inject.js'));
  }

  await build({
    conditions: ['fastly'],
    entryPoints: [input],
    bundle: true,
    write: true,
    outfile: bundle,
    sourcemap: 'external',
    sourcesContent: true,
    format: moduleMode ? 'esm' : 'iife',
    tsconfig: undefined,
    plugins,
    inject,
  });

  await rename(bundle, outfile);
  if (enableStackTraces) {
    await rename(bundle + '.map', outfile + '.map');
  }
}
