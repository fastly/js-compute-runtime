#ifndef FASTLY_CACHE_SIMPLE_H
#define FASTLY_CACHE_SIMPLE_H

#include "../host-api/host_api_fastly.h"
#include "./fetch/request-response.h"
#include "builtin.h"

namespace fastly::cache_simple {

class SimpleCacheEntry final : public builtins::BuiltinImpl<SimpleCacheEntry> {
  template <fetch::RequestOrResponse::BodyReadResult result_type>
  static bool bodyAll(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool body_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool bodyUsed_get(JSContext *cx, unsigned argc, JS::Value *vp);

public:
  static constexpr const char *class_name = "SimpleCacheEntry";

  using Slots = fetch::RequestOrResponse::Slots;
  static const JSFunctionSpec static_methods[];
  static const JSPropertySpec static_properties[];
  static const JSFunctionSpec methods[];
  static const JSPropertySpec properties[];

  static const unsigned ctor_length = 0;

  static bool constructor(JSContext *cx, unsigned argc, JS::Value *vp);
  static JSObject *create(JSContext *cx, host_api::HttpBody body_handle);
};

class SimpleCache : public builtins::BuiltinImpl<SimpleCache> {
private:
public:
  static constexpr const char *class_name = "SimpleCache";
  static const int ctor_length = 0;
  enum Slots { Count };

  static const JSFunctionSpec static_methods[];
  static const JSPropertySpec static_properties[];
  static const JSFunctionSpec methods[];
  static const JSPropertySpec properties[];

  static bool delete_(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool purge(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool set(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool getOrSet(JSContext *cx, unsigned argc, JS::Value *vp);

  static bool getOrSetThenHandler(JSContext *cx, JS::HandleObject owner, JS::HandleValue extra,
                                  JS::CallArgs args);

  static bool constructor(JSContext *cx, unsigned argc, JS::Value *vp);
};

} // namespace fastly::cache_simple

#endif
