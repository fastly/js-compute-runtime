const compareHeaders = require('./compare-headers.js');

// Function to compare an upstream request from a wasm module running on aserver (Viceroy, C@E, etc...)
// With a JSON Request Object in our config. With additional parameters to help our index.js with logging.
const compareUpstreamRequest = async (configRequest, actualRequest, isDownstreamResponseHandled) => {

  // Check if this request should have happened after the downstream request
  if (configRequest.timing === "afterDownstreamRequest" && !isDownstreamResponseHandled) {
    throw new Error(`Local Upstream Request recieved before the Downstream Request was finished`);
  }

  // Check the method
  if (actualRequest.method !== configRequest.method) {
    throw new Error(`[Method mismatch] Expected: ${configRequest.method} - Got: ${actualRequest.method}`);
  }

  // Check the pathname
  if (actualRequest.url !== configRequest.pathname) {
    throw new Error(`[Path mismatch] Expected: ${configRequest.pathname} - Got: ${actualRequest.url}`);
  }

  // Check the headers
  try {
    compareHeaders(configRequest.headers, actualRequest.headers)
  } catch (err) {
    throw new Error(`[Header Error] ${err.message}`);
  }
}

module.exports = compareUpstreamRequest;
