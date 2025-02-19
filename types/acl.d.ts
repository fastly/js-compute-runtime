/// <reference path="../types/globals.d.ts" />

declare module 'fastly:acl' {
  /**
   * Opens the given ACL store by name
   */
  export function open(aclName: string): Acl;

  class Acl {
    /**
     * Lookup a given IP address in the ACL list.
     *
     * @example
     * import { open } from 'fastly:acl';
     * addEventListener('fetch', async (evt) => {
     *   const acl = open('myacl');
     *   const { action, prefix } = await acl.lookup(evt.client.address);
     *   evt.respondWith(new Response(action === 'block' ? 'blocked' : 'allowed'));
     * });
     *
     * @param ipAddress Ipv6 or IPv4 IP address string
     */
    lookup(ipAddress: string): Promise<{
      action: 'allow' | 'block';
      prefix: string;
    }>;
  }
}
