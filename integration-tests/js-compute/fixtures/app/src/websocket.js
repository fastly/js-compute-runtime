import { assert, assertDoesNotThrow, assertThrows } from './assertions.js';
import { routes } from './routes.js';
import { createWebsocketHandoff } from 'fastly:websocket';

routes.set('/createWebsocketHandoff', () => {
  assert(typeof createWebsocketHandoff, 'function', 'typeof createWebsocketHandoff');

  assert(
    createWebsocketHandoff.name,
    'createWebsocketHandoff',
    'createWebsocketHandoff.name',
  );

  assert(createWebsocketHandoff.length, 2, 'createWebsocketHandoff.length');

  assertDoesNotThrow(() => createWebsocketHandoff(new Request('.'), 'hello'));

  assertThrows(() => createWebsocketHandoff());

  assertThrows(() => createWebsocketHandoff(1, ''));

  let result = createWebsocketHandoff(new Request('.'), 'hello');
  assert(result instanceof Response, true, 'result instanceof Response');

  assertThrows(
    () => new createWebsocketHandoff(new Request('.'), 'hello'),
    TypeError,
  );

  assertDoesNotThrow(() => {
    createWebsocketHandoff.call(undefined, new Request('.'), '1');
  });

  // https://tc39.es/ecma262/#sec-tostring
  let sentinel;
  const test = () => {
    sentinel = Symbol();
    const key = {
      toString() {
        throw sentinel;
      },
    };
    createWebsocketHandoff(new Request('.'), key);
  };
  assertThrows(test);
  try {
    test();
  } catch (thrownError) {
    assert(thrownError, sentinel, 'thrownError === sentinel');
  }
  assertThrows(
    () => {
      createWebsocketHandoff(new Request('.'), Symbol());
    },
    TypeError,
    `can't convert symbol to string`,
  );

  assertThrows(
    () => createWebsocketHandoff(new Request('.')),
    TypeError,
    `createWebsocketHandoff: At least 2 arguments required, but only 1 passed`,
  );

  assertThrows(
    () => createWebsocketHandoff(new Request('.'), ''),
    Error,
    `createWebsocketHandoff: Backend parameter can not be an empty string`,
  );
});
