#ifndef FASTLY_BACKEND_H
#define FASTLY_BACKEND_H

#include "builtin.h"
#include "extension-api.h"

namespace fastly::backend {

class Backend : public builtins::BuiltinImpl<Backend> {
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
    DontPool,
    ClientCert,
    ClientCertKey,
    Count
  };

  static const JSFunctionSpec static_methods[];
  static const JSPropertySpec static_properties[];

  static const JSFunctionSpec methods[];
  static const JSPropertySpec properties[];

  inline static JS::PersistentRootedObject backends;

  static bool is_cipher_suite_supported_by_fastly(std::string_view cipherSpec);
  static JSString *name(JSContext *cx, JSObject *self);
  static JS::Result<mozilla::Ok> register_dynamic_backend(JSContext *cx, JS::HandleObject request);
  static JSObject *create(JSContext *cx, JS::HandleObject request);
  static bool set_target(JSContext *cx, JSObject *backend, JS::HandleValue target_val);
  static bool set_timeout_slot(JSContext *cx, JSObject *backend, JS::HandleValue value,
                               Backend::Slots slot, std::string property_name);
  static bool set_host_override(JSContext *cx, JSObject *backend, JS::HandleValue hostOverride_val);
  static bool set_sni_hostname(JSContext *cx, JSObject *backend, JS::HandleValue sniHostname_val);
  static bool set_name(JSContext *cx, JSObject *backend, JS::HandleValue name_val);
  static bool set_client_cert(JSContext *cx, JSObject *backend, JS::HandleValue client_cert_val);
  static bool set_client_cert_key(JSContext *cx, JSObject *backend,
                                  JS::HandleValue client_cert_key_val);

  // static methods
  static bool exists(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool from_name(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool health(JSContext *cx, unsigned argc, JS::Value *vp);

  // prototype methods
  static bool to_name(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool to_string(JSContext *cx, unsigned argc, JS::Value *vp);

  static bool constructor(JSContext *cx, unsigned argc, JS::Value *vp);
};

} // namespace fastly::backend

#endif
