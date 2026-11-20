/* eslint-env serviceworker */

// Regression test: a setTimeout scheduled during a request must still fire
// before the sandbox is torn down, even if respondWith has already resolved.
// Otherwise frameworks that schedule post-response work (e.g. React's
// streaming renderer) break sandbox reuse.
//
// Only runs against real Compute — viceroy's local server doesn't persist
// sandboxes across requests, so the cross-request observation needed here
// doesn't work locally.

import { routes } from './routes.js';

let timerFired = 0;

routes.set('/timer-after-respond/schedule', () => {
  setTimeout(() => {
    timerFired++;
  }, 0);
  return new Response('scheduled');
});

routes.set('/timer-after-respond/check', () => {
  return new Response(String(timerFired));
});
