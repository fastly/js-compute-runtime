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

#include "builtin.h"
#include "builtins/native-stream-source.h"
#include "builtins/object-store.h"
#include "builtins/shared/url.h"
#include "host_interface/host_api.h"
#include "host_interface/host_call.h"
#include "js-compute-builtins.h"

namespace builtins {

namespace {

std::string_view bad_chars{"#?*[]\n\r"};

std::optional<char> find_invalid_character_for_object_store_key(const char *str) {
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
bool ObjectStoreEntry::bodyAll(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0)
  return RequestOrResponse::bodyAll<result_type>(cx, args, self);
}

bool ObjectStoreEntry::body_get(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0)
  if (!JS::GetReservedSlot(self, static_cast<uint32_t>(Slots::HasBody)).isBoolean()) {
    JS::SetReservedSlot(self, static_cast<uint32_t>(Slots::HasBody), JS::BooleanValue(false));
  }
  return RequestOrResponse::body_get(cx, args, self, true);
}

bool ObjectStoreEntry::bodyUsed_get(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0)
  if (!JS::GetReservedSlot(self, static_cast<uint32_t>(Slots::BodyUsed)).isBoolean()) {
    JS::SetReservedSlot(self, static_cast<uint32_t>(Slots::BodyUsed), JS::BooleanValue(false));
  }
  args.rval().setBoolean(RequestOrResponse::body_used(self));
  return true;
}

const JSFunctionSpec ObjectStoreEntry::static_methods[] = {
    JS_FS_END,
};

const JSPropertySpec ObjectStoreEntry::static_properties[] = {
    JS_PS_END,
};

const JSFunctionSpec ObjectStoreEntry::methods[] = {
    JS_FN("arrayBuffer", bodyAll<RequestOrResponse::BodyReadResult::ArrayBuffer>, 0,
          JSPROP_ENUMERATE),
    JS_FN("json", bodyAll<RequestOrResponse::BodyReadResult::JSON>, 0, JSPROP_ENUMERATE),
    JS_FN("text", bodyAll<RequestOrResponse::BodyReadResult::Text>, 0, JSPROP_ENUMERATE),
    JS_FS_END,
};

const JSPropertySpec ObjectStoreEntry::properties[] = {
    JS_PSG("body", body_get, JSPROP_ENUMERATE),
    JS_PSG("bodyUsed", bodyUsed_get, JSPROP_ENUMERATE),
    JS_PS_END,
};

bool ObjectStoreEntry::constructor(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS_ReportErrorUTF8(cx, "ObjectStoreEntry can't be instantiated directly");
  return false;
}

JSObject *ObjectStoreEntry::create(JSContext *cx, fastly_body_handle_t body_handle) {
  JS::RootedObject objectStoreEntry(cx, JS_NewObjectWithGivenProto(cx, &class_, proto_obj));
  if (!objectStoreEntry)
    return nullptr;

  JS::SetReservedSlot(objectStoreEntry, static_cast<uint32_t>(Slots::Body),
                      JS::Int32Value(body_handle));
  JS::SetReservedSlot(objectStoreEntry, static_cast<uint32_t>(Slots::BodyStream), JS::NullValue());
  JS::SetReservedSlot(objectStoreEntry, static_cast<uint32_t>(Slots::HasBody),
                      JS::BooleanValue(true));
  JS::SetReservedSlot(objectStoreEntry, static_cast<uint32_t>(Slots::BodyUsed), JS::FalseValue());

  return objectStoreEntry;
}

bool ObjectStoreEntry::init_class(JSContext *cx, JS::HandleObject global) {
  return init_class_impl(cx, global);
}

namespace {

fastly_object_store_handle_t object_store_handle(JSObject *obj) {
  JS::Value val = JS::GetReservedSlot(obj, static_cast<uint32_t>(ObjectStore::Slots::ObjectStore));
  return fastly_object_store_handle_t{static_cast<uint32_t>(val.toInt32())};
}

bool parse_and_validate_key(JSContext *cx, JS::UniqueChars *key, size_t len) {
  // If the converted string has a length of 0 then we throw an Error
  // because ObjectStore Keys have to be at-least 1 character.
  if (len == 0) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_OBJECT_STORE_KEY_EMPTY);
    return false;
  }

  // If the converted string has a length of more than 1024 then we throw an Error
  // because ObjectStore Keys have to be less than 1025 characters.
  if (len > 1024) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_OBJECT_STORE_KEY_TOO_LONG);
    return false;
  }

  auto key_chars = key->get();
  auto res = find_invalid_character_for_object_store_key(key_chars);
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
    JS_ReportErrorNumberUTF8(cx, GetErrorMessage, nullptr, JSMSG_OBJECT_STORE_KEY_INVALID_CHARACTER,
                             character.c_str());
    return false;
  }
  auto acme_challenge = ".well-known/acme-challenge/";
  if (strncmp(key_chars, acme_challenge, strlen(acme_challenge)) == 0) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_OBJECT_STORE_KEY_ACME);
    return false;
  }

  if (strcmp(key_chars, ".") == 0 || strcmp(key_chars, "..") == 0) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_OBJECT_STORE_KEY_RELATIVE);
    return false;
  }

  return true;
}

} // namespace

