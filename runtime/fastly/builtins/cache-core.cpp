#include "cache-core.h"
#include "../../../StarlingMonkey/builtins/web/fetch/headers.h"
#include "../../../StarlingMonkey/builtins/web/streams/native-stream-source.h"
#include "../../../StarlingMonkey/runtime/encode.h"
#include "../host-api/host_api_fastly.h"
#include "body.h"
#include "builtin.h"
#include "fastly.h"
#include "host_api.h"
#include "js/Stream.h"
#include <iostream>

using builtins::web::fetch::Headers;
using builtins::web::streams::NativeStreamSource;
using fastly::body::FastlyBody;
using fastly::fastly::convertBodyInit;
using fastly::fetch::Request;
using fastly::fetch::RequestOrResponse;

using builtins::web::fetch::Headers;

namespace fastly::cache_core {

namespace {

api::Engine *ENGINE;

// The JavaScript LookupOptions parameter we are parsing should have the below interface:
// interface LookupOptions {
//   headers?: HeadersInit;
// }
JS::Result<host_api::CacheLookupOptions> parseLookupOptions(JSContext *cx,
                                                            JS::HandleValue options_val) {
  host_api::CacheLookupOptions options;
  // options parameter is optional
  // options is meant to be an object with an optional headers field,
  // the headers field can be:
  // Headers | string[][] | Record<string, string>;
  if (!options_val.isUndefined()) {
    if (!options_val.isObject()) {
      JS_ReportErrorASCII(cx, "options argument must be an object");
      return JS::Result<host_api::CacheLookupOptions>(JS::Error());
    }
    JS::RootedObject options_obj(cx, &options_val.toObject());
    JS::RootedValue headers_val(cx);
    if (!JS_GetProperty(cx, options_obj, "headers", &headers_val)) {
      return JS::Result<host_api::CacheLookupOptions>(JS::Error());
    }
    // headers property is optional
    if (!headers_val.isUndefined()) {
      if (!headers_val.isObject()) {
        JS_ReportErrorASCII(
            cx, "Failed to construct Headers object. If defined, the first argument must be either "
                "a [ ['name', 'value'], ... ] sequence, or a { 'name' : 'value', ... } record.");
        return JS::Result<host_api::CacheLookupOptions>(JS::Error());
      }

      auto request_handle_res = host_api::HttpReq::make();
      if (auto *err = request_handle_res.to_err()) {
        HANDLE_ERROR(cx, *err);
        return JS::Result<host_api::CacheLookupOptions>(JS::Error());
      }
      auto request_handle = request_handle_res.unwrap();

      JS::RootedObject headers(cx,
                               Headers::create(cx, headers_val, Headers::HeadersGuard::Request));
      if (!headers) {
        return JS::Result<host_api::CacheLookupOptions>(JS::Error());
      }
      if (Headers::mode(headers) == Headers::Mode::Uninitialized) {
        return options;
      }
      MOZ_ASSERT(Headers::mode(headers) == Headers::Mode::ContentOnly);
      Headers::HeadersList *headers_list = Headers::get_list(cx, headers);
      MOZ_ASSERT(headers_list);
      auto res = host_api::write_headers(request_handle.headers_writable(), *headers_list);
      if (res.is_err()) {
        return JS::Result<host_api::CacheLookupOptions>(JS::Error());
      }
      options.request_headers = host_api::HttpReq(request_handle);
    }
  }
  return options;
}

// The JavaScript TransactionUpdateOptions parameter we are parsing should have the below interface:
// interface TransactionUpdateOptions {
//   maxAge: number,
//   vary?: Array<string>,
//   initialAge?: number,
//   staleWhileRevalidate?: number,
//   surrogateKeys?: Array<string>,
//   length?: number,
//   userMetadata?: ArrayBufferView | ArrayBuffer | URLSearchParams | string,
// }
JS::Result<host_api::CacheWriteOptions> parseTransactionUpdateOptions(JSContext *cx,
                                                                      JS::HandleValue options_val) {
  host_api::CacheWriteOptions options;
  if (!options_val.isObject()) {
    JS_ReportErrorASCII(cx, "options argument must be an object");
    return JS::Result<host_api::CacheWriteOptions>(JS::Error());
  }
  JS::RootedObject options_obj(cx, &options_val.toObject());

  JS::RootedValue maxAge_val(cx);
  if (!JS_GetProperty(cx, options_obj, "maxAge", &maxAge_val) || maxAge_val.isUndefined()) {
    JS_ReportErrorASCII(cx, "maxAge is required");
    return JS::Result<host_api::CacheWriteOptions>(JS::Error());
  }

  // Convert maxAge field into a number and check the value adheres to our
  // validation rules.
  double maxAge;
  if (!JS::ToNumber(cx, maxAge_val, &maxAge)) {
    return JS::Result<host_api::CacheWriteOptions>(JS::Error());
  }
  if (maxAge < 0 || std::isnan(maxAge) || std::isinf(maxAge)) {
    JS_ReportErrorASCII(
        cx,
        "maxAge field is an invalid value, only positive numbers can be used for maxAge values.");
    return JS::Result<host_api::CacheWriteOptions>(JS::Error());
  }
  // turn millisecond representation into nanosecond representation
  options.max_age_ns = JS::ToUint64(maxAge) * 1'000'000;

  if (options.max_age_ns > pow(2, 63)) {
    JS_ReportErrorASCII(cx, "maxAge can not be greater than 2^63.");
    return JS::Result<host_api::CacheWriteOptions>(JS::Error());
  }

  JS::RootedValue initialAge_val(cx);
  if (!JS_GetProperty(cx, options_obj, "initialAge", &initialAge_val)) {
    return JS::Result<host_api::CacheWriteOptions>(JS::Error());
  }
  if (!initialAge_val.isUndefined()) {
    // Convert initialAge field into a number and check the value adheres to our
    // validation rules.
    double initialAge;
    if (!JS::ToNumber(cx, initialAge_val, &initialAge)) {
      return JS::Result<host_api::CacheWriteOptions>(JS::Error());
    }
    if (initialAge < 0 || std::isnan(initialAge) || std::isinf(initialAge)) {
      JS_ReportErrorASCII(cx, "initialAge field is an invalid value, only positive numbers can be "
                              "used for initialAge values.");
      return JS::Result<host_api::CacheWriteOptions>(JS::Error());
    }
    // turn millisecond representation into nanosecond representation
    options.initial_age_ns = JS::ToUint64(initialAge) * 1'000'000;

    if (options.initial_age_ns > pow(2, 63)) {
      JS_ReportErrorASCII(cx, "initialAge can not be greater than 2^63.");
      return JS::Result<host_api::CacheWriteOptions>(JS::Error());
    }
  }

  JS::RootedValue staleWhileRevalidate_val(cx);
  if (!JS_GetProperty(cx, options_obj, "staleWhileRevalidate", &staleWhileRevalidate_val)) {
    return JS::Result<host_api::CacheWriteOptions>(JS::Error());
  }
  if (!staleWhileRevalidate_val.isUndefined()) {
    // Convert staleWhileRevalidate field into a number and check the value adheres to our
    // validation rules.
    double staleWhileRevalidate;
    if (!JS::ToNumber(cx, staleWhileRevalidate_val, &staleWhileRevalidate)) {
      return JS::Result<host_api::CacheWriteOptions>(JS::Error());
    }
    if (staleWhileRevalidate < 0 || std::isnan(staleWhileRevalidate) ||
        std::isinf(staleWhileRevalidate)) {
      JS_ReportErrorASCII(cx, "staleWhileRevalidate field is an invalid value, only positive "
                              "numbers can be used for staleWhileRevalidate values.");
      return JS::Result<host_api::CacheWriteOptions>(JS::Error());
    }
    // turn millisecond representation into nanosecond representation
    options.stale_while_revalidate_ns = JS::ToUint64(staleWhileRevalidate) * 1'000'000;

    if (options.initial_age_ns > pow(2, 63)) {
      JS_ReportErrorASCII(cx, "staleWhileRevalidate can not be greater than 2^63.");
      return JS::Result<host_api::CacheWriteOptions>(JS::Error());
    }
  }

  JS::RootedValue length_val(cx);
  if (!JS_GetProperty(cx, options_obj, "length", &length_val)) {
    return JS::Result<host_api::CacheWriteOptions>(JS::Error());
  }
  if (!length_val.isUndefined()) {
    // Convert length field into a number and check the value adheres to our
    // validation rules.
    double length;
    if (!JS::ToNumber(cx, length_val, &length)) {
      return JS::Result<host_api::CacheWriteOptions>(JS::Error());
    }
    if (length < 0 || std::isnan(length) || std::isinf(length)) {
      JS_ReportErrorASCII(
          cx,
          "length field is an invalid value, only positive numbers can be used for length values.");
      return JS::Result<host_api::CacheWriteOptions>(JS::Error());
    }
    options.length = JS::ToUint64(length);
  }

  JS::RootedValue vary_val(cx);
  if (!JS_GetProperty(cx, options_obj, "vary", &vary_val)) {
    return JS::Result<host_api::CacheWriteOptions>(JS::Error());
  }
  if (!vary_val.isUndefined()) {
    JS::ForOfIterator it(cx);
    if (!it.init(vary_val)) {
      return JS::Result<host_api::CacheWriteOptions>(JS::Error());
    }

    JS::RootedValue entry_val(cx);
    std::string varies;
    bool first = true;
    while (true) {
      bool done;
      if (!it.next(&entry_val, &done)) {
        return JS::Result<host_api::CacheWriteOptions>(JS::Error());
      }

      if (done) {
        break;
      }
      auto vary = core::encode(cx, entry_val);
      if (!vary) {
        return JS::Result<host_api::CacheWriteOptions>(JS::Error());
      }
      if (first) {
        first = false;
      } else {
        varies += " ";
      }
      varies += vary;
    }

    options.vary_rule = varies;
  }

  JS::RootedValue surrogateKeys_val(cx);
  if (!JS_GetProperty(cx, options_obj, "surrogateKeys", &surrogateKeys_val)) {
    return JS::Result<host_api::CacheWriteOptions>(JS::Error());
  }
  if (!surrogateKeys_val.isUndefined()) {
    JS::ForOfIterator it(cx);
    if (!it.init(surrogateKeys_val)) {
      return JS::Result<host_api::CacheWriteOptions>(JS::Error());
    }

    JS::RootedValue entry_val(cx);
    std::string surrogateKeys;
    bool first = true;
    while (true) {
      bool done;
      if (!it.next(&entry_val, &done)) {
        return JS::Result<host_api::CacheWriteOptions>(JS::Error());
      }

      if (done) {
        break;
      }
      auto skey = core::encode(cx, entry_val);
      if (!skey) {
        return JS::Result<host_api::CacheWriteOptions>(JS::Error());
      }
      if (first) {
        first = false;
      } else {
        surrogateKeys += " ";
      }
      surrogateKeys += skey;
    }

    options.surrogate_keys = surrogateKeys;
  }

  JS::RootedValue userMetadata_val(cx);
  if (!JS_GetProperty(cx, options_obj, "userMetadata", &userMetadata_val)) {
    return JS::Result<host_api::CacheWriteOptions>(JS::Error());
  }
  if (!userMetadata_val.isUndefined()) {
    auto result = convertBodyInit(cx, userMetadata_val);
    if (result.isErr()) {
      return JS::Result<host_api::CacheWriteOptions>(JS::Error());
    }
    size_t length;
    JS::UniqueChars data;
    std::tie(data, length) = result.unwrap();
    options.metadata = host_api::HostBytes(
        std::unique_ptr<uint8_t[]>(reinterpret_cast<uint8_t *>(std::move(data).release())), length);
  }

  return options;
}

// The JavaScript TransactionInsertOptions parameter we are parsing should have the below interface:
// interface TransactionInsertOptions {
//   maxAge: number,
//   vary?: Array<string>,
//   initialAge?: number,
//   staleWhileRevalidate?: number,
//   surrogateKeys?: Array<string>,
//   length?: number,
//   userMetadata?: ArrayBufferView | ArrayBuffer | URLSearchParams | string,
//   sensitive?: boolean,
// }
JS::Result<host_api::CacheWriteOptions> parseTransactionInsertOptions(JSContext *cx,
                                                                      JS::HandleValue options_val) {
  auto options_res = parseTransactionUpdateOptions(cx, options_val);
  if (options_res.isErr()) {
    return JS::Result<host_api::CacheWriteOptions>(JS::Error());
  }
  auto options = options_res.unwrap();
  JS::RootedObject options_obj(cx, &options_val.toObject());

  JS::RootedValue sensitive_val(cx);
  if (!JS_GetProperty(cx, options_obj, "sensitive", &sensitive_val)) {
    return JS::Result<host_api::CacheWriteOptions>(JS::Error());
  }
  options.sensitive = JS::ToBoolean(sensitive_val);

  return options;
}

// The JavaScript TransactionInsertOptions parameter we are parsing should have the below interface:
// interface InsertOptions extends TransactionInsertOptions {
//   headers?: HeadersInit,
// }
JS::Result<host_api::CacheWriteOptions> parseInsertOptions(JSContext *cx,
                                                           JS::HandleValue options_val) {
  auto options_res = parseTransactionInsertOptions(cx, options_val);
  if (options_res.isErr()) {
    return JS::Result<host_api::CacheWriteOptions>(JS::Error());
  }
  auto options = options_res.unwrap();
  JS::RootedObject options_obj(cx, &options_val.toObject());
  JS::RootedValue headers_val(cx);
  if (!JS_GetProperty(cx, options_obj, "headers", &headers_val)) {
    return JS::Result<host_api::CacheWriteOptions>(JS::Error());
  }
  // headers property is optional
  if (!headers_val.isUndefined()) {
    if (!headers_val.isObject()) {
      JS_ReportErrorASCII(
          cx, "Failed to construct Headers object. If defined, the first argument must be either "
              "a [ ['name', 'value'], ... ] sequence, or a { 'name' : 'value', ... } record.");
      return JS::Result<host_api::CacheWriteOptions>(JS::Error());
    }

    auto request_handle_res = host_api::HttpReq::make();
    if (auto *err = request_handle_res.to_err()) {
      HANDLE_ERROR(cx, *err);
      return JS::Result<host_api::CacheWriteOptions>(JS::Error());
    }
    auto request_handle = request_handle_res.unwrap();

    JS::RootedObject headers(cx, Headers::create(cx, headers_val, Headers::HeadersGuard::Request));
    if (!headers) {
      return JS::Result<host_api::CacheWriteOptions>(JS::Error());
    }
    if (Headers::mode(headers) == Headers::Mode::Uninitialized) {
      return options;
    }
    MOZ_ASSERT(Headers::mode(headers) == Headers::Mode::ContentOnly);
    Headers::HeadersList *headers_list = Headers::get_list(cx, headers);
    MOZ_ASSERT(headers_list);
    auto res = host_api::write_headers(request_handle.headers_writable(), *headers_list);
    if (res.is_err()) {
      return JS::Result<host_api::CacheWriteOptions>(JS::Error());
    }
    options.request_headers = host_api::HttpReq(request_handle);
  }
  return options;
}
} // namespace

// Below is the implementation of the JavaScript CacheState Class which has this definition:
// class CacheState {
//   found(): boolean;
//   usable(): boolean;
//   stale(): boolean;
//   mustInsertOrUpdate(): boolean;
// }

// found(): boolean;
bool CacheState::found(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0);
  auto state = static_cast<uint32_t>(
      JS::GetReservedSlot(self, static_cast<uint32_t>(Slots::State)).toInt32());

