import test from 'brittle';
import { EnvParser } from '../../src/env.js';

test('EnvParser should parse single key-value pair', function (t) {
  const parser = new EnvParser();
  parser.parse('NODE_ENV=production');

  t.alike(parser.getEnv(), {
    NODE_ENV: 'production',
  });
});

test('EnvParser should parse multiple comma-separated values', function (t) {
  const parser = new EnvParser();
  parser.parse('NODE_ENV=production,DEBUG=true,PORT=3000');

  t.alike(parser.getEnv(), {
    NODE_ENV: 'production',
    DEBUG: 'true',
    PORT: '3000',
  });
});

test('EnvParser should merge multiple parse calls', function (t) {
  const parser = new EnvParser();
  parser.parse('NODE_ENV=production');
  parser.parse('DEBUG=true');

  t.alike(parser.getEnv(), {
    NODE_ENV: 'production',
    DEBUG: 'true',
  });
});

test('EnvParser should inherit existing environment variables', function (t) {
  const parser = new EnvParser();

  // Set up some test environment variables
  process.env.TEST_VAR1 = 'value1';
  process.env.TEST_VAR2 = 'value2';

  parser.parse('TEST_VAR1,TEST_VAR2');

  t.alike(parser.getEnv(), {
    TEST_VAR1: 'value1',
    TEST_VAR2: 'value2',
  });

  // Cleanup
  delete process.env.TEST_VAR1;
  delete process.env.TEST_VAR2;
});

test('EnvParser should handle mixed inheritance and setting', function (t) {
  const parser = new EnvParser();

  process.env.TEST_VAR = 'inherited';

  parser.parse('TEST_VAR,NEW_VAR=set');

  t.alike(parser.getEnv(), {
    TEST_VAR: 'inherited',
    NEW_VAR: 'set',
  });

  // Cleanup
  delete process.env.TEST_VAR;
});

test('EnvParser should handle values with spaces', function (t) {
  const parser = new EnvParser();
  parser.parse('MESSAGE=Hello World');

  t.alike(parser.getEnv(), {
    MESSAGE: 'Hello World',
  });
});

test('EnvParser should handle values with equals signs', function (t) {
  const parser = new EnvParser();
  parser.parse('DATABASE_URL=postgres://user:pass@localhost:5432/db');

  t.alike(parser.getEnv(), {
    DATABASE_URL: 'postgres://user:pass@localhost:5432/db',
  });
});

test('EnvParser should handle empty values', function (t) {
  const parser = new EnvParser();
  parser.parse('EMPTY=');

  t.alike(parser.getEnv(), {
    EMPTY: '',
  });
});

test('EnvParser should handle whitespace', function (t) {
  const parser = new EnvParser();
  parser.parse(' KEY = value with spaces ');

  t.alike(parser.getEnv(), {
    KEY: ' value with spaces', // Leading whitespace preserved, trailing removed
  });

  // Test multiple values with whitespace
  parser.parse(' KEY2 = value2 , KEY3 = value3 ');

  t.alike(parser.getEnv(), {
    KEY: ' value with spaces',
    KEY2: ' value2',
    KEY3: ' value3',
  });
});

test('EnvParser should merge and override values', function (t) {
  const parser = new EnvParser();
  parser.parse('KEY=first');
  parser.parse('KEY=second');

  t.alike(parser.getEnv(), {
    KEY: 'second',
  });
});

test('EnvParser should throw on empty key', function (t) {
  const parser = new EnvParser();

  t.exception(
    () => parser.parse('=value'),
    'Invalid environment variable format: =value\nMust be in format KEY=VALUE',
  );
});

test('EnvParser should handle empty constructor', function (t) {
  const parser = new EnvParser();

  t.alike(parser.getEnv(), {});
});

test('EnvParser should handle multiple commas and whitespace', function (t) {
  const parser = new EnvParser();
  parser.parse('KEY1=value1,  KEY2=value2,,,KEY3=value3');

  t.alike(parser.getEnv(), {
    KEY1: 'value1',
    KEY2: 'value2',
    KEY3: 'value3',
  });
});

test('EnvParser should handle values containing escaped characters', function (t) {
  const parser = new EnvParser();

  // This is how Node.js argv will receive it after shell processing
  parser.parse('A=VERBATIM CONTENTS\\, GO HERE'); // Users will type: --env 'A=VERBATIM CONTENTS\, GO HERE'

  t.alike(parser.getEnv(), {
    A: 'VERBATIM CONTENTS, GO HERE', // Comma should be unescaped in final value
  });
});
