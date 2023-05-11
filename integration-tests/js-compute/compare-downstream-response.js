import compareHeaders from './compare-headers.js';

/**
 * Function to compare a response from a server (Viceroy, C@E, etc...)
 * With a JSON Response Object in our config
 * @param {{
      "status": number,
      "headers": [
        [string, string]
      ],
      "body": string
    }} configResponse
 * @param {import('undici').Dispatcher.ResponseData} actualResponse
 */
export async function compareDownstreamResponse (configResponse, actualResponse) {
  // Status
  if (configResponse.status != actualResponse.statusCode) {
    throw new Error(`[DownstreamResponse: Status mismatch] Expected: ${configResponse.status} - Got: ${actualResponse.status}`);
  }

  // Headers
  if (configResponse.headers) {
    compareHeaders(configResponse.headers, actualResponse.headers);
  }

  // Body
  if (configResponse.body) {

    // Check if we need to stream the response and check the chunks, or the whole body
    if (configResponse.body instanceof Array) {
      // Stream down the response
      let downstreamBody = actualResponse.body;
      let chunkNumber = 0;
      const downstreamTimeout = setTimeout(() => {
        console.error(`[DownstreamResponse: Body Chunk Timeout]`);
        process.exit(1);
      }, 30 * 1000);
      for await (const chunk of downstreamBody) {
        const chunkString = chunk.toString('utf8');

        // Check if the chunk is equal to what we expected
        if (configResponse.body[chunkNumber].includes(chunk.toString('utf8'))) {
          // Yay! We got a matching Chunk, let's see if this is the end of one of our expected chunks. If so, we need to increment our chunk number :)
          if (configResponse.body[chunkNumber].endsWith(chunk.toString('utf8'))) {
            chunkNumber++;
          }
        } else {
          throw new Error(`[DownstreamResponse: Body Chunk mismatch] Expected: ${configResponse.body[chunkNumber]} - Got: ${chunkString}`);
        }
      }

      clearTimeout(downstreamTimeout);

      if (chunkNumber !== configResponse.body.length) {
        throw new Error(`[DownstreamResponse: Body Chunk mismatch] Expected: ${configResponse.body} - Got: (Incomplete stream, Number of chunks returned: ${chunkNumber})`);
      }
    } else {
      // Get the text, and check if it matches the test
      let downstreamBodyText = await actualResponse.body.text();

      if (downstreamBodyText !== configResponse.body) {
        throw new Error(`[DownstreamResponse: Body mismatch] Expected: ${configResponse.body} - Got: ${downstreamBodyText}`);
      }
    }
  }
}
