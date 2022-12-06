
const compareHeaders = (configHeaders, wasmModuleHeaders) => {

  if (!configHeaders) {
    return;
  }

  configHeaders.forEach(([configHeaderKey, configHeaderValue]) => {
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
