import { routes } from "./routes.js";
routes.set("/error", async () => {
  throw new Error("uh oh");
});
