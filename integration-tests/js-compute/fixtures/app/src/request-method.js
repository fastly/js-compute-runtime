/* eslint-env serviceworker */

import { routes } from "./routes.js";

routes.set("/request/method", (event) => {
    return new Response(event.request.method)
});
