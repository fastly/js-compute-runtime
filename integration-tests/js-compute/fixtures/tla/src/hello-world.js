import { routes } from "./routes";

routes.set("/hello-world", () => {
  return new Response('hello world');
});
