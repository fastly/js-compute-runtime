/* eslint-env serviceworker */
import { Logger } from "fastly:logger";

addEventListener("fetch", (event) => {
  let logger = new Logger("ComputeLog");
  logger.log("Hello!");

  // Build a response
  let response = new Response();

  // Send our response back to the client
  event.respondWith(response);
});
