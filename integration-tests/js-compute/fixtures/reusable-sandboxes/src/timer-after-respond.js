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

routes.set('/timer-after-respond/check', async () => {
  // Avoid timing flakiness: the timer scheduled by /schedule may not have fired
  // yet when /check is requested immediately afterwards.
  const sleep = (ms) => new Promise((resolve) => setTimeout(resolve, ms));

  const deadline = Date.now() + 100; // bounded wait to keep the test fast
  while (timerFired === 0 && Date.now() < deadline) {
    await sleep(1);
  }

  return new Response(String(timerFired));
});
