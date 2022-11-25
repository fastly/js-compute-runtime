import virtual from '@rollup/plugin-virtual';
import { rollup } from 'rollup';

export async function bundle(input) {
  try {
    const bundle = await rollup({
      input,
      plugins: [
        virtual({
          'fastly:backend': `export const Backend = globalThis.Backend`,
          'fastly:cache-override': `export const CacheOverride = globalThis.CacheOverride`,
          'fastly:config-store': `export const ConfigStore = globalThis.ConfigStore`,
          'fastly:dictionary': `export const Dictionary = globalThis.Dictionary`,
          'fastly:env': `export const env = globalThis.fastly.env.get`,
          'fastly:object-store': `export const ObjectStore = globalThis.ObjectStore`,
        })
      ]
    });
    return await bundle.generate({
      format: 'iife'
    })
  } catch (error) {
    console.error(error);
  }
}

