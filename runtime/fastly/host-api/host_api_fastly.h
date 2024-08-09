#ifndef FASTLY_HOST_API_H
#define FASTLY_HOST_API_H

#include <cstdint>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "extension-api.h"
#include "host_api.h"
#include "js/TypeDecls.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-offsetof"
#include "js/Utility.h"
#include "jsapi.h"
#pragma clang diagnostic pop

struct JSErrorFormatString;

namespace host_api {
void handle_api_error(JSContext *cx, uint8_t err, int line, const char *func);
bool error_is_generic(APIError e);
bool error_is_invalid_argument(APIError e);
bool error_is_optional_none(APIError e);
bool error_is_bad_handle(APIError e);
void handle_fastly_error(JSContext *cx, APIError err, int line, const char *func);
} // namespace host_api

namespace fastly {

enum FastlyAPIError {
#define MSG_DEF(name, count, exception, format) name,
#include "./error_numbers.msg"
#undef MSG_DEF
  JSErrNum_Limit
};

const JSErrorFormatString fastly_ErrorFormatString[JSErrNum_Limit] = {
#define MSG_DEF(name, count, exception, format) {#name, format, count, exception},
#include "./error_numbers.msg"
#undef MSG_DEF
};

} // namespace fastly

namespace api {

template <typename T, typename E> class FastlyResult final {
  struct Error {
    E value;

    explicit Error(E value) : value{value} {}
  };

  std::variant<T, Error> result;

public:
  FastlyResult() = default;

  /// Explicitly construct an error.
  static FastlyResult err(E err) {
    FastlyResult res;
    res.emplace_err(err);
    return res;
  }

  /// Explicitly construct a successful result.
  template <typename... Args> static FastlyResult ok(Args &&...args) {
    FastlyResult res;
    res.emplace(std::forward<Args>(args)...);
    return res;
  }

  /// Construct an error in-place.
  E &emplace_err(E err) & { return this->result.template emplace<Error>(err).value; }

  /// Construct a value of T in-place.
  template <typename... Args> T &emplace(Args &&...args) {
    return this->result.template emplace<T>(std::forward<Args>(args)...);
  }

  /// True when the result contains an error.
  bool is_err() const { return std::holds_alternative<Error>(this->result); }

  /// Return a pointer to the error value of this result, if the call failed.
  const E *to_err() const { return reinterpret_cast<const E *>(std::get_if<Error>(&this->result)); }

  /// Assume the call was successful, and return a reference to the result.
  T &unwrap() { return std::get<T>(this->result); }
};

typedef bool ProcessAsyncTask(JSContext *cx, uint32_t handle, JS::HandleObject context,
                              JS::HandleObject promise);

class FastlyAsyncTask final : public AsyncTask {
public:
  using Handle = uint32_t;

  static constexpr Handle invalid = UINT32_MAX - 1;

  FastlyAsyncTask(Handle handle, JS::HandleObject context, JS::HandleObject promise,
                  ProcessAsyncTask *process) {
    if (static_cast<int32_t>(handle) == INVALID_POLLABLE_HANDLE)
      abort();
    handle_ = static_cast<int32_t>(handle);
    context_ = Heap<JSObject *>(context);
    promise_ = Heap<JSObject *>(promise);
    process_steps_ = process;
  }

  [[nodiscard]] bool run(Engine *engine) override {
    if (process_steps_) {
      RootedObject promise(engine->cx(), promise_);
      RootedObject context(engine->cx(), context_);
      if (!process_steps_(engine->cx(), handle_, context, promise)) {
        return false;
      }
    }
    return true;
  }

  [[nodiscard]] bool cancel(Engine *engine) override {
    MOZ_ASSERT_UNREACHABLE("Fastly semantics don't allow for cancellation");
    return false;
  }

  Handle handle() { return handle_; }

