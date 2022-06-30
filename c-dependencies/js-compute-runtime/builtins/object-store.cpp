// TODO: remove these once the warnings are fixed
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-offsetof"
#include "js/experimental/TypedData.h"
#pragma clang diagnostic pop

#include "js/ArrayBuffer.h"
#include "js/Stream.h"

#include "builtin.h"
#include "builtins/native-stream-source.h"
#include "builtins/url.h"
#include "host_call.h"
#include "js-compute-builtins.h"

namespace ObjectStoreEntry {
namespace Slots {
enum {
  Body = RequestOrResponse::Slots::Body,
  BodyStream = RequestOrResponse::Slots::BodyStream,
  HasBody = RequestOrResponse::Slots::HasBody,
  BodyUsed = RequestOrResponse::Slots::BodyUsed,
  Count
};
};

const unsigned ctor_length = 0;

bool check_receiver(JSContext *cx, JS::HandleValue receiver, const char *method_name);

template <RequestOrResponse::BodyReadResult result_type>
bool bodyAll(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0)
  return RequestOrResponse::bodyAll<result_type>(cx, args, self);
}

bool body_get(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0)
  return RequestOrResponse::body_get(cx, args, self, true);
}

bool bodyUsed_get(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0)
  args.rval().setBoolean(RequestOrResponse::body_used(self));
  return true;
}

const JSFunctionSpec methods[] = {
    JS_FN("arrayBuffer", bodyAll<RequestOrResponse::BodyReadResult::ArrayBuffer>, 0,
          JSPROP_ENUMERATE),
    JS_FN("json", bodyAll<RequestOrResponse::BodyReadResult::JSON>, 0, JSPROP_ENUMERATE),
    JS_FN("text", bodyAll<RequestOrResponse::BodyReadResult::Text>, 0, JSPROP_ENUMERATE),
    JS_FS_END};

const JSPropertySpec properties[] = {JS_PSG("body", body_get, JSPROP_ENUMERATE),
                                     JS_PSG("bodyUsed", bodyUsed_get, JSPROP_ENUMERATE), JS_PS_END};

bool constructor(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS_ReportErrorUTF8(cx, "ObjectStoreEntry can't be instantiated directly");
  return false;
}

CLASS_BOILERPLATE(ObjectStoreEntry)

JSObject *create(JSContext *cx, BodyHandle body_handle) {
  JS::RootedObject objectStoreEntry(cx, JS_NewObjectWithGivenProto(cx, &class_, proto_obj));
  if (!objectStoreEntry)
    return nullptr;

  JS::SetReservedSlot(objectStoreEntry, Slots::Body, JS::Int32Value((int)body_handle.handle));
  JS::SetReservedSlot(objectStoreEntry, Slots::BodyStream, JS::NullValue());
  JS::SetReservedSlot(objectStoreEntry, Slots::HasBody, JS::TrueValue());
  JS::SetReservedSlot(objectStoreEntry, Slots::BodyUsed, JS::FalseValue());

  return objectStoreEntry;
}
} // namespace ObjectStoreEntry

