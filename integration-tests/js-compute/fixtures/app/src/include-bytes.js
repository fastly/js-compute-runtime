import { routes } from './routes.js';
import { assert } from './assertions.js';
import { includeBytes } from 'fastly:experimental';

let message;
try {
  message = includeBytes('message.txt');
} catch {}

const expected = [
  104, 101, 108, 108, 111, 32, 105, 110, 99, 108, 117, 100, 101, 66, 121, 116,
  101, 115, 10,
];

routes.set('/includeBytes', () => {
  assert(Array.from(message), expected, `message === expected`);
});

let nope = null;
let dotfileNotOk = null;
let dotfileOk = null;

try {
  nope = includeBytes('../../../../README.md');
} catch {}

try {
  dotfileOk = includeBytes('./.hidden.txt');
} catch {}

try {
  dotfileNotOk = includeBytes('../../.hidden.txt');
} catch {}

routes.set('/includeBytes/sandbox', () => {
  assert(nope, null, 'includeBytes sandboxes its path to the project dir');
  assert(
    dotfileNotOk,
    null,
    'includeBytes sandboxes its path to the project dir',
  );
  assert(
    Array.from(dotfileOk),
    [105, 100, 107, 10],
    'dotfile is included just fine',
  );
});