  void trace(JSTracer *trc) override {
    TraceEdge(trc, &context_, "Async task context");
    TraceEdge(trc, &promise_, "Async task promise");
  }

  Heap<JSObject *> context_;
  Heap<JSObject *> promise_;
  ProcessAsyncTask *process_steps_ = nullptr;
};

} // namespace api

namespace fastly::fetch {

class Request;

} // namespace fastly::fetch

using api::FastlyAsyncTask;
using fastly::fetch::Request;

namespace host_api {

JSString *get_geo_info(JSContext *cx, JS::HandleString address_str);

bool error_is_generic(APIError e);
bool error_is_invalid_argument(APIError e);
bool error_is_optional_none(APIError e);
bool error_is_bad_handle(APIError e);

class FastlySendError final {
public:
  enum detail {
    /// The send-error-detail struct has not been populated.
    uninitialized,
    /// There was no send error.
    ok,
    /// The system encountered a timeout when trying to find an IP address for the backend
    /// hostname.
    dns_timeout,
    /// The system encountered a DNS error when trying to find an IP address for the backend
    /// hostname. The fields dns_error_rcode and dns_error_info_code may be set in the
    /// send_error_detail.
    dns_error,
    /// The system cannot determine which backend to use, or the specified backend was invalid.
    destination_not_found,
    /// The system considers the backend to be unavailable; e.g., recent attempts to communicate
    /// with it may have failed, or a health check may indicate that it is down.
    destination_unavailable,
    /// The system cannot find a route to the next_hop IP address.
    destination_ip_unroutable,
    /// The system's connection to the backend was refused.
    connection_refused,
    /// The system's connection to the backend was closed before a complete response was
    /// received.
    connection_terminated,
    /// The system's attempt to open a connection to the backend timed out.
    connection_timeout,
    /// The system is configured to limit the number of connections it has to the backend, and
    /// that limit has been exceeded.
    connection_limit_reached,
    /// The system encountered an error when verifying the certificate presented by the backend.
    tls_certificate_error,
    /// The system encountered an error with the backend TLS configuration.
    tls_configuration_error,
    /// The system received an incomplete response to the request from the backend.
    http_incomplete_response,
    /// The system received a response to the request whose header section was considered too
    /// large.
    http_response_header_section_too_large,
    /// The system received a response to the request whose body was considered too large.
    http_response_body_too_large,
    /// The system reached a configured time limit waiting for the complete response.
    http_response_timeout,
    /// The system received a response to the request whose status code or reason phrase was
    /// invalid.
    http_response_status_invalid,
    /// The process of negotiating an upgrade of the HTTP version between the system and the
    /// backend failed.
    http_upgrade_failed,
    /// The system encountered an HTTP protocol error when communicating with the backend. This
    /// error will only be used when a more specific one is not defined.
    http_protocol_error,
    /// An invalid cache key was provided for the request.
    http_request_cache_key_invalid,
    /// An invalid URI was provided for the request.
    http_request_uri_invalid,
    /// The system encountered an unexpected internal error.
    internal_error,
    /// The system received a TLS alert from the backend. The field tls_alert_id may be set in
    /// the send_error_detail.
    tls_alert_received,
    /// The system encountered a TLS error when communicating with the backend, either during
    /// the handshake or afterwards.
    tls_protocol_error
  };

  detail tag;
  uint16_t dns_error_rcode;
  uint16_t dns_error_info_code;
  uint8_t tls_alert_id;

  const std::optional<std::string> message() const;
};

/// A convenience wrapper for the host calls involving http bodies.
class HttpBody final {
public:
  using Handle = uint32_t;

  static constexpr Handle invalid = UINT32_MAX - 1;

  Handle handle = invalid;

  HttpBody() = default;
  explicit HttpBody(Handle handle) : handle{handle} {}
  explicit HttpBody(FastlyAsyncTask async) : handle{async.handle()} {}

  /// Returns true when this body handle is valid.
  bool valid() const { return this->handle != invalid; }

