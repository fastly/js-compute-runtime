#include "secret-store.h"
#include "host_interface/host_call.h"

namespace builtins {

fastly_secret_handle_t SecretStoreEntry::secret_handle(JSObject *obj) {
  JS::Value val = JS::GetReservedSlot(obj, SecretStoreEntry::Slots::Handle);
  return static_cast<fastly_secret_handle_t>(val.toInt32());
}

bool SecretStoreEntry::plaintext(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0)

  fastly_option_string_t ret;
  fastly_error_t err;
  // Ensure that we throw an exception for all unexpected host errors.
  if (!fastly_secret_store_plaintext(SecretStoreEntry::secret_handle(self), &ret, &err)) {
    HANDLE_ERROR(cx, err);
    return false;
  }

  JS::RootedString text(cx, JS_NewStringCopyUTF8N(cx, JS::UTF8Chars(ret.val.ptr, ret.val.len)));
  JS_free(cx, ret.val.ptr);
  if (!text) {
    return false;
  }

  args.rval().setString(text);
  return true;
}

const JSFunctionSpec SecretStoreEntry::static_methods[] = {
    JS_FS_END,
};

const JSPropertySpec SecretStoreEntry::static_properties[] = {
    JS_PS_END,
};

const JSFunctionSpec SecretStoreEntry::methods[] = {
    JS_FN("plaintext", plaintext, 0, JSPROP_ENUMERATE), JS_FS_END};

const JSPropertySpec SecretStoreEntry::properties[] = {JS_PS_END};

bool SecretStoreEntry::constructor(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS_ReportErrorUTF8(cx, "SecretStoreEntry can't be instantiated directly");
  return false;
}

JSObject *SecretStoreEntry::create(JSContext *cx, fastly_secret_handle_t handle) {
  JS::RootedObject entry(
      cx, JS_NewObjectWithGivenProto(cx, &SecretStoreEntry::class_, SecretStoreEntry::proto_obj));
  if (!entry) {
    return nullptr;
  }

  JS::SetReservedSlot(entry, Slots::Handle, JS::Int32Value(handle));

  return entry;
}

bool SecretStoreEntry::init_class(JSContext *cx, JS::HandleObject global) {
  return BuiltinImpl<SecretStoreEntry>::init_class_impl(cx, global);
}

fastly_secret_store_handle_t SecretStore::secret_store_handle(JSObject *obj) {
  JS::Value val = JS::GetReservedSlot(obj, SecretStore::Slots::Handle);
  return static_cast<fastly_secret_store_handle_t>(val.toInt32());
}

bool SecretStore::get(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(1)

  JS::RootedObject result_promise(cx, JS::NewPromiseObject(cx, nullptr));
  if (!result_promise) {
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }

  size_t length;
  JS::UniqueChars key = encode(cx, args[0], &length);
  if (!key) {
    return false;
  }
  // If the converted string has a length of 0 then we throw an Error
  // because keys have to be at-least 1 character.
  if (length == 0) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_SECRET_STORE_KEY_EMPTY);
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }

  // key has to be less than 256
  if (length > 255) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_SECRET_STORE_KEY_TOO_LONG);
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }

  std::string_view key_view(key.get(), length);

  // key must contain only letters, numbers, dashes (-), underscores (_), and periods (.).
  auto is_valid_key = std::all_of(key_view.begin(), key_view.end(), [&](auto character) {
    return std::isalnum(character) || character == '_' || character == '-' || character == '.';
  });

  if (!is_valid_key) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr,
                              JSMSG_SECRET_STORE_KEY_CONTAINS_INVALID_CHARACTER);
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }

  fastly_world_string_t key_str;
  key_str.len = length;
  key_str.ptr = key.get();
  fastly_option_secret_handle_t secret;
  fastly_error_t err;
  // Ensure that we throw an exception for all unexpected host errors.
  if (!fastly_secret_store_get(SecretStore::secret_store_handle(self), &key_str, &secret, &err)) {
    HANDLE_ERROR(cx, err);
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }

  // When no entry is found, we are going to resolve the Promise with `null`.
  if (!secret.is_some) {
    JS::RootedValue result(cx);
    result.setNull();
    JS::ResolvePromise(cx, result_promise, result);
  } else {
    JS::RootedObject entry(cx, SecretStoreEntry::create(cx, secret.val));
    if (!entry) {
      return ReturnPromiseRejectedWithPendingError(cx, args);
    }
    JS::RootedValue result(cx);
    result.setObject(*entry);
    JS::ResolvePromise(cx, result_promise, result);
  }

  args.rval().setObject(*result_promise);

  return true;
}

const JSFunctionSpec SecretStore::static_methods[] = {
    JS_FS_END,
};

const JSPropertySpec SecretStore::static_properties[] = {
    JS_PS_END,
};

const JSFunctionSpec SecretStore::methods[] = {JS_FN("get", get, 1, JSPROP_ENUMERATE), JS_FS_END};

const JSPropertySpec SecretStore::properties[] = {JS_PS_END};

bool SecretStore::constructor(JSContext *cx, unsigned argc, JS::Value *vp) {
  REQUEST_HANDLER_ONLY("The SecretStore builtin");
  CTOR_HEADER("SecretStore", 1);

  size_t length;
  JS::UniqueChars name_chars = encode(cx, args[0], &length);
  if (!name_chars) {
    return false;
  }

  // If the converted string has a length of 0 then we throw an Error
  // because names have to be at-least 1 character.
  if (length == 0) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_SECRET_STORE_NAME_EMPTY);
    return false;
  }

  // If the converted string has a length of more than 255 then we throw an Error
  // because names have to be less than 255 characters.
  if (length > 255) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_SECRET_STORE_NAME_TOO_LONG);
    return false;
  }

  std::string_view name(name_chars.get(), length);

  // Name must contain only letters, numbers, dashes (-), underscores (_), and periods (.).
  auto is_valid_name = std::all_of(name.begin(), name.end(), [&](auto character) {
    return std::isalnum(character) || character == '_' || character == '-' || character == '.';
  });

  if (!is_valid_name) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr,
                              JSMSG_SECRET_STORE_NAME_CONTAINS_INVALID_CHARACTER);
    return false;
  }

  JS::RootedObject secret_store(cx, JS_NewObjectForConstructor(cx, &class_, args));
  if (!secret_store) {
    return false;
  }
  fastly_world_string_t name_str;
  name_str.ptr = name_chars.get();
  name_str.len = length;
  fastly_secret_store_handle_t handle = INVALID_HANDLE;
  fastly_error_t err;
  if (!fastly_secret_store_open(&name_str, &handle, &err)) {
    if (err == FASTLY_ERROR_OPTIONAL_NONE) {
      JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_SECRET_STORE_DOES_NOT_EXIST,
                                name.data());
      return false;
    } else {
      HANDLE_ERROR(cx, err);
      return false;
    }
  }

  JS::SetReservedSlot(secret_store, SecretStore::Slots::Handle, JS::Int32Value(handle));
  args.rval().setObject(*secret_store);
  return true;
}

bool SecretStore::init_class(JSContext *cx, JS::HandleObject global) {
  return BuiltinImpl<SecretStore>::init_class_impl(cx, global);
}

} // namespace builtins
