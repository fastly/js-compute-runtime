/// <reference path="../../../../../types/index.d.ts" />
import {
  strictEqual,
  assertThrows,
  assertRejects,
  assertResolves,
  deepStrictEqual,
} from './assertions.js';
import { routes } from './routes.js';
import { env } from 'fastly:env';
import { open } from 'fastly:acl';

const ACL_NAME = env('ACL_NAME');

routes.set('/acl', async () => {
  assertThrows(
    () => open(),
    TypeError,
    'Acl open: At least 1 argument required, but only 0 passed',
  );
  assertThrows(() => open(4), TypeError, 'Acl open: name must be a string');
  assertThrows(
    () => open(''),
    TypeError,
    'Acl open: name can not be an empty string',
  );
  assertThrows(() => open('foo'), TypeError, 'Acl open: "foo" acl not found');

  const acl = open(ACL_NAME);

  await assertRejects(() => acl.lookup(), TypeError, 'a');
  await assertRejects(() => acl.lookup(5), TypeError, 'b');
  await assertRejects(() => acl.lookup('not ip'), TypeError, 'c');
  await assertRejects(() => acl.lookup('999.999.999.999'), TypeError, 'd');
  await assertRejects(() => acl.lookup('999.999.999.999'), TypeError, 'd');

  return new Response(JSON.stringify(await acl.lookup('123.123.123.123')));
});
