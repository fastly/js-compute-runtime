#ifndef JS_COMPUTE_RUNTIME_CONFIG_STORE_H
#define JS_COMPUTE_RUNTIME_CONFIG_STORE_H

#include "builtin.h"

namespace builtins {

class ConfigStore : public BuiltinImpl<ConfigStore> {
private:
public:
  static constexpr const char *class_name = "ConfigStore";
  static const int ctor_length = 1;
  enum Slots { Handle, Count };

  static const JSFunctionSpec methods[];
  static const JSPropertySpec properties[];

  static bool get(JSContext *cx, unsigned argc, JS::Value *vp);

  static ConfigStoreHandle config_store_handle(JSObject *obj);
  static bool constructor(JSContext *cx, unsigned argc, JS::Value *vp);

  static bool init_class(JSContext *cx, JS::HandleObject global);
};

} // namespace builtins

#endif
