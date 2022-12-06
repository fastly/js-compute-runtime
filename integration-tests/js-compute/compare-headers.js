/**
 * 
 * @param {[
        [string, string]
      ]} configHeaders
 * @param {Headers | {
        [string]: string
      }} wasmModuleHeaders
 * @returns
 */
const compareHeaders = (configHeaders, wasmModuleHeaders) => {

  if (!configHeaders) {
    return;
  }

  for (const [configHeaderKey, configHeaderValue] of Object.entries(configHeaders)) {
    let wasmModuleHeaderValue = wasmModuleHeaders[configHeaderKey.toLowerCase()]
    if (wasmModuleHeaderValue === null) {
      throw new Error(`[Header Key mismatch] Expected: ${configHeaderKey} - Got: ${null}`);
    } else if (Array.isArray(configHeaderValue)) {
      if (Array.isArray(configHeaderValue)) {
        for (let value of configHeaderValue) {
          if (!configHeaderValue.includes(value)) {
            throw new Error(`[Header mismatch] Missing header named "${configHeaderKey}" with value "${value}"`);
          }
        }
      } else {
        throw new Error(`[Header mismatch] Expected multiple headers with named "${configHeaderKey}" but got only one`);
      }
    } else if (wasmModuleHeaderValue !== configHeaderValue) {
      throw new Error(`[Header Value mismatch] Expected: ${configHeaderValue} - Got: ${wasmModuleHeaderValue}`);
    }
  }
};

export default compareHeaders;
