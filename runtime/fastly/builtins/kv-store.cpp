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

#include "../../../StarlingMonkey/builtins/web/base64.h"
#include "../../../StarlingMonkey/builtins/web/streams/native-stream-source.h"
#include "../../../StarlingMonkey/builtins/web/url.h"
#include "../common/validations.h"
#include "../host-api/host_api_fastly.h"
#include "./fastly.h"
#include "builtin.h"
#include "decode.h"
#include "encode.h"
#include "js/JSON.h"
#include "kv-store.h"

using builtins::web::streams::NativeStreamSource;
using fastly::common::parse_and_validate_timeout;
using fastly::common::validate_bytes;
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

bool KVStoreEntry::metadata(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0)
  args.rval().set(JS::GetReservedSlot(self, static_cast<uint32_t>(Slots::Metadata)));
  return true;
}

bool KVStoreEntry::metadata_text(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0)
  JS::RootedValue metadata(cx, JS::GetReservedSlot(self, static_cast<uint32_t>(Slots::Metadata)));
  if (metadata.isNull()) {
    args.rval().setNull();
    return true;
  }
  uint8_t *data;
  size_t len;
  bool is_shared;
  if (!JS_GetObjectAsArrayBufferView(&metadata.toObject(), &len, &is_shared, &data)) {
    MOZ_ASSERT(false);
  }
  MOZ_ASSERT(!is_shared);
  JS::RootedString metadata_str(
      cx, core::decode(cx, std::string_view(reinterpret_cast<char *>(data), len)));
  if (!metadata_str) {
    return false;
  }
  args.rval().setString(metadata_str);
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
    JS_FN("metadata", metadata, 0, JSPROP_ENUMERATE),
    JS_FN("metadataText", metadata_text, 0, JSPROP_ENUMERATE),
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

JSObject *KVStoreEntry::create(JSContext *cx, host_api::HttpBody body_handle,
                               host_api::HostBytes metadata) {
  JS::RootedObject kvStoreEntry(cx, JS_NewObjectWithGivenProto(cx, &class_, proto_obj));
  if (!kvStoreEntry)
    return nullptr;

  JS::SetReservedSlot(kvStoreEntry, static_cast<uint32_t>(Slots::Body),
                      JS::Int32Value(body_handle.handle));
  JS::SetReservedSlot(kvStoreEntry, static_cast<uint32_t>(Slots::BodyStream), JS::NullValue());
  JS::SetReservedSlot(kvStoreEntry, static_cast<uint32_t>(Slots::HasBody), JS::BooleanValue(true));
  JS::SetReservedSlot(kvStoreEntry, static_cast<uint32_t>(Slots::BodyUsed), JS::FalseValue());
  if (metadata) {
    JS::RootedObject buffer(
        cx, JS::NewArrayBufferWithContents(cx, metadata.len, metadata.ptr.get(),
                                           JS::NewArrayBufferOutOfMemory::CallerMustFreeMemory));
    if (!buffer) {
      JS_ReportOutOfMemory(cx);
      return nullptr;
    }

    // `array_buffer` now owns `metadata`
    static_cast<void>(metadata.ptr.release());

    JS::RootedObject uint8_array(cx, JS_NewUint8ArrayWithBuffer(cx, buffer, 0, metadata.len));

    JS::SetReservedSlot(kvStoreEntry, static_cast<uint32_t>(Slots::Metadata),
                        JS::ObjectValue(*uint8_array));
  } else {
    JS::SetReservedSlot(kvStoreEntry, static_cast<uint32_t>(Slots::Metadata), JS::NullValue());
  }
  return kvStoreEntry;
}

