import { pass, assert } from "./assertions.js";
import { routes, isRunningLocally } from "./routes.js";

let error;
routes.set("/server/address", (event) => {
  error = assert(
    typeof event.server.address,
    "string",
    "typeof event.server.address",
  );
  if (error) {
    return error;
  }

  if (isRunningLocally()) {
    error = assert(event.server.address, "127.0.0.1", "event.server.address");
    if (error) {
      return error;
    }
  }
  return pass("ok");
});
