import { assert, assertThrows } from './assertions.js';
import { routes } from './routes.js';
import { Shield } from 'fastly:shielding';

routes.set('/shielding/invalid-shield', () => {
  assertThrows(() => new Shield('i-am-not-a-real-shield'));
});
