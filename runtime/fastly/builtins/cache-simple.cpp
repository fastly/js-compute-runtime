#include "cache-simple.h"
#include "../../StarlingMonkey/builtins/web/streams/native-stream-source.h"
#include "../../StarlingMonkey/builtins/web/url.h"
#include "../../StarlingMonkey/runtime/encode.h"
#include "../host-api/host_api_fastly.h"
#include "builtin.h"
#include "fastly.h"
#include "js/ArrayBuffer.h"
#include "js/Result.h"
#include "js/Stream.h"
#include "openssl/evp.h"
#include <tuple>

using builtins::BuiltinNoConstructor;
using builtins::web::streams::NativeStreamSource;
using fastly::fastly::convertBodyInit;
using fastly::fastly::FastlyGetErrorMessage;
using fastly::fetch::RequestOrResponse;

namespace fastly::cache_simple {

template <RequestOrResponse::BodyReadResult result_type>
bool SimpleCacheEntry::bodyAll(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0);
  return RequestOrResponse::bodyAll<result_type>(cx, args, self);
}

bool SimpleCacheEntry::body_get(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0);
  if (!JS::GetReservedSlot(self, static_cast<uint32_t>(Slots::HasBody)).isBoolean()) {
    JS::SetReservedSlot(self, static_cast<uint32_t>(Slots::HasBody), JS::BooleanValue(false));
  }
  return RequestOrResponse::body_get(cx, args, self, true);
}

bool SimpleCacheEntry::bodyUsed_get(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0);
  if (!JS::GetReservedSlot(self, static_cast<uint32_t>(Slots::BodyUsed)).isBoolean()) {
    JS::SetReservedSlot(self, static_cast<uint32_t>(Slots::BodyUsed), JS::BooleanValue(false));
  }
  args.rval().setBoolean(RequestOrResponse::body_used(self));
  return true;
}

const JSFunctionSpec SimpleCacheEntry::static_methods[] = {
    JS_FS_END,
};

const JSPropertySpec SimpleCacheEntry::static_properties[] = {
    JS_PS_END,
};

const JSFunctionSpec SimpleCacheEntry::methods[] = {
    JS_FN("arrayBuffer", bodyAll<RequestOrResponse::BodyReadResult::ArrayBuffer>, 0,
          JSPROP_ENUMERATE),
    JS_FN("json", bodyAll<RequestOrResponse::BodyReadResult::JSON>, 0, JSPROP_ENUMERATE),
    JS_FN("text", bodyAll<RequestOrResponse::BodyReadResult::Text>, 0, JSPROP_ENUMERATE),
    JS_FS_END,
};

const JSPropertySpec SimpleCacheEntry::properties[] = {
    JS_PSG("body", body_get, JSPROP_ENUMERATE),
    JS_PSG("bodyUsed", bodyUsed_get, JSPROP_ENUMERATE),
    JS_STRING_SYM_PS(toStringTag, "SimpleCacheEntry", JSPROP_READONLY),
    JS_PS_END,
};

JSObject *SimpleCacheEntry::create(JSContext *cx, host_api::HttpBody body_handle) {
  JS::RootedObject SimpleCacheEntry(cx, JS_NewObjectWithGivenProto(cx, &class_, proto_obj));
  if (!SimpleCacheEntry)
    return nullptr;

  JS::SetReservedSlot(SimpleCacheEntry, static_cast<uint32_t>(Slots::Body),
                      JS::Int32Value(body_handle.handle));
  JS::SetReservedSlot(SimpleCacheEntry, static_cast<uint32_t>(Slots::BodyStream), JS::NullValue());
  JS::SetReservedSlot(SimpleCacheEntry, static_cast<uint32_t>(Slots::HasBody),
                      JS::BooleanValue(true));
  JS::SetReservedSlot(SimpleCacheEntry, static_cast<uint32_t>(Slots::BodyUsed), JS::FalseValue());

  return SimpleCacheEntry;
}

