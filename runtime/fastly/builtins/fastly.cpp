// TODO: remove these once the warnings are fixed
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-offsetof"
#pragma clang diagnostic ignored "-Wdeprecated-enum-enum-conversion"
#include "js/experimental/TypedData.h" // used in "js/Conversions.h"
#pragma clang diagnostic pop
#include "../../StarlingMonkey/builtins/web/url.h"
#include "./fetch/request-response.h"
#include "backend.h"
#include "encode.h"
#include "fastly.h"
#include "js/Conversions.h"
#include "js/JSON.h"
#include "kv-store.h"
#include "logger.h"
#include <arpa/inet.h>

using builtins::web::url::URL;
using builtins::web::url::URLSearchParams;
using fastly::fastly::Fastly;
using fastly::fetch::RequestOrResponse;
using fastly::fetch::Response;
using fastly::logger::Logger;

extern char **environ;

namespace {

api::Engine *ENGINE;

// Global storage for Wizer-time environment
std::unordered_map<std::string, std::string> initialized_env;

static void oom_callback(JSContext *cx, void *data) {
  fprintf(stderr, "Critical Error: out of memory\n");
  fflush(stderr);
}

#ifdef DEBUG
static std::vector<std::string> debug_messages;
#endif

} // namespace

bool debug_logging_enabled() { return fastly::fastly::DEBUG_LOGGING_ENABLED; }

