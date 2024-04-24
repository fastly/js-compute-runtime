import { routes } from "./routes.js";
import { assert, pass, fail } from "./assertions.js";
import { includeBytes } from "fastly:experimental";

let message, err
try {
  message = includeBytes("message.txt");
} catch (e) {
  err = e;
}

routes.set("/includeBytes", () => {
  if (err) {
    return fail(err);
  }
  let error = assert(Array.from(message), expected, `message === expected`);
  if (error) { return error; }
  return pass();
});
