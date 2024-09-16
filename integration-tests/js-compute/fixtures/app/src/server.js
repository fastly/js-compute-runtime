import { assert } from './assertions.js';
import { routes, isRunningLocally } from './routes.js';

routes.set('/server/address', (event) => {
  assert(typeof event.server.address, 'string', 'typeof event.server.address');

  if (isRunningLocally()) {
    assert(event.server.address, '127.0.0.1', 'event.server.address');
  }
});
