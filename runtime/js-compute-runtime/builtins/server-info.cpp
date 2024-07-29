#include "builtins/server-info.h"
#include "core/geo_ip.h"
#include "host_interface/host_api.h"
#include "js/JSON.h"
#include "openssl/evp.h"
#include <arpa/inet.h>

namespace builtins {

namespace {

static JSString *retrieve_address(JSContext *cx, JS::HandleObject self) {
  auto res = host_api::HttpReq::downstream_server_ip_addr();
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
    if (!inet_ntop(addr_family, octets.begin(), address_chars, size)) {
      return nullptr;
    }
    address = JS_NewStringCopyZ(cx, address_chars);
    if (!address) {
      return nullptr;
    }
  }

  JS::SetReservedSlot(self, static_cast<uint32_t>(ServerInfo::Slots::Address),
                      JS::StringValue(address));
  return address;
}

} // namespace

bool ServerInfo::address_get(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0);

  JS::RootedString address_str(cx);
  JS::Value val = JS::GetReservedSlot(self, static_cast<uint32_t>(ServerInfo::Slots::Address));
  if (val.isString()) {
    address_str = val.toString();
  } else {
    address_str = retrieve_address(cx, self);
    if (!address_str) {
      return false;
    }
  }

  args.rval().setString(address_str);
  return true;
}

const JSFunctionSpec ServerInfo::static_methods[] = {
    JS_FS_END,
};

const JSPropertySpec ServerInfo::static_properties[] = {
    JS_PS_END,
};

const JSFunctionSpec ServerInfo::methods[] = {
    JS_FS_END,
};

const JSPropertySpec ServerInfo::properties[] = {
    JS_PSG("address", address_get, JSPROP_ENUMERATE),
    JS_PS_END,
};

JSObject *ServerInfo::create(JSContext *cx) {
  return JS_NewObjectWithGivenProto(cx, &class_, proto_obj);
}

} // namespace builtins
