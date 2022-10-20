#include "geo_ip.h"
#include "host_call.h"
#include "js-compute-builtins.h" // for encode
#include "xqd.h"

JSString *get_geo_info(JSContext *cx, JS::HandleString address_str) {
  size_t address_len;
  JS::UniqueChars address = encode(cx, address_str, &address_len);
  if (!address) {
    return nullptr;
  }

  OwnedHostCallBuffer buffer;
  size_t nwritten = 0;
  auto result = convert_to_fastly_status(xqd_geo_lookup(address.get(), address_len, buffer.get(), HOSTCALL_BUFFER_LEN, &nwritten));
  if (result == FastlyStatus::Inval) {
    // While get_geo_info can be invoked through FetchEvent#client.geo, too,
    // that path can't result in an invalid address here, so we can be more
    // specific in the error message.
    // TODO: Make a TypeError
    JS_ReportErrorLatin1(cx, "Invalid address passed to fastly.getGeolocationForIpAddress");
    return nullptr;
  }
  if (!HANDLE_RESULT(cx, result)) {
    return nullptr;
  }

  return JS_NewStringCopyUTF8N(cx, JS::UTF8Chars(buffer.get(), nwritten));
}
