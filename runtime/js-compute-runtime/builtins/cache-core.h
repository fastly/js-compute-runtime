#ifndef JS_COMPUTE_RUNTIME_CACHE_CORE_H
#define JS_COMPUTE_RUNTIME_CACHE_CORE_H

#include "builtin.h"
#include "js-compute-builtins.h"
#include "request-response.h"

namespace builtins {

// export class CacheState {
//   found(): boolean;
//   usable(): boolean;
//   stale(): boolean;
//   mustInsertOrUpdate(): boolean;
// }
class CacheState : public BuiltinImpl<CacheState> {
  static constexpr const uint8_t found_flag = 1 << 0;
  static constexpr const uint8_t usable_flag = 1 << 1;
  static constexpr const uint8_t stale_flag = 1 << 2;
  static constexpr const uint8_t must_insert_or_update_flag = 1 << 3;

  // found(): boolean;
  static bool found(JSContext *cx, unsigned argc, JS::Value *vp);

  // usable(): boolean;
  static bool usable(JSContext *cx, unsigned argc, JS::Value *vp);

  // stale(): boolean;
  static bool stale(JSContext *cx, unsigned argc, JS::Value *vp);

  // mustInsertOrUpdate(): boolean;
  static bool mustInsertOrUpdate(JSContext *cx, unsigned argc, JS::Value *vp);

public:
  static constexpr const char *class_name = "CacheState";
  static const int ctor_length = 0;
  enum Slots { State, Count };

  static const JSFunctionSpec static_methods[];
  static const JSPropertySpec static_properties[];
  static const JSFunctionSpec methods[];
  static const JSPropertySpec properties[];

  static bool constructor(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool init_class(JSContext *cx, JS::HandleObject global);
  static JSObject *create(JSContext *cx, uint32_t handle);
};

class CacheEntry : public BuiltinImpl<CacheEntry> {
  // cache-close: func(handle: cache-handle) -> result<_, error>
  // close(): void;
  static bool close(JSContext *cx, unsigned argc, JS::Value *vp);

  // cache-get-state: func(handle: cache-handle) -> result<cache-lookup-state, error>
  // state(): CacheState;
  static bool state(JSContext *cx, unsigned argc, JS::Value *vp);

  /// cache-get-user-metadata: func(handle: cache-handle) -> result<list<u8>, error>
  // userMetadata(): ArrayBuffer;
  static bool userMetadata(JSContext *cx, unsigned argc, JS::Value *vp);

  // cache-get-body: func(handle: cache-handle, options: cache-get-body-options) ->
  // result<body-handle, error> body(options?: CacheBodyOptions): ReadableStream;
  static bool body(JSContext *cx, unsigned argc, JS::Value *vp);

  // cache-get-length: func(handle: cache-handle) -> result<u64, error>
  // length(): number;
  static bool length(JSContext *cx, unsigned argc, JS::Value *vp);

  // cache-get-max-age-ns: func(handle: cache-handle) -> result<u64, error>
  // maxAge(): number;
  static bool maxAge(JSContext *cx, unsigned argc, JS::Value *vp);

  // cache-get-stale-while-revalidate-ns: func(handle: cache-handle) -> result<u64, error>
  // staleWhileRevalidate(): number;
  static bool staleWhileRevalidate(JSContext *cx, unsigned argc, JS::Value *vp);

  // cache-get-age-ns: func(handle: cache-handle) -> result<u64, error>
  // age(): number;
  static bool age(JSContext *cx, unsigned argc, JS::Value *vp);

  // cache-get-hits: func(handle: cache-handle) -> result<u64, error>
  // hits(): number;
  static bool hits(JSContext *cx, unsigned argc, JS::Value *vp);

public:
  static constexpr const char *class_name = "CacheEntry";
  static const int ctor_length = 0;
  enum Slots {
    Body = static_cast<int>(RequestOrResponse::Slots::Body),
    BodyStream = static_cast<int>(RequestOrResponse::Slots::BodyStream),
    HasBody = static_cast<int>(RequestOrResponse::Slots::HasBody),
    BodyUsed = static_cast<int>(RequestOrResponse::Slots::BodyUsed),
    Handle = static_cast<int>(RequestOrResponse::Slots::Count),
    Count
  };

  static const JSFunctionSpec static_methods[];
  static const JSPropertySpec static_properties[];
  static const JSFunctionSpec methods[];
  static const JSPropertySpec properties[];

  static bool is_instance(JSObject *obj);
  static bool is_instance(JS::Value val);
  static host_api::CacheHandle get_cache_handle(JSObject *self);

  static bool constructor(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool init_class(JSContext *cx, JS::HandleObject global);
  static JSObject *create(JSContext *cx, uint32_t handle);
};

class TransactionCacheEntry : public BuiltinImpl<TransactionCacheEntry> {

  static bool insert(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool insertAndStreamBack(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool update(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool cancel(JSContext *cx, unsigned argc, JS::Value *vp);

public:
  static constexpr const char *class_name = "TransactionCacheEntry";
  static const int ctor_length = 0;
  enum Slots {
    Body = static_cast<int>(CacheEntry::Slots::Body),
    BodyStream = static_cast<int>(CacheEntry::Slots::BodyStream),
    HasBody = static_cast<int>(CacheEntry::Slots::HasBody),
    BodyUsed = static_cast<int>(CacheEntry::Slots::BodyUsed),
    Handle = static_cast<int>(CacheEntry::Slots::Handle),
    Count
  };

  static const JSFunctionSpec static_methods[];
  static const JSPropertySpec static_properties[];
  static const JSFunctionSpec methods[];
  static const JSPropertySpec properties[];

  static bool constructor(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool init_class(JSContext *cx, JS::HandleObject global);
  static JSObject *create(JSContext *cx, uint32_t handle);
};

class CoreCache : public BuiltinImpl<CoreCache> {
  // cache-lookup: func(cache-key: string, options: cache-lookup-options) -> result<cache-handle,
  // error> static lookup(key: string, options?: LookupOptions): CacheEntry | null;
  static bool lookup(JSContext *cx, unsigned argc, JS::Value *vp);

  // cache-insert: func(cache-key: string, options: cache-write-options) -> result<body-handle,
  // error> static insert(key: string, options: InsertOptions): FastlyBody;
  static bool insert(JSContext *cx, unsigned argc, JS::Value *vp);

  // transaction-lookup: func(cache-key: string, options: cache-lookup-options) ->
  // result<cache-handle, error> static transactionLookup(key: string, optoptions?: LookupOptions):
  // CacheEntry | null;
  static bool transactionLookup(JSContext *cx, unsigned argc, JS::Value *vp);

public:
  static constexpr const char *class_name = "CoreCache";
  static const int ctor_length = 0;
  enum Slots { Count };

  static const JSFunctionSpec static_methods[];
  static const JSPropertySpec static_properties[];
  static const JSFunctionSpec methods[];
  static const JSPropertySpec properties[];

  static bool constructor(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool init_class(JSContext *cx, JS::HandleObject global);
};

} // namespace builtins

#endif
