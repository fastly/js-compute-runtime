import { assertRejects } from './assertions.js';
import { routes } from './routes.js';

routes.set('/missing-backend', async () => {
  await assertRejects(
    async () => fetch('https://example.com', { backend: 'missing' }),
    TypeError,
    `Requested backend named 'missing' does not exist`,
  );
});
