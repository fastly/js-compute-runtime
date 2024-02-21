import { build } from 'esbuild'

let fastlyPlugin = {
  name: 'fastly',
  setup(build) {
    build.onResolve({ filter: /^fastly:.*/ }, args => {
      return {
        path: args.path.replace('fastly:', ''),
        namespace: 'fastly',
      }
    })
    build.onLoad({ filter: /^.*/, namespace: 'fastly' }, async (args) => {
      switch (args.path) {
        case 'backend': { return { contents: `export const Backend = globalThis.Backend;` } }
        case 'body': { return { contents: `export const FastlyBody = globalThis.FastlyBody;` } }
        case 'cache-override': { return { contents: `export const CacheOverride = globalThis.CacheOverride;` } }
        case 'config-store': { return { contents: `export const ConfigStore = globalThis.ConfigStore;` } }
        case 'dictionary': { return { contents: `export const Dictionary = globalThis.Dictionary;` } }
        case 'env': { return { contents: `export const env = globalThis.fastly.env.get;` } }
        case 'experimental': {
          return {
            contents: `
export const includeBytes = globalThis.fastly.includeBytes;
export const enableDebugLogging = globalThis.fastly.enableDebugLogging;
export const setBaseURL = Object.getOwnPropertyDescriptor(globalThis.fastly, 'baseURL').set;
export const setDefaultBackend = Object.getOwnPropertyDescriptor(globalThis.fastly, 'defaultBackend').set;
export const allowDynamicBackends = Object.getOwnPropertyDescriptor(globalThis.fastly, 'allowDynamicBackends').set;
`
          }
        }
        case 'fanout': {
          return {
            contents: `export const createFanoutHandoff = globalThis.fastly.createFanoutHandoff;`
          }
        }
        case 'geolocation': { return { contents: `export const getGeolocationForIpAddress = globalThis.fastly.getGeolocationForIpAddress;` } }
        case 'logger': { return { contents: `export const Logger = globalThis.Logger;` } }
        case 'kv-store': { return { contents: `export const KVStore = globalThis.KVStore;` } }
        case 'secret-store': { return { contents: `export const SecretStore = globalThis.SecretStore;export const SecretStoreEntry = globalThis.SecretStoreEntry;` } }
        case 'cache': { return { contents: `
export const CacheEntry = globalThis.CacheEntry;
export const CacheState = globalThis.CacheState;
export const CoreCache = globalThis.CoreCache;
export const SimpleCache = globalThis.SimpleCache;
export const SimpleCacheEntry = globalThis.SimpleCacheEntry;
export const TransactionCacheEntry = globalThis.TransactionCacheEntry;
`
          }
        }
      }
    })
  },
}

export async function bundle(input) {
  return await build({
    conditions: ['fastly'],
    entryPoints: [input],
    bundle: true,
    write: false,
    tsconfig: undefined,
    plugins: [fastlyPlugin],
  })
}
