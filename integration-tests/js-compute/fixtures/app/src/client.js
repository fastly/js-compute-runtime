import { assert } from './assertions.js';
import { routes, isRunningLocally } from './routes.js';

routes.set('/client/tlsJA3MD5', (event) => {
  if (!isRunningLocally()) {
    assert(
      typeof event.client.tlsJA3MD5,
      'string',
      'typeof event.client.tlsJA3MD5',
    );
    assert(event.client.tlsJA3MD5.length, 32, 'event.client.tlsJA3MD5.length');
  }
});
routes.set('/client/tlsClientHello', (event) => {
  if (!isRunningLocally()) {
    assert(
      event.client.tlsClientHello instanceof ArrayBuffer,
      true,
      'event.client.tlsClientHello instanceof ArrayBuffer',
    );
    assert(
      typeof event.client.tlsClientHello.byteLength,
      'number',
      'typeof event.client.tlsClientHello.byteLength',
    );
  }
});

routes.set('/client/tlsClientCertificate', (event) => {
  if (!isRunningLocally()) {
    assert(
      event.client.tlsClientCertificate instanceof ArrayBuffer,
      true,
      'event.client.tlsClientCertificate instanceof ArrayBuffer',
    );
    assert(
      event.client.tlsClientCertificate.byteLength,
      0,
      'event.client.tlsClientCertificate.byteLength',
    );
  }
});

routes.set('/client/tlsCipherOpensslName', (event) => {
  if (!isRunningLocally()) {
    assert(
      typeof event.client.tlsCipherOpensslName,
      'string',
      'typeof event.client.tlsCipherOpensslName',
    );
  }
});

routes.set('/client/tlsProtocol', (event) => {
  if (!isRunningLocally()) {
    assert(
      typeof event.client.tlsProtocol,
      'string',
      'typeof event.client.tlsProtocol',
    );
  }
});
