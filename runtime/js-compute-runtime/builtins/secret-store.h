#ifndef JS_COMPUTE_RUNTIME_SECRET_STORE_H
#define JS_COMPUTE_RUNTIME_SECRET_STORE_H

#include "builtin.h"
#include "host_interface/host_api.h"
#include "js-compute-builtins.h"

namespace builtins {

class SecretStoreEntry : public BuiltinImpl<SecretStoreEntry> {
private:
public:
  static constexpr const char *class_name = "SecretStoreEntry";
  static const int ctor_length = 0;
  enum Slots { Handle, Count };
  static const JSFunctionSpec static_methods[];
  static const JSPropertySpec static_properties[];
  static const JSFunctionSpec methods[];
  static const JSPropertySpec properties[];

  static bool plaintext(JSContext *cx, unsigned argc, JS::Value *vp);

  static host_api::Secret secret_handle(JSObject *obj);
  static bool constructor(JSContext *cx, unsigned argc, JS::Value *vp);
  static JSObject *create(JSContext *cx, host_api::Secret handle);

  static bool init_class(JSContext *cx, JS::HandleObject global);
};

class SecretStore : public BuiltinImpl<SecretStore> {
private:
public:
  static constexpr const char *class_name = "SecretStore";
  static const int ctor_length = 1;
  enum Slots { Handle, Count };
  static const JSFunctionSpec static_methods[];
  static const JSPropertySpec static_properties[];
  static const JSFunctionSpec methods[];
  static const JSPropertySpec properties[];

  static bool get(JSContext *cx, unsigned argc, JS::Value *vp);

  static host_api::SecretStore secret_store_handle(JSObject *obj);
  static bool constructor(JSContext *cx, unsigned argc, JS::Value *vp);

  static bool init_class(JSContext *cx, JS::HandleObject global);
};

} // namespace builtins

#endif
