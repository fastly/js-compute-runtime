#include <arpa/inet.h>

#include "geo_ip.h"
#include "host_call.h"
#include "js-compute-builtins.h" // for encode
#include "xqd.h"

JSString *get_geo_info(JSContext *cx, JS::HandleString address_str) {
  size_t address_len;
  JS::UniqueChars address = encode(cx, address_str, &address_len);
  if (!address)
    return nullptr;

  // TODO: Remove all of this and rely on the host for validation as the hostcall only takes one
  // user-supplied parameter
  int format = AF_INET;
  size_t octets_len = 4;
  const char *caddress = address.get();
  for (size_t i = 0; i < address_len; i++) {
    if (caddress[i] == ':') {
      format = AF_INET6;
      octets_len = 16;
      break;
    }
  }

  uint8_t octets[sizeof(struct in6_addr)];
  if (inet_pton(format, caddress, octets) != 1) {
    // While get_geo_info can be invoked through FetchEvent#client.geo, too,
    // that path can't result in an invalid address here, so we can be more
    // specific in the error message.
    // TODO: Make a TypeError
    JS_ReportErrorLatin1(cx, "Invalid address passed to fastly.getGeolocationForIpAddress");
    return nullptr;
  }

  fastly_list_u8_t octets_list = {const_cast<uint8_t *>(&octets[0]), octets_len};

  xqd_world_string_t ret;
  if (!HANDLE_RESULT(cx, xqd_fastly_geo_lookup(&octets_list, &ret))) {
    return nullptr;
  }

  return JS_NewStringCopyUTF8N(cx, JS::UTF8Chars(ret.ptr, ret.len));
}
