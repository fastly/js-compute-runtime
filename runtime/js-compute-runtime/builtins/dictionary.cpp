#include "dictionary.h"
#include "core/encode.h"
#include "host_interface/host_api.h"

namespace builtins {

host_api::Dict Dictionary::dictionary_handle(JSObject *obj) {
  JS::Value val = JS::GetReservedSlot(obj, Dictionary::Slots::Handle);
  return host_api::Dict(val.toInt32());
}

bool Dictionary::get(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(1)

  JS::HandleValue name_arg = args.get(0);

  // Convert into a String following https://tc39.es/ecma262/#sec-tostring
  auto name = core::encode(cx, name_arg);
  if (!name) {
    return false;
  }

  // If the converted string has a length of 0 then we throw an Error
  // because Dictionary keys have to be at-least 1 character.
  if (name.len == 0) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_DICTIONARY_KEY_EMPTY);
    return false;
  }
  // key has to be less than 256
  if (name.len > 255) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_DICTIONARY_KEY_TOO_LONG);
    return false;
  }

  // Ensure that we throw an exception for all unexpected host errors.
  auto res = Dictionary::dictionary_handle(self).get(name);
  if (auto *err = res.to_err()) {
    HANDLE_ERROR(cx, *err);
    return false;
  }

  auto ret = std::move(res.unwrap());
  if (!ret.has_value()) {
    args.rval().setNull();
    return true;
  }

  JS::RootedString text(cx, JS_NewStringCopyUTF8N(cx, JS::UTF8Chars(ret->ptr.get(), ret->len)));
  if (!text)
    return false;

  args.rval().setString(text);
  return true;
}

const JSFunctionSpec Dictionary::static_methods[] = {
    JS_FS_END,
};

const JSPropertySpec Dictionary::static_properties[] = {
    JS_PS_END,
};

const JSFunctionSpec Dictionary::methods[] = {JS_FN("get", get, 1, JSPROP_ENUMERATE), JS_FS_END};

const JSPropertySpec Dictionary::properties[] = {JS_PS_END};

bool Dictionary::constructor(JSContext *cx, unsigned argc, JS::Value *vp) {
  REQUEST_HANDLER_ONLY("The Dictionary builtin");
  CTOR_HEADER("Dictionary", 1);

  JS::HandleValue name_arg = args.get(0);

  // Convert into a String following https://tc39.es/ecma262/#sec-tostring
  auto name = core::encode(cx, name_arg);
  if (!name) {
    return false;
  }

  // If the converted string has a length of 0 then we throw an Error
  // because Dictionary names have to be at-least 1 character.
  if (name.len == 0) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_DICTIONARY_NAME_EMPTY);
    return false;
  }

  // If the converted string has a length of more than 255 then we throw an Error
  // because Dictionary names have to be less than 255 characters.
  if (name.len > 255) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_DICTIONARY_NAME_TOO_LONG);
    return false;
  }

  // Name must start with ascii alphabetical and contain only ascii alphanumeric, underscore, and
  // whitespace
  std::string_view name_view = name;
  if (!std::isalpha(name_view.front())) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr,
                              JSMSG_DICTIONARY_NAME_START_WITH_ASCII_ALPHA);
    return false;
  }

  auto is_valid_name =
      std::all_of(std::next(name_view.begin(), 1), name_view.end(), [&](auto character) {
        return std::isalnum(character) || character == '_' || character == ' ';
      });

  if (!is_valid_name) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr,
                              JSMSG_DICTIONARY_NAME_CONTAINS_INVALID_CHARACTER);
    return false;
  }

  JS::RootedObject dictionary(cx, JS_NewObjectForConstructor(cx, &class_, args));

  auto res = host_api::Dict::open(name_view);
  if (auto *err = res.to_err()) {
    if (host_api::error_is_bad_handle(*err)) {
      JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_DICTIONARY_DOES_NOT_EXIST,
                                name_view.data());
      return false;
    } else {
      HANDLE_ERROR(cx, *err);
      return false;
    }
  }

  auto dict = res.unwrap();
  JS::SetReservedSlot(dictionary, Dictionary::Slots::Handle, JS::Int32Value(dict.handle));
  if (!dictionary) {
    return false;
  }

  args.rval().setObject(*dictionary);
  return true;
}

bool Dictionary::init_class(JSContext *cx, JS::HandleObject global) {
  return BuiltinImpl<Dictionary>::init_class_impl(cx, global);
}

} // namespace builtins
