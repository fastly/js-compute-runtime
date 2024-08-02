#include <algorithm>
#include <cstring>
#include <iostream>
#include <optional>
#include <string>

// TODO: remove these once the warnings are fixed
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-offsetof"
#pragma clang diagnostic ignored "-Wdeprecated-enum-enum-conversion"
#include "js/experimental/TypedData.h"
#pragma clang diagnostic pop

#include "js/ArrayBuffer.h"
#include "js/Stream.h"

#include "../../../StarlingMonkey/builtins/web/streams/native-stream-source.h"
#include "../../../StarlingMonkey/builtins/web/url.h"
#include "../../../StarlingMonkey/runtime/encode.h"
#include "../host-api/host_api_fastly.h"
#include "./fastly.h"
#include "builtin.h"
#include "kv-store.h"

using builtins::web::streams::NativeStreamSource;
using fastly::fastly::convertBodyInit;
using fastly::fastly::FastlyGetErrorMessage;
using fastly::fetch::RequestOrResponse;

namespace fastly::kv_store {

namespace {

api::Engine *ENGINE;

std::string_view bad_chars{"#?*[]\n\r"};

std::optional<char> find_invalid_character_for_kv_store_key(const char *str) {
  std::optional<char> res;

  std::string_view view{str, strlen(str)};

  auto it = std::find_if(view.begin(), view.end(),
                         [](auto c) { return bad_chars.find(c) != std::string_view::npos; });

  if (it != view.end()) {
    res = *it;
  }

  return res;
}

} // namespace

template <RequestOrResponse::BodyReadResult result_type>
bool KVStoreEntry::bodyAll(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0)
  return RequestOrResponse::bodyAll<result_type>(cx, args, self);
}

bool KVStoreEntry::body_get(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0)
  if (!JS::GetReservedSlot(self, static_cast<uint32_t>(Slots::HasBody)).isBoolean()) {
    JS::SetReservedSlot(self, static_cast<uint32_t>(Slots::HasBody), JS::BooleanValue(false));
  }
  return RequestOrResponse::body_get(cx, args, self, true);
}

bool KVStoreEntry::bodyUsed_get(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0)
  if (!JS::GetReservedSlot(self, static_cast<uint32_t>(Slots::BodyUsed)).isBoolean()) {
    JS::SetReservedSlot(self, static_cast<uint32_t>(Slots::BodyUsed), JS::BooleanValue(false));
  }
  args.rval().setBoolean(RequestOrResponse::body_used(self));
  return true;
}

const JSFunctionSpec KVStoreEntry::static_methods[] = {
    JS_FS_END,
};

const JSPropertySpec KVStoreEntry::static_properties[] = {
    JS_PS_END,
};

const JSFunctionSpec KVStoreEntry::methods[] = {
    JS_FN("arrayBuffer", bodyAll<RequestOrResponse::BodyReadResult::ArrayBuffer>, 0,
          JSPROP_ENUMERATE),
    JS_FN("json", bodyAll<RequestOrResponse::BodyReadResult::JSON>, 0, JSPROP_ENUMERATE),
    JS_FN("text", bodyAll<RequestOrResponse::BodyReadResult::Text>, 0, JSPROP_ENUMERATE),
    JS_FS_END,
};

const JSPropertySpec KVStoreEntry::properties[] = {
    JS_PSG("body", body_get, JSPROP_ENUMERATE),
    JS_PSG("bodyUsed", bodyUsed_get, JSPROP_ENUMERATE),
    JS_PS_END,
};

bool KVStoreEntry::constructor(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS_ReportErrorUTF8(cx, "KVStoreEntry can't be instantiated directly");
  return false;
}

JSObject *KVStoreEntry::create(JSContext *cx, host_api::HttpBody body_handle) {
  JS::RootedObject kvStoreEntry(cx, JS_NewObjectWithGivenProto(cx, &class_, proto_obj));
  if (!kvStoreEntry)
    return nullptr;

  JS::SetReservedSlot(kvStoreEntry, static_cast<uint32_t>(Slots::Body),
                      JS::Int32Value(body_handle.handle));
  JS::SetReservedSlot(kvStoreEntry, static_cast<uint32_t>(Slots::BodyStream), JS::NullValue());
  JS::SetReservedSlot(kvStoreEntry, static_cast<uint32_t>(Slots::HasBody), JS::BooleanValue(true));
  JS::SetReservedSlot(kvStoreEntry, static_cast<uint32_t>(Slots::BodyUsed), JS::FalseValue());

  return kvStoreEntry;
}

