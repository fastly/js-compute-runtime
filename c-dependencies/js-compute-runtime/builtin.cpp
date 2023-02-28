#include "builtin.h"

const JSErrorFormatString *GetErrorMessageBuiltin(void *userRef, unsigned errorNumber) {
  if (errorNumber > 0 && errorNumber < JSBuiltinErrNum_Limit) {
    return &js_ErrorFormatStringBuiltin[errorNumber];
  }

  return nullptr;
}

JS::UniqueChars encode(JSContext *cx, JS::HandleString str, size_t *encoded_len) {
  JS::UniqueChars text = JS_EncodeStringToUTF8(cx, str);
  if (!text)
    return nullptr;

  // This shouldn't fail, since the encode operation ensured `str` is linear.
  JSLinearString *linear = JS_EnsureLinearString(cx, str);
  *encoded_len = JS::GetDeflatedUTF8StringLength(linear);
  return text;
}

JS::UniqueChars encode(JSContext *cx, JS::HandleValue val, size_t *encoded_len) {
  JS::RootedString str(cx, JS::ToString(cx, val));
  if (!str)
    return nullptr;

  return encode(cx, str, encoded_len);
}

jsurl::SpecString encode(JSContext *cx, JS::HandleValue val) {
  jsurl::SpecString slice(nullptr, 0, 0);
  auto chars = encode(cx, val, &slice.len);
  if (!chars)
    return slice;
  slice.data = (uint8_t *)chars.release();
  slice.cap = slice.len;
  return slice;
}