  args.rval().setBoolean(state & CacheState::found_flag);
  return true;
}

//   usable(): boolean;
bool CacheState::usable(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0);
  auto state = static_cast<uint32_t>(
      JS::GetReservedSlot(self, static_cast<uint32_t>(Slots::State)).toInt32());

  args.rval().setBoolean(state & CacheState::usable_flag);
  return true;
}

//   stale(): boolean;
bool CacheState::stale(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0);
  auto state = static_cast<uint32_t>(
      JS::GetReservedSlot(self, static_cast<uint32_t>(Slots::State)).toInt32());

  args.rval().setBoolean(state & CacheState::stale_flag);
  return true;
}

//   mustInsertOrUpdate(): boolean;
bool CacheState::mustInsertOrUpdate(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0);
  auto state = static_cast<uint32_t>(
      JS::GetReservedSlot(self, static_cast<uint32_t>(Slots::State)).toInt32());

  args.rval().setBoolean(state & CacheState::must_insert_or_update_flag);
  return true;
}

const JSFunctionSpec CacheState::static_methods[] = {JS_FS_END};

const JSPropertySpec CacheState::static_properties[] = {JS_PS_END};

const JSFunctionSpec CacheState::methods[] = {
    JS_FN("found", found, 0, JSPROP_ENUMERATE),
    JS_FN("usable", usable, 0, JSPROP_ENUMERATE),
    JS_FN("stale", stale, 0, JSPROP_ENUMERATE),
    JS_FN("mustInsertOrUpdate", mustInsertOrUpdate, 0, JSPROP_ENUMERATE),
    JS_FS_END,
};