  /// Make a new body handle.
  static Result<HttpBody> make();

  /// Read a chunk from this handle.
  Result<HostString> read(uint32_t chunk_size) const;

  /// Write a chunk to the front of this handle.
  Result<uint32_t> write_front(const uint8_t *bytes, size_t len) const;

  /// Write a chunk to the back of this handle.
  Result<uint32_t> write_back(const uint8_t *bytes, size_t len) const;

  /// Writes the given number of bytes from the given buffer to the front of the given handle.
  ///
  /// The host doesn't necessarily write all bytes in any particular call to
  /// `write`, so to ensure all bytes are written, we call it in a loop.
  Result<Void> write_all_front(const uint8_t *bytes, size_t len) const;

  /// Writes the given number of bytes from the given buffer to the back of the given handle.
  ///
  /// The host doesn't necessarily write all bytes in any particular call to
  /// `write`, so to ensure all bytes are written, we call it in a loop.
  Result<Void> write_all_back(const uint8_t *bytes, size_t len) const;

  /// Append another HttpBody to this one.
  Result<Void> append(HttpBody other) const;

  /// Close this handle, and reset internal state to invalid.
  Result<Void> close();

  FastlyAsyncTask::Handle async_handle() const;
};

struct Response;

class HttpPendingReq final {
public:
  using Handle = uint32_t;

  static constexpr Handle invalid = UINT32_MAX - 1;

  Handle handle = invalid;

  HttpPendingReq() = default;
  explicit HttpPendingReq(Handle handle) : handle{handle} {}
  explicit HttpPendingReq(FastlyAsyncTask async) : handle{async.handle()} {}

  /// Poll for the response to this request.
  Result<std::optional<Response>> poll();

  /// Block until the response is ready.
  api::FastlyResult<Response, FastlySendError> wait();

  /// Fetch the FastlyAsyncTask for this pending request.
  FastlyAsyncTask::Handle async_handle() const;
};

using HttpVersion = uint8_t;

class HttpBase {
public:
  virtual ~HttpBase() = default;

  virtual bool is_valid() const = 0;

  /// Get the http version used for this request.
  virtual Result<HttpVersion> get_version() const = 0;

  virtual HttpHeadersReadOnly *headers() = 0;
  virtual HttpHeaders *headers_writable() = 0;
};

struct TlsVersion {
  uint8_t value = 0;

  explicit TlsVersion(uint8_t raw);

  uint8_t get_version() const;
  static TlsVersion version_1();
  static TlsVersion version_1_1();
  static TlsVersion version_1_2();
  static TlsVersion version_1_3();
};

typedef uint32_t CertKey;

struct ClientCert {
  HostString cert;
  CertKey key;
};

struct BackendConfig {
  std::optional<HostString> host_override;
  std::optional<uint32_t> connect_timeout;
  std::optional<uint32_t> first_byte_timeout;
  std::optional<uint32_t> between_bytes_timeout;
  std::optional<bool> use_ssl;
  std::optional<bool> dont_pool;
  std::optional<TlsVersion> ssl_min_version;
  std::optional<TlsVersion> ssl_max_version;
  std::optional<HostString> cert_hostname;
  std::optional<HostString> ca_cert;
  std::optional<HostString> ciphers;
  std::optional<HostString> sni_hostname;
  std::optional<ClientCert> client_cert;
};

struct CacheOverrideTag final {
  uint8_t value = 0;

  void set_pass();
  void set_ttl();
  void set_stale_while_revalidate();
  void set_pci();
};

enum class FramingHeadersMode : uint8_t {
  Automatic,
  ManuallyFromHeaders,
};

class HttpReq final : public HttpBase {
public:
  using Handle = uint32_t;

  static constexpr Handle invalid = UINT32_MAX - 1;

  Handle handle = invalid;

  HttpReq() = default;
  explicit HttpReq(Handle handle) : handle{handle} {}

