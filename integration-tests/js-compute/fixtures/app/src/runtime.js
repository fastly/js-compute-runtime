import { pass, ok, strictEqual, assertThrows } from './assertions-throwing.js';
import { routes } from './routes.js';
import { purgeSurrogateKey, vCpuTime } from 'fastly:runtime';

routes.set('/runtime/get-vcpu-ms', () => {
  const cpuTime = vCpuTime();
  strictEqual(typeof cpuTime, 'number');
  ok(cpuTime > 0);
  ok(cpuTime < 3000);
  const arr = [];
  for (let i = 0; i < 10_000; i++) {
    arr.push(i);
  }
  const cpuTime2 = vCpuTime();
  ok(cpuTime2 > cpuTime);
  ok(cpuTime2 - cpuTime > 1);
  ok(cpuTime2 - cpuTime < 3000);
  return pass('ok');
});

routes.set('/runtime/purge-surrogate-key-invalid', () => {
  assertThrows(
    () => {
      purgeSurrogateKey();
    },
    TypeError,
    'purgeSurrogateKey: At least 1 argument required, but only 0 passed',
  );
  return pass('ok');
});

routes.set('/runtime/purge-surrogate-key-hard', () => {
  purgeSurrogateKey('test');
  return pass('ok');
});

routes.set('/runtime/purge-surrogate-key-soft', () => {
  purgeSurrogateKey('test', true);
  return pass('ok');
});
