/// <reference types="@fastly/js-compute" />
import { getServer } from '../static-publisher/statics.js';
const staticContentServer = getServer();

// eslint-disable-next-line no-undef
addEventListener("fetch", (event) => event.respondWith(app(event)));

async function app(event) {
  try {
    const response = await staticContentServer.serveRequest(event.request);
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
