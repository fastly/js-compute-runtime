#include <arpa/inet.h>
#include <chrono>
#include <cmath>
#include <iostream>
#include <list>
#include <regex> // std::regex
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <vector>

#include "js-compute-builtins.h"
#include "picosha2.h"
#include "rust-url/rust-url.h"
#include "xqd-world/xqd_world_adapter.h"

#include "js/Array.h"
#include "js/ArrayBuffer.h"
#include "js/Conversions.h"
#include "mozilla/Result.h"

// TODO: remove these once the warnings are fixed
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-offsetof"
#include "js/experimental/TypedData.h"
#pragma clang diagnostic pop

#include "js/JSON.h"
#include "js/Stream.h"
#include "js/StructuredClone.h"
#include "js/Value.h"
#include "js/experimental/TypedData.h"
#include "js/friend/DumpFunctions.h"
#include "js/shadow/Object.h"
#include "zlib.h"

#include "geo_ip.h"
#include "host_call.h"
#include "sequence.hpp"

#include "builtin.h"
#include "builtins/backend.h"
#include "builtins/cache-override.h"
#include "builtins/compression-stream.h"
#include "builtins/config-store.h"
#include "builtins/console.h"
#include "builtins/crypto.h"
#include "builtins/decompression-stream.h"
#include "builtins/dictionary.h"
#include "builtins/env.h"
#include "builtins/fastly.h"
#include "builtins/logger.h"
#include "builtins/native-stream-sink.h"
#include "builtins/native-stream-source.h"
#include "builtins/object-store.h"
#include "builtins/transform-stream-default-controller.h"
#include "builtins/transform-stream.h"
#include "builtins/url.h"
#include "builtins/worker-location.h"

using namespace std::literals;

using std::chrono::ceil;
using std::chrono::milliseconds;
using std::chrono::system_clock;

using JS::CallArgs;
using JS::CallArgsFromVp;
using JS::UniqueChars;

using JS::ObjectOrNullValue;
using JS::ObjectValue;
using JS::PrivateValue;
using JS::Value;

using JS::RootedObject;
using JS::RootedString;
using JS::RootedValue;

using JS::HandleObject;
using JS::HandleString;
using JS::HandleValue;
using JS::HandleValueArray;
using JS::MutableHandleValue;

using JS::PersistentRooted;

const JSErrorFormatString *GetErrorMessage(void *userRef, unsigned errorNumber) {
  if (errorNumber > 0 && errorNumber < JSErrNum_Limit) {
    return &js_ErrorFormatString[errorNumber];
  }

  return nullptr;
}

enum class Mode { PreWizening, PostWizening };

Mode execution_mode = Mode::PreWizening;

bool hasWizeningFinished() { return execution_mode == Mode::PostWizening; }

bool isWizening() { return execution_mode == Mode::PreWizening; }

void markWizeningAsFinished() { execution_mode = Mode::PostWizening; }

namespace Request {
JSObject *response_promise(JSObject *obj);
}

template <InternalMethod fun>
bool enqueue_internal_method(JSContext *cx, HandleObject receiver,
                             HandleValue extra = JS::UndefinedHandleValue, unsigned int nargs = 0,
                             const char *name = "") {
  RootedObject method(cx, create_internal_method<fun>(cx, receiver, extra, nargs, name));
  if (!method) {
    return false;
  }

  RootedObject promise(cx, JS::CallOriginalPromiseResolve(cx, JS::UndefinedHandleValue));
  if (!promise) {
    return false;
  }

  return JS::AddPromiseReactions(cx, promise, method, nullptr);
}

using jsurl::SpecSlice, jsurl::SpecString, jsurl::JSUrl, jsurl::JSUrlSearchParams,
    jsurl::JSSearchParam;

static JS::PersistentRootedObjectVector *pending_async_tasks;

// TODO(performance): introduce a version that writes into an existing buffer, and use that
// with the hostcall buffer where possible.
// https://github.com/fastly/js-compute-runtime/issues/215
UniqueChars encode(JSContext *cx, HandleString str, size_t *encoded_len) {
  UniqueChars text = JS_EncodeStringToUTF8(cx, str);
  if (!text)
    return nullptr;

  // This shouldn't fail, since the encode operation ensured `str` is linear.
  JSLinearString *linear = JS_EnsureLinearString(cx, str);
  *encoded_len = JS::GetDeflatedUTF8StringLength(linear);
  return text;
}

UniqueChars encode(JSContext *cx, HandleValue val, size_t *encoded_len) {
  RootedString str(cx, JS::ToString(cx, val));
  if (!str)
    return nullptr;

  return encode(cx, str, encoded_len);
}

SpecString encode(JSContext *cx, HandleValue val) {
  SpecString slice(nullptr, 0, 0);
  auto chars = encode(cx, val, &slice.len);
  if (!chars)
    return slice;
  slice.data = (uint8_t *)chars.release();
  slice.cap = slice.len;
  return slice;
}

uint8_t *value_to_buffer(JSContext *cx, HandleValue val, const char *val_desc, size_t *len) {
  if (!val.isObject() ||
      !(JS_IsArrayBufferViewObject(&val.toObject()) || JS::IsArrayBufferObject(&val.toObject()))) {
    JS_ReportErrorNumberUTF8(cx, GetErrorMessage, nullptr, JSMSG_INVALID_BUFFER_ARG, val_desc,
                             val.type());
    return nullptr;
  }

  RootedObject input(cx, &val.toObject());
  uint8_t *data;
  bool is_shared;

  if (JS_IsArrayBufferViewObject(input)) {
    js::GetArrayBufferViewLengthAndData(input, len, &is_shared, &data);
  } else {
    JS::GetArrayBufferLengthAndData(input, len, &is_shared, &data);
  }

  return data;
}

bool RejectPromiseWithPendingError(JSContext *cx, HandleObject promise) {
  RootedValue exn(cx);
  if (!JS_IsExceptionPending(cx) || !JS_GetPendingException(cx, &exn)) {
    return false;
  }
  JS_ClearPendingException(cx);
  return JS::RejectPromise(cx, promise, exn);
}

JSObject *PromiseRejectedWithPendingError(JSContext *cx) {
  RootedObject promise(cx, JS::NewPromiseObject(cx, nullptr));
  if (!promise || !RejectPromiseWithPendingError(cx, promise)) {
    return nullptr;
  }
  return promise;
}

#define HANDLE_READ_CHUNK_SIZE 8192

template <auto op>
static char *read_from_handle_all(JSContext *cx, uint32_t handle, size_t *nwritten,
                                  bool read_until_zero) {
  // TODO(performance): investigate passing a size hint in situations where we might know
  // the final size, e.g. via the `content-length` header.
  // https://github.com/fastly/js-compute-runtime/issues/216
  size_t buf_size = HANDLE_READ_CHUNK_SIZE;

  // TODO(performance): make use of malloc slack.
  // https://github.com/fastly/js-compute-runtime/issues/217
  char *buf = static_cast<char *>(JS_malloc(cx, buf_size));
  if (!buf) {
    JS_ReportOutOfMemory(cx);
    return nullptr;
  }

  // For realloc below.
  char *new_buf;

  size_t offset = 0;
  fastly_error_t err;
  while (true) {
    fastly_list_u8_t out_list;
    if (!op(handle, HANDLE_READ_CHUNK_SIZE, &out_list, &err)) {
      HANDLE_ERROR(cx, err);
      JS_free(cx, buf);
      return nullptr;
    }

    // copy the allocated discontiguous buffer into our contiguous buffer
    // TODO(performance): a cabi_realloc hack should be possible in the
    // component API to ensure continguous allocation into the contiguous
    // buffer in the first place
    memcpy(buf + offset, out_list.ptr, out_list.len);
    JS_free(cx, out_list.ptr);

    offset += out_list.len;
    if (out_list.len == 0 || (!read_until_zero && out_list.len < HANDLE_READ_CHUNK_SIZE)) {
      break;
    }

    // TODO(performance): make use of malloc slack, and use a smarter buffer growth strategy.
    // https://github.com/fastly/js-compute-runtime/issues/217
    size_t new_size = buf_size + HANDLE_READ_CHUNK_SIZE;
    new_buf = static_cast<char *>(JS_realloc(cx, buf, buf_size, new_size));
    if (!new_buf) {
      JS_free(cx, buf);
      JS_ReportOutOfMemory(cx);
      return nullptr;
    }
    buf = new_buf;

    buf_size += HANDLE_READ_CHUNK_SIZE;
  }

  new_buf = static_cast<char *>(JS_realloc(cx, buf, buf_size, offset + 1));
  if (!buf) {
    JS_free(cx, buf);
    JS_ReportOutOfMemory(cx);
    return nullptr;
  }
  buf = new_buf;

  buf[offset] = '\0';
  *nwritten = offset;

  return buf;
}

/**
 * Writes the given number of bytes from the given buffer to the given handle.
 *
 * The host doesn't necessarily write all bytes in any particular call to
 * xqd_body_write, so to ensure all bytes are written, we call it in a loop.
 */
bool write_to_body_all(fastly_body_handle_t handle, const char *buf, size_t len,
                       fastly_error_t *err) {
  size_t total_written = 0;
  while (total_written < len) {
    const uint8_t *chunk = reinterpret_cast<const uint8_t *>(buf) + total_written;
    size_t chunk_len = len - total_written;
    uint32_t nwritten = 0;
    fastly_list_u8_t write_list = {.ptr = const_cast<uint8_t *>(chunk), .len = chunk_len};
    if (!xqd_fastly_http_body_write(handle, &write_list, FASTLY_BODY_WRITE_END_BACK, &nwritten,
                                    err)) {
      return false;
    }
    total_written += nwritten;
  }

  return true;
}

enum class BodyReadResult {
  ArrayBuffer,
  JSON,
  Text,
};

namespace Headers {
enum class Mode : int32_t { Standalone, ProxyToRequest, ProxyToResponse };
namespace Slots {
enum { BackingMap, Handle, Mode, HasLazyValues, Count };
};

bool is_instance(JSObject *obj);
bool is_instance(Value val);

namespace detail {
#define HEADERS_ITERATION_METHOD(argc)                                                             \
  METHOD_HEADER(argc)                                                                              \
  RootedObject backing_map(cx, detail::backing_map(self));                                         \
  if (!detail::ensure_all_header_values_from_handle(cx, self, backing_map))                        \
    return false;

static const char VALID_NAME_CHARS[128] = {
    0, 0, 0, 0, 0, 0, 0, 0, //   0
    0, 0, 0, 0, 0, 0, 0, 0, //   8
    0, 0, 0, 0, 0, 0, 0, 0, //  16
    0, 0, 0, 0, 0, 0, 0, 0, //  24

    0, 1, 0, 1, 1, 1, 1, 1, //  32
    0, 0, 1, 1, 0, 1, 1, 0, //  40
    1, 1, 1, 1, 1, 1, 1, 1, //  48
    1, 1, 0, 0, 0, 0, 0, 0, //  56

    0, 1, 1, 1, 1, 1, 1, 1, //  64
    1, 1, 1, 1, 1, 1, 1, 1, //  72
    1, 1, 1, 1, 1, 1, 1, 1, //  80
    1, 1, 1, 0, 0, 0, 1, 1, //  88

    1, 1, 1, 1, 1, 1, 1, 1, //  96
    1, 1, 1, 1, 1, 1, 1, 1, // 104
    1, 1, 1, 1, 1, 1, 1, 1, // 112
    1, 1, 1, 0, 1, 0, 1, 0  // 120
};

#define NORMALIZE_NAME(name, fun_name)                                                             \
  RootedValue normalized_name(cx, name);                                                           \
  size_t name_len;                                                                                 \
  UniqueChars name_chars =                                                                         \
      detail::normalize_header_name(cx, &normalized_name, &name_len, fun_name);                    \
  if (!name_chars)                                                                                 \
    return false;

#define NORMALIZE_VALUE(value, fun_name)                                                           \
  RootedValue normalized_value(cx, value);                                                         \
  size_t value_len;                                                                                \
  UniqueChars value_chars =                                                                        \
      detail::normalize_header_value(cx, &normalized_value, &value_len, fun_name);                 \
  if (!value_chars)                                                                                \
    return false;

JSObject *backing_map(JSObject *self);
Mode mode(JSObject *self);
bool lazy_values(JSObject *self);
uint32_t handle(JSObject *self);
UniqueChars normalize_header_name(JSContext *cx, MutableHandleValue name_val, size_t *name_len,
                                  const char *fun_name);
UniqueChars normalize_header_value(JSContext *cx, MutableHandleValue value_val, size_t *value_len,
                                   const char *fun_name);

static PersistentRooted<JSString *> comma;
bool append_header_value_to_map(JSContext *cx, HandleObject self, HandleValue normalized_name,
                                MutableHandleValue normalized_value);
bool get_header_names_from_handle(JSContext *cx, uint32_t handle, Mode mode,
                                  HandleObject backing_map);
static bool retrieve_value_for_header_from_handle(JSContext *cx, HandleObject self,
                                                  HandleValue name, MutableHandleValue value);
static bool ensure_value_for_header(JSContext *cx, HandleObject self, HandleValue normalized_name,
                                    MutableHandleValue values);
bool get_header_value_for_name(JSContext *cx, HandleObject self, HandleValue name,
                               MutableHandleValue rval, const char *fun_name);
static bool ensure_all_header_values_from_handle(JSContext *cx, HandleObject self,
                                                 HandleObject backing_map);
typedef bool AppendHeaderOperation(fastly_request_handle_t handle, xqd_world_string_t *name,
                                   xqd_world_string_t *value, fastly_error_t *err);
bool append_header_value(JSContext *cx, HandleObject self, HandleValue name, HandleValue value,
                         const char *fun_name);
} // namespace detail

bool delazify(JSContext *cx, HandleObject headers);
JSObject *create(JSContext *cx, HandleObject headers, Mode mode, HandleObject owner,
                 HandleValue initv);
const unsigned ctor_length = 1;
bool check_receiver(JSContext *cx, HandleValue receiver, const char *method_name);
bool get(JSContext *cx, unsigned argc, Value *vp);
bool set(JSContext *cx, unsigned argc, Value *vp);
bool has(JSContext *cx, unsigned argc, Value *vp);
bool append(JSContext *cx, unsigned argc, Value *vp);
bool maybe_add(JSContext *cx, HandleObject self, const char *name, const char *value);
typedef bool HeaderRemoveOperation(fastly_request_handle_t handle, xqd_world_string_t *name,
                                   fastly_error_t *err);
bool delete_(JSContext *cx, unsigned argc, Value *vp);
bool forEach(JSContext *cx, unsigned argc, Value *vp);
bool entries(JSContext *cx, unsigned argc, Value *vp);
bool keys(JSContext *cx, unsigned argc, Value *vp);
bool values(JSContext *cx, unsigned argc, Value *vp);
const JSFunctionSpec methods[] = {
    JS_FN("get", get, 1, JSPROP_ENUMERATE), JS_FN("has", has, 1, JSPROP_ENUMERATE),
    JS_FN("set", set, 2, JSPROP_ENUMERATE), JS_FN("append", append, 2, JSPROP_ENUMERATE),
    JS_FN("delete", delete_, 1, JSPROP_ENUMERATE), JS_FN("forEach", forEach, 1, JSPROP_ENUMERATE),
    JS_FN("entries", entries, 0, JSPROP_ENUMERATE), JS_FN("keys", keys, 0, JSPROP_ENUMERATE),
    JS_FN("values", values, 0, JSPROP_ENUMERATE),
    // [Symbol.iterator] added in init_class.
    JS_FS_END};
const JSPropertySpec properties[] = {JS_PS_END};
bool constructor(JSContext *cx, unsigned argc, Value *vp);
CLASS_BOILERPLATE_CUSTOM_INIT(Headers)
JSObject *create(JSContext *cx, HandleObject headers, Mode mode, HandleObject owner,
                 HandleObject init_headers);
JSObject *create(JSContext *cx, HandleObject headers, Mode mode, HandleObject owner,
                 HandleValue initv);
JSObject *create(JSContext *cx, HandleObject self, Mode mode, HandleObject owner);
} // namespace Headers

namespace Request {
bool is_instance(JSObject *obj);
}

namespace Response {
bool is_instance(JSObject *obj);
}

namespace RequestOrResponse {
bool is_instance(JSObject *obj) {
  return Request::is_instance(obj) || Response::is_instance(obj) ||
         ObjectStoreEntry::is_instance(obj);
}

uint32_t handle(JSObject *obj) {
  MOZ_ASSERT(is_instance(obj));
  return static_cast<uint32_t>(JS::GetReservedSlot(obj, Slots::RequestOrResponse).toInt32());
}

bool has_body(JSObject *obj) {
  MOZ_ASSERT(is_instance(obj));
  return JS::GetReservedSlot(obj, Slots::HasBody).toBoolean();
}

fastly_body_handle_t body_handle(JSObject *obj) {
  MOZ_ASSERT(is_instance(obj));
  return JS::GetReservedSlot(obj, Slots::Body).toInt32();
}

JSObject *body_stream(JSObject *obj) {
  MOZ_ASSERT(is_instance(obj));
  return JS::GetReservedSlot(obj, Slots::BodyStream).toObjectOrNull();
}

JSObject *body_source(JSContext *cx, HandleObject obj) {
  MOZ_ASSERT(has_body(obj));
  RootedObject stream(cx, body_stream(obj));
  return builtins::NativeStreamSource::get_stream_source(cx, stream);
}

bool body_used(JSObject *obj) {
  MOZ_ASSERT(is_instance(obj));
  return JS::GetReservedSlot(obj, Slots::BodyUsed).toBoolean();
}

bool mark_body_used(JSContext *cx, HandleObject obj) {
  MOZ_ASSERT(!body_used(obj));
  JS::SetReservedSlot(obj, Slots::BodyUsed, JS::BooleanValue(true));

  RootedObject stream(cx, body_stream(obj));
  if (stream && builtins::NativeStreamSource::stream_is_body(cx, stream)) {
    if (!builtins::NativeStreamSource::lock_stream(cx, stream)) {
      // The only reason why marking the body as used could fail here is that
      // it's a disturbed ReadableStream. To improve error reporting, we clear
      // the current exception and throw a better one.
      JS_ClearPendingException(cx);
      JS_ReportErrorLatin1(cx, "The ReadableStream body is already locked and can't be consumed");
      return false;
    }
  }

  return true;
}

/**
 * Moves an underlying body handle from one Request/Response object to another.
 *
 * Also marks the source object's body as consumed.
 */
bool move_body_handle(JSContext *cx, HandleObject from, HandleObject to) {
  MOZ_ASSERT(is_instance(from));
  MOZ_ASSERT(is_instance(to));
  MOZ_ASSERT(!body_used(from));

  // Replace the receiving object's body handle with the body stream source's
  // underlying handle.
  // TODO: Let the host know we'll not use the old handle anymore, once C@E has
  // a hostcall for that.
  fastly_body_handle_t body = body_handle(from);
  JS::SetReservedSlot(to, Slots::Body, JS::Int32Value(body));

  // Mark the source's body as used, and the stream as locked to prevent any
  // future attempts to use the underlying handle we just removed.
  return mark_body_used(cx, from);
}

Value url(JSObject *obj) {
  MOZ_ASSERT(is_instance(obj));
  Value val = JS::GetReservedSlot(obj, Slots::URL);
  MOZ_ASSERT(val.isString());
  return val;
}

void set_url(JSObject *obj, Value url) {
  MOZ_ASSERT(is_instance(obj));
  MOZ_ASSERT(url.isString());
  JS::SetReservedSlot(obj, Slots::URL, url);
}

/**
 * Implementation of the `body is unusable` concept at
 * https://fetch.spec.whatwg.org/#body-unusable
 */
bool body_unusable(JSContext *cx, HandleObject body) {
  MOZ_ASSERT(JS::IsReadableStream(body));
  bool disturbed;
  bool locked;
  MOZ_RELEASE_ASSERT(JS::ReadableStreamIsDisturbed(cx, body, &disturbed) &&
                     JS::ReadableStreamIsLocked(cx, body, &locked));
  return disturbed || locked;
}

/**
 * Implementation of the `extract a body` algorithm at
 * https://fetch.spec.whatwg.org/#concept-bodyinit-extract
 *
 * Note: our implementation is somewhat different from what the spec describes
 * in that we immediately write all non-streaming body types to the host instead
 * of creating a stream for them. We don't have threads, so there's nothing "in
 * parallel" to be had anyway.
 *
 * Note: also includes the steps applying the `Content-Type` header from the
 * Request and Response constructors in step 36 and 8 of those, respectively.
 */
bool extract_body(JSContext *cx, HandleObject self, HandleValue body_val) {
  MOZ_ASSERT(is_instance(self));
  MOZ_ASSERT(!has_body(self));
  MOZ_ASSERT(!body_val.isNullOrUndefined());

  const char *content_type = nullptr;

  // We currently support five types of body inputs:
  // - byte sequence
  // - buffer source
  // - USV strings
  // - URLSearchParams
  // - ReadableStream
  // After the other other options are checked explicitly, all other inputs are
  // encoded to a UTF8 string to be treated as a USV string.
  // TODO: Support the other possible inputs to Body.

  RootedObject body_obj(cx, body_val.isObject() ? &body_val.toObject() : nullptr);

  if (body_obj && JS::IsReadableStream(body_obj)) {
    if (body_unusable(cx, body_obj)) {
      JS_ReportErrorLatin1(cx, "Can't use a ReadableStream that's locked or has ever been "
                               "read from or canceled as a Request or Response body.");
      return false;
    }

    JS_SetReservedSlot(self, RequestOrResponse::Slots::BodyStream, body_val);

    // Ensure that we take the right steps for shortcutting operations on
    // TransformStreams later on.
    if (builtins::TransformStream::is_ts_readable(cx, body_obj)) {
      // But only if the TransformStream isn't used as a mixin by other
      // builtins.
      if (!builtins::TransformStream::used_as_mixin(
              builtins::TransformStream::ts_from_readable(cx, body_obj))) {
        builtins::TransformStream::set_readable_used_as_body(cx, body_obj, self);
      }
    }
  } else {
    mozilla::Maybe<JS::AutoCheckCannotGC> maybeNoGC;
    UniqueChars text;
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
      SpecSlice slice = URLSearchParams::serialize(cx, body_obj);
      buf = (char *)slice.data;
      length = slice.len;
      content_type = "application/x-www-form-urlencoded;charset=UTF-8";
    } else {
      text = encode(cx, body_val, &length);
      if (!text)
        return false;
      buf = text.get();
      content_type = "text/plain;charset=UTF-8";
    }

    fastly_body_handle_t body_handle = RequestOrResponse::body_handle(self);
    fastly_error_t err;
    bool ok = write_to_body_all(body_handle, buf, length, &err);

    // Ensure that the NoGC is reset, so throwing an error in HANDLE_ERROR
    // succeeds.
    if (maybeNoGC.isSome()) {
      maybeNoGC.reset();
    }

    if (!ok) {
      HANDLE_ERROR(cx, err);
      return false;
    }
  }

  // Step 36.3 of Request constructor / 8.4 of Response constructor.
  if (content_type) {
    RootedObject headers(cx, &JS::GetReservedSlot(self, Slots::Headers).toObject());
    if (!Headers::maybe_add(cx, headers, "content-type", content_type)) {
      return false;
    }
  }

  JS::SetReservedSlot(self, Slots::HasBody, JS::BooleanValue(true));
  return true;
}

/**
 * Returns the RequestOrResponse's Headers if it has been reified, nullptr if
 * not.
 */
JSObject *maybe_headers(JSObject *obj) {
  MOZ_ASSERT(is_instance(obj));
  return JS::GetReservedSlot(obj, Slots::Headers).toObjectOrNull();
}

/**
 * Returns the RequestOrResponse's Headers, reifying it if necessary.
 */
template <auto mode> JSObject *headers(JSContext *cx, HandleObject obj) {
  JSObject *headers = maybe_headers(obj);
  if (!headers) {
    RootedObject headersInstance(
        cx, JS_NewObjectWithGivenProto(cx, &Headers::class_, Headers::proto_obj));
    if (!headersInstance)
      return nullptr;
    headers = Headers::create(cx, headersInstance, mode, obj);
    if (!headers)
      return nullptr;
    JS_SetReservedSlot(obj, Slots::Headers, ObjectValue(*headers));
  }

  return headers;
}

