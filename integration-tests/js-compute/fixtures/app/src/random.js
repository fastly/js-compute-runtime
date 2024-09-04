/* eslint-env serviceworker */
import { routes } from "./routes.js";
import { pass, assert } from "./assertions.js";

let error;
function random3Decimals() {
  return String(Math.random()).slice(0, 5);
}

let a = random3Decimals();
let b = random3Decimals();

// This tests can fail sporadically as it is testing randomness.
// If it fails, rerun the tests and it should pass, if it does not
// then we may have another bug with how we are seeding the random
// number generator in SpiderMonkey.
routes.set("/random", () => {
  error = assert(
    a !== b,
    true,
    "The first 4 digits were repeated in sequential calls to Math.random() during wizening\n\n" +
      JSON.stringify({ a, b }, undefined, 4),
  );
  if (error) {
    return error;
  }

  let c = random3Decimals();
  let d = random3Decimals();
  error = assert(
    c !== d,
    true,
    "The first 4 digits were repeated in sequential calls to Math.random() during request handling\n\n" +
      JSON.stringify({ c, d }, undefined, 4),
  );
  if (error) {
    return error;
  }

  return pass();
});
