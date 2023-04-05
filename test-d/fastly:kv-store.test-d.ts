/// <reference path="../types/fastly:kv-store.d.ts" />
import { KVStore, KVStoreEntry } from "fastly:kv-store";
import { expectError, expectType } from 'tsd';

// KVStore
{
  expectError(KVStore())
  expectError(KVStore('secrets'))
  expectType<KVStore>(new KVStore("secrets"))
  expectError(new KVStore('secrets', {}))
  const store = new KVStore('secrets')
  expectError(store.get())
  expectError(store.get(1))
  expectType<Promise<KVStoreEntry|null>>(store.get('cat'))
  expectError(store.put())
  expectError(store.put('cat'))
  expectError(store.put('cat', 1))
  expectType<Promise<undefined>>(store.put('cat', 'Aki'))
}

// KVStoreEntry
{
  const entry = {} as KVStoreEntry
  expectType<ReadableStream<any>>(entry.body)
  expectType<boolean>(entry.bodyUsed)
  expectType<Promise<ArrayBuffer>>(entry.arrayBuffer())
  expectType<Promise<object>>(entry.json())
  expectType<Promise<string>>(entry.text())
}