  static Result<HttpReq> make();

  static Result<Void> redirect_to_grip_proxy(std::string_view backend);

  static Result<Void> register_dynamic_backend(std::string_view name, std::string_view target,
                                               const BackendConfig &config);

  /// Fetch the downstream request/body pair
  static Result<Request> downstream_get();

  /// Get the downstream ip address.
  static Result<HostBytes> downstream_client_ip_addr();

  static Result<HostBytes> downstream_server_ip_addr();

  static Result<HostString> http_req_downstream_tls_cipher_openssl_name();

  static Result<HostString> http_req_downstream_tls_protocol();

  static Result<HostBytes> http_req_downstream_tls_client_hello();

  static Result<HostBytes> http_req_downstream_tls_raw_client_certificate();

  static Result<HostBytes> http_req_downstream_tls_ja3_md5();

  Result<Void> auto_decompress_gzip();

  /// Send this request synchronously, and wait for the response.
  Result<Response> send(HttpBody body, std::string_view backend);

  /// Send this request asynchronously.
  Result<HttpPendingReq> send_async(HttpBody body, std::string_view backend);

  /// Send this request asynchronously, and allow sending additional data through the body.
  Result<HttpPendingReq> send_async_streaming(HttpBody body, std::string_view backend);

  /// Get the http version used for this request.

  /// Set the request method.
  Result<Void> set_method(std::string_view method);

  /// Get the request method.
  Result<HostString> get_method() const;

  /// Set the request uri.
  Result<Void> set_uri(std::string_view str);

  /// Get the request uri.
  Result<HostString> get_uri() const;

  /// Configure cache-override settings.
  Result<Void> cache_override(CacheOverrideTag tag, std::optional<uint32_t> ttl,
                              std::optional<uint32_t> stale_while_revalidate,
                              std::optional<std::string_view> surrogate_key);

  /// Set the framing headers mode for this request.
  Result<Void> set_framing_headers_mode(FramingHeadersMode mode);

  bool is_valid() const override;

  Result<HttpVersion> get_version() const override;

  HttpHeadersReadOnly *headers() override;
  HttpHeaders *headers_writable() override;
};

class HttpResp final : public HttpBase {
public:
  using Handle = uint32_t;

  static constexpr Handle invalid = UINT32_MAX - 1;

  Handle handle = invalid;

  HttpResp() = default;
  explicit HttpResp(Handle handle) : handle{handle} {}

  static Result<HttpResp> make();

  /// Get the http status for the response.
  Result<uint16_t> get_status() const;

  /// Set the http status for the response.
  Result<Void> set_status(uint16_t status);

  /// Immediately begin sending this response to the downstream client.
  Result<Void> send_downstream(HttpBody body, bool streaming);

  /// Set the framing headers mode for this response.
  Result<Void> set_framing_headers_mode(FramingHeadersMode mode);

  bool is_valid() const override;

  Result<HttpVersion> get_version() const override;

  /// Get the IP address associated with the response
  Result<std::optional<HostBytes>> get_ip() const;
  /// Get the port associated with the response
  Result<std::optional<uint16_t>> get_port() const;

  HttpHeadersReadOnly *headers() override;
  HttpHeaders *headers_writable() override;
};

/// The pair of a response and its body.
struct Response {
  HttpResp resp;
  HttpBody body;

  Response() = default;
  Response(HttpResp resp, HttpBody body) : resp{resp}, body{body} {}
};

/// The pair of a request and its body.
struct Request {
  HttpReq req;
  HttpBody body;

  Request() = default;
  Request(HttpReq req, HttpBody body) : req{req}, body{body} {}
};

class GeoIp final {
  ~GeoIp() = delete;

public:
  /// Lookup information about the ip address provided.
  static Result<HostString> lookup(std::span<uint8_t> bytes);
};

class LogEndpoint final {
public:
  using Handle = uint32_t;

