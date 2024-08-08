#include "../../../StarlingMonkey/runtime/encode.h"
#include "builtin.h"
#include "host_api.h"

namespace fastly::common {
const char VALID_NAME_CHARS[128] = {
    0, 0, 0, 0, 0, 0, 0, 0, //   0
    0, 0, 0, 0, 0, 0, 0, 0, //   8
    0, 0, 0, 0, 0, 0, 0, 0, //  16
    0, 0, 0, 0, 0, 0, 0, 0, //  24

    0, 1, 0, 1, 1, 1, 1, 1, //  32
    0, 0, 1, 1, 0, 1, 1, 0, //  40
    1, 1, 1, 1, 1, 1, 1, 1, //  48
    1, 1, 0, 0, 0, 0, 0, 0, //  56

    0, 1, 1, 1, 1, 1, 1, 1, //  64
    1, 1, 1, 1, 1, 1, 1, 1, //  72
    1, 1, 1, 1, 1, 1, 1, 1, //  80
    1, 1, 1, 0, 0, 0, 1, 1, //  88

    1, 1, 1, 1, 1, 1, 1, 1, //  96
    1, 1, 1, 1, 1, 1, 1, 1, // 104
    1, 1, 1, 1, 1, 1, 1, 1, // 112
    1, 1, 1, 0, 1, 0, 1, 0  // 120
};

/**
 * Validates and normalizes the given header name, by
 * - checking for invalid characters
 * - converting to lower-case
 *
 * See
 * https://searchfox.org/mozilla-central/rev/9f76a47f4aa935b49754c5608a1c8e72ee358c46/netwerk/protocol/http/nsHttp.cpp#172-215
 * For details on validation.
 *
 * Mutates `name_val` in place, and returns the name as UniqueChars.
 * This is done because most uses of header names require handling of both the
 * JSString and the char* version, so they'd otherwise have to recreate one of
 * the two.
 */
host_api::HostString normalize_header_name(JSContext *cx, JS::MutableHandleValue name_val,
                                           const char *fun_name) {
  JS::RootedString name_str(cx, JS::ToString(cx, name_val));
  if (!name_str) {
    return nullptr;
  }

  auto name = core::encode(cx, name_str);
  if (!name) {
    return nullptr;
  }

  if (name.len == 0) {
    JS_ReportErrorASCII(cx, "%s: Header name can't be empty", fun_name);
    return nullptr;
  }

  bool changed = false;

  char *name_chars = name.begin();
  for (size_t i = 0; i < name.len; i++) {
    unsigned char ch = name_chars[i];
    if (ch > 127 || !VALID_NAME_CHARS[ch]) {
      JS_ReportErrorUTF8(cx, "%s: Invalid header name '%s'", fun_name, name_chars);
      return nullptr;
    }

    if (ch >= 'A' && ch <= 'Z') {
      name_chars[i] = ch - 'A' + 'a';
      changed = true;
    }
  }

  if (changed) {
    name_str = JS_NewStringCopyN(cx, name_chars, name.len);
    if (!name_str) {
      return nullptr;
    }
  }

  name_val.setString(name_str);
  return name;
}

host_api::HostString normalize_header_value(JSContext *cx, JS::MutableHandleValue value_val,
                                            const char *fun_name) {
  JS::RootedString value_str(cx, JS::ToString(cx, value_val));
  if (!value_str) {
    return nullptr;
  }

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
  if (!JS_DeprecatedStringHasLatin1Chars(value_str)) {
#pragma clang diagnostic pop
    JS::AutoCheckCannotGC nogc;
    size_t length;
    const char16_t *chars = JS_GetTwoByteStringCharsAndLength(cx, nogc, value_str, &length);
    for (auto i = 0; i < length; i++) {
      if (chars[i] > 255) {
        JS_ReportErrorASCII(cx, "header value contains bytes greater than 255");
        return nullptr;
      }
    }
  }

  host_api::HostString value;
  value.ptr = JS_EncodeStringToLatin1(cx, value_str);
  if (!value.ptr) {
    return nullptr;
  }
  value.len = JS_GetStringLength(value_str);

  auto *value_chars = value.begin();
  size_t start = 0;
  size_t end = value.len;

  // We follow Gecko's interpretation of what's a valid header value. After
  // stripping leading and trailing whitespace, all interior line breaks and
  // `\0` are considered invalid. See
  // https://searchfox.org/mozilla-central/rev/9f76a47f4aa935b49754c5608a1c8e72ee358c46/netwerk/protocol/http/nsHttp.cpp#247-260
  // for details.
  while (start < end) {
    unsigned char ch = value_chars[start];
    if (ch == '\t' || ch == ' ' || ch == '\r' || ch == '\n') {
      start++;
    } else {
      break;
    }
  }

  while (end > start) {
    unsigned char ch = value_chars[end - 1];
    if (ch == '\t' || ch == ' ' || ch == '\r' || ch == '\n') {
      end--;
    } else {
      break;
    }
  }

  for (size_t i = start; i < end; i++) {
    unsigned char ch = value_chars[i];
    if (ch == '\r' || ch == '\n' || ch == '\0') {
      JS_ReportErrorUTF8(cx, "%s: Invalid header value '%s'", fun_name, value_chars);
      return nullptr;
    }
  }

  if (start != 0 || end != value.len) {
    value_str = JS_NewStringCopyUTF8N(cx, JS::UTF8Chars(value_chars + start, end - start));
    if (!value_str) {
      return nullptr;
    }
  }

  value_val.setString(value_str);

  return value;
}
} // namespace fastly::common