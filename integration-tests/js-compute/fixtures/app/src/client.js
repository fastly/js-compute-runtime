import { strictEqual } from './assertions.js';
import { routes, isRunningLocally } from './routes.js';

routes.set('/client/tlsJA3MD5', (event) => {
  if (isRunningLocally()) {
    strictEqual(event.client.tlsJA3MD5, null);
  } else {
    strictEqual(
      typeof event.client.tlsJA3MD5,
      'string',
      'typeof event.client.tlsJA3MD5',
    );
    strictEqual(
      event.client.tlsJA3MD5.length,
      32,
      'event.client.tlsJA3MD5.length',
    );
  }
});
routes.set('/client/tlsClientHello', (event) => {
  if (isRunningLocally()) {
    strictEqual(event.client.tlsClientHello, null);
  } else {
    strictEqual(
      event.client.tlsClientHello instanceof ArrayBuffer,
      true,
      'event.client.tlsClientHello instanceof ArrayBuffer',
    );
    strictEqual(
      typeof event.client.tlsClientHello.byteLength,
      'number',
      'typeof event.client.tlsClientHello.byteLength',
    );
  }
});

routes.set('/client/tlsClientCertificate', (event) => {
  if (isRunningLocally()) {
    strictEqual(event.client.tlsClientCertificate, null);
  } else {
    strictEqual(
      event.client.tlsClientCertificate instanceof ArrayBuffer,
      true,
      'event.client.tlsClientCertificate instanceof ArrayBuffer',
    );
    strictEqual(
      event.client.tlsClientCertificate.byteLength,
      0,
      'event.client.tlsClientCertificate.byteLength',
    );
  }
});

routes.set('/client/tlsCipherOpensslName', (event) => {
  if (isRunningLocally()) {
    strictEqual(event.client.tlsCipherOpensslName, null);
  } else {
    strictEqual(
      typeof event.client.tlsCipherOpensslName,
      'string',
      'typeof event.client.tlsCipherOpensslName',
    );
  }
});

routes.set('/client/tlsProtocol', (event) => {
  if (isRunningLocally()) {
    strictEqual(event.client.tlsProtocol, null);
  } else {
    strictEqual(
      typeof event.client.tlsProtocol,
      'string',
      'typeof event.client.tlsProtocol',
    );
  }
});

routes.set('/client/tlsJA4', (event) => {
  if (isRunningLocally()) {
    strictEqual(event.client.tlsJA4, null);
  } else {
    strictEqual(
      typeof event.client.tlsJA4,
      'string',
      'typeof event.client.tlsJA4',
    );
  }
});

routes.set('/client/h2Fingerprint', (event) => {
  if (isRunningLocally()) {
    strictEqual(event.client.h2Fingerprint, null);
  } else {
    // h2Fingerprint may be null for HTTP/1.1 connections
    const fp = event.client.h2Fingerprint;
    strictEqual(
      fp === null || typeof fp === 'string',
      true,
      'event.client.h2Fingerprint is null or string',
    );
  }
});

routes.set('/client/ohFingerprint', (event) => {
  if (isRunningLocally()) {
    strictEqual(event.client.ohFingerprint, null);
  } else {
    strictEqual(
      typeof event.client.ohFingerprint,
      'string',
      'typeof event.client.ohFingerprint',
    );
  }
});
