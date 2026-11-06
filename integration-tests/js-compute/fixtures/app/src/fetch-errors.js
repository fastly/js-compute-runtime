/// <reference path="../../../../../types/index.d.ts" />
import { assertRejects } from './assertions.js';
import { routes } from './routes.js';
import { Backend, allowDynamicBackends } from 'fastly:backend';

routes.set('/fetch-errors', async () => {
  allowDynamicBackends(true);
  await assertRejects(
    async () => {
      await fetch('http://127.0.0.1');
    },
    DOMException,
    'Connection refused',
  );

  await assertRejects(
    async () => {
      await fetch('https://fastly.com/', {
        backend: new Backend({
          name: 'b1',
          target: 'fastly.com:8080',
        }),
      });
    },
    DOMException,
    'Connection refused',
  );

  await assertRejects(
    async () => {
      await fetch('https://fastly.com', {
        backend: new Backend({
          name: 'b3',
          target: 'fastly.com',
          useSSL: true,
          certificateHostname: 'google.com',
        }),
      });
    },
    DOMException,
    'TLS certificate error',
  );
});
