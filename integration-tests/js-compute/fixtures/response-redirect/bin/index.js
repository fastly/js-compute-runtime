/* eslint-env serviceworker */
import { pass, assert, assertThrows } from "../../../assertions.js";
import { routes } from "../../../test-harness.js";

routes.set("/response/redirect", async () => {
    const url = "http://test.url:1234/";
    const redirectResponse = Response.redirect(url);
    let error = assert(redirectResponse.type, "default");
    if (error) { return error; }
    error = assert(redirectResponse.redirected, false);
    if (error) { return error; }
    error = assert(redirectResponse.ok, false);
    if (error) { return error; }
    error = assert(redirectResponse.status, 302, "Default redirect status is 302");
    if (error) { return error; }
    error = assert(redirectResponse.headers.get("Location"), url)
    if (error) { return error; }
    error = assert(redirectResponse.statusText, "");
    if (error) { return error; }

    for (const status of [301, 302, 303, 307, 308]) {
        const redirectResponse = Response.redirect(url, status);
        error = assert(redirectResponse.type, "default");
        if (error) { return error; }
        error = assert(redirectResponse.redirected, false);
        if (error) { return error; }
        error = assert(redirectResponse.ok, false);
        if (error) { return error; }
        error = assert(redirectResponse.status, status, "Redirect status is " + status);
        if (error) { return error; }
        error = assert(redirectResponse.headers.get("Location"), url);
        if (error) { return error; }
        error = assert(redirectResponse.statusText, "");
        if (error) { return error; }
    }
    const invalidUrl = "http://:This is not an url";
    error = assertThrows(function () { Response.redirect(invalidUrl); }, TypeError);
    if (error) { return error; }
    for (const invalidStatus of [200, 309, 400, 500]) {
        error = assertThrows(function () { Response.redirect(url, invalidStatus); }, RangeError);
        if (error) { return error; }
    }
    return pass()
})
