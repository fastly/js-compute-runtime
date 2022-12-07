#include "config-store.h"
#include "host_call.h"

namespace builtins {

fastly_dictionary_handle_t ConfigStore::config_store_handle(JSObject *obj) {
  JS::Value val = JS::GetReservedSlot(obj, ConfigStore::Slots::Handle);
  return static_cast<fastly_dictionary_handle_t>(val.toInt32());
}

bool ConfigStore::get(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(1)

<<<<<<< HEAD
  size_t name_len;
  JS::UniqueChars name = encode(cx, args[0], &name_len);

  // If the converted string has a length of 0 then we throw an Error
  // because Dictionary keys have to be at-least 1 character.
  if (name_len == 0) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_CONFIG_STORE_KEY_EMPTY);
    return false;
  }
  // key has to be less than 256
  if (name_len > 255) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_CONFIG_STORE_KEY_TOO_LONG);
    return false;
  }

  OwnedHostCallBuffer buffer;
  size_t nwritten = 0;
  auto status = convert_to_fastly_status(xqd_dictionary_get(ConfigStore::config_store_handle(self),
                                                            name.get(), name_len, buffer.get(),
                                                            CONFIG_STORE_ENTRY_MAX_LEN, &nwritten));
  // FastlyStatus::none indicates the key wasn't found, so we return null.
  if (status == FastlyStatus::None) {
    args.rval().setNull();
    return true;
  }
=======
  xqd_world_string_t key_str;
  JS::UniqueChars key = encode(cx, args[0], &key_str.len);
  if (!key) {
    return false;
  }
  key_str.ptr = key.get();

  fastly_option_string_t ret;
  auto status = convert_to_fastly_status(
      xqd_fastly_dictionary_get(ConfigStore::config_store_handle(self), &key_str, &ret));
>>>>>>> config store

  // Ensure that we throw an exception for all unexpected host errors.
  if (!HANDLE_RESULT(cx, status))
    return false;

  if (!ret.is_some) {
    args.rval().setNull();
    return true;
  }

  JS::RootedString text(cx, JS_NewStringCopyUTF8N(cx, JS::UTF8Chars(ret.val.ptr, ret.val.len)));
  JS_free(cx, ret.val.ptr);
  if (!text)
    return false;

  args.rval().setString(text);
  return true;
}

const JSFunctionSpec ConfigStore::methods[] = {JS_FN("get", get, 1, JSPROP_ENUMERATE), JS_FS_END};

const JSPropertySpec ConfigStore::properties[] = {JS_PS_END};

bool ConfigStore::constructor(JSContext *cx, unsigned argc, JS::Value *vp) {
  REQUEST_HANDLER_ONLY("The ConfigStore builtin");
  CTOR_HEADER("ConfigStore", 1);

  xqd_world_string_t name_str;
  JS::UniqueChars name_chars = encode(cx, args[0], &name_str.len);
  name_str.ptr = name_chars.get();

  // If the converted string has a length of 0 then we throw an Error
  // because Dictionary names have to be at-least 1 character.
  if (name_str.len == 0) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_CONFIG_STORE_NAME_EMPTY);
    return false;
  }

  // If the converted string has a length of more than 255 then we throw an Error
  // because Dictionary names have to be less than 255 characters.
  if (name_str.len > 255) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_CONFIG_STORE_NAME_TOO_LONG);
    return false;
  }

  std::string_view name(name_str.ptr, name_str.len);

  // Name must start with ascii alphabetical and contain only ascii alphanumeric, underscore, and
  // whitespace
  if (!std::isalpha(name.front())) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr,
                              JSMSG_CONFIG_STORE_NAME_START_WITH_ASCII_ALPHA);
    return false;
  }

  auto is_valid_name = std::all_of(std::next(name.begin(), 1), name.end(), [&](auto character) {
    return std::isalnum(character) || character == '_' || character == ' ';
  });

  if (!is_valid_name) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr,
                              JSMSG_CONFIG_STORE_NAME_CONTAINS_INVALID_CHARACTER);
    return false;
  }

  JS::RootedObject config_store(cx, JS_NewObjectForConstructor(cx, &class_, args));
  fastly_dictionary_handle_t dict_handle = INVALID_HANDLE;
  auto status =
      convert_to_fastly_status(xqd_config_store_open(name.data(), name_len, &dict_handle));
  if (status == FastlyStatus::BadF) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_CONFIG_STORE_DOES_NOT_EXIST,
                              name.data());
    return false;
  }
  if (!HANDLE_RESULT(cx, status)) {
    return false;
  }

  JS::SetReservedSlot(config_store, ConfigStore::Slots::Handle, JS::Int32Value(dict_handle));
  if (!config_store)
    return false;
  args.rval().setObject(*config_store);
  return true;
}

bool ConfigStore::init_class(JSContext *cx, JS::HandleObject global) {
  return BuiltinImpl<ConfigStore>::init_class_impl(cx, global);
}

} // namespace builtins
