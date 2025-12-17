/* eslint-env serviceworker */

import { routes } from './routes.js';

routes.set('/request/body/proxy-transform-stream', async (event) => {
    const incomingReq = new Request('https://example.com', {
        body: event.request.body,
        method: "POST"
    });
    const newReq = new Request(incomingReq);
    let bodyText = '';
    const reader = newReq.body.getReader();
    while (true) {
        const { done, value } = await reader.read();
        if (done) break;
        bodyText += new TextDecoder().decode(value);
    }
    return new Response(bodyText, { status: 200 });
});