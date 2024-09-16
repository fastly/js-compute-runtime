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