namespace fastly::fastly {

bool DEBUG_LOGGING_ENABLED = false;

JS::PersistentRooted<JSObject *> Fastly::env;
JS::PersistentRooted<JSObject *> Fastly::baseURL;
JS::PersistentRooted<JSString *> Fastly::defaultBackend;
bool allowDynamicBackendsCalled = false;
bool Fastly::allowDynamicBackends = true;
bool ENABLE_EXPERIMENTAL_HTTP_CACHE = false;

bool Fastly::dump(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS::CallArgs args = CallArgsFromVp(argc, vp);
  if (!args.requireAtLeast(cx, __func__, 1))
    return false;

// special debug mode operations
#ifndef NDEBUG
  if (args.get(0).isNull() && args.get(1).isString()) {
    JS::RootedString str(cx, args.get(1).toString());
    auto str_chars = core::encode(cx, str);
    if (!str_chars) {
      return false;
    }
    if (!strcmp(str_chars.ptr.get(), "invalidkv")) {
      host_api::HttpBody body(-1);
      host_api::HostBytes metadata{};
      // uint32_t gen = std::get<2>(res.unwrap());
      JS::RootedObject entry(
          cx, ::fastly::kv_store::KVStoreEntry::create(cx, body, std::move(metadata)));
      args.rval().setObject(*entry);
      return true;
    }
  }
#endif

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

bool debugLog(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS::CallArgs args = CallArgsFromVp(argc, vp);
  if (!args.requireAtLeast(cx, __func__, 1))
    return false;
  JS::RootedString msg_str(cx, JS::ToString(cx, args[0]));
  if (!msg_str) {
    return false;
  }
  auto msg_host_str = core::encode(cx, msg_str);
  if (!msg_host_str) {
    return false;
  }
#ifdef DEBUG
  debug_messages.push_back(std::string(msg_host_str));
#endif
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

  auto address = core::encode(cx, address_str);
  if (!address) {
    return false;
  }

  // TODO: Remove all of this and rely on the host for validation as the hostcall only takes one
  // user-supplied parameter
  int format = AF_INET;
  size_t octets_len = 4;
  if (std::find(address.begin(), address.end(), ':') != address.end()) {
    format = AF_INET6;
    octets_len = 16;
  }

  uint8_t octets[sizeof(struct in6_addr)];
  if (inet_pton(format, address.begin(), octets) != 1) {
    // While get_geo_info can be invoked through FetchEvent#client.geo, too,
    // that path can't result in an invalid address here, so we can be more
    // specific in the error message.
    // TODO: Make a TypeError
    JS_ReportErrorLatin1(cx, "Invalid address passed to fastly.getGeolocationForIpAddress");
    return false;
  }

  auto res = host_api::GeoIp::lookup(std::span<uint8_t>{octets, octets_len});
  if (auto *err = res.to_err()) {
    HANDLE_ERROR(cx, *err);
    return false;
  }

  if (!res.unwrap().has_value()) {
    args.rval().setNull();
    return true;
  }

  auto ret = std::move(res.unwrap().value());

  JS::RootedString geo_info_str(
      cx, JS_NewStringCopyUTF8N(cx, JS::UTF8Chars(ret.ptr.release(), ret.len)));
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
  auto grip_upgrade_request = &request_value.toObject();

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
  JS::RootedString backend_str(cx, JS::ToString(cx, backend_value));
  if (!backend_str) {
    return false;
  }
  auto backend_chars = core::encode(cx, backend_str);
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

  JS::RootedObject response(cx, Response::create(cx, response_instance, response_handle.unwrap(),
                                                 body_handle.unwrap(), is_upstream,
                                                 grip_upgrade_request, nullptr, backend_str));
  if (!response) {
    return false;
  }

  RequestOrResponse::set_url(response, RequestOrResponse::url(&request_value.toObject()));
  args.rval().setObject(*response);

  return true;
}

bool Fastly::createWebsocketHandoff(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS::CallArgs args = CallArgsFromVp(argc, vp);
  REQUEST_HANDLER_ONLY("createWebsocketHandoff");
  if (!args.requireAtLeast(cx, "createWebsocketHandoff", 2)) {
    return false;
  }

  auto request_value = args.get(0);
  if (!Request::is_instance(request_value)) {
    JS_ReportErrorUTF8(cx,
                       "createWebsocketHandoff: request parameter must be an instance of Request");
    return false;
  }
  auto websocket_upgrade_request = &request_value.toObject();

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
  JS::RootedString backend_str(cx, JS::ToString(cx, backend_value));
  if (!backend_str) {
    return false;
  }
  auto backend_chars = core::encode(cx, backend_str);
  if (!backend_chars) {
    return false;
  }
  if (backend_chars.len == 0) {
    JS_ReportErrorUTF8(cx, "createWebsocketHandoff: Backend parameter can not be an empty string");
    return false;
  }

  if (backend_chars.len > 254) {
    JS_ReportErrorUTF8(cx, "createWebsocketHandoff: name can not be more than 254 characters");
    return false;
  }

  bool is_upstream = true;

  JS::RootedObject response(cx, Response::create(cx, response_instance, response_handle.unwrap(),
                                                 body_handle.unwrap(), is_upstream, nullptr,
                                                 websocket_upgrade_request, backend_str));
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

bool compute_get_vcpu_time(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS::CallArgs args = CallArgsFromVp(argc, vp);
  auto res = host_api::Compute::get_vcpu_ms();
  if (auto *err = res.to_err()) {
    HANDLE_ERROR(cx, *err);
    return false;
  }
  args.rval().setNumber(res.unwrap());
  return true;
}

bool compute_purge_surrogate_key(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS::CallArgs args = CallArgsFromVp(argc, vp);
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
  auto purge_res = host_api::Compute::purge_surrogate_key(surrogate_key_chars, soft);
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

  JS::RootedString str(cx, JS::ToString(cx, args[0]));
  if (!str) {
    return false;
  }

  JS::UniqueChars ptr = JS_EncodeStringToUTF8(cx, str);
  if (!ptr) {
    return false;
  }

  // This shouldn't fail, since the encode operation ensured `str` is linear.
  JSLinearString *linear = JS_EnsureLinearString(cx, str);
  uint32_t len = JS::GetDeflatedUTF8StringLength(linear);

  std::string key_str(ptr.get(), len);

  // First check initialized environment
  if (auto it = initialized_env.find(key_str); it != initialized_env.end()) {
    JS::RootedString env_var(cx, JS_NewStringCopyN(cx, it->second.data(), it->second.size()));
    if (!env_var)
      return false;
    args.rval().setString(env_var);
    return true;
  }

  // Fallback to getenv with caching
  if (const char *value = std::getenv(key_str.c_str())) {
    auto [it, _] = initialized_env.emplace(key_str, value);
    JS::RootedString env_var(cx, JS_NewStringCopyN(cx, it->second.data(), it->second.size()));
    if (!env_var)
      return false;
    args.rval().setString(env_var);
    return true;
  }

  args.rval().setUndefined();
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
  if (!allowDynamicBackendsCalled) {
    allowDynamicBackends = false;
  }
  args.rval().setUndefined();
  return true;
}

#ifdef DEBUG
bool debugMessages_get(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS::CallArgs args = CallArgsFromVp(argc, vp);
  JS::RootedObject debugMessages(cx, JS::NewArrayObject(cx, 0));
  for (const auto &msg : debug_messages) {
    JS::RootedString str(cx, JS_NewStringCopyUTF8N(cx, JS::UTF8Chars(msg.data(), msg.length())));
    MOZ_ASSERT(str);
    bool res;
    uint32_t len;
    res = JS::GetArrayLength(cx, debugMessages, &len);
    MOZ_ASSERT(res);
    res = JS_SetElement(cx, debugMessages, len, str);
    MOZ_ASSERT(res);
  }
  args.rval().setObject(*debugMessages);
  return true;
}
#endif

bool Fastly::allowDynamicBackends_get(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS::CallArgs args = CallArgsFromVp(argc, vp);
  args.rval().setBoolean(allowDynamicBackends);
  return true;
}

bool Fastly::allowDynamicBackends_set(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS::CallArgs args = CallArgsFromVp(argc, vp);
  JS::HandleValue set_value = args.get(0);
  if (set_value.isObject()) {
    RootedObject options_value(cx, &set_value.toObject());
    if (!backend::set_default_backend_config(cx, argc, vp)) {
      return false;
    }
    allowDynamicBackends = true;
  } else {
    allowDynamicBackends = JS::ToBoolean(set_value);
  }
  allowDynamicBackendsCalled = true;
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
#ifdef DEBUG
    JS_PSG("debugMessages", debugMessages_get, JSPROP_ENUMERATE),
#endif
    JS_PS_END};

bool install(api::Engine *engine) {
  ENGINE = engine;

  bool ENABLE_EXPERIMENTAL_HIGH_RESOLUTION_TIME_METHODS =
      std::string(std::getenv("ENABLE_EXPERIMENTAL_HIGH_RESOLUTION_TIME_METHODS")) == "1";
  ENABLE_EXPERIMENTAL_HTTP_CACHE =
      std::string(std::getenv("ENABLE_EXPERIMENTAL_HTTP_CACHE")) == "1";

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
      JS_FN("debugLog", debugLog, 1, JSPROP_ENUMERATE),
      JS_FN("getGeolocationForIpAddress", Fastly::getGeolocationForIpAddress, 1, JSPROP_ENUMERATE),
      JS_FN("getLogger", Fastly::getLogger, 1, JSPROP_ENUMERATE),
      JS_FN("includeBytes", Fastly::includeBytes, 1, JSPROP_ENUMERATE),
      JS_FN("createFanoutHandoff", Fastly::createFanoutHandoff, 2, JSPROP_ENUMERATE),
      JS_FN("createWebsocketHandoff", Fastly::createWebsocketHandoff, 2, JSPROP_ENUMERATE),
      ENABLE_EXPERIMENTAL_HIGH_RESOLUTION_TIME_METHODS ? nowfn : end,
      end};

  if (!JS_DefineFunctions(engine->cx(), fastly, methods) ||
      !JS_DefineProperties(engine->cx(), fastly, Fastly::properties)) {
    return false;
  }

  // fastly:env
  // first, store the initialized environment vars from Wizer
  initialized_env.clear();

  for (char **env = environ; *env; env++) {
    const char *entry = *env;
    const char *eq = entry;
    while (*eq && *eq != '=')
      eq++;

    if (*eq == '=') {
      initialized_env.emplace(std::string(entry, eq - entry), std::string(eq + 1));
    }
  }
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

  // fastly:compute
  RootedObject compute_builtin(engine->cx(), JS_NewObject(engine->cx(), nullptr));
  auto compute_purge_surrogate_key_fn =
      JS_NewFunction(engine->cx(), &compute_purge_surrogate_key, 1, 0, "purgeSurrogateKey");
  RootedObject compute_purge_surrogate_key_obj(
      engine->cx(), JS_GetFunctionObject(compute_purge_surrogate_key_fn));
  RootedValue compute_purge_surrogate_key_val(engine->cx(),
                                              ObjectValue(*compute_purge_surrogate_key_obj));

  if (!JS_SetProperty(engine->cx(), compute_builtin, "purgeSurrogateKey",
                      compute_purge_surrogate_key_val)) {
    return false;
  }
  if (!JS_SetProperty(engine->cx(), fastly, "purgeSurrogateKey", compute_purge_surrogate_key_val)) {
    return false;
  }
  auto compute_vcpu_time_get =
      JS_NewFunction(engine->cx(), &compute_get_vcpu_time, 0, 0, "vCpuTime");
  RootedObject compute_vcpu_time_get_obj(engine->cx(), JS_GetFunctionObject(compute_vcpu_time_get));
  RootedValue compute_vcpu_time_get_val(engine->cx(), ObjectValue(*compute_vcpu_time_get_obj));
  if (!JS_SetProperty(engine->cx(), compute_builtin, "vCpuTime", compute_vcpu_time_get_val)) {
    return false;
  }
  if (!JS_SetProperty(engine->cx(), fastly, "vCpuTime", compute_vcpu_time_get_val)) {
    return false;
  }
  RootedValue compute_builtin_val(engine->cx(), JS::ObjectValue(*compute_builtin));
  if (!engine->define_builtin_module("fastly:compute", compute_builtin_val)) {
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
  auto enable_debug_logging =
      JS_NewFunction(engine->cx(), &Fastly::enableDebugLogging, 1, 0, "enableDebugLogging");
  RootedObject enable_debug_logging_obj(engine->cx(), JS_GetFunctionObject(enable_debug_logging));
  RootedValue enable_debug_logging_val(engine->cx(), ObjectValue(*enable_debug_logging_obj));
  if (!JS_SetProperty(engine->cx(), experimental, "enableDebugLogging", enable_debug_logging_val)) {
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
  // fastly:websocket
  RootedObject websocket(engine->cx(), JS_NewObject(engine->cx(), nullptr));
  RootedValue websocket_val(engine->cx(), JS::ObjectValue(*websocket));
  RootedValue create_websocket_handoff_val(engine->cx());
  if (!JS_GetProperty(engine->cx(), fastly, "createWebsocketHandoff",
                      &create_websocket_handoff_val)) {
    return false;
  }
  if (!JS_SetProperty(engine->cx(), websocket, "createWebsocketHandoff",
                      create_websocket_handoff_val)) {
    return false;
  }
  if (!engine->define_builtin_module("fastly:websocket", websocket_val)) {
    return false;
  }

  // debugMessages for debug-only builds
#ifdef DEBUG
  debug_messages.clear();
#endif

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

void fastly_push_debug_message(std::string msg) {
#ifdef DEBUG
  if (fastly::fastly::DEBUG_LOGGING_ENABLED) {
    // Log to both stderr and debug message log
    fprintf(stderr, "%.*s\n", static_cast<int>(msg.size()), msg.data());
    fflush(stderr);
    debug_messages.push_back(msg);
  }
#endif
}