bool append_body(JSContext *cx, HandleObject self, HandleObject source) {
  MOZ_ASSERT(!body_used(source));
  fastly_body_handle_t source_body = body_handle(source);
  fastly_body_handle_t dest_body = body_handle(self);
  fastly_error_t err;
  if (!xqd_fastly_http_body_append(dest_body, source_body, &err)) {
    HANDLE_ERROR(cx, err);
    return false;
  }
  return true;
}

typedef bool ParseBodyCB(JSContext *cx, HandleObject self, UniqueChars buf, size_t len);

template <RequestOrResponse::BodyReadResult result_type>
bool parse_body(JSContext *cx, HandleObject self, UniqueChars buf, size_t len) {
  RootedObject result_promise(cx, &JS::GetReservedSlot(self, Slots::BodyAllPromise).toObject());
  JS::SetReservedSlot(self, Slots::BodyAllPromise, JS::UndefinedValue());
  RootedValue result(cx);

  if (result_type == RequestOrResponse::BodyReadResult::ArrayBuffer) {
    auto *rawBuf = buf.release();
    RootedObject array_buffer(cx, JS::NewArrayBufferWithContents(cx, len, rawBuf));
    if (!array_buffer) {
      JS_free(cx, rawBuf);
      return RejectPromiseWithPendingError(cx, result_promise);
    }
    result.setObject(*array_buffer);
  } else {
    RootedString text(cx, JS_NewStringCopyUTF8N(cx, JS::UTF8Chars(buf.get(), len)));
    if (!text) {
      return RejectPromiseWithPendingError(cx, result_promise);
    }

    if (result_type == RequestOrResponse::BodyReadResult::Text) {
      result.setString(text);
    } else {
      MOZ_ASSERT(result_type == RequestOrResponse::BodyReadResult::JSON);
      if (!JS_ParseJSON(cx, text, &result)) {
        return RejectPromiseWithPendingError(cx, result_promise);
      }
    }
  }

  return JS::ResolvePromise(cx, result_promise, result);
}

bool content_stream_read_then_handler(JSContext *cx, HandleObject self, HandleValue extra,
                                      CallArgs args) {
  RootedObject then_handler(cx, &args.callee());
  // The reader is stored in the catch handler, which we need here as well.
  // So we get that first, then the reader.
  MOZ_ASSERT(extra.isObject());
  RootedObject catch_handler(cx, &extra.toObject());
#ifdef DEBUG
  bool foundContents;
  if (!JS_HasElement(cx, catch_handler, 1, &foundContents)) {
    return false;
  }
  MOZ_ASSERT(foundContents);
#endif
  RootedValue contents_val(cx);
  if (!JS_GetElement(cx, catch_handler, 1, &contents_val)) {
    return false;
  }
  MOZ_ASSERT(contents_val.isObject());
  RootedObject contents(cx, &contents_val.toObject());
  if (!contents) {
    return false;
  }
#ifdef DEBUG
  bool contentsIsArray;
  if (!JS::IsArrayObject(cx, contents, &contentsIsArray)) {
    return false;
  }
  MOZ_ASSERT(contentsIsArray);
#endif

  auto reader_val = js::GetFunctionNativeReserved(catch_handler, 1);
  MOZ_ASSERT(reader_val.isObject());
  RootedObject reader(cx, &reader_val.toObject());

  // We're guaranteed to work with a native ReadableStreamDefaultReader here as we used
  // `JS::ReadableStreamDefaultReaderRead(cx, reader)`, which in turn is guaranteed to return {done:
  // bool, value: any} objects to read promise then callbacks.
  MOZ_ASSERT(args[0].isObject());
  RootedObject chunk_obj(cx, &args[0].toObject());
  RootedValue done_val(cx);
  RootedValue value(cx);
#ifdef DEBUG
  bool hasValue;
  if (!JS_HasProperty(cx, chunk_obj, "value", &hasValue)) {
    return false;
  }
  MOZ_ASSERT(hasValue);
#endif
  if (!JS_GetProperty(cx, chunk_obj, "value", &value)) {
    return false;
  }
#ifdef DEBUG
  bool hasDone;
  if (!JS_HasProperty(cx, chunk_obj, "done", &hasDone)) {
    return false;
  }
  MOZ_ASSERT(hasDone);
#endif
  if (!JS_GetProperty(cx, chunk_obj, "done", &done_val)) {
    return false;
  }
  MOZ_ASSERT(done_val.isBoolean());
  if (done_val.toBoolean()) {
    // We finished reading the stream
    // Now we need to iterate/reduce `contents` JS Array into UniqueChars
    uint32_t contentsLength;
    if (!JS::GetArrayLength(cx, contents, &contentsLength)) {
      return false;
    }
    // TODO(performance): investigate whether we can infer the size directly from `contents`
    size_t buf_size = HANDLE_READ_CHUNK_SIZE;
    // TODO(performance): make use of malloc slack.
    // https://github.com/fastly/js-compute-runtime/issues/217
    size_t offset = 0;
    // In this loop we are finding the length of each entry in `contents` and resizing the `buf`
    // until it is large enough to fit all the entries in `contents`
    for (uint32_t index = 0; index < contentsLength; index++) {
      RootedValue val(cx);
      if (!JS_GetElement(cx, contents, index, &val)) {
        return false;
      }
      {
        JS::AutoCheckCannotGC nogc;
        MOZ_ASSERT(val.isObject());
        JSObject *array = &val.toObject();
        MOZ_ASSERT(JS_IsUint8Array(array));
        size_t length = JS_GetTypedArrayByteLength(array);
        if (length) {
          offset += length;
          // if buf is not big enough to fit the next uint8array's bytes then resize
          if (offset > buf_size) {
            buf_size =
                buf_size + (HANDLE_READ_CHUNK_SIZE * ((length / HANDLE_READ_CHUNK_SIZE) + 1));
          }
        }
      }
    }

    auto buf = static_cast<char *>(JS_malloc(cx, buf_size + 1));
    if (!buf) {
      JS_ReportOutOfMemory(cx);
      return false;
    }
    // reset the offset for the next loop
    offset = 0;
    // In this loop we are inserting each entry in `contents` into `buf`
    for (uint32_t index = 0; index < contentsLength; index++) {
      RootedValue val(cx);
      if (!JS_GetElement(cx, contents, index, &val)) {
        JS_free(cx, buf);
        return false;
      }
      {
        JS::AutoCheckCannotGC nogc;
        MOZ_ASSERT(val.isObject());
        JSObject *array = &val.toObject();
        MOZ_ASSERT(JS_IsUint8Array(array));
        bool is_shared;
        size_t length = JS_GetTypedArrayByteLength(array);
        if (length) {
          static_assert(CHAR_BIT == 8, "Strange char");
          auto bytes = reinterpret_cast<char *>(JS_GetUint8ArrayData(array, &is_shared, nogc));
          memcpy(buf + offset, bytes, length);
          offset += length;
        }
      }
    }
    buf[offset] = '\0';
#ifdef DEBUG
    bool foundBodyParser;
    if (!JS_HasElement(cx, catch_handler, 2, &foundBodyParser)) {
      JS_free(cx, buf);
      return false;
    }
    MOZ_ASSERT(foundBodyParser);
#endif
    // Now we can call parse_body on the result
    RootedValue body_parser(cx);
    if (!JS_GetElement(cx, catch_handler, 2, &body_parser)) {
      JS_free(cx, buf);
      return false;
    }
    auto parse_body = (ParseBodyCB *)body_parser.toPrivate();
    UniqueChars body(buf);
    return parse_body(cx, self, std::move(body), offset);
  }

  RootedValue val(cx);
  if (!JS_GetProperty(cx, chunk_obj, "value", &val)) {
    return false;
  }

  // The read operation can return anything since this stream comes from the guest
  // If it is not a UInt8Array -- reject with a TypeError
  if (!val.isObject() || !JS_IsUint8Array(&val.toObject())) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_RESPONSE_VALUE_NOT_UINT8ARRAY);
    RootedObject result_promise(cx);
    result_promise = &JS::GetReservedSlot(self, Slots::BodyAllPromise).toObject();
    JS::SetReservedSlot(self, Slots::BodyAllPromise, JS::UndefinedValue());
    return RejectPromiseWithPendingError(cx, result_promise);
  }

  {
    uint32_t contentsLength;
    if (!JS::GetArrayLength(cx, contents, &contentsLength)) {
      return false;
    }
    if (!JS_SetElement(cx, contents, contentsLength, val)) {
      return false;
    }
  }

  // Read the next chunk.
  RootedObject promise(cx, JS::ReadableStreamDefaultReaderRead(cx, reader));
  if (!promise)
    return false;
  return JS::AddPromiseReactions(cx, promise, then_handler, catch_handler);
}

bool content_stream_read_catch_handler(JSContext *cx, HandleObject self, HandleValue extra,
                                       CallArgs args) {
  // The stream errored when being consumed
  // we need to propagate the stream error
  MOZ_ASSERT(extra.isObject());
  RootedObject reader(cx, &extra.toObject());
  RootedValue stream_val(cx);
  if (!JS_GetElement(cx, reader, 1, &stream_val)) {
    return false;
  }
  MOZ_ASSERT(stream_val.isObject());
  RootedObject stream(cx, &stream_val.toObject());
  if (!stream) {
    return false;
  }
  MOZ_ASSERT(JS::IsReadableStream(stream));
#ifdef DEBUG
  bool isError;
  if (!JS::ReadableStreamIsErrored(cx, stream, &isError)) {
    return false;
  }
  MOZ_ASSERT(isError);
#endif
  RootedValue error(cx, JS::ReadableStreamGetStoredError(cx, stream));
  JS_ClearPendingException(cx);
  JS_SetPendingException(cx, error, JS::ExceptionStackBehavior::DoNotCapture);
  RootedObject result_promise(cx);
  result_promise = &JS::GetReservedSlot(self, Slots::BodyAllPromise).toObject();
  JS::SetReservedSlot(self, Slots::BodyAllPromise, JS::UndefinedValue());
  return RejectPromiseWithPendingError(cx, result_promise);
}

bool consume_content_stream_for_bodyAll(JSContext *cx, HandleObject self, HandleValue stream_val,
                                        CallArgs args) {
  // The body_parser is stored in the stream object, which we need here as well.
  RootedObject stream(cx, &stream_val.toObject());
  RootedValue body_parser(cx);
  if (!JS_GetElement(cx, stream, 1, &body_parser)) {
    return false;
  }
  MOZ_ASSERT(JS::IsReadableStream(stream));
  if (RequestOrResponse::body_unusable(cx, stream)) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr,
                              JSMSG_RESPONSE_BODY_DISTURBED_OR_LOCKED);
    RootedObject result_promise(cx);
    result_promise = &JS::GetReservedSlot(self, Slots::BodyAllPromise).toObject();
    JS::SetReservedSlot(self, Slots::BodyAllPromise, JS::UndefinedValue());
    return RejectPromiseWithPendingError(cx, result_promise);
  }
  JS::Rooted<JSObject *> unwrappedReader(
      cx, JS::ReadableStreamGetReader(cx, stream, JS::ReadableStreamReaderMode::Default));
  if (!unwrappedReader) {
    return false;
  }

  // contents is the JS Array we store the stream chunks within, to later convert to
  // arrayBuffer/json/text
  JS::Rooted<JSObject *> contents(cx, JS::NewArrayObject(cx, 0));
  if (!contents) {
    return false;
  }

  RootedValue extra(cx, ObjectValue(*unwrappedReader));
  // TODO: confirm whether this is observable to the JS application
  if (!JS_SetElement(cx, unwrappedReader, 1, stream)) {
    return false;
  }

  // Create handlers for both `then` and `catch`.
  // These are functions with two reserved slots, in which we store all
  // information required to perform the reactions. We store the actually
  // required information on the catch handler, and a reference to that on the
  // then handler. This allows us to reuse these functions for the next read
  // operation in the then handler. The catch handler won't ever have a need to
  // perform another operation in this way.
  RootedObject catch_handler(
      cx, create_internal_method<content_stream_read_catch_handler>(cx, self, extra));
  if (!catch_handler) {
    return false;
  }

  extra.setObject(*catch_handler);
  if (!JS_SetElement(cx, catch_handler, 1, contents)) {
    return false;
  }
  if (!JS_SetElement(cx, catch_handler, 2, body_parser)) {
    return false;
  }
  RootedObject then_handler(
      cx, create_internal_method<content_stream_read_then_handler>(cx, self, extra));
  if (!then_handler) {
    return false;
  }

  // Read the next chunk.
  RootedObject promise(cx, JS::ReadableStreamDefaultReaderRead(cx, unwrappedReader));
  if (!promise) {
    return false;
  }
  return JS::AddPromiseReactions(cx, promise, then_handler, catch_handler);
}

bool consume_body_handle_for_bodyAll(JSContext *cx, HandleObject self, HandleValue body_parser,
                                     CallArgs args) {
  fastly_body_handle_t body = body_handle(self);
  auto parse_body = (ParseBodyCB *)body_parser.toPrivate();
  size_t bytes_read;
  UniqueChars buf(read_from_handle_all<xqd_fastly_http_body_read>(cx, body, &bytes_read, true));
  if (!buf) {
    RootedObject result_promise(cx);
    result_promise = &JS::GetReservedSlot(self, Slots::BodyAllPromise).toObject();
    JS::SetReservedSlot(self, Slots::BodyAllPromise, JS::UndefinedValue());
    return RejectPromiseWithPendingError(cx, result_promise);
  }

  return parse_body(cx, self, std::move(buf), bytes_read);
}

template <RequestOrResponse::BodyReadResult result_type>
bool bodyAll(JSContext *cx, CallArgs args, HandleObject self) {
  // TODO: mark body as consumed when operating on stream, too.
  if (body_used(self)) {
    JS_ReportErrorASCII(cx, "Body has already been consumed");
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }

  RootedObject bodyAll_promise(cx, JS::NewPromiseObject(cx, nullptr));
  if (!bodyAll_promise) {
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }
  JS::SetReservedSlot(self, Slots::BodyAllPromise, ObjectValue(*bodyAll_promise));

  // If the Request/Response doesn't have a body, empty default results need to
  // be returned.
  if (!has_body(self)) {
    UniqueChars chars;
    if (!parse_body<result_type>(cx, self, std::move(chars), 0)) {
      return ReturnPromiseRejectedWithPendingError(cx, args);
    }

    args.rval().setObject(*bodyAll_promise);
    return true;
  }

  if (!mark_body_used(cx, self)) {
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }

  RootedValue body_parser(cx, JS::PrivateValue((void *)parse_body<result_type>));

  // If the body is a ReadableStream that's not backed by a fastly_body_handle_t,
  // we need to manually read all chunks from the stream.
  // TODO(performance): ensure that we're properly shortcutting reads from TransformStream
  // readables.
  // https://github.com/fastly/js-compute-runtime/issues/218
  RootedObject stream(cx, body_stream(self));
  if (stream && !builtins::NativeStreamSource::stream_is_body(cx, stream)) {
    if (!JS_SetElement(cx, stream, 1, body_parser)) {
      return false;
    }
    RootedValue extra(cx, ObjectValue(*stream));
    if (!enqueue_internal_method<consume_content_stream_for_bodyAll>(cx, self, extra)) {
      return ReturnPromiseRejectedWithPendingError(cx, args);
    }
  } else {
    if (!enqueue_internal_method<consume_body_handle_for_bodyAll>(cx, self, body_parser)) {
      return ReturnPromiseRejectedWithPendingError(cx, args);
    }
  }

  args.rval().setObject(*bodyAll_promise);
  return true;
}

bool body_source_pull_algorithm(JSContext *cx, CallArgs args, HandleObject source,
                                HandleObject body_owner, HandleObject controller) {
  // If the stream has been piped to a TransformStream whose readable end was
  // then passed to a Request or Response as the body, we can just append the
  // entire source body to the destination using a single native hostcall, and
  // then close the source stream, instead of reading and writing it in
  // individual chunks. Note that even in situations where multiple streams are
  // piped to the same destination this is guaranteed to happen in the right
  // order: ReadableStream#pipeTo locks the destination WritableStream until the
  // source ReadableStream is closed/canceled, so only one stream can ever be
  // piped in at the same time.
  RootedObject pipe_dest(cx, builtins::NativeStreamSource::piped_to_transform_stream(source));
  if (pipe_dest) {
    if (builtins::TransformStream::readable_used_as_body(pipe_dest)) {
      RootedObject dest_owner(cx, builtins::TransformStream::owner(pipe_dest));
      if (!RequestOrResponse::append_body(cx, dest_owner, body_owner)) {
        return false;
      }

      RootedObject stream(cx, builtins::NativeStreamSource::stream(source));
      bool success = JS::ReadableStreamClose(cx, stream);
      MOZ_RELEASE_ASSERT(success);

      args.rval().setUndefined();
      return true;
    }
  }

  // The actual read from the body needs to be delayed, because it'd otherwise
  // be a blocking operation in case the backend didn't yet send any data.
  // That would lead to situations where we block on I/O before processing
  // all pending Promises, which in turn can result in operations happening in
  // observably different behavior, up to and including causing deadlocks
  // because a body read response is blocked on content making another request.
  //
  // (This deadlock happens in automated tests, but admittedly might not happen
  // in real usage.)

  if (!pending_async_tasks->append(source))
    return false;

  args.rval().setUndefined();
  return true;
}

bool body_source_cancel_algorithm(JSContext *cx, CallArgs args, HandleObject stream,
                                  HandleObject owner, HandleValue reason) {
  args.rval().setUndefined();
  return true;
}

bool body_reader_then_handler(JSContext *cx, HandleObject body_owner, HandleValue extra,
                              CallArgs args) {
  RootedObject then_handler(cx, &args.callee());
  // The reader is stored in the catch handler, which we need here as well.
  // So we get that first, then the reader.
  RootedObject catch_handler(cx, &extra.toObject());
  RootedObject reader(cx, &js::GetFunctionNativeReserved(catch_handler, 1).toObject());
  fastly_body_handle_t body_handle = RequestOrResponse::body_handle(body_owner);

  // We're guaranteed to work with a native ReadableStreamDefaultReader here,
  // which in turn is guaranteed to vend {done: bool, value: any} objects to
  // read promise then callbacks.
  RootedObject chunk_obj(cx, &args[0].toObject());
  RootedValue done_val(cx);
  if (!JS_GetProperty(cx, chunk_obj, "done", &done_val))
    return false;

  if (done_val.toBoolean()) {
    // The only response we ever send is the one passed to
    // `FetchEvent#respondWith` to send to the client. As such, we can be
    // certain that if we have a response here, we can advance the FetchState to
    // `responseDone`.
    if (Response::is_instance(body_owner)) {
      FetchEvent::set_state(FetchEvent::instance(), FetchEvent::State::responseDone);
    }

    fastly_error_t err;
    if (!xqd_fastly_http_body_close(body_handle, &err)) {
      HANDLE_ERROR(cx, err);
      return false;
    }

    if (Request::is_instance(body_owner)) {
      if (!pending_async_tasks->append(body_owner)) {
        return false;
      }
    }

    return true;
  }

  RootedValue val(cx);
  if (!JS_GetProperty(cx, chunk_obj, "value", &val))
    return false;

  // The read operation returned something that's not a Uint8Array
  if (!val.isObject() || !JS_IsUint8Array(&val.toObject())) {
    // reject the request promise
    if (Request::is_instance(body_owner)) {
      RootedObject response_promise(cx, Request::response_promise(body_owner));
      RootedValue exn(cx);

      // TODO: this should be a TypeError, but I'm not sure how to make that work
      JS_ReportErrorUTF8(cx, "TypeError");
      if (!JS_GetPendingException(cx, &exn)) {
        return false;
      }
      JS_ClearPendingException(cx);

      return JS::RejectPromise(cx, response_promise, exn);
    }

    // TODO: should we also create a rejected promise if a response reads something that's not a
    // Uint8Array?
    fprintf(stderr, "Error: read operation on body ReadableStream didn't respond with a "
                    "Uint8Array. Received value: ");
    dump_value(cx, val, stderr);
    return false;
  }

  bool ok;
  fastly_error_t err;
  {
    JS::AutoCheckCannotGC nogc;
    JSObject *array = &val.toObject();
    bool is_shared;
    uint8_t *bytes = JS_GetUint8ArrayData(array, &is_shared, nogc);
    size_t length = JS_GetTypedArrayByteLength(array);
    ok = write_to_body_all(body_handle, (char *)bytes, length, &err);
  }

  // Needs to be outside the nogc block in case we need to create an exception.
  if (!ok) {
    HANDLE_ERROR(cx, err);
    return false;
  }

  // Read the next chunk.
  RootedObject promise(cx, JS::ReadableStreamDefaultReaderRead(cx, reader));
  if (!promise)
    return false;
  return JS::AddPromiseReactions(cx, promise, then_handler, catch_handler);
}

bool body_reader_catch_handler(JSContext *cx, HandleObject body_owner, HandleValue extra,
                               CallArgs args) {
  // TODO: check if this should create a rejected promise instead, so an
  // in-content handler for unhandled rejections could deal with it. The body
  // stream errored during the streaming send. Not much we can do, but at least
  // close the stream, and warn.
  fprintf(stderr, "Warning: body ReadableStream closed during body streaming. Exception: ");
  dump_value(cx, args.get(0), stderr);

  // The only response we ever send is the one passed to
  // `FetchEvent#respondWith` to send to the client. As such, we can be certain
  // that if we have a response here, we can advance the FetchState to
  // `responseDone`. (Note that even though we encountered an error,
  // `responseDone` is the right state: `responsedWithError` is for when sending
  // a response at all failed.)
  if (Response::is_instance(body_owner)) {
    FetchEvent::set_state(FetchEvent::instance(), FetchEvent::State::responseDone);
  }
  return true;
}

/**
 * Ensures that the given |body_owner|'s body is properly streamed, if it
 * requires streaming.
 *
 * If streaming is required, starts the process of reading from the
 * ReadableStream representing the body and sets the |requires_streaming| bool
 * to `true`.
 */
bool maybe_stream_body(JSContext *cx, HandleObject body_owner, bool *requires_streaming) {
  RootedObject stream(cx, RequestOrResponse::body_stream(body_owner));
  if (!stream) {
    return true;
  }

  if (RequestOrResponse::body_unusable(cx, stream)) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr,
                              JSMSG_RESPONSE_BODY_DISTURBED_OR_LOCKED);
    return false;
  }

  // If the body stream is backed by a C@E body handle, we can directly pipe
  // that handle into the body we're about to send.
  if (builtins::NativeStreamSource::stream_is_body(cx, stream)) {
    // First, move the source's body handle to the target and lock the stream.
    RootedObject stream_source(cx, builtins::NativeStreamSource::get_stream_source(cx, stream));
    RootedObject source_owner(cx, builtins::NativeStreamSource::owner(stream_source));
    if (!RequestOrResponse::move_body_handle(cx, source_owner, body_owner)) {
      return false;
    }

    // Then, send the request/response without streaming. We know that content
    // won't append to this body handle, because we don't expose any means to do
    // so, so it's ok for it to be closed immediately.
    return true;
  }

  RootedObject reader(
      cx, JS::ReadableStreamGetReader(cx, stream, JS::ReadableStreamReaderMode::Default));
  if (!reader)
    return false;

  bool is_closed;
  if (!JS::ReadableStreamReaderIsClosed(cx, reader, &is_closed))
    return false;

  // It's ok for the stream to be closed, as its contents might
  // already have fully been written to the body handle.
  // In that case, we can do a blocking send instead.
  if (is_closed) {
    return true;
  }

  // Create handlers for both `then` and `catch`.
  // These are functions with two reserved slots, in which we store all
  // information required to perform the reactions. We store the actually
  // required information on the catch handler, and a reference to that on the
  // then handler. This allows us to reuse these functions for the next read
  // operation in the then handler. The catch handler won't ever have a need to
  // perform another operation in this way.
  RootedObject catch_handler(cx);
  RootedValue extra(cx, ObjectValue(*reader));
  catch_handler = create_internal_method<body_reader_catch_handler>(cx, body_owner, extra);
  if (!catch_handler)
    return false;

  RootedObject then_handler(cx);
  extra.setObject(*catch_handler);
  then_handler = create_internal_method<body_reader_then_handler>(cx, body_owner, extra);
  if (!then_handler)
    return false;

  RootedObject promise(cx, JS::ReadableStreamDefaultReaderRead(cx, reader));
  if (!promise)
    return false;
  if (!JS::AddPromiseReactions(cx, promise, then_handler, catch_handler))
    return false;

  *requires_streaming = true;
  return true;
}

