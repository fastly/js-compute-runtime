#ifndef JS_COMPUTE_RUNTIME_LOGGER_H
#define JS_COMPUTE_RUNTIME_LOGGER_H

#include "builtin.h"

namespace builtins {

class Logger : public BuiltinNoConstructor<Logger> {
private:
  static bool log(JSContext *cx, unsigned argc, JS::Value *vp);

public:
  static constexpr const char *class_name = "Logger";

  enum Slots { Endpoint, Count };

  static const JSFunctionSpec methods[];
  static const JSPropertySpec properties[];

  static JSObject *create(JSContext *cx, const char *name);
};

} // namespace builtins

#endif
