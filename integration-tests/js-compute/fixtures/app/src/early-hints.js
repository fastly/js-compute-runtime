import { routes } from './routes.js';
import { assertThrows, assert, assertResolves } from './assertions.js';
import { env } from 'fastly:env';

routes.set('/early-hints', (event) => {
    event.sendEarlyHint({
        "Link": "</style>; rel=preload; as=style"
    });
    //event.respondWith(new Response('OK'));
});