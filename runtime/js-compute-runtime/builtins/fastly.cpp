// TODO: remove these once the warnings are fixed
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-offsetof"
#pragma clang diagnostic ignored "-Wdeprecated-enum-enum-conversion"
#include "js/experimental/TypedData.h" // used in "js/Conversions.h"
#pragma clang diagnostic pop

#include "js/Conversions.h"
#include "js/JSON.h"

#include "builtin.h"
#include "builtins/env.h"
#include "builtins/fastly.h"
#include "builtins/logger.h"
#include "builtins/shared/url.h"
#include "core/geo_ip.h"
#include "host_interface/host_call.h"

namespace builtins {

bool Fastly::debug_logging_enabled = false;

JS::PersistentRooted<JSObject *> Fastly::env;

JS::PersistentRooted<JSObject *> Fastly::baseURL;
JS::PersistentRooted<JSString *> Fastly::defaultBackend;
bool Fastly::allowDynamicBackends = false;

bool Fastly::dump(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS::CallArgs args = CallArgsFromVp(argc, vp);
  if (!args.requireAtLeast(cx, __func__, 1))
    return false;

  dump_value(cx, args[0], stdout);

  args.rval().setUndefined();
  return true;
}

bool Fastly::enableDebugLogging(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS::CallArgs args = CallArgsFromVp(argc, vp);
  if (!args.requireAtLeast(cx, __func__, 1))
    return false;

  debug_logging_enabled = JS::ToBoolean(args[0]);

  args.rval().setUndefined();
  return true;
}

bool Fastly::getGeolocationForIpAddress(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS::CallArgs args = CallArgsFromVp(argc, vp);
  REQUEST_HANDLER_ONLY("fastly.getGeolocationForIpAddress");
  if (!args.requireAtLeast(cx, "fastly.getGeolocationForIpAddress", 1))
    return false;

  JS::RootedString address_str(cx, JS::ToString(cx, args[0]));
  if (!address_str)
    return false;

  JS::RootedString geo_info_str(cx, get_geo_info(cx, address_str));
  if (!geo_info_str)
    return false;

  return JS_ParseJSON(cx, geo_info_str, args.rval());
}

// TODO(performance): consider allowing logger creation during initialization, but then throw
// when trying to log.
// https://github.com/fastly/js-compute-runtime/issues/225
bool Fastly::getLogger(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS::CallArgs args = CallArgsFromVp(argc, vp);
  REQUEST_HANDLER_ONLY("fastly.getLogger");
  JS::RootedObject self(cx, &args.thisv().toObject());
  if (!args.requireAtLeast(cx, "fastly.getLogger", 1))
    return false;

  size_t name_len;
  JS::UniqueChars name = encode(cx, args[0], &name_len);
  if (!name)
    return false;

  JS::RootedObject logger(cx, builtins::Logger::create(cx, name.get()));
  if (!logger)
    return false;

  args.rval().setObject(*logger);
  return true;
}

bool Fastly::includeBytes(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS::CallArgs args = CallArgsFromVp(argc, vp);
  INIT_ONLY("fastly.includeBytes");
  JS::RootedObject self(cx, &args.thisv().toObject());
  if (!args.requireAtLeast(cx, "fastly.includeBytes", 1))
    return false;

  size_t path_len;
  JS::UniqueChars path = encode(cx, args[0], &path_len);
  if (!path)
    return false;

  FILE *fp = fopen(path.get(), "r");
  if (!fp) {
    JS_ReportErrorUTF8(cx, "Error opening file %s", path.get());
    return false;
  }

  fseek(fp, 0L, SEEK_END);
  size_t size = ftell(fp);
  rewind(fp);
  JS::RootedObject typed_array(cx, JS_NewUint8Array(cx, size));
  if (!typed_array)
    return false;

  size_t read_bytes;
  {
    JS::AutoCheckCannotGC noGC(cx);
    bool is_shared;
    void *buffer = JS_GetArrayBufferViewData(typed_array, &is_shared, noGC);
    read_bytes = fread(buffer, 1, size, fp);
  }

  if (read_bytes != size) {
    JS_ReportErrorUTF8(cx, "Failed to read contents of file %s", path.get());
    return false;
  }

  args.rval().setObject(*typed_array);
  return true;
}

bool Fastly::now(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS::CallArgs args = CallArgsFromVp(argc, vp);
  args.rval().setNumber(JS_Now());
  return true;
}

bool Fastly::env_get(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS::CallArgs args = CallArgsFromVp(argc, vp);
  args.rval().setObject(*env);
  return true;
}

bool Fastly::baseURL_get(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS::CallArgs args = CallArgsFromVp(argc, vp);
  args.rval().setObjectOrNull(baseURL);
  return true;
}

bool Fastly::baseURL_set(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS::CallArgs args = CallArgsFromVp(argc, vp);
  if (args.get(0).isNullOrUndefined()) {
    baseURL.set(nullptr);
  } else if (!URL::is_instance(args.get(0))) {
    JS_ReportErrorUTF8(cx, "Invalid value assigned to fastly.baseURL, must be an instance of "
                           "URL, null, or undefined");
    return false;
  }

  baseURL.set(&args.get(0).toObject());

  args.rval().setObjectOrNull(baseURL);
  return true;
}

bool Fastly::defaultBackend_get(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS::CallArgs args = CallArgsFromVp(argc, vp);
  args.rval().setString(defaultBackend);
  return true;
}

bool Fastly::defaultBackend_set(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS::CallArgs args = CallArgsFromVp(argc, vp);
  JS::RootedString backend(cx, JS::ToString(cx, args.get(0)));
  if (!backend)
    return false;

  defaultBackend = backend;
  args.rval().setUndefined();
  return true;
}

bool Fastly::allowDynamicBackends_get(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS::CallArgs args = CallArgsFromVp(argc, vp);
  args.rval().setBoolean(allowDynamicBackends);
  return true;
}

bool Fastly::allowDynamicBackends_set(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS::CallArgs args = CallArgsFromVp(argc, vp);
  allowDynamicBackends = JS::ToBoolean(args.get(0));
  args.rval().setUndefined();
  return true;
}

const JSPropertySpec Fastly::properties[] = {
    JS_PSG("env", env_get, JSPROP_ENUMERATE),
    JS_PSGS("baseURL", baseURL_get, baseURL_set, JSPROP_ENUMERATE),
    JS_PSGS("defaultBackend", defaultBackend_get, defaultBackend_set, JSPROP_ENUMERATE),
    JS_PSGS("allowDynamicBackends", allowDynamicBackends_get, allowDynamicBackends_set,
            JSPROP_ENUMERATE),
    JS_PS_END};

bool Fastly::create(JSContext *cx, JS::HandleObject global, FastlyOptions options) {
  JS::RootedObject fastly(cx, JS_NewPlainObject(cx));
  if (!fastly) {
    return false;
  }

  env.init(cx, Env::create(cx));
  if (!env) {
    return false;
  }
  baseURL.init(cx);
  defaultBackend.init(cx);

  if (!JS_DefineProperty(cx, global, "fastly", fastly, 0)) {
    return false;
  }

  JSFunctionSpec nowfn = JS_FN("now", now, 0, JSPROP_ENUMERATE);
  JSFunctionSpec end = JS_FS_END;

  const JSFunctionSpec methods[] = {
      JS_FN("dump", dump, 1, 0),
      JS_FN("enableDebugLogging", enableDebugLogging, 1, JSPROP_ENUMERATE),
      JS_FN("getGeolocationForIpAddress", getGeolocationForIpAddress, 1, JSPROP_ENUMERATE),
      JS_FN("getLogger", getLogger, 1, JSPROP_ENUMERATE),
      JS_FN("includeBytes", includeBytes, 1, JSPROP_ENUMERATE),
      options.getExperimentalHighResolutionTimeMethodsEnabled() ? nowfn : end,
      end};

  return JS_DefineFunctions(cx, fastly, methods) && JS_DefineProperties(cx, fastly, properties);
}

} // namespace builtins
