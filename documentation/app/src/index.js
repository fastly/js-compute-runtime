/// <reference types="@fastly/js-compute" />
/* eslint-env serviceworker */

import { get } from "../c-at-e-file-server.js";
import { env } from "fastly:env";

addEventListener("fetch", (event) => event.respondWith(app(event)));

async function app(event) {
    try {
        console.log(`FASTLY_SERVICE_VERSION: ${env('FASTLY_SERVICE_VERSION')}`)
        const response = await get('site', event.request)
        if (response) {
            response.headers.set("x-compress-hint", "on");
            return response
        } else {
            return new Response("Not Found", { status: 404 });
        }
    } catch (error) {
        console.error(error);
        return new Response(error.message + '\n' + error.stack, { status: 500 })
    }
}