const JSPropertySpec CacheState::properties[] = {
    JS_STRING_SYM_PS(toStringTag, "CacheState", JSPROP_READONLY), JS_PS_END};

JSObject *CacheState::create(JSContext *cx, uint32_t state) {
  JS::RootedObject instance(cx, JS_NewObjectWithGivenProto(cx, &class_, proto_obj));
  if (!instance) {
    return nullptr;
  }
  JS::SetReservedSlot(instance, static_cast<uint32_t>(Slots::State), JS::Int32Value(state));
  return instance;
}

// Below is the implementation of the JavaScript CacheEntry Class which has this definition:
// class CacheEntry {
//   close(): void;
//   state(): CacheState;
//   userMetadata(): ArrayBuffer;
//   body(options?: CacheBodyOptions): ReadableStream;
//   length(): number | null;
//   maxAge(): number;
//   staleWhileRevalidate(): number;
//   age(): number;
//   hits(): number;
// }

bool CacheEntry::is_instance(JSObject *obj) {
  return BuiltinImpl::is_instance(obj) || TransactionCacheEntry::is_instance(obj);
}

bool CacheEntry::is_instance(JS::Value val) {
  return val.isObject() && is_instance(&val.toObject());
}

host_api::CacheHandle CacheEntry::get_cache_handle(JSObject *self) {
  MOZ_ASSERT(CacheEntry::is_instance(self));
  host_api::CacheHandle handle{static_cast<uint32_t>(
      JS::GetReservedSlot(self, static_cast<uint32_t>(Slots::Handle)).toInt32())};
  return handle;
}