  static constexpr Handle invalid = UINT32_MAX - 1;

  Handle handle = invalid;

  LogEndpoint() = default;
  explicit LogEndpoint(Handle handle) : handle{handle} {}

  static Result<LogEndpoint> get(std::string_view name);

  Result<Void> write(std::string_view msg);
};

class Dict final {
public:
  using Handle = uint32_t;

  static constexpr Handle invalid = UINT32_MAX - 1;

  Handle handle = invalid;

  Dict() = default;
  explicit Dict(Handle handle) : handle{handle} {}

  static Result<Dict> open(std::string_view name);

  Result<std::optional<HostString>> get(std::string_view name);
};

class ConfigStore final {
public:
  using Handle = uint32_t;

  static constexpr Handle invalid = UINT32_MAX - 1;

  Handle handle = invalid;

  ConfigStore() = default;
  explicit ConfigStore(Handle handle) : handle{handle} {}

  static Result<ConfigStore> open(std::string_view name);

  Result<std::optional<HostString>> get(std::string_view name);
};

class ObjectStorePendingLookup final {
public:
  using Handle = uint32_t;

  static constexpr Handle invalid = UINT32_MAX - 1;

  Handle handle = invalid;

  ObjectStorePendingLookup() = default;
  explicit ObjectStorePendingLookup(Handle handle) : handle{handle} {}
  explicit ObjectStorePendingLookup(FastlyAsyncTask async) : handle{async.handle()} {}

  /// Block until the response is ready.
  api::FastlyResult<std::optional<HttpBody>, fastly::FastlyAPIError> wait();

  /// Fetch the handle for this pending request.
  FastlyAsyncTask::Handle async_handle() const;
};

class ObjectStorePendingDelete final {
public:
  using Handle = uint32_t;

  static constexpr Handle invalid = UINT32_MAX - 1;

  Handle handle = invalid;

  ObjectStorePendingDelete() = default;
  explicit ObjectStorePendingDelete(Handle handle) : handle{handle} {}
  explicit ObjectStorePendingDelete(FastlyAsyncTask async) : handle{async.handle()} {}

  /// Block until the response is ready.
  Result<Void> wait();

  /// Fetch the handle for this pending request.
  FastlyAsyncTask::Handle async_handle() const;
};

class ObjectStore final {
public:
  using Handle = uint32_t;

  static constexpr Handle invalid = UINT32_MAX - 1;

  Handle handle = invalid;

  ObjectStore() = default;
  explicit ObjectStore(Handle handle) : handle{handle} {}

  static Result<ObjectStore> open(std::string_view name);

  Result<std::optional<HttpBody>> lookup(std::string_view name);
  Result<ObjectStorePendingLookup::Handle> lookup_async(std::string_view name);
  Result<ObjectStorePendingDelete::Handle> delete_async(std::string_view name);

  Result<Void> insert(std::string_view name, HttpBody body);
};

class Secret final {
public:
  using Handle = uint32_t;

  static constexpr Handle invalid = UINT32_MAX - 1;

  Handle handle = invalid;

  Secret() = default;
  explicit Secret(Handle handle) : handle{handle} {}

  Result<std::optional<HostBytes>> plaintext() const;
};

class SecretStore final {
public:
  using Handle = uint32_t;

  static constexpr Handle invalid = UINT32_MAX - 1;

  Handle handle = invalid;

  SecretStore() = default;
  explicit SecretStore(Handle handle) : handle{handle} {}

  static Result<SecretStore> open(std::string_view name);

  Result<std::optional<Secret>> get(std::string_view name);
  static Result<Secret> from_bytes(uint8_t *bytes, size_t len);
};

struct CacheLookupOptions final {
  /// A full request handle, used only for its headers.
  HttpReq request_headers;
};

struct CacheGetBodyOptions final {
  std::optional<uint64_t> start;
  std::optional<uint64_t> end;
};

struct CacheWriteOptions final {
  uint64_t max_age_ns = 0;
  HttpReq request_headers;
  std::string vary_rule;

