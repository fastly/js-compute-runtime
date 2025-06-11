import { pass, ok, strictEqual, assertThrows } from './assertions.js';
import { routes } from './routes.js';
import { purgeSurrogateKey, vCpuTime } from 'fastly:compute';

routes.set('/compute/get-vcpu-ms', () => {
  const cpuTime = vCpuTime();
  strictEqual(typeof cpuTime, 'number');
  ok(cpuTime > 0, 'cpuTime > 0');
  ok(cpuTime < 3000, 'cputime < 3000');
  const arr = new Array(100_000).fill(1);
  for (let j = 1; j < 100; j++) {
    for (let i = 1; i < 100_000; i++) {
      arr[i] = (arr[i] + arr[i - 1] + i) / 3;
    }
  }
  const cpuTime2 = vCpuTime();
  ok(cpuTime2 > cpuTime, 'cpuTime2 > cpuTime');
  ok(cpuTime2 - cpuTime > 1, 'cpuTime2 - cpuTime > 1');
  return pass('ok');
});

routes.set('/compute/purge-surrogate-key-invalid', () => {
  assertThrows(
    () => {
      purgeSurrogateKey();
    },
    TypeError,
    'purgeSurrogateKey: At least 1 argument required, but only 0 passed',
  );
  return pass('ok');
});

routes.set('/compute/purge-surrogate-key-hard', () => {
  purgeSurrogateKey('test');
  return pass('ok');
});

routes.set('/compute/purge-surrogate-key-soft', () => {
  purgeSurrogateKey('test', true);
  return pass('ok');
});
