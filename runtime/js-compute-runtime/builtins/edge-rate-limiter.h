#ifndef JS_COMPUTE_RUNTIME_EDGE_RATE_LIMITER_H
#define JS_COMPUTE_RUNTIME_EDGE_RATE_LIMITER_H

#include "builtin.h"
#include "js-compute-builtins.h"

namespace builtins {

class PenaltyBox final : public BuiltinImpl<PenaltyBox> {
  static bool add(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool has(JSContext *cx, unsigned argc, JS::Value *vp);

public:
  static constexpr const char *class_name = "PenaltyBox";
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
};

} // namespace builtins

#endif