// close(): void;
bool CacheEntry::close(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  if (!check_receiver(cx, args.thisv(), "close")) {
    return false;
  }
  JS::RootedObject self(cx, &args.thisv().toObject());
  if (!args.requireAtLeast(cx, "close", 0)) {
    return false;
  }
  auto handle = CacheEntry::get_cache_handle(self);
  auto res = handle.close();
  if (auto *err = res.to_err()) {
    HANDLE_ERROR(cx, *err);
    return false;
  }
  args.rval().setUndefined();
  return true;
}

// state(): CacheState;
bool CacheEntry::state(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  if (!check_receiver(cx, args.thisv(), "state")) {
    return false;
  }
  JS::RootedObject self(cx, &args.thisv().toObject());
  if (!args.requireAtLeast(cx, "state", 0)) {
    return false;
  }
  auto handle = CacheEntry::get_cache_handle(self);
  auto res = handle.get_state();
  if (auto *err = res.to_err()) {
    HANDLE_ERROR(cx, *err);
    return false;
  }

  JS::RootedObject state(cx, CacheState::create(cx, res.unwrap().state));

  args.rval().setObjectOrNull(state);
  return true;
}

// userMetadata(): ArrayBuffer;
bool CacheEntry::userMetadata(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  if (!check_receiver(cx, args.thisv(), "userMetadata")) {
    return false;
  }
  JS::RootedObject self(cx, &args.thisv().toObject());
  if (!args.requireAtLeast(cx, "userMetadata", 0)) {
    return false;
  }
  auto handle = CacheEntry::get_cache_handle(self);
  auto res = handle.get_user_metadata();
  if (auto *err = res.to_err()) {
    HANDLE_ERROR(cx, *err);
    return false;
  }
  auto metadata = std::move(res.unwrap());
  JS::RootedObject array_buffer(cx);
  array_buffer.set(JS::NewArrayBufferWithContents(
      cx, metadata.len, metadata.ptr.get(), JS::NewArrayBufferOutOfMemory::CallerMustFreeMemory));
  if (!array_buffer) {
    JS_ReportOutOfMemory(cx);
    return false;
  }

  // `array_buffer` now owns `metadata`
  static_cast<void>(metadata.ptr.release());

  args.rval().setObject(*array_buffer);
  return true;
}

