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
        case 'geolocation': { return { contents: `export const getGeolocationForIpAddress = globalThis.fastly.getGeolocationForIpAddress;` } }
        case 'logger': { return { contents: `export const Logger = globalThis.Logger;` } }
        case 'object-store': { return { contents: `export const ObjectStore = globalThis.ObjectStore;` } }
      }
    })
  },
}

export async function bundle(input, outfile) {
  return await build({
    entryPoints: [input],
    bundle: true,
    write: false,
    tsconfig: undefined,
    sourcemap: 'external',
    outfile,
    plugins: [fastlyPlugin],
  })
}
