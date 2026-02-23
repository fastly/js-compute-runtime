import { pass, ok, strictEqual, assertThrows } from './assertions.js';
import { routes } from './routes.js';

routes.set('/early-hints/manual-response', (event) => {
  event.respondWith(
    new Response(null, {
      status: 103,
      headers: { link: '</style.css>; rel=preload; as=style' },
    }),
  );
  event.respondWith(new Response('ok'));
});

routes.set('/early-hints/manual-response-late', (event) => {
  event.respondWith(new Response('ok'));
  assertThrows(() => {
    event.respondWith(
      new Response(null, {
        status: 103,
        headers: { link: '</style.css>; rel=preload; as=style' },
      }),
    );
  });
});

routes.set('/early-hints/manual-response-async', async (event) => {
  await (async () => {
    event.respondWith(
      new Response(null, {
        status: 103,
        headers: { link: '</style.css>; rel=preload; as=style' },
      }),
    );
    event.respondWith(new Response('ok'));
  })();
});

routes.set('/early-hints/manual-response-late-async', async (event) => {
  await (async () => {
    event.respondWith(new Response('ok'));
    assertThrows(() => {
      event.respondWith(
        new Response(null, {
          status: 103,
          headers: { link: '</style.css>; rel=preload; as=style' },
        }),
      );
    });
  })();
});

routes.set('/early-hints/send-early-hints', (event) => {
  event.sendEarlyHints({ link: '</style.css>; rel=preload; as=style' });
  event.respondWith(new Response('ok'));
});

routes.set('/early-hints/send-early-hints-late', (event) => {
  event.respondWith(new Response('ok'));
  assertThrows(() => {
    event.sendEarlyHints({ link: '</style.css>; rel=preload; as=style' });
  });
});

routes.set('/early-hints/send-early-hints-multiple-headers', (event) => {
  event.sendEarlyHints([
    ['link', '</style.css>; rel=preload; as=style'],
    ['link', '</style2.css>; rel=preload; as=style'],
  ]);
  event.respondWith(new Response('ok'));
});

routes.set('/early-hints/send-early-hints-async', async (event) => {
  await (async () => {
    event.sendEarlyHints({ link: '</style.css>; rel=preload; as=style' });
    event.respondWith(new Response('ok'));
  })();
});

routes.set('/early-hints/send-early-hints-late-async', async (event) => {
  await (async () => {
    event.respondWith(new Response('ok'));
    assertThrows(() => {
      event.sendEarlyHints({ link: '</style.css>; rel=preload; as=style' });
    });
  })();
});