bool ObjectStore::get(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(1)

  JS::RootedObject result_promise(cx, JS::NewPromiseObject(cx, nullptr));
  if (!result_promise) {
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }

  JS::RootedValue key(cx, args.get(0));

  // Convert the key argument into a String following https://tc39.es/ecma262/#sec-tostring
  fastly_world_string_t key_str;
  JS::UniqueChars key_chars = encode(cx, key, &key_str.len);
  if (!key_chars)
    return false;
  key_str.ptr = key_chars.get();

  if (!parse_and_validate_key(cx, &key_chars, key_str.len))
    return ReturnPromiseRejectedWithPendingError(cx, args);

  fastly_option_body_handle_t ret;
  fastly_error_t err;
  if (!fastly_object_store_lookup(object_store_handle(self), &key_str, &ret, &err)) {
    HANDLE_ERROR(cx, err);
    return false;
  }

  // When no entry is found, we are going to resolve the Promise with `null`.
  if (!ret.is_some) {
    JS::RootedValue result(cx);
    result.setNull();
    JS::ResolvePromise(cx, result_promise, result);
  } else {
    JS::RootedObject entry(cx, ObjectStoreEntry::create(cx, ret.val));
    if (!entry) {
      return false;
    }
    JS::RootedValue result(cx);
    result.setObject(*entry);
    JS::ResolvePromise(cx, result_promise, result);
  }

  args.rval().setObject(*result_promise);
  return true;
}

