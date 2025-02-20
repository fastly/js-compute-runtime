/// <reference path="../../../../../types/index.d.ts" />
import {
  strictEqual,
  assertThrows,
  assertRejects,
  deepStrictEqual,
} from './assertions.js';
import { routes } from './routes.js';
import { env } from 'fastly:env';
import { Acl } from 'fastly:acl';

const ACL_NAME = env('ACL_NAME');

routes.set('/acl', async () => {
  assertThrows(
    () => Acl(),
    TypeError,
    "Acl builtin can't be instantiated directly",
  );
  assertThrows(
    () => new Acl(),
    TypeError,
    "Acl builtin can't be instantiated directly",
  );
  assertThrows(
    () => Acl.open(),
    TypeError,
    'Acl open: At least 1 argument required, but only 0 passed',
  );
  assertThrows(() => Acl.open(4), TypeError, 'Acl open: name must be a string');
  assertThrows(
    () => Acl.open(''),
    TypeError,
    'Acl open: name can not be an empty string',
  );
  assertThrows(
    () => Acl.open('foo'),
    TypeError,
    'Acl open: "foo" acl not found',
  );

  const acl = Acl.open(ACL_NAME);

  await assertRejects(
    () => acl.lookup(),
    TypeError,
    'lookup: At least 1 argument required, but only 0 passed',
  );
  await assertRejects(
    () => acl.lookup(5),
    Error,
    'Invalid address passed to acl.lookup',
  );
  await assertRejects(
    () => acl.lookup('not ip'),
    Error,
    'Invalid address passed to acl.lookup',
  );
  await assertRejects(
    () => acl.lookup('999.999.999.999'),
    Error,
    'Invalid address passed to acl.lookup',
  );

  strictEqual(await acl.lookup('123.123.123.123'), null);
  strictEqual(await acl.lookup('100.99.100.100'), null);
  strictEqual(
    await acl.lookup('2a04:4b80:1234:5678:9abc:def0:1234:5678'),
    null,
  );

  deepStrictEqual(await acl.lookup('100.100.100.100'), {
    action: 'BLOCK',
    prefix: '100.100.0.0/16',
  });
  deepStrictEqual(await acl.lookup('2a03:4b80:5c1d:e8f7:92a3:b45c:61d8:7e9f'), {
    action: 'ALLOW',
    prefix: '2a03:4b80::/32',
  });
  deepStrictEqual(await acl.lookup('2a03:4b80::1'), {
    action: 'ALLOW',
    prefix: '2a03:4b80::/32',
  });
});
