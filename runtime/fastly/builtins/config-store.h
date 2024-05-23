#ifndef FASTLY_CONFIG_STORE_H
#define FASTLY_CONFIG_STORE_H

#include "../host-api/host_api_fastly.h"
#include "builtin.h"
#include "extension-api.h"

namespace fastly::config_store {

class ConfigStore : public builtins::BuiltinImpl<ConfigStore> {
private:
public:
  static constexpr const char *class_name = "ConfigStore";
  static const int ctor_length = 1;
  enum Slots { Handle, Count };

  static const JSFunctionSpec static_methods[];
  static const JSPropertySpec static_properties[];
  static const JSFunctionSpec methods[];
  static const JSPropertySpec properties[];

  static bool get(JSContext *cx, unsigned argc, JS::Value *vp);

  static host_api::Dict config_store_handle(JSObject *obj);
  static bool constructor(JSContext *cx, unsigned argc, JS::Value *vp);

  static bool init_class(JSContext *cx, JS::HandleObject global);
};

} // namespace fastly::config_store

#endif
