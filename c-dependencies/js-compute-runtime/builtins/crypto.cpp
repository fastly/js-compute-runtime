// TODO: remove these once the warnings are fixed
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-offsetof"
#include "js/experimental/TypedData.h" // used in "js/Conversions.h"
#pragma clang diagnostic pop

#include "crypto.h"

bool is_int_typed_array(JSObject *obj) {
  return JS_IsInt8Array(obj) || JS_IsUint8Array(obj) || JS_IsInt16Array(obj) ||
         JS_IsUint16Array(obj) || JS_IsInt32Array(obj) || JS_IsUint32Array(obj) ||
         JS_IsUint8ClampedArray(obj) || JS_IsBigInt64Array(obj) || JS_IsBigUint64Array(obj);
}

namespace builtins {
#define MAX_BYTE_LENGTH 65536
/**
 * Implementation of
 * https://www.w3.org/TR/WebCryptoAPI/#Crypto-method-getRandomValues
 * TODO: investigate ways to automatically wipe the buffer passed in here when
 * it is GC'd. Content can roughly approximate that using finalizers for views
 * of the buffer, but it's far from ideal.
 */
bool Crypto::get_random_values(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS::CallArgs args = CallArgsFromVp(argc, vp);
  if (!args.requireAtLeast(cx, "crypto.getRandomValues", 1))
    return false;

  if (!args[0].isObject() || !is_int_typed_array(&args[0].toObject())) {
    JS_ReportErrorUTF8(cx, "crypto.getRandomValues: input must be an integer-typed TypedArray");
    return false;
  }

  JS::RootedObject typed_array(cx, &args[0].toObject());
  size_t byte_length = JS_GetArrayBufferViewByteLength(typed_array);
  if (byte_length > MAX_BYTE_LENGTH) {
    JS_ReportErrorUTF8(cx,
                       "crypto.getRandomValues: input byteLength must be at most %u, "
                       "but is %zu",
                       MAX_BYTE_LENGTH, byte_length);
    return false;
  }

  JS::AutoCheckCannotGC noGC(cx);
  bool is_shared;
  void *buffer = JS_GetArrayBufferViewData(typed_array, &is_shared, noGC);
  arc4random_buf(buffer, byte_length);

  args.rval().setObject(*typed_array);
  return true;
}

const JSFunctionSpec Crypto::methods[] = {
    JS_FN("getRandomValues", get_random_values, 1, JSPROP_ENUMERATE), JS_FS_END};

const JSPropertySpec Crypto::properties[] = {JS_PS_END};

bool Crypto::create(JSContext *cx, JS::HandleObject global) {
  JS::RootedObject crypto(cx, JS_NewPlainObject(cx));
  if (!crypto)
    return false;
  if (!JS_DefineProperty(cx, global, "crypto", crypto, JSPROP_ENUMERATE))
    return false;
  return JS_DefineFunctions(cx, crypto, Crypto::methods);
}
} // namespace builtins
