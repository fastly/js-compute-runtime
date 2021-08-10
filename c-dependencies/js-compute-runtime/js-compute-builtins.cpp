#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <arpa/inet.h>

#include "../xqd.h"
#include "js-compute-builtins.h"

#include "rust-url/rust-url.h"

#include "js/Array.h"
#include "js/ArrayBuffer.h"
#include "js/Conversions.h"
#include "js/experimental/TypedData.h"
#include "js/JSON.h"
#include "js/shadow/Object.h"
#include "js/Stream.h"
#include "js/Value.h"

using JS::CallArgs;
using JS::CallArgsFromVp;
using JS::UniqueChars;

using JS::Value;

using JS::RootedValue;
using JS::RootedObject;
using JS::RootedString;

using JS::HandleValue;
using JS::HandleValueArray;
using JS::HandleObject;
using JS::HandleString;
using JS::MutableHandleValue;

using JS::PersistentRooted;

// Ensure that all the things we want to use the hostcall buffer for actually fit into the buffer.
#define HOSTCALL_BUFFER_LEN DICTIONARY_ENTRY_MAX_LEN
static_assert(HEADER_MAX_LEN < HOSTCALL_BUFFER_LEN);
static_assert(METHOD_MAX_LEN < HOSTCALL_BUFFER_LEN);
static_assert(URI_MAX_LEN < HOSTCALL_BUFFER_LEN);


class OwnedHostCallBuffer {
  static char* hostcall_buffer;
  char* borrowed_buffer;

public:
  static bool initialize(JSContext* cx) {
    // Ensure the buffer is all zeros so it doesn't add too much to the snapshot.
    hostcall_buffer = (char*)js_calloc(HOSTCALL_BUFFER_LEN);
    return !!hostcall_buffer;
  }

  OwnedHostCallBuffer() {
    MOZ_RELEASE_ASSERT(hostcall_buffer != nullptr);
    borrowed_buffer = hostcall_buffer;
    hostcall_buffer = nullptr;
  }

  char* get() {
    return borrowed_buffer;
  }

  ~OwnedHostCallBuffer() {
    // TODO: consider adding a build config that makes this zero the buffer.
    hostcall_buffer = borrowed_buffer;
  }
};

char* OwnedHostCallBuffer::hostcall_buffer;

using jsurl::SpecSlice, jsurl::SpecString, jsurl::JSUrl, jsurl::JSUrlSearchParams, jsurl::JSSearchParam;

static JS::PersistentRootedObjectVector* pending_requests;
static JS::PersistentRootedObjectVector* pending_body_reads;

// TODO: introduce a version that writes into an existing buffer, and use that
// with the hostcall buffer where possible.
UniqueChars encode(JSContext* cx, HandleString str, size_t* encoded_len) {
  UniqueChars text = JS_EncodeStringToUTF8(cx, str);
  if (!text)
    return nullptr;

  // This shouldn't fail, since the encode operation ensured `str` is linear.
  JSLinearString* linear = JS_EnsureLinearString(cx, str);
  *encoded_len = JS::GetDeflatedUTF8StringLength(linear);
  return text;
}

UniqueChars encode(JSContext* cx, HandleValue val, size_t* encoded_len) {
  RootedString str(cx, JS::ToString(cx, val));
  if (!str) return nullptr;

  return encode(cx, str, encoded_len);
}

SpecString encode(JSContext* cx, HandleValue val) {
  SpecString slice(nullptr, 0, 0);
  auto chars = encode(cx, val, &slice.len);
  if (!chars) return slice;
  slice.data = (uint8_t*)chars.release();
  slice.cap = slice.len;
  return slice;
}

/* Returns false if an exception is set on `cx` and the caller should immediately
   return to propagate the exception. */
static inline bool handle_fastly_result(JSContext* cx, int result, int line, const char* func) {
    if (result == 0) {
        return true;
    } else {
        fprintf(stdout, __FILE__":%d (%s) - Fastly error code %d\n", line, func, result);
        JS_ReportErrorUTF8(cx, "Fastly error code %d", result);
        return false;
    }
}

#define HANDLE_RESULT(cx, result) \
  handle_fastly_result(cx, result, __LINE__, __func__)

#define DBG(...) \
  printf("%s#%d: ", __func__, __LINE__); printf(__VA_ARGS__); fflush(stdout);

static bool rejected_promise_with_current_exception(JSContext* cx, CallArgs* args = nullptr) {
  RootedValue exn(cx);
  if (!JS_GetPendingException(cx, &exn)) return false;
  RootedObject promise(cx, JS::CallOriginalPromiseReject(cx, exn));
  if (!promise) return false;
  if (args) {
    args->rval().setObject(*promise);
  }
  return true;
}

static bool resolved_promise_with_value(JSContext* cx, JS::HandleValue value,
                                        CallArgs* args = nullptr)
{
  RootedObject promise(cx, JS::CallOriginalPromiseResolve(cx, value));
  if (!promise) return false;
  if (args) {
    args->rval().setObject(*promise);
  }
  return true;
}

JSObject* PromiseRejectedWithPendingError(JSContext* cx) {
  RootedValue exn(cx);
  if (!JS_IsExceptionPending(cx) || !JS_GetPendingException(cx, &exn)) {
    return nullptr;
  }
  JS_ClearPendingException(cx);
  return JS::CallOriginalPromiseReject(cx, exn);
}

inline bool ReturnPromiseRejectedWithPendingError(JSContext* cx, const JS::CallArgs& args) {
  JSObject* promise = PromiseRejectedWithPendingError(cx);
  if (!promise) {
    return false;
  }

  args.rval().setObject(*promise);
  return true;
}

#define HANDLE_READ_CHUNK_SIZE 1024

template<auto op, class HandleType>
static char* read_from_handle_all(JSContext* cx, HandleType handle,
                                  size_t* nwritten, bool read_until_zero)
{
  // TODO: investigate passing a size hint in situations where we might know
  // the final size, e.g. via the `content-length` header.
  size_t buf_size = HANDLE_READ_CHUNK_SIZE;
  // TODO: make use of malloc slack.
  char* buf = static_cast<char*>(JS_malloc(cx, buf_size));
  if (!buf) {
      return nullptr;
  }

  // For realloc below.
  char* new_buf;

  size_t offset = 0;
  while (true) {
      size_t num_written = 0;
      int result = op(handle, buf + offset, HANDLE_READ_CHUNK_SIZE, &num_written);
      if (!HANDLE_RESULT(cx, result)) {
          JS_free(cx, buf);
          return nullptr;
      }

      offset += num_written;
      if (num_written == 0 ||
          (!read_until_zero && num_written < HANDLE_READ_CHUNK_SIZE))
      {
          break;
      }

      // TODO: make use of malloc slack, and use a smarter buffer growth strategy.
      size_t new_size = buf_size + HANDLE_READ_CHUNK_SIZE;
      new_buf = static_cast<char*>(JS_realloc(cx, buf, buf_size, new_size));
      if (!new_buf) {
        JS_free(cx, buf);
        return nullptr;
      }
      buf = new_buf;

      buf_size += HANDLE_READ_CHUNK_SIZE;
  }

  new_buf = static_cast<char*>(JS_realloc(cx, buf, buf_size, offset + 1));
  if (!buf) {
    JS_free(cx, buf);
    return nullptr;
  }
  buf = new_buf;

  buf[offset] = '\0';
  *nwritten = offset;

  return buf;
}

#define MULTI_VALUE_HOSTCALL(op, accum) \
    uint32_t cursor = 0; \
    int64_t ending_cursor = 0; \
    size_t nwritten; \
 \
    while (true) { \
        op \
 \
        if (nwritten == 0) { \
            break; \
        } \
 \
        accum \
 \
        if (ending_cursor < 0) { \
            break; \
        } \
 \
        cursor = (uint32_t)ending_cursor; \
    }

static constexpr const JSClassOps class_ops = {};
static const uint32_t class_flags = 0;

#define CLASS_BOILERPLATE_CUSTOM_INIT(cls) \
  const JSClass class_ = { #cls, JSCLASS_HAS_RESERVED_SLOTS(Slots::Count) | class_flags, \
                           &class_ops }; \
  static PersistentRooted<JSObject*> proto_obj; \
 \
  bool is_instance(JSObject* obj) { \
    return JS::GetClass(obj) == &class_; \
  } \
 \
  bool is_instance(JS::Value val) { \
    return val.isObject() && is_instance(&val.toObject()); \
  } \
 \
  bool check_receiver(JSContext* cx, HandleObject self, const char* method_name) { \
    if (!is_instance(self)) { \
      JS_ReportErrorUTF8(cx, "Method %s called on receiver that's not an instance of %s\n", \
                         method_name, class_.name); \
      return false; \
    } \
    return true; \
  }; \
 \
  bool init_class_impl(JSContext* cx, HandleObject global, \
                                    HandleObject parent_proto = nullptr) \
  { \
    proto_obj.init(cx, JS_InitClass(cx, global, parent_proto, &class_, constructor, ctor_length, \
                                    properties, methods, nullptr, nullptr)); \
    return proto_obj; \
  }; \

#define CLASS_BOILERPLATE(cls) \
  CLASS_BOILERPLATE_CUSTOM_INIT(cls) \
 \
  bool init_class(JSContext* cx, HandleObject global) { \
    return init_class_impl(cx, global); \
  } \

