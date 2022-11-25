/// <reference path="../types/fastly:object-store.d.ts" />
import { ObjectStore, ObjectStoreEntry } from "fastly:object-store";
import { expectError, expectType } from 'tsd';

// ObjectStore
{
  expectError(ObjectStore())
  expectError(ObjectStore('secrets'))
  expectType<ObjectStore>(new ObjectStore("secrets"))
  expectError(new ObjectStore('secrets', {}))
  const store = new ObjectStore('secrets')
  expectError(store.get())
  expectError(store.get(1))
  expectType<Promise<ObjectStoreEntry|null>>(store.get('cat'))
  expectError(store.put())
  expectError(store.put('cat'))
  expectError(store.put('cat', 1))
  expectType<Promise<undefined>>(store.put('cat', 'Aki'))
}

// ObjectStoreEntry
{
  const entry = {} as ObjectStoreEntry
  expectType<ReadableStream<any>>(entry.body)
  expectType<boolean>(entry.bodyUsed)
  expectType<Promise<ArrayBuffer>>(entry.arrayBuffer())
  expectType<Promise<object>>(entry.json())
  expectType<Promise<string>>(entry.text())
}