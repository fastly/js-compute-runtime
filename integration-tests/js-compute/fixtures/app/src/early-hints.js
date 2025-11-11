import { pass, ok, strictEqual, assertThrows } from './assertions.js';
import { routes } from './routes.js';

routes.set('/early-hints/manual-response', (event) => {
    event.respondWith(new Response(null, { status: 103, headers: { link: '</style.css>; rel=preload; as=style' } }));
    event.respondWith(new Response('ok'));
})

routes.set('/early-hints/send-early-hint', (event) => {
    event.sendEarlyHints({ link: '</style.css>; rel=preload; as=style' });
    event.respondWith(new Response('ok'));
})

