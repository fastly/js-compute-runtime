#ifndef JS_COMPUTE_RUNTIME_BUILTIN_ENV_H
#define JS_COMPUTE_RUNTIME_BUILTIN_ENV_H

#include "builtin.h"

namespace builtins {

class Env : public BuiltinNoConstructor<Env> {
private:
  static bool env_get(JSContext *cx, unsigned argc, JS::Value *vp);

public:
  static constexpr const char *class_name = "Env";

  static const JSFunctionSpec methods[];
  static const JSPropertySpec properties[];

  static JSObject *create(JSContext *cx);
};

} // namespace builtins

#endif