bool ObjectStore::put(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(2)

  JS::RootedObject result_promise(cx, JS::NewPromiseObject(cx, nullptr));
  if (!result_promise) {
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }

  JS::RootedValue key(cx, args.get(0));

  // Convert the key argument into a String following https://tc39.es/ecma262/#sec-tostring
  fastly_world_string_t key_str;
  JS::UniqueChars key_chars = encode(cx, key, &key_str.len);
  if (!key_chars)
    return false;
  key_str.ptr = key_chars.get();

  if (!parse_and_validate_key(cx, &key_chars, key_str.len))
    return ReturnPromiseRejectedWithPendingError(cx, args);

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
      JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr,
                                JSMSG_READABLE_STREAM_LOCKED_OR_DISTRUBED);
      return ReturnPromiseRejectedWithPendingError(cx, args);
    }

    // If the body stream is backed by a C@E body handle, we can directly pipe
    // that handle into the object store.
    if (builtins::NativeStreamSource::stream_is_body(cx, body_obj)) {
      JS::RootedObject stream_source(cx,
                                     builtins::NativeStreamSource::get_stream_source(cx, body_obj));
      JS::RootedObject source_owner(cx, builtins::NativeStreamSource::owner(stream_source));
      fastly_body_handle_t body = RequestOrResponse::body_handle(source_owner);

      fastly_error_t err;
      if (!fastly_object_store_insert(object_store_handle(self), &key_str, body, &err)) {
        HANDLE_ERROR(cx, err);
        return ReturnPromiseRejectedWithPendingError(cx, args);
      }

      // The insert was successful so we return a Promise which resolves to undefined
      JS::RootedValue rval(cx);
      rval.setUndefined();
      JS::ResolvePromise(cx, result_promise, rval);
      args.rval().setObject(*result_promise);

      return true;
    } else {
      JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr,
                                JSMSG_OBJECT_STORE_PUT_CONTENT_STREAM);
      return ReturnPromiseRejectedWithPendingError(cx, args);
    }
  } else {
    mozilla::Maybe<JS::AutoCheckCannotGC> maybeNoGC;
    JS::UniqueChars text;
    char *buf;
    size_t length;

    if (body_obj && JS_IsArrayBufferViewObject(body_obj)) {
      // `maybeNoGC` needs to be populated for the lifetime of `buf` because
      // short typed arrays have inline data which can move on GC, so assert
      // that no GC happens. (Which it doesn't, because we're not allocating
      // before `buf` goes out of scope.)
      maybeNoGC.emplace(cx);
      JS::AutoCheckCannotGC &noGC = maybeNoGC.ref();
      bool is_shared;
      length = JS_GetArrayBufferViewByteLength(body_obj);
      buf = (char *)JS_GetArrayBufferViewData(body_obj, &is_shared, noGC);
    } else if (body_obj && JS::IsArrayBufferObject(body_obj)) {
      bool is_shared;
      JS::GetArrayBufferLengthAndData(body_obj, &length, &is_shared, (uint8_t **)&buf);
    } else if (body_obj && builtins::URLSearchParams::is_instance(body_obj)) {
      jsurl::SpecSlice slice = builtins::URLSearchParams::serialize(cx, body_obj);
      buf = (char *)slice.data;
      length = slice.len;
    } else {
      // Convert into a String following https://tc39.es/ecma262/#sec-tostring
      text = encode(cx, body_val, &length);
      // 30MB in bytes is the max size allowed for ObjectStore.
      if (length > 30 * 1024 * 1024) {
        JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_OBJECT_STORE_PUT_OVER_30_MB);
        return ReturnPromiseRejectedWithPendingError(cx, args);
      }
      if (!text) {
        return ReturnPromiseRejectedWithPendingError(cx, args);
      }
      buf = text.get();
    }

    auto make_res = HttpBody::make();
    if (auto *err = make_res.to_err()) {
      HANDLE_ERROR(cx, *err);
      return ReturnPromiseRejectedWithPendingError(cx, args);
    }

    auto body = make_res.unwrap();
    if (!body.valid()) {
      return ReturnPromiseRejectedWithPendingError(cx, args);
    }

    auto write_res = body.write_all(reinterpret_cast<uint8_t *>(buf), length);

    // Ensure that the NoGC is reset, so throwing an error in HANDLE_ERROR
    // succeeds.
    if (maybeNoGC.isSome()) {
      maybeNoGC.reset();
    }

    if (auto *err = write_res.to_err()) {
      HANDLE_ERROR(cx, *err);
      return ReturnPromiseRejectedWithPendingError(cx, args);
    }

    fastly_error_t err;
    if (!fastly_object_store_insert(object_store_handle(self), &key_str, body.handle, &err)) {
      // Ensure that we throw an exception for all unexpected host errors.
      HANDLE_ERROR(cx, err);
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

const JSFunctionSpec ObjectStore::static_methods[] = {
    JS_FS_END,
};

const JSPropertySpec ObjectStore::static_properties[] = {
    JS_PS_END,
};

const JSFunctionSpec ObjectStore::methods[] = {
    JS_FN("get", get, 1, JSPROP_ENUMERATE),
    JS_FN("put", put, 1, JSPROP_ENUMERATE),
    JS_FS_END,
};

const JSPropertySpec ObjectStore::properties[] = {
    JS_PS_END,
};

bool ObjectStore::constructor(JSContext *cx, unsigned argc, JS::Value *vp) {
  REQUEST_HANDLER_ONLY("The ObjectStore builtin");
  CTOR_HEADER("ObjectStore", 1);

  JS::HandleValue name_arg = args.get(0);

  fastly_world_string_t name_str;
  // Convert into a String following https://tc39.es/ecma262/#sec-tostring
  JS::UniqueChars name = encode(cx, name_arg, &name_str.len);
  if (!name) {
    return false;
  }
  name_str.ptr = name.get();

  // If the converted string has a length of 0 then we throw an Error
  // because ObjectStore names have to be at-least 1 character.
  if (name_str.len == 0) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_OBJECT_STORE_NAME_EMPTY);
    return false;
  }

  // If the converted string has a length of more than 255 then we throw an Error
  // because ObjectStore names have to be less than 255 characters.
  if (name_str.len > 255) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_OBJECT_STORE_NAME_TOO_LONG);
    return false;
  }

  for (int i = 0; i < name_str.len; i++) {
    char character = name_str.ptr[i];
    if (std::iscntrl(static_cast<unsigned char>(character)) != 0) {
      JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr,
                                JSMSG_OBJECT_STORE_NAME_NO_CONTROL_CHARACTERS);
      return false;
    }
  }

  fastly_object_store_handle_t object_store_handle = INVALID_HANDLE;
  fastly_error_t err;
  if (!fastly_object_store_open(&name_str, &object_store_handle, &err)) {
    if (err == FASTLY_ERROR_INVALID_ARGUMENT) {
      JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_OBJECT_STORE_DOES_NOT_EXIST,
                                name_str.ptr);
      return false;
    } else {
      HANDLE_ERROR(cx, err);
      return false;
    }
  }

  JS::RootedObject object_store(cx, JS_NewObjectForConstructor(cx, &class_, args));
  if (!object_store) {
    return false;
  }
  JS::SetReservedSlot(object_store, static_cast<uint32_t>(Slots::ObjectStore),
                      JS::Int32Value(object_store_handle));
  args.rval().setObject(*object_store);
  return true;
}

bool ObjectStore::init_class(JSContext *cx, JS::HandleObject global) {
  return init_class_impl(cx, global);
}

} // namespace builtins