JSObject *create_body_stream(JSContext *cx, HandleObject owner) {
  MOZ_ASSERT(is_instance(owner));
  MOZ_ASSERT(!body_stream(owner));
  RootedObject source(cx, builtins::NativeStreamSource::create(cx, owner, JS::UndefinedHandleValue,
                                                               body_source_pull_algorithm,
                                                               body_source_cancel_algorithm));
  if (!source)
    return nullptr;

  // Create a readable stream with a highwater mark of 0.0 to prevent an eager
  // pull. With the default HWM of 1.0, the streams implementation causes a
  // pull, which means we enqueue a read from the host handle, which we quite
  // often have no interest in at all.
  RootedObject body_stream(cx, JS::NewReadableDefaultStreamObject(cx, source, nullptr, 0.0));
  if (!body_stream) {
    return nullptr;
  }

  // TODO: immediately lock the stream if the owner's body is already used.

  JS_SetReservedSlot(owner, Slots::BodyStream, JS::ObjectValue(*body_stream));
  return body_stream;
}

bool body_get(JSContext *cx, CallArgs args, HandleObject self, bool create_if_undefined) {
  MOZ_ASSERT(is_instance(self));
  if (!has_body(self)) {
    args.rval().setNull();
    return true;
  }

  RootedObject body_stream(cx, ::RequestOrResponse::body_stream(self));
  if (!body_stream && create_if_undefined) {
    body_stream = create_body_stream(cx, self);
    if (!body_stream)
      return false;
  }

  args.rval().setObjectOrNull(body_stream);
  return true;
}
} // namespace RequestOrResponse

// https://fetch.spec.whatwg.org/#concept-method-normalize
// Returns `true` if the method name was normalized, `false` otherwise.
static bool normalize_http_method(char *method) {
  static const char *names[6] = {"DELETE", "GET", "HEAD", "OPTIONS", "POST", "PUT"};

  for (size_t i = 0; i < 6; i++) {
    auto name = names[i];
    if (strcasecmp(method, name) == 0) {
      if (strcmp(method, name) == 0) {
        return false;
      }

      // Note: Safe because `strcasecmp` returning 0 above guarantees
      // same-length strings.
      strcpy(method, name);
      return true;
    }
  }

  return false;
}

namespace Request {
namespace Slots {
enum {
  Request = RequestOrResponse::Slots::RequestOrResponse,
  Body = RequestOrResponse::Slots::Body,
  BodyStream = RequestOrResponse::Slots::BodyStream,
  HasBody = RequestOrResponse::Slots::HasBody,
  BodyUsed = RequestOrResponse::Slots::BodyUsed,
  Headers = RequestOrResponse::Slots::Headers,
  URL = RequestOrResponse::Slots::URL,
  Backend = RequestOrResponse::Slots::Count,
  Method,
  CacheOverride,
  PendingRequest,
  ResponsePromise,
  IsDownstream,
  Count
};
};

fastly_request_handle_t request_handle(JSObject *obj) {
  return JS::GetReservedSlot(obj, Slots::Request).toInt32();
}

fastly_pending_request_handle_t pending_handle(JSObject *obj) {
  Value handle_val = JS::GetReservedSlot(obj, Slots::PendingRequest);
  if (handle_val.isInt32())
    return handle_val.toInt32();
  return INVALID_HANDLE;
}

bool is_pending(JSObject *obj) { return pending_handle(obj) != INVALID_HANDLE; }

bool is_downstream(JSObject *obj) {
  return JS::GetReservedSlot(obj, Slots::IsDownstream).toBoolean();
}

JSObject *response_promise(JSObject *obj) {
  return &JS::GetReservedSlot(obj, Slots::ResponsePromise).toObject();
}

JSString *backend(JSObject *obj) {
  Value val = JS::GetReservedSlot(obj, Slots::Backend);
  return val.isString() ? val.toString() : nullptr;
}

JSString *method(JSContext *cx, HandleObject obj) {
  return JS::GetReservedSlot(obj, Slots::Method).toString();
}

bool set_cache_key(JSContext *cx, HandleObject self, HandleValue cache_key_val) {
  MOZ_ASSERT(is_instance(self));
  size_t key_len;
  // Convert the key argument into a String following https://tc39.es/ecma262/#sec-tostring
  JS::UniqueChars keyString = encode(cx, cache_key_val, &key_len);
  if (!keyString) {
    return false;
  }
  std::string_view key(keyString.get(), key_len);
  std::string hex_str;
  picosha2::hash256_hex_string(key, hex_str);
  std::transform(hex_str.begin(), hex_str.end(), hex_str.begin(),
                 [](unsigned char c) { return std::toupper(c); });

  JSObject *headers = RequestOrResponse::headers<Headers::Mode::ProxyToRequest>(cx, self);
  if (!headers) {
    return false;
  }
  RootedObject headers_val(cx, headers);
  RootedValue name_val(cx, JS::StringValue(JS_NewStringCopyN(cx, "fastly-xqd-cache-key", 20)));
  RootedValue value_val(cx,
                        JS::StringValue(JS_NewStringCopyN(cx, hex_str.c_str(), hex_str.length())));
  if (!Headers::detail::append_header_value(cx, headers_val, name_val, value_val,
                                            "Request.prototype.setCacheKey")) {
    return false;
  }

  return true;
}

bool set_cache_override(JSContext *cx, HandleObject self, HandleValue cache_override_val) {
  MOZ_ASSERT(is_instance(self));
  if (!builtins::CacheOverride::is_instance(cache_override_val)) {
    JS_ReportErrorUTF8(cx, "Value passed in as cacheOverride must be an "
                           "instance of CacheOverride");
    return false;
  }

  RootedObject input(cx, &cache_override_val.toObject());
  JSObject *override = builtins::CacheOverride::clone(cx, input);
  if (!override) {
    return false;
  }

  JS::SetReservedSlot(self, Slots::CacheOverride, ObjectValue(*override));
  return true;
}

/**
 * Apply the CacheOverride to a host-side request handle.
 */
bool apply_cache_override(JSContext *cx, HandleObject self) {
  MOZ_ASSERT(is_instance(self));
  RootedObject override(cx, JS::GetReservedSlot(self, Slots::CacheOverride).toObjectOrNull());
  if (!override) {
    return true;
  }

  uint8_t tag = builtins::CacheOverride::abi_tag(override);

  bool has_ttl = true;
  uint32_t ttl;
  RootedValue val(cx, builtins::CacheOverride::ttl(override));
  if (val.isUndefined()) {
    has_ttl = false;
  } else {
    ttl = val.toInt32();
  }

  bool has_swr = true;
  uint32_t swr;
  val = builtins::CacheOverride::swr(override);
  if (val.isUndefined()) {
    has_swr = false;
  } else {
    swr = val.toInt32();
  }

  xqd_world_string_t sk_str;
  val = builtins::CacheOverride::surrogate_key(override);
  if (val.isUndefined()) {
    sk_str.len = 0;
  } else {
    UniqueChars sk_chars;
    sk_chars = encode(cx, val, &sk_str.len);
    if (!sk_chars)
      return false;
    sk_str.ptr = sk_chars.release();
  }

  fastly_error_t err;
  if (!xqd_fastly_http_req_cache_override_set(request_handle(self), tag, has_ttl ? &ttl : NULL,
                                              has_swr ? &swr : NULL, sk_str.len ? &sk_str : NULL,
                                              &err)) {
    HANDLE_ERROR(cx, err);
    return false;
  }
  return true;
}

const unsigned ctor_length = 1;

bool check_receiver(JSContext *cx, HandleValue receiver, const char *method_name);

bool method_get(JSContext *cx, unsigned argc, Value *vp) {
  METHOD_HEADER(0)

  JSString *method = Request::method(cx, self);
  if (!method)
    return false;

  args.rval().setString(method);
  return true;
}

bool url_get(JSContext *cx, unsigned argc, Value *vp) {
  METHOD_HEADER(0)

  args.rval().set(RequestOrResponse::url(self));
  return true;
}

bool version_get(JSContext *cx, unsigned argc, Value *vp) {
  METHOD_HEADER(0)

  fastly_error_t err;
  fastly_http_version_t version = 0;
  if (!xqd_fastly_http_req_version_get(request_handle(self), &version, &err)) {
    HANDLE_ERROR(cx, err);
    return false;
  }

  args.rval().setInt32(version);
  return true;
}

bool headers_get(JSContext *cx, unsigned argc, Value *vp) {
  METHOD_HEADER(0)

  JSObject *headers = RequestOrResponse::headers<Headers::Mode::ProxyToRequest>(cx, self);
  if (!headers)
    return false;

  args.rval().setObject(*headers);
  return true;
}

template <RequestOrResponse::BodyReadResult result_type>
bool bodyAll(JSContext *cx, unsigned argc, Value *vp) {
  METHOD_HEADER(0)
  return RequestOrResponse::bodyAll<result_type>(cx, args, self);
}

bool body_get(JSContext *cx, unsigned argc, Value *vp) {
  METHOD_HEADER(0)
  return RequestOrResponse::body_get(cx, args, self, is_downstream(self));
}

bool bodyUsed_get(JSContext *cx, unsigned argc, Value *vp) {
  METHOD_HEADER(0)
  args.rval().setBoolean(RequestOrResponse::body_used(self));
  return true;
}

bool setCacheOverride(JSContext *cx, unsigned argc, Value *vp) {
  METHOD_HEADER(1)

  if (!set_cache_override(cx, self, args[0]))
    return false;

  args.rval().setUndefined();
  return true;
}

bool setCacheKey(JSContext *cx, unsigned argc, Value *vp) {
  METHOD_HEADER(1)

  if (!set_cache_key(cx, self, args[0])) {
    return false;
  }

  args.rval().setUndefined();
  return true;
}
JSString *GET_atom;
JSObject *create_instance(JSContext *cx);
JSObject *create(JSContext *cx, HandleObject requestInstance, HandleValue input,
                 HandleValue init_val);

bool clone(JSContext *cx, unsigned argc, Value *vp) {
  METHOD_HEADER(0)

  if (RequestOrResponse::body_used(self)) {
    JS_ReportErrorLatin1(cx, "Request.prototype.clone: the request's body isn't usable.");
    return false;
  }

  // Here we get the current requests body stream and call ReadableStream.prototype.tee to return
  // two versions of the stream. Once we get the two streams, we create a new request handle and
  // attach one of the streams to the new handle and the other stream is attached to the request
  // handle that `clone()` was called upon.
  RootedObject body_stream(cx, ::RequestOrResponse::body_stream(self));
  if (!body_stream) {
    body_stream = ::RequestOrResponse::create_body_stream(cx, self);
    if (!body_stream) {
      return false;
    }
  }
  RootedValue tee_val(cx);
  if (!JS_GetProperty(cx, body_stream, "tee", &tee_val)) {
    return false;
  }
  JS::Rooted<JSFunction *> tee(cx, JS_GetObjectFunction(&tee_val.toObject()));
  if (!tee) {
    return false;
  }
  JS::RootedVector<JS::Value> argv(cx);
  RootedValue rval(cx);
  if (!JS::Call(cx, body_stream, tee, argv, &rval)) {
    return false;
  }
  RootedObject rval_array(cx, &rval.toObject());
  RootedValue body1_val(cx);
  if (!JS_GetProperty(cx, rval_array, "0", &body1_val)) {
    return false;
  }
  RootedValue body2_val(cx);
  if (!JS_GetProperty(cx, rval_array, "1", &body2_val)) {
    return false;
  }

  fastly_error_t err;
  fastly_request_handle_t request_handle = INVALID_HANDLE;
  if (!xqd_fastly_http_req_new(&request_handle, &err)) {
    HANDLE_ERROR(cx, err);
    return false;
  }

  fastly_body_handle_t body_handle = INVALID_HANDLE;
  if (!xqd_fastly_http_body_new(&body_handle, &err)) {
    HANDLE_ERROR(cx, err);
    return false;
  }

  if (!JS::IsReadableStream(&body1_val.toObject())) {
    return false;
  }
  body_stream.set(&body1_val.toObject());
  if (RequestOrResponse::body_unusable(cx, body_stream)) {
    JS_ReportErrorLatin1(cx, "Can't use a ReadableStream that's locked or has ever been "
                             "read from or canceled as a Request body.");
    return false;
  }

  RootedObject requestInstance(cx, create_instance(cx));
  JS::SetReservedSlot(requestInstance, Slots::Request, JS::Int32Value(request_handle));
  JS::SetReservedSlot(requestInstance, Slots::Body, JS::Int32Value(body_handle));
  JS::SetReservedSlot(requestInstance, Slots::BodyStream, body1_val);
  JS::SetReservedSlot(requestInstance, Slots::BodyUsed, JS::FalseValue());
  JS::SetReservedSlot(requestInstance, Slots::HasBody, JS::BooleanValue(true));
  JS::SetReservedSlot(requestInstance, Slots::URL, JS::GetReservedSlot(self, Slots::URL));
  JS::SetReservedSlot(requestInstance, Slots::IsDownstream,
                      JS::GetReservedSlot(self, Slots::IsDownstream));

  JS::SetReservedSlot(self, Slots::BodyStream, body2_val);
  JS::SetReservedSlot(self, Slots::BodyUsed, JS::FalseValue());
  JS::SetReservedSlot(self, Slots::HasBody, JS::BooleanValue(true));

  RootedObject headers(cx);
  RootedObject headers_obj(cx, RequestOrResponse::headers<Headers::Mode::ProxyToRequest>(cx, self));
  if (!headers_obj) {
    return false;
  }
  RootedObject headersInstance(
      cx, JS_NewObjectWithGivenProto(cx, &Headers::class_, Headers::proto_obj));
  if (!headersInstance)
    return false;

  headers = Headers::create(cx, headersInstance, Headers::Mode::ProxyToRequest, requestInstance,
                            headers_obj);

  if (!headers) {
    return false;
  }

  JS::SetReservedSlot(requestInstance, Slots::Headers, JS::ObjectValue(*headers));

  JSString *method = Request::method(cx, self);
  if (!method) {
    return false;
  }

  JS::SetReservedSlot(requestInstance, Slots::Method, JS::StringValue(method));
  RootedValue cache_override(cx, JS::GetReservedSlot(self, Slots::CacheOverride));
  if (!cache_override.isNullOrUndefined()) {
    if (!set_cache_override(cx, requestInstance, cache_override)) {
      return false;
    }
  } else {
    JS::SetReservedSlot(requestInstance, Slots::CacheOverride, cache_override);
  }

  args.rval().setObject(*requestInstance);
  return true;
}

const JSFunctionSpec methods[] = {
    JS_FN("arrayBuffer", bodyAll<RequestOrResponse::BodyReadResult::ArrayBuffer>, 0,
          JSPROP_ENUMERATE),
    JS_FN("json", bodyAll<RequestOrResponse::BodyReadResult::JSON>, 0, JSPROP_ENUMERATE),
    JS_FN("text", bodyAll<RequestOrResponse::BodyReadResult::Text>, 0, JSPROP_ENUMERATE),
    JS_FN("setCacheOverride", setCacheOverride, 3, JSPROP_ENUMERATE),
    JS_FN("setCacheKey", setCacheKey, 0, JSPROP_ENUMERATE),
    JS_FN("clone", clone, 0, JSPROP_ENUMERATE),
    JS_FS_END};

const JSPropertySpec properties[] = {JS_PSG("method", method_get, JSPROP_ENUMERATE),
                                     JS_PSG("url", url_get, JSPROP_ENUMERATE),
                                     JS_PSG("version", version_get, JSPROP_ENUMERATE),
                                     JS_PSG("headers", headers_get, JSPROP_ENUMERATE),
                                     JS_PSG("body", body_get, JSPROP_ENUMERATE),
                                     JS_PSG("bodyUsed", bodyUsed_get, JSPROP_ENUMERATE),
                                     JS_STRING_SYM_PS(toStringTag, "Request", JSPROP_READONLY),
                                     JS_PS_END};

bool constructor(JSContext *cx, unsigned argc, Value *vp);

CLASS_BOILERPLATE_CUSTOM_INIT(Request)

bool init_class(JSContext *cx, HandleObject global) {
  if (!init_class_impl(cx, global)) {
    return false;
  }

  // Initialize a pinned (i.e., never-moved, living forever) atom for the
  // default HTTP method.
  GET_atom = JS_AtomizeAndPinString(cx, "GET");
  return !!GET_atom;
}

JSObject *create(JSContext *cx, HandleObject requestInstance,
                 fastly_request_handle_t request_handle, fastly_body_handle_t body_handle,
                 bool is_downstream) {
  JS::SetReservedSlot(requestInstance, Slots::Request, JS::Int32Value(request_handle));
  JS::SetReservedSlot(requestInstance, Slots::Headers, JS::NullValue());
  JS::SetReservedSlot(requestInstance, Slots::Body, JS::Int32Value(body_handle));
  JS::SetReservedSlot(requestInstance, Slots::BodyStream, JS::NullValue());
  JS::SetReservedSlot(requestInstance, Slots::HasBody, JS::FalseValue());
  JS::SetReservedSlot(requestInstance, Slots::BodyUsed, JS::FalseValue());
  JS::SetReservedSlot(requestInstance, Slots::Method, JS::StringValue(GET_atom));
  JS::SetReservedSlot(requestInstance, Slots::CacheOverride, JS::NullValue());
  JS::SetReservedSlot(requestInstance, Slots::IsDownstream, JS::BooleanValue(is_downstream));

  return requestInstance;
}

/**
 * Create a new Request object, roughly according to
 * https://fetch.spec.whatwg.org/#dom-request
 *
 * "Roughly" because not all aspects of Request handling make sense in C@E.
 * The places where we deviate from the spec are called out inline.
 */
JSObject *create(JSContext *cx, HandleObject requestInstance, HandleValue input,
                 HandleValue init_val) {
  fastly_error_t err;
  fastly_request_handle_t request_handle = INVALID_HANDLE;
  if (!xqd_fastly_http_req_new(&request_handle, &err)) {
    HANDLE_ERROR(cx, err);
    return nullptr;
  }

  fastly_body_handle_t body_handle = INVALID_HANDLE;
  if (!xqd_fastly_http_body_new(&body_handle, &err)) {
    HANDLE_ERROR(cx, err);
    return nullptr;
  }

  RootedObject request(cx, create(cx, requestInstance, request_handle, body_handle, false));
  if (!request) {
    return nullptr;
  }

  RootedString url_str(cx);
  size_t method_len;
  RootedString method_str(cx);
  bool method_needs_normalization = false;

  RootedObject input_request(cx);
  RootedObject input_headers(cx);
  bool input_has_body = false;

  // 1.  Let `request` be null.
  // 4.  Let `signal` be null.
  // (implicit)

  // 2.  Let `fallbackMode` be null.
  // (N/A)

  // 3.  Let `baseURL` be this’s relevant settings object’s API base URL.
  // (implicit)

  // 6.  Otherwise:
  // (reordered because it's easier to check is_instance and otherwise
  // stringify.)
  if (is_instance(input)) {
    input_request = &input.toObject();
    input_has_body = RequestOrResponse::has_body(input_request);

    // 1.  Assert: `input` is a `Request` object.
    // 2.  Set `request` to `input`’s request.
    // (implicit)

    // 3.  Set `signal` to `input`’s signal.
    // (signals not yet supported)

    // 12.  Set `request` to a new request with the following properties:
    // (moved into step 6 because we can leave everything at the default values
    // if step 5 runs.) URL: `request`’s URL. Will actually be applied below.
    url_str = RequestOrResponse::url(input_request).toString();

    // method: `request`’s method.
    method_str = Request::method(cx, input_request);
    if (!method_str) {
      return nullptr;
    }
    method_len = JS::GetStringLength(method_str);

    // referrer: `request`’s referrer.
    // TODO: evaluate whether we want to implement support for setting the
    // `referer` [sic] header based on this or not.

    // cache mode: `request`’s cache mode.
    // TODO: implement support for cache mode-based headers setting.

    // header list: A copy of `request`’s header list.
    // Note: copying the headers is postponed, see step 32 below.
    input_headers = RequestOrResponse::headers<Headers::Mode::ProxyToRequest>(cx, input_request);
    if (!input_headers) {
      return nullptr;
    }

    // The following properties aren't applicable:
    // unsafe-request flag: Set.
    // client: This’s relevant settings object.
    // window: `window`.
    // priority: `request`’s priority
    // origin: `request`’s origin.
    // referrer policy: `request`’s referrer policy.
    // mode: `request`’s mode.
    // credentials mode: `request`’s credentials mode.
    // redirect mode: `request`’s redirect mode.
    // integrity metadata: `request`’s integrity metadata.
    // keepalive: `request`’s keepalive.
    // reload-navigation flag: `request`’s reload-navigation flag.
    // history-navigation flag: `request`’s history-navigation flag.
    // URL list: A clone of `request`’s URL list.
  }

  // 5.  If `input` is a string, then:
  else {
    // 1.  Let `parsedURL` be the result of parsing `input` with `baseURL`.
    RootedObject url_instance(cx, JS_NewObjectWithGivenProto(cx, &URL::class_, URL::proto_obj));
    if (!url_instance)
      return nullptr;

    RootedObject parsedURL(cx, URL::create(cx, url_instance, input, builtins::Fastly::baseURL));

    // 2.  If `parsedURL` is failure, then throw a `TypeError`.
    if (!parsedURL) {
      return nullptr;
    }

    // 3.  If `parsedURL` includes credentials, then throw a `TypeError`.
    // (N/A)

    // 4.  Set `request` to a new request whose URL is `parsedURL`.
    // Instead, we store `url_str` to apply below.
    RootedValue url_val(cx, JS::ObjectValue(*parsedURL));
    url_str = JS::ToString(cx, url_val);
    if (!url_str) {
      return nullptr;
    }

    // 5.  Set `fallbackMode` to "`cors`".
    // (N/A)
  }

  // Actually set the URL derived in steps 5 or 6 above.
  RequestOrResponse::set_url(request, StringValue(url_str));
  xqd_world_string_t url_xqd_str;
  UniqueChars url = encode(cx, url_str, &url_xqd_str.len);
  if (!url) {
    return nullptr;
  } else {
    url_xqd_str.ptr = url.get();
    fastly_error_t err;
    if (!xqd_fastly_http_req_uri_set(request_handle, &url_xqd_str, &err)) {
      HANDLE_ERROR(cx, err);
      return nullptr;
    }
  }

  // 7.  Let `origin` be this’s relevant settings object’s origin.
  // 8.  Let `window` be "`client`".
  // 9.  If `request`’s window is an environment settings object and its origin
  // is same origin with `origin`, then set `window` to `request`’s window.
  // 10.  If `init`["window"] exists and is non-null, then throw a `TypeError.
  // 11.  If `init`["window"] exists, then set `window` to "`no-window`".
  // (N/A)

  // Extract all relevant properties from the init object.
  // TODO: evaluate how much we care about precisely matching evaluation order.
  // If "a lot", we need to make sure that all side effects that value
  // conversions might trigger occur in the right order—presumably by running
  // them all right here as WebIDL bindings would.
  RootedValue method_val(cx);
  RootedValue headers_val(cx);
  RootedValue body_val(cx);
  RootedValue backend_val(cx);
  RootedValue cache_override(cx);
  if (init_val.isObject()) {
    RootedObject init(cx, init_val.toObjectOrNull());
    if (!JS_GetProperty(cx, init, "method", &method_val) ||
        !JS_GetProperty(cx, init, "headers", &headers_val) ||
        !JS_GetProperty(cx, init, "body", &body_val) ||
        !JS_GetProperty(cx, init, "backend", &backend_val) ||
        !JS_GetProperty(cx, init, "cacheOverride", &cache_override)) {
      return nullptr;
    }
  } else if (!init_val.isNullOrUndefined()) {
    JS_ReportErrorLatin1(cx, "Request constructor: |init| parameter can't be converted to "
                             "a dictionary");
    return nullptr;
  }

  // 13.  If `init` is not empty, then:
  // 1.  If `request`’s mode is "`navigate`", then set it to "`same-origin`".
  // 2.  Unset `request`’s reload-navigation flag.
  // 3.  Unset `request`’s history-navigation flag.
  // 4.  Set `request`’s origin to "`client`".
  // 5.  Set `request`’s referrer to "`client`".
  // 6.  Set `request`’s referrer policy to the empty string.
  // 7.  Set `request`’s URL to `request`’s current URL.
  // 8.  Set `request`’s URL list to « `request`’s URL ».
  // (N/A)

  // 14.  If `init["referrer"]` exists, then:
  // TODO: implement support for referrer application.
  // 1.  Let `referrer` be `init["referrer"]`.
  // 2.  If `referrer` is the empty string, then set `request`’s referrer to
  // "`no-referrer`".
  // 3.  Otherwise:
  //   1.  Let `parsedReferrer` be the result of parsing `referrer` with
  //   `baseURL`.
  //   2.  If `parsedReferrer` is failure, then throw a `TypeError`.

  //   3.  If one of the following is true
  //     *   `parsedReferrer`’s scheme is "`about`" and path is the string
  //     "`client`"
  //     *   `parsedReferrer`’s origin is not same origin with `origin`
  //     then set `request`’s referrer to "`client`".
  //   (N/A)

  //   4.  Otherwise, set `request`’s referrer to `parsedReferrer`.

  // 15.  If `init["referrerPolicy"]` exists, then set `request`’s referrer
  // policy to it.
  // 16.  Let `mode` be `init["mode"]` if it exists, and `fallbackMode`
  // otherwise.
  // 17.  If `mode` is "`navigate`", then throw a `TypeError`.
  // 18.  If `mode` is non-null, set `request`’s mode to `mode`.
  // 19.  If `init["credentials"]` exists, then set `request`’s credentials mode
  // to it. (N/A)

  // 20.  If `init["cache"]` exists, then set `request`’s cache mode to it.
  // TODO: implement support for cache mode application.

  // 21.  If `request`’s cache mode is "`only-if-cached`" and `request`’s mode
  // is _not_
  //      "`same-origin`", then throw a TypeError.
  // 22.  If `init["redirect"]` exists, then set `request`’s redirect mode to
  // it.
  // 23.  If `init["integrity"]` exists, then set `request`’s integrity metadata
  // to it.
  // 24.  If `init["keepalive"]` exists, then set `request`’s keepalive to it.
  // (N/A)

  // 25.  If `init["method"]` exists, then:
  if (!method_val.isUndefined()) {
    // 1.  Let `method` be `init["method"]`.
    method_str = JS::ToString(cx, method_val);
    if (!method_str) {
      return nullptr;
    }

    // 2.  If `method` is not a method or `method` is a forbidden method, then
    // throw a
    //     `TypeError`.
    // TODO: evaluate whether we should barr use of methods forbidden by the
    // WHATWG spec.

    // 3.  Normalize `method`.
    // Delayed to below to reduce some code duplication.
    method_needs_normalization = true;

    // 4.  Set `request`’s method to `method`.
    // Done below, unified with the non-init case.
  }

  // Apply the method derived in step 6 or 25.
  // This only needs to happen if the method was set explicitly and isn't the
  // default `GET`.
  bool is_get = true;
  if (method_str && !JS_StringEqualsLiteral(cx, method_str, "GET", &is_get)) {
    return nullptr;
  }

  bool is_get_or_head = is_get;

  if (!is_get) {
    UniqueChars method = encode(cx, method_str, &method_len);
    if (!method) {
      return nullptr;
    }

    if (method_needs_normalization) {
      if (normalize_http_method(method.get())) {
        // Replace the JS string with the normalized name.
        method_str = JS_NewStringCopyN(cx, method.get(), method_len);
        if (!method_str) {
          return nullptr;
        }
      }
    }

    is_get_or_head = strcmp(method.get(), "GET") == 0 || strcmp(method.get(), "HEAD") == 0;

    JS::SetReservedSlot(request, Slots::Method, JS::StringValue(method_str));
    xqd_world_string_t method_xqd_str = {method.get(), method_len};
    fastly_error_t err;
    if (!xqd_fastly_http_req_method_set(request_handle, &method_xqd_str, &err)) {
      HANDLE_ERROR(cx, err);
      return nullptr;
    }
  }

  // 26.  If `init["signal"]` exists, then set `signal` to it.
  // (signals NYI)

  // 27.  Set this’s request to `request`.
  // (implicit)

  // 28.  Set this’s signal to a new `AbortSignal` object with this’s relevant
  // Realm.
  // 29.  If `signal` is not null, then make this’s signal follow `signal`.
  // (signals NYI)

  // 30.  Set this’s headers to a new `Headers` object with this’s relevant
  // Realm, whose header list is `request`’s header list and guard is
  // "`request`". (implicit)

  // 31.  If this’s requests mode is "`no-cors`", then:
  // 1.  If this’s requests method is not a CORS-safelisted method, then throw a
  // `TypeError`.
  // 2.  Set this’s headers’s guard to "`request-no-cors`".
  // (N/A)

  // 32.  If `init` is not empty, then:
  // 1.  Let `headers` be a copy of this’s headers and its associated header
  // list.
  // 2.  If `init["headers"]` exists, then set `headers` to `init["headers"]`.
  // 3.  Empty this’s headers’s header list.
  // 4.  If `headers` is a `Headers` object, then for each `header` in its
  // header list, append (`header`’s name, `header`’s value) to this’s headers.
  // 5.  Otherwise, fill this’s headers with `headers`.
  // Note: the substeps of 32 are somewhat convoluted because they don't just
  // serve to ensure that the contents of `init["headers"]` are added to the
  // request's headers, but also that all headers, including those from the
  // `input` object are sanitized in accordance with the request's `mode`. Since
  // we don't implement this sanitization, we do a much simpler thing: if
  // `init["headers"]` exists, create the request's `headers` from that,
  // otherwise create it from the `init` object's `headers`, or create a new,
  // empty one.
  RootedObject headers(cx);
  if (!headers_val.isUndefined()) {
    RootedObject headersInstance(
        cx, JS_NewObjectWithGivenProto(cx, &Headers::class_, Headers::proto_obj));
    if (!headersInstance)
      return nullptr;

    headers =
        Headers::create(cx, headersInstance, Headers::Mode::ProxyToRequest, request, headers_val);
  } else {
    RootedObject headersInstance(
        cx, JS_NewObjectWithGivenProto(cx, &Headers::class_, Headers::proto_obj));
    if (!headersInstance)
      return nullptr;

    headers =
        Headers::create(cx, headersInstance, Headers::Mode::ProxyToRequest, request, input_headers);
  }

  if (!headers) {
    return nullptr;
  }

  JS::SetReservedSlot(request, Slots::Headers, JS::ObjectValue(*headers));

  // 33.  Let `inputBody` be `input`’s requests body if `input` is a `Request`
  // object;
  //      otherwise null.
  // (skipped)

  // 34.  If either `init["body"]` exists and is non-null or `inputBody` is
  // non-null, and `request`’s method is ``GET`` or ``HEAD``, then throw a
  // TypeError.
  if ((input_has_body || !body_val.isNullOrUndefined()) && is_get_or_head) {
    JS_ReportErrorLatin1(cx, "Request constructor: HEAD or GET Request cannot have a body.");
    return nullptr;
  }

  // 35.  Let `initBody` be null.
  // (skipped)

  // Note: steps 36-41 boil down to "if there's an init body, use that.
  // Otherwise, if there's an input body, use that, but proxied through a
  // TransformStream to make sure it's not consumed by something else in the
  // meantime." Given that, we're restructuring things quite a bit below.

  // 36.  If `init["body"]` exists and is non-null, then:
  if (!body_val.isNullOrUndefined()) {
    // 1.  Let `Content-Type` be null.
    // 2.  Set `initBody` and `Content-Type` to the result of extracting
    // `init["body"]`, with
    //     `keepalive` set to `request`’s keepalive.
    // 3.  If `Content-Type` is non-null and this’s headers’s header list does
    // not contain
    //     ``Content-Type``, then append (``Content-Type``, `Content-Type`) to
    //     this’s headers.
    // Note: these steps are all inlined into RequestOrResponse::extract_body.
    if (!RequestOrResponse::extract_body(cx, request, body_val)) {
      return nullptr;
    }
  } else if (input_has_body) {
    // 37.  Let `inputOrInitBody` be `initBody` if it is non-null; otherwise
    // `inputBody`. (implicit)
    // 38.  If `inputOrInitBody` is non-null and `inputOrInitBody`’s source is
    // null, then:
    // 1.  If this’s requests mode is neither "`same-origin`" nor "`cors`", then
    // throw a `TypeError.
    // 2.  Set this’s requests use-CORS-preflight flag.
    // (N/A)
    // 39.  Let `finalBody` be `inputOrInitBody`.
    // 40.  If `initBody` is null and `inputBody` is non-null, then:
    // (implicit)
    // 1.  If `input` is unusable, then throw a TypeError.
    // 2.  Set `finalBody` to the result of creating a proxy for `inputBody`.

    // All the above steps boil down to "if the input request has an unusable
    // body, throw. Otherwise, use the body." Our implementation is a bit more
    // involved, because we might not have a body reified as a ReadableStream at
    // all, in which case we can directly append the input body to the new
    // request's body with a single hostcall.

    RootedObject inputBody(cx, RequestOrResponse::body_stream(input_request));

    // Throw an error if the input request's body isn't usable.
    if (RequestOrResponse::body_used(input_request) ||
        (inputBody && RequestOrResponse::body_unusable(cx, inputBody))) {
      JS_ReportErrorLatin1(cx, "Request constructor: the input request's body isn't usable.");
      return nullptr;
    }

    if (!inputBody) {
      // If `inputBody` is null, that means that it was never created, and hence
      // content can't have access to it. Instead of reifying it here to pass it
      // into a TransformStream, we just append the body on the host side and
      // mark it as used on the input Request.
      RequestOrResponse::append_body(cx, request, input_request);
      RequestOrResponse::mark_body_used(cx, input_request);
    } else {
      inputBody = builtins::TransformStream::create_rs_proxy(cx, inputBody);
      if (!inputBody) {
        return nullptr;
      }

      builtins::TransformStream::set_readable_used_as_body(cx, inputBody, request);
      JS::SetReservedSlot(request, Slots::BodyStream, JS::ObjectValue(*inputBody));
    }

    JS::SetReservedSlot(request, Slots::HasBody, JS::BooleanValue(true));
  }

  // 41.  Set this’s requests body to `finalBody`.
  // (implicit)

  // Apply the C@E-proprietary `backend` property.
  if (!backend_val.isUndefined()) {
    RootedString backend(cx, JS::ToString(cx, backend_val));
    if (!backend) {
      return nullptr;
    }
    JS::SetReservedSlot(request, Slots::Backend, JS::StringValue(backend));
  } else if (input_request) {
    JS::SetReservedSlot(request, Slots::Backend,
                        JS::GetReservedSlot(input_request, Slots::Backend));
  }

  // Apply the C@E-proprietary `cacheOverride` property.
  if (!cache_override.isUndefined()) {
    if (!set_cache_override(cx, request, cache_override)) {
      return nullptr;
    }
  } else if (input_request) {
    JS::SetReservedSlot(request, Slots::CacheOverride,
                        JS::GetReservedSlot(input_request, Slots::CacheOverride));
  }

  return request;
}

JSObject *create_instance(JSContext *cx) {
  RootedObject requestInstance(
      cx, JS_NewObjectWithGivenProto(cx, &Request::class_, Request::proto_obj));
  return requestInstance;
}

bool constructor(JSContext *cx, unsigned argc, Value *vp) {
  REQUEST_HANDLER_ONLY("The Request builtin");
  CTOR_HEADER("Request", 1);
  RootedObject requestInstance(cx, JS_NewObjectForConstructor(cx, &class_, args));
  RootedObject request(cx, create(cx, requestInstance, args[0], args.get(1)));
  if (!request)
    return false;

  args.rval().setObject(*request);
  return true;
}
} // namespace Request

