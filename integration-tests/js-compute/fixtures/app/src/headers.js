/* eslint-env serviceworker */

import { routes } from "./routes.js";
import { pass, assert } from "./assertions.js";


routes.set("/headers/non-ascii-latin1-field-value", async () => {
    let response = await fetch("https://secondly-enhanced-yak.edgecompute.app", {
        backend: "meow",
        headers: {
            cat: 'é'
        }
    })

    let text = response.headers.get('cat')
    console.log("response.headers.get('cat')", response.headers.get('cat'))

    let error = assert(text, "é", `response.headers.get('cat') === "é"`)
    if (error) { return error }
    return pass("ok")
})
