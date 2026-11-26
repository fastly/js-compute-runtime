import { enableDebugLogging } from 'fastly:debug';
import { setReusableSandboxOptions } from 'fastly:runtime';

enableDebugLogging(true);
setReusableSandboxOptions({ maxRequests: 0, betweenRequestTimeoutMs: 2000 });

addEventListener('fetch', (event) => event.respondWith(new Response('ok\n')));
