/**
 *
 * @param {[
 *   [string, string | string[]]
 * ] | Record<string, string | string[]>} configHeaders
 * @param {Headers | {
 *   [string]: string
 * }} wasmModuleHeaders
 * @returns
 */
import { strictEqual } from "node:assert";

const compareHeaders = (configHeaders, wasmModuleHeaders) => {
  if (!configHeaders) {
    return;
  }

  // convert an array of entries into an object of arrays for easier asserting
  if (Array.isArray(configHeaders)) {
    const combinedHeaders = Object.create(null);
    for (const [key, val] of configHeaders) {
      if (!Object.hasOwnProperty.call(combinedHeaders, key)) {
        combinedHeaders[key] = val;
      } else {
        if (Array.isArray(combinedHeaders[key])) {
          if (Array.isArray(val)) {
            combinedHeaders[key] = combinedHeaders[key].concat(val);
          } else {
            combinedHeaders[key].push(val);
          }
        } else {
          combinedHeaders[key] = [
            combinedHeaders[key],
            ...(Array.isArray(val) ? val : [val]),
          ];
        }
      }
    }
    configHeaders = combinedHeaders;
  }

  for (let [configHeaderKey, configHeaderValue] of Object.entries(
    configHeaders
  )) {
    let wasmModuleHeaderValue =
      wasmModuleHeaders[configHeaderKey.toLowerCase()];
    if (Array.isArray(configHeaderValue) && configHeaderValue.length === 1) {
      configHeaderValue = configHeaderValue[0];
    }
    if (Array.isArray(configHeaderValue)) {
      if (!Array.isArray(wasmModuleHeaderValue)) {
        throw new Error(`[Header Value mismatch] Expected multiple headers for '${configHeaderKey}', but ony got one.`);
      }
      if (configHeaderValue.length !== wasmModuleHeaderValue.length) {
        throw new Error(`[Header Value mismatch] Expected ${configHeaderValue.length} headers for '${configHeaderKey}', but got ${wasmModuleHeaderValue.length}.`);
      }
      for (const [idx, configValue] of configHeaderValue.entries()) {
        if (wasmModuleHeaderValue[idx] !== configValue) {
          throw new Error(`[Header Value mismatch] Expected '${configValue}' for header item ${idx} of '${configHeaderKey}', but got '${wasmModuleHeaderValue[idx]}'.`);
        }
      }
    }
    else if (wasmModuleHeaderValue !== configHeaderValue) {
      throw new Error(
        `[Header Value mismatch] Expected: '${configHeaderKey}: ${configHeaderValue}' (${configHeaderValue.length}), got '${configHeaderKey}: ${wasmModuleHeaderValue}' (${wasmModuleHeaderValue.length})`
      );
    }
  }
};

export default compareHeaders;
