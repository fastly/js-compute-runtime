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
  static bool metadata(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool metadata_text(JSContext *cx, unsigned argc, JS::Value *vp);

public:
  static constexpr const char *class_name = "KVStoreEntry";

  enum class Slots {
    Request = static_cast<int>(fetch::RequestOrResponse::Slots::RequestOrResponse),
    Body = static_cast<int>(fetch::RequestOrResponse::Slots::Body),
    BodyStream = static_cast<int>(fetch::RequestOrResponse::Slots::BodyStream),
    HasBody = static_cast<int>(fetch::RequestOrResponse::Slots::HasBody),
    BodyUsed = static_cast<int>(fetch::RequestOrResponse::Slots::BodyUsed),
    Headers = static_cast<int>(fetch::RequestOrResponse::Slots::Headers),
    URL = static_cast<int>(fetch::RequestOrResponse::Slots::URL),
    ManualFramingHeaders = static_cast<int>(fetch::RequestOrResponse::Slots::ManualFramingHeaders),
    Backend = static_cast<int>(fetch::RequestOrResponse::Slots::Backend),
    Method = static_cast<int>(fetch::RequestOrResponse::Slots::Count),
    Metadata,
    Count,
  };
  static const JSFunctionSpec static_methods[];
  static const JSPropertySpec static_properties[];
  static const JSFunctionSpec methods[];
  static const JSPropertySpec properties[];

  static const unsigned ctor_length = 0;

  static bool init_class(JSContext *cx, JS::HandleObject global);
  static bool constructor(JSContext *cx, unsigned argc, JS::Value *vp);
  static JSObject *create(JSContext *cx, host_api::HttpBody body_handle,
                          host_api::HostBytes metadata);
};

class KVStore final : public builtins::BuiltinImpl<KVStore> {
  static bool delete_(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool put(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool list(JSContext *cx, unsigned argc, JS::Value *vp);

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
};

} // namespace fastly::kv_store

#endif
