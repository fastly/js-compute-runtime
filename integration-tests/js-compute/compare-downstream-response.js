import compareHeaders from './compare-headers.js';

/**
 * Function to compare a response from a server (Viceroy, Fastly Compute, etc...)
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

function maybeUint8Array(body) {
  if (Array.isArray(body) && typeof body[0] === 'number')
    return new Uint8Array(body);
  if (typeof body === 'string') return new TextEncoder().encode(body);
  return body;
}

/**
 * @param {Uint8Array[]} buffers
 * @returns
 */
function concat(buffers) {
  const out = new Uint8Array(buffers.reduce((len, buf) => len + buf.length, 0));
  let curPos = 0;
  for (const buf of buffers) {
    if (!(buf instanceof Uint8Array)) throw new Error('not a uint8 array');
    out.set(buf, curPos);
    curPos += buf.length;
  }
  return out;
}

function bufferToString(actualBodyChunks) {
  try {
    return new TextDecoder().decode(concat(actualBodyChunks));
  } catch {
    return concat(actualBodyChunks);
  }
}

function bufferEq(a, b) {
  for (let i = 0; i < a.byteLength; i++) {
    if (a[i] !== b[i]) return false;
  }
  return true;
}

export async function compareDownstreamResponse(
  configResponse,
  actualResponse,
  actualBodyChunks,
) {
  // Status
  if (
    configResponse.status !== undefined &&
    configResponse.status != actualResponse.statusCode
  ) {
    throw new Error(
      `[DownstreamResponse: Status mismatch] Expected: ${configResponse.status} - Got: ${actualResponse.statusCode}\n${actualBodyChunks.length ? `\n"${bufferToString(actualBodyChunks)}"` : ''}`,
    );
  }

  // Headers
  if (configResponse.headers) {
    compareHeaders(configResponse.headers, actualResponse.headers);
  }

  // Body
  if (
    configResponse.body ||
    configResponse.body_prefix ||
    configResponse.body_suffix
  ) {
    if (configResponse.body_prefix) {
      const body_prefix = maybeUint8Array(configResponse.body_prefix);
      const actual_prefix = concat(actualBodyChunks).slice(
        0,
        body_prefix.byteLength,
      );
      if (body_prefix.byteLength !== actual_prefix.byteLength) {
        throw new Error(
          `[DownstreamResponse: Body Prefix length mismatch] Expected: ${body_prefix.byteLength} - Got ${actual_prefix.byteLength}: \n"${bufferToString(actualBodyChunks)}"`,
        );
      }
      if (!bufferEq(actual_prefix, body_prefix)) {
        throw new Error(
          `[DownstreamResponse: Body Prefix mismatch] Expected: ${body_prefix} - Got ${actual_prefix}:\n"${bufferToString(actualBodyChunks)}"`,
        );
      }
    }
    if (configResponse.body_suffix) {
      const body_suffix = maybeUint8Array(configResponse.body_suffix);
      const actual_suffix = concat(actualBodyChunks).slice(
        0,
        body_suffix.byteLength,
      );
      if (body_suffix.byteLength !== actual_suffix.byteLength) {
        throw new Error(
          `[DownstreamResponse: Body Suffix length mismatch] Expected: ${body_suffix.byteLength} - Got: ${actual_suffix.byteLength}: \n"${bufferToString(actualBodyChunks)}"`,
        );
      }
      if (!bufferEq(actual_suffix, body_suffix)) {
        throw new Error(
          `[DownstreamResponse: Body Suffix mismatch] Expected: ${body_suffix} - Got ${actual_suffix}:\n"${bufferToString(actualBodyChunks)}"`,
        );
      }
    }
    // Check if we need to stream the response and check the chunks, or the whole body
    configResponse.body = maybeUint8Array(configResponse.body);
    if (configResponse.body instanceof Array) {
      // Stream down the response
      let chunkNumber = 0;
      for (const chunk of actualBodyChunks) {
        const chunkString = chunk.toString('utf8');

        // Check if the chunk is equal to what we expected
        if (configResponse.body[chunkNumber].includes(chunk.toString('utf8'))) {
          // Yay! We got a matching Chunk, let's see if this is the end of one of our expected chunks. If so, we need to increment our chunk number :)
          if (
            configResponse.body[chunkNumber].endsWith(chunk.toString('utf8'))
          ) {
            chunkNumber++;
          }
        } else {
          throw new Error(
            `[DownstreamResponse: Body Chunk mismatch] Expected: ${configResponse.body[chunkNumber]} - Got: ${chunkString}`,
          );
        }
      }

      if (chunkNumber !== configResponse.body.length) {
        throw new Error(
          `[DownstreamResponse: Body Chunk mismatch] Expected: ${configResponse.body} - Got: (Incomplete stream, Number of chunks returned: ${chunkNumber})`,
        );
      }
    } else if (configResponse.body !== undefined) {
      // Get the text, and check if it matches the test
      const downstreamBodyText = new TextDecoder().decode(
        concat(actualBodyChunks),
      );
      const eq =
        typeof configResponse.body === 'string'
          ? downstreamBodyText === configResponse.body
          : bufferEq(configResponse.body, concat(actualBodyChunks));
      if (!eq) {
        throw new Error(
          `[DownstreamResponse: Body mismatch] Expected: ${configResponse.body} - Got: ${downstreamBodyText}`,
        );
      }
    }
  }
}
