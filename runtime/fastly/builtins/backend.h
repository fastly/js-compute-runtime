#ifndef FASTLY_BACKEND_H
#define FASTLY_BACKEND_H

#include "builtin.h"
#include "extension-api.h"

namespace fastly::backend {

class Backend : public builtins::FinalizableBuiltinImpl<Backend> {
private:
public:
  static constexpr const char *class_name = "Backend";
  static const int ctor_length = 1;
  enum Slots { Name, Count };

  static const JSFunctionSpec static_methods[];
  static const JSPropertySpec static_properties[];

  static const JSFunctionSpec methods[];
  static const JSPropertySpec properties[];

  inline static JS::PersistentRootedObject backends;

  static JSString *name(JSContext *cx, JSObject *self);
  static JSObject *create(JSContext *cx, JS::HandleObject request);
  static std::optional<host_api::HostString> set_name(JSContext *cx, JSObject *backend,
                                                      JS::HandleValue name_val);

  // static methods
  static bool exists(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool from_name(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool health(JSContext *cx, unsigned argc, JS::Value *vp);

  // prototype methods
  static bool to_name(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool to_string(JSContext *cx, unsigned argc, JS::Value *vp);

  static bool constructor(JSContext *cx, unsigned argc, JS::Value *vp);
  static void finalize(JS::GCContext *gcx, JSObject *obj);
};

bool set_default_backend_config(JSContext *cx, unsigned argc, JS::Value *vp);

} // namespace fastly::backend

#endif
