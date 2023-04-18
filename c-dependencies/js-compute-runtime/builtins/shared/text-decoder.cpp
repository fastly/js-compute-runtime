#include "builtins/shared/text-decoder.h"
#include "builtin.h"

namespace builtins {

bool TextDecoder::decode(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(1)

  // Default to empty string if no input is given.
  if (args[0].isUndefined()) {
    args.rval().set(JS_GetEmptyStringValue(cx));
    return true;
  }

  auto data = value_to_buffer(cx, args[0], "TextDecoder#decode: input");
  if (!data.has_value()) {
    return false;
  }

  JS::RootedString str(
      cx, JS_NewStringCopyUTF8N(cx, JS::UTF8Chars((char *)data->data(), data->size())));
  if (!str)
    return false;

  args.rval().setString(str);
  return true;
}

bool TextDecoder::encoding_get(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0)

  JS::RootedString str(cx, JS_NewStringCopyN(cx, "utf-8", 5));
  if (!str)
    return false;

  args.rval().setString(str);
  return true;
}

const JSFunctionSpec TextDecoder::static_methods[] = {
    JS_FS_END,
};

const JSPropertySpec TextDecoder::static_properties[] = {
    JS_PS_END,
};

const JSFunctionSpec TextDecoder::methods[] = {
    JS_FN("decode", decode, 1, JSPROP_ENUMERATE),
    JS_FS_END,
};

const JSPropertySpec TextDecoder::properties[] = {
    JS_PSG("encoding", encoding_get, JSPROP_ENUMERATE),
    JS_STRING_SYM_PS(toStringTag, "TextDecoder", JSPROP_READONLY),
    JS_PS_END,
};

bool TextDecoder::constructor(JSContext *cx, unsigned argc, JS::Value *vp) {
  CTOR_HEADER("TextDecoder", 0);

  JS::RootedObject self(cx, JS_NewObjectForConstructor(cx, &class_, args));

  args.rval().setObject(*self);
  return true;
}

bool TextDecoder::init_class(JSContext *cx, JS::HandleObject global) {
  return init_class_impl(cx, global);
}

} // namespace builtins
