#ifndef JS_COMPUTE_RUNTIME_OBJECT_STORE_H
#define JS_COMPUTE_RUNTIME_OBJECT_STORE_H

#include "builtin.h"
#include "builtins/request-response.h"
#include "js-compute-builtins.h"

namespace builtins {

class ObjectStoreEntry final : public BuiltinImpl<ObjectStoreEntry> {
  template <RequestOrResponse::BodyReadResult result_type>
  static bool bodyAll(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool body_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool bodyUsed_get(JSContext *cx, unsigned argc, JS::Value *vp);

public:
  static constexpr const char *class_name = "ObjectStoreEntry";

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

class ObjectStore final : public BuiltinImpl<ObjectStore> {
  static bool get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool put(JSContext *cx, unsigned argc, JS::Value *vp);

public:
  static constexpr const char *class_name = "ObjectStore";
  enum class Slots {
    ObjectStore,
    Count,
  };
  static const JSFunctionSpec static_methods[];
  static const JSPropertySpec static_properties[];
  static const JSFunctionSpec methods[];
  static const JSPropertySpec properties[];

  static const unsigned ctor_length = 1;

  static bool init_class(JSContext *cx, JS::HandleObject global);
  static bool constructor(JSContext *cx, unsigned argc, JS::Value *vp);
};

} // namespace builtins

#endif
