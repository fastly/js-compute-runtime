/// <reference path="../types/globals.d.ts" />

declare module 'fastly:acl' {
  class Acl {
    /**
     * Opens the given ACL store by name
     */
    static open(aclName: string);

    /**
     * Lookup a given IP address in the ACL list.
     *
     * @example
     * import { Acl } from 'fastly:acl';
     * addEventListener('fetch', async (evt) => {
     *   const myAcl = Acl.open('myacl');
     *   const { action, prefix } = await myAcl.lookup(evt.client.address);
     *   evt.respondWith(new Response(action === 'block' ? 'blocked' : 'allowed'));
     * });
     *
     * @param ipAddress Ipv6 or IPv4 IP address string
     */
    lookup(ipAddress: string): Promise<{
      action: 'allow' | 'block';
      prefix: string;
    } | null>;
  }
}
