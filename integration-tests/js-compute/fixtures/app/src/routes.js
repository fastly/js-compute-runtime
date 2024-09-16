import { env } from 'fastly:env';

/**
 * @type {Map<string, (FetchEvent) => Promise<Response>>}
 */
export const routes = new Map();
routes.set('/', () => {
  routes.delete('/');
  let test_routes = Array.from(routes.keys());
  return new Response(JSON.stringify(test_routes), {
    headers: { 'content-type': 'application/json' },
  });
});

export function isRunningLocally() {
  return (
    env('FASTLY_SERVICE_VERSION') === '' ||
    env('FASTLY_SERVICE_VERSION') === '0'
  );
}
