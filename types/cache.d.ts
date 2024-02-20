declare module "fastly:cache" {

  interface PurgeOptions {
    /**
     * "pop" - This will remove the content from the POP that contains the currently executing instance.
     * "global" - This will remove the content from all of Fastly.
     */
    scope: "pop" | "global"
  }

  export class SimpleCache {
    static get(key: string): SimpleCacheEntry | null;
    /**
      * @deprecated Use `SimpleCache.getOrSet` instead.
      */
    static set(key: string, value: BodyInit, ttl: number): undefined;
    /**
      * @deprecated Use `SimpleCache.getOrSet` instead.
      */
    static set(key: string, value: ReadableStream, ttl: number, length: number): undefined;
    static getOrSet(key: string, set: () => Promise<{value: BodyInit,  ttl: number}>): Promise<SimpleCacheEntry>;
    static getOrSet(key: string, set: () => Promise<{value: ReadableStream, ttl: number, length: number}>): Promise<SimpleCacheEntry>;
    static purge(key: string, options: PurgeOptions): undefined;
  }

  export interface SimpleCacheEntry {
    get body(): ReadableStream;
    get bodyUsed(): boolean;
    text(): Promise<string>;
    json(): Promise<object>;
    arrayBuffer(): Promise<ArrayBuffer>;
  }
}
