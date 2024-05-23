#ifndef FASTLY_LOGGER_H
#define FASTLY_LOGGER_H

#include "builtin.h"
#include "extension-api.h"

namespace fastly::logger {

class Logger : public builtins::BuiltinImpl<Logger> {
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
};

} // namespace fastly::logger

#endif
