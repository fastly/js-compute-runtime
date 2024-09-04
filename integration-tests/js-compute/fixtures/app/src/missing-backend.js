import { pass, assertRejects } from "./assertions.js";
import { routes } from "./routes.js";

routes.set("/missing-backend", async () => {
  let error = await assertRejects(
    async () => fetch("https://example.com", { backend: "missing" }),
    TypeError,
    `Requested backend named 'missing' does not exist`,
  );
  if (error) {
    return error;
  }
  return pass();
});
