#include "config-store.h"
#include "core/encode.h"
#include "host_interface/host_api.h"

namespace builtins {

host_api::ConfigStore ConfigStore::config_store_handle(JSObject *obj) {
  JS::Value val = JS::GetReservedSlot(obj, ConfigStore::Slots::Handle);
  return host_api::ConfigStore(val.toInt32());
}

bool ConfigStore::get(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(1)

  auto key = core::encode(cx, args[0]);
  // If the converted string has a length of 0 then we throw an Error
  // because config-store keys have to be at-least 1 character.
  if (!key || key.len == 0) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_CONFIG_STORE_KEY_EMPTY);
    return false;
  }

  // key has to be less than 256
  if (key.len > 255) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_CONFIG_STORE_KEY_TOO_LONG);
    return false;
  }

  std::string_view key_str = key;
  // Ensure that we throw an exception for all unexpected host errors.
  auto get_res = ConfigStore::config_store_handle(self).get(key_str);
  if (auto *err = get_res.to_err()) {
    HANDLE_ERROR(cx, *err);
    return false;
  }

  // None indicates the key wasn't found, so we return null.
  auto ret = std::move(get_res.unwrap());
  if (!ret.has_value()) {
    args.rval().setNull();
    return true;
  }

  JS::RootedString text(cx, JS_NewStringCopyUTF8N(cx, JS::UTF8Chars(ret->begin(), ret->size())));
  if (!text) {
    return false;
  }

  args.rval().setString(text);
  return true;
}

const JSFunctionSpec ConfigStore::static_methods[] = {
    JS_FS_END,
};

const JSPropertySpec ConfigStore::static_properties[] = {
    JS_PS_END,
};

const JSFunctionSpec ConfigStore::methods[] = {JS_FN("get", get, 1, JSPROP_ENUMERATE), JS_FS_END};

const JSPropertySpec ConfigStore::properties[] = {JS_PS_END};

bool ConfigStore::constructor(JSContext *cx, unsigned argc, JS::Value *vp) {
  REQUEST_HANDLER_ONLY("The ConfigStore builtin");
  CTOR_HEADER("ConfigStore", 1);

  auto name = core::encode(cx, args[0]);

  // If the converted string has a length of 0 then we throw an Error
  // because config-store names have to be at-least 1 character.
  if (!name) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_CONFIG_STORE_NAME_EMPTY);
    return false;
  }

  // If the converted string has a length of more than 255 then we throw an Error
  // because config-store names have to be less than 255 characters.
  if (name.size() > 255) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_CONFIG_STORE_NAME_TOO_LONG);
    return false;
  }

  // Name must start with ascii alphabetical and contain only ascii alphanumeric, underscore, and
  // whitespace
  if (!std::isalpha(*name.begin())) {
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
  auto open_res = host_api::ConfigStore::open(name);
  if (auto *err = open_res.to_err()) {
    if (host_api::error_is_bad_handle(*err)) {
      JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_CONFIG_STORE_DOES_NOT_EXIST,
                                name.begin());
      return false;
    } else {
      HANDLE_ERROR(cx, *err);
      return false;
    }
  }

  JS::SetReservedSlot(config_store, ConfigStore::Slots::Handle,
                      JS::Int32Value(open_res.unwrap().handle));
  if (!config_store) {
    return false;
  }

  args.rval().setObject(*config_store);
  return true;
}

bool ConfigStore::init_class(JSContext *cx, JS::HandleObject global) {
  return BuiltinImpl<ConfigStore>::init_class_impl(cx, global);
}

} // namespace builtins