// body(options?: CacheBodyOptions): ReadableStream;
bool CacheEntry::body(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  if (!check_receiver(cx, args.thisv(), "body")) {
    return false;
  }
  JS::RootedObject self(cx, &args.thisv().toObject());
  if (!args.requireAtLeast(cx, "body", 0)) {
    return false;
  }
  auto handle = CacheEntry::get_cache_handle(self);

  host_api::CacheGetBodyOptions options;
  auto options_val = args.get(0);
  // options parameter is optional
  // options is meant to be an object with an optional `start` and `end` fields, both which can be
  // Numbers.
  if (!options_val.isUndefined()) {
    if (!options_val.isObject()) {
      JS_ReportErrorASCII(cx, "options argument must be an object");
      return false;
    }
    JS::RootedObject options_obj(cx, &options_val.toObject());

    JS::RootedValue start_val(cx);
    if (!JS_GetProperty(cx, options_obj, "start", &start_val)) {
      return false;
    }
    // start property is optional
    if (!start_val.isUndefined()) {
      // Convert start field into a number and check the value adheres to our
      // validation rules.
      double start;
      if (!JS::ToNumber(cx, start_val, &start)) {
        return false;
      }
      if (start < 0 || std::isnan(start) || std::isinf(start)) {
        JS_ReportErrorASCII(
            cx,
            "start field is an invalid value, only positive numbers can be used for start values.");
        return false;
      }
      options.start = JS::ToUint64(start);
    }

    JS::RootedValue end_val(cx);
    if (!JS_GetProperty(cx, options_obj, "end", &end_val)) {
      return false;
    }
    // end property is optional
    if (!end_val.isUndefined()) {
      // Convert start field into a number and check the value adheres to our
      // validation rules.
      double end;
      if (!JS::ToNumber(cx, end_val, &end)) {
        return false;
      }
      if (end < 0 || std::isnan(end) || std::isinf(end)) {
        JS_ReportErrorASCII(
            cx, "end field is an invalid value, only positive numbers can be used for end values.");
        return false;
      }
      options.end = JS::ToUint64(end);
    }

    // Reject cases where the start is greater than the end.
    // Ideally this would be a host-side check... but we didn't do it there to begin with,
    // so we couple it to an SDK/runtime upgrade.
    if (!start_val.isUndefined() && !end_val.isUndefined() && options.end > options.start) {
      JS_ReportErrorASCII(cx, "end field is before the start field");
      return false;
    }
  }

  auto res = handle.get_body(options);
  if (auto *err = res.to_err()) {
    HANDLE_ERROR(cx, *err);
    return false;
  }

  auto body = res.unwrap();
  JS::SetReservedSlot(self, static_cast<uint32_t>(Slots::Body), JS::Int32Value(body.handle));
  JS::SetReservedSlot(self, static_cast<uint32_t>(Slots::BodyStream), JS::NullValue());
  JS::SetReservedSlot(self, static_cast<uint32_t>(Slots::HasBody), JS::BooleanValue(true));
  JS::SetReservedSlot(self, static_cast<uint32_t>(Slots::BodyUsed), JS::FalseValue());

  JS::RootedObject source(
      cx, NativeStreamSource::create(cx, self, JS::UndefinedHandleValue,
                                     RequestOrResponse::body_source_pull_algorithm,
                                     RequestOrResponse::body_source_cancel_algorithm));
  if (!source) {
    return false;
  }

  // Create a readable stream with a highwater mark of 0.0 to prevent an eager
  // pull. With the default HWM of 1.0, the streams implementation causes a
  // pull, which means we enqueue a read from the host handle, which we quite
  // often have no interest in at all.
  JS::RootedObject body_stream(cx, NativeStreamSource::stream(source));
  if (!body_stream) {
    return false;
  }

  args.rval().setObject(*body_stream);

  return true;
}

// length(): number | null;
bool CacheEntry::length(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  if (!check_receiver(cx, args.thisv(), "length")) {
    return false;
  }
  JS::RootedObject self(cx, &args.thisv().toObject());
  if (!args.requireAtLeast(cx, "length", 0)) {
    return false;
  }
  auto handle = CacheEntry::get_cache_handle(self);
  auto res = handle.get_length();
  if (auto *err = res.to_err()) {
    if (host_api::error_is_optional_none(*err)) {
      args.rval().setNull();
      return true;
    }
    HANDLE_ERROR(cx, *err);
    return false;
  }
  auto length = res.unwrap();
  JS::RootedValue result(cx, JS::NumberValue(length));
  args.rval().set(result);
  return true;
}

// maxAge(): number;
bool CacheEntry::maxAge(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  if (!check_receiver(cx, args.thisv(), "maxAge")) {
    return false;
  }
  JS::RootedObject self(cx, &args.thisv().toObject());
  if (!args.requireAtLeast(cx, "maxAge", 0)) {
    return false;
  }
  auto handle = CacheEntry::get_cache_handle(self);
  auto res = handle.get_max_age_ns();
  if (auto *err = res.to_err()) {
    HANDLE_ERROR(cx, *err);
    return false;
  }
  auto age = res.unwrap();
  JS::RootedValue result(cx, JS::NumberValue(age / 1'000'000));
  args.rval().set(result);
  return true;
}

// staleWhileRevalidate(): number;
bool CacheEntry::staleWhileRevalidate(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  if (!check_receiver(cx, args.thisv(), "staleWhileRevalidate")) {
    return false;
  }
  JS::RootedObject self(cx, &args.thisv().toObject());
  if (!args.requireAtLeast(cx, "staleWhileRevalidate", 0)) {
    return false;
  }
  auto handle = CacheEntry::get_cache_handle(self);
  auto res = handle.get_stale_while_revalidate_ns();
  if (auto *err = res.to_err()) {
    HANDLE_ERROR(cx, *err);
    return false;
  }
  auto staleWhileRevalidateNs = res.unwrap();
  JS::RootedValue result(cx, JS::NumberValue(staleWhileRevalidateNs / 1'000'000));
  args.rval().set(result);
  return true;
}

