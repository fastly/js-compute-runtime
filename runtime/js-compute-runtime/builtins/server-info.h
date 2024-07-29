#ifndef BUILTIN_SERVER_INFO_H
#define BUILTIN_SERVER_INFO_H

#include "builtin.h"

namespace builtins {

class ServerInfo final : public BuiltinNoConstructor<ServerInfo> {
  static bool address_get(JSContext *cx, unsigned argc, JS::Value *vp);

public:
  static constexpr const char *class_name = "ServerInfo";

  enum class Slots {
    Address,
    Count,
  };
  static const JSFunctionSpec static_methods[];
  static const JSPropertySpec static_properties[];
  static const JSFunctionSpec methods[];
  static const JSPropertySpec properties[];

  static JSObject *create(JSContext *cx);
};

} // namespace builtins

#endif