  uint64_t initial_age_ns = 0;
  uint64_t stale_while_revalidate_ns = 0;

  std::string surrogate_keys;

  uint64_t length = 0;

  HostBytes metadata;

  bool sensitive = false;
};

struct CacheState final {
  uint8_t state = 0;

  CacheState() = default;
  CacheState(uint8_t state) : state{state} {}

  bool is_found() const;
  bool is_usable() const;
  bool is_stale() const;
  bool must_insert_or_update() const;
};

class CacheHandle final {
public:
  using Handle = uint32_t;

  static constexpr Handle invalid = UINT32_MAX - 1;

  Handle handle = invalid;

  CacheHandle() = default;
  explicit CacheHandle(Handle handle) : handle{handle} {}

  /// Lookup a cached object.
  static Result<CacheHandle> lookup(std::string_view key, const CacheLookupOptions &opts);

  static Result<CacheHandle> transaction_lookup(std::string_view key,
                                                const CacheLookupOptions &opts);

  /// Insert a cache object.
  static Result<HttpBody> insert(std::string_view key, const CacheWriteOptions &opts);

  Result<HttpBody> transaction_insert(const CacheWriteOptions &opts);

  Result<Void> transaction_update(const CacheWriteOptions &opts);

  /// Insert this cached object and stream it back.
  Result<std::tuple<HttpBody, CacheHandle>>
  transaction_insert_and_stream_back(const CacheWriteOptions &opts);

  bool is_valid() const { return this->handle != invalid; }

  /// Cancel a transaction.
  Result<Void> transaction_cancel();

  /// Fetch the body handle for the cached data.
  Result<HttpBody> get_body(const CacheGetBodyOptions &opts);

  /// Fetch the state for this cache handle.
  Result<CacheState> get_state();

  Result<Void> close();

  Result<HostBytes> get_user_metadata();

  Result<uint64_t> get_length();

  Result<uint64_t> get_max_age_ns();

  Result<uint64_t> get_stale_while_revalidate_ns();

  Result<uint64_t> get_age_ns();

  Result<uint64_t> get_hits();
};

class Fastly final {
  ~Fastly() = delete;

public:
  /// Purge the given surrogate key.
  static Result<std::optional<HostString>> purge_surrogate_key(std::string_view key);
};

struct BackendHealth final {
public:
  uint8_t state = 0;

  BackendHealth() = default;
  BackendHealth(uint8_t state) : state{state} {}

  bool is_unknown() const;
  bool is_healthy() const;
  bool is_unhealthy() const;
};

class Backend final {
  std::string_view name;

  Backend() = default;
  explicit Backend(std::string_view name) : name{name} {}

public:
  static Result<bool> exists(std::string_view name);
  static Result<BackendHealth> health(std::string_view name);
};

class PenaltyBox final {
public:
  static Result<Void> add(std::string_view name, std::string_view entry, uint32_t time_to_live);
  static Result<bool> has(std::string_view name, std::string_view entry);
};

class RateCounter final {
public:
  static Result<Void> increment(std::string_view name, std::string_view entry, uint32_t delta);
  static Result<uint32_t> lookup_rate(std::string_view name, std::string_view entry,
                                      uint32_t window);
  static Result<uint32_t> lookup_count(std::string_view name, std::string_view entry,
                                       uint32_t duration);
};

class EdgeRateLimiter final {
public:
  static Result<bool> check_rate(std::string_view rate_counter_name, std::string_view entry,
                                 uint32_t delta, uint32_t window, uint32_t limit,
                                 std::string_view penalty_box_name, uint32_t time_to_live);
};

class DeviceDetection final {
public:
  static Result<HostString> lookup(std::string_view user_agent);
};

} // namespace host_api

#endif