// age(): number;
bool CacheEntry::age(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  if (!check_receiver(cx, args.thisv(), "age")) {
    return false;
  }
  JS::RootedObject self(cx, &args.thisv().toObject());
  if (!args.requireAtLeast(cx, "age", 0)) {
    return false;
  }
  auto handle = CacheEntry::get_cache_handle(self);
  auto res = handle.get_age_ns();
  if (auto *err = res.to_err()) {
    HANDLE_ERROR(cx, *err);
    return false;
  }
  auto age = res.unwrap();
  JS::RootedValue result(cx, JS::NumberValue(age / 1'000'000));
  args.rval().set(result);
  return true;
}

// hits(): number;
bool CacheEntry::hits(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  if (!check_receiver(cx, args.thisv(), "hits")) {
    return false;
  }
  JS::RootedObject self(cx, &args.thisv().toObject());
  if (!args.requireAtLeast(cx, "hits", 0)) {
    return false;
  }
  auto handle = CacheEntry::get_cache_handle(self);
  auto res = handle.get_hits();
  if (auto *err = res.to_err()) {
    HANDLE_ERROR(cx, *err);
    return false;
  }
  auto hits = res.unwrap();
  JS::RootedValue result(cx, JS::NumberValue(hits));
  args.rval().set(result);
  return true;
}

const JSFunctionSpec CacheEntry::static_methods[] = {JS_FS_END};

const JSPropertySpec CacheEntry::static_properties[] = {JS_PS_END};

const JSFunctionSpec CacheEntry::methods[] = {
    JS_FN("close", close, 0, JSPROP_ENUMERATE),
    JS_FN("state", state, 0, JSPROP_ENUMERATE),
    JS_FN("userMetadata", userMetadata, 0, JSPROP_ENUMERATE),
    JS_FN("body", body, 0, JSPROP_ENUMERATE),
    JS_FN("length", length, 0, JSPROP_ENUMERATE),
    JS_FN("maxAge", maxAge, 0, JSPROP_ENUMERATE),
    JS_FN("staleWhileRevalidate", staleWhileRevalidate, 0, JSPROP_ENUMERATE),
    JS_FN("age", age, 0, JSPROP_ENUMERATE),
    JS_FN("hits", hits, 0, JSPROP_ENUMERATE),
    JS_FS_END,
};

const JSPropertySpec CacheEntry::properties[] = {
    JS_STRING_SYM_PS(toStringTag, "CacheEntry", JSPROP_READONLY), JS_PS_END};

JSObject *CacheEntry::create(JSContext *cx, uint32_t handle) {
  JS::RootedObject instance(cx, JS_NewObjectWithGivenProto(cx, &class_, proto_obj));
  if (!instance) {
    return nullptr;
  }
  JS::SetReservedSlot(instance, static_cast<uint32_t>(Slots::Handle), JS::Int32Value(handle));
  return instance;
}

// Below is the implementation of the JavaScript TransactionCacheEntry Class which has this
// definition: class TransactionCacheEntry extends CacheEntry {
//   insert(options: TransactionInsertOptions): import("fastly:body").FastlyBody;
//   insertAndStreamBack(options: TransactionInsertOptions): [import("fastly:body").FastlyBody,
//   CacheEntry]; update(options: TransactionUpdateOptions): void; cancel(): void;
// }

// insert(options: TransactionInsertOptions): FastlyBody;
bool TransactionCacheEntry::insert(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(1);

  auto handle = CacheEntry::get_cache_handle(self);
  auto options_res = parseTransactionInsertOptions(cx, args.get(0));
  if (options_res.isErr()) {
    return false;
  }
  auto options = options_res.unwrap();
  auto res = handle.transaction_insert(options);

  if (auto *err = res.to_err()) {
    HANDLE_ERROR(cx, *err);
    return false;
  }
  auto body = res.unwrap();

  auto instance = FastlyBody::create(cx, body.handle);
  if (!instance) {
    return false;
  }

  args.rval().setObjectOrNull(instance);
  return true;
}

// insertAndStreamBack(options: TransactionInsertOptions): [FastlyBody, CacheEntry];
bool TransactionCacheEntry::insertAndStreamBack(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(1);

  auto options_res = parseTransactionInsertOptions(cx, args.get(0));
  if (options_res.isErr()) {
    return false;
  }
  auto options = options_res.unwrap();

  auto handle = CacheEntry::get_cache_handle(self);
  auto res = handle.transaction_insert_and_stream_back(options);

  if (auto *err = res.to_err()) {
    HANDLE_ERROR(cx, *err);
    return false;
  }
  host_api::HttpBody body;
  host_api::CacheHandle cache_handle;
  std::tie(body, cache_handle) = res.unwrap();

  JS::RootedValue writer(cx, JS::ObjectOrNullValue(FastlyBody::create(cx, body.handle)));

  JS::RootedValue reader(cx, JS::ObjectOrNullValue(CacheEntry::create(cx, cache_handle.handle)));

  JS::RootedValueVector result(cx);
  if (!result.append(writer)) {
    js::ReportOutOfMemory(cx);
    return false;
  }
  if (!result.append(reader)) {
    js::ReportOutOfMemory(cx);
    return false;
  }

  JS::Rooted<JSObject *> writer_and_reader(cx, JS::NewArrayObject(cx, result));
  if (!writer_and_reader) {
    return false;
  }

  args.rval().setObjectOrNull(writer_and_reader);
  return true;
}

