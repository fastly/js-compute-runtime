#ifndef FASTLY_KV_STORE_H
#define FASTLY_KV_STORE_H

#include "../host-api/host_api_fastly.h"
#include "./fetch/request-response.h"
#include "builtin.h"

namespace fastly::kv_store {

class KVStoreEntry final : public builtins::BuiltinImpl<KVStoreEntry> {
  template <fetch::RequestOrResponse::BodyReadResult result_type>
  static bool bodyAll(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool body_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool bodyUsed_get(JSContext *cx, unsigned argc, JS::Value *vp);

public:
  static constexpr const char *class_name = "KVStoreEntry";

  using Slots = fetch::RequestOrResponse::Slots;
  static const JSFunctionSpec static_methods[];
  static const JSPropertySpec static_properties[];
  static const JSFunctionSpec methods[];
  static const JSPropertySpec properties[];

  static const unsigned ctor_length = 0;

  static bool init_class(JSContext *cx, JS::HandleObject global);
  static bool constructor(JSContext *cx, unsigned argc, JS::Value *vp);
  static JSObject *create(JSContext *cx, host_api::HttpBody body_handle);
};

class KVStore final : public builtins::BuiltinImpl<KVStore> {
  static bool delete_(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool put(JSContext *cx, unsigned argc, JS::Value *vp);

public:
  static constexpr const char *class_name = "KVStore";
  enum class Slots {
    KVStore,
    Count,
  };
  static const JSFunctionSpec static_methods[];
  static const JSPropertySpec static_properties[];
  static const JSFunctionSpec methods[];
  static const JSPropertySpec properties[];

  static const unsigned ctor_length = 1;

  static bool constructor(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool process_pending_kv_store_lookup(FastlyHandle handle, JS::HandleObject context,
                                              JS::HandleObject promise);
  static bool process_pending_kv_store_delete(FastlyHandle handle, JS::HandleObject context,
                                              JS::HandleObject promise);
};

} // namespace fastly::kv_store

#endif
