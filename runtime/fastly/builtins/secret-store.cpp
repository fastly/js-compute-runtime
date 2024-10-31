#include "secret-store.h"
#include "../../../StarlingMonkey/runtime/encode.h"
#include "../common/validations.h"
#include "../host-api/host_api_fastly.h"
#include "fastly.h"

using fastly::common::validate_bytes;
using fastly::fastly::FastlyGetErrorMessage;

namespace fastly::secret_store {

host_api::Secret SecretStoreEntry::secret_handle(JSObject *obj) {
  JS::Value val = JS::GetReservedSlot(obj, SecretStoreEntry::Slots::Handle);
  return host_api::Secret(val.toInt32());
}

bool SecretStoreEntry::plaintext(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0)

  // Ensure that we throw an exception for all unexpected host errors.
  auto res = SecretStoreEntry::secret_handle(self).plaintext();
  if (auto *err = res.to_err()) {
    HANDLE_ERROR(cx, *err);
    return false;
  }

  auto ret = std::move(res.unwrap());
  if (!ret.has_value()) {
    return false;
  }

  JS::RootedString text(
      cx,
      JS_NewStringCopyUTF8N(cx, JS::UTF8Chars(reinterpret_cast<char *>(ret->ptr.get()), ret->len)));
  if (!text) {
    return false;
  }

  args.rval().setString(text);
  return true;
}

bool SecretStoreEntry::raw_bytes(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0)

  // Ensure that we throw an exception for all unexpected host errors.
  auto res = SecretStoreEntry::secret_handle(self).plaintext();
  if (auto *err = res.to_err()) {
    HANDLE_ERROR(cx, *err);
    return false;
  }

  auto ret = std::move(res.unwrap());
  if (!ret.has_value()) {
    return false;
  }

  JS::RootedObject array_buffer(
      cx, JS::NewArrayBufferWithContents(cx, ret->len, ret->ptr.get(),
                                         JS::NewArrayBufferOutOfMemory::CallerMustFreeMemory));
  if (!array_buffer) {
    JS_ReportOutOfMemory(cx);
    return false;
  }

  // `array_buffer` now owns `metadata`
  static_cast<void>(ret->ptr.release());

  JS::RootedObject uint8_array(cx, JS_NewUint8ArrayWithBuffer(cx, array_buffer, 0, ret->len));

  args.rval().setObject(*uint8_array);

  return true;
}

const JSFunctionSpec SecretStoreEntry::static_methods[] = {
    JS_FS_END,
};

const JSPropertySpec SecretStoreEntry::static_properties[] = {
    JS_PS_END,
};

const JSFunctionSpec SecretStoreEntry::methods[] = {
    JS_FN("plaintext", plaintext, 0, JSPROP_ENUMERATE),
    JS_FN("rawBytes", raw_bytes, 0, JSPROP_ENUMERATE), JS_FS_END};

const JSPropertySpec SecretStoreEntry::properties[] = {JS_PS_END};

bool SecretStoreEntry::constructor(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS_ReportErrorUTF8(cx, "SecretStoreEntry can't be instantiated directly");
  return false;
}

JSObject *SecretStoreEntry::create(JSContext *cx, host_api::Secret secret) {
  JS::RootedObject entry(
      cx, JS_NewObjectWithGivenProto(cx, &SecretStoreEntry::class_, SecretStoreEntry::proto_obj));
  if (!entry) {
    return nullptr;
  }

  JS::SetReservedSlot(entry, Slots::Handle, JS::Int32Value(secret.handle));

  return entry;
}

host_api::SecretStore SecretStore::secret_store_handle(JSObject *obj) {
  JS::Value val = JS::GetReservedSlot(obj, SecretStore::Slots::Handle);
  return host_api::SecretStore(val.toInt32());
}

