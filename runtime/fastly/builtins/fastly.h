#ifndef FASTLY_FASTLY_H
#define FASTLY_FASTLY_H

#include "builtin.h"
#include "extension-api.h"
#include "host_api.h"
#include "../host-api/host_api_fastly.h"
#include "fastly.h"
#include "./fetch/request-response.h"
#include "../../StarlingMonkey/builtins/web/url.h"

using namespace builtins;

namespace fastly::fastly {

class Env : public BuiltinNoConstructor<Env> {
private:
  static bool env_get(JSContext *cx, unsigned argc, JS::Value *vp);

public:
  static constexpr const char *class_name = "Env";

  static const JSFunctionSpec static_methods[];
  static const JSPropertySpec static_properties[];
  static const JSFunctionSpec methods[];
  static const JSPropertySpec properties[];

  static JSObject *create(JSContext *cx);
};

const JSErrorFormatString *FastlyGetErrorMessage(void *userRef, unsigned errorNumber);

class Fastly : public BuiltinNoConstructor<Fastly> {
private:
  // static bool log(JSContext *cx, unsigned argc, JS::Value *vp);

public:
  static constexpr const char *class_name = "Fastly";

  static JS::PersistentRooted<JSObject *> env;
  static JS::PersistentRooted<JSObject *> baseURL;
  static JS::PersistentRooted<JSString *> defaultBackend;
  static bool allowDynamicBackends;

  static const JSPropertySpec properties[];

  // static bool createFanoutHandoff(JSContext *cx, unsigned argc, JS::Value *vp);
  // static bool now(JSContext *cx, unsigned argc, JS::Value *vp);
  // static bool dump(JSContext *cx, unsigned argc, JS::Value *vp);
  // static bool enableDebugLogging(JSContext *cx, unsigned argc, JS::Value *vp);
  // static bool getGeolocationForIpAddress(JSContext *cx, unsigned argc, JS::Value *vp);
  // static bool getLogger(JSContext *cx, unsigned argc, JS::Value *vp);
  // static bool includeBytes(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool env_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool baseURL_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool baseURL_set(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool defaultBackend_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool defaultBackend_set(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool allowDynamicBackends_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool allowDynamicBackends_set(JSContext *cx, unsigned argc, JS::Value *vp);
};

} // namespace fastly::fastly

#endif
