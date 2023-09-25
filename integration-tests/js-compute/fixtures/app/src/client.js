import { pass, assert } from "./assertions.js";
import { routes, isRunningLocally } from "./routes.js";

let error;
routes.set("/client/tlsJA3MD5", event => {
    if (!isRunningLocally()) {
        error = assert(typeof event.client.tlsJA3MD5, "string", 'typeof event.client.tlsJA3MD5')
        if (error) { return error }
        error = assert(event.client.tlsJA3MD5.length, 32, 'event.client.tlsJA3MD5.length')
        if (error) { return error }
    }
    return pass('ok')
});
routes.set("/client/tlsClientHello", event => {
    if (!isRunningLocally()) {
        error = assert(event.client.tlsClientHello instanceof ArrayBuffer, true, 'event.client.tlsClientHello instanceof ArrayBuffer')
        if (error) { return error }
        error = assert(typeof event.client.tlsClientHello.byteLength, 'number', 'typeof event.client.tlsClientHello.byteLength')
        if (error) { return error }
    }
    return pass('ok')
});

routes.set("/client/tlsClientCertificate", event => {
    if (!isRunningLocally()) {
        error = assert(event.client.tlsClientCertificate instanceof ArrayBuffer, true, 'event.client.tlsClientCertificate instanceof ArrayBuffer')
        if (error) { return error }
        error = assert(event.client.tlsClientCertificate.byteLength, 0, 'event.client.tlsClientCertificate.byteLength')
        if (error) { return error }
    }
    return pass('ok')
});

routes.set("/client/tlsCipherOpensslName", event => {
    if (!isRunningLocally()) {
        error = assert(typeof event.client.tlsCipherOpensslName, 'string', 'typeof event.client.tlsCipherOpensslName')
        if (error) { return error }
    }
    return pass('ok')
});

routes.set("/client/tlsProtocol", event => {
    if (!isRunningLocally()) {
        error = assert(typeof event.client.tlsProtocol, 'string', 'typeof event.client.tlsProtocol')
        if (error) { return error }
    }
    return pass('ok')
});
