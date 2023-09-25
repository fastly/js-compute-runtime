/* eslint-env serviceworker */

import { CacheOverride } from 'fastly:cache-override';
import { routes } from "./routes";

routes.set('/byte-repeater', () => {
    let headers = new Headers();
    headers.set("AssemblyScriptHeader", "AssemblyScriptValue");

    let streamController;
    let stream = new ReadableStream({
        start: (controller) => {
            streamController = controller;
        },
    });
    let response = new Response(stream, {
        headers,
    });

    // Make a request upstream
    let upstreamRequest = new Request(
        "https://compute-sdk-test-backend.edgecompute.app/byte_repeater"
    );
    upstreamRequest.setCacheOverride(new CacheOverride("pass"));
    fetch(upstreamRequest, {
        backend: "TheOrigin",
    }).then(async (upstreamResponse) => {
        let body = upstreamResponse.body;
        let streamReader = body.getReader();

        // eslint-disable-next-line no-constant-condition
        while (true) {
            let chunk = await streamReader.read();

            // Check if we are done
            if (chunk.done) {
                break;
            }

            if (chunk.value.byteLength == 0) {
                continue;
            }

            // Otherwise get the byte and repeat them
            let upstreamBytes = chunk.value;
            let downstreamBytes = new Uint8Array(upstreamBytes.length * 2);
            for (let i = 0; i < upstreamBytes.length; i++) {
                let downstreamIndex = i * 2;
                downstreamBytes[downstreamIndex] = upstreamBytes[i];
                downstreamBytes[downstreamIndex + 1] = upstreamBytes[i];
            }

            streamController.enqueue(downstreamBytes);
        }
        streamController.close();
    });

    // Send downstream (back to the client)
    return response;
});
