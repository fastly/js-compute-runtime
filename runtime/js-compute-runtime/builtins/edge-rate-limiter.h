#ifndef JS_COMPUTE_RUNTIME_EDGE_RATE_LIMITER_H
#define JS_COMPUTE_RUNTIME_EDGE_RATE_LIMITER_H

#include "builtin.h"
#include "js-compute-builtins.h"

namespace builtins {

class RateCounter final : public BuiltinImpl<RateCounter> {
  static bool increment(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool lookupRate(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool lookupCount(JSContext *cx, unsigned argc, JS::Value *vp);

public:
  static constexpr const char *class_name = "RateCounter";
  enum Slots {
    Name,
    Count
  };
  static const JSFunctionSpec static_methods[];
  static const JSPropertySpec static_properties[];
  static const JSFunctionSpec methods[];
  static const JSPropertySpec properties[];

  static const unsigned ctor_length = 0;

  static bool init_class(JSContext *cx, JS::HandleObject global);
  static bool constructor(JSContext *cx, unsigned argc, JS::Value *vp);

  static JSString* get_name(JSObject* self);
};

} // namespace builtins

#endif