namespace Response {
namespace Slots {
enum {
  Response = RequestOrResponse::Slots::RequestOrResponse,
  Body = RequestOrResponse::Slots::Body,
  BodyStream = RequestOrResponse::Slots::BodyStream,
  HasBody = RequestOrResponse::Slots::HasBody,
  BodyUsed = RequestOrResponse::Slots::BodyUsed,
  Headers = RequestOrResponse::Slots::Headers,
  IsUpstream = RequestOrResponse::Slots::Count,
  Status,
  StatusMessage,
  Count
};
};

// Needed for uniform access to Request and Response slots.
static_assert((int)Slots::Body == (int)Request::Slots::Body);
static_assert((int)Slots::BodyStream == (int)Request::Slots::BodyStream);
static_assert((int)Slots::HasBody == (int)Request::Slots::HasBody);
static_assert((int)Slots::BodyUsed == (int)Request::Slots::BodyUsed);
static_assert((int)Slots::Headers == (int)Request::Slots::Headers);
static_assert((int)Slots::Response == (int)Request::Slots::Request);

fastly_response_handle_t response_handle(JSObject *obj) {
  MOZ_ASSERT(is_instance(obj));
  return fastly_response_handle_t{(uint32_t)(JS::GetReservedSlot(obj, Slots::Response).toInt32())};
}

bool is_upstream(JSObject *obj) {
  MOZ_ASSERT(is_instance(obj));
  return JS::GetReservedSlot(obj, Slots::IsUpstream).toBoolean();
}

uint16_t status(JSObject *obj) {
  MOZ_ASSERT(is_instance(obj));
  return (uint16_t)JS::GetReservedSlot(obj, Slots::Status).toInt32();
}

JSString *status_message(JSObject *obj) {
  MOZ_ASSERT(is_instance(obj));
  return JS::GetReservedSlot(obj, Slots::StatusMessage).toString();
}

// TODO(jake): Remove this when the reason phrase host-call is implemented
void set_status_message_from_code(JSContext *cx, JSObject *obj, uint16_t code) {
  auto phrase = "";

  switch (code) {
  case 100: // 100 Continue - https://tools.ietf.org/html/rfc7231#section-6.2.1
    phrase = "Continue";
    break;
  case 101: // 101 Switching Protocols - https://tools.ietf.org/html/rfc7231#section-6.2.2
    phrase = "Switching Protocols";
    break;
  case 102: // 102 Processing - https://tools.ietf.org/html/rfc2518
    phrase = "Processing";
    break;
  case 200: // 200 OK - https://tools.ietf.org/html/rfc7231#section-6.3.1
    phrase = "OK";
    break;
  case 201: // 201 Created - https://tools.ietf.org/html/rfc7231#section-6.3.2
    phrase = "Created";
    break;
  case 202: // 202 Accepted - https://tools.ietf.org/html/rfc7231#section-6.3.3
    phrase = "Accepted";
    break;
  case 203: // 203 Non-Authoritative Information - https://tools.ietf.org/html/rfc7231#section-6.3.4
    phrase = "Non Authoritative Information";
    break;
  case 204: // 204 No Content - https://tools.ietf.org/html/rfc7231#section-6.3.5
    phrase = "No Content";
    break;
  case 205: // 205 Reset Content - https://tools.ietf.org/html/rfc7231#section-6.3.6
    phrase = "Reset Content";
    break;
  case 206: // 206 Partial Content - https://tools.ietf.org/html/rfc7233#section-4.1
    phrase = "Partial Content";
    break;
  case 207: // 207 Multi-Status - https://tools.ietf.org/html/rfc4918
    phrase = "Multi-Status";
    break;
  case 208: // 208 Already Reported - https://tools.ietf.org/html/rfc5842
    phrase = "Already Reported";
    break;
  case 226: // 226 IM Used - https://tools.ietf.org/html/rfc3229
    phrase = "IM Used";
    break;
  case 300: // 300 Multiple Choices - https://tools.ietf.org/html/rfc7231#section-6.4.1
    phrase = "Multiple Choices";
    break;
  case 301: // 301 Moved Permanently - https://tools.ietf.org/html/rfc7231#section-6.4.2
    phrase = "Moved Permanently";
    break;
  case 302: // 302 Found - https://tools.ietf.org/html/rfc7231#section-6.4.3
    phrase = "Found";
    break;
  case 303: // 303 See Other - https://tools.ietf.org/html/rfc7231#section-6.4.4
    phrase = "See Other";
    break;
  case 304: // 304 Not Modified - https://tools.ietf.org/html/rfc7232#section-4.1
    phrase = "Not Modified";
    break;
  case 305: // 305 Use Proxy - https://tools.ietf.org/html/rfc7231#section-6.4.5
    phrase = "Use Proxy";
    break;
  case 307: // 307 Temporary Redirect - https://tools.ietf.org/html/rfc7231#section-6.4.7
    phrase = "Temporary Redirect";
    break;
  case 308: // 308 Permanent Redirect - https://tools.ietf.org/html/rfc7238
    phrase = "Permanent Redirect";
    break;
  case 400: // 400 Bad Request - https://tools.ietf.org/html/rfc7231#section-6.5.1
    phrase = "Bad Request";
    break;
  case 401: // 401 Unauthorized - https://tools.ietf.org/html/rfc7235#section-3.1
    phrase = "Unauthorized";
    break;
  case 402: // 402 Payment Required - https://tools.ietf.org/html/rfc7231#section-6.5.2
    phrase = "Payment Required";
    break;
  case 403: // 403 Forbidden - https://tools.ietf.org/html/rfc7231#section-6.5.3
    phrase = "Forbidden";
    break;
  case 404: // 404 Not Found - https://tools.ietf.org/html/rfc7231#section-6.5.4
    phrase = "Not Found";
    break;
  case 405: // 405 Method Not Allowed - https://tools.ietf.org/html/rfc7231#section-6.5.5
    phrase = "Method Not Allowed";
    break;
  case 406: // 406 Not Acceptable - https://tools.ietf.org/html/rfc7231#section-6.5.6
    phrase = "Not Acceptable";
    break;
  case 407: // 407 Proxy Authentication Required - https://tools.ietf.org/html/rfc7235#section-3.2
    phrase = "Proxy Authentication Required";
    break;
  case 408: // 408 Request Timeout - https://tools.ietf.org/html/rfc7231#section-6.5.7
    phrase = "Request Timeout";
    break;
  case 409: // 409 Conflict - https://tools.ietf.org/html/rfc7231#section-6.5.8
    phrase = "Conflict";
    break;
  case 410: // 410 Gone - https://tools.ietf.org/html/rfc7231#section-6.5.9
    phrase = "Gone";
    break;
  case 411: // 411 Length Required - https://tools.ietf.org/html/rfc7231#section-6.5.10
    phrase = "Length Required";
    break;
  case 412: // 412 Precondition Failed - https://tools.ietf.org/html/rfc7232#section-4.2
    phrase = "Precondition Failed";
    break;
  case 413: // 413 Payload Too Large - https://tools.ietf.org/html/rfc7231#section-6.5.11
    phrase = "Payload Too Large";
    break;
  case 414: // 414 URI Too Long - https://tools.ietf.org/html/rfc7231#section-6.5.12
    phrase = "URI Too Long";
    break;
  case 415: // 415 Unsupported Media Type - https://tools.ietf.org/html/rfc7231#section-6.5.13
    phrase = "Unsupported Media Type";
    break;
  case 416: // 416 Range Not Satisfiable - https://tools.ietf.org/html/rfc7233#section-4.4
    phrase = "Range Not Satisfiable";
    break;
  case 417: // 417 Expectation Failed - https://tools.ietf.org/html/rfc7231#section-6.5.14
    phrase = "Expectation Failed";
    break;
  case 418: // 418 I'm a teapot - https://tools.ietf.org/html/rfc2324
    phrase = "I'm a teapot";
    break;
  case 421: // 421 Misdirected Request - http://tools.ietf.org/html/rfc7540#section-9.1.2
    phrase = "Misdirected Request";
    break;
  case 422: // 422 Unprocessable Entity - https://tools.ietf.org/html/rfc4918
    phrase = "Unprocessable Entity";
    break;
  case 423: // 423 Locked - https://tools.ietf.org/html/rfc4918
    phrase = "Locked";
    break;
  case 424: // 424 Failed Dependency - https://tools.ietf.org/html/rfc4918
    phrase = "Failed Dependency";
    break;
  case 426: // 426 Upgrade Required - https://tools.ietf.org/html/rfc7231#section-6.5.15
    phrase = "Upgrade Required";
    break;
  case 428: // 428 Precondition Required - https://tools.ietf.org/html/rfc6585
    phrase = "Precondition Required";
    break;
  case 429: // 429 Too Many Requests - https://tools.ietf.org/html/rfc6585
    phrase = "Too Many Requests";
    break;
  case 431: // 431 Request Header Fields Too Large - https://tools.ietf.org/html/rfc6585
    phrase = "Request Header Fields Too Large";
    break;
  case 451: // 451 Unavailable For Legal Reasons - http://tools.ietf.org/html/rfc7725
    phrase = "Unavailable For Legal Reasons";
    break;
  case 500: // 500 Internal Server Error - https://tools.ietf.org/html/rfc7231#section-6.6.1
    phrase = "Internal Server Error";
    break;
  case 501: // 501 Not Implemented - https://tools.ietf.org/html/rfc7231#section-6.6.2
    phrase = "Not Implemented";
    break;
  case 502: // 502 Bad Gateway - https://tools.ietf.org/html/rfc7231#section-6.6.3
    phrase = "Bad Gateway";
    break;
  case 503: // 503 Service Unavailable - https://tools.ietf.org/html/rfc7231#section-6.6.4
    phrase = "Service Unavailable";
    break;
  case 504: // 504 Gateway Timeout - https://tools.ietf.org/html/rfc7231#section-6.6.5
    phrase = "Gateway Timeout";
    break;
  case 505: // 505 HTTP Version Not Supported - https://tools.ietf.org/html/rfc7231#section-6.6.6
    phrase = "HTTP Version Not Supported";
    break;
  case 506: // 506 Variant Also Negotiates - https://tools.ietf.org/html/rfc2295
    phrase = "Variant Also Negotiates";
    break;
  case 507: // 507 Insufficient Storage - https://tools.ietf.org/html/rfc4918
    phrase = "Insufficient Storage";
    break;
  case 508: // 508 Loop Detected - https://tools.ietf.org/html/rfc5842
    phrase = "Loop Detected";
    break;
  case 510: // 510 Not Extended - https://tools.ietf.org/html/rfc2774
    phrase = "Not Extended";
    break;
  case 511: // 511 Network Authentication Required - https://tools.ietf.org/html/rfc6585
    phrase = "Network Authentication Required";
    break;
  default:
    phrase = "";
    break;
  }
  JS::SetReservedSlot(obj, Slots::StatusMessage,
                      JS::StringValue(JS_NewStringCopyN(cx, phrase, strlen(phrase))));
}

const unsigned ctor_length = 1;

bool check_receiver(JSContext *cx, HandleValue receiver, const char *method_name);

bool ok_get(JSContext *cx, unsigned argc, Value *vp) {
  METHOD_HEADER(0)

  uint16_t status = Response::status(self);
  args.rval().setBoolean(status >= 200 && status < 300);
  return true;
}

bool status_get(JSContext *cx, unsigned argc, Value *vp) {
  METHOD_HEADER(0)

  args.rval().setInt32(status(self));
  return true;
}

bool statusText_get(JSContext *cx, unsigned argc, Value *vp) {
  METHOD_HEADER(0)

  args.rval().setString(status_message(self));
  return true;
}

bool url_get(JSContext *cx, unsigned argc, Value *vp) {
  METHOD_HEADER(0)

  args.rval().set(RequestOrResponse::url(self));
  return true;
}

// TODO: store version client-side.
bool version_get(JSContext *cx, unsigned argc, Value *vp) {
  METHOD_HEADER(0)

  fastly_http_version_t version = 0;
  fastly_error_t err;
  if (!xqd_fastly_http_resp_version_get(response_handle(self), &version, &err)) {
    HANDLE_ERROR(cx, err);
    return false;
  }

  args.rval().setInt32(version);
  return true;
}

JSString *type_default_atom;
JSString *type_error_atom;

bool type_get(JSContext *cx, unsigned argc, Value *vp) {
  METHOD_HEADER(0)

  args.rval().setString(status(self) == 0 ? type_error_atom : type_default_atom);
  return true;
}

bool headers_get(JSContext *cx, unsigned argc, Value *vp) {
  METHOD_HEADER(0)

  JSObject *headers = RequestOrResponse::headers<Headers::Mode::ProxyToResponse>(cx, self);
  if (!headers)
    return false;

  args.rval().setObject(*headers);
  return true;
}

template <RequestOrResponse::BodyReadResult result_type>
bool bodyAll(JSContext *cx, unsigned argc, Value *vp) {
  METHOD_HEADER(0)
  return RequestOrResponse::bodyAll<result_type>(cx, args, self);
}

bool body_get(JSContext *cx, unsigned argc, Value *vp) {
  METHOD_HEADER(0)
  return RequestOrResponse::body_get(cx, args, self, true);
}

bool bodyUsed_get(JSContext *cx, unsigned argc, Value *vp) {
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

const JSPropertySpec properties[] = {JS_PSG("type", type_get, JSPROP_ENUMERATE),
                                     JS_PSG("url", url_get, JSPROP_ENUMERATE),
                                     JS_PSG("status", status_get, JSPROP_ENUMERATE),
                                     JS_PSG("ok", ok_get, JSPROP_ENUMERATE),
                                     JS_PSG("statusText", statusText_get, JSPROP_ENUMERATE),
                                     JS_PSG("version", version_get, JSPROP_ENUMERATE),
                                     JS_PSG("headers", headers_get, JSPROP_ENUMERATE),
                                     JS_PSG("body", body_get, JSPROP_ENUMERATE),
                                     JS_PSG("bodyUsed", bodyUsed_get, JSPROP_ENUMERATE),
                                     JS_STRING_SYM_PS(toStringTag, "Response", JSPROP_READONLY),
                                     JS_PS_END};

bool constructor(JSContext *cx, unsigned argc, Value *vp);

CLASS_BOILERPLATE_CUSTOM_INIT(Response)

JSObject *create(JSContext *cx, HandleObject response, fastly_response_handle_t response_handle,
                 fastly_body_handle_t body_handle, bool is_upstream);

/**
 * The `Response` constructor https://fetch.spec.whatwg.org/#dom-response
 */
bool constructor(JSContext *cx, unsigned argc, Value *vp) {
  REQUEST_HANDLER_ONLY("The Response builtin");

  CTOR_HEADER("Response", 0);

  RootedValue body_val(cx, args.get(0));
  RootedValue init_val(cx, args.get(1));

  RootedValue status_val(cx);
  uint16_t status = 200;

  RootedValue statusText_val(cx);
  RootedString statusText(cx, JS_GetEmptyString(cx));
  RootedValue headers_val(cx);

  if (init_val.isObject()) {
    RootedObject init(cx, init_val.toObjectOrNull());
    if (!JS_GetProperty(cx, init, "status", &status_val) ||
        !JS_GetProperty(cx, init, "statusText", &statusText_val) ||
        !JS_GetProperty(cx, init, "headers", &headers_val)) {
      return false;
    }

    if (!status_val.isUndefined() && !JS::ToUint16(cx, status_val, &status)) {
      return false;
    }

    if (!statusText_val.isUndefined() && !(statusText = JS::ToString(cx, statusText_val))) {
      return false;
    }

  } else if (!init_val.isNullOrUndefined()) {
    JS_ReportErrorLatin1(cx, "Response constructor: |init| parameter can't be converted to "
                             "a dictionary");
    return false;
  }

  // 1.  If `init`["status"] is not in the range 200 to 599, inclusive, then
  // `throw` a ``RangeError``.
  if (status < 200 || status > 599) {
    JS_ReportErrorLatin1(cx, "Response constructor: invalid status %u", status);
    return false;
  }

  // 2.  If `init`["statusText"] does not match the `reason-phrase` token
  // production, then `throw` a ``TypeError``. Skipped: the statusText can only
  // be consumed by the content creating it, so we're lenient about its format.

  // 3.  Set `this`’s `response` to a new `response`.
  // TODO(performance): consider not creating a host-side representation for responses
  // eagerly. Some applications create Response objects purely for internal use,
  // e.g. to represent cache entries. While that's perhaps not ideal to begin
  // with, it exists, so we should handle it in a good way, and not be
  // superfluously slow.
  // https://github.com/fastly/js-compute-runtime/issues/219
  // TODO(performance): enable creating Response objects during the init phase, and only
  // creating the host-side representation when processing requests.
  // https://github.com/fastly/js-compute-runtime/issues/220
  fastly_response_handle_t response_handle = INVALID_HANDLE;
  fastly_error_t err;
  if (!xqd_fastly_http_resp_new(&response_handle, &err)) {
    HANDLE_ERROR(cx, err);
    return false;
  }

  fastly_body_handle_t body_handle = INVALID_HANDLE;
  if (!xqd_fastly_http_body_new(&body_handle, &err)) {
    HANDLE_ERROR(cx, err);
    return false;
  }

  RootedObject responseInstance(cx, JS_NewObjectForConstructor(cx, &class_, args));
  RootedObject response(cx, create(cx, responseInstance, response_handle, body_handle, false));
  if (!response) {
    return false;
  }

  RequestOrResponse::set_url(response, JS_GetEmptyStringValue(cx));

  // 4.  Set `this`’s `headers` to a `new` ``Headers`` object with `this`’s
  // `relevant Realm`,
  //     whose `header list` is `this`’s `response`’s `header list` and `guard`
  //     is "`response`".
  // (implicit)

  // 5.  Set `this`’s `response`’s `status` to `init`["status"].
  if (!xqd_fastly_http_resp_status_set(response_handle, status, &err)) {
    HANDLE_ERROR(cx, err);
    return false;
  }
  // To ensure that we really have the same status value as the host,
  // we always read it back here.
  if (!xqd_fastly_http_resp_status_get(response_handle, &status, &err)) {
    HANDLE_ERROR(cx, err);
    return false;
  }

  JS::SetReservedSlot(response, Slots::Status, JS::Int32Value(status));

  // 6.  Set `this`’s `response`’s `status message` to `init`["statusText"].
  JS::SetReservedSlot(response, Slots::StatusMessage, JS::StringValue(statusText));

  // 7.  If `init`["headers"] `exists`, then `fill` `this`’s `headers` with
  // `init`["headers"].
  RootedObject headers(cx);
  RootedObject headersInstance(
      cx, JS_NewObjectWithGivenProto(cx, &Headers::class_, Headers::proto_obj));
  if (!headersInstance)
    return false;

  headers =
      Headers::create(cx, headersInstance, Headers::Mode::ProxyToResponse, response, headers_val);
  if (!headers) {
    return false;
  }
  JS::SetReservedSlot(response, Slots::Headers, JS::ObjectValue(*headers));
  // 8.  If `body` is non-null, then:
  if ((!body_val.isNullOrUndefined())) {
    //     1.  If `init`["status"] is a `null body status`, then `throw` a
    //     ``TypeError``.
    if (status == 204 || status == 205 || status == 304) {
      JS_ReportErrorLatin1(cx, "Response constructor: Response body is given "
                               "with a null body status.");
      return false;
    }

    //     2.  Let `Content-Type` be null.
    //     3.  Set `this`’s `response`’s `body` and `Content-Type` to the result
    //     of `extracting`
    //         `body`.
    //     4.  If `Content-Type` is non-null and `this`’s `response`’s `header
    //     list` `does not
    //         contain` ``Content-Type``, then `append` (``Content-Type``,
    //         `Content-Type`) to `this`’s `response`’s `header list`.
    // Note: these steps are all inlined into RequestOrResponse::extract_body.
    if (!RequestOrResponse::extract_body(cx, response, body_val)) {
      return false;
    }
  }

  args.rval().setObject(*response);
  return true;
}

bool init_class(JSContext *cx, HandleObject global) {
  if (!init_class_impl(cx, global)) {
    return false;
  }

  // Initialize a pinned (i.e., never-moved, living forever) atom for the
  // response type values.
  return (type_default_atom = JS_AtomizeAndPinString(cx, "default")) &&
         (type_error_atom = JS_AtomizeAndPinString(cx, "error"));
}

JSObject *create(JSContext *cx, HandleObject response, fastly_response_handle_t response_handle,
                 fastly_body_handle_t body_handle, bool is_upstream) {
  JS::SetReservedSlot(response, Slots::Response, JS::Int32Value(response_handle));
  JS::SetReservedSlot(response, Slots::Headers, JS::NullValue());
  JS::SetReservedSlot(response, Slots::Body, JS::Int32Value(body_handle));
  JS::SetReservedSlot(response, Slots::BodyStream, JS::NullValue());
  JS::SetReservedSlot(response, Slots::HasBody, JS::FalseValue());
  JS::SetReservedSlot(response, Slots::BodyUsed, JS::FalseValue());
  JS::SetReservedSlot(response, Slots::IsUpstream, JS::BooleanValue(is_upstream));

  if (is_upstream) {
    uint16_t status = 0;
    fastly_error_t err;
    if (!xqd_fastly_http_resp_status_get(response_handle, &status, &err)) {
      HANDLE_ERROR(cx, err);
      return nullptr;
    }

    JS::SetReservedSlot(response, Slots::Status, JS::Int32Value(status));
    set_status_message_from_code(cx, response, status);

    if (!(status == 204 || status == 205 || status == 304)) {
      JS::SetReservedSlot(response, Slots::HasBody, JS::TrueValue());
    }
  }

  return response;
}
} // namespace Response

namespace TextEncoder {
namespace Slots {
enum { Count };
};

const unsigned ctor_length = 0;

bool check_receiver(JSContext *cx, HandleValue receiver, const char *method_name);

bool encode(JSContext *cx, unsigned argc, Value *vp) {
  METHOD_HEADER(1)

  // Default to empty string if no input is given.
  if (args.get(0).isUndefined()) {
    RootedObject byte_array(cx, JS_NewUint8Array(cx, 0));
    if (!byte_array)
      return false;

    args.rval().setObject(*byte_array);
    return true;
  }

  size_t chars_len;
  UniqueChars chars = encode(cx, args[0], &chars_len);

  auto *rawChars = chars.release();
  RootedObject buffer(cx, JS::NewArrayBufferWithContents(cx, chars_len, rawChars));
  if (!buffer) {
    JS_free(cx, rawChars);
    return false;
  }

  RootedObject byte_array(cx, JS_NewUint8ArrayWithBuffer(cx, buffer, 0, chars_len));
  if (!byte_array)
    return false;

  args.rval().setObject(*byte_array);
  return true;
}

bool encoding_get(JSContext *cx, unsigned argc, Value *vp) {
  METHOD_HEADER(0)

  RootedString str(cx, JS_NewStringCopyN(cx, "utf-8", 5));
  if (!str)
    return false;

  args.rval().setString(str);
  return true;
}

const JSFunctionSpec methods[] = {JS_FN("encode", encode, 1, JSPROP_ENUMERATE), JS_FS_END};

const JSPropertySpec properties[] = {JS_PSG("encoding", encoding_get, JSPROP_ENUMERATE),
                                     JS_STRING_SYM_PS(toStringTag, "TextEncoder", JSPROP_READONLY),
                                     JS_PS_END};
bool constructor(JSContext *cx, unsigned argc, Value *vp);
CLASS_BOILERPLATE(TextEncoder)

bool constructor(JSContext *cx, unsigned argc, Value *vp) {
  CTOR_HEADER("TextEncoder", 0);

  RootedObject self(cx, JS_NewObjectForConstructor(cx, &class_, args));
  if (!self)
    return false;

  args.rval().setObject(*self);
  return true;
}
} // namespace TextEncoder

namespace TextDecoder {
namespace Slots {
enum { Count };
};

const unsigned ctor_length = 0;

bool check_receiver(JSContext *cx, HandleValue receiver, const char *method_name);

bool decode(JSContext *cx, unsigned argc, Value *vp) {
  METHOD_HEADER(1)

  // Default to empty string if no input is given.
  if (args[0].isUndefined()) {
    args.rval().set(JS_GetEmptyStringValue(cx));
    return true;
  }

  size_t length;
  uint8_t *data = value_to_buffer(cx, args[0], "TextDecoder#decode: input", &length);
  if (!data) {
    return false;
  }

  RootedString str(cx, JS_NewStringCopyUTF8N(cx, JS::UTF8Chars((char *)data, length)));
  if (!str)
    return false;

  args.rval().setString(str);
  return true;
}

bool encoding_get(JSContext *cx, unsigned argc, Value *vp) {
  METHOD_HEADER(0)

  RootedString str(cx, JS_NewStringCopyN(cx, "utf-8", 5));
  if (!str)
    return false;

  args.rval().setString(str);
  return true;
}

const JSFunctionSpec methods[] = {JS_FN("decode", decode, 1, JSPROP_ENUMERATE), JS_FS_END};

const JSPropertySpec properties[] = {JS_PSG("encoding", encoding_get, JSPROP_ENUMERATE),
                                     JS_STRING_SYM_PS(toStringTag, "TextDecoder", JSPROP_READONLY),
                                     JS_PS_END};
bool constructor(JSContext *cx, unsigned argc, Value *vp);
CLASS_BOILERPLATE(TextDecoder)

bool constructor(JSContext *cx, unsigned argc, Value *vp) {
  CTOR_HEADER("TextDecoder", 0);

  RootedObject self(cx, JS_NewObjectForConstructor(cx, &class_, args));

  args.rval().setObject(*self);
  return true;
}
} // namespace TextDecoder

namespace Headers {
namespace detail {
JSObject *backing_map(JSObject *self) {
  MOZ_ASSERT(is_instance(self));
  return &JS::GetReservedSlot(self, Slots::BackingMap).toObject();
}

Mode mode(JSObject *self) {
  MOZ_ASSERT(is_instance(self));
  return static_cast<Mode>(JS::GetReservedSlot(self, Slots::Mode).toInt32());
}

bool lazy_values(JSObject *self) {
  MOZ_ASSERT(is_instance(self));
  return JS::GetReservedSlot(self, Slots::HasLazyValues).toBoolean();
}

fastly_request_handle_t handle(JSObject *self) {
  MOZ_ASSERT(is_instance(self));
  return static_cast<uint32_t>(JS::GetReservedSlot(self, Slots::Handle).toInt32());
}

/**
 * Validates and normalizes the given header name, by
 * - checking for invalid characters
 * - converting to lower-case
 *
 * See
 * https://searchfox.org/mozilla-central/rev/9f76a47f4aa935b49754c5608a1c8e72ee358c46/netwerk/protocol/http/nsHttp.cpp#172-215
 * For details on validation.
 *
 * Mutates `name_val` in place, and returns the name as UniqueChars.
 * This is done because most uses of header names require handling of both the
 * JSString and the char* version, so they'd otherwise have to recreate one of
 * the two.
 */
UniqueChars normalize_header_name(JSContext *cx, MutableHandleValue name_val, size_t *name_len,
                                  const char *fun_name) {
  RootedString name_str(cx, JS::ToString(cx, name_val));
  if (!name_str)
    return nullptr;

  size_t len;
  UniqueChars name = encode(cx, name_str, &len);
  if (!name)
    return nullptr;

  if (len == 0) {
    JS_ReportErrorASCII(cx, "%s: Header name can't be empty", fun_name);
    return nullptr;
  }

  bool changed = false;

  char *name_chars = name.get();
  for (size_t i = 0; i < len; i++) {
    unsigned char ch = name_chars[i];
    if (ch > 127 || !VALID_NAME_CHARS[ch]) {
      JS_ReportErrorUTF8(cx, "%s: Invalid header name '%s'", fun_name, name_chars);
      return nullptr;
    }

    if (ch >= 'A' && ch <= 'Z') {
      name_chars[i] = ch - 'A' + 'a';
      changed = true;
    }
  }

  if (changed) {
    name_str = JS_NewStringCopyN(cx, name_chars, len);
    if (!name_str)
      return nullptr;
  }

  name_val.setString(name_str);
  *name_len = len;
  return name;
}

UniqueChars normalize_header_value(JSContext *cx, MutableHandleValue value_val, size_t *value_len,
                                   const char *fun_name) {
  RootedString value_str(cx, JS::ToString(cx, value_val));
  if (!value_str)
    return nullptr;

  size_t len;
  UniqueChars value = encode(cx, value_str, &len);
  if (!value)
    return nullptr;

  char *value_chars = value.get();
  size_t start = 0;
  size_t end = len;

  // We follow Gecko's interpretation of what's a valid header value. After
  // stripping leading and trailing whitespace, all interior line breaks and
  // `\0` are considered invalid. See
  // https://searchfox.org/mozilla-central/rev/9f76a47f4aa935b49754c5608a1c8e72ee358c46/netwerk/protocol/http/nsHttp.cpp#247-260
  // for details.
  while (start < end) {
    unsigned char ch = value_chars[start];
    if (ch == '\t' || ch == ' ' || ch == '\r' || ch == '\n') {
      start++;
    } else {
      break;
    }
  }

  while (end > start) {
    unsigned char ch = value_chars[end - 1];
    if (ch == '\t' || ch == ' ' || ch == '\r' || ch == '\n') {
      end--;
    } else {
      break;
    }
  }

  for (size_t i = start; i < end; i++) {
    unsigned char ch = value_chars[i];
    if (ch == '\r' || ch == '\n' || ch == '\0') {
      JS_ReportErrorUTF8(cx, "%s: Invalid header value '%s'", fun_name, value_chars);
      return nullptr;
    }
  }

  if (start != 0 || end != len) {
    value_str = JS_NewStringCopyUTF8N(cx, JS::UTF8Chars(value_chars + start, end - start));
    if (!value_str)
      return nullptr;
  }

  value_val.setString(value_str);
  *value_len = len;
  return value;
}

// Append an already normalized value for an already normalized header name
// to the JS side map, but not the host.
//
// Returns the resulting combined value in `normalized_value`.
bool append_header_value_to_map(JSContext *cx, HandleObject self, HandleValue normalized_name,
                                MutableHandleValue normalized_value) {
  RootedValue existing(cx);
  RootedObject map(cx, backing_map(self));
  if (!JS::MapGet(cx, map, normalized_name, &existing))
    return false;

  // Existing value must only be null if we're in the process if applying
  // header values from a handle.
  if (!existing.isNullOrUndefined()) {
    if (!comma.get()) {
      comma.init(cx, JS_NewStringCopyN(cx, ", ", 2));
      if (!comma)
        return false;
    }

    RootedString str(cx, existing.toString());
    str = JS_ConcatStrings(cx, str, comma);
    if (!str)
      return false;

    RootedString val_str(cx, normalized_value.toString());
    str = JS_ConcatStrings(cx, str, val_str);
    if (!str)
      return false;

    normalized_value.setString(str);
  }

  return JS::MapSet(cx, map, normalized_name, normalized_value);
}

bool get_header_names_from_handle(JSContext *cx, uint32_t handle, Mode mode,
                                  HandleObject backing_map) {
  RootedString name(cx);
  RootedValue name_val(cx);
  char *buf = static_cast<char *>(JS_malloc(cx, HOSTCALL_BUFFER_LEN));

  bool ok;
  fastly_list_string_t ret;
  fastly_error_t err;
  if (mode == Mode::ProxyToRequest) {
    ok = xqd_fastly_http_req_header_names_get(handle, &ret, &err);
  } else {
    ok = xqd_fastly_http_resp_header_names_get(handle, &ret, &err);
  }

  if (!ok) {
    HANDLE_ERROR(cx, err);
    JS_free(cx, buf);
    return false;
  }

  for (size_t i = 0; i < ret.len; i++) {
    name = JS_NewStringCopyN(cx, ret.ptr[i].ptr, ret.ptr[i].len);
    JS_free(cx, ret.ptr[i].ptr);
    if (!name)
      return false;

    name_val.setString(name);
    JS::MapSet(cx, backing_map, name_val, JS::NullHandleValue);
  }

  JS_free(cx, buf);
  JS_free(cx, ret.ptr);
  return true;
}

static bool retrieve_value_for_header_from_handle(JSContext *cx, HandleObject self,
                                                  HandleValue name, MutableHandleValue value) {
  Mode mode = detail::mode(self);
  MOZ_ASSERT(mode != Mode::Standalone);
  uint32_t handle = detail::handle(self);

  xqd_world_string_t str;
  RootedString name_str(cx, name.toString());
  UniqueChars name_chars = encode(cx, name_str, &str.len);
  str.ptr = name_chars.get();

  fastly_option_list_string_t ret;

  bool ok;
  fastly_error_t err;
  if (mode == Headers::Mode::ProxyToRequest) {
    ok = xqd_fastly_http_req_header_values_get(handle, &str, &ret, &err);
  } else {
    ok = xqd_fastly_http_resp_header_values_get(handle, &str, &ret, &err);
  }

  if (!ok) {
    HANDLE_ERROR(cx, err);
    return false;
  }

  if (!ret.is_some)
    return true;

  RootedString val_str(cx);
  for (size_t i = 0; i < ret.val.len; i++) {
    val_str = JS_NewStringCopyUTF8N(cx, JS::UTF8Chars(ret.val.ptr[i].ptr, ret.val.ptr[i].len));
    JS_free(cx, ret.val.ptr[i].ptr);
    if (!val_str)
      return false;
    value.setString(val_str);
    if (!append_header_value_to_map(cx, self, name, value))
      return false;
  }

  JS_free(cx, ret.val.ptr);
  return true;
}

/**
 * Ensures that a value for the given header is available to client code.
 *
 * The calling code must ensure that a header with the given name exists, but
 * might not yet have been retrieved from the host, i.e., it might be a "lazy"
 * value.
 *
 * The value is returned via the `values` outparam, but *only* if the Headers
 * object has lazy values at all. This is to avoid the map lookup in those cases
 * where none is necessary in this function, and the consumer wouldn't use the
 * value anyway.
 */
static bool ensure_value_for_header(JSContext *cx, HandleObject self, HandleValue normalized_name,
                                    MutableHandleValue values) {
  if (!detail::lazy_values(self))
    return true;

  RootedObject map(cx, detail::backing_map(self));
  if (!JS::MapGet(cx, map, normalized_name, values))
    return false;

  // Value isn't lazy, just return it.
  if (!values.isNull())
    return true;

  return detail::retrieve_value_for_header_from_handle(cx, self, normalized_name, values);
}

bool get_header_value_for_name(JSContext *cx, HandleObject self, HandleValue name,
                               MutableHandleValue rval, const char *fun_name) {
  NORMALIZE_NAME(name, fun_name)

  if (!ensure_value_for_header(cx, self, normalized_name, rval))
    return false;

  if (rval.isString())
    return true;

  RootedObject map(cx, detail::backing_map(self));
  if (!JS::MapGet(cx, map, normalized_name, rval)) {
    return false;
  }

  // Return `null` for non-existent headers.
  if (rval.isUndefined()) {
    rval.setNull();
  }

  return true;
}

std::string_view special_chars = "=,;";

std::vector<std::string_view> splitCookiesString(std::string_view cookiesString) {
  std::vector<std::string_view> cookiesStrings;
  std::size_t currentPosition = 0; // Current position in the string
  std::size_t start;               // Start position of the current cookie
  std::size_t lastComma;           // Position of the last comma found
  std::size_t nextStart;           // Position of the start of the next cookie

  // Iterate over the string and split it into cookies.
  while (currentPosition < cookiesString.length()) {
    start = currentPosition;

    // Iterate until we find a comma that might be used as a separator.
    while ((currentPosition = cookiesString.find_first_of(",", currentPosition)) !=
           std::string_view::npos) {
      // ',' is a cookie separator only if we later have '=', before having ';' or ','
      lastComma = currentPosition;
      nextStart = ++currentPosition;

      // Check if the next sequence of characters is a non-special character followed by an equals
      // sign.
      currentPosition = cookiesString.find_first_of(special_chars, currentPosition);

      // If the current character is an equals sign, we have found a cookie separator.
      if (currentPosition != std::string_view::npos && cookiesString.at(currentPosition) == '=') {
        // currentPosition is inside the next cookie, so back up and return it.
        currentPosition = nextStart;
        cookiesStrings.push_back(cookiesString.substr(start, lastComma - start));
        start = currentPosition;
      } else {
        // The cookie contains ';' or ',' as part of the value
        // so we need to keep accumulating characters
        currentPosition = lastComma + 1;
      }
    }

    // If we reach the end of the string without finding a separator, add the last cookie to the
    // vector.
    if (currentPosition >= cookiesString.length()) {
      cookiesStrings.push_back(cookiesString.substr(start, cookiesString.length() - start));
    }
  }
  return cookiesStrings;
}

static bool ensure_all_header_values_from_handle(JSContext *cx, HandleObject self,
                                                 HandleObject backing_map) {
  if (!lazy_values(self))
    return true;

  RootedValue iterable(cx);
  if (!JS::MapKeys(cx, backing_map, &iterable))
    return false;

  JS::ForOfIterator it(cx);
  if (!it.init(iterable))
    return false;

  RootedValue name(cx);
  RootedValue v(cx);
  while (true) {
    bool done;
    if (!it.next(&name, &done))
      return false;

    if (done)
      break;

    if (!ensure_value_for_header(cx, self, name, &v))
      return false;
  }

  JS::SetReservedSlot(self, Slots::HasLazyValues, JS::BooleanValue(false));

  return true;
}

// Appends a non-normalized value for a non-normalized header name to both
// the JS side Map and, in non-standalone mode, the host.
//
// Verifies and normalizes the name and value.
bool append_header_value(JSContext *cx, HandleObject self, HandleValue name, HandleValue value,
                         const char *fun_name) {
  NORMALIZE_NAME(name, fun_name)
  NORMALIZE_VALUE(value, fun_name)

  // Ensure that any host-side values have been applied JS-side.
  RootedValue v(cx);
  if (!ensure_value_for_header(cx, self, normalized_name, &v)) {
    return false;
  }

  Mode mode = detail::mode(self);
  if (mode != Mode::Standalone) {
    AppendHeaderOperation *op;
    if (mode == Mode::ProxyToRequest) {
      op = (AppendHeaderOperation *)xqd_fastly_http_req_header_append;
    } else {
      op = (AppendHeaderOperation *)xqd_fastly_http_resp_header_append;
    }
    std::string_view name(name_chars.get(), name_len);
    if (name == "set-cookie") {
      std::string_view value(value_chars.get(), value_len);
      auto values = splitCookiesString(value);
      for (auto value : values) {
        xqd_world_string_t name = {name_chars.get(), name_len};
        xqd_world_string_t val = {const_cast<char *>(value.data()), value.length()};
        fastly_error_t err;
        if (!op(handle(self), &name, &val, &err)) {
          HANDLE_ERROR(cx, err);
          return false;
        }
      }
    } else {
      xqd_world_string_t name = {name_chars.get(), name_len};
      xqd_world_string_t val = {value_chars.get(), value_len};
      fastly_error_t err;
      if (!op(handle(self), &name, &val, &err)) {
        HANDLE_ERROR(cx, err);
        return false;
      }
    }
  }

  return append_header_value_to_map(cx, self, normalized_name, &normalized_value);
}
} // namespace detail

bool delazify(JSContext *cx, HandleObject headers) {
  RootedObject backing_map(cx, detail::backing_map(headers));
  return detail::ensure_all_header_values_from_handle(cx, headers, backing_map);
}

JSObject *create(JSContext *cx, HandleObject self, Mode mode, HandleObject owner,
                 HandleObject init_headers) {
  RootedObject headers(cx, create(cx, self, mode, owner));
  if (!headers) {
    return nullptr;
  }

  if (!init_headers) {
    return headers;
  }

  if (!delazify(cx, init_headers)) {
    return nullptr;
  }

  RootedObject headers_map(cx, detail::backing_map(headers));
  RootedObject init_map(cx, detail::backing_map(init_headers));

  RootedValue iterable(cx);
  if (!JS::MapEntries(cx, init_map, &iterable))
    return nullptr;

  JS::ForOfIterator it(cx);
  if (!it.init(iterable))
    return nullptr;

  RootedObject entry(cx);
  RootedValue entry_val(cx);
  RootedValue name_val(cx);
  RootedValue value_val(cx);
  while (true) {
    bool done;
    if (!it.next(&entry_val, &done))
      return nullptr;

    if (done)
      break;

    entry = &entry_val.toObject();
    JS_GetElement(cx, entry, 0, &name_val);
    JS_GetElement(cx, entry, 1, &value_val);

    if (!detail::append_header_value(cx, headers, name_val, value_val, "Headers constructor")) {
      return nullptr;
    }
  }

  return headers;
}

JSObject *create(JSContext *cx, HandleObject self, Mode mode, HandleObject owner,
                 HandleValue initv) {
  RootedObject headers(cx, create(cx, self, mode, owner));
  if (!headers)
    return nullptr;

  bool consumed = false;
  if (!maybe_consume_sequence_or_record<detail::append_header_value>(cx, initv, headers, &consumed,
                                                                     "Headers")) {
    return nullptr;
  }

  if (!consumed) {
    report_sequence_or_record_arg_error(cx, "Headers", "");
    return nullptr;
  }

  return headers;
}

bool check_receiver(JSContext *cx, HandleValue receiver, const char *method_name);

bool get(JSContext *cx, unsigned argc, Value *vp) {
  METHOD_HEADER(1)

  NORMALIZE_NAME(args[0], "Headers.get")

  return detail::get_header_value_for_name(cx, self, normalized_name, args.rval(), "Headers.get");
}

typedef bool HeaderInsertOperation(fastly_request_handle_t handle, xqd_world_string_t *name,
                                   xqd_world_string_t *values, fastly_error_t *err);

bool set(JSContext *cx, unsigned argc, Value *vp) {
  METHOD_HEADER(2)

  NORMALIZE_NAME(args[0], "Headers.set")
  NORMALIZE_VALUE(args[1], "Headers.set")

  Mode mode = detail::mode(self);
  if (mode != Mode::Standalone) {
    HeaderInsertOperation *op;
    if (mode == Mode::ProxyToRequest)
      op = (HeaderInsertOperation *)xqd_fastly_http_req_header_insert;
    else
      op = (HeaderInsertOperation *)xqd_fastly_http_resp_header_insert;
    xqd_world_string_t name = {name_chars.get(), name_len};
    xqd_world_string_t val = {value_chars.get(), value_len};
    fastly_error_t err;
    if (!op(detail::handle(self), &name, &val, &err)) {
      HANDLE_ERROR(cx, err);
      return false;
    }
  }

  RootedObject map(cx, detail::backing_map(self));
  if (!JS::MapSet(cx, map, normalized_name, normalized_value))
    return false;

  args.rval().setUndefined();
  return true;
}

bool has(JSContext *cx, unsigned argc, Value *vp) {
  METHOD_HEADER(1)

  NORMALIZE_NAME(args[0], "Headers.has")
  bool has;
  RootedObject map(cx, detail::backing_map(self));
  if (!JS::MapHas(cx, map, normalized_name, &has))
    return false;
  args.rval().setBoolean(has);
  return true;
}

bool append(JSContext *cx, unsigned argc, Value *vp) {
  METHOD_HEADER(2)

  if (!detail::append_header_value(cx, self, args[0], args[1], "Headers.append"))
    return false;

  args.rval().setUndefined();
  return true;
}

/**
 * Adds the given header name/value to `self`'s list of headers iff `self`
 * doesn't already contain a header with that name.
 *
 * Assumes that both the name and value are valid and normalized.
 * TODO(performance): fully skip normalization.
 * https://github.com/fastly/js-compute-runtime/issues/221
 */
bool maybe_add(JSContext *cx, HandleObject self, const char *name, const char *value) {
  MOZ_ASSERT(is_instance(self));
  RootedString name_str(cx, JS_NewStringCopyN(cx, name, strlen(name)));
  if (!name_str) {
    return false;
  }
  RootedValue name_val(cx, JS::StringValue(name_str));

  RootedObject map(cx, detail::backing_map(self));
  bool has;
  if (!JS::MapHas(cx, map, name_val, &has)) {
    return false;
  }
  if (has) {
    return true;
  }

  RootedString value_str(cx, JS_NewStringCopyN(cx, value, strlen(value)));
  if (!value_str) {
    return false;
  }
  RootedValue value_val(cx, JS::StringValue(value_str));

  return detail::append_header_value(cx, self, name_val, value_val, "internal_maybe_add");
}

bool delete_(JSContext *cx, unsigned argc, Value *vp) {
  METHOD_HEADER_WITH_NAME(1, "delete")

  NORMALIZE_NAME(args[0], "Headers.delete")

  bool has;
  RootedObject map(cx, detail::backing_map(self));
  if (!JS::MapDelete(cx, map, normalized_name, &has))
    return false;

  // If no header with the given name exists, `delete` is a no-op.
  if (!has) {
    args.rval().setUndefined();
    return true;
  }

  Mode mode = detail::mode(self);
  if (mode != Mode::Standalone) {
    HeaderRemoveOperation *op;
    if (mode == Mode::ProxyToRequest)
      op = (HeaderRemoveOperation *)xqd_fastly_http_req_header_remove;
    else
      op = (HeaderRemoveOperation *)xqd_fastly_http_resp_header_remove;
    xqd_world_string_t name = {name_chars.get(), name_len};
    fastly_error_t err;
    if (!op(detail::handle(self), &name, &err)) {
      HANDLE_ERROR(cx, err);
      return false;
    }
  }

  args.rval().setUndefined();
  return true;
}

bool forEach(JSContext *cx, unsigned argc, Value *vp) {
  HEADERS_ITERATION_METHOD(1)

  if (!args[0].isObject() || !JS::IsCallable(&args[0].toObject())) {
    JS_ReportErrorASCII(cx, "Failed to execute 'forEach' on 'Headers': "
                            "parameter 1 is not of type 'Function'");
    return false;
  }

  JS::RootedValueArray<3> newArgs(cx);
  newArgs[2].setObject(*self);

  RootedValue rval(cx);

  RootedValue iterable(cx);
  if (!JS::MapEntries(cx, backing_map, &iterable))
    return false;

  JS::ForOfIterator it(cx);
  if (!it.init(iterable))
    return false;

  RootedValue entry_val(cx);
  RootedObject entry(cx);
  while (true) {
    bool done;
    if (!it.next(&entry_val, &done))
      return false;

    if (done)
      break;

    entry = &entry_val.toObject();
    JS_GetElement(cx, entry, 1, newArgs[0]);
    JS_GetElement(cx, entry, 0, newArgs[1]);

    if (!JS::Call(cx, args.thisv(), args[0], newArgs, &rval))
      return false;
  }

  args.rval().setUndefined();
  return true;
}

bool entries(JSContext *cx, unsigned argc, Value *vp) {
  HEADERS_ITERATION_METHOD(0)
  return JS::MapEntries(cx, backing_map, args.rval());
}

bool keys(JSContext *cx, unsigned argc, Value *vp) {
  HEADERS_ITERATION_METHOD(0)
  return JS::MapKeys(cx, backing_map, args.rval());
}

bool values(JSContext *cx, unsigned argc, Value *vp) {
  HEADERS_ITERATION_METHOD(0)
  return JS::MapValues(cx, backing_map, args.rval());
}

bool constructor(JSContext *cx, unsigned argc, Value *vp) {
  CTOR_HEADER("Headers", 0);
  RootedObject headersInstance(cx, JS_NewObjectForConstructor(cx, &class_, args));
  RootedObject headers(cx, create(cx, headersInstance, Mode::Standalone, nullptr, args.get(0)));
  if (!headers)
    return false;

  args.rval().setObject(*headers);
  return true;
}

bool init_class(JSContext *cx, HandleObject global) {
  bool ok = init_class_impl(cx, global);
  if (!ok)
    return false;

  RootedValue entries(cx);
  if (!JS_GetProperty(cx, proto_obj, "entries", &entries))
    return false;

  JS::SymbolCode code = JS::SymbolCode::iterator;
  JS::RootedId iteratorId(cx, JS::GetWellKnownSymbolKey(cx, code));
  return JS_DefinePropertyById(cx, proto_obj, iteratorId, entries, 0);
}

JSObject *create(JSContext *cx, HandleObject self, Mode mode, HandleObject owner) {
  JS_SetReservedSlot(self, Slots::Mode, JS::Int32Value(static_cast<int32_t>(mode)));
  uint32_t handle = UINT32_MAX - 1;
  if (mode != Mode::Standalone)
    handle = RequestOrResponse::handle(owner);
  JS_SetReservedSlot(self, Slots::Handle, JS::Int32Value(static_cast<int32_t>(handle)));

  RootedObject backing_map(cx, JS::NewMapObject(cx));
  if (!backing_map)
    return nullptr;
  JS::SetReservedSlot(self, Slots::BackingMap, JS::ObjectValue(*backing_map));

  bool lazy = false;
  if ((mode == Mode::ProxyToRequest && Request::is_downstream(owner)) ||
      (mode == Mode::ProxyToResponse && Response::is_upstream(owner))) {
    lazy = true;
    if (!detail::get_header_names_from_handle(cx, handle, mode, backing_map))
      return nullptr;
  }

  JS_SetReservedSlot(self, Slots::HasLazyValues, JS::BooleanValue(lazy));

  return self;
}
} // namespace Headers

namespace ClientInfo {
namespace Slots {
enum { Address, GeoInfo, Count };
};

JSString *address(JSObject *obj) {
  JS::Value val = JS::GetReservedSlot(obj, Slots::Address);
  return val.isString() ? val.toString() : nullptr;
}

JSString *geo_info(JSObject *obj) {
  JS::Value val = JS::GetReservedSlot(obj, Slots::GeoInfo);
  return val.isString() ? val.toString() : nullptr;
}

static JSString *retrieve_address(JSContext *cx, HandleObject self) {
  RootedString address(cx);

  fastly_list_u8_t octets;
  fastly_error_t err;
  if (!xqd_fastly_http_req_downstream_client_ip_addr(&octets, &err)) {
    HANDLE_ERROR(cx, err);
    return nullptr;
  }

  switch (octets.len) {
  case 0: {
    // No address to be had, leave `address` as a nullptr.
    JS_free(cx, octets.ptr);
    break;
  }
  case 4: {
    char address_chars[INET_ADDRSTRLEN];
    // TODO: do we need to do error handling here, or can we depend on the
    // host giving us a valid address?
    inet_ntop(AF_INET, octets.ptr, address_chars, INET_ADDRSTRLEN);
    address = JS_NewStringCopyZ(cx, address_chars);
    JS_free(cx, octets.ptr);
    if (!address)
      return nullptr;

    break;
  }
  case 16: {
    char address_chars[INET6_ADDRSTRLEN];
    // TODO: do we need to do error handling here, or can we depend on the
    // host giving us a valid address?
    inet_ntop(AF_INET6, octets.ptr, address_chars, INET6_ADDRSTRLEN);
    address = JS_NewStringCopyZ(cx, address_chars);
    JS_free(cx, octets.ptr);
    if (!address)
      return nullptr;

    break;
  }
  }

  JS::SetReservedSlot(self, Slots::Address, JS::StringValue(address));
  return address;
}

static JSString *retrieve_geo_info(JSContext *cx, HandleObject self) {
  RootedString address_str(cx, address(self));
  if (!address_str) {
    address_str = retrieve_address(cx, self);
    if (!address_str)
      return nullptr;
  }

  RootedString geo(cx, get_geo_info(cx, address_str));
  if (!geo)
    return nullptr;

  JS::SetReservedSlot(self, Slots::GeoInfo, JS::StringValue(geo));
  return geo;
}

const unsigned ctor_length = 0;

bool check_receiver(JSContext *cx, HandleValue receiver, const char *method_name);

bool address_get(JSContext *cx, unsigned argc, Value *vp) {
  METHOD_HEADER(0)

  RootedString address_str(cx, address(self));
  if (!address_str) {
    address_str = retrieve_address(cx, self);
    if (!address_str)
      return false;
  }

  args.rval().setString(address_str);
  return true;
}

bool geo_get(JSContext *cx, unsigned argc, Value *vp) {
  METHOD_HEADER(0)

  RootedString geo_info_str(cx, geo_info(self));
  if (!geo_info_str) {
    geo_info_str = retrieve_geo_info(cx, self);
    if (!geo_info_str)
      return false;
  }

  return JS_ParseJSON(cx, geo_info_str, args.rval());
}

const JSFunctionSpec methods[] = {JS_FS_END};

const JSPropertySpec properties[] = {JS_PSG("address", address_get, JSPROP_ENUMERATE),
                                     JS_PSG("geo", geo_get, JSPROP_ENUMERATE), JS_PS_END};

CLASS_BOILERPLATE_NO_CTOR(ClientInfo)

JSObject *create(JSContext *cx) { return JS_NewObjectWithGivenProto(cx, &class_, proto_obj); }
} // namespace ClientInfo

namespace FetchEvent {
namespace Slots {
enum {
  Dispatch,
  Request,
  State,
  PendingPromiseCount,
  DecPendingPromiseCountFunc,
  ClientInfo,
  Count
};
};

bool is_instance(JSObject *obj);

static PersistentRooted<JSObject *> INSTANCE;

namespace detail {
void inc_pending_promise_count(JSObject *self) {
  MOZ_ASSERT(is_instance(self));
  auto count = JS::GetReservedSlot(self, Slots::PendingPromiseCount).toInt32();
  count++;
  MOZ_ASSERT(count > 0);
  JS::SetReservedSlot(self, Slots::PendingPromiseCount, JS::Int32Value(count));
}

void dec_pending_promise_count(JSObject *self) {
  MOZ_ASSERT(is_instance(self));
  auto count = JS::GetReservedSlot(self, Slots::PendingPromiseCount).toInt32();
  MOZ_ASSERT(count > 0);
  count--;
  JS::SetReservedSlot(self, Slots::PendingPromiseCount, JS::Int32Value(count));
}

bool add_pending_promise(JSContext *cx, HandleObject self, HandleObject promise) {
  MOZ_ASSERT(is_instance(self));
  MOZ_ASSERT(JS::IsPromiseObject(promise));
  RootedObject handler(cx);
  handler = &JS::GetReservedSlot(self, Slots::DecPendingPromiseCountFunc).toObject();
  if (!JS::AddPromiseReactions(cx, promise, handler, handler))
    return false;

  inc_pending_promise_count(self);
  return true;
}
} // namespace detail

const unsigned ctor_length = 0;

bool check_receiver(JSContext *cx, HandleValue receiver, const char *method_name);

bool client_get(JSContext *cx, unsigned argc, Value *vp) {
  METHOD_HEADER(0)
  RootedValue clientInfo(cx, JS::GetReservedSlot(self, Slots::ClientInfo));

  if (clientInfo.isUndefined()) {
    RootedObject obj(cx, ClientInfo::create(cx));
    if (!obj)
      return false;
    clientInfo.setObject(*obj);
    JS::SetReservedSlot(self, Slots::ClientInfo, clientInfo);
  }

  args.rval().set(clientInfo);
  return true;
}

/**
 * Create a Request object for the incoming request.
 *
 * Since this happens during initialization time, the object will not be fully
 * initialized. It's filled in at runtime using `init_downstream_request`.
 */
static JSObject *prepare_downstream_request(JSContext *cx) {
  RootedObject requestInstance(
      cx, JS_NewObjectWithGivenProto(cx, &Request::class_, Request::proto_obj));
  if (!requestInstance)
    return nullptr;
  return Request::create(cx, requestInstance, INVALID_HANDLE, INVALID_HANDLE, true);
}

/**
 * Fully initialize the Request object based on the incoming request.
 */
static bool init_downstream_request(JSContext *cx, HandleObject request) {
  MOZ_ASSERT(Request::request_handle(request) == INVALID_HANDLE);

  fastly_request_t req;
  fastly_error_t err;
  if (!xqd_fastly_http_req_body_downstream_get(&req, &err)) {
    HANDLE_ERROR(cx, err);
    return false;
  }

  fastly_request_handle_t request_handle = req.f0;
  fastly_body_handle_t body_handle = req.f1;

  JS::SetReservedSlot(request, Request::Slots::Request, JS::Int32Value(request_handle));
  JS::SetReservedSlot(request, Request::Slots::Body, JS::Int32Value(body_handle));

  // Set the method.
  xqd_world_string_t method_str;
  if (!xqd_fastly_http_req_method_get(request_handle, &method_str, &err)) {
    HANDLE_ERROR(cx, err);
    return false;
  }

  bool is_get = strncmp(method_str.ptr, "GET", method_str.len) == 0;
  bool is_head = strncmp(method_str.ptr, "HEAD", method_str.len) == 0;

  if (!is_get) {
    RootedString method(cx, JS_NewStringCopyN(cx, method_str.ptr, method_str.len));
    JS_free(cx, method_str.ptr);
    if (!method) {
      return false;
    }

    JS::SetReservedSlot(request, Request::Slots::Method, JS::StringValue(method));
  }

  // Set whether we have a body depending on the method.
  // TODO: verify if that's right. I.e. whether we should treat all requests
  // that are not GET or HEAD as having a body, which might just be 0-length.
  // It's not entirely clear what else we even could do here though.
  if (!is_get && !is_head) {
    JS::SetReservedSlot(request, Request::Slots::HasBody, JS::TrueValue());
  }

  xqd_world_string_t uri_str;
  if (!xqd_fastly_http_req_uri_get(request_handle, &uri_str, &err)) {
    HANDLE_ERROR(cx, err);
    return false;
  }

  RootedString url(cx, JS_NewStringCopyN(cx, uri_str.ptr, uri_str.len));
  if (!url) {
    JS_free(cx, uri_str.ptr);
    return false;
  }
  JS::SetReservedSlot(request, Request::Slots::URL, JS::StringValue(url));

  // Set the URL for `globalThis.location` to the client request's URL.
  RootedObject url_instance(cx, JS_NewObjectWithGivenProto(cx, &URL::class_, URL::proto_obj));
  if (!url_instance) {
    JS_free(cx, uri_str.ptr);
    return false;
  }

  SpecString spec(reinterpret_cast<uint8_t *>(uri_str.ptr), uri_str.len, uri_str.len);
  builtins::WorkerLocation::url = URL::create(cx, url_instance, spec);
  JS_free(cx, uri_str.ptr);
  if (!builtins::WorkerLocation::url) {
    return false;
  }

  // Set `fastly.baseURL` to the origin of the client request's URL.
  // Note that this only happens if baseURL hasn't already been set to another
  // value explicitly.
  if (!builtins::Fastly::baseURL.get()) {
    RootedObject url_instance(cx, JS_NewObjectWithGivenProto(cx, &URL::class_, URL::proto_obj));
    if (!url_instance)
      return false;

    builtins::Fastly::baseURL =
        URL::create(cx, url_instance, URL::origin(cx, builtins::WorkerLocation::url));
    if (!builtins::Fastly::baseURL)
      return false;
  }

  return true;
}

bool request_get(JSContext *cx, unsigned argc, Value *vp) {
  METHOD_HEADER(0)

  args.rval().set(JS::GetReservedSlot(self, Slots::Request));
  return true;
}

bool start_response(JSContext *cx, HandleObject response_obj, bool streaming) {
  fastly_response_handle_t response = Response::response_handle(response_obj);
  fastly_body_handle_t body = RequestOrResponse::body_handle(response_obj);

  fastly_error_t err;
  if (!xqd_fastly_http_resp_send_downstream(response, body, streaming, &err)) {
    HANDLE_ERROR(cx, err);
    return false;
  }
  return true;
}

// Steps in this function refer to the spec at
// https://w3c.github.io/ServiceWorker/#fetch-event-respondwith
bool response_promise_then_handler(JSContext *cx, HandleObject event, HandleValue extra,
                                   CallArgs args) {
  // Step 10.1
  // Note: the `then` handler is only invoked after all Promise resolution has
  // happened. (Even if there were multiple Promises to unwrap first.) That
  // means that at this point we're guaranteed to have the final value instead
  // of a Promise wrapping it, so either the value is a Response, or we have to
  // bail.
  if (!Response::is_instance(args.get(0))) {
    JS_ReportErrorUTF8(cx, "FetchEvent#respondWith must be called with a Response "
                           "object or a Promise resolving to a Response object as "
                           "the first argument");
    RootedObject rejection(cx, PromiseRejectedWithPendingError(cx));
    if (!rejection)
      return false;
    args.rval().setObject(*rejection);
    return respondWithError(cx, event);
  }

  // Step 10.2 (very roughly: the way we handle responses and their bodies is
  // very different.)
  RootedObject response_obj(cx, &args[0].toObject());

  // Ensure that all headers are stored client-side, so we retain access to them
  // after sending the response off.
  if (Response::is_upstream(response_obj)) {
    RootedObject headers(cx);
    headers = RequestOrResponse::headers<Headers::Mode::ProxyToResponse>(cx, response_obj);
    if (!Headers::delazify(cx, headers))
      return false;
  }

  bool streaming = false;
  if (!RequestOrResponse::maybe_stream_body(cx, response_obj, &streaming)) {
    return false;
  }

  set_state(event, streaming ? State::responseStreaming : State::responseDone);
  return start_response(cx, response_obj, streaming);
}

// Steps in this function refer to the spec at
// https://w3c.github.io/ServiceWorker/#fetch-event-respondwith
bool response_promise_catch_handler(JSContext *cx, HandleObject event, HandleValue promise_val,
                                    CallArgs args) {
  RootedObject promise(cx, &promise_val.toObject());

  fprintf(stderr, "Error while running request handler: ");
  dump_promise_rejection(cx, args.get(0), promise, stderr);

  // TODO: verify that this is the right behavior.
  // Steps 9.1-2
  return respondWithError(cx, event);
}

// Steps in this function refer to the spec at
// https://w3c.github.io/ServiceWorker/#fetch-event-respondwith
bool respondWith(JSContext *cx, unsigned argc, Value *vp) {
  METHOD_HEADER(1)

  // Coercion of argument `r` to a Promise<Response>
  RootedObject response_promise(cx, JS::CallOriginalPromiseResolve(cx, args.get(0)));
  if (!response_promise)
    return false;

  // Step 2
  if (!is_dispatching(self)) {
    JS_ReportErrorUTF8(cx, "FetchEvent#respondWith must be called synchronously from "
                           "within a FetchEvent handler");
    return false;
  }

  // Step 3
  if (state(self) != State::unhandled) {
    JS_ReportErrorUTF8(cx, "FetchEvent#respondWith can't be called twice on the same event");
    return false;
  }

  // Step 4
  detail::add_pending_promise(cx, self, response_promise);

  // Steps 5-7 (very roughly)
  set_state(self, State::waitToRespond);

  // Step 9 (continued in `response_promise_catch_handler` above)
  RootedObject catch_handler(cx);
  RootedValue extra(cx, ObjectValue(*response_promise));
  catch_handler = create_internal_method<response_promise_catch_handler>(cx, self, extra);
  if (!catch_handler)
    return false;

  // Step 10 (continued in `response_promise_then_handler` above)
  RootedObject then_handler(cx);
  then_handler = create_internal_method<response_promise_then_handler>(cx, self);
  if (!then_handler)
    return false;

  if (!JS::AddPromiseReactions(cx, response_promise, then_handler, catch_handler))
    return false;

  args.rval().setUndefined();
  return true;
}

bool respondWithError(JSContext *cx, HandleObject self) {
  MOZ_RELEASE_ASSERT(state(self) == State::unhandled || state(self) == State::waitToRespond);
  set_state(self, State::responsedWithError);
  fastly_response_handle_t response = INVALID_HANDLE;
  fastly_body_handle_t body;
  fastly_error_t err;

  if (!xqd_fastly_http_resp_new(&response, &err) || !xqd_fastly_http_body_new(&body, &err) ||
      !xqd_fastly_http_resp_status_set(response, 500, &err) ||
      !xqd_fastly_http_resp_send_downstream(response, body, false, &err)) {
    HANDLE_ERROR(cx, err);
    return false;
  }
  return true;
}

// Step 5 of https://w3c.github.io/ServiceWorker/#wait-until-method
bool dec_pending_promise_count(JSContext *cx, HandleObject event, HandleValue extra,
                               CallArgs args) {
  // Step 5.1
  detail::dec_pending_promise_count(event);

  // Note: step 5.2 not relevant to our implementation.
  return true;
}

// Steps in this function refer to the spec at
// https://w3c.github.io/ServiceWorker/#wait-until-method
bool waitUntil(JSContext *cx, unsigned argc, Value *vp) {
  METHOD_HEADER(1)

  RootedObject promise(cx, JS::CallOriginalPromiseResolve(cx, args.get(0)));
  if (!promise)
    return false;

  // Step 2
  if (!is_active(self)) {
    JS_ReportErrorUTF8(cx, "FetchEvent#waitUntil called on inactive event");
    return false;
  }

  // Steps 3-4
  detail::add_pending_promise(cx, self, promise);

  // Note: step 5 implemented in dec_pending_promise_count

  args.rval().setUndefined();
  return true;
}

const JSFunctionSpec methods[] = {JS_FN("respondWith", respondWith, 1, JSPROP_ENUMERATE),
                                  JS_FN("waitUntil", waitUntil, 1, JSPROP_ENUMERATE), JS_FS_END};

const JSPropertySpec properties[] = {JS_PSG("client", client_get, JSPROP_ENUMERATE),
                                     JS_PSG("request", request_get, JSPROP_ENUMERATE), JS_PS_END};

CLASS_BOILERPLATE_NO_CTOR(FetchEvent)

JSObject *create(JSContext *cx) {
  RootedObject self(cx, JS_NewObjectWithGivenProto(cx, &class_, proto_obj));
  if (!self)
    return nullptr;

  RootedObject request(cx, prepare_downstream_request(cx));
  if (!request)
    return nullptr;

  RootedObject dec_count_handler(cx, create_internal_method<dec_pending_promise_count>(cx, self));
  if (!dec_count_handler)
    return nullptr;

  JS::SetReservedSlot(self, Slots::Request, JS::ObjectValue(*request));
  JS::SetReservedSlot(self, Slots::Dispatch, JS::FalseValue());
  JS::SetReservedSlot(self, Slots::State, JS::Int32Value((int)State::unhandled));
  JS::SetReservedSlot(self, Slots::PendingPromiseCount, JS::Int32Value(0));
  JS::SetReservedSlot(self, Slots::DecPendingPromiseCountFunc, JS::ObjectValue(*dec_count_handler));

  INSTANCE.init(cx, self);
  return self;
}

HandleObject instance() { return INSTANCE; }

bool init_request(JSContext *cx, HandleObject self) {
  RootedObject request(cx, &JS::GetReservedSlot(self, Slots::Request).toObject());
  return init_downstream_request(cx, request);
}

bool is_active(JSObject *self) {
  MOZ_ASSERT(is_instance(self));
  // Note: we also treat the FetchEvent as active if it's in `responseStreaming`
  // state because that requires us to extend the service's lifetime as well. In
  // the spec this is achieved using individual promise counts for the body read
  // operations.
  return JS::GetReservedSlot(self, Slots::Dispatch).toBoolean() ||
         state(self) == State::responseStreaming ||
         JS::GetReservedSlot(self, Slots::PendingPromiseCount).toInt32() > 0;
}

bool is_dispatching(JSObject *self) {
  MOZ_ASSERT(is_instance(self));
  return JS::GetReservedSlot(self, Slots::Dispatch).toBoolean();
}

void start_dispatching(JSObject *self) {
  MOZ_ASSERT(!is_dispatching(self));
  JS::SetReservedSlot(self, Slots::Dispatch, JS::TrueValue());
}

void stop_dispatching(JSObject *self) {
  MOZ_ASSERT(is_dispatching(self));
  JS::SetReservedSlot(self, Slots::Dispatch, JS::FalseValue());
}

State state(JSObject *self) {
  MOZ_ASSERT(is_instance(self));
  return (State)JS::GetReservedSlot(self, Slots::State).toInt32();
}

void set_state(JSObject *self, State new_state) {
  MOZ_ASSERT(is_instance(self));
  MOZ_ASSERT((uint8_t)new_state > (uint8_t)state(self));
  JS::SetReservedSlot(self, Slots::State, JS::Int32Value((int)new_state));
}

bool response_started(JSObject *self) {
  State current_state = state(self);
  return current_state != State::unhandled && current_state != State::waitToRespond;
}
} // namespace FetchEvent

using TimerArgumentsVector = std::vector<JS::Heap<JS::Value>>;

namespace {
class Timer {
public:
  uint32_t id;
  JS::Heap<JSObject *> callback;
  TimerArgumentsVector arguments;
  uint32_t delay;
  system_clock::time_point deadline;
  bool repeat;

