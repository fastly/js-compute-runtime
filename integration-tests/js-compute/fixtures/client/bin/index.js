/// <reference path="../../../../../types/index.d.ts" />
/* eslint-env serviceworker */
import { pass, assert } from "../../../assertions.js";
import { routes } from "../../../test-harness.js";

let error;

routes.set("/client/tlsJA3MD5", event => {
    error = assert(typeof event.client.tlsJA3MD5, "string", 'typeof event.client.tlsJA3MD5')
    if (error) { return error }
    error = assert(event.client.tlsJA3MD5.length, 32, 'event.client.tlsJA3MD5.length')
    if (error) { return error }
    return pass('ok')
});
routes.set("/client/tlsClientHello", event => {
    console.log('hello');
    console.log('event.client.tlsJA3MD5', event.client.tlsJA3MD5);
    console.log('event.client.tlsClientHello', event.client.tlsClientHello);
    error = assert(event.client.tlsClientHello instanceof ArrayBuffer, true, 'event.client.tlsClientHello instanceof ArrayBuffer')
    if (error) { return error }
    error = assert(typeof event.client.tlsClientHello.byteLength, 'number', 'typeof event.client.tlsClientHello.byteLength')
    if (error) { return error }
    return pass('ok')
});

routes.set("/client/tlsClientCertificate", event => {
    error = assert(event.client.tlsClientCertificate instanceof ArrayBuffer, true, 'event.client.tlsClientCertificate instanceof ArrayBuffer')
    if (error) { return error }
    error = assert(event.client.tlsClientCertificate.byteLength, 0, 'event.client.tlsClientCertificate.byteLength')
    if (error) { return error }
    return pass('ok')
});

routes.set("/client/tlsCipherOpensslName", event => {
    error = assert(typeof event.client.tlsCipherOpensslName, 'string', 'typeof event.client.tlsCipherOpensslName')
    if (error) { return error }
    return pass('ok')
});

routes.set("/client/tlsProtocol", event => {
    console.log("tlsProtocol", event.client.tlsProtocol)
    error = assert(typeof event.client.tlsProtocol, 'string', 'typeof event.client.tlsProtocol')
    if (error) { return error }
    return pass('ok')
});