namespace ObjectStore {
namespace Slots {
enum { ObjectStore, Count };
};

bool is_instance(JSObject *obj);
bool is_instance(JS::Value val);

ObjectStoreHandle object_store_handle(JSObject *obj) {
  JS::Value val = JS::GetReservedSlot(obj, Slots::ObjectStore);
  return ObjectStoreHandle{static_cast<uint32_t>(val.toInt32())};
}

const unsigned ctor_length = 1;

bool check_receiver(JSContext *cx, JS::HandleValue receiver, const char *method_name);

bool lookup(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(1)

  JS::RootedObject result_promise(cx, JS::NewPromiseObject(cx, nullptr));
  if (!result_promise) {
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }

  size_t key_len;
  // Convert the key argument into a String following https://tc39.es/ecma262/#sec-tostring
  JS::UniqueChars key = encode(cx, args.get(0), &key_len);
  if (!key) {
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }

  // If the converted string has a length of 0 then we throw an Error
  // because ObjectStore Keys have to be at-least 1 character.
  if (key_len == 0) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_OBJECT_STORE_KEY_EMPTY);
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }

  // If the converted string has a length of more than 1024 then we throw an Error
  // because ObjectStore Keys have to be less than 1025 characters.
  if (key_len > 1024) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_OBJECT_STORE_KEY_TOO_LONG);
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }

  char *key_chars = key.get();

  if (strchr(key_chars, '#') != NULL) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr,
                              JSMSG_OBJECT_STORE_KEY_INVALID_CHARACTER, "#");
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }
  if (strchr(key_chars, '?') != NULL) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr,
                              JSMSG_OBJECT_STORE_KEY_INVALID_CHARACTER, "?");
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }
  if (strchr(key_chars, '*') != NULL) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr,
                              JSMSG_OBJECT_STORE_KEY_INVALID_CHARACTER, "*");
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }
  if (strchr(key_chars, '[') != NULL) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr,
                              JSMSG_OBJECT_STORE_KEY_INVALID_CHARACTER, "[");
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }
  if (strchr(key_chars, ']') != NULL) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr,
                              JSMSG_OBJECT_STORE_KEY_INVALID_CHARACTER, "]");
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }
  if (strchr(key_chars, '\n') != NULL) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr,
                              JSMSG_OBJECT_STORE_KEY_INVALID_CHARACTER, "newline");
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }
  if (strchr(key_chars, '\r') != NULL) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr,
                              JSMSG_OBJECT_STORE_KEY_INVALID_CHARACTER, "carriage return");
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }
  auto acme_challenge = ".well-known/acme-challenge/";
  if (strncmp(key_chars, acme_challenge, strlen(acme_challenge)) == 0) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_OBJECT_STORE_KEY_ACME);
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }

  if (strcmp(key_chars, ".") == 0 || strcmp(key_chars, "..") == 0) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_OBJECT_STORE_KEY_RELATIVE);
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }

  BodyHandle body_handle = {INVALID_HANDLE};
  int status =
      fastly_object_store_lookup(object_store_handle(self), key_chars, key_len, &body_handle);
  if (!HANDLE_RESULT(cx, status)) {
    return false;
  }

  // If the handle is invalid it means no entry was found
  // When no entry is found, we are going to resolve the Promise with `null`.
  if (body_handle.handle == INVALID_HANDLE) {
    JS::RootedValue result(cx);
    result.setNull();
    JS::ResolvePromise(cx, result_promise, result);
  } else {
    JS::RootedObject entry(cx, ObjectStoreEntry::create(cx, body_handle));
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

bool put(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(2)

  JS::RootedObject result_promise(cx, JS::NewPromiseObject(cx, nullptr));
  if (!result_promise) {
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }

  size_t key_len;
  // Convert the key argument into a String following https://tc39.es/ecma262/#sec-tostring
  JS::UniqueChars key = encode(cx, args.get(0), &key_len);
  if (!key) {
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }

  // If the converted string has a length of 0 then we throw an Error
  // because ObjectStore Keys have to be at-least 1 character.
  if (key_len == 0) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_OBJECT_STORE_KEY_EMPTY);
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }

  // If the converted string has a length of more than 1024 then we throw an Error
  // because ObjectStore Keys have to be less than 1025 characters.
  if (key_len > 1024) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_OBJECT_STORE_KEY_TOO_LONG);
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }

  char *key_chars = key.get();

  if (strchr(key_chars, '#') != NULL) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr,
                              JSMSG_OBJECT_STORE_KEY_INVALID_CHARACTER, "#");
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }
  if (strchr(key_chars, '?') != NULL) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr,
                              JSMSG_OBJECT_STORE_KEY_INVALID_CHARACTER, "?");
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }
  if (strchr(key_chars, '*') != NULL) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr,
                              JSMSG_OBJECT_STORE_KEY_INVALID_CHARACTER, "*");
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }
  if (strchr(key_chars, '[') != NULL) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr,
                              JSMSG_OBJECT_STORE_KEY_INVALID_CHARACTER, "[");
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }
  if (strchr(key_chars, ']') != NULL) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr,
                              JSMSG_OBJECT_STORE_KEY_INVALID_CHARACTER, "]");
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }
  if (strchr(key_chars, '\n') != NULL) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr,
                              JSMSG_OBJECT_STORE_KEY_INVALID_CHARACTER, "newline");
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }
  if (strchr(key_chars, '\r') != NULL) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr,
                              JSMSG_OBJECT_STORE_KEY_INVALID_CHARACTER, "carriage return");
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }
  auto acme_challenge = ".well-known/acme-challenge/";
  if (strncmp(key_chars, acme_challenge, strlen(acme_challenge)) == 0) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_OBJECT_STORE_KEY_ACME);
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }

  if (strcmp(key_chars, ".") == 0 || strcmp(key_chars, "..") == 0) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_OBJECT_STORE_KEY_RELATIVE);
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
      JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr,
                                JSMSG_READABLE_STREAM_LOCKED_OR_DISTRUBED);
      return ReturnPromiseRejectedWithPendingError(cx, args);
    }

    // If the body stream is backed by a C@E body handle, we can directly pipe
    // that handle into the object store.
    if (builtins::NativeStreamSource::stream_is_body(cx, body_obj)) {
      JS::RootedObject stream_source(cx, builtins::NativeStreamSource::get_stream_source(cx, body_obj));
      JS::RootedObject source_owner(cx, builtins::NativeStreamSource::owner(stream_source));
      BodyHandle body = RequestOrResponse::body_handle(source_owner);

      int status = fastly_object_store_insert(object_store_handle(self), key_chars, key_len, body);
      if (!HANDLE_RESULT(cx, status)) {
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
      // Short typed arrays have inline data which can move on GC, so assert
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
    } else if (body_obj && URLSearchParams::is_instance(body_obj)) {
      jsurl::SpecSlice slice = URLSearchParams::serialize(cx, body_obj);
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

    BodyHandle body_handle = {INVALID_HANDLE};
    if (!HANDLE_RESULT(cx, xqd_body_new(&body_handle))) {
      return ReturnPromiseRejectedWithPendingError(cx, args);
    }

    if (body_handle.handle == INVALID_HANDLE) {
      return ReturnPromiseRejectedWithPendingError(cx, args);
    }

    int result = write_to_body_all(body_handle, buf, length);

    // Ensure that the NoGC is reset, so throwing an error in HANDLE_RESULT
    // succeeds.
    if (maybeNoGC.isSome()) {
      maybeNoGC.reset();
    }

    if (!HANDLE_RESULT(cx, result)) {
      return ReturnPromiseRejectedWithPendingError(cx, args);
    }

    int status =
        fastly_object_store_insert(object_store_handle(self), key_chars, key_len, body_handle);
    // Ensure that we throw an exception for all unexpected host errors.
    if (!HANDLE_RESULT(cx, status)) {
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

const JSFunctionSpec methods[] = {JS_FN("lookup", lookup, 1, JSPROP_ENUMERATE),
                                  JS_FN("put", put, 1, JSPROP_ENUMERATE), JS_FS_END};

const JSPropertySpec properties[] = {JS_PS_END};

bool constructor(JSContext *cx, unsigned argc, JS::Value *vp);
CLASS_BOILERPLATE(ObjectStore)

bool constructor(JSContext *cx, unsigned argc, JS::Value *vp) {
  REQUEST_HANDLER_ONLY("The ObjectStore builtin");
  CTOR_HEADER("ObjectStore", 1);

  JS::HandleValue name_arg = args.get(0);

  size_t name_len;
  // Convert into a String following https://tc39.es/ecma262/#sec-tostring
  JS::UniqueChars name = encode(cx, name_arg, &name_len);
  if (!name) {
    return false;
  }

  // If the converted string has a length of 0 then we throw an Error
  // because ObjectStore names have to be at-least 1 character.
  if (name_len == 0) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_OBJECT_STORE_NAME_EMPTY);
    return false;
  }

  // If the converted string has a length of more than 255 then we throw an Error
  // because ObjectStore names have to be less than 1025 characters.
  if (name_len > 255) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_OBJECT_STORE_NAME_TOO_LONG);
    return false;
  }

  auto name_chars = name.get();

  for (int i = 0; i < name_len; i++) {
    char character = name_chars[i];
    if (std::iscntrl(static_cast<unsigned char>(character)) != 0) {
      JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr,
                                JSMSG_OBJECT_STORE_NAME_NO_CONTROL_CHARACTERS);
      return false;
    }
  }

  ObjectStoreHandle object_store_handle = {INVALID_HANDLE};
  int status = fastly_object_store_open(name_chars, name_len, &object_store_handle);
  if (status == 2) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_OBJECT_STORE_DOES_NOT_EXIST,
                              name_chars);
    return false;
  }

  if (!HANDLE_RESULT(cx, fastly_object_store_open(name_chars, name_len, &object_store_handle))) {
    return false;
  }

  JS::RootedObject object_store(cx, JS_NewObjectForConstructor(cx, &class_, args));
  if (!object_store) {
    return false;
  }
  JS::SetReservedSlot(object_store, Slots::ObjectStore,
                      JS::Int32Value((int)object_store_handle.handle));
  args.rval().setObject(*object_store);
  return true;
}
} // namespace ObjectStore
