import { Logger } from "fastly:logger";
import { routes, isRunningLocally } from "./routes";

routes.set("/logger", () => {
  if (isRunningLocally()) {
    let logger = new Logger("ComputeLog");
    logger.log("Hello!");
  }

  return new Response();
});
