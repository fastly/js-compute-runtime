/// <reference types="@fastly/js-compute" />
/* eslint-env serviceworker */

import { pass, assert, assertDoesNotThrow, assertThrows } from "../../../assertions.js";
import { routes } from "../../../test-harness.js";
import { createFanoutHandoff } from "fastly:fanout";

let error;
routes.set("/createFanoutHandoff", async () => {
  error = assert(typeof createFanoutHandoff, "function", "typeof createFanoutHandoff");
  if (error) { return error; }

  error = assert(createFanoutHandoff.name, "createFanoutHandoff", "createFanoutHandoff.name");
  if (error) { return error; }

  error = assert(createFanoutHandoff.length, 2, "createFanoutHandoff.length");
  if (error) { return error; }

  error = assertDoesNotThrow(() => createFanoutHandoff(new Request('.'), 'hello'));
  if (error) { return error; }

  error = assertThrows(() => createFanoutHandoff());
  if (error) { return error; }

  error = assertThrows(() => createFanoutHandoff(1, ''));
  if (error) { return error; }

  let result = createFanoutHandoff(new Request('.'), 'hello');
  error = assert(result instanceof Response, true, 'result instanceof Response');
  if (error) { return error; }

  error = assertThrows(() => new createFanoutHandoff(new Request('.'), 'hello'), TypeError, `createFanoutHandoff is not a constructor`)
  if (error) { return error }

  error = await assertDoesNotThrow(async () => {
    createFanoutHandoff.call(undefined, new Request('.'), '1')
  })
  if (error) { return error }

  // https://tc39.es/ecma262/#sec-tostring
  let sentinel;
  const test = () => {
    sentinel = Symbol();
    const key = {
      toString() {
        throw sentinel;
      }
    }
    createFanoutHandoff(new Request('.'), key)
  }
  error = assertThrows(test)
  if (error) { return error }
  try {
    test()
  } catch (thrownError) {
    let error = assert(thrownError, sentinel, 'thrownError === sentinel')
    if (error) { return error }
  }
  error = assertThrows(() => {
    createFanoutHandoff(new Request('.'), Symbol())
  }, TypeError, `can't convert symbol to string`)
  if (error) { return error }

  error = assertThrows(() => createFanoutHandoff(new Request('.')), TypeError, `createFanoutHandoff: At least 2 arguments required, but only 1 passed`)
  if (error) { return error }

  error = assertThrows(() => createFanoutHandoff(new Request('.'), ''), Error, `createFanoutHandoff: Backend parameter can not be an empty string`)
  if (error) { return error }


  return pass();
});