  Timer(uint32_t id, HandleObject callback, uint32_t delay, JS::HandleValueVector args, bool repeat)
      : id(id), callback(callback), delay(delay), repeat(repeat) {
    deadline = system_clock::now() + system_clock::duration(delay * 1000);
    arguments.reserve(args.length());
    for (auto &arg : args) {
      arguments.push_back(JS::Heap(arg));
    }
  }

  void trace(JSTracer *trc) {
    JS::TraceEdge(trc, &callback, "Timer callback");
    for (auto &arg : arguments) {
      JS::TraceEdge(trc, &arg, "Timer callback arguments");
    }
  }
};

class ScheduledTimers {
public:
  Timer *first() {
    if (std::empty(timers)) {
      return nullptr;
    } else {
      return timers.front();
    }
  }

private:
  std::list<Timer *> timers;
  static uint32_t next_id;

  void add_timer(Timer *timer) {
    auto iter = timers.begin();

    for (; iter != timers.end(); iter++) {
      if ((*iter)->deadline > timer->deadline) {
        break;
      }
    }

    timers.insert(iter, timer);
  }

  // `repeat_first` must only be called if the `timers` list is not empty
  // The caller of repeat_first needs to check the `timers` list is not empty
  void repeat_first() {
    Timer *timer = first();
    MOZ_ASSERT(timer);
    timer->deadline = system_clock::now() + milliseconds(timer->delay);
    timers.remove(timer);
    add_timer(timer);
  }

public:
  bool empty() { return timers.empty(); }

