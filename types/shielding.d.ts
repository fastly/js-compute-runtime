import { Backend } from 'fastly:backend';

declare module 'fastly:shielding' {
  export class Shield {
    constructor(name: string);
    runningOn(): boolean;
    unencryptedBackend(): Backend;
    encryptedBackend(): Backend;
  }
}
