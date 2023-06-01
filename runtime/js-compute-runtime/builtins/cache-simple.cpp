#include "cache-simple.h"
#include "builtin.h"
#include "builtins/native-stream-source.h"
#include "builtins/shared/url.h"
#include "host_interface/host_api.h"
#include "host_interface/host_call.h"
#include "js-compute-builtins.h"
#include "js/ArrayBuffer.h"
#include "js/Result.h"
#include "js/Stream.h"
#include "openssl/evp.h"
#include <tuple>

namespace builtins {

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

bool SimpleCacheEntry::constructor(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS_ReportErrorUTF8(cx, "SimpleCacheEntry can't be instantiated directly");
  return false;
}

JSObject *SimpleCacheEntry::create(JSContext *cx, fastly_body_handle_t body_handle) {
  JS::RootedObject SimpleCacheEntry(cx, JS_NewObjectWithGivenProto(cx, &class_, proto_obj));
  if (!SimpleCacheEntry)
    return nullptr;

  JS::SetReservedSlot(SimpleCacheEntry, static_cast<uint32_t>(Slots::Body),
                      JS::Int32Value(body_handle));
  JS::SetReservedSlot(SimpleCacheEntry, static_cast<uint32_t>(Slots::BodyStream), JS::NullValue());
  JS::SetReservedSlot(SimpleCacheEntry, static_cast<uint32_t>(Slots::HasBody),
                      JS::BooleanValue(true));
  JS::SetReservedSlot(SimpleCacheEntry, static_cast<uint32_t>(Slots::BodyUsed), JS::FalseValue());

  return SimpleCacheEntry;
}

bool SimpleCacheEntry::init_class(JSContext *cx, JS::HandleObject global) {
  return init_class_impl(cx, global);
}

namespace {
// We currently support five types of body inputs:
// - byte sequence
// - buffer source
// - USV strings
// - URLSearchParams
// - ReadableStream (Currently only supports Host-backed ReadableStreams)
// After the other other options are checked explicitly, all other inputs are
// encoded to a UTF8 string to be treated as a USV string.
// TODO: Add support for Blob and FormData when we have implemented those classes.
JS::Result<std::tuple<JS::UniqueChars, size_t>> convertBodyInit(JSContext *cx,
                                                                JS::HandleValue bodyInit) {
  JS::RootedObject bodyObj(cx, bodyInit.isObject() ? &bodyInit.toObject() : nullptr);
  mozilla::Maybe<JS::AutoCheckCannotGC> maybeNoGC;
  JS::UniqueChars buf;
  size_t length;

  if (bodyObj && JS_IsArrayBufferViewObject(bodyObj)) {
    // `maybeNoGC` needs to be populated for the lifetime of `buf` because
    // short typed arrays have inline data which can move on GC, so assert
    // that no GC happens. (Which it doesn't, because we're not allocating
    // before `buf` goes out of scope.)
    maybeNoGC.emplace(cx);
    JS::AutoCheckCannotGC &noGC = maybeNoGC.ref();
    bool is_shared;
    length = JS_GetArrayBufferViewByteLength(bodyObj);
    buf = JS::UniqueChars(
        reinterpret_cast<char *>(JS_GetArrayBufferViewData(bodyObj, &is_shared, noGC)));
  } else if (bodyObj && JS::IsArrayBufferObject(bodyObj)) {
    bool is_shared;
    JS::GetArrayBufferLengthAndData(bodyObj, &length, &is_shared, (uint8_t **)&buf);
  } else if (bodyObj && builtins::URLSearchParams::is_instance(bodyObj)) {
    jsurl::SpecSlice slice = builtins::URLSearchParams::serialize(cx, bodyObj);
    buf = JS::UniqueChars(reinterpret_cast<char *>(const_cast<uint8_t *>(slice.data)));
    length = slice.len;
  } else {
    // Convert into a String following https://tc39.es/ecma262/#sec-tostring
    buf = encode(cx, bodyInit, &length);
    if (!buf) {
      return JS::Result<std::tuple<JS::UniqueChars, size_t>>(JS::Error());
    }
  }
  return JS::Result<std::tuple<JS::UniqueChars, size_t>>(std::make_tuple(std::move(buf), length));
}

// Purging/Deleting a cache item within the Compute SDKs via a hostcall is only
// possible via surrogate-keys. We add a surrogate key to all the cache entries,
// which is the sha-256 digest of the cache entries cache-key, converted to
// uppercase hexadecimal.
// Note: We should keep this consistent across the Compute SDKs, this would allow
// a Compute Service to move from one SDK to another, and have consistent purging
// behavior between the Compute Service Versions which were using a different SDK.
JS::Result<std::string> createSurrogateKeyFromCacheKey(JSContext *cx,
                                                       fastly_world_string_t cache_key) {
  const EVP_MD *algorithm = EVP_sha256();
  unsigned int size = EVP_MD_size(algorithm);
  std::vector<unsigned char> md(size);

  if (!EVP_Digest(cache_key.ptr, cache_key.len, md.data(), &size, algorithm, nullptr)) {
    return JS::Result<std::string>(JS::Error());
  }
  std::string surrogate_key(OPENSSL_buf2hexstr(md.data(), size), size);
  surrogate_key.erase(std::remove(surrogate_key.begin(), surrogate_key.end(), ':'),
                      surrogate_key.end());

  return JS::Result<std::string>(surrogate_key);
}

} // namespace

// static set(key: string, value: BodyInit, ttl: number): undefined;
// static set(key: string, value: ReadableStream, ttl: number, length: number): undefined;
bool SimpleCache::set(JSContext *cx, unsigned argc, JS::Value *vp) {
  REQUEST_HANDLER_ONLY("The SimpleCache builtin");
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  if (!args.requireAtLeast(cx, "SimpleCache.set", 3)) {
    return false;
  }

  fastly_world_string_t key;
  // Convert key parameter into a string and check the value adheres to our validation rules.
  JS::UniqueChars key_chars = encode(cx, args.get(0), &key.len);
  if (!key_chars) {
    return false;
  }
  key.ptr = key_chars.get();

  if (key.len == 0) {
    JS_ReportErrorASCII(cx, "SimpleCache.set: key can not be an empty string");
    return false;
  }
  if (key.len > 8135) {
    JS_ReportErrorASCII(cx,
                        "SimpleCache.set: key is too long, the maximum allowed length is 8135.");
    return false;
  }

  fastly_cache_write_options_t options;
  // Convert ttl (time-to-live) parameter into a number and check the value adheres to our
  // validation rules.
  JS::HandleValue ttl_val = args.get(2);
  std::memset(&options, 0, sizeof(options));
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
  HttpBody source_body;
  JS::UniqueChars buf;
  size_t length;
  JS::RootedObject body_obj(cx, body_val.isObject() ? &body_val.toObject() : nullptr);
  // If the body parameter is a Host-backed ReadableStream we optimise our implementation
  // by using the ReadableStream's handle directly.
  if (body_obj && JS::IsReadableStream(body_obj)) {
    if (RequestOrResponse::body_unusable(cx, body_obj)) {
      JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr,
                                JSMSG_READABLE_STREAM_LOCKED_OR_DISTRUBED);
      return false;
    }

    // If the stream is backed by a C@E body handle, we can use that handle directly.
    if (builtins::NativeStreamSource::stream_is_body(cx, body_obj)) {
      JS::RootedObject stream_source(cx,
                                     builtins::NativeStreamSource::get_stream_source(cx, body_obj));
      JS::RootedObject source_owner(cx, builtins::NativeStreamSource::owner(stream_source));
      auto body = RequestOrResponse::body_handle(source_owner);
      source_body = HttpBody(body.handle);
    } else {
      JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr,
                                JSMSG_SIMPLE_CACHE_SET_CONTENT_STREAM);
      return false;
    }

