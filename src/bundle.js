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
        case 'backend': {return {contents:`export const Backend = globalThis.Backend`}}
        case 'cache-override': {return {contents:`export const CacheOverride = globalThis.CacheOverride`}}
        case 'config-store': {return {contents:`export const ConfigStore = globalThis.ConfigStore`}}
        case 'dictionary': {return {contents:`export const Dictionary = globalThis.Dictionary`}}
        case 'env': {return {contents:`export const env = globalThis.fastly.env.get`}}
        case 'geolocation': {return {contents:`export const getGeolocationForIpAddress = globalThis.fastly.getGeolocationForIpAddress`}}
        case 'logger': {return {contents:`export const Logger = globalThis.Logger`}}
        case 'object-store': {return {contents:`export const ObjectStore = globalThis.ObjectStore`}}
      }
    })
  },
}

export async function bundle(input) {
  return await build({
    entryPoints: [input],
    bundle: true,
    write: false,
    tsconfig: undefined,
    plugins: [fastlyPlugin],
  })
}
