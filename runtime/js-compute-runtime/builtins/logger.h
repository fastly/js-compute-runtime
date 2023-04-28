#ifndef JS_COMPUTE_RUNTIME_LOGGER_H
#define JS_COMPUTE_RUNTIME_LOGGER_H

#include "builtin.h"
#include "js-compute-builtins.h"

namespace builtins {

class Logger : public BuiltinImpl<Logger> {
private:
  static bool log(JSContext *cx, unsigned argc, JS::Value *vp);

public:
  static constexpr const char *class_name = "Logger";
  static const int ctor_length = 1;

  enum Slots { Endpoint, Count };
  static const JSFunctionSpec static_methods[];
  static const JSPropertySpec static_properties[];
  static const JSFunctionSpec methods[];
  static const JSPropertySpec properties[];

  static JSObject *create(JSContext *cx, const char *name);
  static bool constructor(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool init_class(JSContext *cx, JS::HandleObject global);
};

} // namespace builtins

#endif