namespace {
// Purging/Deleting a cache item within the Compute SDKs via a hostcall is only
// possible via surrogate-keys. We add a surrogate key to all the cache entries,
// which is the sha-256 digest of the cache entries cache-key, converted to
// uppercase hexadecimal.
// Note: We should keep this consistent across the Compute SDKs, this would allow
// a Compute Service to move from one SDK to another, and have consistent purging
// behavior between the Compute Service Versions which were using a different SDK.
JS::Result<std::string> createGlobalSurrogateKeyFromCacheKey(JSContext *cx,
                                                             std::string_view cache_key) {
  const EVP_MD *algorithm = EVP_sha256();
  unsigned int size = EVP_MD_size(algorithm);
  std::vector<unsigned char> md(size);

  if (!EVP_Digest(cache_key.data(), cache_key.size(), md.data(), &size, algorithm, nullptr)) {
    return JS::Result<std::string>(JS::Error());
  }
  JS::UniqueChars data{OPENSSL_buf2hexstr(md.data(), size)};
  std::string surrogate_key{data.get(), std::remove(data.get(), data.get() + size, ':')};

  return JS::Result<std::string>(surrogate_key);
}

// Purging/Deleting a cache item within the Compute SDKs via a hostcall is only
// possible via surrogate-keys. We add a surrogate key to all the cache entries,
// which is the sha-256 digest of the cache entries cache-key and the FASTLY_POP
// environment variable, converted to uppercase hexadecimal.
// Note: We should keep this consistent across the Compute SDKs, this would allow
// a Compute Service to move from one SDK to another, and have consistent purging
// behavior between the Compute Service Versions which were using a different SDK.
JS::Result<std::string> createPopSurrogateKeyFromCacheKey(JSContext *cx,
                                                          std::string_view cache_key) {
  const EVP_MD *algorithm = EVP_sha256();
  unsigned int size = EVP_MD_size(algorithm);
  std::vector<unsigned char> md(size);

  std::string key{cache_key};
  auto pop = getenv("FASTLY_POP");
  if (pop) {
    key += pop;
  }

  // TODO: use the incremental Digest api instead of allocating a string
  if (!EVP_Digest(key.c_str(), key.length(), md.data(), &size, algorithm, nullptr)) {
    return JS::Result<std::string>(JS::Error());
  }
  JS::UniqueChars data{OPENSSL_buf2hexstr(md.data(), size)};
  std::string surrogate_key{data.get(), std::remove(data.get(), data.get() + size, ':')};

  return JS::Result<std::string>(surrogate_key);
}

// Create all the surrogate keys for the cache key
JS::Result<std::string> createSurrogateKeysFromCacheKey(JSContext *cx, std::string_view cache_key) {
  const EVP_MD *algorithm = EVP_sha256();
  unsigned int size = EVP_MD_size(algorithm);
  std::vector<unsigned char> md(size);

  if (!EVP_Digest(cache_key.data(), cache_key.size(), md.data(), &size, algorithm, nullptr)) {
    return JS::Result<std::string>(JS::Error());
  }
  JS::UniqueChars data{OPENSSL_buf2hexstr(md.data(), size)};
  std::string surrogate_keys{data.get(), std::remove(data.get(), data.get() + size, ':')};

  if (auto *pop = getenv("FASTLY_POP")) {
    // TODO: use the incremental Digest api instead of allocating a string
    std::string key{cache_key};
    key += pop;
    if (!EVP_Digest(key.c_str(), key.length(), md.data(), &size, algorithm, nullptr)) {
      return JS::Result<std::string>(JS::Error());
    }
    JS::UniqueChars data{OPENSSL_buf2hexstr(md.data(), size)};
    surrogate_keys.push_back(' ');
    surrogate_keys.append(data.get(), std::remove(data.get(), data.get() + size, ':'));
  }

  return JS::Result<std::string>(surrogate_keys);
}

#define BEGIN_TRANSACTION(t, cx, promise, handle)                                                  \
  CacheTransaction t{cx, promise, handle, __func__, __LINE__};

class CacheTransaction final {
  JSContext *cx;
  JS::RootedObject promise;
  host_api::CacheHandle handle;

  const char *func;
  int line;

public:
  CacheTransaction(JSContext *cx, JS::HandleObject promise, host_api::CacheHandle handle,
                   const char *func, const int line)
      : cx{cx}, promise{this->cx, promise}, handle{handle}, func{func}, line{line} {};

