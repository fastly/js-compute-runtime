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
  auto result = convert_to_fastly_status(
      xqd_geo_lookup(address.get(), address_len, buffer.get(), HOSTCALL_BUFFER_LEN, &nwritten));
  if (result == FastlyStatus::Inval) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_GEO_IP_INVALID);
    return nullptr;
  }
  if (!HANDLE_RESULT(cx, result)) {
    return nullptr;
  }

  return JS_NewStringCopyUTF8N(cx, JS::UTF8Chars(buffer.get(), nwritten));
}
