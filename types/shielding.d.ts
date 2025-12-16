/// <reference path="../types/backend.d.ts" />

declare module 'fastly:shielding' {
  export class Shield {
    constructor(name: string);
    runningOn(): boolean;
    unencryptedBackend(): import('fastly:backend').Backend;
    encryptedBackend(): import('fastly:backend').Backend;
  }
}