  ~CacheTransaction() {
    // An invalid handle indicates that this transaction has been committed.
    if (!this->handle.is_valid()) {
      return;
    }

    auto res = this->handle.transaction_cancel();
    if (auto *err = res.to_err()) {
      host_api::handle_fastly_error(this->cx, *err, this->line, this->func);
    }

    // We always reject the promise if the transaction hasn't committed.
    RejectPromiseWithPendingError(this->cx, this->promise);
  }

  /// Commit this transaction.
  void commit() {
    // Invalidate the handle to indicate that the transaction has been committed.
    MOZ_ASSERT(this->handle.is_valid());
    this->handle = host_api::CacheHandle{};
    MOZ_ASSERT(!this->handle.is_valid());
  }
};

} // namespace

bool SimpleCache::getOrSetThenHandler(JSContext *cx, JS::HandleObject owner, JS::HandleValue extra,
                                      JS::CallArgs args) {
  MOZ_ASSERT(extra.isObject());
  JS::RootedObject extraObj(cx, &extra.toObject());
  JS::RootedValue handleVal(cx);
  JS::RootedValue promiseVal(cx);
  if (!JS_GetProperty(cx, extraObj, "promise", &promiseVal)) {
    return false;
  }
  MOZ_ASSERT(promiseVal.isObject());
  JS::RootedObject promise(cx, &promiseVal.toObject());
  if (!promise) {
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }

  if (!JS_GetProperty(cx, extraObj, "handle", &handleVal)) {
    return RejectPromiseWithPendingError(cx, promise);
  }
  MOZ_ASSERT(handleVal.isInt32());

  host_api::CacheHandle handle(handleVal.toInt32());

  BEGIN_TRANSACTION(transaction, cx, promise, handle);

  JS::RootedValue keyVal(cx);
  if (!JS_GetProperty(cx, extraObj, "key", &keyVal)) {
    return false;
  }

  auto arg0 = args.get(0);
  if (!arg0.isObject()) {
    JS_ReportErrorASCII(cx, "SimpleCache.getOrSet: does not adhere to interface {value: BodyInit,  "
                            "ttl: number, length?:number}");
    return false;
  }
  JS::RootedObject insertionObject(cx, &arg0.toObject());

  JS::RootedValue ttl_val(cx);
  if (!JS_GetProperty(cx, insertionObject, "ttl", &ttl_val)) {
    return false;
  }
  // Convert ttl (time-to-live) field into a number and check the value adheres to our
  // validation rules.
  double ttl;
  if (!JS::ToNumber(cx, ttl_val, &ttl)) {
    return false;
  }
  if (ttl < 0 || std::isnan(ttl) || std::isinf(ttl)) {
    JS_ReportErrorASCII(
        cx, "SimpleCache.getOrSet: TTL field is an invalid value, only positive numbers can "
            "be used for TTL values.");
    return false;
  }
  host_api::CacheWriteOptions options;
  // turn second representation into nanosecond representation
  options.max_age_ns = JS::ToUint64(ttl) * 1'000'000'000;

  JS::RootedValue body_val(cx);
  if (!JS_GetProperty(cx, insertionObject, "value", &body_val)) {
    return false;
  }

  host_api::HttpBody source_body;
  JS::UniqueChars buf;
  JS::RootedObject body_obj(cx, body_val.isObject() ? &body_val.toObject() : nullptr);
  // If the body is a Host-backed ReadableStream we optimise our implementation
  // by using the ReadableStream's handle directly.
  if (body_obj && JS::IsReadableStream(body_obj)) {
    if (RequestOrResponse::body_unusable(cx, body_obj)) {
      JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr,
                                JSMSG_READABLE_STREAM_LOCKED_OR_DISTRUBED);
      return false;
    }

    // If the stream is backed by a Fastly Compute body handle, we can use that handle directly.
    if (NativeStreamSource::stream_is_body(cx, body_obj)) {
      JS::RootedObject stream_source(cx, NativeStreamSource::get_stream_source(cx, body_obj));
      JS::RootedObject source_owner(cx, NativeStreamSource::owner(stream_source));
      source_body = RequestOrResponse::body_handle(source_owner);
    } else {
      JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr,
                                JSMSG_SIMPLE_CACHE_SET_CONTENT_STREAM);
      return false;
    }

    // The cache APIs require the length to be known upfront, we don't know the length of a
    // stream upfront, which means the caller will need to supply the information explicitly for us.
    bool found;
    if (!JS_HasProperty(cx, insertionObject, "length", &found)) {
      return false;
    }
    if (found) {

      JS::RootedValue length_val(cx);
      if (!JS_GetProperty(cx, insertionObject, "length", &length_val)) {
        return false;
      }
      double number;
      if (!JS::ToNumber(cx, length_val, &number)) {
        return false;
      }
      if (number < 0 || std::isnan(number) || std::isinf(number)) {
        JS_ReportErrorASCII(
            cx,
            "SimpleCache.getOrSet: length property is an invalid value, only positive numbers can "
            "be used for length values.");
        return false;
      }
      options.length = JS::ToInteger(number);
    }
  } else {
    auto result = convertBodyInit(cx, body_val);
    if (result.isErr()) {
      return false;
    }
    std::tie(buf, options.length) = result.unwrap();
  }

  // We create a surrogate-key from the cache-key, as this allows the cached contents to be purgable
  // from within the JavaScript application
  // This is because the cache API currently only supports purging via surrogate-key
  auto key_chars = core::encode(cx, keyVal);
  if (!key_chars) {
    return false;
  }
  auto key_result = createSurrogateKeysFromCacheKey(cx, key_chars);
  if (key_result.isErr()) {
    return false;
  }
  options.surrogate_keys = key_result.inspect();

  auto inserted_res = handle.transaction_insert_and_stream_back(options);
  if (auto *err = inserted_res.to_err()) {
    return false;
  }

  auto [body, inserted_handle] = inserted_res.unwrap();
  if (!body.valid()) {
    return false;
  }
  // source_body will only be valid when the body is a Host-backed ReadableStream
  if (source_body.valid()) {
    auto res = body.append(source_body);
    if (auto *error = res.to_err()) {
      return false;
    }
  } else {
    auto write_res = body.write_all_back(reinterpret_cast<uint8_t *>(buf.get()), options.length);
    if (auto *error = write_res.to_err()) {
      return false;
    }
    auto close_res = body.close();
    if (auto *error = close_res.to_err()) {
      return false;
    }
  }

  auto res = inserted_handle.get_body(host_api::CacheGetBodyOptions{});
  if (auto *err = res.to_err()) {
    return false;
  }

  JS::RootedObject entry(cx, SimpleCacheEntry::create(cx, res.unwrap()));
  if (!entry) {
    return false;
  }

  transaction.commit();

  JS::RootedValue result(cx);
  result.setObject(*entry);
  JS::ResolvePromise(cx, promise, result);
  return true;
}