namespace {

host_api::KVStore kv_store(JSObject *obj) {
  JS::Value val = JS::GetReservedSlot(obj, static_cast<uint32_t>(KVStore::Slots::KVStore));
  return host_api::KVStore(val.toInt32());
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

constexpr size_t HANDLE_READ_CHUNK_SIZE = 8192;
constexpr size_t HANDLE_READ_BUFFER_SIZE = 500000;

bool process_pending_kv_store_list(JSContext *cx, host_api::KVStorePendingList::Handle handle,
                                   JS::HandleObject context, JS::HandleObject promise) {
  host_api::KVStorePendingList pending_list(handle);

  auto res = pending_list.wait();
  if (auto *err = res.to_err()) {
    std::string message = std::move(err->message()).value_or("when attempting to fetch resource.");
    JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr, JSMSG_KV_STORE_LIST_ERROR,
                              message.c_str());
    return RejectPromiseWithPendingError(cx, promise);
  }

  size_t buf_len = 0;
  char *buf = static_cast<char *>(malloc(HANDLE_READ_BUFFER_SIZE));
  do {
    host_api::Result<size_t> chunk =
        res.unwrap().read_into(reinterpret_cast<uint8_t *>(buf + buf_len), HANDLE_READ_CHUNK_SIZE);
    if (auto *err = chunk.to_err()) {
      HANDLE_ERROR(cx, *err);
      return RejectPromiseWithPendingError(cx, promise);
    }
    size_t len = chunk.unwrap();
    if (len == 0) {
      buf = static_cast<char *>(realloc(buf, buf_len));
      break;
    }
    buf_len += len;
    if (buf_len > HANDLE_READ_BUFFER_SIZE) {
      JS_ReportErrorLatin1(cx, "Buffer error: Buffer too large.");
      return RejectPromiseWithPendingError(cx, promise);
    }
  } while (true);
  JS::RootedString str(cx, JS_NewStringCopyUTF8N(cx, JS::UTF8Chars(buf, buf_len)));
  if (!str) {
    return false;
  }
  JS::RootedValue str_val(cx, JS::StringValue(str));
  JS::RootedValue json(cx);
  if (!JS_ParseJSON(cx, str, &json)) {
    return false;
  }
  if (!json.isObject()) {
    JS_ReportErrorLatin1(cx, "Bad data.");
    return false;
  }
  JS::RootedValue list(cx);
  JS::RootedObject json_obj(cx, &json.toObject());
  if (!JS_GetProperty(cx, json_obj, "data", &list)) {
    return false;
  }
  return JS::ResolvePromise(cx, promise, list);
}

bool process_pending_kv_store_delete(JSContext *cx, host_api::KVStorePendingDelete::Handle handle,
                                     JS::HandleObject context, JS::HandleObject promise) {
  host_api::KVStorePendingDelete pending_delete(handle);

  auto res = pending_delete.wait();
  if (auto *err = res.to_err()) {
    std::string message = std::move(err->message()).value_or("when attempting to fetch resource.");
    JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr, JSMSG_KV_STORE_DELETE_ERROR,
                              message.c_str());
    return RejectPromiseWithPendingError(cx, promise);
  }

  JS::ResolvePromise(cx, promise, JS::UndefinedHandleValue);
  return true;
}

bool process_pending_kv_store_lookup(JSContext *cx, host_api::KVStorePendingLookup::Handle handle,
                                     JS::HandleObject context, JS::HandleObject promise) {
  host_api::KVStorePendingLookup pending_lookup(handle);

  auto res = pending_lookup.wait();

  if (auto *err = res.to_err()) {
    std::string message = std::move(err->message()).value_or("when attempting to fetch resource.");
    JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr, JSMSG_KV_STORE_LOOKUP_ERROR,
                              message.c_str());
    return RejectPromiseWithPendingError(cx, promise);
  }

  // When no entry is found, we are going to resolve the Promise with `null`.
  if (!res.unwrap().has_value()) {
    JS::RootedValue result(cx);
    result.setNull();
    JS::ResolvePromise(cx, promise, result);
  } else {
    host_api::HttpBody body = std::get<0>(res.unwrap().value());
    host_api::HostBytes metadata = std::move(std::get<1>(res.unwrap().value()));
    // uint32_t gen = std::get<2>(res.unwrap());
    JS::RootedObject entry(cx, KVStoreEntry::create(cx, body, std::move(metadata)));
    if (!entry) {
      return false;
    }
    JS::RootedValue result(cx);
    result.setObject(*entry);
    JS::ResolvePromise(cx, promise, result);
  }

  return true;
}

} // namespace

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

  auto res = kv_store(self).delete_(key_chars);

  if (auto *err = res.to_err()) {
    HANDLE_ERROR(cx, *err);
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }
  auto handle = res.unwrap();

  ENGINE->queue_async_task(
      new FastlyAsyncTask(handle, self, result_promise, process_pending_kv_store_delete));

  args.rval().setObject(*result_promise);
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

  auto res = kv_store(self).lookup(key_chars);

  if (auto *err = res.to_err()) {
    HANDLE_ERROR(cx, *err);
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }
  auto handle = res.unwrap();

  auto task = new FastlyAsyncTask(handle, self, result_promise, process_pending_kv_store_lookup);
  ENGINE->queue_async_task(task);

  args.rval().setObject(*result_promise);
  return true;
}

