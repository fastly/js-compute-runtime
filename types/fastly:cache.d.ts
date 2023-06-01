declare module "fastly:cache" {
  export class SimpleCache {
    static get(key: string): SimpleCacheEntry | null;
    static set(key: string, value: BodyInit, ttl: number): undefined;
    static set(key: string, value: ReadableStream, ttl: number, length: number): undefined;
    static delete(key: string): undefined;
  }

  export interface SimpleCacheEntry {
    get body(): ReadableStream;
    get bodyUsed(): boolean;
    text(): Promise<string>;
    json(): Promise<object>;
    arrayBuffer(): Promise<ArrayBuffer>;
  }
}