namespace {

host_api::ObjectStore kv_store_handle(JSObject *obj) {
  JS::Value val = JS::GetReservedSlot(obj, static_cast<uint32_t>(KVStore::Slots::KVStore));
  return host_api::ObjectStore(val.toInt32());
}

bool parse_and_validate_key(JSContext *cx, const char *key, size_t len) {
  // If the converted string has a length of 0 then we throw an Error
  // because KVStore Keys have to be at-least 1 character.
  if (len == 0) {
    JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr, JSMSG_KV_STORE_KEY_EMPTY);
    return false;
  }

  // If the converted string has a length of more than 1024 then we throw an Error
  // because KVStore Keys have to be less than 1025 characters.
  if (len > 1024) {
    JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr, JSMSG_KV_STORE_KEY_TOO_LONG);
    return false;
  }

  auto key_chars = key;
  auto res = find_invalid_character_for_kv_store_key(key_chars);
  if (res.has_value()) {
    std::string character;
    switch (res.value()) {
    case '\n':
      character = "newline";
      break;
    case '\r':
      character = "carriage return";
      break;
    case '[':
      character = '[';
      break;
    case ']':
      character = ']';
      break;
    case '*':
      character = '*';
      break;
    case '?':
      character = '?';
      break;
    case '#':
      character = '#';
      break;
    }
    JS_ReportErrorNumberUTF8(cx, FastlyGetErrorMessage, nullptr,
                             JSMSG_KV_STORE_KEY_INVALID_CHARACTER, character.c_str());
    return false;
  }
  auto acme_challenge = ".well-known/acme-challenge/";
  if (strncmp(key_chars, acme_challenge, strlen(acme_challenge)) == 0) {
    JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr, JSMSG_KV_STORE_KEY_ACME);
    return false;
  }

  if (strcmp(key_chars, ".") == 0 || strcmp(key_chars, "..") == 0) {
    JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr, JSMSG_KV_STORE_KEY_RELATIVE);
    return false;
  }

  return true;
}

} // namespace

bool KVStore::process_pending_kv_store_delete(JSContext *cx,
                                              host_api::ObjectStorePendingDelete::Handle handle,
                                              JS::HandleObject context, JS::HandleObject promise) {
  host_api::ObjectStorePendingDelete pending_delete(handle);

  auto res = pending_delete.wait();
  if (auto *err = res.to_err()) {
    if (host_api::error_is_invalid_argument(*err)) {
      JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr,
                                JSMSG_KV_STORE_DELETE_KEY_DOES_NOT_EXIST);
    } else {
      HANDLE_ERROR(cx, *err);
    }
    return RejectPromiseWithPendingError(cx, promise);
  }

  JS::ResolvePromise(cx, promise, JS::UndefinedHandleValue);
  return true;
}

bool KVStore::delete_(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER_WITH_NAME(1, "delete");

  JS::RootedObject result_promise(cx, JS::NewPromiseObject(cx, nullptr));
  if (!result_promise) {
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }

  JS::RootedValue key(cx, args.get(0));

  // Convert the key argument into a String following https://tc39.es/ecma262/#sec-tostring
  auto key_chars = core::encode(cx, key);
  if (!key_chars) {
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }

  if (!parse_and_validate_key(cx, key_chars.begin(), key_chars.len)) {
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }

  auto res = kv_store_handle(self).delete_async(key_chars);

  if (auto *err = res.to_err()) {
    HANDLE_ERROR(cx, *err);
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }
  auto handle = res.unwrap();

  ENGINE->queue_async_task(
      new FastlyAsyncTask(handle, self, result_promise, KVStore::process_pending_kv_store_delete));

  args.rval().setObject(*result_promise);
  return true;
}

bool KVStore::process_pending_kv_store_lookup(JSContext *cx,
                                              host_api::ObjectStorePendingLookup::Handle handle,
                                              JS::HandleObject context, JS::HandleObject promise) {
  host_api::ObjectStorePendingLookup pending_lookup(handle);

  auto res = pending_lookup.wait();

  if (auto *err = res.to_err()) {
    HANDLE_ERROR(cx, *err);
    return RejectPromiseWithPendingError(cx, promise);
  }

  auto ret = res.unwrap();

  // When no entry is found, we are going to resolve the Promise with `null`.
  if (!ret.has_value()) {
    JS::RootedValue result(cx);
    result.setNull();
    JS::ResolvePromise(cx, promise, result);
  } else {
    JS::RootedObject entry(cx, KVStoreEntry::create(cx, ret.value()));
    if (!entry) {
      return false;
    }
    JS::RootedValue result(cx);
    result.setObject(*entry);
    JS::ResolvePromise(cx, promise, result);
  }

  return true;
}

