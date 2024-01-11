/* eslint-env serviceworker */

import { routes } from "./routes.js";
import { pass, assert } from "./assertions.js";

routes.set("/headers/wtf8", async () => {
    let response = await fetch("https://http-me.glitch.me/meow?header=cat:%C3%A9", {
        backend: "httpme"
    })

    let text = response.headers.get('cat')

    let error = assert(text, "é", `response.headers.get('cat') === "é"`)
    if (error) { return error }
    return pass("ok")
})
