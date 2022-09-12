
/**
 * 
 * @param {{
        [string]: string
      }} configHeaders
 * @param {Headers | {
        [string]: string
      }} wasmModuleHeaders
 * @returns
 */
const compareHeaders = (configHeaders, wasmModuleHeaders) => {

  if (!configHeaders) {
    return;
  }

  const configHeaderKeys = Object.keys(configHeaders);
  configHeaderKeys.forEach(configHeaderKey => {
    const configHeaderValue = configHeaders[configHeaderKey];

    let wasmModuleHeaderValue = null; 
    if (wasmModuleHeaders.get) {
      wasmModuleHeaderValue = wasmModuleHeaders.get(configHeaderKey);
    } else {
      wasmModuleHeaderValue = wasmModuleHeaders[configHeaderKey.toLowerCase()];
    }
    if (wasmModuleHeaderValue === null) {
      throw new Error(`[Header Key mismatch] Expected: ${configHeaderKey} - Got: ${null}`);
    } else if (wasmModuleHeaderValue !== configHeaderValue) {
      throw new Error(`[Header Value mismatch] Expected: ${configHeaderValue} - Got: ${wasmModuleHeaderValue}`);
    }
  });
};

export default compareHeaders;
