#ifndef JS_COMPUTE_RUNTIME_BUILTIN_FASTLY_H
#define JS_COMPUTE_RUNTIME_BUILTIN_FASTLY_H

#include "builtin.h"
#include "js-compute-builtins.h"

namespace builtins {

class Fastly : public BuiltinNoConstructor<Fastly> {
private:
  static bool log(JSContext *cx, unsigned argc, JS::Value *vp);

public:
  static constexpr const char *class_name = "Fastly";

  static bool debug_logging_enabled;

  static JS::PersistentRooted<JSObject *> env;

  static JS::PersistentRooted<JSObject *> baseURL;
  static JS::PersistentRooted<JSString *> defaultBackend;
  static bool allowDynamicBackends;

  static const JSPropertySpec properties[];

  static bool now(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool dump(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool enableDebugLogging(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool getGeolocationForIpAddress(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool getLogger(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool includeBytes(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool env_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool baseURL_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool baseURL_set(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool defaultBackend_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool defaultBackend_set(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool allowDynamicBackends_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool allowDynamicBackends_set(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool create(JSContext *cx, JS::HandleObject global, FastlyOptions options);
};

} // namespace builtins

#endif
