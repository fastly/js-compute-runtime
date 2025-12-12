/* eslint-env serviceworker */
import { assert, assertThrows, strictEqual } from './assertions.js';
import { inspect } from 'fastly:security';
import { routes } from './routes.js';

routes.set('/fastly/security/inspect/invalid-config', () => {
  const req = new Request('https://example.com');
  assertThrows(() => {
    inspect(req);
  });
  assertThrows(() => {
    inspect(req, {
      corp: 'test',
    });
  });
  assertThrows(() => {
    inspect(req, {
      workspace: 'test',
    });
  });
});

routes.set('/fastly/security/inspect/basic', () => {
  const req = new Request('https://example.com');
  const config = {
    corp: 'test',
    workspace: 'test',
    overrideClientIp: '10.10.10.10',
  };
  const result = inspect(req, config);
  strictEqual(result.verdict, 'allow');
  strictEqual(result.waf_response, 200);
  strictEqual(Array.isArray(result.tags), true);
  strictEqual(typeof result.decision_ms, 'number');
  assert(result.decision_ms >= 0, true);
});