bool KVStore::get(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(1)

  JS::RootedObject result_promise(cx, JS::NewPromiseObject(cx, nullptr));
  if (!result_promise) {
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }

  JS::RootedValue key(cx, args.get(0));

  // Convert the key argument into a String following https://tc39.es/ecma262/#sec-tostring
  auto key_chars = core::encode(cx, key);
  if (!key_chars) {
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }

  if (!parse_and_validate_key(cx, key_chars.begin(), key_chars.len)) {
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }

  auto res = kv_store_handle(self).lookup_async(key_chars);

  if (auto *err = res.to_err()) {
    HANDLE_ERROR(cx, *err);
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }
  auto handle = res.unwrap();

  auto task =
      new FastlyAsyncTask(handle, self, result_promise, KVStore::process_pending_kv_store_lookup);
  ENGINE->queue_async_task(task);

  args.rval().setObject(*result_promise);
  return true;
}

bool KVStore::put(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(2)

  JS::RootedObject result_promise(cx, JS::NewPromiseObject(cx, nullptr));
  if (!result_promise) {
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }

  JS::RootedValue key(cx, args.get(0));

  // Convert the key argument into a String following https://tc39.es/ecma262/#sec-tostring
  auto key_chars = core::encode(cx, key);
  if (!key_chars) {
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }

  if (!parse_and_validate_key(cx, key_chars.begin(), key_chars.len)) {
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }

  JS::HandleValue body_val = args.get(1);

  // We currently support five types of body inputs:
  // - byte sequence
  // - buffer source
  // - USV strings
  // - URLSearchParams
  // - ReadableStream
  // After the other other options are checked explicitly, all other inputs are
  // encoded to a UTF8 string to be treated as a USV string.
  // TODO: Support the other possible inputs to Body.

  JS::RootedObject body_obj(cx, body_val.isObject() ? &body_val.toObject() : nullptr);

  if (body_obj && JS::IsReadableStream(body_obj)) {
    if (RequestOrResponse::body_unusable(cx, body_obj)) {
      JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr,
                                JSMSG_READABLE_STREAM_LOCKED_OR_DISTRUBED);
      return ReturnPromiseRejectedWithPendingError(cx, args);
    }

    // If the body stream is backed by a Fastly Compute body handle, we can directly pipe
    // that handle into the kv store.
    if (NativeStreamSource::stream_is_body(cx, body_obj)) {
      JS::RootedObject stream_source(cx, NativeStreamSource::get_stream_source(cx, body_obj));
      JS::RootedObject source_owner(cx, NativeStreamSource::owner(stream_source));
      auto body = RequestOrResponse::body_handle(source_owner);

      auto res = kv_store_handle(self).insert(key_chars, body);
      if (auto *err = res.to_err()) {
        HANDLE_ERROR(cx, *err);
        return ReturnPromiseRejectedWithPendingError(cx, args);
      }

      // The insert was successful so we return a Promise which resolves to undefined
      JS::RootedValue rval(cx);
      rval.setUndefined();
      JS::ResolvePromise(cx, result_promise, rval);
      args.rval().setObject(*result_promise);

      return true;
    } else {
      JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr,
                                JSMSG_KV_STORE_PUT_CONTENT_STREAM);
      return ReturnPromiseRejectedWithPendingError(cx, args);
    }
  } else {
    auto result = convertBodyInit(cx, body_val);
    if (result.isErr()) {
      return false;
    }
    size_t length;
    JS::UniqueChars data;
    std::tie(data, length) = result.unwrap();

    // 30MB in bytes is the max size allowed for KVStore.
    if (length > 30 * 1024 * 1024) {
      JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr, JSMSG_KV_STORE_PUT_OVER_30_MB);
      return ReturnPromiseRejectedWithPendingError(cx, args);
    }

    auto make_res = host_api::HttpBody::make();
    if (auto *err = make_res.to_err()) {
      HANDLE_ERROR(cx, *err);
      return ReturnPromiseRejectedWithPendingError(cx, args);
    }

    auto body = make_res.unwrap();
    if (!body.valid()) {
      return ReturnPromiseRejectedWithPendingError(cx, args);
    }

    auto write_res = body.write_all_back(reinterpret_cast<uint8_t *>(data.get()), length);

    if (auto *err = write_res.to_err()) {
      HANDLE_ERROR(cx, *err);
      return ReturnPromiseRejectedWithPendingError(cx, args);
    }

    auto insert_res = kv_store_handle(self).insert(key_chars, body);
    if (auto *err = insert_res.to_err()) {
      // Ensure that we throw an exception for all unexpected host errors.
      HANDLE_ERROR(cx, *err);
      return RejectPromiseWithPendingError(cx, result_promise);
    }

    // The insert was successful so we return a Promise which resolves to undefined
    JS::RootedValue rval(cx);
    rval.setUndefined();
    JS::ResolvePromise(cx, result_promise, rval);
    args.rval().setObject(*result_promise);

    return true;
  }

  return false;
}