// update(options: TransactionUpdateOptions): void;
bool TransactionCacheEntry::update(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(1);

  auto options_res = parseTransactionUpdateOptions(cx, args.get(0));
  if (options_res.isErr()) {
    return false;
  }
  auto options = options_res.unwrap();

  auto handle = CacheEntry::get_cache_handle(self);

  auto state_res = handle.get_state();
  if (auto *err = state_res.to_err()) {
    HANDLE_ERROR(cx, *err);
    return false;
  }
  auto state = state_res.unwrap();
  if (!state.is_found() || !state.must_insert_or_update()) {
    JS_ReportErrorASCII(cx,
                        "TransactionCacheEntry.update: entry does not exist or is not updatable");
    return false;
  }
  auto res = handle.transaction_update(options);

  if (auto *err = res.to_err()) {
    HANDLE_ERROR(cx, *err);
    return false;
  }

  args.rval().setUndefined();
  return true;
}

// cancel(): void;
bool TransactionCacheEntry::cancel(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0);

  auto handle = CacheEntry::get_cache_handle(self);
  auto res = handle.transaction_cancel();

  if (auto *err = res.to_err()) {
    HANDLE_ERROR(cx, *err);
    return false;
  }
  args.rval().setUndefined();
  return true;
}

const JSFunctionSpec TransactionCacheEntry::static_methods[] = {JS_FS_END};

const JSPropertySpec TransactionCacheEntry::static_properties[] = {JS_PS_END};

const JSFunctionSpec TransactionCacheEntry::methods[] = {
    JS_FN("insert", insert, 1, JSPROP_ENUMERATE),
    JS_FN("insertAndStreamBack", insertAndStreamBack, 1, JSPROP_ENUMERATE),
    JS_FN("update", update, 1, JSPROP_ENUMERATE),
    JS_FN("cancel", cancel, 0, JSPROP_ENUMERATE),
    JS_FS_END,
};

const JSPropertySpec TransactionCacheEntry::properties[] = {
    JS_STRING_SYM_PS(toStringTag, "TransactionCacheEntry", JSPROP_READONLY), JS_PS_END};

JSObject *TransactionCacheEntry::create(JSContext *cx, uint32_t handle) {
  JS::RootedObject instance(cx, JS_NewObjectWithGivenProto(cx, &class_, proto_obj));
  if (!instance) {
    return nullptr;
  }
  JS::SetReservedSlot(instance, static_cast<uint32_t>(Slots::Handle), JS::Int32Value(handle));
  return instance;
}

// Below is the implementation of the JavaScript CoreCache Class which has this definition:
// class CoreCache {
//   static lookup(key: string, options?: LookupOptions): CacheEntry | null;
//   static insert(key: string, options: InsertOptions): import("fastly:body").FastlyBody;
//   static transactionLookup(key: string, options?: LookupOptions): TransactionCacheEntry;
// }

// static lookup(key: string, options?: LookupOptions): CacheEntry | null;
bool CoreCache::lookup(JSContext *cx, unsigned argc, JS::Value *vp) {
  REQUEST_HANDLER_ONLY("The CoreCache builtin");
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  if (!args.requireAtLeast(cx, "CoreCache.lookup", 1)) {
    return false;
  }

  // Convert key parameter into a string and check the value adheres to our validation rules.
  auto key = core::encode(cx, args.get(0));
  if (!key) {
    return false;
  }

  if (key.len == 0) {
    JS_ReportErrorASCII(cx, "CoreCache.lookup: key can not be an empty string");
    return false;
  }
  if (key.len > 8135) {
    JS_ReportErrorASCII(cx,
                        "CoreCache.lookup: key is too long, the maximum allowed length is 8135.");
    return false;
  }

  auto options_result = parseLookupOptions(cx, args.get(1));
  if (options_result.isErr()) {
    return false;
  }
  auto options = options_result.unwrap();

  auto res = host_api::CacheHandle::lookup(key, options);
  if (auto *err = res.to_err()) {
    HANDLE_ERROR(cx, *err);
    return false;
  }
  auto cache_handle = res.unwrap();

  auto cache_state_res = cache_handle.get_state();
  if (auto *err = cache_state_res.to_err()) {
    HANDLE_ERROR(cx, *err);
    return false;
  }

  auto cache_state = cache_state_res.unwrap();

  if (cache_state.is_found()) {
    JS::RootedObject entry(cx, CacheEntry::create(cx, cache_handle.handle));
    args.rval().setObject(*entry);
  } else {
    args.rval().setNull();
  }
  return true;
}

// static insert(key: string, options: InsertOptions): FastlyBody;
bool CoreCache::insert(JSContext *cx, unsigned argc, JS::Value *vp) {
  REQUEST_HANDLER_ONLY("The CoreCache builtin");
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  if (!args.requireAtLeast(cx, "CoreCache.insert", 2)) {
    return false;
  }

  // Convert key parameter into a string and check the value adheres to our validation rules.
  auto key = core::encode(cx, args.get(0));
  if (!key) {
    return false;
  }

  if (key.len == 0) {
    JS_ReportErrorASCII(cx, "CoreCache.insert: key can not be an empty string");
    return false;
  }
  if (key.len > 8135) {
    JS_ReportErrorASCII(cx,
                        "CoreCache.insert: key is too long, the maximum allowed length is 8135.");
    return false;
  }

  auto options_res = parseInsertOptions(cx, args.get(1));
  if (options_res.isErr()) {
    return false;
  }
  auto options = options_res.unwrap();

  auto res = host_api::CacheHandle::insert(key, options);

  if (auto *err = res.to_err()) {
    HANDLE_ERROR(cx, *err);
    return false;
  }
  auto body = res.unwrap();

  JS::RootedObject instance(cx, FastlyBody::create(cx, body.handle));
  if (!instance) {
    return false;
  }

  args.rval().setObjectOrNull(instance);
  return true;
}