// static getOrSet(key: string, set: () => Promise<{value: BodyInit,  ttl: number}>):
// SimpleCacheEntry | null; static getOrSet(key: string, set: () => Promise<{value: ReadableStream,
// ttl: number, length: number}>): SimpleCacheEntry | null;
bool SimpleCache::getOrSet(JSContext *cx, unsigned argc, JS::Value *vp) {
  REQUEST_HANDLER_ONLY("The SimpleCache builtin");
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  if (!args.requireAtLeast(cx, "SimpleCache.getOrSet", 2)) {
    return false;
  }

  // Convert key parameter into a string and check the value adheres to our validation rules.
  auto key_chars = core::encode(cx, args.get(0));
  if (!key_chars) {
    return false;
  }

  if (key_chars.len == 0) {
    JS_ReportErrorASCII(cx, "SimpleCache.getOrSet: key can not be an empty string");
    return false;
  }
  if (key_chars.len > 8135) {
    JS_ReportErrorASCII(
        cx, "SimpleCache.getOrSet: key is too long, the maximum allowed length is 8135.");
    return false;
  }

  JS::RootedObject promise(cx, JS::NewPromiseObject(cx, nullptr));
  if (!promise) {
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }

  auto res = host_api::CacheHandle::transaction_lookup(key_chars, host_api::CacheLookupOptions{});
  if (auto *err = res.to_err()) {
    HANDLE_ERROR(cx, *err);
    return false;
  }

  auto handle = res.unwrap();
  BEGIN_TRANSACTION(transaction, cx, promise, handle);

  // Check if a fresh cache item was found, if that's the case, then we will resolve
  // with a SimpleCacheEntry containing the value. Else, call the content-provided
  // function in the `set` parameter and insert it's returned value property into the
  // cache under the provided `key`, and then we will resolve with a SimpleCacheEntry
  // containing the value.
  auto state_res = handle.get_state();
  if (auto *err = state_res.to_err()) {
    return false;
  }

  auto state = state_res.unwrap();
  args.rval().setObject(*promise);
  if (state.is_usable()) {
    auto body_res = handle.get_body(host_api::CacheGetBodyOptions{});
    if (auto *err = body_res.to_err()) {
      return false;
    }

    JS::RootedObject entry(cx, SimpleCacheEntry::create(cx, body_res.unwrap()));
    if (!entry) {
      return false;
    }

    JS::RootedValue result(cx);
    result.setObject(*entry);
    JS::ResolvePromise(cx, promise, result);
    return true;
  } else {
    auto arg1 = args.get(1);
    if (!arg1.isObject() || !JS::IsCallable(&arg1.toObject())) {
      JS_ReportErrorLatin1(cx, "SimpleCache.getOrSet: set argument is not a function");
      return false;
    }
    JS::RootedValueArray<0> fnargs(cx);
    JS::RootedObject fn(cx, &arg1.toObject());
    JS::RootedValue result(cx);
    if (!JS::Call(cx, JS::NullHandleValue, fn, fnargs, &result)) {
      return false;
    }
    // Coercion of `result` to a Promise<typeof result>
    JS::RootedObject result_promise(cx, JS::CallOriginalPromiseResolve(cx, result));
    if (!result_promise) {
      return false;
    }

    // JS::RootedObject owner(cx, JS_NewPlainObject(cx));
    JS::RootedObject extraObj(cx, JS_NewPlainObject(cx));
    JS::RootedValue handleVal(cx, JS::NumberValue(handle.handle));
    if (!JS_SetProperty(cx, extraObj, "handle", handleVal)) {
      return false;
    }
    JS::RootedValue keyVal(
        cx, JS::StringValue(JS_NewStringCopyN(cx, key_chars.begin(), key_chars.len)));
    if (!JS_SetProperty(cx, extraObj, "key", keyVal)) {
      return false;
    }
    JS::RootedValue promiseVal(cx, JS::ObjectValue(*promise));
    if (!JS_SetProperty(cx, extraObj, "promise", promiseVal)) {
      return false;
    }

    JS::RootedValue extra(cx, JS::ObjectValue(*extraObj));
    JS::RootedObject global(cx, JS::CurrentGlobalOrNull(cx));
    JS::RootedObject then_handler(cx,
                                  create_internal_method<getOrSetThenHandler>(cx, global, extra));
    if (!then_handler) {
      return false;
    }
    if (!JS::AddPromiseReactions(cx, result_promise, then_handler, nullptr)) {
      return false;
    }
    transaction.commit();
    return true;
  }
}

