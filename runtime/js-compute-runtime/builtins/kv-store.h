#ifndef JS_COMPUTE_RUNTIME_KV_STORE_H
#define JS_COMPUTE_RUNTIME_KV_STORE_H

#include "builtin.h"
#include "builtins/request-response.h"
#include "host_interface/host_api.h"
#include "js-compute-builtins.h"

namespace builtins {

class KVStoreEntry final : public BuiltinImpl<KVStoreEntry> {
  template <RequestOrResponse::BodyReadResult result_type>
  static bool bodyAll(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool body_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool bodyUsed_get(JSContext *cx, unsigned argc, JS::Value *vp);

public:
  static constexpr const char *class_name = "KVStoreEntry";

  using Slots = RequestOrResponse::Slots;
  static const JSFunctionSpec static_methods[];
  static const JSPropertySpec static_properties[];
  static const JSFunctionSpec methods[];
  static const JSPropertySpec properties[];

  static const unsigned ctor_length = 0;

  static bool init_class(JSContext *cx, JS::HandleObject global);
  static bool constructor(JSContext *cx, unsigned argc, JS::Value *vp);
  static JSObject *create(JSContext *cx, host_api::HttpBody body_handle);
};

class KVStore final : public BuiltinImpl<KVStore> {
  static bool delete_(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool put(JSContext *cx, unsigned argc, JS::Value *vp);

public:
  static constexpr const char *class_name = "KVStore";
  enum class Slots {
    KVStore,
    PendingLookupPromise,
    PendingLookupHandle,
    PendingDeletePromise,
    PendingDeleteHandle,
    Count,
  };
  static const JSFunctionSpec static_methods[];
  static const JSPropertySpec static_properties[];
  static const JSFunctionSpec methods[];
  static const JSPropertySpec properties[];

  static const unsigned ctor_length = 1;

  static bool init_class(JSContext *cx, JS::HandleObject global);
  static bool constructor(JSContext *cx, unsigned argc, JS::Value *vp);
  static host_api::ObjectStorePendingLookup pending_lookup_handle(JSObject *self);
  static bool process_pending_kv_store_lookup(JSContext *cx, JS::HandleObject self);
  static host_api::ObjectStorePendingDelete pending_delete_handle(JSObject *self);
  static bool process_pending_kv_store_delete(JSContext *cx, JS::HandleObject self);
  static bool has_pending_delete_handle(JSObject *self);
};

} // namespace builtins

#endif