// static transactionLookup(key: string, options?: LookupOptions): TransactionCacheEntry;
bool CoreCache::transactionLookup(JSContext *cx, unsigned argc, JS::Value *vp) {
  REQUEST_HANDLER_ONLY("The CoreCache builtin");
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  if (!args.requireAtLeast(cx, "CoreCache.transactionLookup", 1)) {
    return false;
  }

  // Convert key parameter into a string and check the value adheres to our validation rules.
  auto key = core::encode(cx, args.get(0));
  if (!key) {
    return false;
  }

  if (key.len == 0) {
    JS_ReportErrorASCII(cx, "CoreCache.transactionLookup: key can not be an empty string");
    return false;
  }
  if (key.len > 8135) {
    JS_ReportErrorASCII(
        cx, "CoreCache.transactionLookup: key is too long, the maximum allowed length is 8135.");
    return false;
  }

  auto options_result = parseLookupOptions(cx, args.get(1));
  if (options_result.isErr()) {
    return false;
  }
  auto options = options_result.unwrap();

  auto res = host_api::CacheHandle::transaction_lookup(key, options);
  if (auto *err = res.to_err()) {
    HANDLE_ERROR(cx, *err);
    return false;
  }
  auto cache_handle = res.unwrap();

  JS::RootedObject entry(cx, TransactionCacheEntry::create(cx, cache_handle.handle));
  args.rval().setObject(*entry);
  return true;
}

const JSFunctionSpec CoreCache::static_methods[] = {
    JS_FN("lookup", lookup, 1, JSPROP_ENUMERATE),
    JS_FN("insert", insert, 2, JSPROP_ENUMERATE),
    JS_FN("transactionLookup", transactionLookup, 1, JSPROP_ENUMERATE),
    JS_FS_END,
};

const JSPropertySpec CoreCache::static_properties[] = {JS_PS_END};

const JSFunctionSpec CoreCache::methods[] = {JS_FS_END};

const JSPropertySpec CoreCache::properties[] = {
    JS_STRING_SYM_PS(toStringTag, "CoreCache", JSPROP_READONLY), JS_PS_END};

bool install(api::Engine *engine) {
  ENGINE = engine;
  if (!CoreCache::init_class_impl(engine->cx(), engine->global())) {
    return false;
  }
  if (!CacheEntry::init_class_impl(engine->cx(), engine->global())) {
    return false;
  }
  JS::RootedObject proto(engine->cx(), CacheEntry::proto_obj);
  if (!proto) {
    return false;
  }
  if (!TransactionCacheEntry::init_class_impl(engine->cx(), engine->global(), proto)) {
    return false;
  }
  if (!CacheState::init_class_impl(engine->cx(), engine->global())) {
    return false;
  }

  // fastly:cache
  RootedObject cache(engine->cx(), JS_NewObject(engine->cx(), nullptr));
  RootedValue cache_val(engine->cx(), JS::ObjectValue(*cache));
  RootedObject core_cache_obj(engine->cx(), JS_GetConstructor(engine->cx(), CoreCache::proto_obj));
  RootedValue core_cache_val(engine->cx(), ObjectValue(*core_cache_obj));
  if (!JS_SetProperty(engine->cx(), cache, "CoreCache", core_cache_val)) {
    return false;
  }
  RootedObject cache_entry_obj(engine->cx(),
                               JS_GetConstructor(engine->cx(), CacheEntry::proto_obj));
  RootedValue cache_entry_val(engine->cx(), ObjectValue(*cache_entry_obj));
  if (!JS_SetProperty(engine->cx(), cache, "CacheEntry", cache_entry_val)) {
    return false;
  }
  RootedObject cache_state_obj(engine->cx(),
                               JS_GetConstructor(engine->cx(), CacheState::proto_obj));
  RootedValue cache_state_val(engine->cx(), ObjectValue(*cache_state_obj));
  if (!JS_SetProperty(engine->cx(), cache, "CacheState", cache_state_val)) {
    return false;
  }
  RootedObject transaction_cache_entry_obj(
      engine->cx(), JS_GetConstructor(engine->cx(), TransactionCacheEntry::proto_obj));
  RootedValue transaction_cache_entry_val(engine->cx(), ObjectValue(*transaction_cache_entry_obj));
  if (!JS_SetProperty(engine->cx(), cache, "TransactionCacheEntry", transaction_cache_entry_val)) {
    return false;
  }
  RootedValue simple_cache_val(engine->cx());
  if (!JS_GetProperty(engine->cx(), engine->global(), "SimpleCache", &simple_cache_val)) {
    return false;
  }
  if (!JS_SetProperty(engine->cx(), cache, "SimpleCache", simple_cache_val)) {
    return false;
  }
  RootedValue simple_cache_entry_val(engine->cx());
  if (!JS_GetProperty(engine->cx(), engine->global(), "SimpleCacheEntry",
                      &simple_cache_entry_val)) {
    return false;
  }
  if (!JS_SetProperty(engine->cx(), cache, "SimpleCacheEntry", simple_cache_entry_val)) {
    return false;
  }
  if (!engine->define_builtin_module("fastly:cache", cache_val)) {
    return false;
  }
  return true;
}

} // namespace fastly::cache_core