bool KVStore::put(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(2)

  JS::RootedObject result_promise(cx, JS::NewPromiseObject(cx, nullptr));
  if (!result_promise) {
    return false;
  }

  JS::RootedValue key(cx, args.get(0));

  // Convert the key argument into a String following https://tc39.es/ecma262/#sec-tostring
  auto key_chars = core::encode(cx, key);
  if (!key_chars) {
    return false;
  }

  if (!parse_and_validate_key(cx, key_chars.begin(), key_chars.len)) {
    return false;
  }

  JS::HandleValue body_val = args.get(1);

  JS::RootedValue metadata_val(cx);
  std::optional<uint32_t> ttl = std::nullopt;
  std::optional<std::tuple<const uint8_t *, size_t>> metadata = std::nullopt;
  std::optional<host_api::KVStore::InsertMode> mode = std::nullopt;
  if (args.get(2).isObject()) {
    JS::RootedObject opts_val(cx, &args.get(2).toObject());

    JS::RootedValue ttl_val(cx);
    if (!JS_GetProperty(cx, opts_val, "ttl", &ttl_val)) {
      return false;
    }

    if (!ttl_val.isUndefined()) {
      auto parsed = parse_and_validate_timeout(cx, ttl_val, "KVStore.put", "ttl", 0x100000000);
      if (!parsed) {
        return false;
      }
      ttl = parsed;
    }

    if (!JS_GetProperty(cx, opts_val, "metadata", &metadata_val)) {
      return false;
    }
    // metadata is actually read just before the hostcall

    JS::RootedValue mode_val(cx);
    if (!JS_GetProperty(cx, opts_val, "mode", &mode_val)) {
      return false;
    }
    if (!mode_val.isUndefined()) {
      auto mode_name = JS::RootedString(cx, JS::ToString(cx, mode_val));
      if (!mode_name) {
        return false;
      }
      bool match = false;
      if (!JS_StringEqualsLiteral(cx, mode_name, "add", &match)) {
        return false;
      }
      if (match) {
        mode = host_api::KVStore::InsertMode::add;
      } else {
        if (!JS_StringEqualsLiteral(cx, mode_name, "append", &match)) {
          return false;
        }
        if (match) {
          mode = host_api::KVStore::InsertMode::append;
        } else {
          if (!JS_StringEqualsLiteral(cx, mode_name, "overwrite", &match)) {
            return false;
          }
          if (match) {
            mode = host_api::KVStore::InsertMode::overwrite;
          } else {
            if (!JS_StringEqualsLiteral(cx, mode_name, "prepend", &match)) {
              return false;
            }
            if (match) {
              mode = host_api::KVStore::InsertMode::prepend;
            }
          }
        }
      }
    }
  }

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

      // metadata object is read last because no JS can run after getting byte reference
      if (!metadata_val.isUndefined()) {
        auto maybe_byte_data = validate_bytes(cx, metadata_val, "KVStore.put metadata");
        if (!maybe_byte_data) {
          return ReturnPromiseRejectedWithPendingError(cx, args);
        }
        metadata = maybe_byte_data;
      }

      auto res = kv_store(self).insert(key_chars, body, mode, std::nullopt, metadata, ttl);
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

    // metadata object is read last because no JS can run after getting byte reference
    if (!metadata_val.isUndefined()) {
      auto maybe_byte_data = validate_bytes(cx, metadata_val, "KVStore.put metadata");
      if (!maybe_byte_data) {
        return ReturnPromiseRejectedWithPendingError(cx, args);
      }
      metadata = maybe_byte_data;
    }

    auto insert_res = kv_store(self).insert(key_chars, body, mode, std::nullopt, metadata, ttl);
    if (auto *err = insert_res.to_err()) {
      // Ensure that we throw an exception for all unexpected host errors.
      HANDLE_ERROR(cx, *err);
      return RejectPromiseWithPendingError(cx, result_promise);
    }

    host_api::KVStorePendingInsert pending_insert(insert_res.unwrap());

    auto res = pending_insert.wait();
    if (auto *err = res.to_err()) {
      std::string message =
          std::move(err->message()).value_or("when attempting to fetch resource.");
      JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr, JSMSG_KV_STORE_LIST_ERROR,
                                message.c_str());
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

bool KVStore::list(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0)

  if (!args.get(0).isObject()) {
    api::throw_error(cx, api::Errors::TypeError, "KVStore.list", "options", "be an object");
    return false;
  }

  JS::RootedObject options(cx, &args.get(0).toObject());

  JS::RootedValue limit_val(cx);
  if (!JS_GetProperty(cx, options, "limit", &limit_val)) {
    return false;
  }

  std::optional<uint32_t> limit = std::nullopt;
  if (!limit_val.isNullOrUndefined()) {
    if (limit_val.isNumber()) {
      if (limit_val.isDouble()) {
        double limit_double = limit_val.toDouble();
        if (std::floor(limit_double) == limit_double) {
          limit.emplace(limit_double);
        }
      } else if (limit_val.isInt32()) {
        limit.emplace(limit_val.toInt32());
      }
    }
    if (!limit.has_value()) {
      api::throw_error(cx, api::Errors::TypeError, "KVStore.list", "limit", "be an integer");
      return false;
    }
  }

  std::optional<std::string_view> prefix = std::nullopt;
  host_api::HostString prefix_str;

  JS::RootedValue prefix_val(cx);
  if (!JS_GetProperty(cx, options, "prefix", &prefix_val)) {
    return false;
  }
  if (!prefix_val.isNullOrUndefined()) {
    if (!prefix_val.isString()) {
      api::throw_error(cx, api::Errors::TypeError, "KVStore.list", "prefix", "be a string");
      return ReturnPromiseRejectedWithPendingError(cx, args);
    }
    prefix_str = core::encode(cx, prefix_val);
    if (!prefix_str) {
      return ReturnPromiseRejectedWithPendingError(cx, args);
    }
    prefix = prefix_str;
  }

  bool no_sync = false;
  JS::RootedValue no_sync_val(cx);
  if (!JS_GetProperty(cx, options, "noSync", &no_sync_val)) {
    return false;
  }
  if (!no_sync_val.isNullOrUndefined()) {
    if (!no_sync_val.isBoolean()) {
      api::throw_error(cx, api::Errors::TypeError, "KVStore.list", "noSync", "be a boolean");
      return ReturnPromiseRejectedWithPendingError(cx, args);
    }
    no_sync = no_sync_val.toBoolean();
  }

  std::string cursor_64;
  std::optional<std::string_view> cursor = std::nullopt;
  JS::RootedValue from_key(cx);
  if (!JS_GetProperty(cx, options, "fromKey", &from_key)) {
    return false;
  }
  if (!from_key.isNullOrUndefined()) {
    if (!from_key.isString()) {
      api::throw_error(cx, api::Errors::TypeError, "KVStore.list", "fromKey", "be a string");
      return ReturnPromiseRejectedWithPendingError(cx, args);
    }
    auto byteStringResult = builtins::web::base64::valueToJSByteString(cx, from_key);
    if (byteStringResult.isErr()) {
      return false;
    }
    auto byteString = byteStringResult.unwrap();
    cursor_64 = builtins::web::base64::forgivingBase64Encode(
        byteString, builtins::web::base64::base64EncodeTable);
    cursor.emplace(cursor_64);
  }

  auto res = kv_store(self).list(cursor, limit, prefix, no_sync);
  if (auto *err = res.to_err()) {
    // Ensure that we throw an exception for all unexpected host errors.
    HANDLE_ERROR(cx, *err);
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }

  auto handle = res.unwrap();

  JS::RootedObject result_promise(cx, JS::NewPromiseObject(cx, nullptr));
  if (!result_promise) {
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }

  auto task = new FastlyAsyncTask(handle, self, result_promise, process_pending_kv_store_list);
  ENGINE->queue_async_task(task);

  args.rval().setObject(*result_promise);
  return true;
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
    JS_FN("list", list, 1, JSPROP_ENUMERATE),
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

  auto res = host_api::KVStore::open(name);
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