bool SecretStore::get(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(1)

  JS::RootedObject result_promise(cx, JS::NewPromiseObject(cx, nullptr));
  if (!result_promise) {
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }

  auto key = core::encode(cx, args[0]);
  if (!key) {
    return false;
  }
  // If the converted string has a length of 0 then we throw an Error
  // because keys have to be at-least 1 character.
  if (key.len == 0) {
    JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr, JSMSG_SECRET_STORE_KEY_EMPTY);
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }

  // key has to be less than 256
  if (key.len > 255) {
    JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr, JSMSG_SECRET_STORE_KEY_TOO_LONG);
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }

  // key must contain only letters, numbers, dashes (-), underscores (_), and periods (.).
  auto is_valid_key = std::all_of(key.begin(), key.end(), [&](auto character) {
    return std::isalnum(character) || character == '_' || character == '-' || character == '.';
  });

  if (!is_valid_key) {
    JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr,
                              JSMSG_SECRET_STORE_KEY_CONTAINS_INVALID_CHARACTER);
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }

  // Ensure that we throw an exception for all unexpected host errors.
  auto get_res = SecretStore::secret_store_handle(self).get(key);
  if (auto *err = get_res.to_err()) {
    HANDLE_ERROR(cx, *err);
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }

  // When no entry is found, we are going to resolve the Promise with `null`.
  auto secret = get_res.unwrap();
  if (!secret.has_value()) {
    JS::RootedValue result(cx);
    result.setNull();
    JS::ResolvePromise(cx, result_promise, result);
  } else {
    JS::RootedObject entry(cx, SecretStoreEntry::create(cx, *secret));
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

bool SecretStore::from_bytes(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  if (!args.requireAtLeast(cx, "SecretStore.fromBytes", 1)) {
    return false;
  }

  JS::RootedObject entry(
      cx, JS_NewObjectWithGivenProto(cx, &SecretStoreEntry::class_, SecretStoreEntry::proto_obj));
  if (!entry) {
    return false;
  }

  auto bytes = args.get(0);

  auto maybe_byte_data = validate_bytes(cx, bytes, "SecretStore.fromBytes", false);
  if (!maybe_byte_data) {
    return false;
  }
  // important no work is done here before the host call so our buffer doesn't move
  const uint8_t *data;
  size_t len;
  std::tie(data, len) = maybe_byte_data.value();
  auto res = host_api::SecretStore::from_bytes(data, len);
  if (auto *err = res.to_err()) {
    HANDLE_ERROR(cx, *err);
    return false;
  }

  JS::SetReservedSlot(entry, Slots::Handle, JS::Int32Value(res.unwrap().handle));
  args.rval().setObject(*entry);
  return true;
}

const JSFunctionSpec SecretStore::static_methods[] = {
    JS_FN("fromBytes", from_bytes, 1, JSPROP_ENUMERATE),
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

  auto name = core::encode(cx, args[0]);
  if (!name) {
    return false;
  }

  // If the converted string has a length of 0 then we throw an Error
  // because names have to be at-least 1 character.
  if (name.len == 0) {
    JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr, JSMSG_SECRET_STORE_NAME_EMPTY);
    return false;
  }

  // If the converted string has a length of more than 255 then we throw an Error
  // because names have to be less than 255 characters.
  if (name.len > 255) {
    JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr, JSMSG_SECRET_STORE_NAME_TOO_LONG);
    return false;
  }

  // Name must contain only letters, numbers, dashes (-), underscores (_), and periods (.).
  auto is_valid_name = std::all_of(name.begin(), name.end(), [&](auto character) {
    return std::isalnum(character) || character == '_' || character == '-' || character == '.';
  });

  if (!is_valid_name) {
    JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr,
                              JSMSG_SECRET_STORE_NAME_CONTAINS_INVALID_CHARACTER);
    return false;
  }

  JS::RootedObject secret_store(cx, JS_NewObjectForConstructor(cx, &class_, args));
  if (!secret_store) {
    return false;
  }

  auto res = host_api::SecretStore::open(name);
  if (auto *err = res.to_err()) {
    if (host_api::error_is_optional_none(*err)) {
      JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr,
                                JSMSG_SECRET_STORE_DOES_NOT_EXIST, name.begin());
      return false;
    } else {
      HANDLE_ERROR(cx, *err);
      return false;
    }
  }

  JS::SetReservedSlot(secret_store, SecretStore::Slots::Handle,
                      JS::Int32Value(res.unwrap().handle));
  args.rval().setObject(*secret_store);
  return true;
}

bool install(api::Engine *engine) {
  if (!SecretStoreEntry::init_class_impl(engine->cx(), engine->global())) {
    return false;
  }
  if (!SecretStore::init_class_impl(engine->cx(), engine->global())) {
    return false;
  }

  RootedObject secret_store_ns_obj(engine->cx(), JS_NewObject(engine->cx(), nullptr));
  RootedValue secret_store_ns_val(engine->cx(), JS::ObjectValue(*secret_store_ns_obj));
  RootedObject secret_store_obj(engine->cx(),
                                JS_GetConstructor(engine->cx(), SecretStore::proto_obj));
  RootedValue secret_store_val(engine->cx(), ObjectValue(*secret_store_obj));
  if (!JS_SetProperty(engine->cx(), secret_store_ns_obj, "SecretStore", secret_store_val)) {
    return false;
  }
  RootedObject secret_store_entry_obj(engine->cx(),
                                      JS_GetConstructor(engine->cx(), SecretStoreEntry::proto_obj));
  RootedValue secret_store_entry_val(engine->cx(), ObjectValue(*secret_store_entry_obj));
  if (!JS_SetProperty(engine->cx(), secret_store_ns_obj, "SecretStoreEntry",
                      secret_store_entry_val)) {
    return false;
  }
  if (!engine->define_builtin_module("fastly:secret-store", secret_store_ns_val)) {
    return false;
  }

  return true;
}

} // namespace fastly::secret_store
