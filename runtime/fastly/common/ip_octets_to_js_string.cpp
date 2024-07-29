#include <arpa/inet.h>

#include "builtin.h"
#include "extension-api.h"
#include "host_api.h"

namespace fastly::common {

JSString *ip_octets_to_js_string(JSContext *cx, host_api::HostBytes octets) {
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

  return address;
}

} // namespace fastly::common
