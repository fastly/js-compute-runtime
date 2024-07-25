#ifndef BUILTIN_CLIENT_INFO_H
#define BUILTIN_CLIENT_INFO_H

#include "builtin.h"

namespace builtins {

class ClientInfo final : public BuiltinNoConstructor<ClientInfo> {
  static bool address_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool geo_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool tls_cipher_openssl_name_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool tls_protocol_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool tls_client_hello_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool tls_ja3_md5_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool tls_client_certificate_get(JSContext *cx, unsigned argc, JS::Value *vp);

public:
  static constexpr const char *class_name = "ClientInfo";

  enum class Slots {
    Address,
    GeoInfo,
    Cipher,
    Protocol,
    ClientHello,
    JA3,
    ClientCert,
    Count,
  };
  static const JSFunctionSpec static_methods[];
  static const JSPropertySpec static_properties[];
  static const JSFunctionSpec methods[];
  static const JSPropertySpec properties[];

  static JSObject *create(JSContext *cx);
};

} // namespace builtins

#endif
