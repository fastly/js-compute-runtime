#ifndef FASTLY_FASTLY_H
#define FASTLY_FASTLY_H

#include "../../StarlingMonkey/builtins/web/url.h"
#include "../host-api/host_api_fastly.h"
#include "./fetch/request-response.h"
#include "builtin.h"
#include "extension-api.h"
#include "fastly.h"
#include "host_api.h"

namespace fastly::fastly {

extern bool DEBUG_LOGGING_ENABLED;
extern bool ENABLE_EXPERIMENTAL_HTTP_CACHE;

class Env : public builtins::BuiltinNoConstructor<Env> {
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

class ReusableSandboxOptions {
public:
  ReusableSandboxOptions() = default;
  ReusableSandboxOptions(const ReusableSandboxOptions &) = delete;
  ReusableSandboxOptions &operator=(const ReusableSandboxOptions &) = delete;

  std::optional<uint32_t> max_requests() const { return max_requests_; }
  bool set_max_requests(uint32_t max_requests) {
    if (frozen_) {
      return false;
    }
    max_requests_ = max_requests;
    return true;
  }
  std::optional<std::chrono::milliseconds> between_request_timeout() const {
    return between_request_timeout_;
  }
  bool set_between_request_timeout(std::chrono::milliseconds timeout) {
    if (frozen_) {
      return false;
    }
    between_request_timeout_ = timeout;
    return true;
  }
  std::optional<uint32_t> max_memory_mib() const { return max_memory_mib_; }
  bool set_max_memory_mib(uint32_t max_memory_mib) {
    if (frozen_) {
      return false;
    }
    max_memory_mib_ = max_memory_mib;
    return true;
  }
  std::optional<std::chrono::milliseconds> sandbox_timeout() const { return sandbox_timeout_; }
  bool set_sandbox_timeout(std::chrono::milliseconds timeout) {
    if (frozen_) {
      return false;
    }
    sandbox_timeout_ = timeout;
    return true;
  }
  bool frozen() const { return frozen_; }
  void freeze() { frozen_ = true; }

private:
  bool frozen_ = false;
  std::optional<uint32_t> max_requests_;
  std::optional<std::chrono::milliseconds> between_request_timeout_;
  std::optional<uint32_t> max_memory_mib_;
  std::optional<std::chrono::milliseconds> sandbox_timeout_;
};

class Fastly : public builtins::BuiltinNoConstructor<Fastly> {
private:
  static bool log(JSContext *cx, unsigned argc, JS::Value *vp);

public:
  static constexpr const char *class_name = "Fastly";

  static JS::PersistentRooted<JSObject *> env;
  static JS::PersistentRooted<JSObject *> baseURL;
  static JS::PersistentRooted<JSString *> defaultBackend;
  static bool allowDynamicBackends;
  static host_api::BackendConfig defaultDynamicBackendConfig;
  static ReusableSandboxOptions reusableSandboxOptions;

  static const JSPropertySpec properties[];

  static bool createFanoutHandoff(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool createWebsocketHandoff(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool now(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool dump(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool enableDebugLogging(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool getGeolocationForIpAddress(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool getLogger(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool includeBytes(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool version_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool env_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool baseURL_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool baseURL_set(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool defaultBackend_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool defaultBackend_set(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool allowDynamicBackends_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool allowDynamicBackends_set(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool inspect(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool setReusableSandboxOptions(JSContext *cx, unsigned argc, JS::Value *vp);
};

JS::Result<std::tuple<JS::UniqueChars, size_t>> convertBodyInit(JSContext *cx,
                                                                JS::HandleValue bodyInit);

inline bool get_fastly_object(api::Engine *engine, JS::MutableHandleObject out) {
  JS::RootedValue fastly_val(engine->cx());
  if (!JS_GetProperty(engine->cx(), engine->global(), "fastly", &fastly_val)) {
    return false;
  }
  if (fastly_val.isObject()) {
    out.set(&fastly_val.toObject());
    return true;
  }
  JS::RootedObject fastly_obj(engine->cx(), JS_NewPlainObject(engine->cx()));
  if (!fastly_obj) {
    return false;
  }
  if (!JS_DefineProperty(engine->cx(), engine->global(), "fastly", fastly_obj, 0)) {
    return false;
  }
  out.set(fastly_obj);
  return true;
}

/**
 * Debug only logging system, adding messages to `fastly.debugMessages`
 *
 * This is useful for debugging compute, allowing messages to be inlined into the response in test
 * case debugging, where other logging systems may introduce greater latency than this.
 */

} // namespace fastly::fastly

void fastly_push_debug_message(std::string msg);

#endif
