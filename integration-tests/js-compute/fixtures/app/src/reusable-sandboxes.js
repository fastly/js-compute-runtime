const { CacheOverride } = require("fastly:cache-override");
const { createWebsocketHandoff } = require("fastly:websocket");
import { routes } from './routes.js';

routes.set('/reusable-sandboxes/ws', (event) => {
  return createWebsocketHandoff(event.request, 'httpme');
});

routes.set('/reusable-sandboxes/echo', async (event) => {
  const url = new URL(event.request.url);
  const searchParams = url.searchParams.toString();
  const backendUrl = searchParams ? `/anything?${searchParams}` : '/anything';
  
  return await fetch(backendUrl, {
    backend: 'httpme',
    cacheOverride: new CacheOverride('pass'),
    headers: event.request.headers,
    method: event.request.method,
  });
});