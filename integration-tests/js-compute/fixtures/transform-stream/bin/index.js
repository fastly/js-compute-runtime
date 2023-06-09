/// <reference path="../../../../../types/index.d.ts" />
/* eslint-env serviceworker */
/* global TransformStream */

import { routes } from "../../../test-harness.js";

function upperCase() {
    const decoder = new TextDecoder()
    const encoder = new TextEncoder()
    return new TransformStream({
        transform(chunk, controller) {
            controller.enqueue(encoder.encode(decoder.decode(chunk).toUpperCase()));
        },
    });
}

routes.set("/identity", () => {
    return fetch('https://http-me.glitch.me/test?body=hello', { backend: 'http-me' }).then(response => {
        return new Response(response.body.pipeThrough(new TransformStream));
    })
});
routes.set("/uppercase", () => {
    return fetch('https://http-me.glitch.me/test?body=hello', { backend: 'http-me' }).then(response => {
        return new Response(response.body.pipeThrough(upperCase()));
    })
});