// static set(key: string, value: BodyInit, ttl: number): undefined;
// static set(key: string, value: ReadableStream, ttl: number, length: number): undefined;
bool SimpleCache::set(JSContext *cx, unsigned argc, JS::Value *vp) {
  REQUEST_HANDLER_ONLY("The SimpleCache builtin");
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  if (!args.requireAtLeast(cx, "SimpleCache.set", 3)) {
    return false;
  }

  // Convert key parameter into a string and check the value adheres to our validation rules.
  auto key = core::encode(cx, args.get(0));
  if (!key) {
    return false;
  }

  if (key.len == 0) {
    JS_ReportErrorASCII(cx, "SimpleCache.set: key can not be an empty string");
    return false;
  }
  if (key.len > 8135) {
    JS_ReportErrorASCII(cx,
                        "SimpleCache.set: key is too long, the maximum allowed length is 8135.");
    return false;
  }

  host_api::CacheWriteOptions options;
  // Convert ttl (time-to-live) parameter into a number and check the value adheres to our
  // validation rules.
  JS::HandleValue ttl_val = args.get(2);
  double ttl;
  if (!JS::ToNumber(cx, ttl_val, &ttl)) {
    return false;
  }
  if (ttl < 0 || std::isnan(ttl) || std::isinf(ttl)) {
    JS_ReportErrorASCII(
        cx, "SimpleCache.set: TTL parameter is an invalid value, only positive numbers can "
            "be used for TTL values.");
    return false;
  }
  options.max_age_ns = JS::ToUint64(ttl) *
                       1'000'000'000; // turn second representation into nanosecond representation

  JS::HandleValue body_val = args.get(1);
  host_api::HttpBody source_body;
  JS::UniqueChars buf;
  JS::RootedObject body_obj(cx, body_val.isObject() ? &body_val.toObject() : nullptr);
  // If the body parameter is a Host-backed ReadableStream we optimise our implementation
  // by using the ReadableStream's handle directly.
  if (body_obj && JS::IsReadableStream(body_obj)) {
    if (RequestOrResponse::body_unusable(cx, body_obj)) {
      JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr,
                                JSMSG_READABLE_STREAM_LOCKED_OR_DISTRUBED);
      return false;
    }

    // If the stream is backed by a Fastly Compute body handle, we can use that handle directly.
    if (NativeStreamSource::stream_is_body(cx, body_obj)) {
      JS::RootedObject stream_source(cx, NativeStreamSource::get_stream_source(cx, body_obj));
      JS::RootedObject source_owner(cx, NativeStreamSource::owner(stream_source));
      source_body = RequestOrResponse::body_handle(source_owner);
    } else {
      JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr,
                                JSMSG_SIMPLE_CACHE_SET_CONTENT_STREAM);
      return false;
    }

    if (args.hasDefined(3)) {
      JS::HandleValue length_val = args.get(3);
      double number;
      if (!JS::ToNumber(cx, length_val, &number)) {
        return false;
      }
      if (number < 0 || std::isnan(number) || std::isinf(number)) {
        JS_ReportErrorASCII(
            cx, "SimpleCache.set: length parameter is an invalid value, only positive numbers can "
                "be used for length values.");
        return false;
      }
      options.length = JS::ToInteger(number);
    }
  } else {
    auto result = convertBodyInit(cx, body_val);
    if (result.isErr()) {
      return false;
    }
    std::tie(buf, options.length) = result.unwrap();
  }

  // We create a surrogate-key from the cache-key, as this allows the cached contents to be purgable
  // from within the JavaScript application
  // This is because the cache API currently only supports purging via surrogate-key
  auto key_result = createSurrogateKeysFromCacheKey(cx, key);
  if (key_result.isErr()) {
    return false;
  }
  options.surrogate_keys = key_result.inspect();

  auto insert_res = host_api::CacheHandle::insert(key, options);
  if (auto *err = insert_res.to_err()) {
    HANDLE_ERROR(cx, *err);
    return false;
  }

  auto body = insert_res.unwrap();
  if (!body.valid()) {
    return false;
  }
  // source_body will only be valid when the body parameter is a Host-backed ReadableStream
  if (source_body.valid()) {
    auto res = body.append(source_body);
    if (auto *err = res.to_err()) {
      HANDLE_ERROR(cx, *err);
      return false;
    }
    args.rval().setUndefined();
    return true;
  } else {
    auto write_res = body.write_all_back(reinterpret_cast<uint8_t *>(buf.get()), options.length);
    if (auto *err = write_res.to_err()) {
      HANDLE_ERROR(cx, *err);
      return false;
    }
  }
  auto close_res = body.close();
  if (auto *err = close_res.to_err()) {
    HANDLE_ERROR(cx, *err);
    return false;
  }

  args.rval().setUndefined();
  return true;
}

