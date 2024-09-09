// TODO: remove these once the warnings are fixed
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-offsetof"
#pragma clang diagnostic ignored "-Wdeprecated-enum-enum-conversion"
#include "js/experimental/TypedData.h" // used in "js/Conversions.h"
#pragma clang diagnostic pop
#include "../../StarlingMonkey/builtins/web/url.h"
#include "./fetch/request-response.h"
#include "encode.h"
#include "fastly.h"
#include "js/Conversions.h"
#include "js/JSON.h"
#include "logger.h"
#include <arpa/inet.h>

using builtins::web::url::URL;
using builtins::web::url::URLSearchParams;
using fastly::fastly::Fastly;
using fastly::fetch::RequestOrResponse;
using fastly::fetch::Response;
using fastly::logger::Logger;

namespace {

bool DEBUG_LOGGING_ENABLED = false;

api::Engine *ENGINE;

static void oom_callback(JSContext *cx, void *data) {
  fprintf(stderr, "Critical Error: out of memory\n");
  fflush(stderr);
}

} // namespace

bool debug_logging_enabled() { return DEBUG_LOGGING_ENABLED; }

namespace fastly::fastly {

const JSErrorFormatString *FastlyGetErrorMessage(void *userRef, unsigned errorNumber) {
  if (errorNumber > 0 && errorNumber < JSErrNum_Limit) {
    return &fastly_ErrorFormatString[errorNumber];
  }

  return nullptr;
}

JS::PersistentRooted<JSObject *> Fastly::env;
JS::PersistentRooted<JSObject *> Fastly::baseURL;
JS::PersistentRooted<JSString *> Fastly::defaultBackend;
bool Fastly::allowDynamicBackends = false;
host_api::BackendConfig Fastly::defaultDynamicBackendConfig;

bool Fastly::dump(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS::CallArgs args = CallArgsFromVp(argc, vp);
  if (!args.requireAtLeast(cx, __func__, 1))
    return false;

  ENGINE->dump_value(args[0], stdout);

  args.rval().setUndefined();
  return true;
}

bool Fastly::enableDebugLogging(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS::CallArgs args = CallArgsFromVp(argc, vp);
  if (!args.requireAtLeast(cx, __func__, 1))
    return false;
  DEBUG_LOGGING_ENABLED = JS::ToBoolean(args[0]);
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

  JS::RootedString geo_info_str(cx, host_api::get_geo_info(cx, address_str));
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

  JS::RootedObject logger(cx, Logger::create(cx, args[0]));
  if (!logger) {
    return false;
  }

  args.rval().setObject(*logger);
  return true;
}

bool Fastly::includeBytes(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS::CallArgs args = CallArgsFromVp(argc, vp);
  INIT_ONLY("fastly.includeBytes");
  if (!args.requireAtLeast(cx, "fastly.includeBytes", 1))
    return false;

  auto path = core::encode(cx, args[0]);
  if (!path) {
    return false;
  }

  FILE *fp = fopen(path.begin(), "r");
  if (!fp) {
    JS_ReportErrorUTF8(cx, "Error opening file %s", path.begin());
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
    JS_ReportErrorUTF8(cx, "Failed to read contents of file %s", path.begin());
    return false;
  }

  args.rval().setObject(*typed_array);
  return true;
}

bool Fastly::createFanoutHandoff(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS::CallArgs args = CallArgsFromVp(argc, vp);
  REQUEST_HANDLER_ONLY("createFanoutHandoff");
  if (!args.requireAtLeast(cx, "createFanoutHandoff", 2)) {
    return false;
  }

  auto request_value = args.get(0);
  if (!Request::is_instance(request_value)) {
    JS_ReportErrorUTF8(cx, "createFanoutHandoff: request parameter must be an instance of Request");
    return false;
  }

  auto response_handle = host_api::HttpResp::make();
  if (auto *err = response_handle.to_err()) {
    HANDLE_ERROR(cx, *err);
    return false;
  }
  auto body_handle = host_api::HttpBody::make();
  if (auto *err = body_handle.to_err()) {
    HANDLE_ERROR(cx, *err);
    return false;
  }

  JS::RootedObject response_instance(
      cx, JS_NewObjectWithGivenProto(cx, &Response::class_, Response::proto_obj));
  if (!response_instance) {
    return false;
  }

  auto backend_value = args.get(1);
  auto backend_chars = core::encode(cx, backend_value);
  if (!backend_chars) {
    return false;
  }
  if (backend_chars.len == 0) {
    JS_ReportErrorUTF8(cx, "createFanoutHandoff: Backend parameter can not be an empty string");
    return false;
  }

  if (backend_chars.len > 254) {
    JS_ReportErrorUTF8(cx, "createFanoutHandoff: name can not be more than 254 characters");
    return false;
  }

  bool is_upstream = true;
  bool is_grip_upgrade = true;
  JS::RootedObject response(cx, Response::create(cx, response_instance, response_handle.unwrap(),
                                                 body_handle.unwrap(), is_upstream, is_grip_upgrade,
                                                 std::move(backend_chars.ptr)));
  if (!response) {
    return false;
  }

  RequestOrResponse::set_url(response, RequestOrResponse::url(&request_value.toObject()));
  args.rval().setObject(*response);

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

bool runtime_get_vcpu_time(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS::CallArgs args = CallArgsFromVp(argc, vp);
  auto res = host_api::Runtime::get_vcpu_ms();
  if (auto *err = res.to_err()) {
    HANDLE_ERROR(cx, *err);
    return false;
  }
  args.rval().setNumber(res.unwrap());
  return true;
}

bool runtime_purge_surrogate_key(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS::CallArgs args = CallArgsFromVp(argc, vp);
  fprintf(stderr, "WAT");
  if (!args.requireAtLeast(cx, "purgeSurrogateKey", 1)) {
    return false;
  }
  JS::RootedString surrogate_key(cx, JS::ToString(cx, args.get(0)));
  if (!surrogate_key) {
    return false;
  }
  auto surrogate_key_chars = core::encode(cx, surrogate_key);
  bool soft = false;
  auto soft_val = args.get(1);
  if (soft_val.isBoolean()) {
    soft = soft_val.toBoolean();
  }
  auto purge_res = host_api::Runtime::purge_surrogate_key(surrogate_key_chars, soft);
  if (auto *err = purge_res.to_err()) {
    HANDLE_ERROR(cx, *err);
    return false;
  }
  MOZ_ASSERT(!purge_res.unwrap().has_value());

  args.rval().setUndefined();
  return true;
}

bool Env::env_get(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS::CallArgs args = CallArgsFromVp(argc, vp);
  if (!args.requireAtLeast(cx, "fastly.env.get", 1))
    return false;

  auto var_name_chars = core::encode(cx, args[0]);
  if (!var_name_chars) {
    return false;
  }
  JS::RootedString env_var(cx, JS_NewStringCopyZ(cx, getenv(var_name_chars.begin())));
  if (!env_var)
    return false;

  args.rval().setString(env_var);
  return true;
}

const JSFunctionSpec Env::static_methods[] = {
    JS_FS_END,
};

const JSPropertySpec Env::static_properties[] = {
    JS_PS_END,
};

const JSFunctionSpec Env::methods[] = {JS_FN("get", env_get, 1, JSPROP_ENUMERATE), JS_FS_END};

const JSPropertySpec Env::properties[] = {JS_PS_END};

JSObject *Env::create(JSContext *cx) {
  JS::RootedObject env(cx, JS_NewPlainObject(cx));
  if (!env || !JS_DefineFunctions(cx, env, methods))
    return nullptr;
  return env;
}

bool Fastly::version_get(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS::CallArgs args = CallArgsFromVp(argc, vp);
  JS::RootedString version_str(cx, JS_NewStringCopyN(cx, RUNTIME_VERSION, strlen(RUNTIME_VERSION)));
  args.rval().setString(version_str);
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
  JS::HandleValue set_value = args.get(0);
  if (set_value.isObject()) {
    allowDynamicBackends = true;
    RootedObject options_value(cx, &set_value.toObject());

    RootedValue connect_timeout(cx);
    if (!JS_GetProperty(cx, options_value, "connectTimeout", &connect_timeout)) {
      return false;
    }
    RootedValue between_bytes_timeout(cx);
    if (!JS_GetProperty(cx, options_value, "betweenBytesTimeout", &between_bytes_timeout)) {
      return false;
    }
    RootedValue first_byte_timeout(cx);
    if (!JS_GetProperty(cx, options_value, "firstByteTimeout", &first_byte_timeout)) {
      return false;
    }

    if (connect_timeout.isNumber()) {
      defaultDynamicBackendConfig.connect_timeout = connect_timeout.toNumber();
    }
    if (between_bytes_timeout.isNumber()) {
      defaultDynamicBackendConfig.between_bytes_timeout = between_bytes_timeout.toNumber();
    }
    if (first_byte_timeout.isNumber()) {
      defaultDynamicBackendConfig.first_byte_timeout = first_byte_timeout.toNumber();
    }
  } else {
    allowDynamicBackends = JS::ToBoolean(set_value);
  }
  args.rval().setUndefined();
  return true;
}

const JSPropertySpec Fastly::properties[] = {
    JS_PSG("env", env_get, JSPROP_ENUMERATE),
    JS_PSGS("baseURL", baseURL_get, baseURL_set, JSPROP_ENUMERATE),
    JS_PSGS("defaultBackend", defaultBackend_get, defaultBackend_set, JSPROP_ENUMERATE),
    JS_PSGS("allowDynamicBackends", allowDynamicBackends_get, allowDynamicBackends_set,
            JSPROP_ENUMERATE),
    JS_PSG("sdkVersion", version_get, JSPROP_ENUMERATE),
    JS_PS_END};

bool install(api::Engine *engine) {
  ENGINE = engine;

  bool ENABLE_EXPERIMENTAL_HIGH_RESOLUTION_TIME_METHODS =
      std::string(std::getenv("ENABLE_EXPERIMENTAL_HIGH_RESOLUTION_TIME_METHODS")) == "1";

  engine->enable_module_mode(std::string(std::getenv("ENABLE_MODULE_MODE")) == "1");

  JS::SetOutOfMemoryCallback(engine->cx(), oom_callback, nullptr);

  JS::RootedObject fastly(engine->cx(), JS_NewPlainObject(engine->cx()));
  if (!fastly) {
    return false;
  }

  Fastly::env.init(engine->cx(), Env::create(engine->cx()));
  if (!Fastly::env) {
    return false;
  }

  Fastly::baseURL.init(engine->cx());
  Fastly::defaultBackend.init(engine->cx());

  if (!JS_DefineProperty(engine->cx(), engine->global(), "fastly", fastly, 0)) {
    return false;
  }

  JSFunctionSpec nowfn = JS_FN("now", Fastly::now, 0, JSPROP_ENUMERATE);
  JSFunctionSpec end = JS_FS_END;

  const JSFunctionSpec methods[] = {
      JS_FN("dump", Fastly::dump, 1, 0),
      JS_FN("enableDebugLogging", Fastly::enableDebugLogging, 1, JSPROP_ENUMERATE),
      JS_FN("getGeolocationForIpAddress", Fastly::getGeolocationForIpAddress, 1, JSPROP_ENUMERATE),
      JS_FN("getLogger", Fastly::getLogger, 1, JSPROP_ENUMERATE),
      JS_FN("includeBytes", Fastly::includeBytes, 1, JSPROP_ENUMERATE),
      JS_FN("createFanoutHandoff", Fastly::createFanoutHandoff, 2, JSPROP_ENUMERATE),
      ENABLE_EXPERIMENTAL_HIGH_RESOLUTION_TIME_METHODS ? nowfn : end,
      end};

  if (!JS_DefineFunctions(engine->cx(), fastly, methods) ||
      !JS_DefineProperties(engine->cx(), fastly, Fastly::properties)) {
    return false;
  }

  // fastly:env
  RootedValue env_get(engine->cx());
  if (!JS_GetProperty(engine->cx(), Fastly::env, "get", &env_get)) {
    return false;
  }
  RootedObject env_builtin(engine->cx(), JS_NewObject(engine->cx(), nullptr));
  if (!JS_SetProperty(engine->cx(), env_builtin, "env", env_get)) {
    return false;
  }
  RootedValue env_builtin_val(engine->cx(), JS::ObjectValue(*env_builtin));
  if (!engine->define_builtin_module("fastly:env", env_builtin_val)) {
    return false;
  }

  // fastly:runtime
  RootedObject runtime_builtin(engine->cx(), JS_NewObject(engine->cx(), nullptr));
  auto runtime_purge_surrogate_key_fn =
      JS_NewFunction(engine->cx(), &runtime_purge_surrogate_key, 1, 0, "purgeSurrogateKey");
  RootedObject runtime_purge_surrogate_key_obj(
      engine->cx(), JS_GetFunctionObject(runtime_purge_surrogate_key_fn));
  RootedValue runtime_purge_surrogate_key_val(engine->cx(),
                                              ObjectValue(*runtime_purge_surrogate_key_obj));

  if (!JS_SetProperty(engine->cx(), runtime_builtin, "purgeSurrogateKey",
                      runtime_purge_surrogate_key_val)) {
    return false;
  }
  if (!JS_SetProperty(engine->cx(), fastly, "purgeSurrogateKey", runtime_purge_surrogate_key_val)) {
    return false;
  }
  auto runtime_vcpu_time_get =
      JS_NewFunction(engine->cx(), &runtime_get_vcpu_time, 0, 0, "vCpuTime");
  RootedObject runtime_vcpu_time_get_obj(engine->cx(), JS_GetFunctionObject(runtime_vcpu_time_get));
  RootedValue runtime_vcpu_time_get_val(engine->cx(), ObjectValue(*runtime_vcpu_time_get_obj));
  if (!JS_SetProperty(engine->cx(), runtime_builtin, "vCpuTime", runtime_vcpu_time_get_val)) {
    return false;
  }
  if (!JS_SetProperty(engine->cx(), fastly, "vCpuTime", runtime_vcpu_time_get_val)) {
    return false;
  }
  RootedValue runtime_builtin_val(engine->cx(), JS::ObjectValue(*runtime_builtin));
  if (!engine->define_builtin_module("fastly:runtime", runtime_builtin_val)) {
    return false;
  }

  // fastly:experimental
  RootedObject experimental(engine->cx(), JS_NewObject(engine->cx(), nullptr));
  RootedValue experimental_val(engine->cx(), JS::ObjectValue(*experimental));
  RootedValue include_bytes_val(engine->cx());
  if (!JS_GetProperty(engine->cx(), fastly, "includeBytes", &include_bytes_val)) {
    return false;
  }
  if (!JS_SetProperty(engine->cx(), experimental, "includeBytes", include_bytes_val)) {
    return false;
  }
  auto set_default_backend =
      JS_NewFunction(engine->cx(), &Fastly::defaultBackend_set, 1, 0, "setDefaultBackend");
  RootedObject set_default_backend_obj(engine->cx(), JS_GetFunctionObject(set_default_backend));
  RootedValue set_default_backend_val(engine->cx(), ObjectValue(*set_default_backend_obj));
  if (!JS_SetProperty(engine->cx(), experimental, "setDefaultBackend", set_default_backend_val)) {
    return false;
  }
  auto allow_dynamic_backends =
      JS_NewFunction(engine->cx(), &Fastly::allowDynamicBackends_set, 1, 0, "allowDynamicBackends");
  RootedObject allow_dynamic_backends_obj(engine->cx(),
                                          JS_GetFunctionObject(allow_dynamic_backends));
  RootedValue allow_dynamic_backends_val(engine->cx(), ObjectValue(*allow_dynamic_backends_obj));
  if (!JS_SetProperty(engine->cx(), experimental, "allowDynamicBackends",
                      allow_dynamic_backends_val)) {
    return false;
  }
  RootedString version_str(
      engine->cx(), JS_NewStringCopyN(engine->cx(), RUNTIME_VERSION, strlen(RUNTIME_VERSION)));
  RootedValue version_str_val(engine->cx(), StringValue(version_str));
  if (!JS_SetProperty(engine->cx(), experimental, "sdkVersion", version_str_val)) {
    return false;
  }
  if (!engine->define_builtin_module("fastly:experimental", experimental_val)) {
    return false;
  }

  // fastly:geo
  RootedValue get_geolocation_for_ip_address_val(engine->cx());
  if (!JS_GetProperty(engine->cx(), fastly, "getGeolocationForIpAddress",
                      &get_geolocation_for_ip_address_val)) {
    return false;
  }
  RootedObject geo_builtin(engine->cx(), JS_NewObject(engine->cx(), nullptr));
  RootedValue geo_builtin_val(engine->cx(), JS::ObjectValue(*geo_builtin));
  if (!JS_SetProperty(engine->cx(), geo_builtin, "getGeolocationForIpAddress",
                      get_geolocation_for_ip_address_val)) {
    return false;
  }
  if (!engine->define_builtin_module("fastly:geolocation", geo_builtin_val)) {
    return false;
  }
  // fastly:fanout
  RootedObject fanout(engine->cx(), JS_NewObject(engine->cx(), nullptr));
  RootedValue fanout_val(engine->cx(), JS::ObjectValue(*fanout));
  RootedValue create_fanout_handoff_val(engine->cx());
  if (!JS_GetProperty(engine->cx(), fastly, "createFanoutHandoff", &create_fanout_handoff_val)) {
    return false;
  }
  if (!JS_SetProperty(engine->cx(), fanout, "createFanoutHandoff", create_fanout_handoff_val)) {
    return false;
  }
  if (!engine->define_builtin_module("fastly:fanout", fanout_val)) {
    return false;
  }

  return true;
}

// We currently support five types of body inputs:
// - byte sequence
// - buffer source
// - USV strings
// - URLSearchParams
// After the other other options are checked explicitly, all other inputs are
// encoded to a UTF8 string to be treated as a USV string.
// TODO: Support the other possible inputs to Body.
JS::Result<std::tuple<JS::UniqueChars, size_t>> convertBodyInit(JSContext *cx,
                                                                JS::HandleValue bodyInit) {
  JS::RootedObject bodyObj(cx, bodyInit.isObject() ? &bodyInit.toObject() : nullptr);
  JS::UniqueChars buf;
  size_t length;

  if (bodyObj && JS_IsArrayBufferViewObject(bodyObj)) {
    // `maybeNoGC` needs to be populated for the lifetime of `buf` because
    // short typed arrays have inline data which can move on GC, so assert
    // that no GC happens. (Which it doesn't, because we're not allocating
    // before `buf` goes out of scope.)
    JS::AutoCheckCannotGC noGC;
    bool is_shared;
    length = JS_GetArrayBufferViewByteLength(bodyObj);
    buf = JS::UniqueChars(
        reinterpret_cast<char *>(JS_GetArrayBufferViewData(bodyObj, &is_shared, noGC)));
    MOZ_ASSERT(!is_shared);
    return JS::Result<std::tuple<JS::UniqueChars, size_t>>(std::make_tuple(std::move(buf), length));
  } else if (bodyObj && JS::IsArrayBufferObject(bodyObj)) {
    bool is_shared;
    uint8_t *bytes;
    JS::GetArrayBufferLengthAndData(bodyObj, &length, &is_shared, &bytes);
    MOZ_ASSERT(!is_shared);
    buf.reset(reinterpret_cast<char *>(bytes));
    return JS::Result<std::tuple<JS::UniqueChars, size_t>>(std::make_tuple(std::move(buf), length));
  } else if (bodyObj && URLSearchParams::is_instance(bodyObj)) {
    jsurl::SpecSlice slice = URLSearchParams::serialize(cx, bodyObj);
    buf = JS::UniqueChars(reinterpret_cast<char *>(const_cast<uint8_t *>(slice.data)));
    length = slice.len;
    return JS::Result<std::tuple<JS::UniqueChars, size_t>>(std::make_tuple(std::move(buf), length));
  } else {
    // Convert into a String following https://tc39.es/ecma262/#sec-tostring
    auto str = core::encode(cx, bodyInit);
    buf = std::move(str.ptr);
    length = str.len;
    if (!buf) {
      return JS::Result<std::tuple<JS::UniqueChars, size_t>>(JS::Error());
    }
    return JS::Result<std::tuple<JS::UniqueChars, size_t>>(std::make_tuple(std::move(buf), length));
  }
  abort();
}

} // namespace fastly::fastly
