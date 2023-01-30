/* eslint-env serviceworker */
import { env } from 'fastly:env';
import { fail } from "../../../assertions.js";

addEventListener("fetch", event => {
    event.respondWith(app(event))
})
/**
 * @param {FetchEvent} event
 * @returns {Response}
 */
async function app(event) {
    try {
        const path = (new URL(event.request.url)).pathname;
        console.log(`path: ${path}`)
        console.log(`FASTLY_SERVICE_VERSION: ${env('FASTLY_SERVICE_VERSION')}`)
        if (routes.has(path)) {
            const routeHandler = routes.get(path);
            return await routeHandler()
        }
        return fail(`${path} endpoint does not exist`)
    } catch (error) {
        return fail(`The routeHandler threw an error: ${error.message}` + '\n' + error.stack)
    }
}

const routes = new Map();
routes.set('/', () => {
    routes.delete('/');
    let test_routes = Array.from(routes.keys())
    return new Response(JSON.stringify(test_routes), { 'headers': { 'content-type': 'application/json' } });
});
{
  routes.set("/multiple-set-cookie/response-init", async () => {
    let h = new Headers();
    
    h.append("Set-Cookie", "test=1; expires=Tue, 06-Dec-2022 12:34:56 GMT; Max-Age=60; Path=/; HttpOnly; Secure, test_id=1; Max-Age=60; Path=/; expires=Tue, 06-Dec-2022 12:34:56 GMT, test_id=1; Max-Age=60; Path=/; expires=Tue, 06-Dec-2022 12:34:56 GMT");
    h.append("Set-Cookie", "test2=2");
    h.append("Set-Cookie", "test3=3");
    h.append("Set-Cookie", "test4=4");
    h.append("Set-Cookie", "test5=5");
    h.append("Set-Cookie", "test6=6");
    h.append("Set-Cookie", "test7=7");
    h.append("Set-Cookie", "test8=8");
    h.append("Set-Cookie", "test9=9");
    h.append("Set-Cookie", "test10=10");
    h.append("Set-Cookie", "test11=11");
    let r =  new Response("Hello", {
      headers: h
    });
    return r;
  });
  routes.set("/multiple-set-cookie/response-direct", async () => {
    let r = new Response("Hello");

    r.headers.append("Set-Cookie", "test=1; expires=Tue, 06-Dec-2022 12:34:56 GMT; Max-Age=60; Path=/; HttpOnly; Secure, test_id=1; Max-Age=60; Path=/; expires=Tue, 06-Dec-2022 12:34:56 GMT, test_id=1; Max-Age=60; Path=/; expires=Tue, 06-Dec-2022 12:34:56 GMT");
    r.headers.append("Set-Cookie", "test2=2");
    r.headers.append("Set-Cookie", "test3=3");
    r.headers.append("Set-Cookie", "test4=4");
    r.headers.append("Set-Cookie", "test5=5");
    r.headers.append("Set-Cookie", "test6=6");
    r.headers.append("Set-Cookie", "test7=7");
    r.headers.append("Set-Cookie", "test8=8");
    r.headers.append("Set-Cookie", "test9=9");
    r.headers.append("Set-Cookie", "test10=10");
    r.headers.append("Set-Cookie", "test11=11");
    return r;
  });
  routes.set("/multiple-set-cookie/downstream", async () => {
    let response = await fetch('https://httpbin.org/cookies/set?1=1&2=2&3=3&4=4&5=5&6=6&7=7&8=8&9=9&10=10&11=11', {
      backend: 'httpbin'
    });

    return new Response('', response);
  });
}