// static get(key: string): SimpleCacheEntry | null;
bool SimpleCache::get(JSContext *cx, unsigned argc, JS::Value *vp) {
  REQUEST_HANDLER_ONLY("The SimpleCache builtin");
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  if (!args.requireAtLeast(cx, "SimpleCache.get", 1)) {
    return false;
  }

  // Convert key parameter into a string and check the value adheres to our validation rules.
  auto key = core::encode(cx, args[0]);
  if (!key) {
    return false;
  }

  if (key.len == 0) {
    JS_ReportErrorASCII(cx, "SimpleCache.get: key can not be an empty string");
    return false;
  }
  if (key.len > 8135) {
    JS_ReportErrorASCII(cx,
                        "SimpleCache.get: key is too long, the maximum allowed length is 8135.");
    return false;
  }

  auto lookup_res = host_api::CacheHandle::lookup(key, host_api::CacheLookupOptions{});
  if (auto *err = lookup_res.to_err()) {
    HANDLE_ERROR(cx, *err);
    return false;
  }
  auto handle = lookup_res.unwrap();

  auto body_res = handle.get_body(host_api::CacheGetBodyOptions{});
  if (auto *err = body_res.to_err()) {
    HANDLE_ERROR(cx, *err);
    return false;
  }
  auto body = body_res.unwrap();

  if (!body.valid()) {
    args.rval().setNull();
  } else {
    JS::RootedObject entry(cx, SimpleCacheEntry::create(cx, body));
    if (!entry) {
      return false;
    }
    args.rval().setObject(*entry);
  }

  return true;
}