#define CLASS_BOILERPLATE_NO_CTOR(cls) \
  bool constructor(JSContext* cx, unsigned argc, Value* vp) { \
    JS_ReportErrorUTF8(cx, #cls " can't be instantiated directly"); \
    return false; \
  } \
 \
  CLASS_BOILERPLATE_CUSTOM_INIT(cls) \
 \
  bool init_class(JSContext* cx, HandleObject global) { \
    /* Right now, deleting the ctor from the global object after class \
       initialization seems to be the best we can do. Not ideal, but works. */ \
    return init_class_impl(cx, global) && \
           JS_DeleteProperty(cx, global, class_.name); \
  } \

#define METHOD_HEADER_WITH_NAME(required_argc, name) \
  /* \
  // printf("method: %s\n", name); \
  */ \
  CallArgs args = CallArgsFromVp(argc, vp); \
  if (!args.requireAtLeast(cx, name, required_argc)) \
    return false; \
  RootedObject self(cx, &args.thisv().toObject()); \
  if (!check_receiver(cx, self, name)) return false; \

#define METHOD_HEADER(required_argc) \
  METHOD_HEADER_WITH_NAME(required_argc, __func__)

namespace Logger {
  namespace Slots { enum {
    Endpoint,
    Count
  };};

  const unsigned ctor_length = 1;

  bool check_receiver(JSContext* cx, HandleObject self, const char* method_name);

  static bool log(JSContext* cx, unsigned argc, Value* vp) {
    METHOD_HEADER(1)

    auto endpoint = LogEndpointHandle { (uint32_t)JS::GetReservedSlot(self, Slots::Endpoint).toInt32() };

    size_t msg_len;
    UniqueChars msg = encode(cx, args.get(0), &msg_len);
    if (!msg) return false;

    size_t nwritten;
    if (!HANDLE_RESULT(cx, xqd_log_write(endpoint, msg.get(), msg_len, &nwritten)))
      return false;

    args.rval().setUndefined();
    return true;
  }

  const JSFunctionSpec methods[] = {
    JS_FN("log", log, 1, JSPROP_ENUMERATE),
    JS_FS_END
  };

  const JSPropertySpec properties[] = {JS_PS_END};

  CLASS_BOILERPLATE_NO_CTOR(Logger)

  JSObject* create(JSContext* cx, const char* name) {
    RootedObject logger(cx, JS_NewObjectWithGivenProto(cx, &class_, proto_obj));
    if (!logger) return nullptr;

    auto handle = LogEndpointHandle { INVALID_HANDLE };

    if (!HANDLE_RESULT(cx, xqd_log_endpoint_get(name, strlen(name), &handle)))
      return nullptr;

    JS::SetReservedSlot(logger, Slots::Endpoint, JS::Int32Value(handle.handle));

    return logger;
  }
}

static JSString* get_geo_info(JSContext* cx, HandleString address_str);

namespace Fastly {

  static bool debug_logging_enabled = false;

  bool dump(JSContext* cx, unsigned argc, Value* vp) {
    CallArgs args = CallArgsFromVp(argc, vp);
    if (!args.requireAtLeast(cx, __func__, 1))
      return false;

    dump_value(cx, args[0], stdout);

    args.rval().setUndefined();
    return true;
  }

  bool enableDebugLogging(JSContext* cx, unsigned argc, Value* vp) {
    CallArgs args = CallArgsFromVp(argc, vp);
    if (!args.requireAtLeast(cx, __func__, 1))
      return false;

    debug_logging_enabled = JS::ToBoolean(args[0]);

    args.rval().setUndefined();
    return true;
  }

  bool getGeolocationForIpAddress(JSContext* cx, unsigned argc, Value* vp) {
    CallArgs args = CallArgsFromVp(argc, vp);
    if (!args.requireAtLeast(cx, __func__, 1))
      return false;

    RootedString address_str(cx, JS::ToString(cx, args[0]));
    if (!address_str)
      return false;

    RootedString geo_info_str(cx, get_geo_info(cx, address_str));
    if (!geo_info_str) return false;

    return JS_ParseJSON(cx, geo_info_str, args.rval());
  }

  // TODO: throw a proper error when trying to create a logger during the init phase.
  // Alternatively, allow doing so to reduce per-request work, but then throw when trying
  // to log.
  bool getLogger(JSContext* cx, unsigned argc, Value* vp) {
    CallArgs args = CallArgsFromVp(argc, vp);
    RootedObject self(cx, &args.thisv().toObject());
    if (!args.requireAtLeast(cx, "fastly.getLogger", 1))
      return false;

    size_t name_len;
    UniqueChars name = encode(cx, args[0], &name_len);
    if (!name) return false;

    RootedObject logger(cx, Logger::create(cx, name.get()));
    if (!logger) return false;

    args.rval().setObject(*logger);
    return true;
  }

  const JSFunctionSpec methods[] = {
    JS_FN("dump", dump, 1, 0),
    JS_FN("enableDebugLogging", enableDebugLogging, 1, JSPROP_ENUMERATE),
    JS_FN("getGeolocationForIpAddress", getGeolocationForIpAddress, 1, JSPROP_ENUMERATE),
    JS_FN("getLogger", getLogger, 1, JSPROP_ENUMERATE),
    JS_FS_END
  };

  static bool create(JSContext* cx, HandleObject global) {
    RootedObject fastly(cx, JS_NewPlainObject(cx));
    if (!fastly) return false;

    if (!JS_DefineProperty(cx, global, "fastly", fastly, 0)) return false;
    return JS_DefineFunctions(cx, fastly, methods);
  }
}

namespace Console {
  template<const char* prefix, uint8_t prefix_len>
  static bool console_out(JSContext* cx, unsigned argc, Value* vp) {
    CallArgs args = CallArgsFromVp(argc, vp);
    size_t msg_len;
    UniqueChars msg = encode(cx, args.get(0), &msg_len);
    if (!msg) return false;

    printf("%s: %s\n", prefix, msg.get());
    fflush(stdout);

    args.rval().setUndefined();
    return true;
  }

  static constexpr char PREFIX_LOG[] = "Log";
  static constexpr char PREFIX_TRACE[] = "Trace";
  static constexpr char PREFIX_INFO[] = "Info";
  static constexpr char PREFIX_WARN[] = "Warn";
  static constexpr char PREFIX_ERROR[] = "Error";

  const JSFunctionSpec methods[] = {
    JS_FN("log", (console_out<PREFIX_LOG, 3>), 1, JSPROP_ENUMERATE),
    JS_FN("trace", (console_out<PREFIX_TRACE, 5>), 1, JSPROP_ENUMERATE),
    JS_FN("info", (console_out<PREFIX_INFO, 4>), 1, JSPROP_ENUMERATE),
    JS_FN("warn", (console_out<PREFIX_WARN, 4>), 1, JSPROP_ENUMERATE),
    JS_FN("error", (console_out<PREFIX_ERROR, 5>), 1, JSPROP_ENUMERATE),
    JS_FS_END
  };

  static bool create(JSContext* cx, HandleObject global) {
    RootedObject console(cx, JS_NewPlainObject(cx));
    if (!console) return false;
    if (!JS_DefineProperty(cx, global, "console", console, JSPROP_ENUMERATE)) return false;
    return JS_DefineFunctions(cx, console, methods);
  }
}

bool is_int_typed_array(JSObject* obj) {
  return JS_IsInt8Array(obj) ||
         JS_IsUint8Array(obj) ||
         JS_IsInt16Array(obj) ||
         JS_IsUint16Array(obj) ||
         JS_IsInt32Array(obj) ||
         JS_IsUint32Array(obj) ||
         JS_IsUint8ClampedArray(obj);
}

namespace Crypto {

  #define MAX_BYTE_LENGTH 65536

  /**
   * Implementation of https://www.w3.org/TR/WebCryptoAPI/#Crypto-method-getRandomValues
   * TODO: investigate ways to automatically wipe the buffer passed in here when it is
   * GC'd. Content can roughly approximate that using finalizers for views of the buffer,
   * but it's far from ideal.
   */
  bool get_random_values(JSContext* cx, unsigned argc, Value* vp) {
    CallArgs args = CallArgsFromVp(argc, vp);
    if (!args.requireAtLeast(cx, "crypto.getRandomValues", 1))
      return false;

    if (!args[0].isObject() || !is_int_typed_array(&args[0].toObject())) {
      JS_ReportErrorUTF8(cx, "crypto.getRandomValues: input must be an integer-typed TypedArray");
      return false;
    }

    RootedObject typed_array(cx, &args[0].toObject());
    size_t byte_length = JS_GetArrayBufferViewByteLength(typed_array);
    if (byte_length > MAX_BYTE_LENGTH) {
      JS_ReportErrorUTF8(cx, "crypto.getRandomValues: input byteLength must be at most %u, "
                              "but is %zu", MAX_BYTE_LENGTH, byte_length);
        return false;
    }

    JS::AutoAssertNoGC noGC(cx);
    bool is_shared;
    void* buffer = JS_GetArrayBufferViewData(typed_array, &is_shared, noGC);
    arc4random_buf(buffer, byte_length);

    args.rval().setObject(*typed_array);
    return true;
  }

  const JSFunctionSpec methods[] = {
    JS_FN("getRandomValues", get_random_values, 1, JSPROP_ENUMERATE),
    JS_FS_END
  };

  static bool create(JSContext* cx, HandleObject global) {
    RootedObject crypto(cx, JS_NewPlainObject(cx));
    if (!crypto) return false;
    if (!JS_DefineProperty(cx, global, "crypto", crypto, JSPROP_ENUMERATE)) return false;
    return JS_DefineFunctions(cx, crypto, methods);
  }
}

namespace BodyStreamSource {
  JSObject* create(JSContext* cx, HandleObject owner);
  static JSObject* get_stream_source(JSObject* stream);
  bool stream_has_native_source(JSObject* stream);
  bool lock_stream(JSContext* cx, HandleObject stream);
}

enum class BodyReadResult {
    ArrayBuffer,
    JSON,
    Text,
};

namespace Headers {
  enum class Mode : int32_t {
    Standalone,
    ProxyToRequest,
    ProxyToResponse
  };

  JSObject* create(JSContext* cx, Mode mode, HandleObject owner);
  JSObject* create(JSContext* cx, Mode mode, HandleObject owner, HandleValue initv);

  bool delazify(JSContext* cx, HandleObject headers);
}

namespace Request {
  bool is_instance(JSObject* obj);
}

namespace Response {
  bool is_instance(JSObject* obj);
}

namespace RequestOrResponse {
  namespace Slots { enum {
    RequestOrResponse,
    Body,
    BodyStream,
    HasBody,
    BodyUsed,
    Headers,
    URL,
    Count
  };};

  bool is_instance(JSObject* obj) {
    return Request::is_instance(obj) || Response::is_instance(obj);
  }

  uint32_t handle(JSObject* obj) {
    return static_cast<uint32_t>(JS::GetReservedSlot(obj, Slots::RequestOrResponse).toInt32());
  }

  bool has_body(JSObject* obj) {
    return JS::GetReservedSlot(obj, Slots::HasBody).toBoolean();
  }

  BodyHandle body_handle(JSObject* obj) {
    return BodyHandle { static_cast<uint32_t>(JS::GetReservedSlot(obj, Slots::Body).toInt32()) };
  }

  JSObject* body_stream(JSObject* obj) {
    // Can't use toObjectOrNull here because the Value might be `undefined`.
    Value val = JS::GetReservedSlot(obj, Slots::BodyStream);
    return val.isNullOrUndefined() ? nullptr : &val.toObject();
  }

  JSObject* body_source(JSObject* obj) {
    MOZ_RELEASE_ASSERT(has_body(obj));
    return BodyStreamSource::get_stream_source(body_stream(obj));
  }

  bool mark_body_used(JSContext* cx, HandleObject obj) {
    JS::SetReservedSlot(obj, Slots::BodyUsed, JS::BooleanValue(true));

    RootedObject stream(cx, body_stream(obj));
    if (stream)
      return BodyStreamSource::lock_stream(cx, stream);

    return true;
  }

  /**
   * Moves an underlying body handle from one Request/Response object to another.
   *
   * Also marks the source object's body as consumed.
   */
  bool move_body_handle(JSContext* cx, HandleObject from, HandleObject to) {
    MOZ_RELEASE_ASSERT(is_instance(from));
    MOZ_RELEASE_ASSERT(is_instance(to));


    // Replace the receiving object's body handle with the body stream source's underlying handle.
    // TODO: Let the host know we'll not use the old handle anymore, once C@E has a hostcall
    // for that.
    BodyHandle body = body_handle(from);
    JS::SetReservedSlot(to, Slots::Body, JS::Int32Value(body.handle));

    // Mark the source's body as used, and the stream as locked to prevent any future attempts
    // to use the underlying handle we just removed.
    return mark_body_used(cx, from);
  }

  Value url(JSObject* obj) {
    Value val = JS::GetReservedSlot(obj, Slots::URL);
    MOZ_RELEASE_ASSERT(val.isString());
    return val;
  }

  void set_url(JSObject* obj, Value url) {
    MOZ_RELEASE_ASSERT(url.isString());
    JS::SetReservedSlot(obj, Slots::URL, url);
  }

  bool body_used(JSObject* obj) {
    return JS::GetReservedSlot(obj, Slots::BodyUsed).toBoolean();
  }

  bool set_body(JSContext*cx, HandleObject obj, HandleValue body_val) {
    if (body_val.isNullOrUndefined()) {
      JS::SetReservedSlot(obj, Slots::HasBody, JS::BooleanValue(false));
      return true;
    }

    // TODO: throw if method is GET or HEAD.

    // TODO: properly implement the spec steps for extracting the body and setting the content-type.
    // (https://fetch.spec.whatwg.org/#dom-response)

    if (body_val.isObject() && JS::IsReadableStream(&body_val.toObject())) {
      JS_SetReservedSlot(obj, RequestOrResponse::Slots::BodyStream, body_val);
    } else {
      // TODO: Support BufferSource and the other possible inputs to Body.
      BodyHandle body_handle = RequestOrResponse::body_handle(obj);

      size_t utf8Length;
      UniqueChars text = encode(cx, body_val, &utf8Length);
      if (!text) return false;

      size_t num_written = 0;
      int result = xqd_body_write(body_handle, text.get(), utf8Length, BodyWriteEndBack,
                                  &num_written);
      if (!HANDLE_RESULT(cx, result))
        return false;
    }

    JS::SetReservedSlot(obj, Slots::HasBody, JS::BooleanValue(true));
    return true;
  }

  template<auto mode>
  JSObject* headers(JSContext*cx, HandleObject obj) {
    Value val = JS::GetReservedSlot(obj, Slots::Headers);
    if (val.isNullOrUndefined()) {
      JSObject* headers = Headers::create(cx, mode, obj);
      if (!headers) return nullptr;
      val = JS::ObjectValue(*headers);
      JS_SetReservedSlot(obj, Slots::Headers, val);
    }
    return &val.toObject();
  }

  template<BodyReadResult result_type>
  bool bodyAll(JSContext* cx, CallArgs args, HandleObject self) {
    // TODO: mark body as consumed when operating on stream, too.
    if (body_used(self)) {
      JS_ReportErrorASCII(cx, "Body has already been consumed");
      return rejected_promise_with_current_exception(cx, &args);
    }

    BodyHandle body = body_handle(self);

    // TODO: check if this should be lazified. JS code might expect to be able to trigger
    // multiple requests for response body contents in parallel instead of blocking
    // on them sequentially.
    size_t bytes_read;
    UniqueChars buf(read_from_handle_all<xqd_body_read, BodyHandle>(cx, body, &bytes_read, true));
    if (!buf) {
      return rejected_promise_with_current_exception(cx, &args);
    }

    if (!mark_body_used(cx, self))
      return false;

    RootedValue result(cx);

    if (result_type == BodyReadResult::ArrayBuffer) {
      auto* rawBuf = buf.release();
      RootedObject array_buffer(cx, JS::NewArrayBufferWithContents(cx, bytes_read, rawBuf));
      if (!array_buffer) {
        JS_free(cx, rawBuf);
        return rejected_promise_with_current_exception(cx, &args);
      }
      result.setObject(*array_buffer);
    } else {
      RootedString text(cx, JS_NewStringCopyUTF8N(cx, JS::UTF8Chars(buf.get(), bytes_read)));
      if (!text) {
        return rejected_promise_with_current_exception(cx, &args);
      }

      if (result_type == BodyReadResult::Text) {
        result.setString(text);
      } else if (result_type == BodyReadResult::JSON) {
          if (!JS_ParseJSON(cx, text, &result)) {
            return rejected_promise_with_current_exception(cx, &args);
          }
      } else {
        MOZ_ASSERT_UNREACHABLE("Unsupported body read result type");
      }
    }

    return resolved_promise_with_value(cx, result, &args);
  }

  JSObject* create_body_stream(JSContext* cx, HandleObject owner) {
    RootedObject source(cx, BodyStreamSource::create(cx, owner));
    if (!source) return nullptr;

    // Create a readable stream with a highwater mark of 0.0 to prevent an eager pull.
    // With the default HWM of 1.0, the streams implementation causes a pull, which means
    // we enqueue a read from the host handle, which we quite often have no interest in
    // at all.
    RootedObject body_stream(cx, JS::NewReadableDefaultStreamObject(cx, source, nullptr, 0.0));
    if (!body_stream) return nullptr;

    // This assert guarantees that we fail early in case SpiderMonkey ever changes how
    // the underlying source for streams is stored, instead of operating on invalid
    // values.
    MOZ_RELEASE_ASSERT(BodyStreamSource::get_stream_source(body_stream) == source);

    JS_SetReservedSlot(owner, RequestOrResponse::Slots::BodyStream, JS::ObjectValue(*body_stream));
    return body_stream;
  }

  bool body_get(JSContext* cx, CallArgs args, HandleObject self, bool create_if_undefined) {
    if (!has_body(self)) {
      args.rval().setNull();
      return true;
    }

    RootedObject body_stream(cx, ::RequestOrResponse::body_stream(self));
    if (!body_stream && create_if_undefined) {
      body_stream = create_body_stream(cx, self);
      if (!body_stream) return false;
    }

    if (body_stream) {
      args.rval().setObject(*body_stream);
    }

    return true;
  }
}

static JS::Value get_fixed_slot(JSObject* obj, size_t slot) {
  const auto* nobj = reinterpret_cast<const JS::shadow::Object*>(obj);
  return nobj->fixedSlots()[slot];
}

// A JS class to use as the underlying source for native body streams.
// In principle, SpiderMonkey has the concept of a native ReadableStreamUnderlyingSource
// for just that, but in practice making that work turned out to be extremely difficult,
// because it involves GC tracing through a non-GC object (because our underlying source
// needs access to the Request/Response object), which is ... non-trivial.
//
// This is a bit unfortunate, because SpiderMonkey doesn't provide an embedding API for
// retrieving a JS object as the underlying source, so we have to resort to retrieving
// it from a fixed slot directly, which isn't API exposed and could change.
// To ensure we don't accidentally do unsafe things, we use various release asserts
// verifying that we operate on the objects we expect. SpiderMonkey might change how
// it stores the source, but at least we'd fail early.
namespace BodyStreamSource {
  namespace Slots { enum {
    Owner, // Request or Response object.
    Controller, // The ReadableStreamDefaultController.
    InternalReader, // Only used to lock the stream if it's consumed internally.
    Count
  };};

  bool is_instance(JSObject* obj);

  // Fixed slot SpiderMonkey stores the controller in on ReadableStream objects.
  #define STREAM_SLOT_CONTROLLER 1
  // Fixed slot SpiderMonkey stores the underlying source in on ReadableStream
  // controller objects.
  #define CONTROLLER_SLOT_SOURCE 3

  JSObject* owner(JSObject* self) {
    return &JS::GetReservedSlot(self, Slots::Owner).toObject();
  }

  JSObject* controller(JSObject* self) {
    return &JS::GetReservedSlot(self, Slots::Controller).toObject();
  }

  static JSObject* get_controller_source(JSObject* controller) {
    return &get_fixed_slot(controller, CONTROLLER_SLOT_SOURCE).toObject();
  }

  static JSObject* get_stream_source(JSObject* stream) {
    JSObject* controller = &get_fixed_slot(stream, STREAM_SLOT_CONTROLLER).toObject();
    return get_controller_source(controller);
  }

  bool stream_has_native_source(JSObject* stream) {
    MOZ_RELEASE_ASSERT(JS::IsReadableStream(stream));

    JSObject* source = get_stream_source(stream);
    return is_instance(source);
  }

  bool lock_stream(JSContext* cx, HandleObject stream) {
    MOZ_RELEASE_ASSERT(JS::IsReadableStream(stream));

    bool locked;
    JS::ReadableStreamIsLocked(cx, stream, &locked);
    MOZ_RELEASE_ASSERT(!locked);

    RootedObject self(cx, get_stream_source(stream));
    MOZ_RELEASE_ASSERT(is_instance(self));

    auto mode = JS::ReadableStreamReaderMode::Default;
    RootedObject reader(cx, JS::ReadableStreamGetReader(cx, stream, mode));
    if (!reader)
      return false;

    JS::SetReservedSlot(self, Slots::InternalReader, JS::ObjectValue(*reader));
    return true;
  }

  const unsigned ctor_length = 0;

  bool start(JSContext* cx, unsigned argc, Value* vp) {
    CallArgs args = CallArgsFromVp(argc, vp);
    RootedObject self(cx, &args.thisv().toObject());

    MOZ_RELEASE_ASSERT(args[0].isObject());
    RootedObject controller(cx, &args[0].toObject());
    MOZ_RELEASE_ASSERT(get_controller_source(controller) == self);

    JS::SetReservedSlot(self, Slots::Controller, args[0]);
    args.rval().setUndefined();
    return true;
  }

  bool pull(JSContext* cx, unsigned argc, Value* vp) {
    CallArgs args = CallArgsFromVp(argc, vp);
    RootedObject self(cx, &args.thisv().toObject());
    RootedObject controller(cx, &args[0].toObject());
    MOZ_RELEASE_ASSERT(controller == BodyStreamSource::controller(self));
    MOZ_RELEASE_ASSERT(get_controller_source(controller) == self.get());

    // The actual read from the body needs to be delayed, because it'd otherwise
    // be a blocking operation in case the backend didn't yet send any data.
    // That would lead to situations where we block on I/O before processing
    // all pending Promises, which in turn can result in operations happening in
    // observably different behavior, up to and including causing deadlocks
    // because a body read response is blocked on content making another request.
    //
    // (This deadlock happens in automated tests, but admittedly might not happen
    // in real usage.)

    if (!pending_body_reads->append(self))
      return false;

    args.rval().setUndefined();
    return true;
  }

  const JSFunctionSpec methods[] = {
    JS_FN("start", start, 1, 0),
    JS_FN("pull", pull, 1, 0),
  JS_FS_END};

  const JSPropertySpec properties[] = {JS_PS_END};

  CLASS_BOILERPLATE_NO_CTOR(BodyStreamSource)

  JSObject* create(JSContext* cx, HandleObject owner) {
    RootedObject source(cx, JS_NewObjectWithGivenProto(cx, &class_, proto_obj));
    if (!source) return nullptr;

    JS::SetReservedSlot(source, Slots::Owner, JS::ObjectValue(*owner));
    return source;
  }
}

namespace CacheOverride {

  // The values stored in these slots are ultimately passed to the host
  // via the xqd_req_cache_override_v2_set hostcall.
  //
  // If `Mode` is not `Override`, all other values are ignored.
  //
  // If `Mode` is `Override`, the values are interpreted in the following way:
  //
  // If `TTL`, `SWR`, or `SurrogateKey` are `undefined`, they're ignored.
  // For each of them, if the value isn't `undefined`, a flag gets set in the
  // hostcall's `tag` parameter, and the value itself is encoded as a uint32
  // parameter.
  //
  // `PCI` is interpreted as a boolean, and a flag gets set in the hostcall's
  // `tag` parameter if `PCI` is true.
  namespace Slots { enum {
    Mode,
    TTL,
    SWR,
    SurrogateKey,
    PCI,
    Count
  };};

  enum class Mode {
    None,
    Pass,
    Override
  };

  // These values are defined by the Fastly ABI:
  // https://docs.rs/fastly-shared/0.6.1/src/fastly_shared/lib.rs.html#407-412
  enum class CacheOverrideTag {
    None = 0,
    Pass = 1 << 0,
    TTL = 1 << 1,
    SWR = 1 << 2,
    PCI = 1 << 3,
  };

  Mode mode(JSObject* self) {
    return (Mode)JS::GetReservedSlot(self, Slots::Mode).toInt32();
  }

  void set_mode(JSObject* self, Mode mode) {
    JS::SetReservedSlot(self, Slots::Mode, JS::Int32Value((int32_t)mode));
  }

  JS::Value ttl(JSObject* self) {
    if (mode(self) != Mode::Override)
      return JS::UndefinedValue();
    return JS::GetReservedSlot(self, Slots::TTL);
  }

  void set_ttl(JSObject* self, uint32_t ttl) {
    MOZ_RELEASE_ASSERT(mode(self) == Mode::Override);
    JS::SetReservedSlot(self, Slots::TTL, JS::Int32Value((int32_t)ttl));
  }

  JS::Value swr(JSObject* self) {
    if (mode(self) != Mode::Override)
      return JS::UndefinedValue();
    return JS::GetReservedSlot(self, Slots::SWR);
  }

  void set_swr(JSObject* self, uint32_t swr) {
    MOZ_RELEASE_ASSERT(mode(self) == Mode::Override);
    JS::SetReservedSlot(self, Slots::SWR, JS::Int32Value((int32_t)swr));
  }

  JS::Value surrogate_key(JSObject* self) {
    if (mode(self) != Mode::Override)
      return JS::UndefinedValue();
    return JS::GetReservedSlot(self, Slots::SurrogateKey);
  }

  void set_surrogate_key(JSObject* self, JSString* key) {
    MOZ_RELEASE_ASSERT(mode(self) == Mode::Override);
    JS::SetReservedSlot(self, Slots::SurrogateKey, JS::StringValue(key));
  }

  JS::Value pci(JSObject* self) {
    if (mode(self) != Mode::Override)
      return JS::UndefinedValue();
    return JS::GetReservedSlot(self, Slots::PCI);
  }

  void set_pci(JSObject* self, bool pci) {
    MOZ_RELEASE_ASSERT(mode(self) == Mode::Override);
    JS::SetReservedSlot(self, Slots::PCI, JS::BooleanValue(pci));
  }

  uint32_t abi_tag(JSObject* self) {
    switch (mode(self)) {
      case Mode::None: return (uint32_t)CacheOverrideTag::None;
      case Mode::Pass: return (uint32_t)CacheOverrideTag::Pass;
      default:;
    }

    uint32_t tag = 0;
    if (!ttl(self).isUndefined())
      tag |= (uint32_t)CacheOverrideTag::TTL;
    if (!swr(self).isUndefined())
      tag |= (uint32_t)CacheOverrideTag::SWR;
    if (!pci(self).isUndefined())
      tag |= (uint32_t)CacheOverrideTag::PCI;

    return tag;
  }

  bool check_receiver(JSContext* cx, HandleObject self, const char* method_name);

  bool mode_get(JSContext* cx, HandleObject self, MutableHandleValue rval) {
    const char* mode_chars;
    switch (mode(self)) {
      case Mode::None:
        mode_chars = "none"; break;
      case Mode::Pass:
        mode_chars = "pass"; break;
      case Mode::Override:
        mode_chars = "override"; break;
    }

    RootedString mode_str(cx, JS_NewStringCopyZ(cx, mode_chars));
    if (!mode_str) return false;

    rval.setString(mode_str);
    return true;
  }

  bool ensure_override(JSContext* cx, HandleObject self, const char* field) {
    if (mode(self) == Mode::Override)
      return true;

    JS_ReportErrorUTF8(cx, "Can't set %s on CacheOverride object whose mode "
                           "isn't \"override\"", field);
    return false;
  }

  bool mode_set(JSContext* cx, HandleObject self, HandleValue val, MutableHandleValue rval) {
    size_t mode_len;
    UniqueChars mode_chars = encode(cx, val, &mode_len);
    if (!mode_chars) return false;

    Mode mode;
    if (!strcmp(mode_chars.get(), "none")) {
      mode = Mode::None;
    } else if (!strcmp(mode_chars.get(), "pass")) {
      mode = Mode::Pass;
    } else if (!strcmp(mode_chars.get(), "override")) {
      mode = Mode::Override;
    } else {
      JS_ReportErrorUTF8(cx, "'mode' has to be \"none\", \"pass\", or \"override\", "
                             "but got %s", mode_chars.get());
      return false;
    }

    set_mode(self, mode);
    return true;
  }

  bool ttl_get(JSContext* cx, HandleObject self, MutableHandleValue rval) {
    rval.set(ttl(self));
    return true;
  }

  bool ttl_set(JSContext* cx, HandleObject self, HandleValue val, MutableHandleValue rval) {
    if (!ensure_override(cx, self, "a TTL"))
      return false;

    if (val.isUndefined()) {
      JS::SetReservedSlot(self, Slots::TTL, val);
    } else {
      int32_t ttl;
      if (!JS::ToInt32(cx, val, &ttl))
        return false;

      set_ttl(self, ttl);
    }
    rval.set(CacheOverride::ttl(self));
    return true;
  }

  bool swr_get(JSContext* cx, HandleObject self, MutableHandleValue rval) {
    rval.set(swr(self));
    return true;
  }

  bool swr_set(JSContext* cx, HandleObject self, HandleValue val, MutableHandleValue rval) {
    if (!ensure_override(cx, self, "SWR"))
      return false;

    if (val.isUndefined()) {
      JS::SetReservedSlot(self, Slots::SWR, val);
    } else {
      int32_t swr;
      if (!JS::ToInt32(cx, val, &swr))
        return false;

      set_swr(self, swr);
    }
    rval.set(CacheOverride::swr(self));
    return true;
  }

  bool surrogate_key_get(JSContext* cx, HandleObject self, MutableHandleValue rval) {
    rval.set(surrogate_key(self));
    return true;
  }

  bool surrogate_key_set(JSContext* cx, HandleObject self, HandleValue val, MutableHandleValue rval) {
    if (!ensure_override(cx, self, "a surrogate key"))
      return false;

    if (val.isUndefined()) {
      JS::SetReservedSlot(self, Slots::SurrogateKey, val);
    } else {
      RootedString surrogate_key(cx, JS::ToString(cx, val));
      if (!surrogate_key) return false;

      set_surrogate_key(self, surrogate_key);
    }
    rval.set(CacheOverride::surrogate_key(self));
    return true;
  }

  bool pci_get(JSContext* cx, HandleObject self, MutableHandleValue rval) {
    rval.set(pci(self));
    return true;
  }

  bool pci_set(JSContext* cx, HandleObject self, HandleValue val, MutableHandleValue rval) {
    if (!ensure_override(cx, self, "PCI"))
      return false;

    if (val.isUndefined()) {
      JS::SetReservedSlot(self, Slots::PCI, val);
    } else {
      bool pci = JS::ToBoolean(val);
      set_pci(self, pci);
    }
    rval.set(CacheOverride::pci(self));
    return true;
  }

  template<auto accessor_fn>
  bool accessor_get(JSContext* cx, unsigned argc, Value* vp) {
    METHOD_HEADER(0)
    return accessor_fn(cx, self, args.rval());
  }

  template<auto accessor_fn>
  bool accessor_set(JSContext* cx, unsigned argc, Value* vp) {
    METHOD_HEADER(1)
    return accessor_fn(cx, self, args[0], args.rval());
  }

  const unsigned ctor_length = 1;

  JSObject* create(JSContext* cx);

  bool constructor(JSContext* cx, unsigned argc, Value* vp) {
    CallArgs args = CallArgsFromVp(argc, vp);
    if (!args.requireAtLeast(cx, "CacheOverride", 1))
      return false;

    RootedObject self(cx, create(cx));
    if (!self) return false;

    RootedValue val(cx);
    if (!mode_set(cx, self, args[0], &val))
      return false;

    if (mode(self) == Mode::Override) {
      if (!args.get(1).isObject()) {
        JS_ReportErrorUTF8(cx, "Creating a CacheOverride object with mode \"override\" requires "
                               "an init object for the override parameters as the second argument");
        return false;
      }

      RootedObject override_init(cx, &args[1].toObject());

      if (!JS_GetProperty(cx, override_init, "ttl", &val) ||
          !ttl_set(cx, self, val, &val))
      {
        return false;
      }

      if (!JS_GetProperty(cx, override_init, "swr", &val) ||
          !swr_set(cx, self, val, &val))
      {
        return false;
      }

      if (!JS_GetProperty(cx, override_init, "surrogateKey", &val) ||
          !surrogate_key_set(cx, self, val, &val))
      {
        return false;
      }

      if (!JS_GetProperty(cx, override_init, "pci", &val) ||
          !pci_set(cx, self, val, &val))
      {
        return false;
      }
    }

    args.rval().setObject(*self);
    return true;
  }

  const JSFunctionSpec methods[] = {JS_FS_END};

  const JSPropertySpec properties[] = {
    JS_PSGS("mode", accessor_get<mode_get>, accessor_set<mode_set>, JSPROP_ENUMERATE),
    JS_PSGS("ttl", accessor_get<ttl_get>, accessor_set<ttl_set>, JSPROP_ENUMERATE),
    JS_PSGS("swr", accessor_get<swr_get>, accessor_set<swr_set>, JSPROP_ENUMERATE),
    JS_PSGS("surrogateKey", accessor_get<surrogate_key_get>, accessor_set<surrogate_key_set>, JSPROP_ENUMERATE),
    JS_PSGS("pci", accessor_get<pci_get>, accessor_set<pci_set>, JSPROP_ENUMERATE),
  JS_PS_END};

  CLASS_BOILERPLATE(CacheOverride)

  JSObject* create(JSContext* cx) {
    return JS_NewObjectWithGivenProto(cx, &class_, proto_obj);
  }
}


// https://fetch.spec.whatwg.org/#concept-method-normalize
// Returns `true` if the method name was normalized, `false` otherwise.
static bool normalize_http_method(char* method) {
  static const char* names[6] = { "DELETE", "GET", "HEAD", "OPTIONS", "POST", "PUT" };

  for (size_t i = 0; i < 6; i++) {
    auto name = names[i];
    if (strcasecmp(method, name) == 0) {
      if (strcmp(method, name) == 0) {
        return false;
      }

      // Note: Safe because `strcasecmp` returning 0 above guarantees same-length strings.
      strcpy(method, name);
      return true;
    }
  }

  return false;
}

namespace Request {
  namespace Slots { enum {
    Request = RequestOrResponse::Slots::RequestOrResponse,
    Body = RequestOrResponse::Slots::Body,
    BodyStream = RequestOrResponse::Slots::BodyStream,
    HasBody = RequestOrResponse::Slots::HasBody,
    BodyUsed = RequestOrResponse::Slots::BodyUsed,
    Headers = RequestOrResponse::Slots::Headers,
    URL = RequestOrResponse::Slots::URL,
    Backend = RequestOrResponse::Slots::Count,
    Method,
    PendingRequest,
    ResponsePromise,
    IsDownstream,
    Count
  };};

  RequestHandle request_handle(JSObject* obj) {
    return RequestHandle { static_cast<uint32_t>(JS::GetReservedSlot(obj, Slots::Request).toInt32()) };
  }

  PendingRequestHandle pending_handle(JSObject* obj) {
    Value handle_val = JS::GetReservedSlot(obj, Slots::PendingRequest);
    if (handle_val.isInt32())
      return PendingRequestHandle { static_cast<uint32_t>(handle_val.toInt32()) };
    return PendingRequestHandle { INVALID_HANDLE };
  }

  bool is_pending(JSObject* obj) {
    return pending_handle(obj).handle != INVALID_HANDLE;
  }

  bool is_downstream(JSObject* obj) {
    return JS::GetReservedSlot(obj, Slots::IsDownstream).toBoolean();
  }

  JSObject* response_promise(JSObject* obj) {
    return &JS::GetReservedSlot(obj, Slots::ResponsePromise).toObject();
  }

  JSString* backend(JSObject* obj) {
    Value val = JS::GetReservedSlot(obj, Slots::Backend);
    return val.isString() ? val.toString() : nullptr;
  }

  // TODO: eagerly retrieve method and store client-side.
  JSString* method(JSContext*cx, HandleObject obj) {
    Value val = JS::GetReservedSlot(obj, Slots::Method);
    if (val.isNullOrUndefined()) {
      char buf[16];
      size_t num_written = 0;
      if (!HANDLE_RESULT(cx, xqd_req_method_get(request_handle(obj), buf, 16, &num_written)))
        return nullptr;

      RootedString method(cx, JS_NewStringCopyN(cx, buf, num_written));
      if (!method) return nullptr;

      val = JS::StringValue(method);
      JS_SetReservedSlot(obj, Slots::Method, val);
    }
    return val.toString();
  }

  bool set_cache_override(JSContext* cx, HandleObject self, HandleValue cache_override_val) {
    if (!CacheOverride::is_instance(cache_override_val)) {
      JS_ReportErrorUTF8(cx, "Value passed in as cacheOverride must be an "
                             "instance of CacheOverride");
      return false;
    }

    RootedObject cache_override(cx, &cache_override_val.toObject());
    RootedValue val(cx);

    uint32_t tag = CacheOverride::abi_tag(cache_override);
    val = CacheOverride::ttl(cache_override);
    uint32_t ttl = val.isUndefined() ? 0 : val.toInt32();
    val = CacheOverride::swr(cache_override);
    uint32_t swr = val.isUndefined() ? 0 : val.toInt32();
    val = CacheOverride::surrogate_key(cache_override);
    UniqueChars sk_chars;
    size_t sk_len = 0;
    if (!val.isUndefined()) {
      sk_chars = encode(cx, val, &sk_len);
      if (!sk_chars) return false;
    }

    return HANDLE_RESULT(cx, xqd_req_cache_override_v2_set(request_handle(self), tag, ttl, swr,
                                                           sk_chars.get(), sk_len));
  }

  JSObject* create(JSContext* cx, HandleValue input, HandleValue init);

  bool constructor(JSContext* cx, unsigned argc, Value* vp) {
    CallArgs args = CallArgsFromVp(argc, vp);
    if (!args.requireAtLeast(cx, "Request", 1))
      return false;

    RootedObject request(cx, create(cx, args[0], args.get(1)));
    if (!request) return false;

    args.rval().setObject(*request);
    return true;
  }

  const unsigned ctor_length = 1;

  bool check_receiver(JSContext* cx, HandleObject self, const char* method_name);

  bool method_get(JSContext* cx, unsigned argc, Value* vp) {
    METHOD_HEADER(0)

    JSString* method = Request::method(cx, self);
    if (!method)
      return false;

    args.rval().setString(method);
    return true;
  }

  bool url_get(JSContext* cx, unsigned argc, Value* vp) {
    METHOD_HEADER(0)

    args.rval().set(RequestOrResponse::url(self));
    return true;
  }

  bool version_get(JSContext* cx, unsigned argc, Value* vp) {
    METHOD_HEADER(0)

    uint32_t version = 0;
    if (!HANDLE_RESULT(cx, xqd_req_version_get(request_handle(self), &version)))
      return false;

    args.rval().setInt32(version);
    return true;
  }

  bool headers_get(JSContext* cx, unsigned argc, Value* vp) {
    METHOD_HEADER(0)

    JSObject* headers = RequestOrResponse::headers<Headers::Mode::ProxyToRequest>(cx, self);
    if (!headers)
      return false;

    args.rval().setObject(*headers);
    return true;
  }

  template<BodyReadResult result_type>
  bool bodyAll(JSContext* cx, unsigned argc, Value* vp) {
    METHOD_HEADER(0)
    return RequestOrResponse::bodyAll<result_type>(cx, args, self);
  }

  bool body_get(JSContext* cx, unsigned argc, Value* vp) {
    METHOD_HEADER(0)
    return RequestOrResponse::body_get(cx, args, self, is_downstream(self));
  }

  bool bodyUsed_get(JSContext* cx, unsigned argc, Value* vp) {
    METHOD_HEADER(0)
    args.rval().setBoolean(RequestOrResponse::body_used(self));
    return true;
  }

  bool setCacheOverride(JSContext* cx, unsigned argc, Value* vp) {
    METHOD_HEADER(1)

    if (!set_cache_override(cx, self, args[0]))
      return false;

    args.rval().setUndefined();
    return true;
  }

  const JSFunctionSpec methods[] = {
    JS_FN("arrayBuffer", bodyAll<BodyReadResult::ArrayBuffer>, 0, 0),
    JS_FN("json", bodyAll<BodyReadResult::JSON>, 0, 0),
    JS_FN("text", bodyAll<BodyReadResult::Text>, 0, 0),
    JS_FN("setCacheOverride", setCacheOverride, 3, 0),
  JS_FS_END};

  const JSPropertySpec properties[] = {
    JS_PSG("method", method_get, 0),
    JS_PSG("url", url_get, 0),
    JS_PSG("version", version_get, 0),
    JS_PSG("headers", headers_get, 0),
    JS_PSG("body", body_get, 0),
    JS_PSG("bodyUsed", bodyUsed_get, 0),
  JS_PS_END};

  CLASS_BOILERPLATE(Request)

  JSObject* create(JSContext* cx, RequestHandle request_handle, BodyHandle body_handle, bool is_downstream) {
    RootedObject request(cx, JS_NewObjectWithGivenProto(cx, &class_, proto_obj));
    if (!request) return nullptr;

    JS::SetReservedSlot(request, Slots::Request, JS::Int32Value(request_handle.handle));
    JS::SetReservedSlot(request, Slots::Body, JS::Int32Value(body_handle.handle));
    // TODO: properly set HasBody based on whether we actually do have a body.
    JS::SetReservedSlot(request, Slots::HasBody, JS::BooleanValue(true));
    JS::SetReservedSlot(request, Slots::BodyUsed, JS::BooleanValue(false));
    JS::SetReservedSlot(request, Slots::IsDownstream, JS::BooleanValue(is_downstream));

    return request;
  }

  JSObject* create(JSContext* cx, HandleValue input, HandleValue init_val) {
    RequestHandle request_handle = { INVALID_HANDLE };
    BodyHandle body_handle = { INVALID_HANDLE };
    RootedString url_str(cx);

    // TODO: clone the handles if needed, e.g. if the new request is sent
    // multiple times, or its headers are changed, etc.
    // This is costly, as it requires copying all the headers, etc, and
    // probably also the body, so we should only do it if really necessary.
    if (input.isObject() && is_instance(input)) {
      RootedObject input_request(cx, &input.toObject());
      request_handle = Request::request_handle(input_request);
      body_handle = RequestOrResponse::body_handle(input_request);
      url_str = RequestOrResponse::url(input_request).toString();
      if (is_downstream(input_request)) {
        RootedObject headers(cx);
        headers = RequestOrResponse::headers<Headers::Mode::ProxyToRequest>(cx, input_request);
        if (!Headers::delazify(cx, headers))
          return nullptr;
      }
    } else {
      url_str = JS::ToString(cx, input);
      if (!url_str) return nullptr;

      size_t url_len;
      UniqueChars url = encode(cx, url_str, &url_len);
      if (!url) return nullptr;

      if (!(HANDLE_RESULT(cx, xqd_req_new(&request_handle)) &&
            HANDLE_RESULT(cx, xqd_req_uri_set(request_handle, url.get(), url_len)) &&
            HANDLE_RESULT(cx, xqd_body_new(&body_handle))))
      {
        return nullptr;
      }
    }

    RootedObject request(cx, create(cx, request_handle, body_handle, false));
    if (!request) return nullptr;

    RequestOrResponse::set_url(request, StringValue(url_str));

    // TODO: apply cache and referrer from init object.

    if (init_val.isObject()) {
      RootedObject init(cx, &init_val.toObject());

      RootedValue method_val(cx);
      if (!JS_GetProperty(cx, init, "method", &method_val))
        return nullptr;

      UniqueChars method;
      if (!method_val.isUndefined()) {
        RootedString method_str(cx, JS::ToString(cx, method_val));
        if (!method_str) return nullptr;

        size_t method_len;
        method = encode(cx, method_str, &method_len);
        if (!method) return nullptr;

        if (normalize_http_method(method.get())) {
          // Replace the JS string with the normalized name.
          method_str = JS_NewStringCopyN(cx, method.get(), method_len);
          if (!method_str) return nullptr;
        }

        if (!HANDLE_RESULT(cx, xqd_req_method_set(request_handle, method.get(), method_len)))
          return nullptr;
        JS::SetReservedSlot(request, Slots::Method, JS::StringValue(method_str));
      }

      RootedValue body_val(cx);
      if (!JS_GetProperty(cx, init, "body", &body_val))
        return nullptr;

      if (!body_val.isNullOrUndefined()) {
        // TODO: throw if `method` is "GET" or "HEAD"
        if (!RequestOrResponse::set_body(cx, request, body_val))
          return nullptr;
      }

      RootedValue headers_val(cx);
      if (!JS_GetProperty(cx, init, "headers", &headers_val))
        return nullptr;

      if (!headers_val.isUndefined()) {
        RootedObject headers(cx, Headers::create(cx, Headers::Mode::ProxyToRequest,
                                                 request, headers_val));
        if (!headers) return nullptr;
        JS::SetReservedSlot(request, Slots::Headers, JS::ObjectValue(*headers));
      }

      RootedValue backend_val(cx);
      if (!JS_GetProperty(cx, init, "backend", &backend_val))
        return nullptr;
      if (!backend_val.isUndefined()) {
        RootedString backend(cx, JS::ToString(cx, backend_val));
        if (!backend) return nullptr;
        JS::SetReservedSlot(request, Slots::Backend, JS::StringValue(backend));
      }

      RootedValue cache_override(cx);
      if (!JS_GetProperty(cx, init, "cacheOverride", &cache_override))
        return nullptr;
      if (!cache_override.isUndefined() && !set_cache_override(cx, request, cache_override))
          return nullptr;
    }

    return request;
  }
}


namespace Response {
  namespace Slots { enum {
    Response = RequestOrResponse::Slots::RequestOrResponse,
    Body = RequestOrResponse::Slots::Body,
    BodyStream = RequestOrResponse::Slots::BodyStream,
    HasBody = RequestOrResponse::Slots::HasBody,
    BodyUsed = RequestOrResponse::Slots::BodyUsed,
    Headers = RequestOrResponse::Slots::Headers,
    IsUpstream = RequestOrResponse::Slots::Count,
    Status,
    Count
  };};

  // Needed for uniform access to Request and Response slots.
  static_assert((int)Slots::Body == (int)Request::Slots::Body);
  static_assert((int)Slots::BodyStream == (int)Request::Slots::BodyStream);
  static_assert((int)Slots::HasBody == (int)Request::Slots::HasBody);
  static_assert((int)Slots::BodyUsed == (int)Request::Slots::BodyUsed);
  static_assert((int)Slots::Headers == (int)Request::Slots::Headers);
  static_assert((int)Slots::Response == (int)Request::Slots::Request);

  ResponseHandle response_handle(JSObject* obj) {
    return ResponseHandle { (uint32_t)(JS::GetReservedSlot(obj, Slots::Response).toInt32()) };
  }

  bool is_upstream(JSObject* obj) {
    return JS::GetReservedSlot(obj, Slots::IsUpstream).toBoolean();
  }

  uint16_t status(JSObject* obj) {
    return (uint16_t)JS::GetReservedSlot(obj, Slots::Status).toInt32();
  }

  JSObject* create(JSContext* cx, ResponseHandle response_handle, BodyHandle body_handle,
                   bool is_upstream);

  // TODO: consider not creating a host-side representation for responses eagerly.
  // Some applications create Response objects purely for internal use, e.g. to represent
  // cache entries. While that's perhaps not ideal to begin with, it exists, so we should
  // handle it in a good way, and not be superfluously slow.
  bool constructor(JSContext* cx, unsigned argc, Value* vp) {
    CallArgs args = CallArgsFromVp(argc, vp);

    // TODO: enable creating Response objects during the init phase, and only
    // creating the host-side representation when processing requests.
    ResponseHandle response_handle = { .handle = INVALID_HANDLE };
    if (!HANDLE_RESULT(cx, xqd_resp_new(&response_handle)))
      return false;

    BodyHandle body_handle = { .handle = INVALID_HANDLE };
    if (!HANDLE_RESULT(cx, xqd_body_new(&body_handle)))
      return false;

    RootedObject response(cx, create(cx, response_handle, body_handle, false));
    if (!response) return false;

    if (!RequestOrResponse::set_body(cx, response, args.get(0)))
      return false;

    RequestOrResponse::set_url(response, JS::StringValue(JS_GetEmptyString(cx)));

    if (args.get(1).isObject()) {
      RootedObject init(cx, &args[1].toObject());

      RootedValue status_val(cx);
      if (!JS_GetProperty(cx, init, "status", &status_val))
        return false;

      uint16_t status = 200;
      if (!status_val.isUndefined() && !JS::ToUint16(cx, status_val, &status))
        return false;

      if (!HANDLE_RESULT(cx, xqd_resp_status_set(response_handle, status)))
        return false;

      RootedValue headers_val(cx);
      if (!JS_GetProperty(cx, init, "headers", &headers_val))
        return false;

      if (!headers_val.isUndefined()) {
        RootedObject headers(cx, Headers::create(cx, Headers::Mode::ProxyToResponse,
                                                 response, headers_val));
        if (!headers) return false;

        JS::SetReservedSlot(response, Slots::Headers, JS::ObjectValue(*headers));
      }
    }

    // To ensure that we really have the same status value as the host,
    // we always read it back here.
    uint16_t status = 0;
    if (!HANDLE_RESULT(cx, xqd_resp_status_get(response_handle, &status)))
      return false;

    JS::SetReservedSlot(response, Slots::Status, JS::Int32Value(status));
    args.rval().setObject(*response);
    return true;
  }

  const unsigned ctor_length = 1;

  bool check_receiver(JSContext* cx, HandleObject self, const char* method_name);

  bool ok_get(JSContext* cx, unsigned argc, Value* vp) {
    METHOD_HEADER(0)

    uint16_t status = Response::status(self);
    args.rval().setBoolean(status >= 200 && status < 300);
    return true;
  }

  bool status_get(JSContext* cx, unsigned argc, Value* vp) {
    METHOD_HEADER(0)

    uint16_t status = Response::status(self);
    args.rval().setInt32(status);
    return true;
  }

  bool url_get(JSContext* cx, unsigned argc, Value* vp) {
    METHOD_HEADER(0)

    args.rval().set(RequestOrResponse::url(self));
    return true;
  }

  // TODO: store version client-side.
  bool version_get(JSContext* cx, unsigned argc, Value* vp) {
    METHOD_HEADER(0)

    uint32_t version = 0;
    if (!HANDLE_RESULT(cx, xqd_resp_version_get(response_handle(self), &version)))
      return false;

    args.rval().setInt32(version);
    return true;
  }

  bool headers_get(JSContext* cx, unsigned argc, Value* vp) {
    METHOD_HEADER(0)

    JSObject* headers = RequestOrResponse::headers<Headers::Mode::ProxyToResponse>(cx, self);
    if (!headers)
      return false;

    args.rval().setObject(*headers);
    return true;
  }

  template<BodyReadResult result_type>
  bool bodyAll(JSContext* cx, unsigned argc, Value* vp) {
    METHOD_HEADER(0)
    return RequestOrResponse::bodyAll<result_type>(cx, args, self);
  }

  bool body_get(JSContext* cx, unsigned argc, Value* vp) {
    METHOD_HEADER(0)
    return RequestOrResponse::body_get(cx, args, self, true);
  }

  bool bodyUsed_get(JSContext* cx, unsigned argc, Value* vp) {
    METHOD_HEADER(0)
    args.rval().setBoolean(RequestOrResponse::body_used(self));
    return true;
  }

  const JSFunctionSpec methods[] = {
    JS_FN("arrayBuffer", bodyAll<BodyReadResult::ArrayBuffer>, 0, 0),
    JS_FN("json", bodyAll<BodyReadResult::JSON>, 0, 0),
    JS_FN("text", bodyAll<BodyReadResult::Text>, 0, 0),
  JS_FS_END};

  const JSPropertySpec properties[] = {
    JS_PSG("ok", ok_get, 0),
    JS_PSG("status", status_get, 0),
    JS_PSG("version", version_get, 0),
    JS_PSG("headers", headers_get, 0),
    JS_PSG("body", body_get, 0),
    JS_PSG("bodyUsed", bodyUsed_get, 0),
    JS_PSG("url", url_get, 0),
  JS_PS_END};

  CLASS_BOILERPLATE(Response)

  JSObject* create(JSContext* cx, ResponseHandle response_handle, BodyHandle body_handle,
                   bool is_upstream)
  {
    RootedObject response(cx, JS_NewObjectWithGivenProto(cx, &class_, proto_obj));
    if (!response) return nullptr;

    JS::SetReservedSlot(response, Slots::Response, JS::Int32Value(response_handle.handle));
    JS::SetReservedSlot(response, Slots::Body, JS::Int32Value(body_handle.handle));
    // TODO: only set HasBody for upstream responses if we actually do have a body.
    JS::SetReservedSlot(response, Slots::HasBody, JS::BooleanValue(is_upstream));
    JS::SetReservedSlot(response, Slots::BodyUsed, JS::BooleanValue(false));
    JS::SetReservedSlot(response, Slots::IsUpstream, JS::BooleanValue(is_upstream));

    if (is_upstream) {
      uint16_t status = 0;
      if (!HANDLE_RESULT(cx, xqd_resp_status_get(response_handle, &status)))
        return nullptr;

      JS::SetReservedSlot(response, Slots::Status, JS::Int32Value(status));
    }

    return response;
  }
}


namespace Dictionary {
  namespace Slots { enum {
    Dictionary,
    Count
  };};

  DictionaryHandle dictionary_handle(JSObject* obj) {
    JS::Value val = JS::GetReservedSlot(obj, Slots::Dictionary);
    return DictionaryHandle { static_cast<uint32_t>(val.toInt32()) };
  }

  JSObject* create(JSContext* cx, const char* name, size_t name_len);

  bool constructor(JSContext* cx, unsigned argc, Value* vp) {
    CallArgs args = CallArgsFromVp(argc, vp);
    if (!args.requireAtLeast(cx, "Dictionary", 1))
      return false;

    size_t name_len;
    UniqueChars name = encode(cx, args[0], &name_len);
    RootedObject dictionary(cx, create(cx, name.get(), name_len));
    if (!dictionary) return false;
    args.rval().setObject(*dictionary);
    return true;
  }

  const unsigned ctor_length = 1;

  bool check_receiver(JSContext* cx, HandleObject self, const char* method_name);

  bool get(JSContext* cx, unsigned argc, Value* vp) {
    METHOD_HEADER(1)

    size_t name_len;
    UniqueChars name = encode(cx, args[0], &name_len);

    OwnedHostCallBuffer buffer;
    size_t nwritten = 0;
    if (!HANDLE_RESULT(cx, xqd_dictionary_get(dictionary_handle(self), name.get(), name_len,
                                              buffer.get(), DICTIONARY_ENTRY_MAX_LEN,
                                              &nwritten)))
    {
      return false;
    }

    RootedString text(cx, JS_NewStringCopyUTF8N(cx, JS::UTF8Chars(buffer.get(), nwritten)));
    if (!text) return false;

    args.rval().setString(text);
    return true;
  }

  const JSFunctionSpec methods[] = {
    JS_FN("get", get, 1, 0),
  JS_FS_END};

  const JSPropertySpec properties[] = {JS_PS_END};

  CLASS_BOILERPLATE(Dictionary)

  JSObject* create(JSContext* cx, const char* name, size_t name_len) {
    RootedObject dict(cx, JS_NewObjectWithGivenProto(cx, &class_, proto_obj));
    if (!dict) return nullptr;
    DictionaryHandle dict_handle = { INVALID_HANDLE };
    if (!HANDLE_RESULT(cx, xqd_dictionary_open(name, name_len, &dict_handle)))
      return nullptr;

    JS::SetReservedSlot(dict, Slots::Dictionary, JS::Int32Value((int)dict_handle.handle));

    return dict;
  }
}

namespace TextEncoder {
  namespace Slots { enum {
    Count
  };};

  JSObject* create(JSContext* cx);

  bool constructor(JSContext* cx, unsigned argc, Value* vp) {
    CallArgs args = CallArgsFromVp(argc, vp);

    RootedObject self(cx, create(cx));
    if (!self) return false;

    args.rval().setObject(*self);
    return true;
  }

  const unsigned ctor_length = 0;

  bool check_receiver(JSContext* cx, HandleObject self, const char* method_name);

  bool encode(JSContext* cx, unsigned argc, Value* vp) {
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

    auto* rawChars = chars.release();
    RootedObject buffer(cx, JS::NewArrayBufferWithContents(cx, chars_len, rawChars));
    if (!buffer) {
      JS_free(cx, rawChars);
      return false;
    }

    RootedObject byte_array(cx, JS_NewUint8ArrayWithBuffer(cx, buffer, 0, chars_len));
    if (!byte_array) return false;

    args.rval().setObject(*byte_array);
    return true;
  }

  bool encoding_get(JSContext* cx, unsigned argc, Value* vp) {
    METHOD_HEADER(0)

    RootedString str(cx, JS_NewStringCopyN(cx, "utf-8", 5));
    if (!str)
      return false;

    args.rval().setString(str);
    return true;
  }

  const JSFunctionSpec methods[] = {
    JS_FN("encode", encode, 1, 0),
  JS_FS_END};

  const JSPropertySpec properties[] = {
    JS_PSG("encoding", encoding_get, 0),
  JS_PS_END};

  CLASS_BOILERPLATE(TextEncoder)

  JSObject* create(JSContext* cx) {
    return JS_NewObjectWithGivenProto(cx, &class_, proto_obj);
  }
}

namespace TextDecoder {
  namespace Slots { enum {
    Count
  };};

  JSObject* create(JSContext* cx);

  bool constructor(JSContext* cx, unsigned argc, Value* vp) {
    CallArgs args = CallArgsFromVp(argc, vp);

    RootedObject self(cx, create(cx));
    if (!self) return false;

    args.rval().setObject(*self);
    return true;
  }

  const unsigned ctor_length = 0;

  bool check_receiver(JSContext* cx, HandleObject self, const char* method_name);

  bool decode(JSContext* cx, unsigned argc, Value* vp) {
    METHOD_HEADER(1)

    // Default to empty string if no input is given.
    if (args.get(0).isUndefined()) {
      args.rval().set(JS_GetEmptyStringValue(cx));
      return true;
    }

    if (!args[0].isObject() || !(JS_IsArrayBufferViewObject(&args[0].toObject()) ||
                                 JS::IsArrayBufferObject(&args[0].toObject())) )
    {
      JS_ReportErrorUTF8(cx, "TextDecoder#decode: input must be of type ArrayBuffer or ArrayBufferView");
      return false;
    }

    RootedObject input(cx, &args[0].toObject());
    size_t length;
    uint8_t* data;
    bool is_shared;

    if (JS_IsArrayBufferViewObject(input)) {
      js::GetArrayBufferViewLengthAndData(input, &length, &is_shared, &data);
    } else {
      JS::GetArrayBufferLengthAndData(input, &length, &is_shared, &data);
    }

    RootedString str(cx, JS_NewStringCopyUTF8N(cx, JS::UTF8Chars((char*)data, length)));
    if (!str)
      return false;

    args.rval().setString(str);
    return true;
  }

  bool encoding_get(JSContext* cx, unsigned argc, Value* vp) {
    METHOD_HEADER(0)

    RootedString str(cx, JS_NewStringCopyN(cx, "utf-8", 5));
    if (!str)
      return false;

    args.rval().setString(str);
    return true;
  }

  const JSFunctionSpec methods[] = {
    JS_FN("decode", decode, 1, 0),
  JS_FS_END};

  const JSPropertySpec properties[] = {
    JS_PSG("encoding", encoding_get, 0),
  JS_PS_END};

  CLASS_BOILERPLATE(TextDecoder)

  JSObject* create(JSContext* cx) {
    return JS_NewObjectWithGivenProto(cx, &class_, proto_obj);
  }
}


bool report_sequence_or_record_arg_error(JSContext* cx, const char* name, const char* alt_text) {
  JS_ReportErrorUTF8(cx, "Failed to construct %s object. If defined, the first "
                          "argument must be either a [ ['name', 'value'], ... ] sequence, "
                          "or a { 'name' : 'value', ... } record%s.", name, alt_text);
  return false;
}
/**
 * Extract <key,value> pairs from the given value if it is either a sequence<sequence<Value>
 * or a record<Value, Value>.
 */
template<auto apply>
bool maybe_consume_sequence_or_record(JSContext* cx, HandleValue initv, HandleObject target,
                                      bool* consumed, const char* ctor_name,
                                      const char* alt_text = "")
{
    if (initv.isUndefined()) {
      *consumed = true;
      return true;
    }

    RootedValue key(cx);
    RootedValue value(cx);

    // First, try consuming args[0] as a sequence<sequence<Value>>.
    JS::ForOfIterator it(cx);
    if (!it.init(initv, JS::ForOfIterator::AllowNonIterable))
      return false;

    // Note: this currently doesn't treat strings as iterable even though they are.
    // We don't have any constructors that want to iterate over strings, and this
    // makes things a lot easier.
    if (initv.isObject() && it.valueIsIterable()) {
      RootedValue entry(cx);

      while (true) {
        bool done;
        if (!it.next(&entry, &done))
          return false;

        if (done)
          break;

        if (!entry.isObject())
          return report_sequence_or_record_arg_error(cx, ctor_name, alt_text);

        JS::ForOfIterator entr_iter(cx);
        if (!entr_iter.init(entry, JS::ForOfIterator::AllowNonIterable))
          return false;

        if (!entr_iter.valueIsIterable())
          return report_sequence_or_record_arg_error(cx, ctor_name, alt_text);

        {
          bool done;

          // Extract key.
          if (!entr_iter.next(&key, &done))
            return false;
          if (done)
            return report_sequence_or_record_arg_error(cx, ctor_name, alt_text);

          // Extract value.
          if (!entr_iter.next(&value, &done))
            return false;
          if (done)
            return report_sequence_or_record_arg_error(cx, ctor_name, alt_text);

          // Ensure that there aren't any further entries.
          if (!entr_iter.next(&entry, &done))
            return false;
          if (!done)
            return report_sequence_or_record_arg_error(cx, ctor_name, alt_text);

          if (!apply(cx, target, key, value, ctor_name))
            return false;
        }
      }
      *consumed = true;
    } else if (initv.isObject()) {
      // init isn't an iterator, so if it's an object, it must be a record to be valid input.
      RootedObject init(cx, &initv.toObject());
      JS::RootedIdVector ids(cx);
      if (!js::GetPropertyKeys(cx, init, JSITER_OWNONLY | JSITER_SYMBOLS, &ids))
        return false;

      JS::RootedId curId(cx);
      for (size_t i = 0; i < ids.length(); ++i) {
        curId = ids[i];
        key = js::IdToValue(curId);

        if (!JS_GetPropertyById(cx, init, curId, &value))
          return false;

        if (!apply(cx, target, key, value, ctor_name))
          return false;
      }
      *consumed = true;
    } else {
      *consumed = false;
    }

    return true;
}
namespace Headers {
  namespace Slots { enum {
    BackingMap,
    Handle,
    Mode,
    HasLazyValues,
    Count
  };};

  namespace detail {
    #define HEADERS_ITERATION_METHOD(argc) \
      METHOD_HEADER(argc) \
      RootedObject backing_map(cx, detail::backing_map(self)); \
      if (!detail::ensure_all_header_values_from_handle(cx, self, backing_map)) \
        return false; \

    static const char VALID_NAME_CHARS[128] = {
      0, 0, 0, 0, 0, 0, 0, 0,  //   0
      0, 0, 0, 0, 0, 0, 0, 0,  //   8
      0, 0, 0, 0, 0, 0, 0, 0,  //  16
      0, 0, 0, 0, 0, 0, 0, 0,  //  24

      0, 1, 0, 1, 1, 1, 1, 1,  //  32
      0, 0, 1, 1, 0, 1, 1, 0,  //  40
      1, 1, 1, 1, 1, 1, 1, 1,  //  48
      1, 1, 0, 0, 0, 0, 0, 0,  //  56

      0, 1, 1, 1, 1, 1, 1, 1,  //  64
      1, 1, 1, 1, 1, 1, 1, 1,  //  72
      1, 1, 1, 1, 1, 1, 1, 1,  //  80
      1, 1, 1, 0, 0, 0, 1, 1,  //  88

      1, 1, 1, 1, 1, 1, 1, 1,  //  96
      1, 1, 1, 1, 1, 1, 1, 1,  // 104
      1, 1, 1, 1, 1, 1, 1, 1,  // 112
      1, 1, 1, 0, 1, 0, 1, 0   // 120
    };

    #define NORMALIZE_NAME(name, fun_name) \
      RootedValue normalized_name(cx, name); \
      size_t name_len; \
      UniqueChars name_chars = detail::normalize_header_name(cx, &normalized_name, &name_len, \
                                                             fun_name); \
      if (!name_chars) return false; \

    #define NORMALIZE_VALUE(value, fun_name) \
      RootedValue normalized_value(cx, value); \
      size_t value_len; \
      UniqueChars value_chars = detail::normalize_header_value(cx, &normalized_value, \
                                                               &value_len, fun_name); \
      if (!value_chars) return false; \

    JSObject* backing_map(JSObject* self) {
      return &JS::GetReservedSlot(self, Slots::BackingMap).toObject();
    }

    Mode mode(JSObject* self) {
      return static_cast<Mode>(JS::GetReservedSlot(self, Slots::Mode).toInt32());
    }

    bool lazy_values(JSObject* self) {
      return JS::GetReservedSlot(self, Slots::HasLazyValues).toBoolean();
    }

    uint32_t handle(JSObject* self) {
      return static_cast<uint32_t>(JS::GetReservedSlot(self, Slots::Handle).toInt32());
    }

    /**
     * Validates and normalizes the given header name, by
     * - checking for invalid characters
     * - converting to lower-case
     *
     * See https://searchfox.org/mozilla-central/rev/9f76a47f4aa935b49754c5608a1c8e72ee358c46/netwerk/protocol/http/nsHttp.cpp#172-215
     * For details on validation.
     *
     * Mutates `name_val` in place, and returns the name as UniqueChars.
     * This is done because most uses of header names require handling of both the JSString
     * and the char* version, so they'd otherwise have to recreate one of the two.
     */
    UniqueChars normalize_header_name(JSContext* cx, MutableHandleValue name_val,
                                      size_t* name_len, const char* fun_name)
    {
      RootedString name_str(cx, JS::ToString(cx, name_val));
      if (!name_str) return nullptr;

      size_t len;
      UniqueChars name = encode(cx, name_str, &len);
      if (!name) return nullptr;

      if (len == 0) {
        JS_ReportErrorASCII(cx, "%s: Header name can't be empty", fun_name);
        return nullptr;
      }

      bool changed = false;

      char* name_chars = name.get();
      for (size_t i = 0; i < len; i++) {
        unsigned char ch = name_chars[i];
        if (ch > 127 || !VALID_NAME_CHARS[ch]) {
          JS_ReportErrorASCII(cx, "%s: Invalid header name '%s'", fun_name, name_chars);
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

    UniqueChars normalize_header_value(JSContext* cx, MutableHandleValue value_val,
                                       size_t* value_len, const char* fun_name)
    {
      RootedString value_str(cx, JS::ToString(cx, value_val));
      if (!value_str) return nullptr;

      size_t len;
      UniqueChars value = encode(cx, value_str, &len);
      if (!value) return nullptr;

      char* value_chars = value.get();
      size_t start = 0;
      size_t end = len;

      // We follow Gecko's interpretation of what's a valid header value. After stripping
      // leading and trailing whitespace, all interior line breaks and `\0` are considered
      // invalid.
      // See https://searchfox.org/mozilla-central/rev/9f76a47f4aa935b49754c5608a1c8e72ee358c46/netwerk/protocol/http/nsHttp.cpp#247-260
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
        if (!value_str) return nullptr;
      }

      value_val.setString(value_str);
      *value_len = len;
      return value;
    }

    static PersistentRooted<JSString*> comma;

    // Append an already normalized value for an already normalized header name
    // to the JS side map, but not the host.
    //
    // Returns the resulting combined value in `value`.
    bool append_header_value_to_map(JSContext* cx, HandleObject self,
                                    HandleValue normalized_name,
                                    MutableHandleValue normalized_value)
    {
      RootedValue existing(cx);
      RootedObject map(cx, backing_map(self));
      if (!JS::MapGet(cx, map, normalized_name, &existing))
        return false;

      // Existing value must only be null if we're in the process if applying
      // header values from a handle.
      if (!existing.isNullOrUndefined()) {
        if (!comma.get()) {
          comma.init(cx, JS_NewStringCopyN(cx, ", ", 2));
          if (!comma) return false;
        }

        RootedString str(cx, existing.toString());
        str = JS_ConcatStrings(cx, str, comma);
        if (!str) return false;

        RootedString val_str(cx, normalized_value.toString());
        str = JS_ConcatStrings(cx, str, val_str);
        if (!str) return false;

        normalized_value.setString(str);
      }

      return JS::MapSet(cx, map, normalized_name, normalized_value);
    }

    bool get_header_names_from_handle(JSContext* cx, uint32_t handle, Mode mode,
                                      HandleObject backing_map)
    {
      RootedString name(cx);
      RootedValue name_val(cx);
      OwnedHostCallBuffer buffer;

      MULTI_VALUE_HOSTCALL({
        int result;
        if (mode == Mode::ProxyToRequest) {
          RequestHandle request = { handle };
          result = xqd_req_header_names_get(request, buffer.get(), HEADER_MAX_LEN,
                                            cursor, &ending_cursor,
                                            &nwritten);
        } else {
          ResponseHandle response = { handle };
          result = xqd_resp_header_names_get(response, buffer.get(), HEADER_MAX_LEN,
                                             cursor, &ending_cursor,
                                             &nwritten);
        }

        if (!HANDLE_RESULT(cx, result))
          return false;
      },
      {
        uint32_t offset = 0;
        for (size_t i = 0; i < nwritten; i++)
        {
          if (buffer.get()[i] != '\0') {
            continue;
          }

          name = JS_NewStringCopyN(cx, buffer.get() + offset, i - offset);
          if (!name) return false;

          name_val.setString(name);
          JS::MapSet(cx, backing_map, name_val, JS::NullHandleValue);

          offset = i + 1;
        }
      })

      return true;
    }

    static bool retrieve_value_for_header_from_handle(JSContext* cx, HandleObject self,
                                                      HandleValue name, MutableHandleValue value)
    {
      Mode mode = detail::mode(self);
      MOZ_ASSERT(mode != Mode::Standalone);
      uint32_t handle = detail::handle(self);

      size_t name_len;
      RootedString name_str(cx, name.toString());
      UniqueChars name_chars = encode(cx, name_str, &name_len);

      RootedString val_str(cx);
      OwnedHostCallBuffer buffer;

      MULTI_VALUE_HOSTCALL({
        int result;
        if (mode == Headers::Mode::ProxyToRequest) {
          RequestHandle request = { handle };
          result = xqd_req_header_values_get(request, name_chars.get(), name_len,
                                             buffer.get(), HEADER_MAX_LEN,
                                             cursor, &ending_cursor, &nwritten);
        } else {
          ResponseHandle response = { handle };
              result = xqd_resp_header_values_get(response, name_chars.get(), name_len,
                                                  buffer.get(), HEADER_MAX_LEN,
                                                  cursor, &ending_cursor, &nwritten);
        }

        if (!HANDLE_RESULT(cx, result))
          return false;
      }, {
        size_t offset = 0;
        for (size_t i = 0; i < nwritten; i++) {
          if (buffer.get()[i] == '\0') {
            val_str = JS_NewStringCopyUTF8N(cx, JS::UTF8Chars(buffer.get() + offset,
                                                              i - offset));
            if (!val_str) return false;

            value.setString(val_str);

            if (!append_header_value_to_map(cx, self, name, value))
              return false;

            offset = i + 1;
          }
        }
      })

      return true;
    }

    /**
     * Ensures that a value for the given header is available to client code.
     *
     * The calling code must ensure that a header with the given name exists, but might not yet
     * have been retrieved from the host, i.e., it might be a "lazy" value.
     *
     * The value is returned via the `values` outparam, but *only* if the Headers object has
     * lazy values at all. This is to avoid the map lookup in those cases where none is
     * necessary in this function, and the consumer wouldn't use the value anyway.
     */
    static bool ensure_value_for_header(JSContext* cx, HandleObject self,
                                        HandleValue normalized_name, MutableHandleValue values)
    {
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

    bool get_header_value_for_name(JSContext* cx, HandleObject self, HandleValue name,
                                   MutableHandleValue rval, const char* fun_name)
    {
      NORMALIZE_NAME(name, fun_name)

      if (!ensure_value_for_header(cx, self, normalized_name, rval))
        return false;

      if (rval.isString())
        return true;

      RootedObject map(cx, detail::backing_map(self));
      return JS::MapGet(cx, map, normalized_name, rval);
    }

    static bool ensure_all_header_values_from_handle(JSContext* cx, HandleObject self,
                                                     HandleObject backing_map)
    {
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

    typedef int AppendHeaderOperation(int handle, const char *name, size_t name_len,
                                      const char *value, size_t value_len);

    // Appends a non-normalized value for a non-normalized header name to both
    // the JS side Map and, in non-standalone mode, the host.
    //
    // Verifies and normalizes the name and value.
    bool append_header_value(JSContext* cx, HandleObject self,
                             HandleValue name, HandleValue value, const char* fun_name)
    {
      NORMALIZE_NAME(name, fun_name)
      NORMALIZE_VALUE(value, fun_name)

      // Ensure that any host-side values have been applied JS-side.
      RootedValue v(cx);
      if (!ensure_value_for_header(cx, self, normalized_name, &v))
        return false;

      Mode mode = detail::mode(self);
      if (mode != Mode::Standalone) {
        AppendHeaderOperation* op;
        if (mode == Mode::ProxyToRequest)
          op = (AppendHeaderOperation*)xqd_req_header_append;
        else
          op = (AppendHeaderOperation*)xqd_resp_header_append;
        if (!HANDLE_RESULT(cx, op(handle(self), name_chars.get(), name_len,
                           value_chars.get(), value_len)))
        {
          return false;
        }
      }

      return append_header_value_to_map(cx, self, normalized_name, &normalized_value);
    }
  }

  bool delazify(JSContext* cx, HandleObject headers) {
    RootedObject backing_map(cx, detail::backing_map(headers));
    return detail::ensure_all_header_values_from_handle(cx, headers, backing_map);
  }

  JSObject* create(JSContext* cx, Mode mode, HandleObject owner, HandleValue initv) {
    RootedObject headers(cx, create(cx, mode, owner));
    if (!headers) return nullptr;

    bool consumed = false;
    if (!maybe_consume_sequence_or_record<detail::append_header_value>(cx, initv, headers,
                                                                       &consumed, "Headers"))
    {
      return nullptr;
    }

    if (!consumed) {
      report_sequence_or_record_arg_error(cx, "Headers", "");
      return nullptr;
    }

    return headers;
  }

  bool constructor(JSContext* cx, unsigned argc, Value* vp) {
    CallArgs args = CallArgsFromVp(argc, vp);
    RootedObject headers(cx, create(cx, Mode::Standalone, nullptr, args.get(0)));
    if (!headers) return false;

    args.rval().setObject(*headers);
    return true;
  }

  const unsigned ctor_length = 1;

  bool check_receiver(JSContext* cx, HandleObject self, const char* method_name);

  bool get(JSContext* cx, unsigned argc, Value* vp) {
    METHOD_HEADER(1)

    NORMALIZE_NAME(args[0], "Headers.get")

    return detail::get_header_value_for_name(cx, self, normalized_name, args.rval(),
                                             "Headers.get");
  }

  typedef int HeaderValuesSetOperation(int handle, const char *name, size_t name_len,
                                       const char *values, size_t values_len);

  bool set(JSContext* cx, unsigned argc, Value* vp) {
    METHOD_HEADER(2)

    NORMALIZE_NAME(args[0], "Headers.set")
    NORMALIZE_VALUE(args[1], "Headers.set")

    Mode mode = detail::mode(self);
    if (mode != Mode::Standalone) {
      HeaderValuesSetOperation* op;
      if (mode == Mode::ProxyToRequest)
        op = (HeaderValuesSetOperation*)xqd_req_header_insert;
      else
        op = (HeaderValuesSetOperation*)xqd_resp_header_insert;
      if (!HANDLE_RESULT(cx, op(detail::handle(self), name_chars.get(), name_len,
                         value_chars.get(), value_len)))
      {
        return false;
      }
    }

    RootedObject map(cx, detail::backing_map(self));
    if (!JS::MapSet(cx, map, normalized_name, normalized_value))
      return false;

    args.rval().setUndefined();
    return true;
  }

  bool has(JSContext* cx, unsigned argc, Value* vp) {
    METHOD_HEADER(1)

    NORMALIZE_NAME(args[0], "Headers.has")
    bool has;
    RootedObject map(cx, detail::backing_map(self));
    if (!JS::MapHas(cx, map, normalized_name, &has))
      return false;
    args.rval().setBoolean(has);
    return true;
  }

  bool append(JSContext* cx, unsigned argc, Value* vp) {
    METHOD_HEADER(2)

    if (!detail::append_header_value(cx, self, args[0], args[1], "Headers.append"))
      return false;

    args.rval().setUndefined();
    return true;
  }

  typedef int HeaderRemoveOperation(int handle, const char *name, size_t name_len);

  bool delete_(JSContext* cx, unsigned argc, Value* vp) {
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
      HeaderRemoveOperation* op;
      if (mode == Mode::ProxyToRequest)
        op = (HeaderRemoveOperation*)xqd_req_header_remove;
      else
        op = (HeaderRemoveOperation*)xqd_resp_header_remove;
      if (!HANDLE_RESULT(cx, op(detail::handle(self), name_chars.get(), name_len)))
        return false;
    }

    args.rval().setUndefined();
    return true;
  }

  bool forEach(JSContext* cx, unsigned argc, Value* vp) {
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

  bool entries(JSContext* cx, unsigned argc, Value* vp) {
    HEADERS_ITERATION_METHOD(0)
    return JS::MapEntries(cx, backing_map, args.rval());
  }

  bool keys(JSContext* cx, unsigned argc, Value* vp) {
    HEADERS_ITERATION_METHOD(0)
    return JS::MapKeys(cx, backing_map, args.rval());
  }

  bool values(JSContext* cx, unsigned argc, Value* vp) {
    HEADERS_ITERATION_METHOD(0)
    return JS::MapValues(cx, backing_map, args.rval());
  }

  const JSFunctionSpec methods[] = {
    JS_FN("get", get, 1, 0),
    JS_FN("has", has, 1, 0),
    JS_FN("set", set, 2, 0),
    JS_FN("append", append, 2, 0),
    JS_FN("delete", delete_, 1, 0),
    JS_FN("forEach", forEach, 1, 0),
    JS_FN("entries", entries, 0, 0),
    JS_FN("keys", keys, 0, 0),
    JS_FN("values", values, 0, 0),
    // [Symbol.iterator] added in init_class.
  JS_FS_END};

  const JSPropertySpec properties[] = {JS_PS_END};

  CLASS_BOILERPLATE_CUSTOM_INIT(Headers)

  bool init_class(JSContext* cx, HandleObject global) {
    bool ok = init_class_impl(cx, global);
    if (!ok) return false;

    RootedValue entries(cx);
    if (!JS_GetProperty(cx, proto_obj, "entries", &entries))
      return false;

    JS::SymbolCode code = JS::SymbolCode::iterator;
    JS::RootedId iteratorId(cx, SYMBOL_TO_JSID(JS::GetWellKnownSymbol(cx, code)));
    return JS_DefinePropertyById(cx, proto_obj, iteratorId, entries, 0);
  }

  JSObject* create(JSContext* cx, Mode mode, HandleObject owner) {
    RootedObject self(cx, JS_NewObjectWithGivenProto(cx, &class_, proto_obj));
    if (!self) return nullptr;

    JS_SetReservedSlot(self, Slots::Mode, JS::Int32Value(static_cast<int32_t>(mode)));
    uint32_t handle = UINT32_MAX - 1;
    if (mode != Mode::Standalone)
      handle = RequestOrResponse::handle(owner);
    JS_SetReservedSlot(self, Slots::Handle, JS::Int32Value(static_cast<int32_t>(handle)));

    RootedObject backing_map(cx, JS::NewMapObject(cx));
    if (!backing_map) return nullptr;
    JS::SetReservedSlot(self, Slots::BackingMap, JS::ObjectValue(*backing_map));

    bool lazy = false;
    if ((mode == Mode::ProxyToRequest && Request::is_downstream(owner)) ||
        (mode == Mode::ProxyToResponse && Response::is_upstream(owner)))
    {
      lazy = true;
      if (!detail::get_header_names_from_handle(cx, handle, mode, backing_map))
        return nullptr;
    }

    JS_SetReservedSlot(self, Slots::HasLazyValues, JS::BooleanValue(lazy));

    return self;
  }
}

static JSString* get_geo_info(JSContext* cx, HandleString address_str) {
  RequestHandle request = { INVALID_HANDLE };
  BodyHandle body = { INVALID_HANDLE };
  const char* url = "http://www.fastly.com/geolocation";

  if (!(HANDLE_RESULT(cx, xqd_req_new(&request)) &&
        HANDLE_RESULT(cx, xqd_body_new(&body)) &&
        HANDLE_RESULT(cx, xqd_req_uri_set(request, url, strlen(url)))))
  {
    return nullptr;
  }

  const char* header_name = "Fastly-XQD-API";
  const char* api_name = "geolocation";
  if (!HANDLE_RESULT(cx, xqd_req_header_append(request, header_name, strlen(header_name),
                                               api_name, strlen(api_name))))
  {
    return nullptr;
  }

  header_name = "Fastly-XQD-arg1";
  size_t address_len;
  UniqueChars address = encode(cx, address_str, &address_len);

  if (!HANDLE_RESULT(cx, xqd_req_header_append(request, header_name, strlen(header_name),
                                               address.get(), address_len)))
  {
    return nullptr;
  }

  ResponseHandle response = { INVALID_HANDLE };
  if (!HANDLE_RESULT(cx, xqd_req_send(request, body, "geo", strlen("geo"), &response, &body)))
    return nullptr;

  size_t bytes_read;
  UniqueChars buf(read_from_handle_all<xqd_body_read, BodyHandle>(cx, body, &bytes_read, false));
  if (!buf)
    return nullptr;

  return JS_NewStringCopyUTF8N(cx, JS::UTF8Chars(buf.get(), bytes_read));
}

namespace ClientInfo {
  namespace Slots { enum {
    Address,
    GeoInfo,
    Count
  };};

  JSString* address(JSObject* obj) {
    JS::Value val = JS::GetReservedSlot(obj, Slots::Address);
    return val.isString() ? val.toString() : nullptr;
  }

  JSString* geo_info(JSObject* obj) {
    JS::Value val = JS::GetReservedSlot(obj, Slots::GeoInfo);
    return val.isString() ? val.toString() : nullptr;
  }

  static JSString* retrieve_address(JSContext* cx, HandleObject self) {
    RootedString address(cx);
    char octets[16];

    size_t nwritten = 0;
    if (!HANDLE_RESULT(cx, xqd_req_downstream_client_ip_addr_get(octets, &nwritten)))
      return nullptr;

    switch (nwritten) {
      case 0: {
        // No address to be had, leave `address` as a nullptr.
        break;
      }
      case 4: {
        char address_chars[INET_ADDRSTRLEN];
        // TODO: do we need to do error handling here, or can we depend on the
        // host giving us a valid address?
        inet_ntop(AF_INET, octets, address_chars, INET_ADDRSTRLEN);
        address = JS_NewStringCopyZ(cx, address_chars);
        if (!address) return nullptr;

        break;
      }
      case 16: {
        char address_chars[INET6_ADDRSTRLEN];
        // TODO: do we need to do error handling here, or can we depend on the
        // host giving us a valid address?
        inet_ntop(AF_INET6, octets, address_chars, INET6_ADDRSTRLEN);
        address = JS_NewStringCopyZ(cx, address_chars);
        if (!address) return nullptr;

        break;
      }
    }

    JS::SetReservedSlot(self, Slots::Address, JS::StringValue(address));
    return address;
  }

  static JSString* retrieve_geo_info(JSContext* cx, HandleObject self) {
    RootedString address_str(cx, address(self));
    if (!address_str) {
      address_str = retrieve_address(cx, self);
      if (!address_str) return nullptr;
    }

    RootedString geo(cx, get_geo_info(cx, address_str));
    if (!geo) return nullptr;

    JS::SetReservedSlot(self, Slots::GeoInfo, JS::StringValue(geo));
    return geo;
  }

  const unsigned ctor_length = 0;

  bool check_receiver(JSContext* cx, HandleObject self, const char* method_name);

  bool address_get(JSContext* cx, unsigned argc, Value* vp) {
    METHOD_HEADER(0)

    RootedString address_str(cx, address(self));
    if (!address_str) {
      address_str = retrieve_address(cx, self);
      if (!address_str) return false;
    }

    args.rval().setString(address_str);
    return true;
  }

  bool geo_get(JSContext* cx, unsigned argc, Value* vp) {
    METHOD_HEADER(0)

    RootedString geo_info_str(cx, geo_info(self));
    if (!geo_info_str) {
      geo_info_str = retrieve_geo_info(cx, self);
      if (!geo_info_str) return false;
    }

    return JS_ParseJSON(cx, geo_info_str, args.rval());
  }

  const JSFunctionSpec methods[] = {
  JS_FS_END};

  const JSPropertySpec properties[] = {
    JS_PSG("address", address_get, 0),
    JS_PSG("geo", geo_get, 0),
  JS_PS_END};

  CLASS_BOILERPLATE_NO_CTOR(ClientInfo)

  JSObject* create(JSContext* cx) {
    return JS_NewObjectWithGivenProto(cx, &class_, proto_obj);
  }
}

namespace ServiceInfo {
  namespace Slots { enum {
    Count
  };};

  static constexpr char ENV_CACHE_GENERATION[] = "FASTLY_CACHE_GENERATION";
  static constexpr char ENV_CUSTOMER_ID[] = "FASTLY_CUSTOMER_ID";
  static constexpr char ENV_HOSTNAME[] = "FASTLY_HOSTNAME";
  static constexpr char ENV_POP[] = "FASTLY_POP";
  static constexpr char ENV_REGION[] = "FASTLY_REGION";
  static constexpr char ENV_SERVICE_ID[] = "FASTLY_SERVICE_ID";
  static constexpr char ENV_SERVICE_VERSION[] = "FASTLY_SERVICE_VERSION";
  static constexpr char ENV_TRACE_ID[] = "FASTLY_TRACE_ID";

  const unsigned ctor_length = 0;

  bool check_receiver(JSContext* cx, HandleObject self, const char* method_name);

  template<const char* var_name>
  bool env_var_get(JSContext* cx, unsigned argc, Value* vp) {
    METHOD_HEADER(0)

    RootedString env_var(cx, JS_NewStringCopyZ(cx, getenv(var_name)));
    if (!env_var) return false;

    args.rval().setString(env_var);
    return true;
  }

  const JSFunctionSpec methods[] = {
  JS_FS_END};

  const JSPropertySpec properties[] = {
    JS_PSG("cacheGeneration", env_var_get<ENV_CACHE_GENERATION>, 0),
    JS_PSG("customerId", env_var_get<ENV_CUSTOMER_ID>, 0),
    JS_PSG("hostname", env_var_get<ENV_HOSTNAME>, 0),
    JS_PSG("pop", env_var_get<ENV_POP>, 0),
    JS_PSG("region", env_var_get<ENV_REGION>, 0),
    JS_PSG("serviceId", env_var_get<ENV_SERVICE_ID>, 0),
    JS_PSG("serviceVersion", env_var_get<ENV_SERVICE_VERSION>, 0),
    JS_PSG("traceId", env_var_get<ENV_TRACE_ID>, 0),
  JS_PS_END};

  CLASS_BOILERPLATE_NO_CTOR(ServiceInfo)

  JSObject* create(JSContext* cx) {
    return JS_NewObjectWithGivenProto(cx, &class_, proto_obj);
  }
}


namespace FetchEvent {
  namespace Slots { enum {
    Dispatch,
    Request,
    State,
    PendingPromiseCount,
    DecPendingPromiseCountFunc,
    ClientInfo,
    ServiceInfo,
    Count
  };};

  namespace detail {
    void inc_pending_promise_count(JSObject* self) {
      MOZ_ASSERT(is_instance(self));
      auto count = JS::GetReservedSlot(self, Slots::PendingPromiseCount).toInt32();
      count++;
      MOZ_ASSERT(count > 0);
      JS::SetReservedSlot(self, Slots::PendingPromiseCount, JS::Int32Value(count));
    }

    void dec_pending_promise_count(JSObject* self) {
      MOZ_ASSERT(is_instance(self));
      auto count = JS::GetReservedSlot(self, Slots::PendingPromiseCount).toInt32();
      MOZ_ASSERT(count > 0);
      count--;
      JS::SetReservedSlot(self, Slots::PendingPromiseCount, JS::Int32Value(count));
    }

    bool add_pending_promise(JSContext* cx, HandleObject self, HandleObject promise) {
      MOZ_ASSERT(is_instance(self));
      MOZ_ASSERT(JS::IsPromiseObject(promise));
      RootedObject handler(cx);
      handler = &JS::GetReservedSlot(self, Slots::DecPendingPromiseCountFunc).toObject();
      if (!JS::AddPromiseReactions(cx, promise, handler, handler))
        return false;

      inc_pending_promise_count(self);
      return true;
    }
  }

  const unsigned ctor_length = 0;

  bool check_receiver(JSContext* cx, HandleObject self, const char* method_name);

  bool client_get(JSContext* cx, unsigned argc, Value* vp) {
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

  bool service_get(JSContext* cx, unsigned argc, Value* vp) {
    METHOD_HEADER(0)
    RootedValue serviceInfo(cx, JS::GetReservedSlot(self, Slots::ServiceInfo));

    if (serviceInfo.isUndefined()) {
      RootedObject obj(cx, ServiceInfo::create(cx));
      if (!obj)
        return false;
      serviceInfo.setObject(*obj);
      JS::SetReservedSlot(self, Slots::ServiceInfo, serviceInfo);
    }

    args.rval().set(serviceInfo);
    return true;
  }

  static JSObject* prepare_downstream_request(JSContext* cx) {
      return Request::create(cx, RequestHandle { INVALID_HANDLE },
                             BodyHandle { INVALID_HANDLE }, true);
  }

  static bool init_downstream_request(JSContext* cx, HandleObject request) {
    RequestHandle request_handle = { INVALID_HANDLE };
    BodyHandle body_handle = { INVALID_HANDLE };
    if (!HANDLE_RESULT(cx, xqd_req_body_downstream_get(&request_handle, &body_handle)))
      return false;

    JS::SetReservedSlot(request, Request::Slots::Request, JS::Int32Value(request_handle.handle));
    JS::SetReservedSlot(request, RequestOrResponse::Slots::Body,
                        JS::Int32Value(body_handle.handle));

    size_t bytes_read;
    UniqueChars buf(read_from_handle_all<xqd_req_uri_get, RequestHandle>(cx, request_handle,
                                                                         &bytes_read, false));
    if (!buf) return false;

    RootedString url(cx, JS_NewStringCopyN(cx, buf.get(), bytes_read));
    if (!url) return false;
    JS::SetReservedSlot(request, Request::Slots::URL, JS::StringValue(url));

    return true;
  }

  bool request_get(JSContext* cx, unsigned argc, Value* vp) {
    METHOD_HEADER(0)

    RootedObject request(cx, &JS::GetReservedSlot(self, Slots::Request).toObject());
    if (Request::request_handle(request).handle == INVALID_HANDLE) {
      if (!init_downstream_request(cx, request))
        return false;
    }

    args.rval().setObject(*request);
    return true;
  }

  bool start_response(JSContext* cx, HandleObject response_obj, bool streaming) {
    ResponseHandle response = Response::response_handle(response_obj);
    BodyHandle body = RequestOrResponse::body_handle(response_obj);

    return HANDLE_RESULT(cx, xqd_resp_send_downstream(response, body, streaming));
  }

  bool respond_blocking(JSContext* cx, HandleObject response_obj) {
    return start_response(cx, response_obj, false);
  }

  bool body_reader_then_handler(JSContext* cx, unsigned argc, Value* vp) {
    CallArgs args = CallArgsFromVp(argc, vp);
    RootedObject then_handler(cx, &args.callee());
    RootedObject catch_handler(cx, &js::GetFunctionNativeReserved(then_handler, 0).toObject());
    RootedObject response(cx, &js::GetFunctionNativeReserved(catch_handler, 0).toObject());
    RootedObject reader(cx, &js::GetFunctionNativeReserved(catch_handler, 1).toObject());
    BodyHandle body_handle = RequestOrResponse::body_handle(response);

    // We're guaranteed to work with a native ReadableStreamDefaultReader here, which in turn is
    // guaranteed to vend {done: bool, value: any} objects to read promise then callbacks.
    RootedObject chunk_obj(cx, &args[0].toObject());
    RootedValue done_val(cx);
    if (!JS_GetProperty(cx, chunk_obj, "done", &done_val))
      return false;

    if (done_val.toBoolean())
      return HANDLE_RESULT(cx, xqd_body_close(body_handle));

    RootedValue val(cx);
    if (!JS_GetProperty(cx, chunk_obj, "value", &val))
      return false;

    if (!val.isObject() || !JS_IsUint8Array(&val.toObject())) {
      // TODO: check if this should create a rejected promise instead, so an in-content handler
      // for unhandled rejections could deal with it.
      // The read operation returned a chunk that's not a Uint8Array.
      fprintf(stderr, "Error: read operation on body ReadableStream didn't respond with a "
                      "Uint8Array. Received value: ");
      dump_value(cx, val, stderr);
      return false;
    }

    {
      JS::AutoCheckCannotGC nogc;
      JSObject* array = &val.toObject();
      bool is_shared;
      uint8_t* bytes = JS_GetUint8ArrayData(array, &is_shared, nogc);
      size_t length = JS_GetTypedArrayByteLength(array);
      size_t nwritten;
      if (!HANDLE_RESULT(cx, xqd_body_write(body_handle, (char*)bytes, length,
                                            BodyWriteEndBack, &nwritten)))
      {
        return false;
      }
    }

    // Read the next chunk.
    RootedObject promise(cx, JS::ReadableStreamDefaultReaderRead(cx, reader));
    if (!promise) return false;
    return JS::AddPromiseReactions(cx, promise, then_handler, catch_handler);
  }

  bool body_reader_catch_handler(JSContext* cx, unsigned argc, Value* vp) {
    CallArgs args = CallArgsFromVp(argc, vp);
    RootedObject response(cx, &js::GetFunctionNativeReserved(&args.callee(), 0).toObject());
    RootedObject reader(cx, &js::GetFunctionNativeReserved(&args.callee(), 1).toObject());

    // TODO: check if this should create a rejected promise instead, so an in-content handler
    // for unhandled rejections could deal with it.
    // The body stream errored during the streaming response.
    // Not much we can do, but at least close the stream, and warn.
    fprintf(stderr, "Warning: body ReadableStream closed during streaming response. Exception: ");
    dump_value(cx, args.get(0), stderr);

    return HANDLE_RESULT(cx, xqd_body_close(RequestOrResponse::body_handle(response)));
  }

  bool respond_streaming(JSContext* cx, HandleObject response_obj) {
    RootedObject stream(cx, RequestOrResponse::body_stream(response_obj));
    MOZ_ASSERT(stream);

    bool locked_or_disturbed;
    if (!JS::ReadableStreamIsLocked(cx, stream, &locked_or_disturbed))
      return false;
    if (!locked_or_disturbed && !JS::ReadableStreamIsDisturbed(cx, stream, &locked_or_disturbed))
      return false;
    if (locked_or_disturbed) {
      // TODO: Improve this message; `disturbed` is probably too spec-internal a term.
      JS_ReportErrorUTF8(cx, "respondWith called with a Response containing "
                             "a body stream that's locked or disturbed");
      return false;
    }

    if (BodyStreamSource::stream_has_native_source(stream)) {
      // If the body stream is backed by a C@E body handle, we can directly pipe that handle
      // into the response body we're about to send.

      // First, move the source's body handle to the target.
      RootedObject stream_source(cx, BodyStreamSource::get_stream_source(stream));
      RootedObject source_owner(cx, BodyStreamSource::owner(stream_source));
      if (!RequestOrResponse::move_body_handle(cx, source_owner, response_obj))
        return false;

      // Then, send the response without streaming. We know that content won't append to this body
      // handle, because we don't expose any means to do so, so it's ok for it to be closed
      // immediately.
      return start_response(cx, response_obj, false);
    }

    RootedObject reader(cx, JS::ReadableStreamGetReader(cx, stream,
                                                        JS::ReadableStreamReaderMode::Default));
    if (!reader) return false;

    bool is_closed;
    if (!JS::ReadableStreamReaderIsClosed(cx, reader, &is_closed))
      return false;

    // It's ok for the stream to be closed, as its contents might
    // already have fully been written to the body handle.
    // In that case, we can do a blocking send instead.
    if (is_closed)
      return start_response(cx, response_obj, false);

    // Create handlers for both `then` and `catch`.
    // These are functions with two reserved slots, in which we store all information required
    // to perform the reactions.
    // We store the actually required information on the catch handler, and a reference to that
    // on the then handler. This allows us to reuse these functions for the next read operation
    // in the then handler. The catch handler won't ever have a need to perform another operation
    // in this way.
    JSFunction* catch_fun = js::NewFunctionWithReserved(cx, body_reader_catch_handler, 1, 0,
                                                        "catch_handler");
    if (!catch_fun) return false;
    RootedObject catch_handler(cx, JS_GetFunctionObject(catch_fun));

    js::SetFunctionNativeReserved(catch_handler, 0, JS::ObjectValue(*response_obj));
    js::SetFunctionNativeReserved(catch_handler, 1, JS::ObjectValue(*reader));

    JSFunction* then_fun = js::NewFunctionWithReserved(cx, body_reader_then_handler, 1, 0,
                                                       "then_handler");
    if (!then_fun) return false;
    RootedObject then_handler(cx, JS_GetFunctionObject(then_fun));

    js::SetFunctionNativeReserved(then_handler, 0, JS::ObjectValue(*catch_handler));

    RootedObject promise(cx, JS::ReadableStreamDefaultReaderRead(cx, reader));
    if (!promise) return false;
    if (!JS::AddPromiseReactions(cx, promise, then_handler, catch_handler))
      return false;

    if (!start_response(cx, response_obj, true))
      return false;

    return true;
  }

  // Steps in this function refer to the spec at
  // https://w3c.github.io/ServiceWorker/#fetch-event-respondwith
  bool response_promise_then_handler(JSContext* cx, unsigned argc, Value* vp) {
    CallArgs args = CallArgsFromVp(argc, vp);
    RootedObject event(cx, &js::GetFunctionNativeReserved(&args.callee(), 0).toObject());

    // Step 10.1
    // Note: the `then` handler is only invoked after all Promise resolution has happened.
    // (Even if there were multiple Promises to unwrap first.)
    // That means that at this point we're guaranteed to have the final value instead of a
    // Promise wrapping it, so either the value is a Response, or we have to bail.
    if (!Response::is_instance(args.get(0))) {
      JS_ReportErrorUTF8(cx, "FetchEvent#respondWith must be called with a Response "
                             "object or a Promise resolving to a Response object as "
                             "the first argument");
      RootedObject rejection(cx, PromiseRejectedWithPendingError(cx));
      if (!rejection) return false;
      args.rval().setObject(*rejection);
      return respondWithError(cx, event);
    }

    // Step 10.2 (very roughly: the way we handle responses and their bodies is very different.)
    RootedObject response_obj(cx, &args[0].toObject());

    // Ensure that all headers are stored client-side, so we retain access to them after
    // sending the response off.
    if (Response::is_upstream(response_obj)) {
        RootedObject headers(cx);
        headers = RequestOrResponse::headers<Headers::Mode::ProxyToResponse>(cx, response_obj);
        if (!Headers::delazify(cx, headers))
          return false;
    }

    if (RequestOrResponse::body_stream(response_obj)) {
      if (!respond_streaming(cx, response_obj))
        return false;
      set_state(event, State::responseStreaming);
    } else {
      if (!respond_blocking(cx, response_obj))
        return false;
      set_state(event, State::responseDone);
    }

    return true;
  }

  // Steps in this function refer to the spec at
  // https://w3c.github.io/ServiceWorker/#fetch-event-respondwith
  bool response_promise_catch_handler(JSContext* cx, unsigned argc, Value* vp) {
    CallArgs args = CallArgsFromVp(argc, vp);
    RootedObject event(cx, &js::GetFunctionNativeReserved(&args.callee(), 0).toObject());

    // TODO: verify that this is the right behavior.
    // Steps 9.1-2
    return respondWithError(cx, event);
  }

  // Steps in this function refer to the spec at
  // https://w3c.github.io/ServiceWorker/#fetch-event-respondwith
  bool respondWith(JSContext* cx, unsigned argc, Value* vp) {
    METHOD_HEADER(1)

    // Coercion of argument `r` to a Promise<Response>
    RootedObject response_promise(cx, JS::CallOriginalPromiseResolve(cx, args.get(0)));
    if (!response_promise) return false;

    // Step 2
    if (!is_dispatching(self)) {
      JS_ReportErrorUTF8(cx, "FetchEvent#respondWith must be called synchronously from "
                             "within a FetchEvent handler");
      return false;
    }

    // Step 3
    if (state(self) != State::unhandled) {
      JS_ReportErrorUTF8(cx,
                         "FetchEvent#respondWith can't be called twice on the same event");
      return false;
    }

    // Step 4
    detail::add_pending_promise(cx, self, response_promise);

    // Steps 5-7 (very roughly)
    set_state(self, State::waitToRespond);

    // Step 9 (continued in `response_promise_catch_handler` above)
    JSFunction* catch_fun = js::NewFunctionWithReserved(cx, response_promise_catch_handler, 1, 0,
                                                        "catch_handler");
    if (!catch_fun) return false;
    RootedObject catch_handler(cx, JS_GetFunctionObject(catch_fun));
    js::SetFunctionNativeReserved(catch_handler, 0, JS::ObjectValue(*self));

    // Step 10 (continued in `response_promise_then_handler` above)
    JSFunction* then_fun = js::NewFunctionWithReserved(cx, response_promise_then_handler, 1, 0,
                                                       "then_handler");
    if (!then_fun) return false;
    RootedObject then_handler(cx, JS_GetFunctionObject(then_fun));
    js::SetFunctionNativeReserved(then_handler, 0, JS::ObjectValue(*self));

    if (!JS::AddPromiseReactions(cx, response_promise, then_handler, catch_handler))
      return false;

    args.rval().setUndefined();
    return true;
  }

  bool respondWithError(JSContext* cx, HandleObject self) {
    MOZ_RELEASE_ASSERT(state(self) == State::unhandled || state(self) == State::waitToRespond);
    set_state(self, State::responsedWithError);
    ResponseHandle response { INVALID_HANDLE };
    BodyHandle body { INVALID_HANDLE };
    return HANDLE_RESULT(cx, xqd_resp_new(&response)) &&
          HANDLE_RESULT(cx, xqd_body_new(&body)) &&
          HANDLE_RESULT(cx, xqd_resp_status_set(response, 500)) &&
          HANDLE_RESULT(cx, xqd_resp_send_downstream(response, body, false));
  }

  // Step 5 of https://w3c.github.io/ServiceWorker/#wait-until-method
  bool dec_pending_promise_count(JSContext* cx, unsigned argc, Value* vp) {
    CallArgs args = CallArgsFromVp(argc, vp);
    RootedObject event(cx, &js::GetFunctionNativeReserved(&args.callee(), 0).toObject());

    // Step 5.1
    detail::dec_pending_promise_count(event);

    // Note: step 5.2 not relevant to our implementation.
    return true;
  }

  // Steps in this function refer to the spec at
  // https://w3c.github.io/ServiceWorker/#wait-until-method
  bool waitUntil(JSContext* cx, unsigned argc, Value* vp) {
    METHOD_HEADER(1)

    RootedObject promise(cx, JS::CallOriginalPromiseResolve(cx, args.get(0)));
    if (!promise) return false;

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

  const JSFunctionSpec methods[] = {
    JS_FN("respondWith", respondWith, 1, 0),
    JS_FN("waitUntil", waitUntil, 1, 0),
  JS_FS_END};

  const JSPropertySpec properties[] = {
    JS_PSG("client", client_get, 0),
    JS_PSG("service", service_get, 0),
    JS_PSG("request", request_get, 0),
  JS_PS_END};

  CLASS_BOILERPLATE_NO_CTOR(FetchEvent)

  JSObject* create(JSContext* cx) {
    RootedObject self(cx, JS_NewObjectWithGivenProto(cx, &class_, proto_obj));
    if (!self) return nullptr;

    RootedObject request(cx, prepare_downstream_request(cx));
    if (!request) return nullptr;

    JSFunction* dec_count_fun = js::NewFunctionWithReserved(cx, dec_pending_promise_count, 1, 0,
                                                       "dec_pending_promise_count");
    if (!dec_count_fun) return nullptr;
    RootedObject dec_count_handler(cx, JS_GetFunctionObject(dec_count_fun));
    js::SetFunctionNativeReserved(dec_count_handler, 0, JS::ObjectValue(*self));

    JS::SetReservedSlot(self, Slots::Request, JS::ObjectValue(*request));
    JS::SetReservedSlot(self, Slots::Dispatch, JS::FalseValue());
    JS::SetReservedSlot(self, Slots::State, JS::Int32Value((int)State::unhandled));
    JS::SetReservedSlot(self, Slots::PendingPromiseCount, JS::Int32Value(0));
    JS::SetReservedSlot(self, Slots::DecPendingPromiseCountFunc,
                        JS::ObjectValue(*dec_count_handler));

    return self;
  }

  bool is_active(JSObject* self) {
    MOZ_ASSERT(is_instance(self));
    return JS::GetReservedSlot(self, Slots::Dispatch).toBoolean() ||
           JS::GetReservedSlot(self, Slots::PendingPromiseCount).toInt32() > 0;
  }

  bool is_dispatching(JSObject* self) {
    MOZ_ASSERT(is_instance(self));
    return JS::GetReservedSlot(self, Slots::Dispatch).toBoolean();
  }

  void start_dispatching(JSObject* self) {
    MOZ_ASSERT(!is_dispatching(self));
    JS::SetReservedSlot(self, Slots::Dispatch, JS::TrueValue());
  }

  void stop_dispatching(JSObject* self) {
    MOZ_ASSERT(is_dispatching(self));
    JS::SetReservedSlot(self, Slots::Dispatch, JS::FalseValue());
  }

  State state(JSObject* self) {
    MOZ_ASSERT(is_instance(self));
    return (State)JS::GetReservedSlot(self, Slots::State).toInt32();
  }

  void set_state(JSObject* self, State new_state) {
    MOZ_ASSERT(is_instance(self));
    MOZ_ASSERT((uint)new_state > (uint)state(self));
    JS::SetReservedSlot(self, Slots::State, JS::Int32Value((int)new_state));
  }

  bool response_started(JSObject* self) {
    State current_state = state(self);
    return current_state != State::unhandled && current_state != State::waitToRespond;
  }
}


namespace URLSearchParams {
  JSObject* create(JSContext* cx, jsurl::JSUrl* url);
  JSUrlSearchParams* get_params(JSObject* self);
}

namespace URL {
  namespace Slots { enum {
    Url,
    Params,
    Count
  };};

  const unsigned ctor_length = 1;

  bool check_receiver(JSContext* cx, HandleObject self, const char* method_name);
  JSObject* create(JSContext* cx, HandleValue url_val, HandleValue base_val);

  bool constructor(JSContext* cx, unsigned argc, Value* vp) {
    CallArgs args = CallArgsFromVp(argc, vp);
    if (!args.requireAtLeast(cx, "URL", 1))
      return false;

    RootedObject self(cx, create(cx, args.get(0), args.get(1)));
    if (!self) return false;

    args.rval().setObject(*self);
    return true;
  }

#define ACCESSOR_GET(field) \
  bool field##_get(JSContext* cx, unsigned argc, Value* vp) { \
    METHOD_HEADER(0) \
    const JSUrl* url = (JSUrl*)JS::GetReservedSlot(self, Slots::Url).toPrivate(); \
    const SpecSlice slice = jsurl::field(url); \
    RootedString str(cx, JS_NewStringCopyUTF8N(cx, JS::UTF8Chars((char*)slice.data, slice.len))); \
    if (!str) return false; \
    args.rval().setString(str); \
    return true; \
  } \

#define ACCESSOR_SET(field) \
  bool field##_set(JSContext* cx, unsigned argc, Value* vp) { \
    METHOD_HEADER(1) \
    JSUrl* url = (JSUrl*)JS::GetReservedSlot(self, Slots::Url).toPrivate(); \
    \
    SpecString str = encode(cx, args.get(0)); \
    if (!str.data) return false; \
    jsurl::set_##field(url, &str); \
    \
    args.rval().set(args.get(0)); \
    return true; \
  }

#define ACCESSOR(field) \
  ACCESSOR_GET(field) \
  ACCESSOR_SET(field)

  ACCESSOR(hash)
  ACCESSOR(host)
  ACCESSOR(hostname)
  ACCESSOR(href)
  ACCESSOR(password)
  ACCESSOR(pathname)
  ACCESSOR(port)
  ACCESSOR(protocol)
  ACCESSOR(search)
  ACCESSOR(username)

  bool origin_get(JSContext* cx, unsigned argc, Value* vp) {
    METHOD_HEADER(0)
    const JSUrl* url = (JSUrl*)JS::GetReservedSlot(self, Slots::Url).toPrivate();
    SpecString slice = jsurl::origin(url);
    RootedString str(cx, JS_NewStringCopyUTF8N(cx, JS::UTF8Chars((char*)slice.data, slice.len)));
    if (!str) return false;
    args.rval().setString(str);
    return true;
  }

  bool searchParams_get(JSContext* cx, unsigned argc, Value* vp) {
    METHOD_HEADER(0)
    JS::Value params_val = JS::GetReservedSlot(self, Slots::Params);
    RootedObject params(cx);
    if (params_val.isNullOrUndefined()) {
      JSUrl* url = (JSUrl*)JS::GetReservedSlot(self, Slots::Url).toPrivate();
      params = URLSearchParams::create(cx, url);
      if (!params) return false;
      JS::SetReservedSlot(self, Slots::Params, JS::ObjectValue(*params));
    } else {
      params = &params_val.toObject();
    }

    args.rval().setObject(*params);
    return true;
  }

  bool toString(JSContext* cx, unsigned argc, Value* vp) {
    METHOD_HEADER(0)
    return href_get(cx, argc, vp);
  }

  bool toJSON(JSContext* cx, unsigned argc, Value* vp) {
    METHOD_HEADER(0)
    return href_get(cx, argc, vp);
  }

  const JSFunctionSpec methods[] = {
    JS_FN("toString", toString, 0, JSPROP_ENUMERATE),
    JS_FN("toJSON", toJSON, 0, JSPROP_ENUMERATE),
    JS_FS_END
  };

  const JSPropertySpec properties[] = {
    JS_PSGS("hash", hash_get, hash_set, JSPROP_ENUMERATE),
    JS_PSGS("host", host_get, host_set, JSPROP_ENUMERATE),
    JS_PSGS("hostname", hostname_get, hostname_set, JSPROP_ENUMERATE),
    JS_PSGS("href", href_get, href_set, JSPROP_ENUMERATE),
    JS_PSG("origin", origin_get, JSPROP_ENUMERATE),
    JS_PSGS("password", password_get, password_set, JSPROP_ENUMERATE),
    JS_PSGS("pathname", pathname_get, pathname_set, JSPROP_ENUMERATE),
    JS_PSGS("port", port_get, port_set, JSPROP_ENUMERATE),
    JS_PSGS("protocol", protocol_get, protocol_set, JSPROP_ENUMERATE),
    JS_PSGS("search", search_get, search_set, JSPROP_ENUMERATE),
    JS_PSG("searchParams", searchParams_get, JSPROP_ENUMERATE),
    JS_PSGS("username", username_get, username_set, JSPROP_ENUMERATE),
    JS_PS_END
  };

  CLASS_BOILERPLATE(URL)

  JSObject* create(JSContext* cx, HandleValue url_val, HandleValue base_val) {
    RootedObject self(cx, JS_NewObjectWithGivenProto(cx, &class_, proto_obj));
    if (!self) return nullptr;

    JSUrl* base = nullptr;

    if (!base_val.isUndefined()) {
      auto str = encode(cx, base_val);
      if (!str.data) return nullptr;

      JSUrl* base = jsurl::new_jsurl(&str);
      if (!base) {
        JS_ReportErrorUTF8(cx, "URL constructor: %s is not a valid URL.", (char*)str.data);
        return nullptr;
      }
    }

    auto str = encode(cx, url_val);
    if (!str.data) return nullptr;

    JSUrl* url;
    if (base) {
      url = jsurl::new_jsurl_with_base(&str, base);
    } else {
      url = jsurl::new_jsurl(&str);
    }

    JS::SetReservedSlot(self, Slots::Url, JS::PrivateValue(url));

    return self;
  }
}


#define ITERTYPE_ENTRIES 0
#define ITERTYPE_KEYS 1
#define ITERTYPE_VALUES 2

namespace URLSearchParamsIterator {
  namespace Slots { enum {
    Params,
    Type,
    Index,
    Count
  };};

  const unsigned ctor_length = 0;
  // This constructor will be deleted from the class prototype right after class initialization.
  bool constructor(JSContext* cx, unsigned argc, Value* vp) {
    MOZ_RELEASE_ASSERT(false, "Should be deleted");
    return false;
  }

  bool check_receiver(JSContext* cx, HandleObject self, const char* method_name);

  bool next(JSContext* cx, unsigned argc, Value* vp) {
    METHOD_HEADER(0)
    RootedObject params_obj(cx, &JS::GetReservedSlot(self, Slots::Params).toObject());
    const auto params = URLSearchParams::get_params(params_obj);
    size_t index = JS::GetReservedSlot(self, Slots::Index).toInt32();
    uint8_t type = JS::GetReservedSlot(self, Slots::Type).toInt32();

    RootedObject result(cx, JS_NewPlainObject(cx));
    if (!result) return false;

    auto param = JSSearchParam(SpecSlice(nullptr, 0), SpecSlice(nullptr, 0), false);
    jsurl::params_at(params, index, &param);

    if (param.done) {
      JS_DefineProperty(cx, result, "done", true, JSPROP_ENUMERATE);
      JS_DefineProperty(cx, result, "value", JS::UndefinedHandleValue, JSPROP_ENUMERATE);

      args.rval().setObject(*result);
      return true;
    }

    JS_DefineProperty(cx, result, "done", false, JSPROP_ENUMERATE);

    RootedValue key_val(cx);
    RootedValue val_val(cx);

    if (type != ITERTYPE_VALUES) {
      auto chars = JS::UTF8Chars((char*)param.name.data, param.name.len);
      RootedString str(cx, JS_NewStringCopyUTF8N(cx, chars));
      if (!str) return false;
      key_val = JS::StringValue(str);
    }

    if (type != ITERTYPE_KEYS) {
      auto chars = JS::UTF8Chars((char*)param.value.data, param.value.len);
      RootedString str(cx, JS_NewStringCopyUTF8N(cx, chars));
      if (!str) return false;
      val_val = JS::StringValue(str);
    }

    RootedValue result_val(cx);

    switch (type) {
      case ITERTYPE_ENTRIES: {
        RootedObject pair(cx, JS::NewArrayObject(cx, 2));
        if (!pair) return false;
        JS_DefineElement(cx, pair, 0, key_val, JSPROP_ENUMERATE);
        JS_DefineElement(cx, pair, 1, val_val, JSPROP_ENUMERATE);
        result_val = JS::ObjectValue(*pair);
        break;
      }
      case ITERTYPE_KEYS: {
        result_val = key_val;
        break;
      }
      case ITERTYPE_VALUES: {
        result_val = val_val;
        break;
      }
      default:
        MOZ_RELEASE_ASSERT(false, "Invalid iter type");
    }

    JS_DefineProperty(cx, result, "value", result_val, JSPROP_ENUMERATE);

    JS::SetReservedSlot(self, Slots::Index, JS::Int32Value(index + 1));
    args.rval().setObject(*result);
    return true;
  }

  const JSFunctionSpec methods[] = {
    JS_FN("next", next, 0, JSPROP_ENUMERATE),
  JS_FS_END};

  const JSPropertySpec properties[] = {JS_PS_END};

  CLASS_BOILERPLATE_CUSTOM_INIT(URLSearchParamsIterator)

  bool init_class(JSContext* cx, HandleObject global) {
    RootedObject iterator_proto(cx, JS::GetRealmIteratorPrototype(cx));
    if (!iterator_proto) return false;

    if (!init_class_impl(cx, global, iterator_proto))
      return false;

    // Delete both the `URLSearchParamsIterator` global property and the `constructor`
    // property on `URLSearchParamsIterator.prototype`.
    // The latter because Iterators don't have their own constructor on the prototype.
    return JS_DeleteProperty(cx, global, class_.name) &&
           JS_DeleteProperty(cx, proto_obj, "constructor");
  }

  JSObject* create(JSContext* cx, HandleObject params, uint8_t type) {
    MOZ_RELEASE_ASSERT(type <= ITERTYPE_VALUES);

    RootedObject self(cx, JS_NewObjectWithGivenProto(cx, &class_, proto_obj));
    if (!self) return nullptr;

    JS::SetReservedSlot(self, Slots::Params, JS::ObjectValue(*params));
    JS::SetReservedSlot(self, Slots::Type, JS::Int32Value(type));
    JS::SetReservedSlot(self, Slots::Index, JS::Int32Value(0));

    return self;
  }
}


namespace URLSearchParams {
  namespace Slots { enum {
    Url,
    Params,
    Count
  };};

  namespace detail {
    bool append(JSContext* cx, HandleObject self, HandleValue key, HandleValue val, const char* _) {
      const auto params = (JSUrlSearchParams*)JS::GetReservedSlot(self, Slots::Params).toPrivate();

      auto name = encode(cx, key);
      if (!name.data) return false;

      auto value = encode(cx, val);
      if (!value.data) return false;

      jsurl::params_append(params, name, value);
      return true;
    }
  }

  JSUrlSearchParams* get_params(JSObject* self) {
    return (JSUrlSearchParams*)JS::GetReservedSlot(self, Slots::Params).toPrivate();
  }

  bool check_receiver(JSContext* cx, HandleObject self, const char* method_name);
  JSObject* create(JSContext* cx, HandleValue params_val);

  const unsigned ctor_length = 1;

  bool constructor(JSContext* cx, unsigned argc, Value* vp) {
    CallArgs args = CallArgsFromVp(argc, vp);

    RootedObject self(cx, create(cx, args.get(0)));
    if (!self) return false;

    args.rval().setObject(*self);
    return true;
  }

  bool append(JSContext* cx, unsigned argc, Value* vp) {
    METHOD_HEADER(2)
    if (!detail::append(cx, self, args[0], args[1], "append"))
      return false;

    args.rval().setUndefined();
    return true;
  }

  bool delete_(JSContext* cx, unsigned argc, Value* vp) {
    METHOD_HEADER_WITH_NAME(1, "delete")
    const auto params = (JSUrlSearchParams*)JS::GetReservedSlot(self, Slots::Params).toPrivate();

    auto name = encode(cx, args.get(0));
    if (!name.data) return false;

    jsurl::params_delete(params, &name);
    args.rval().setUndefined();
    return true;
  }

  bool has(JSContext* cx, unsigned argc, Value* vp) {
    METHOD_HEADER(1)
    const auto params = (JSUrlSearchParams*)JS::GetReservedSlot(self, Slots::Params).toPrivate();

    auto name = encode(cx, args.get(0));
    if (!name.data) return false;

    args.rval().setBoolean(jsurl::params_has(params, &name));
    return true;
  }

  bool get(JSContext* cx, unsigned argc, Value* vp) {
    METHOD_HEADER(1)
    const auto params = (JSUrlSearchParams*)JS::GetReservedSlot(self, Slots::Params).toPrivate();

    auto name = encode(cx, args.get(0));
    if (!name.data) return false;

    const SpecSlice slice = jsurl::params_get(params, &name);
    if (!slice.data) {
      args.rval().setNull();
      return true;
    }

    RootedString str(cx, JS_NewStringCopyUTF8N(cx, JS::UTF8Chars((char*)slice.data, slice.len)));
    if (!str) return false;
    args.rval().setString(str);
    return true;
  }

  bool getAll(JSContext* cx, unsigned argc, Value* vp) {
    METHOD_HEADER(1)
    const auto params = (JSUrlSearchParams*)JS::GetReservedSlot(self, Slots::Params).toPrivate();

    auto name = encode(cx, args.get(0));
    if (!name.data) return false;

    const jsurl::CVec<SpecSlice> values = jsurl::params_get_all(params, &name);

    RootedObject result(cx, JS::NewArrayObject(cx, values.len));
    if (!result) return false;

    RootedString str(cx);
    RootedValue str_val(cx);
    for (size_t i = 0; i < values.len; i++) {
      const SpecSlice value = values.ptr[i];
      str = JS_NewStringCopyUTF8N(cx, JS::UTF8Chars((char*)value.data, value.len));
      if (!str) return false;

      str_val.setString(str);
      if (!JS_SetElement(cx, result, i, str_val))
        return false;
    }

    args.rval().setObject(*result);
    return true;
  }

  bool set(JSContext* cx, unsigned argc, Value* vp) {
    METHOD_HEADER(2)
    const auto params = (JSUrlSearchParams*)JS::GetReservedSlot(self, Slots::Params).toPrivate();

    auto name = encode(cx, args[0]);
    if (!name.data) return false;

    auto value = encode(cx, args[1]);
    if (!value.data) return false;

    jsurl::params_set(params, name, value);
    return true;

    args.rval().setUndefined();
    return true;
  }

  bool sort(JSContext* cx, unsigned argc, Value* vp) {
    METHOD_HEADER(0)
    const auto params = (JSUrlSearchParams*)JS::GetReservedSlot(self, Slots::Params).toPrivate();
    jsurl::params_sort(params);
    args.rval().setUndefined();
    return true;
  }

  bool toString(JSContext* cx, unsigned argc, Value* vp) {
    METHOD_HEADER(0)
    const auto params = (JSUrlSearchParams*)JS::GetReservedSlot(self, Slots::Params).toPrivate();

    const SpecSlice slice = jsurl::params_to_string(params);
    RootedString str(cx, JS_NewStringCopyUTF8N(cx, JS::UTF8Chars((char*)slice.data, slice.len)));
    if (!str) return false;

    args.rval().setString(str);
    return true;
  }

  bool forEach(JSContext* cx, unsigned argc, Value* vp) {
    METHOD_HEADER(1)
    const auto params = get_params(self);

    if (!args[0].isObject() || !JS::IsCallable(&args[0].toObject())) {
      JS_ReportErrorASCII(cx, "Failed to execute 'forEach' on 'URLSearchParams': "
                              "parameter 1 is not of type 'Function'");
      return false;
    }

    HandleValue callback = args[0];
    HandleValue thisv = args.get(1);

    JS::RootedValueArray<3> newArgs(cx);
    newArgs[2].setObject(*self);
    RootedValue rval(cx);

    auto param = JSSearchParam(SpecSlice(nullptr, 0), SpecSlice(nullptr, 0), false);
    RootedString name_str(cx);
    RootedString val_str(cx);
    size_t index = 0;
    while (true) {
      jsurl::params_at(params, index, &param);
      if (param.done)
        break;

      name_str = JS_NewStringCopyUTF8N(cx, JS::UTF8Chars((char*)param.name.data, param.name.len));
      if (!name_str) return false;
      newArgs[1].setString(name_str);

      val_str = JS_NewStringCopyUTF8N(cx, JS::UTF8Chars((char*)param.value.data, param.value.len));
      if (!val_str) return false;
      newArgs[0].setString(val_str);

      if (!JS::Call(cx, thisv, callback, newArgs, &rval))
        return false;

      index++;
    }

    args.rval().setUndefined();
    return true;
  }

  template<auto type>
  bool get_iter(JSContext* cx, unsigned argc, Value* vp) {
    METHOD_HEADER(0)

    RootedObject iter(cx, URLSearchParamsIterator::create(cx, self, type));
    if (!iter) return false;
    args.rval().setObject(*iter);
    return true;
  }

  const JSFunctionSpec methods[] = {
    JS_FN("append", append, 2, JSPROP_ENUMERATE),
    JS_FN("delete", delete_, 1, JSPROP_ENUMERATE),
    JS_FN("has", has, 1, JSPROP_ENUMERATE),
    JS_FN("get", get, 1, JSPROP_ENUMERATE),
    JS_FN("getAll", getAll, 1, JSPROP_ENUMERATE),
    JS_FN("set", set, 2, JSPROP_ENUMERATE),
    JS_FN("sort", sort, 0, JSPROP_ENUMERATE),
    JS_FN("toString", toString, 0, JSPROP_ENUMERATE),
    JS_FN("forEach", forEach, 0, JSPROP_ENUMERATE),
    JS_FN("entries", get_iter<ITERTYPE_ENTRIES>, 0, 0),
    JS_FN("keys", get_iter<ITERTYPE_KEYS>, 0, 0),
    JS_FN("values", get_iter<ITERTYPE_VALUES>, 0, 0),
    // [Symbol.iterator] added in init_class.
    JS_FS_END
  };

  const JSPropertySpec properties[] = {JS_PS_END};

  CLASS_BOILERPLATE_CUSTOM_INIT(URLSearchParams)

  bool init_class(JSContext* cx, HandleObject global) {
    if (!init_class_impl(cx, global))
      return false;

    RootedValue entries(cx);
    if (!JS_GetProperty(cx, proto_obj, "entries", &entries))
      return false;

    JS::SymbolCode code = JS::SymbolCode::iterator;
    JS::RootedId iteratorId(cx, SYMBOL_TO_JSID(JS::GetWellKnownSymbol(cx, code)));
    return JS_DefinePropertyById(cx, proto_obj, iteratorId, entries, 0);
  }

  JSObject* create(JSContext* cx, HandleValue params_val) {
    RootedObject self(cx, JS_NewObjectWithGivenProto(cx, &class_, proto_obj));
    if (!self) return nullptr;

    auto params = jsurl::new_params();
    JS::SetReservedSlot(self, Slots::Params, JS::PrivateValue(params));

    bool consumed = false;
    const char* alt_text = ", or a value that can be stringified";
    if (!maybe_consume_sequence_or_record<detail::append>(cx, params_val, self, &consumed,
                                                          "URLSearchParams", alt_text))
    {
      return nullptr;
    }

    if (!consumed) {
      auto init = encode(cx, params_val);
      if (!init.data) return nullptr;

      jsurl::params_init(params, &init);
    }

    return self;
  }

  JSObject* create(JSContext* cx, JSUrl* url) {
    RootedObject self(cx, JS_NewObjectWithGivenProto(cx, &class_, proto_obj));
    if (!self) return nullptr;

    JSUrlSearchParams* params = jsurl::url_search_params(url);
    if (!params) return nullptr;

    JS::SetReservedSlot(self, Slots::Params, JS::PrivateValue(params));
    JS::SetReservedSlot(self, Slots::Url, JS::PrivateValue(url));

    return self;
  }
}


// TODO: throw in all Request methods/getters that rely on host calls once a
// request has been sent. The host won't let us act on them anymore anyway.
bool fetch(JSContext* cx, unsigned argc, Value* vp) {
  CallArgs args = CallArgsFromVp(argc, vp);
  if (!args.requireAtLeast(cx, "fetch", 1)) {
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }

  RootedObject request(cx, Request::create(cx, args[0], args.get(1)));

  RootedString backend(cx, Request::backend(request));
  if (!backend) {
    size_t bytes_read;
    RequestHandle handle = Request::request_handle(request);
    UniqueChars buf(read_from_handle_all<xqd_req_uri_get, RequestHandle>(cx, handle, &bytes_read,
                                                                         false));
    if (buf) {
      JS_ReportErrorUTF8(cx, "No backend specified for request with url %s. "
                             "Must provide a `backend` property on the `init` object "
                             "passed to either `new Request()` or `fetch`", buf.get());
    }
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }

  size_t backend_len;
  UniqueChars backend_chars = encode(cx, backend, &backend_len);
  if (!backend_chars)
    return ReturnPromiseRejectedWithPendingError(cx, args);

  PendingRequestHandle request_handle = { INVALID_HANDLE };
  RootedObject response_promise(cx, JS::NewPromiseObject(cx, nullptr));
  if (!response_promise)
    return ReturnPromiseRejectedWithPendingError(cx, args);

  // TODO: ensure that we properly handle body handles forwarded from other Request/Response
  // objects.
  int result = xqd_req_send_async(Request::request_handle(request),
                                  RequestOrResponse::body_handle(request),
                                  backend_chars.get(), backend_len, &request_handle);
  if (!HANDLE_RESULT(cx, result))
    return ReturnPromiseRejectedWithPendingError(cx, args);

  if (!pending_requests->append(request))
    return ReturnPromiseRejectedWithPendingError(cx, args);

  JS::SetReservedSlot(request, Request::Slots::PendingRequest, JS::Int32Value(request_handle.handle));
  JS::SetReservedSlot(request, Request::Slots::ResponsePromise, JS::ObjectValue(*response_promise));

  args.rval().setObject(*response_promise);
  return true;
}

bool has_pending_requests() {
  return pending_requests->length() > 0 || pending_body_reads->length() > 0;
}

bool process_pending_requests(JSContext* cx) {
  if (pending_requests->length() == 0) return true;

  size_t count = pending_requests->length();
  auto handles = mozilla::MakeUnique<PendingRequestHandle[]>(sizeof(PendingRequestHandle) * count);
  if (!handles)
    return false;

  for (size_t i = 0; i < count; i++) {
      handles[i] = Request::pending_handle((*pending_requests)[i]);
  }

  uint32_t done_index;
  ResponseHandle response_handle = { INVALID_HANDLE };
  BodyHandle body = { INVALID_HANDLE };
  int result = xqd_req_pending_req_select(handles.get(), count, &done_index,
                                          &response_handle, &body);
  if (!HANDLE_RESULT(cx, result))
    return false;

  HandleObject request = (*pending_requests)[done_index];
  RootedObject response(cx, Response::create(cx, response_handle, body, true));
  if (!response) return false;

  RequestOrResponse::set_url(response, RequestOrResponse::url(request));
  RootedValue response_val(cx, JS::ObjectValue(*response));

  RootedObject response_promise(cx, Request::response_promise(request));

  pending_requests->erase(const_cast<JSObject**>(request.address()));

  return JS::ResolvePromise(cx, response_promise, response_val);
}

bool process_next_body_read(JSContext* cx) {
  if (pending_body_reads->length() == 0) return true;

  RootedObject owner(cx);
  RootedObject controller(cx);
  {
    HandleObject streamSource = (*pending_body_reads)[0];
    owner = BodyStreamSource::owner(streamSource);
    controller = BodyStreamSource::controller(streamSource);
    pending_body_reads->erase(const_cast<JSObject**>(streamSource.address()));
  }

  BodyHandle body = RequestOrResponse::body_handle(owner);
  char* bytes = static_cast<char*>(JS_malloc(cx, HANDLE_READ_CHUNK_SIZE));
  if (!bytes) return false;
  size_t nwritten;
  int result = xqd_body_read(body, bytes, HANDLE_READ_CHUNK_SIZE, &nwritten);
  if (!HANDLE_RESULT(cx, result)) {
    JS_free(cx, bytes);
    return false;
  }

  if (nwritten == 0) {
    RootedValue r(cx);
    return JS::Call(cx, controller, "close", HandleValueArray::empty(), &r);
  }

  char* new_bytes = static_cast<char*>(JS_realloc(cx, bytes, HANDLE_READ_CHUNK_SIZE, nwritten));
  if (!new_bytes) {
    JS_free(cx, bytes);
    return false;
  }
  bytes = new_bytes;

  RootedObject buffer(cx, JS::NewArrayBufferWithContents(cx, nwritten, bytes));
  if (!buffer) {
    JS_free(cx, bytes);
    return false;
  }

  RootedObject byte_array(cx, JS_NewUint8ArrayWithBuffer(cx, buffer, 0, nwritten));
  if (!byte_array) return false;

  JS::RootedValueArray<1> enqueue_args(cx);
  enqueue_args[0].setObject(*byte_array);
  RootedValue r(cx);
  if (!JS::Call(cx, controller, "enqueue", enqueue_args, &r))
    return false;


  return true;
}

bool process_network_io(JSContext* cx) {
  if (!has_pending_requests()) return true;

  if (!process_pending_requests(cx)) return false;

  if (!process_next_body_read(cx)) return false;

  return true;
}

bool define_fastly_sys(JSContext* cx, HandleObject global) {
  // Allocating the reusable hostcall buffer here means it's baked into the
  // snapshot, and since it's all zeros, it won't increase the size of the snapshot.
  if (!OwnedHostCallBuffer::initialize(cx)) return false;
  if (!JS_DefineProperty(cx, global, "self", global, JSPROP_ENUMERATE)) return false;

  if (!Fastly::create(cx, global)) return false;
  if (!Console::create(cx, global)) return false;
  if (!Crypto::create(cx, global)) return false;

  if (!BodyStreamSource::init_class(cx, global)) return false;
  if (!Request::init_class(cx, global)) return false;
  if (!Response::init_class(cx, global)) return false;
  if (!Dictionary::init_class(cx, global)) return false;
  if (!Headers::init_class(cx, global)) return false;
  if (!ClientInfo::init_class(cx, global)) return false;
  if (!FetchEvent::init_class(cx, global)) return false;
  if (!CacheOverride::init_class(cx, global)) return false;
  if (!TextEncoder::init_class(cx, global)) return false;
  if (!TextDecoder::init_class(cx, global)) return false;
  if (!Logger::init_class(cx, global)) return false;
  if (!URL::init_class(cx, global)) return false;
  if (!URLSearchParams::init_class(cx, global)) return false;
  if (!URLSearchParamsIterator::init_class(cx, global)) return false;

  pending_requests = new JS::PersistentRootedObjectVector(cx);
  pending_body_reads = new JS::PersistentRootedObjectVector(cx);

  return JS_DefineFunction(cx, global, "fetch", fetch, 2, JSPROP_ENUMERATE);
}

JSObject* create_fetch_event(JSContext* cx) {
    return FetchEvent::create(cx);
}

UniqueChars stringify_value(JSContext* cx, JS::HandleValue value) {
  JS::RootedString str(cx, JS_ValueToSource(cx, value));
  if (!str)
  return nullptr;

  return JS_EncodeStringToUTF8(cx, str);
}

bool debug_logging_enabled() {
  return Fastly::debug_logging_enabled;
}

bool dump_value(JSContext* cx, JS::Value val, FILE* fp) {
  RootedValue value(cx, val);
  UniqueChars utf8chars = stringify_value(cx, value);
  if (!utf8chars)
    return false;
  fprintf(fp, "%s\n", utf8chars.get());
  return true;
}

bool print_stack(JSContext* cx, HandleObject stack, FILE* fp) {
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

bool print_stack(JSContext* cx, FILE* fp) {
  RootedObject stackp(cx);
  if (!JS::CaptureCurrentStack(cx, &stackp))
    return false;
  return print_stack(cx, stackp, fp);
}
