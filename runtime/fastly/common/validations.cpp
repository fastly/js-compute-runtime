#include "validations.h"

#include "../builtins/fastly.h"
#include "../host-api/host_api_fastly.h"
#include "encode.h"

using fastly::fastly::FastlyGetErrorMessage;

namespace fastly::common {
std::optional<uint32_t> parse_and_validate_timeout(JSContext *cx, JS::HandleValue value,
                                                   const char *subsystem, std::string property_name,
                                                   uint64_t max_timeout) {
  double native_value;
  if (!JS::ToNumber(cx, value, &native_value)) {
    return std::nullopt;
  }
  if (std::isnan(native_value)) {
    JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr, JSMSG_TIMEOUT_NAN, subsystem,
                              property_name.c_str());
    return std::nullopt;
  }
  if (native_value < 0) {
    JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr, JSMSG_TIMEOUT_NEGATIVE, subsystem,
                              property_name.c_str());
    return std::nullopt;
  }
  if (native_value >= max_timeout) {
    std::string max_timeout_str = std::to_string(max_timeout);
    JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr, JSMSG_TIMEOUT_TOO_BIG, subsystem,
                              property_name.c_str(), max_timeout_str.c_str());
    return std::nullopt;
  }
  return std::round(native_value);
}

std::optional<std::tuple<const uint8_t *, size_t>>
validate_bytes(JSContext *cx, JS::HandleValue bytes, const char *subsystem) {
  if (!bytes.isObject()) {
    JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr, JSMSG_INVALID_BUFFER, subsystem);
    return std::nullopt;
  }

  JS::RootedObject bytes_obj(cx, &bytes.toObject());

  if (!JS::IsArrayBufferObject(bytes_obj) && !JS_IsArrayBufferViewObject(bytes_obj)) {
    JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr, JSMSG_INVALID_BUFFER, subsystem);
    return std::nullopt;
  }

  mozilla::Maybe<JS::AutoCheckCannotGC> maybeNoGC;
  uint8_t *buf;
  size_t length;
  if (JS_IsArrayBufferViewObject(bytes_obj)) {
    JS::AutoCheckCannotGC noGC;
    bool is_shared;
    length = JS_GetArrayBufferViewByteLength(bytes_obj);
    buf = (uint8_t *)JS_GetArrayBufferViewData(bytes_obj, &is_shared, noGC);
    MOZ_ASSERT(!is_shared);
  } else if (JS::IsArrayBufferObject(bytes_obj)) {
    bool is_shared;
    JS::GetArrayBufferLengthAndData(bytes_obj, &length, &is_shared, (uint8_t **)&buf);
  } else {
    JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr, JSMSG_INVALID_BUFFER, subsystem);
    return std::nullopt;
  }

  return std::make_tuple(buf, length);
}

} // namespace fastly::common
