/* eslint-env serviceworker */
import { pass, assert } from "./assertions.js";
import { routes } from "./routes.js";

routes.set("/urlsearchparams/sort", async () => {
  const urlObj = new URL("http://www.example.com");
  urlObj.searchParams.sort();
  let error = assert(
    urlObj.toString(),
    "http://www.example.com/",
    `urlObj.toString()`,
  );
  if (error) {
    return error;
  }
  return pass();
});
