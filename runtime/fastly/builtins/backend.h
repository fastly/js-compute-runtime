#ifndef FASTLY_BACKEND_H
#define FASTLY_BACKEND_H

#include "../host-api/host_api_fastly.h"
#include "builtin.h"
#include "extension-api.h"
#include "../state.h"

namespace fastly::backend {

class Backend : public builtins::FinalizableBuiltinImpl<Backend> {
private:
public:
  static constexpr const char *class_name = "Backend";

  // Request State: Initialized once, snapshotted before first request,
  // then restored to snapshot between requests
  struct RequestState {
    JS::PersistentRootedObject backends;
    host_api::BackendConfig default_backend_config;

    bool init(JSContext *cx) {
      backends.init(cx, JS_NewPlainObject(cx));
      if (!backends) {
        return false;
      }
      return true;
    }

    bool snapshot(JSContext *cx, RequestState& into) {
      into.default_backend_config = default_backend_config.clone();

      into.backends.reset();
      into.backends.init(cx, JS_NewPlainObject(cx));

      JS::Rooted<JS::IdVector> props(cx, cx);
      if (!JS_Enumerate(cx, backends, &props)) {
        return false;
      }
      JS::RootedValue backend(cx);
      for (uint32_t i = 0, len = props.length(); i < len; i++) {
        if (!JS_GetPropertyById(cx, backends, props[i], &backend)) {
          return false;
        }
        if (!JS_SetPropertyById(cx, into.backends, props[i], backend)) {
          return false;
        }
      }

      return true;
    } 
  };

  static state::RequestStateHolder<RequestState> request_state;

  static const int ctor_length = 1;
  enum Slots { HostBackend, Count };

  static const JSFunctionSpec static_methods[];
  static const JSPropertySpec static_properties[];

  static const JSFunctionSpec methods[];
  static const JSPropertySpec properties[];
  static JSString *name(JSContext *cx, JSObject *self);
  static JSObject *create(JSContext *cx, JS::HandleObject request);

  // static methods
  static bool exists(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool from_name(JSContext *cx, unsigned argc, JS::Value *vp);

  static bool health_for_name(JSContext *cx, unsigned argc, JS::Value *vp);

  // prototype methods
  static bool health(JSContext *cx, unsigned argc, JS::Value *vp);

  // getters
  static bool name_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool is_dynamic_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool target_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool host_override_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool port_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool connect_timeout_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool first_byte_timeout_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool between_bytes_timeout_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool http_keepalive_time_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool tcp_keepalive_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool is_ssl_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool tls_min_version_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool tls_max_version_get(JSContext *cx, unsigned argc, JS::Value *vp);

  static bool constructor(JSContext *cx, unsigned argc, JS::Value *vp);
  static void finalize(JS::GCContext *gcx, JSObject *obj);

  static bool get_from_valid_name(JSContext *cx, host_api::HostString name,
                                  JS::MutableHandleValue out);
};

bool set_default_backend_config(JSContext *cx, unsigned argc, JS::Value *vp);

} // namespace fastly::backend

#endif
