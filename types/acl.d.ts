/// <reference path="../types/globals.d.ts" />

declare module 'fastly:acl' {
  /**
   * @version 3.32.0
   */
  class Acl {
    /**
     * Opens the given ACL store by name
     */
    static open(aclName: string): Acl;

    /**
     * Lookup a given IP address in the ACL list.
     *
     * @example
     * import { Acl } from 'fastly:acl';
     * addEventListener('fetch', async (evt) => {
     *   const myAcl = Acl.open('myacl');
     *   const result = await myAcl.lookup(evt.client.address);
     *   evt.respondWith(new Response(result?.action === 'BLOCK' ? 'blocked' : 'allowed'));
     * });
     *
     * @param ipAddress Ipv6 or IPv4 IP address string
     * @returns An object containing the ACL action and IP prefix if the given IP address matches an ACL entry, or null if there is no match.
     */
    lookup(ipAddress: string): Promise<{
      action: 'ALLOW' | 'BLOCK';
      prefix: string;
    } | null>;
  }
}