const JSFunctionSpec KVStore::static_methods[] = {
    JS_FS_END,
};

const JSPropertySpec KVStore::static_properties[] = {
    JS_PS_END,
};

const JSFunctionSpec KVStore::methods[] = {
    JS_FN("delete", delete_, 1, JSPROP_ENUMERATE),
    JS_FN("get", get, 1, JSPROP_ENUMERATE),
    JS_FN("put", put, 1, JSPROP_ENUMERATE),
    JS_FS_END,
};

const JSPropertySpec KVStore::properties[] = {
    JS_PS_END,
};

bool KVStore::constructor(JSContext *cx, unsigned argc, JS::Value *vp) {
  REQUEST_HANDLER_ONLY("The KVStore builtin");
  CTOR_HEADER("KVStore", 1);

  JS::HandleValue name_arg = args.get(0);

  // Convert into a String following https://tc39.es/ecma262/#sec-tostring
  auto name = core::encode(cx, name_arg);
  if (!name) {
    return false;
  }

  // If the converted string has a length of 0 then we throw an Error
  // because KVStore names have to be at-least 1 character.
  if (name.len == 0) {
    JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr, JSMSG_KV_STORE_NAME_EMPTY);
    return false;
  }

  // If the converted string has a length of more than 255 then we throw an Error
  // because KVStore names have to be less than 255 characters.
  if (name.len > 255) {
    JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr, JSMSG_KV_STORE_NAME_TOO_LONG);
    return false;
  }

  if (std::any_of(name.begin(), name.end(), [](auto character) {
        return std::iscntrl(static_cast<unsigned char>(character)) != 0;
      })) {
    JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr,
                              JSMSG_KV_STORE_NAME_NO_CONTROL_CHARACTERS);
    return false;
  }

  auto res = host_api::ObjectStore::open(name);
  if (auto *err = res.to_err()) {
    if (host_api::error_is_invalid_argument(*err)) {
      JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr, JSMSG_KV_STORE_DOES_NOT_EXIST,
                                name.begin());
      return false;
    } else {
      HANDLE_ERROR(cx, *err);
      return false;
    }
  }

  JS::RootedObject kv_store(cx, JS_NewObjectForConstructor(cx, &class_, args));
  if (!kv_store) {
    return false;
  }
  JS::SetReservedSlot(kv_store, static_cast<uint32_t>(Slots::KVStore),
                      JS::Int32Value(res.unwrap().handle));
  args.rval().setObject(*kv_store);
  return true;
}

bool install(api::Engine *engine) {
  ENGINE = engine;
  if (!KVStore::init_class_impl(engine->cx(), engine->global())) {
    return false;
  }
  if (!KVStoreEntry::init_class_impl(engine->cx(), engine->global())) {
    return false;
  }
  RootedValue kv_store_val(engine->cx());
  if (!JS_GetProperty(engine->cx(), engine->global(), "KVStore", &kv_store_val)) {
    return false;
  }
  RootedObject kv_store_ns(engine->cx(), JS_NewObject(engine->cx(), nullptr));
  if (!JS_SetProperty(engine->cx(), kv_store_ns, "KVStore", kv_store_val)) {
    return false;
  }
  RootedValue kv_store_ns_val(engine->cx(), JS::ObjectValue(*kv_store_ns));
  if (!engine->define_builtin_module("fastly:kv-store", kv_store_ns_val)) {
    return false;
  }

  return true;
}

} // namespace fastly::kv_store
