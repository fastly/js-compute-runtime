/* eslint-env serviceworker */

import { routes } from "./routes.js";
import { pass, assert } from "./assertions.js";
import { CacheOverride } from "fastly:cache-override";

routes.set("/headers/construct", async () => {
    const headers = new Headers()
    headers.set('foo', 'bar')
    return new Response("check headers", { headers })
})

routes.set("/headers/non-ascii-latin1-field-value", async () => {
    let response = await fetch("https://http-me.glitch.me/meow?header=cat:é", {
        backend: "httpme"
    })

    let text = response.headers.get('cat')
    console.log("response.headers.get('cat')", response.headers.get('cat'))

    let error = assert(text, "é", `response.headers.get('cat') === "é"`)
    if (error) { return error }
    return pass("ok")
})

routes.set("/headers/from-response/set", async () => {
    const response = await fetch("https://httpbin.org/stream-bytes/11", {
        backend: "httpbin",
        cacheOverride: new CacheOverride('pass')
    });
    response.headers.set("cuStom", "test")
    return response;
})

routes.set("/headers/from-response/delete-invalid", async () => {
    const response = await fetch("https://httpbin.org/stream-bytes/11", {
        backend: "httpbin",
        cacheOverride: new CacheOverride('pass')
    });
    response.headers.delete("none")
    return response;
})


routes.set("/headers/from-response/set-delete", async () => {
    const response = await fetch("https://httpbin.org/stream-bytes/11", {
        backend: "httpbin",
        cacheOverride: new CacheOverride('pass')
    });
    response.headers.set("custom", "test")
    response.headers.delete("access-control-allow-origin")
    return response;
})
