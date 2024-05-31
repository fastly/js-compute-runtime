#ifndef FASTLY_SECRET_STORE_H
#define FASTLY_SECRET_STORE_H

#include "../host-api/host_api_fastly.h"
#include "builtin.h"
#include "extension-api.h"

namespace fastly::secret_store {

class SecretStoreEntry : public builtins::BuiltinImpl<SecretStoreEntry> {
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
  static bool rawbytes(JSContext *cx, unsigned argc, JS::Value *vp);

  static host_api::Secret secret_handle(JSObject *obj);
  static bool constructor(JSContext *cx, unsigned argc, JS::Value *vp);
  static JSObject *create(JSContext *cx, host_api::Secret handle);
};

class SecretStore : public builtins::BuiltinImpl<SecretStore> {
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
  static bool from_bytes(JSContext *cx, unsigned argc, JS::Value *vp);

  static host_api::SecretStore secret_store_handle(JSObject *obj);
  static bool constructor(JSContext *cx, unsigned argc, JS::Value *vp);
};

} // namespace fastly::secret_store

#endif
