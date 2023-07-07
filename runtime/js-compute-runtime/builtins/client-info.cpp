#include "builtins/client-info.h"
#include "core/geo_ip.h"
#include "host_interface/host_api.h"
#include "openssl/evp.h"

#include "js/JSON.h"
#include <arpa/inet.h>

namespace builtins {

namespace {

JSString *address(JSObject *obj) {
  JS::Value val = JS::GetReservedSlot(obj, static_cast<uint32_t>(ClientInfo::Slots::Address));
  return val.isString() ? val.toString() : nullptr;
}

JSString *geo_info(JSObject *obj) {
  JS::Value val = JS::GetReservedSlot(obj, static_cast<uint32_t>(ClientInfo::Slots::GeoInfo));
  return val.isString() ? val.toString() : nullptr;
}

static JSString *retrieve_address(JSContext *cx, JS::HandleObject self) {
  auto res = HttpReq::downstream_client_ip_addr();
  if (auto *err = res.to_err()) {
    HANDLE_ERROR(cx, *err);
    return nullptr;
  }

  auto octets = std::move(res.unwrap());
  char address_chars[INET6_ADDRSTRLEN];
  int addr_family = 0;
  socklen_t size = 0;

  switch (octets.len) {
  case 0: {
    // No address to be had, leave `address` as a nullptr.
    break;
  }
  case 4: {
    addr_family = AF_INET;
    size = INET_ADDRSTRLEN;
    break;
  }
  case 16: {
    addr_family = AF_INET6;
    size = INET6_ADDRSTRLEN;
    break;
  }
  }

  JS::RootedString address(cx);
  if (octets.len > 0) {
    // TODO: do we need to do error handling here, or can we depend on the
    // host giving us a valid address?
    inet_ntop(addr_family, octets.begin(), address_chars, size);
    address = JS_NewStringCopyZ(cx, address_chars);
    if (!address) {
      return nullptr;
    }
  }

  JS::SetReservedSlot(self, static_cast<uint32_t>(ClientInfo::Slots::Address),
                      JS::StringValue(address));
  return address;
}

JSString *retrieve_geo_info(JSContext *cx, JS::HandleObject self) {
  JS::RootedString address_str(cx, address(self));
  if (!address_str) {
    address_str = retrieve_address(cx, self);
    if (!address_str)
      return nullptr;
  }

  JS::RootedString geo(cx, get_geo_info(cx, address_str));
  if (!geo)
    return nullptr;

  JS::SetReservedSlot(self, static_cast<uint32_t>(ClientInfo::Slots::GeoInfo),
                      JS::StringValue(geo));
  return geo;
}

} // namespace

bool ClientInfo::address_get(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0)

  JS::RootedString address_str(cx, address(self));
  if (!address_str) {
    address_str = retrieve_address(cx, self);
    if (!address_str)
      return false;
  }

  args.rval().setString(address_str);
  return true;
}

bool ClientInfo::geo_get(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0)

  JS::RootedString geo_info_str(cx, geo_info(self));
  if (!geo_info_str) {
    geo_info_str = retrieve_geo_info(cx, self);
    if (!geo_info_str)
      return false;
  }

  return JS_ParseJSON(cx, geo_info_str, args.rval());
}

bool ClientInfo::tls_cipher_openssl_name_get(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0);

  auto res = HttpReq::http_req_downstream_tls_cipher_openssl_name();
  if (auto *err = res.to_err()) {
    HANDLE_ERROR(cx, *err);
    return false;
  }

  HostString cipher = std::move(res.unwrap());
  JS::RootedString result(cx, JS_NewStringCopyN(cx, cipher.ptr.get(), cipher.len));

  args.rval().setString(result);
  return true;
}

bool ClientInfo::tls_ja3_md5_get(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0);

  auto res = HttpReq::http_req_downstream_tls_ja3_md5();
  if (auto *err = res.to_err()) {
    HANDLE_ERROR(cx, *err);
    return false;
  }

  auto ja3 = std::move(res.unwrap());
  JS::UniqueChars hex{OPENSSL_buf2hexstr(ja3.ptr.get(), ja3.len)};
  std::string ja3hex{hex.get(), std::remove(hex.get(), hex.get() + strlen(hex.get()), ':')};

  JS::RootedString s(cx, JS_NewStringCopyN(cx, ja3hex.c_str(), ja3hex.length()));
  args.rval().setString(s);
  return true;
}

bool ClientInfo::tls_client_hello_get(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0);

  auto res = HttpReq::http_req_downstream_tls_client_hello();
  if (auto *err = res.to_err()) {
    HANDLE_ERROR(cx, *err);
    return false;
  }

  HostBytes hello = std::move(res.unwrap());
  JS::RootedObject buffer(cx, JS::NewArrayBufferWithContents(cx, hello.len, hello.ptr.get()));
  if (!buffer) {
    // We can be here if the array buffer was too large -- if that was the case then a
    // JSMSG_BAD_ARRAY_LENGTH will have been created.
    return false;
  }

  // `hello` is now owned by `buffer`
  static_cast<void>(hello.ptr.release());

  args.rval().setObject(*buffer);
  return true;
}

bool ClientInfo::tls_client_certificate_get(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0);

  auto res = HttpReq::http_req_downstream_tls_raw_client_certificate();
  if (auto *err = res.to_err()) {
    HANDLE_ERROR(cx, *err);
    return false;
  }
  HostBytes cert = std::move(res.unwrap());

  JS::RootedObject buffer(cx, JS::NewArrayBufferWithContents(cx, cert.len, cert.ptr.get()));
  if (!buffer) {
    // We can be here if the array buffer was too large -- if that was the case then a
    // JSMSG_BAD_ARRAY_LENGTH will have been created.
    return false;
  }

  // `cert` is now owned by `buffer`
  static_cast<void>(cert.ptr.release());

  args.rval().setObject(*buffer);
  return true;
}

bool ClientInfo::tls_protocol_get(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0);

  auto res = HttpReq::http_req_downstream_tls_protocol();
  if (auto *err = res.to_err()) {
    HANDLE_ERROR(cx, *err);
    return false;
  }

  HostString protocol = std::move(res.unwrap());
  JS::RootedString result(cx, JS_NewStringCopyN(cx, protocol.ptr.get(), protocol.len));

  args.rval().setString(result);
  return true;
}

const JSFunctionSpec ClientInfo::static_methods[] = {
    JS_FS_END,
};

const JSPropertySpec ClientInfo::static_properties[] = {
    JS_PS_END,
};

const JSFunctionSpec ClientInfo::methods[] = {
    JS_FS_END,
};

const JSPropertySpec ClientInfo::properties[] = {
    JS_PSG("address", address_get, JSPROP_ENUMERATE),
    JS_PSG("geo", geo_get, JSPROP_ENUMERATE),
    JS_PSG("tlsCipherOpensslName", tls_cipher_openssl_name_get, JSPROP_ENUMERATE),
    JS_PSG("tlsProtocol", tls_protocol_get, JSPROP_ENUMERATE),
    JS_PSG("tlsJA3MD5", tls_ja3_md5_get, JSPROP_ENUMERATE),
    JS_PSG("tlsClientCertificate", tls_client_certificate_get, JSPROP_ENUMERATE),
    JS_PSG("tlsClientHello", tls_client_hello_get, JSPROP_ENUMERATE),
    JS_PS_END,
};

JSObject *ClientInfo::create(JSContext *cx) {
  return JS_NewObjectWithGivenProto(cx, &class_, proto_obj);
}

} // namespace builtins
