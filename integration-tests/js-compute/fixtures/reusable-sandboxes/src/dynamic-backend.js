/// <reference path="../../../../../types/index.d.ts" />
import {
  Backend,
  enforceExplicitBackends,
  setDefaultDynamicBackendConfig,
} from 'fastly:backend';
import { assert, strictEqual, assertRejects } from './assertions.js';
import { isRunningLocally, routes } from './routes.js';

routes.set('/backend/ephemeral', async () => {
  if (isRunningLocally()) {
    return;
  }
  assert(!Backend.exists('ephemeral'));
  new Backend({
    name: 'ephemeral',
    target: 'http-me.fastly.dev',
    hostOverride: 'http-me.fastly.dev',
    useSSL: true,
  });
  assert(Backend.exists('ephemeral'));
});

// The following tests ensure that the default dynamic backend is reset back
// to the global state on each request.

setDefaultDynamicBackendConfig({
  useSSL: true,
});

routes.set('/backend/defaultConfig1', async () => {
  const b1 = new Backend({
    name: 'default-config-b1',
    target: 'http-me.fastly.dev',
    hostOverride: 'http-me.fastly.dev',
  });

  strictEqual(b1.isSSL, true);

  setDefaultDynamicBackendConfig({
    useSSL: false,
  });

  const b2 = new Backend({
    name: 'default-config-b2',
    target: 'http-me.fastly.dev',
    hostOverride: 'http-me.fastly.dev',
  });

  strictEqual(b2.isSSL, false);
});

routes.set('/backend/defaultConfig2', async () => {
  const b3 = new Backend({
    name: 'default-config-b3',
    target: 'http-me.fastly.dev',
    hostOverride: 'http-me.fastly.dev',
  });

  strictEqual(b3.isSSL, true);

  setDefaultDynamicBackendConfig({
    useSSL: false,
  });

  const b4 = new Backend({
    name: 'default-config-b4',
    target: 'http-me.fastly.dev',
    hostOverride: 'http-me.fastly.dev',
  });

  strictEqual(b4.isSSL, false);
});

// Ensure that the default backend does not persist across requests
enforceExplicitBackends('non-existent');

routes.set('/backend/defaultBackend', async () => {
  await assertRejects(
    () => fetch('https://http-me.fastly.dev/anything'),
    TypeError,
    "Requested backend named 'non-existent' does not exist",
  );
  enforceExplicitBackends('httpme');
  const res = await fetch('https://http-me.fastly.dev/anything');
  strictEqual(res.status, 200);
});