// static purge(key: string, options: PurgeOptions): undefined;
bool SimpleCache::purge(JSContext *cx, unsigned argc, JS::Value *vp) {
  REQUEST_HANDLER_ONLY("The SimpleCache builtin");
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  if (!args.requireAtLeast(cx, "SimpleCache.purge", 2)) {
    return false;
  }

  // Convert key parameter into a string and check the value adheres to our validation rules.
  auto key_chars = core::encode(cx, args.get(0));
  if (!key_chars) {
    return false;
  }

  if (key_chars.len == 0) {
    JS_ReportErrorASCII(cx, "SimpleCache.purge: key can not be an empty string");
    return false;
  }
  if (key_chars.len > 8135) {
    JS_ReportErrorASCII(cx,
                        "SimpleCache.purge: key is too long, the maximum allowed length is 8135.");
    return false;
  }

  auto secondArgument = args.get(1);
  if (!secondArgument.isObject()) {
    JS_ReportErrorASCII(cx, "SimpleCache.purge: options parameter is not an object.");
    return false;
  }

  JS::RootedObject options(cx, &secondArgument.toObject());
  JS::RootedValue scope_val(cx);
  if (!JS_GetProperty(cx, options, "scope", &scope_val)) {
    return false;
  }
  auto scope_chars = core::encode(cx, scope_val);
  if (!scope_chars) {
    return false;
  }

  std::string_view scope = scope_chars;
  std::string surrogate_key;
  if (scope == "pop") {
    auto surrogate_key_result = createPopSurrogateKeyFromCacheKey(cx, key_chars);
    if (surrogate_key_result.isErr()) {
      return false;
    }
    surrogate_key = surrogate_key_result.unwrap();
  } else if (scope == "global") {
    auto surrogate_key_result = createGlobalSurrogateKeyFromCacheKey(cx, key_chars);
    if (surrogate_key_result.isErr()) {
      return false;
    }
    surrogate_key = surrogate_key_result.unwrap();
  } else {
    JS_ReportErrorASCII(
        cx,
        "SimpleCache.purge: scope field of options parameter must be either 'pop', or 'global'.");
    return false;
  }

  auto purge_res = host_api::Fastly::purge_surrogate_key(surrogate_key);
  if (auto *err = purge_res.to_err()) {
    HANDLE_ERROR(cx, *err);
    return false;
  }
  MOZ_ASSERT(!purge_res.unwrap().has_value());

  args.rval().setUndefined();
  return true;
}

const JSFunctionSpec SimpleCache::static_methods[] = {
    JS_FN("purge", purge, 2, JSPROP_ENUMERATE),
    JS_FN("get", get, 1, JSPROP_ENUMERATE),
    JS_FN("getOrSet", getOrSet, 2, JSPROP_ENUMERATE),
    JS_FN("set", set, 3, JSPROP_ENUMERATE),
    JS_FS_END,
};

const JSPropertySpec SimpleCache::static_properties[] = {
    JS_PS_END,
};

const JSFunctionSpec SimpleCache::methods[] = {JS_FS_END};

const JSPropertySpec SimpleCache::properties[] = {
    JS_STRING_SYM_PS(toStringTag, "SimpleCache", JSPROP_READONLY), JS_PS_END};

bool install(api::Engine *engine) {
  if (!BuiltinNoConstructor<SimpleCacheEntry>::init_class_impl(engine->cx(), engine->global())) {
    return false;
  }
  if (!BuiltinNoConstructor<SimpleCache>::init_class_impl(engine->cx(), engine->global())) {
    return false;
  }
  return true;
}

} // namespace fastly::cache_simple
