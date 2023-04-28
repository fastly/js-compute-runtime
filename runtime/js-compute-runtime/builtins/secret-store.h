#ifndef JS_COMPUTE_RUNTIME_SECRET_STORE_H
#define JS_COMPUTE_RUNTIME_SECRET_STORE_H

#include "builtin.h"
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

  static fastly_secret_handle_t secret_handle(JSObject *obj);
  static bool constructor(JSContext *cx, unsigned argc, JS::Value *vp);
  static JSObject *create(JSContext *cx, fastly_secret_handle_t handle);

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

  static fastly_secret_store_handle_t secret_store_handle(JSObject *obj);
  static bool constructor(JSContext *cx, unsigned argc, JS::Value *vp);

  static bool init_class(JSContext *cx, JS::HandleObject global);
};

} // namespace builtins

#endif
