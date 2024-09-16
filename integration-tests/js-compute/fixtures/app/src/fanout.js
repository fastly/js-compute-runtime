import {
  assert,
  assertDoesNotThrow,
  assertThrows,
} from './assertions.js';
import { routes } from './routes.js';
import { createFanoutHandoff } from 'fastly:fanout';

routes.set('/createFanoutHandoff', () => {
  assert(typeof createFanoutHandoff, 'function', 'typeof createFanoutHandoff');

  assert(
    createFanoutHandoff.name,
    'createFanoutHandoff',
    'createFanoutHandoff.name',
  );

  assert(createFanoutHandoff.length, 2, 'createFanoutHandoff.length');

  assertDoesNotThrow(() => createFanoutHandoff(new Request('.'), 'hello'));

  assertThrows(() => createFanoutHandoff());

  assertThrows(() => createFanoutHandoff(1, ''));

  let result = createFanoutHandoff(new Request('.'), 'hello');
  assert(result instanceof Response, true, 'result instanceof Response');

  assertThrows(
    () => new createFanoutHandoff(new Request('.'), 'hello'),
    TypeError,
  );

  assertDoesNotThrow(() => {
    createFanoutHandoff.call(undefined, new Request('.'), '1');
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
    createFanoutHandoff(new Request('.'), key);
  };
  assertThrows(test);
  try {
    test();
  } catch (thrownError) {
    assert(thrownError, sentinel, 'thrownError === sentinel');
  }
  assertThrows(
    () => {
      createFanoutHandoff(new Request('.'), Symbol());
    },
    TypeError,
    `can't convert symbol to string`,
  );

  assertThrows(
    () => createFanoutHandoff(new Request('.')),
    TypeError,
    `createFanoutHandoff: At least 2 arguments required, but only 1 passed`,
  );

  assertThrows(
    () => createFanoutHandoff(new Request('.'), ''),
    Error,
    `createFanoutHandoff: Backend parameter can not be an empty string`,
  );
});
