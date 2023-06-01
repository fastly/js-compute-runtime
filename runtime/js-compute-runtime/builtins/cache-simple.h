#ifndef JS_COMPUTE_RUNTIME_CACHE_SIMPLE_H
#define JS_COMPUTE_RUNTIME_CACHE_SIMPLE_H

#include "builtin.h"
#include "js-compute-builtins.h"
#include "request-response.h"

namespace builtins {

class SimpleCacheEntry final : public BuiltinImpl<SimpleCacheEntry> {
  template <RequestOrResponse::BodyReadResult result_type>
  static bool bodyAll(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool body_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool bodyUsed_get(JSContext *cx, unsigned argc, JS::Value *vp);

public:
  static constexpr const char *class_name = "SimpleCacheEntry";

  using Slots = RequestOrResponse::Slots;
  static const JSFunctionSpec static_methods[];
  static const JSPropertySpec static_properties[];
  static const JSFunctionSpec methods[];
  static const JSPropertySpec properties[];

  static const unsigned ctor_length = 0;

  static bool init_class(JSContext *cx, JS::HandleObject global);
  static bool constructor(JSContext *cx, unsigned argc, JS::Value *vp);
  static JSObject *create(JSContext *cx, fastly_body_handle_t body_handle);
};

class SimpleCache : public BuiltinImpl<SimpleCache> {
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
  static bool set(JSContext *cx, unsigned argc, JS::Value *vp);

  static bool constructor(JSContext *cx, unsigned argc, JS::Value *vp);

  static bool init_class(JSContext *cx, JS::HandleObject global);
};

} // namespace builtins

#endif
