#include <algorithm>
#include <arpa/inet.h>

#include "core/encode.h"
#include "core/geo_ip.h"

JSString *get_geo_info(JSContext *cx, JS::HandleString address_str) {
  auto address = fastly::core::encode(cx, address_str);
  if (!address) {
    return nullptr;
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
    return nullptr;
  }

  auto res = GeoIp::lookup(std::span<uint8_t>{octets, octets_len});
  if (auto *err = res.to_err()) {
    HANDLE_ERROR(cx, *err);
    return nullptr;
  }

  auto ret = std::move(res.unwrap());

  return JS_NewStringCopyUTF8N(cx, JS::UTF8Chars(ret.ptr.release(), ret.len));
}
