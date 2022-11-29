#include "dictionary.h"
#include "host_call.h"

namespace builtins {

fastly_dictionary_handle_t Dictionary::dictionary_handle(JSObject *obj) {
  JS::Value val = JS::GetReservedSlot(obj, Dictionary::Slots::Handle);
  return fastly_dictionary_handle_t{static_cast<uint32_t>(val.toInt32())};
}

bool Dictionary::get(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(1)

  JS::HandleValue name_arg = args.get(0);

  size_t name_len;
  // Convert into a String following https://tc39.es/ecma262/#sec-tostring
  JS::UniqueChars name_chars = encode(cx, name_arg, &name_len);
  if (!name_chars) {
    return false;
  }

  // If the converted string has a length of 0 then we throw an Error
  // because Dictionary keys have to be at-least 1 character.
  if (name_len == 0) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_DICTIONARY_KEY_EMPTY);
    return false;
  }
  // key has to be less than 256
  if (name_len > 255) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_DICTIONARY_KEY_TOO_LONG);
    return false;
  }

  OwnedHostCallBuffer buffer;
  size_t nwritten = 0;
  auto status = convert_to_fastly_status(
      xqd_dictionary_get(Dictionary::dictionary_handle(self), name_chars.get(), name_len,
                         buffer.get(), DICTIONARY_ENTRY_MAX_LEN, &nwritten));
  // FastlyStatus::none indicates the key wasn't found, so we return null.
  if (status == FastlyStatus::None) {
    args.rval().setNull();
    return true;
  }

  // Ensure that we throw an exception for all unexpected host errors.
  if (!HANDLE_RESULT(cx, status))
    return false;

  JS::RootedString text(cx, JS_NewStringCopyUTF8N(cx, JS::UTF8Chars(buffer.get(), nwritten)));
  if (!text)
    return false;

  args.rval().setString(text);
  return true;
}

const JSFunctionSpec Dictionary::methods[] = {JS_FN("get", get, 1, JSPROP_ENUMERATE), JS_FS_END};

const JSPropertySpec Dictionary::properties[] = {JS_PS_END};

bool Dictionary::constructor(JSContext *cx, unsigned argc, JS::Value *vp) {
  REQUEST_HANDLER_ONLY("The Dictionary builtin");
  CTOR_HEADER("Dictionary", 1);

  JS::HandleValue name_arg = args.get(0);

  size_t name_len;
  // Convert into a String following https://tc39.es/ecma262/#sec-tostring
  JS::UniqueChars name_chars = encode(cx, name_arg, &name_len);
  if (!name_chars) {
    return false;
  }

  // If the converted string has a length of 0 then we throw an Error
  // because Dictionary names have to be at-least 1 character.
  if (name_len == 0) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_DICTIONARY_NAME_EMPTY);
    return false;
  }

  // If the converted string has a length of more than 255 then we throw an Error
  // because Dictionary names have to be less than 255 characters.
  if (name_len > 255) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_DICTIONARY_NAME_TOO_LONG);
    return false;
  }

  std::string_view name(name_chars.get(), name_len);

  // Name must start with ascii alphabetical and contain only ascii alphanumeric, underscore, and
  // whitespace
  if (!std::isalpha(name.front())) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr,
                              JSMSG_DICTIONARY_NAME_START_WITH_ASCII_ALPHA);
    return false;
  }

  auto is_valid_name = std::all_of(std::next(name.begin(), 1), name.end(), [&](auto character) {
    return std::isalnum(character) || character == '_' || character == ' ';
  });

  if (!is_valid_name) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr,
                              JSMSG_DICTIONARY_NAME_CONTAINS_INVALID_CHARACTER);
    return false;
  }
  JS::RootedObject dictionary(cx, JS_NewObjectForConstructor(cx, &class_, args));
  fastly_dictionary_handle_t dict_handle = INVALID_HANDLE;
  auto status = convert_to_fastly_status(xqd_dictionary_open(name.data(), name_len, &dict_handle));
  if (status == FastlyStatus::BadF) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_DICTIONARY_DOES_NOT_EXIST,
                              name.data());
    return false;
  }
  if (!HANDLE_RESULT(cx, status)) {
    return false;
  }

  JS::SetReservedSlot(dictionary, Dictionary::Slots::Handle, JS::Int32Value(dict_handle));
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
