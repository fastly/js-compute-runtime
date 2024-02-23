#ifndef JS_COMPUTE_RUNTIME_BODY_H
#define JS_COMPUTE_RUNTIME_BODY_H

#include "builtin.h"
#include "builtins/request-response.h"
#include "js-compute-builtins.h"

namespace builtins {

class FastlyBody final : public BuiltinImpl<FastlyBody> {
  static bool concat(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool read(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool append(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool prepend(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool close(JSContext *cx, unsigned argc, JS::Value *vp);

public:
  static constexpr const char *class_name = "FastlyBody";
  enum class Slots {
    Body,
    Count,
  };
  static const JSFunctionSpec static_methods[];
  static const JSPropertySpec static_properties[];
  static const JSFunctionSpec methods[];
  static const JSPropertySpec properties[];

  static const unsigned ctor_length = 0;

  static JSObject *create(JSContext *cx, uint32_t handle);
  static bool init_class(JSContext *cx, JS::HandleObject global);
  static bool constructor(JSContext *cx, unsigned argc, JS::Value *vp);
};

} // namespace builtins

#endif