  uint32_t add_timer(HandleObject callback, uint32_t delay, JS::HandleValueVector arguments,
                     bool repeat) {
    auto timer = new Timer(next_id++, callback, delay, arguments, repeat);
    add_timer(timer);
    return timer->id;
  }

  void remove_timer(uint32_t id) {
    for (auto timer : timers) {
      if (timer->id == id) {
        timers.remove(timer);
        break;
      }
    }
  }

  bool run_first_timer(JSContext *cx) {
    RootedValue fun_val(cx);
    JS::RootedVector<JS::Value> argv(cx);
    uint32_t id;
    {
      Timer *timer = first();
      MOZ_ASSERT(timer);
      MOZ_ASSERT(system_clock::now() > timer->deadline);
      id = timer->id;
      RootedObject fun(cx, timer->callback);
      fun_val.setObject(*fun.get());
      if (!argv.initCapacity(timer->arguments.size())) {
        JS_ReportOutOfMemory(cx);
        return false;
      }

      for (auto &arg : timer->arguments) {
        argv.infallibleAppend(arg);
      }
    }

    RootedObject fun(cx, &fun_val.toObject());

    RootedValue rval(cx);
    if (!JS::Call(cx, JS::NullHandleValue, fun, argv, &rval)) {
      return false;
    }

    // Repeat / remove the first timer if it's still the one we just ran.
    auto timer = first();
    if (timer && timer->id == id) {
      if (timer->repeat) {
        repeat_first();
      } else {
        remove_timer(timer->id);
      }
    }

    return true;
  }

