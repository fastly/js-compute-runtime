/// <reference path="../types/backend.d.ts" />

declare module 'fastly:shielding' {
  interface ShieldBackendConfiguration {
    firstByteTimeout?: number;
  }
  export class Shield {
    constructor(name: string);
    runningOn(): boolean;
    unencryptedBackend(
      configuration?: ShieldBackendConfiguration,
    ): import('fastly:backend').Backend;
    encryptedBackend(
      configuration?: ShieldBackendConfiguration,
    ): import('fastly:backend').Backend;
  }
}