    // The cache APIs require the length to be known upfront, we don't know the length of a
    // stream upfront, which means the caller will need to supply the information explicitly for us.
    if (!args.hasDefined(3)) {
      JS_ReportErrorASCII(cx, "SimpleCache.set: length parameter is required when the value "
                              "parameter is a ReadableStream. The length of the stream needs to be "
                              "known before inserting into the cache.");
      return false;
    }

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
  } else {
    auto result = convertBodyInit(cx, body_val);
    if (result.isErr()) {
      return false;
    }
    std::tie(buf, length) = result.unwrap();
    options.length = length;
  }

  // We create a surrogate-key from the cache-key, as this allows the cached contents to be purgable
  // from within the JavaScript application
  // This is because the cache API currently only supports purging via surrogate-key
  auto key_result = createSurrogateKeyFromCacheKey(cx, key);
  if (key_result.isErr()) {
    return false;
  }
  auto surrogate_key = key_result.unwrap();
  options.surrogate_keys.ptr = const_cast<char *>(surrogate_key.c_str());
  options.surrogate_keys.len = surrogate_key.length();

  fastly_error_t err;
  fastly_body_handle_t body_handle = INVALID_HANDLE;
  if (!fastly_cache_insert(&key, &options, &body_handle, &err)) {
    HANDLE_ERROR(cx, err);
    return false;
  }

  auto body = HttpBody(body_handle);
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
    auto write_res = body.write_all(reinterpret_cast<uint8_t *>(buf.get()), length);
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

  fastly_world_string_t key;
  // Convert key parameter into a string and check the value adheres to our validation rules.
  JS::UniqueChars key_chars = encode(cx, args[0], &key.len);
  if (!key_chars) {
    return false;
  }
  key.ptr = key_chars.get();

  if (key.len == 0) {
    JS_ReportErrorASCII(cx, "SimpleCache.get: key can not be an empty string");
    return false;
  }
  if (key.len > 8135) {
    JS_ReportErrorASCII(cx,
                        "SimpleCache.get: key is too long, the maximum allowed length is 8135.");
    return false;
  }

  fastly_error_t err;
  fastly_cache_lookup_options_t options;
  std::memset(&options, 0, sizeof(options));

  fastly_cache_handle_t handle = INVALID_HANDLE;
  if (!fastly_cache_lookup(&key, &options, &handle, &err)) {
    HANDLE_ERROR(cx, err);
    return false;
  }

  fastly_body_handle_t body = INVALID_HANDLE;
  fastly_cache_get_body_options_t opts;
  if (!fastly_cache_get_body(handle, &opts, &body, &err)) {
    HANDLE_ERROR(cx, err);
    return false;
  }

  if (body == INVALID_HANDLE) {
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

// static delete(key: string): undefined;
bool SimpleCache::delete_(JSContext *cx, unsigned argc, JS::Value *vp) {
  REQUEST_HANDLER_ONLY("The SimpleCache builtin");
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  if (!args.requireAtLeast(cx, "SimpleCache.delete", 1)) {
    return false;
  }

  fastly_world_string_t key;
  // Convert key parameter into a string and check the value adheres to our validation rules.
  {
    JS::UniqueChars key_chars = encode(cx, args.get(0), &key.len);
    if (!key_chars) {
      return false;
    }
    key.ptr = key_chars.get();

    if (key.len == 0) {
      JS_ReportErrorASCII(cx, "SimpleCache.delete: key can not be an empty string");
      return false;
    }
    if (key.len > 1024) {
      JS_ReportErrorASCII(
          cx, "SimpleCache.delete: key is too long, the maximum allowed length is 1024.");
      return false;
    }
  }

  // We create a surrogate-key from the cache-key, as this allows the cached contents to be purgable
  // from within the JavaScript application
  // This is because the cache API currently only supports purging via surrogate-key
  auto surrogate_key_result = createSurrogateKeyFromCacheKey(cx, key);
  if (surrogate_key_result.isErr()) {
    return false;
  }
  auto surrogate_key = surrogate_key_result.unwrap();

  fastly_world_string_t skey;
  skey.ptr = const_cast<char *>(surrogate_key.c_str());
  skey.len = surrogate_key.length();

  fastly_error_t err;
  fastly_option_string_t ret;
  fastly_purge_options_mask_t purge_options = 0;
  if (!fastly_purge_surrogate_key(&skey, purge_options, &ret, &err)) {
    HANDLE_ERROR(cx, err);
    return false;
  }

  args.rval().setUndefined();
  return true;
}

const JSFunctionSpec SimpleCache::static_methods[] = {
    JS_FN("delete", delete_, 1, JSPROP_ENUMERATE),
    JS_FN("get", get, 1, JSPROP_ENUMERATE),
    JS_FN("set", set, 3, JSPROP_ENUMERATE),
    JS_FS_END,
};

const JSPropertySpec SimpleCache::static_properties[] = {
    JS_PS_END,
};

const JSFunctionSpec SimpleCache::methods[] = {JS_FS_END};

const JSPropertySpec SimpleCache::properties[] = {
    JS_STRING_SYM_PS(toStringTag, "SimpleCache", JSPROP_READONLY), JS_PS_END};

bool SimpleCache::constructor(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS_ReportErrorNumberASCII(cx, GetErrorMessageBuiltin, nullptr, JSMSG_ILLEGAL_CTOR);
  return false;
}

bool SimpleCache::init_class(JSContext *cx, JS::HandleObject global) {
  return BuiltinImpl<SimpleCache>::init_class_impl(cx, global);
}

} // namespace builtins
