/// <reference path="../../../../../types/index.d.ts" />
import { Backend } from 'fastly:backend';
import { assert } from './assertions.js';
import { isRunningLocally, routes } from './routes.js';

routes.set('/backend/ephemeral1', async () => {
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

routes.set('/backend/ephemeral2', async () => {
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