  void trace(JSTracer *trc) {
    for (auto &timer : timers) {
      timer->trace(trc);
    }
  }
};

} // namespace

uint32_t ScheduledTimers::next_id = 1;
JS::PersistentRooted<js::UniquePtr<ScheduledTimers>> timers;

namespace GlobalProperties {

JS::Result<std::string> ConvertJSValueToByteString(JSContext *cx, JS::Handle<JS::Value> v) {
  JS::RootedString s(cx);
  if (v.isString()) {
    s = v.toString();
  } else {
    s = JS::ToString(cx, v);
    if (!s) {
      JS_ReportErrorNumberUTF8(cx, GetErrorMessage, nullptr, JSMSG_INVALID_CHARACTER_ERROR);
      return JS::Result<std::string>(JS::Error());
    }
  }

  // Conversion from JavaScript string to ByteString is only valid if all
  // characters < 256. This is always the case for Latin1 strings.
  size_t length;
  if (!JS::StringHasLatin1Chars(s)) {
    // Creating an exception can GC, so we first scan the string for bad chars
    // and report the error outside the AutoCheckCannotGC scope.
    bool foundBadChar = false;
    {
      JS::AutoCheckCannotGC nogc;
      const char16_t *chars = JS_GetTwoByteStringCharsAndLength(cx, nogc, s, &length);
      if (!chars) {
        JS_ReportErrorNumberUTF8(cx, GetErrorMessage, nullptr, JSMSG_INVALID_CHARACTER_ERROR);
        return JS::Result<std::string>(JS::Error());
      }

      for (size_t i = 0; i < length; i++) {
        if (chars[i] > 255) {
          foundBadChar = true;
          break;
        }
      }
    }

    if (foundBadChar) {
      JS_ReportErrorNumberUTF8(cx, GetErrorMessage, nullptr, JSMSG_INVALID_CHARACTER_ERROR);
      return JS::Result<std::string>(JS::Error());
    }
  } else {
    length = JS::GetStringLength(s);
  }

  UniqueChars result = JS_EncodeStringToLatin1(cx, s);
  if (!result) {
    return JS::Result<std::string>(JS::Error());
  }
  std::string byteString(result.get(), length);
  return byteString;
}

// Maps an encoded character to a value in the Base64 alphabet, per
// RFC 4648, Table 1. Invalid input characters map to UINT8_MAX.
// https://datatracker.ietf.org/doc/html/rfc4648#section-4

static const uint8_t base64DecodeTable[] = {
    // clang-format off
  /* 0 */  255, 255, 255, 255, 255, 255, 255, 255,
  /* 8 */  255, 255, 255, 255, 255, 255, 255, 255,
  /* 16 */ 255, 255, 255, 255, 255, 255, 255, 255,
  /* 24 */ 255, 255, 255, 255, 255, 255, 255, 255,
  /* 32 */ 255, 255, 255, 255, 255, 255, 255, 255,
  /* 40 */ 255, 255, 255,
  62 /* + */,
  255, 255, 255,
  63 /* / */,

  /* 48 */ /* 0 - 9 */ 52, 53, 54, 55, 56, 57, 58, 59,
  /* 56 */ 60, 61, 255, 255, 255, 255, 255, 255,

  /* 64 */ 255, /* A - Z */ 0, 1, 2, 3, 4, 5, 6,
  /* 72 */ 7, 8, 9, 10, 11, 12, 13, 14,
  /* 80 */ 15, 16, 17, 18, 19, 20, 21, 22,
  /* 88 */ 23, 24, 25, 255, 255, 255, 255, 255,
  /* 96 */ 255, /* a - z */ 26, 27, 28, 29, 30, 31, 32,
  /* 104 */ 33, 34, 35, 36, 37, 38, 39, 40,
  /* 112 */ 41, 42, 43, 44, 45, 46, 47, 48,
  /* 120 */ 49, 50, 51, 255, 255, 255, 255, 255,
};
// clang-format on

bool base64CharacterToValue(char character, uint8_t *value) {
  static const size_t mask = 127;
  auto index = static_cast<size_t>(character);

  if (index & ~mask) {
    return false;
  }
  *value = base64DecodeTable[index & mask];

  return *value != 255;
}

inline JS::Result<mozilla::Ok> base64Decode4to3(std::string_view input, std::string &output) {
  uint8_t w, x, y, z;
  // 8.1 Find the code point pointed to by position in the second column of Table 1: The Base 64
  // Alphabet of RFC 4648. Let n be the number given in the first cell of the same row. [RFC4648]
  if (!base64CharacterToValue(input[0], &w) || !base64CharacterToValue(input[1], &x) ||
      !base64CharacterToValue(input[2], &y) || !base64CharacterToValue(input[3], &z)) {
    return JS::Result<mozilla::Ok>(JS::Error());
  }

  // 8.3 If buffer has accumulated 24 bits, interpret them as three 8-bit big-endian numbers. Append
  // three bytes with values equal to those numbers to output, in the same order, and then empty
  // buffer.
  output += (uint8_t(w << 2 | x >> 4));
  output += (uint8_t(x << 4 | y >> 2));
  output += (uint8_t(y << 6 | z));
  return mozilla::Ok();
}

inline JS::Result<mozilla::Ok> base64Decode3to2(std::string_view input, std::string &output) {
  uint8_t w, x, y;
  // 8.1 Find the code point pointed to by position in the second column of Table 1: The Base 64
  // Alphabet of RFC 4648. Let n be the number given in the first cell of the same row. [RFC4648]
  if (!base64CharacterToValue(input[0], &w) || !base64CharacterToValue(input[1], &x) ||
      !base64CharacterToValue(input[2], &y)) {
    return JS::Result<mozilla::Ok>(JS::Error());
  }
  // 9. If buffer is not empty, it contains either 12 or 18 bits. If it contains 12 bits, then
  // discard the last four and interpret the remaining eight as an 8-bit big-endian number. If it
  // contains 18 bits, then discard the last two and interpret the remaining 16 as two 8-bit
  // big-endian numbers. Append the one or two bytes with values equal to those one or two numbers
  // to output, in the same order.
  output += (uint8_t(w << 2 | x >> 4));
  output += (uint8_t(x << 4 | y >> 2));
  return mozilla::Ok();
}

inline JS::Result<mozilla::Ok> base64Decode2to1(std::string_view input, std::string &output) {
  uint8_t w, x;
  // 8.1 Find the code point pointed to by position in the second column of Table 1: The Base 64
  // Alphabet of RFC 4648. Let n be the number given in the first cell of the same row. [RFC4648]
  if (!base64CharacterToValue(input[0], &w) || !base64CharacterToValue(input[1], &x)) {
    return JS::Result<mozilla::Ok>(JS::Error());
  }
  // 9. If buffer is not empty, it contains either 12 or 18 bits. If it contains 12 bits, then
  // discard the last four and interpret the remaining eight as an 8-bit big-endian number. If it
  // contains 18 bits, then discard the last two and interpret the remaining 16 as two 8-bit
  // big-endian numbers. Append the one or two bytes with values equal to those one or two numbers
  // to output, in the same order.
  output += (uint8_t(w << 2 | x >> 4));
  return mozilla::Ok();
}

bool isAsciiWhitespace(char c) {
  switch (c) {
  case '\t':
  case '\n':
  case '\f':
  case '\r':
  case ' ':
    return true;
  default:
    return false;
  }
}

// https://infra.spec.whatwg.org/#forgiving-base64-decode
JS::Result<std::string> forgivingBase64Decode(std::string_view data) {
  // 1. Remove all ASCII whitespace from data.
  // ASCII whitespace is U+0009 TAB, U+000A LF, U+000C FF, U+000D CR, or U+0020 SPACE.
  auto hasWhitespace = std::find_if(data.begin(), data.end(), &isAsciiWhitespace);
  std::string dataWithoutAsciiWhitespace;

  if (hasWhitespace) {
    dataWithoutAsciiWhitespace = data;
    dataWithoutAsciiWhitespace.erase(std::remove_if(dataWithoutAsciiWhitespace.begin() +
                                                        std::distance(data.begin(), hasWhitespace),
                                                    dataWithoutAsciiWhitespace.end(),
                                                    &isAsciiWhitespace),
                                     dataWithoutAsciiWhitespace.end());
    data = dataWithoutAsciiWhitespace;
  }
  std::string_view data_view(data);
  size_t length = data_view.length();

  // 2. If data’s code point length divides by 4 leaving no remainder, then:
  if (length && (length % 4 == 0)) {
    // 2.1 If data ends with one or two U+003D (=) code points, then remove them from data.
    if (data_view.at(length - 1) == '=') {
      if (data_view.at(length - 2) == '=') {
        data_view.remove_suffix(2);
      } else {
        data_view.remove_suffix(1);
      }
    }
  }

  // 3. If data’s code point length divides by 4 leaving a remainder of 1, then return failure.
  if ((data_view.length() % 4 == 1)) {
    return JS::Result<std::string>(JS::Error());
  }

  // 4. If data contains a code point that is not one of
  //    U+002B (+)
  //    U+002F (/)
  //    ASCII alphanumeric
  // then return failure.

  // Step 4 is handled within the calls below to
  // base64Decode4to3, base64Decode3to2, and base64Decode2to1

  // 5. Let output be an empty byte sequence.
  std::string output = "";
  output.reserve(data_view.length() / 3);

  // 6. Let buffer be an empty buffer that can have bits appended to it.

  // Step 6 is handled within the calls below to
  // base64Decode4to3, base64Decode3to2, and base64Decode2to1

  // 7. Let position be a position variable for data, initially pointing at the start of data.

  // We don't use a position variable, instead we remove_prefix from the `data` each time we have
  // dealt with some characters.

  while (data_view.length() >= 4) {
    MOZ_TRY(base64Decode4to3(data_view, output));
    data_view.remove_prefix(4);
  }

  switch (data_view.length()) {
  case 3: {
    MOZ_TRY(base64Decode3to2(data_view, output));
    break;
  }
  case 2: {
    MOZ_TRY(base64Decode2to1(data_view, output));
    break;
  }
  case 1:
    return JS::Result<std::string>(JS::Error());
  case 0:
    break;
  default:
    MOZ_CRASH("Too many characters leftover");
  }

  return output;
}

// https://html.spec.whatwg.org/multipage/webappapis.html#dom-atob
bool atob(JSContext *cx, unsigned argc, Value *vp) {
  CallArgs args = CallArgsFromVp(argc, vp);
  if (!args.requireAtLeast(cx, "atob", 1)) {
    return false;
  }
  auto dataResult = ConvertJSValueToByteString(cx, args.get(0));
  if (dataResult.isErr()) {
    return false;
  }
  auto data = dataResult.unwrap();

  // 1. Let decodedData be the result of running forgiving-base64 decode on data.
  auto decoded_result = forgivingBase64Decode(data);
  // 2. If decodedData is failure, then throw an "InvalidCharacterError" DOMException.
  if (decoded_result.isErr()) {
    JS_ReportErrorNumberUTF8(cx, GetErrorMessage, nullptr, JSMSG_INVALID_CHARACTER_ERROR);
    return false;
  }
  auto decoded = decoded_result.unwrap();
  RootedString decodedData(cx, JS_NewStringCopyN(cx, decoded.c_str(), decoded.length()));
  if (!decodedData) {
    return false;
  }

  // 3. Return decodedData.
  args.rval().setString(decodedData);
  return true;
}

const char base[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                      "abcdefghijklmnopqrstuvwxyz"
                      "0123456789+/";

inline uint8_t CharTo8Bit(char character) { return uint8_t(character); }
inline void base64Encode3to4(std::string_view data, std::string &output) {
  uint32_t b32 = 0;
  int i, j = 18;

  for (i = 0; i < 3; ++i) {
    b32 <<= 8;
    b32 |= CharTo8Bit(data[i]);
  }

  for (i = 0; i < 4; ++i) {
    output += base[(uint32_t)((b32 >> j) & 0x3F)];
    j -= 6;
  }
}

inline void base64Encode2to4(std::string_view data, std::string &output) {
  uint8_t src0 = CharTo8Bit(data[0]);
  uint8_t src1 = CharTo8Bit(data[1]);
  output += base[(uint32_t)((src0 >> 2) & 0x3F)];
  output += base[(uint32_t)(((src0 & 0x03) << 4) | ((src1 >> 4) & 0x0F))];
  output += base[(uint32_t)((src1 & 0x0F) << 2)];
  output += '=';
}

inline void base64Encode1to4(std::string_view data, std::string &output) {
  uint8_t src0 = CharTo8Bit(data[0]);
  output += base[(uint32_t)((src0 >> 2) & 0x3F)];
  output += base[(uint32_t)((src0 & 0x03) << 4)];
  output += '=';
  output += '=';
}

// https://infra.spec.whatwg.org/#forgiving-base64-encode
// To forgiving-base64 encode given a byte sequence data, apply the base64 algorithm defined in
// section 4 of RFC 4648 to data and return the result. [RFC4648] Note: This is named
// forgiving-base64 encode for symmetry with forgiving-base64 decode, which is different from the
// RFC as it defines error handling for certain inputs.
std::string forgivingBase64Encode(std::string_view data) {
  int length = data.length();
  std::string output = "";
  // The Base64 version of a string will be at least 133% the size of the string.
  output.reserve(length * 1.33);
  while (length >= 3) {
    base64Encode3to4(data, output);
    data.remove_prefix(3);
    length -= 3;
  }

  switch (length) {
  case 2:
    base64Encode2to4(data, output);
    break;
  case 1:
    base64Encode1to4(data, output);
    break;
  case 0:
    break;
  default:
    MOZ_ASSERT_UNREACHABLE("coding error");
  }
  return output;
}

// The btoa(data) method must throw an "InvalidCharacterError" DOMException
// if data contains any character whose code point is greater than U+00FF.
// Otherwise, the user agent must convert data to a byte sequence whose
// nth byte is the eight-bit representation of the nth code point of data,
// and then must apply forgiving-base64 encode to that byte sequence and return the result.
bool btoa(JSContext *cx, unsigned argc, Value *vp) {
  CallArgs args = CallArgsFromVp(argc, vp);

  if (!args.requireAtLeast(cx, "btoa", 1)) {
    return false;
  }

  auto data = args.get(0);
  auto out = args.rval();
  // Note: We do not check if data contains any character whose code point is greater than U+00FF
  // before calling ConvertJSValueToByteString as ConvertJSValueToByteString does the same check
  auto byteStringResult = ConvertJSValueToByteString(cx, data);
  if (byteStringResult.isErr()) {
    return false;
  }
  auto byteString = byteStringResult.unwrap();

  auto result = forgivingBase64Encode(byteString);

  JSString *str = JS_NewStringCopyN(cx, result.c_str(), result.length());
  if (!str) {
    JS_ReportErrorNumberUTF8(cx, GetErrorMessage, nullptr, JSMSG_INVALID_CHARACTER_ERROR);

    return false;
  }

  out.setString(str);
  return true;
}

// TODO: throw in all Request methods/getters that rely on host calls once a
// request has been sent. The host won't let us act on them anymore anyway.
/**
 * The `fetch` global function
 * https://fetch.spec.whatwg.org/#fetch-method
 */
bool fetch(JSContext *cx, unsigned argc, Value *vp) {
  CallArgs args = CallArgsFromVp(argc, vp);

  REQUEST_HANDLER_ONLY("fetch")

  if (!args.requireAtLeast(cx, "fetch", 1)) {
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }

  RootedObject requestInstance(
      cx, JS_NewObjectWithGivenProto(cx, &Request::class_, Request::proto_obj));
  if (!requestInstance)
    return false;

  RootedObject request(cx, Request::create(cx, requestInstance, args[0], args.get(1)));
  if (!request) {
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }

  RootedString backend(cx, Request::backend(request));
  if (!backend) {
    if (builtins::Fastly::allowDynamicBackends) {
      JS::RootedObject dynamicBackend(cx, builtins::Backend::create(cx, request));
      if (!dynamicBackend) {
        return false;
      }
      backend.set(builtins::Backend::name(cx, dynamicBackend));
    } else {
      backend = builtins::Fastly::defaultBackend;
      if (!backend) {
        fastly_request_handle_t handle = Request::request_handle(request);

        xqd_world_string_t uri_str;
        fastly_error_t err;
        if (xqd_fastly_http_req_uri_get(handle, &uri_str, &err)) {
          JS_ReportErrorLatin1(cx,
                               "No backend specified for request with url %s. "
                               "Must provide a `backend` property on the `init` object "
                               "passed to either `new Request()` or `fetch`",
                               uri_str.ptr);
          JS_free(cx, uri_str.ptr);
        } else {
          HANDLE_ERROR(cx, err);
        }
        return ReturnPromiseRejectedWithPendingError(cx, args);
      }
    }
  }

  size_t backend_len;
  UniqueChars backend_chars = encode(cx, backend, &backend_len);
  if (!backend_chars)
    return ReturnPromiseRejectedWithPendingError(cx, args);

  fastly_pending_request_handle_t request_handle = INVALID_HANDLE;
  RootedObject response_promise(cx, JS::NewPromiseObject(cx, nullptr));
  if (!response_promise)
    return ReturnPromiseRejectedWithPendingError(cx, args);

  if (!Request::apply_cache_override(cx, request)) {
    return false;
  }

  bool streaming = false;
  if (!RequestOrResponse::maybe_stream_body(cx, request, &streaming)) {
    return false;
  }

  xqd_world_string_t backend_str = {backend_chars.get(), backend_len};

  {
    fastly_error_t err;
    bool ok;
    if (streaming) {
      ok = xqd_fastly_http_req_send_async_streaming(Request::request_handle(request),
                                                    RequestOrResponse::body_handle(request),
                                                    &backend_str, &request_handle, &err);
    } else {
      ok = xqd_fastly_http_req_send_async(Request::request_handle(request),
                                          RequestOrResponse::body_handle(request), &backend_str,
                                          &request_handle, &err);
    }

    if (!ok) {
      HANDLE_ERROR(cx, err);
      return ReturnPromiseRejectedWithPendingError(cx, args);
    }
  }

  // If the request body is streamed, we need to wait for streaming to complete before marking the
  // request as pending.
  if (!streaming) {
    if (!pending_async_tasks->append(request))
      return ReturnPromiseRejectedWithPendingError(cx, args);
  }

  JS::SetReservedSlot(request, Request::Slots::PendingRequest, JS::Int32Value(request_handle));
  JS::SetReservedSlot(request, Request::Slots::ResponsePromise, JS::ObjectValue(*response_promise));

  args.rval().setObject(*response_promise);
  return true;
}

/**
 * The `queueMicrotask` global function
 * https://html.spec.whatwg.org/multipage/timers-and-user-prompts.html#microtask-queuing
 */
bool queueMicrotask(JSContext *cx, unsigned argc, Value *vp) {
  CallArgs args = CallArgsFromVp(argc, vp);
  if (!args.requireAtLeast(cx, "queueMicrotask", 1)) {
    return false;
  }

  if (!args[0].isObject() || !JS::IsCallable(&args[0].toObject())) {
    JS_ReportErrorLatin1(cx, "queueMicrotask: Argument 1 is not a function");
    return false;
  }

  RootedObject callback(cx, &args[0].toObject());

  RootedObject promise(cx, JS::CallOriginalPromiseResolve(cx, JS::UndefinedHandleValue));
  if (!promise) {
    return false;
  }

  if (!JS::AddPromiseReactions(cx, promise, callback, nullptr)) {
    return false;
  }

  args.rval().setUndefined();
  return true;
}

bool self_get(JSContext *cx, unsigned argc, Value *vp) {
  CallArgs args = CallArgsFromVp(argc, vp);
  args.rval().setObject(*JS::CurrentGlobalOrNull(cx));
  return true;
}

bool self_set(JSContext *cx, unsigned argc, Value *vp) {
  CallArgs args = CallArgsFromVp(argc, vp);
  if (!args.requireAtLeast(cx, "globalThis.self setter", 1)) {
    return false;
  }

  RootedObject global(cx, JS::CurrentGlobalOrNull(cx));
  if (args.thisv() != ObjectValue(*global)) {
    JS_ReportErrorLatin1(cx, "globalThis.self setter can only be called on the global object");
    return false;
  }

  if (!JS_DefineProperty(cx, global, "self", args[0], JSPROP_ENUMERATE)) {
    return false;
  }

  args.rval().setUndefined();
  return true;
}

// Magic number used in structured cloning as a tag to identify a
// URLSearchParam.
#define SCTAG_DOM_URLSEARCHPARAMS JS_SCTAG_USER_MIN

/**
 * Reads non-JS builtins during structured cloning.
 *
 * Currently the only relevant builtin is URLSearchParams, but that'll grow to
 * include Blob and FormData, too.
 */
JSObject *ReadStructuredClone(JSContext *cx, JSStructuredCloneReader *r,
                              const JS::CloneDataPolicy &cloneDataPolicy, uint32_t tag,
                              uint32_t len, void *closure) {
  MOZ_ASSERT(tag == SCTAG_DOM_URLSEARCHPARAMS);

  RootedObject urlSearchParamsInstance(
      cx, JS_NewObjectWithGivenProto(cx, &URLSearchParams::class_, URLSearchParams::proto_obj));
  RootedObject params_obj(cx, URLSearchParams::create(cx, urlSearchParamsInstance));
  if (!params_obj) {
    return nullptr;
  }

  void *bytes = JS_malloc(cx, len);
  if (!bytes) {
    JS_ReportOutOfMemory(cx);
    return nullptr;
  }

  if (!JS_ReadBytes(r, bytes, len)) {
    return nullptr;
  }

  SpecString init((uint8_t *)bytes, len, len);
  jsurl::params_init(URLSearchParams::get_params(params_obj), &init);

  return params_obj;
}

/**
 * Writes non-JS builtins during structured cloning.
 *
 * Currently the only relevant builtin is URLSearchParams, but that'll grow to
 * include Blob and FormData, too.
 */
bool WriteStructuredClone(JSContext *cx, JSStructuredCloneWriter *w, JS::HandleObject obj,
                          bool *sameProcessScopeRequired, void *closure) {
  if (!URLSearchParams::is_instance(obj)) {
    JS_ReportErrorLatin1(cx, "The object could not be cloned");
    return false;
  }

  auto slice = URLSearchParams::serialize(cx, obj);
  if (!JS_WriteUint32Pair(w, SCTAG_DOM_URLSEARCHPARAMS, slice.len) ||
      !JS_WriteBytes(w, (void *)slice.data, slice.len)) {
    return false;
  }

  return true;
}

JSStructuredCloneCallbacks sc_callbacks = {ReadStructuredClone, WriteStructuredClone};

/**
 * The `structuredClone` global function
 * https://html.spec.whatwg.org/multipage/structured-data.html#dom-structuredclone
 */
bool structuredClone(JSContext *cx, unsigned argc, Value *vp) {
  CallArgs args = CallArgsFromVp(argc, vp);
  if (!args.requireAtLeast(cx, "structuredClone", 1)) {
    return false;
  }

  RootedValue transferables(cx);
  if (args.get(1).isObject()) {
    RootedObject options(cx, &args[1].toObject());
    if (!JS_GetProperty(cx, options, "transfer", &transferables)) {
      return false;
    }
  }

  JSAutoStructuredCloneBuffer buf(JS::StructuredCloneScope::SameProcess, &sc_callbacks, nullptr);
  JS::CloneDataPolicy policy;

  if (!buf.write(cx, args[0], transferables, policy)) {
    return false;
  }

  return buf.read(cx, args.rval());
}

/**
 * The `setTimeout` and `setInterval` global functions
 * https://html.spec.whatwg.org/multipage/timers-and-user-prompts.html#dom-settimeout
 * https://html.spec.whatwg.org/multipage/timers-and-user-prompts.html#dom-setinterval
 */
template <bool repeat> bool setTimeout_or_interval(JSContext *cx, unsigned argc, Value *vp) {
  REQUEST_HANDLER_ONLY(repeat ? "setInterval" : "setTimeout");
  CallArgs args = CallArgsFromVp(argc, vp);
  if (!args.requireAtLeast(cx, repeat ? "setInterval" : "setTimeout", 1)) {
    return false;
  }

  if (!(args[0].isObject() && JS::IsCallable(&args[0].toObject()))) {
    JS_ReportErrorASCII(cx, "First argument to %s must be a function",
                        repeat ? "setInterval" : "setTimeout");
    return false;
  }
  RootedObject handler(cx, &args[0].toObject());

  int32_t delay = 0;
  if (args.length() > 1 && !JS::ToInt32(cx, args.get(1), &delay)) {
    return false;
  }
  if (delay < 0) {
    delay = 0;
  }

  JS::RootedValueVector handler_args(cx);
  if (args.length() > 2 && !handler_args.initCapacity(args.length() - 2)) {
    JS_ReportOutOfMemory(cx);
    return false;
  }
  for (size_t i = 2; i < args.length(); i++) {
    handler_args.infallibleAppend(args[i]);
  }

  uint32_t id = timers->add_timer(handler, delay, handler_args, repeat);

  args.rval().setInt32(id);
  return true;
}

/**
 * The `clearTimeout` and `clearInterval` global functions
 * https://html.spec.whatwg.org/multipage/timers-and-user-prompts.html#dom-cleartimeout
 * https://html.spec.whatwg.org/multipage/timers-and-user-prompts.html#dom-clearinterval
 */
template <bool interval> bool clearTimeout_or_interval(JSContext *cx, unsigned argc, Value *vp) {
  // REQUEST_HANDLER_ONLY(interval ? "clearInterval" : "clearTimeout");
  CallArgs args = CallArgsFromVp(argc, vp);
  if (!args.requireAtLeast(cx, interval ? "clearInterval" : "clearTimeout", 1)) {
    return false;
  }

  int32_t id = 0;
  if (!JS::ToInt32(cx, args[0], &id)) {
    return false;
  }

  timers->remove_timer(uint32_t(id));

  args.rval().setUndefined();
  return true;
}

const JSFunctionSpec methods[] = {
    JS_FN("atob", atob, 1, JSPROP_ENUMERATE),
    JS_FN("btoa", btoa, 1, JSPROP_ENUMERATE),
    JS_FN("clearInterval", clearTimeout_or_interval<true>, 1, JSPROP_ENUMERATE),
    JS_FN("clearTimeout", clearTimeout_or_interval<false>, 1, JSPROP_ENUMERATE),
    JS_FN("fetch", fetch, 2, JSPROP_ENUMERATE),
    JS_FN("queueMicrotask", queueMicrotask, 1, JSPROP_ENUMERATE),
    JS_FN("setInterval", setTimeout_or_interval<true>, 1, JSPROP_ENUMERATE),
    JS_FN("setTimeout", setTimeout_or_interval<false>, 1, JSPROP_ENUMERATE),
    JS_FN("structuredClone", structuredClone, 1, JSPROP_ENUMERATE),
    JS_FN("structuredClone", structuredClone, 1, JSPROP_ENUMERATE),
    JS_FS_END};

const JSPropertySpec properties[] = {JS_PSGS("self", self_get, self_set, JSPROP_ENUMERATE),
                                     JS_PS_END};

static bool init(JSContext *cx, HandleObject global) {
  return JS_DefineFunctions(cx, global, methods) && JS_DefineProperties(cx, global, properties);
}
} // namespace GlobalProperties

bool has_pending_async_tasks() { return pending_async_tasks->length() > 0 || !timers->empty(); }

bool process_body_read(JSContext *cx, HandleObject streamSource);

bool process_pending_request(JSContext *cx, HandleObject request) {

  fastly_response_t ret = {INVALID_HANDLE, INVALID_HANDLE};
  fastly_error_t err;
  bool ok = xqd_fastly_http_req_pending_req_wait(Request::pending_handle(request), &ret, &err);
  fastly_response_handle_t response_handle = ret.f0;
  fastly_body_handle_t body = ret.f1;

  RootedObject response_promise(cx, Request::response_promise(request));

  if (!ok) {
    JS_ReportErrorUTF8(cx, "NetworkError when attempting to fetch resource.");
    return RejectPromiseWithPendingError(cx, response_promise);
  }

  RootedObject response_instance(
      cx, JS_NewObjectWithGivenProto(cx, &Response::class_, Response::proto_obj));
  if (!response_instance) {
    return false;
  }

  RootedObject response(cx, Response::create(cx, response_instance, response_handle, body, true));
  if (!response) {
    return false;
  }

  RequestOrResponse::set_url(response, RequestOrResponse::url(request));
  RootedValue response_val(cx, JS::ObjectValue(*response));
  return JS::ResolvePromise(cx, response_promise, response_val);
}

bool error_stream_controller_with_pending_exception(JSContext *cx, HandleObject controller) {
  RootedValue exn(cx);
  if (!JS_GetPendingException(cx, &exn))
    return false;
  JS_ClearPendingException(cx);

  JS::RootedValueArray<1> args(cx);
  args[0].set(exn);
  RootedValue r(cx);
  return JS::Call(cx, controller, "error", args, &r);
}

bool process_body_read(JSContext *cx, HandleObject streamSource) {
  RootedObject owner(cx, builtins::NativeStreamSource::owner(streamSource));
  RootedObject controller(cx, builtins::NativeStreamSource::controller(streamSource));

  fastly_body_handle_t body = RequestOrResponse::body_handle(owner);
  fastly_list_u8_t out_list;
  fastly_error_t err;
  if (!xqd_fastly_http_body_read(body, HANDLE_READ_CHUNK_SIZE, &out_list, &err)) {
    HANDLE_ERROR(cx, err);
    JS_free(cx, out_list.ptr);
    return error_stream_controller_with_pending_exception(cx, controller);
  }

  if (out_list.len == 0) {
    RootedValue r(cx);
    return JS::Call(cx, controller, "close", HandleValueArray::empty(), &r);
  }

  uint8_t *new_bytes =
      static_cast<uint8_t *>(JS_realloc(cx, out_list.ptr, HANDLE_READ_CHUNK_SIZE, out_list.len));
  if (!new_bytes) {
    JS_free(cx, out_list.ptr);
    return error_stream_controller_with_pending_exception(cx, controller);
  }
  out_list.ptr = new_bytes;

  RootedObject buffer(cx, JS::NewArrayBufferWithContents(cx, out_list.len, out_list.ptr));
  if (!buffer) {
    JS_free(cx, out_list.ptr);
    return error_stream_controller_with_pending_exception(cx, controller);
  }

  RootedObject byte_array(cx, JS_NewUint8ArrayWithBuffer(cx, buffer, 0, out_list.len));
  if (!byte_array)
    return false;

  JS::RootedValueArray<1> enqueue_args(cx);
  enqueue_args[0].setObject(*byte_array);
  RootedValue r(cx);
  if (!JS::Call(cx, controller, "enqueue", enqueue_args, &r)) {
    return error_stream_controller_with_pending_exception(cx, controller);
  }

  return true;
}

bool process_pending_async_tasks(JSContext *cx) {
  MOZ_ASSERT(has_pending_async_tasks());

  uint32_t timeout = 0;
  if (!timers->empty()) {
    Timer *timer = timers->first();
    double diff = ceil<milliseconds>(timer->deadline - system_clock::now()).count();

    // If a timeout is already overdue, run it immediately and return.
    if (diff <= 0) {
      return timers->run_first_timer(cx);
    }

    timeout = uint32_t(diff);
  }

  size_t count = pending_async_tasks->length();
  auto handles =
      mozilla::MakeUnique<fastly_async_handle_t[]>(sizeof(fastly_async_handle_t) * count);
  if (!handles) {
    return false;
  }

  for (size_t i = 0; i < count; i++) {
    HandleObject pending_obj = (*pending_async_tasks)[i];
    if (Request::is_instance(pending_obj)) {
      handles[i] = Request::pending_handle(pending_obj);
    } else {
      MOZ_ASSERT(builtins::NativeStreamSource::is_instance(pending_obj));
      RootedObject owner(cx, builtins::NativeStreamSource::owner(pending_obj));
      handles[i] = RequestOrResponse::body_handle(owner);
    }
  }

  fastly_list_async_handle_t handle_list = {handles.get(), count};

  fastly_option_u32_t ret;
  fastly_error_t err;
  if (!xqd_fastly_async_io_select(&handle_list, timeout, &ret, &err)) {
    HANDLE_ERROR(cx, err);
    return false;
  }

  if (!ret.is_some) {
    MOZ_ASSERT(!timers->empty());
    return timers->run_first_timer(cx);
  }

  uint32_t ready_index = ret.val;

  if (ready_index == UINT32_MAX) {
    // The index will be UINT32_MAX if the timeout expires before any objects are ready for I/O.
    return true;
  }

#ifdef DEBUG
  bool is_ready = 0;
  MOZ_ASSERT(xqd_fastly_async_io_is_ready(handles[ready_index], &is_ready, &err));
  MOZ_ASSERT(is_ready);
#endif

  HandleObject ready_obj = (*pending_async_tasks)[ready_index];

  bool ok;
  if (Request::is_instance(ready_obj)) {
    ok = process_pending_request(cx, ready_obj);
  } else {
    MOZ_ASSERT(builtins::NativeStreamSource::is_instance(ready_obj));
    ok = process_body_read(cx, ready_obj);
  }

  pending_async_tasks->erase(const_cast<JSObject **>(ready_obj.address()));
  return ok;
}

bool math_random(JSContext *cx, unsigned argc, Value *vp) {
  uint32_t storage;
  random_get(reinterpret_cast<int32_t>(&storage), sizeof(storage));
  double newvalue = static_cast<double>(storage) / std::pow(2.0, 32.0);

  CallArgs args = CallArgsFromVp(argc, vp);
  args.rval().setDouble(newvalue);
  return true;
}

bool define_fastly_sys(JSContext *cx, HandleObject global) {
  // Allocating the reusable hostcall buffer here means it's baked into the
  // snapshot, and since it's all zeros, it won't increase the size of the
  // snapshot.
  if (!OwnedHostCallBuffer::initialize(cx))
    return false;

  if (!GlobalProperties::init(cx, global))
    return false;

  if (!builtins::Backend::init_class(cx, global))
    return false;
  if (!builtins::Fastly::create(cx, global))
    return false;
  if (!builtins::Console::create(cx, global))
    return false;
  if (!builtins::Crypto::create(cx, global))
    return false;

  if (!builtins::NativeStreamSource::init_class(cx, global))
    return false;
  if (!builtins::NativeStreamSink::init_class(cx, global))
    return false;
  if (!builtins::TransformStreamDefaultController::init_class(cx, global))
    return false;
  if (!builtins::TransformStream::init_class(cx, global))
    return false;
  if (!CompressionStream::init_class(cx, global))
    return false;
  if (!DecompressionStream::init_class(cx, global))
    return false;
  if (!Request::init_class(cx, global))
    return false;
  if (!Response::init_class(cx, global))
    return false;
  if (!builtins::ConfigStore::init_class(cx, global))
    return false;
  if (!builtins::Dictionary::init_class(cx, global))
    return false;
  if (!Headers::init_class(cx, global))
    return false;
  if (!ClientInfo::init_class(cx, global))
    return false;
  if (!FetchEvent::init_class(cx, global))
    return false;
  if (!builtins::CacheOverride::init_class(cx, global))
    return false;
  if (!TextEncoder::init_class(cx, global))
    return false;
  if (!TextDecoder::init_class(cx, global))
    return false;
  if (!builtins::Logger::init_class(cx, global))
    return false;
  if (!URL::init_class(cx, global))
    return false;
  if (!URLSearchParams::init_class(cx, global))
    return false;
  if (!URLSearchParamsIterator::init_class(cx, global))
    return false;
  if (!builtins::WorkerLocation::init_class(cx, global))
    return false;
  if (!ObjectStore::init_class(cx, global))
    return false;
  if (!ObjectStoreEntry::init_class(cx, global))
    return false;

  pending_async_tasks = new JS::PersistentRootedObjectVector(cx);

  timers.init(cx, js::MakeUnique<ScheduledTimers>());

  JS::RootedValue math_val(cx);
  if (!JS_GetProperty(cx, global, "Math", &math_val)) {
    return false;
  }
  JS::RootedObject math(cx, &math_val.toObject());

  const JSFunctionSpec funs[] = {JS_FN("random", math_random, 0, 0), JS_FS_END};
  if (!JS_DefineFunctions(cx, math, funs)) {
    return false;
  }

  return true;
}

JSObject *create_fetch_event(JSContext *cx) { return FetchEvent::create(cx); }

UniqueChars stringify_value(JSContext *cx, JS::HandleValue value) {
  JS::RootedString str(cx, JS_ValueToSource(cx, value));
  if (!str)
    return nullptr;

  return JS_EncodeStringToUTF8(cx, str);
}

bool debug_logging_enabled() { return builtins::Fastly::debug_logging_enabled; }

bool dump_value(JSContext *cx, JS::Value val, FILE *fp) {
  RootedValue value(cx, val);
  UniqueChars utf8chars = stringify_value(cx, value);
  if (!utf8chars)
    return false;
  fprintf(fp, "%s\n", utf8chars.get());
  return true;
}

bool print_stack(JSContext *cx, HandleObject stack, FILE *fp) {
  RootedString stackStr(cx);
  if (!BuildStackString(cx, nullptr, stack, &stackStr, 2)) {
    return false;
  }
  size_t stack_len;

  UniqueChars utf8chars = encode(cx, stackStr, &stack_len);
  if (!utf8chars)
    return false;
  fprintf(fp, "%s\n", utf8chars.get());
  return true;
}

void dump_promise_rejection(JSContext *cx, HandleValue reason, HandleObject promise, FILE *fp) {
  bool reported = false;
  RootedObject stack(cx);

  if (reason.isObject()) {
    RootedObject err(cx, &reason.toObject());
    JSErrorReport *report = JS_ErrorFromException(cx, err);
    if (report) {
      fprintf(stderr, "%s\n", report->message().c_str());
      reported = true;
    }

    stack = JS::ExceptionStackOrNull(err);
  }

  // If the rejection reason isn't an `Error` object, we just dump the value
  // as-is.
  if (!reported) {
    dump_value(cx, reason, stderr);
  }

  // If the rejection reason isn't an `Error` object, we can't get an exception
  // stack from it. In that case, fall back to getting the stack from the
  // promise resolution site. These should be identical in many cases, such as
  // for exceptions thrown in async functions, but for some reason the
  // resolution site stack seems to sometimes be wrong, so we only fall back to
  // it as a last resort.
  if (!stack) {
    stack = JS::GetPromiseResolutionSite(promise);
  }

  if (stack) {
    fprintf(stderr, "Stack:\n");
    print_stack(cx, stack, stderr);
  }
}

bool print_stack(JSContext *cx, FILE *fp) {
  RootedObject stackp(cx);
  if (!JS::CaptureCurrentStack(cx, &stackp))
    return false;
  return print_stack(cx, stackp, fp);
}
