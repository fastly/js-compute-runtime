#include "builtins/client-info.h"
#include "core/geo_ip.h"
#include "host_interface/host_call.h"

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
  JS::RootedString address(cx);

  fastly_list_u8_t octets;
  fastly_error_t err;
  if (!fastly_http_req_downstream_client_ip_addr(&octets, &err)) {
    HANDLE_ERROR(cx, err);
    return nullptr;
  }

  switch (octets.len) {
  case 0: {
    // No address to be had, leave `address` as a nullptr.
    JS_free(cx, octets.ptr);
    break;
  }
  case 4: {
    char address_chars[INET_ADDRSTRLEN];
    // TODO: do we need to do error handling here, or can we depend on the
    // host giving us a valid address?
    inet_ntop(AF_INET, octets.ptr, address_chars, INET_ADDRSTRLEN);
    address = JS_NewStringCopyZ(cx, address_chars);
    JS_free(cx, octets.ptr);
    if (!address)
      return nullptr;

    break;
  }
  case 16: {
    char address_chars[INET6_ADDRSTRLEN];
    // TODO: do we need to do error handling here, or can we depend on the
    // host giving us a valid address?
    inet_ntop(AF_INET6, octets.ptr, address_chars, INET6_ADDRSTRLEN);
    address = JS_NewStringCopyZ(cx, address_chars);
    JS_free(cx, octets.ptr);
    if (!address)
      return nullptr;

    break;
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
    JS_PS_END,
};

JSObject *ClientInfo::create(JSContext *cx) {
  return JS_NewObjectWithGivenProto(cx, &class_, proto_obj);
}

} // namespace builtins
