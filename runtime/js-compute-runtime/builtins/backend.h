#ifndef JS_COMPUTE_RUNTIME_BACKEND_H
#define JS_COMPUTE_RUNTIME_BACKEND_H

#include "builtin.h"
#include "js-compute-builtins.h"

namespace builtins {

class Backend : public BuiltinImpl<Backend> {
private:
public:
  static constexpr const char *class_name = "Backend";
  static const int ctor_length = 1;
  enum Slots {
    Name,
    Target,
    HostOverride,
    ConnectTimeout,
    FirstByteTimeout,
    BetweenBytesTimeout,
    UseSsl,
    TlsMinVersion,
    TlsMaxVersion,
    CertificateHostname,
    CaCertificate,
    Ciphers,
    SniHostname,
    Count
  };

  static const JSFunctionSpec static_methods[];
  static const JSPropertySpec static_properties[];

  static const JSFunctionSpec methods[];
  static const JSPropertySpec properties[];

  inline static JS::PersistentRootedObject backends;

  static bool isCipherSuiteSupportedByFastly(std::string_view cipherSpec);
  static JSString *name(JSContext *cx, JSObject *self);
  static JS::Result<mozilla::Ok> register_dynamic_backend(JSContext *cx, JS::HandleObject request);
  static JSObject *create(JSContext *cx, JS::HandleObject request);
  static bool set_target(JSContext *cx, JSObject *backend, JS::HandleValue target_val);
  static bool set_timeout_slot(JSContext *cx, JSObject *backend, JS::HandleValue value,
                               Backend::Slots slot, std::string property_name);
  static bool set_host_override(JSContext *cx, JSObject *backend, JS::HandleValue hostOverride_val);
  static bool set_sni_hostname(JSContext *cx, JSObject *backend, JS::HandleValue sniHostname_val);
  static bool set_name(JSContext *cx, JSObject *backend, JS::HandleValue name_val);
  static bool toString(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool constructor(JSContext *cx, unsigned argc, JS::Value *vp);

  static bool init_class(JSContext *cx, JS::HandleObject global);
};

} // namespace builtins

#endif
