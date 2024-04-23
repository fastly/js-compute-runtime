import * as logger from "fastly:logger";
import { routes, isRunningLocally } from "./routes";

const { Logger } = logger;

routes.set("/logger", () => {
  if (isRunningLocally()) {
    let logger = new Logger("ComputeLog");
    logger.log("Hello!");
  }

  return new Response();
});
