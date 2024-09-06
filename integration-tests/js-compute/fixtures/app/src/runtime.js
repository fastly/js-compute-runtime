import {
  pass,
  ok,
  strictEqual
} from "./assertions-throwing.js";
import { routes } from "./routes.js";
import { vCpuTime } from "fastly:runtime";

routes.set("/runtime/get-vcpu-ms", () => {
  const cpuTime = vCpuTime();
  strictEqual(typeof cpuTime, 'number');
  ok(cpuTime > 0);
  ok(cpuTime < 1000);
  const arr = [];
  for (let i = 0; i < 1000; i++) {
    arr.push(i);
  }
  const cpuTime2 = vCpuTime();
  ok(cpuTime2 > cpuTime);
  ok(cpuTime2 - cpuTime > 1);
  ok(cpuTime2 - cpuTime < 1000);
  return pass('ok');
});
