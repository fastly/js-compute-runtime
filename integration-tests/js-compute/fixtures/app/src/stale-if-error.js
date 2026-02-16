import { assert, assertThrows } from './assertions.js';
import { routes } from './routes.js';

routes.set('/stale-if-error/use-stale-if-error', async (event) => {
  fetch(event.request, { backend: 'httpme' });
});
