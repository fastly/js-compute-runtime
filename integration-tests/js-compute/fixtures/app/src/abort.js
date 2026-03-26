import { routes } from './routes';
import { strictEqual } from './assertions.js';

routes.set('/abort/fetch', async() => {
    const controller = new AbortController();
    const signal = controller.signal;

    const timeoutId = setTimeout(() => controller.abort(), 500);

    let error;
    try {
      await fetch('https://http-me.fastly.dev/wait=5000', { signal });
    } catch (err) {
      error = err;
    } finally {
      clearTimeout(timeoutId);
    }

    strictEqual(error.name, 'AbortError');
  });

routes.set('/abort/complete-before', async () => {
    const controller = new AbortController();
    const signal = controller.signal;

    // Abort after 5000ms (won't fire for a fast endpoint)
    const timeoutId = setTimeout(() => controller.abort(), 5000);

    const response = await fetch('https://http-me.glitch.me/wait=100', { signal });
    clearTimeout(timeoutId);

    strictEqual(response.ok, true);
    strictEqual(response.status, 200);
  });