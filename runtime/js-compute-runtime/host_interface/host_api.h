#ifndef JS_COMPUTE_RUNTIME_HOST_API_H
#define JS_COMPUTE_RUNTIME_HOST_API_H

#include <cstdint>
#include <fmt/format.h>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "core/allocator.h"
#include "js/TypeDecls.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-offsetof"
#include "js/Utility.h"
#include "jsapi.h"
#pragma clang diagnostic pop

namespace host_api {

/// A type to signal that a result produces no value.
struct Void final {};

/// The type of errors returned from the host.
using FastlyError = uint8_t;

bool error_is_generic(FastlyError e);
bool error_is_invalid_argument(FastlyError e);
bool error_is_optional_none(FastlyError e);
bool error_is_bad_handle(FastlyError e);

/// Generate an error in the JSContext.
void handle_fastly_error(JSContext *cx, FastlyError err, int line, const char *func);

/// Wrap up a call to handle_fastly_error with the current line and function.
#define HANDLE_ERROR(cx, err) ::host_api::handle_fastly_error(cx, err, __LINE__, __func__)

template <typename T, typename E = FastlyError> class Result final {
  /// A private wrapper to distinguish `fastly_compute_at_edge_types_error_t` in the private
  /// variant.
  struct Error {
    E value;

    explicit Error(E value) : value{value} {}
  };

  std::variant<T, Error> result;

public:
  Result() = default;

  /// Explicitly construct an error.
  static Result err(E err) {
    Result res;
    res.emplace_err(err);
    return res;
  }

  /// Explicitly construct a successful result.
  template <typename... Args> static Result ok(Args &&...args) {
    Result res;
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

/// A string allocated by the host interface. Holds ownership of the data.
struct HostString final {
  JS::UniqueChars ptr;
  size_t len;

  HostString() = default;
  HostString(std::nullptr_t) : HostString() {}
  HostString(JS::UniqueChars ptr, size_t len) : ptr{std::move(ptr)}, len{len} {}

  HostString(const HostString &other) = delete;
  HostString &operator=(const HostString &other) = delete;

  HostString(HostString &&other) : ptr{std::move(other.ptr)}, len{other.len} {}
  HostString &operator=(HostString &&other) {
    this->ptr.reset(other.ptr.release());
    this->len = other.len;
    return *this;
  }

  using iterator = char *;
  using const_iterator = const char *;

  size_t size() const { return this->len; }

  iterator begin() { return this->ptr.get(); }
  iterator end() { return this->begin() + this->len; }

  const_iterator begin() const { return this->ptr.get(); }
  const_iterator end() const { return this->begin() + this->len; }

  /// Conversion to a bool, testing for an empty pointer.
  operator bool() const { return this->ptr != nullptr; }

  /// Comparison against nullptr
  bool operator==(std::nullptr_t) { return this->ptr == nullptr; }

  /// Comparison against nullptr
  bool operator!=(std::nullptr_t) { return this->ptr != nullptr; }

  /// Conversion to a `std::string_view`.
  operator std::string_view() const { return std::string_view(this->ptr.get(), this->len); }
};

struct HostBytes final {
  std::unique_ptr<uint8_t[]> ptr;
  size_t len;

  HostBytes() = default;
  HostBytes(std::nullptr_t) : HostBytes() {}
  HostBytes(std::unique_ptr<uint8_t[]> ptr, size_t len) : ptr{std::move(ptr)}, len{len} {}
  HostBytes(JS::UniqueChars ptr, size_t len)
      : ptr{reinterpret_cast<uint8_t *>(ptr.release())}, len{len} {}

  HostBytes(const HostBytes &other) = delete;
  HostBytes &operator=(const HostBytes &other) = delete;

  HostBytes(HostBytes &&other) : ptr{std::move(other.ptr)}, len{other.len} {}
  HostBytes &operator=(HostBytes &&other) {
    this->ptr.reset(other.ptr.release());
    this->len = other.len;
    return *this;
  }

  /// Allocate a zeroed HostBytes with the given number of bytes.
  static HostBytes with_capacity(size_t len) {
    HostBytes ret;
    ret.ptr = std::make_unique<uint8_t[]>(len);
    ret.len = len;
    return ret;
  }

  using iterator = uint8_t *;
  using const_iterator = const uint8_t *;

  size_t size() const { return this->len; }

  iterator begin() { return this->ptr.get(); }
  iterator end() { return this->begin() + this->len; }

  const_iterator begin() const { return this->ptr.get(); }
  const_iterator end() const { return this->begin() + this->len; }

  /// Conversion to a bool, testing for an empty pointer.
  operator bool() const { return this->ptr != nullptr; }

  /// Comparison against nullptr
  bool operator==(std::nullptr_t) { return this->ptr == nullptr; }

  /// Comparison against nullptr
  bool operator!=(std::nullptr_t) { return this->ptr != nullptr; }

  /// Converstion to a `std::span<uint8_t>`.
  operator std::span<uint8_t>() const { return std::span{this->ptr.get(), this->len}; }
};

/// Common methods for async handles.
class AsyncHandle {
public:
  using Handle = uint32_t;

  static constexpr Handle invalid = UINT32_MAX - 1;

  Handle handle;

  AsyncHandle() = default;
  explicit AsyncHandle(Handle handle) : handle{handle} {}

  /// Check to see if this handle is ready.
  Result<bool> is_ready() const;

  /// Return the index in handles of the `AsyncHandle` that's ready. If the select call finishes
  /// successfully and returns `std::nullopt`, the timeout has expired.
  ///
  /// If the timeout is `0`, two behaviors are possible
  ///   * if handles is empty, an error will be returned immediately
  ///   * otherwise, block until a handle is ready and return its index
  ///
  /// If the timeout is non-zero, two behaviors are possible
  ///   * no handle becomes ready within timeout, and the successful `std::nullopt` is returned
  ///   * a handle becomes ready within the timeout, and its index is returned.
  static Result<std::optional<uint32_t>> select(const std::vector<AsyncHandle> &handles,
                                                uint32_t timeout_ms);
};

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

  /// The handle to use when making host calls, initialized to the special invalid value used by
  /// executed.
  Handle handle = invalid;

  HttpBody() = default;
  explicit HttpBody(Handle handle) : handle{handle} {}
  explicit HttpBody(AsyncHandle async) : handle{async.handle} {}

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

  AsyncHandle async_handle() const;
};

struct Response;

class HttpPendingReq final {
public:
  using Handle = uint32_t;

  static constexpr Handle invalid = UINT32_MAX - 1;

  Handle handle = invalid;

  HttpPendingReq() = default;
  explicit HttpPendingReq(Handle handle) : handle{handle} {}
  explicit HttpPendingReq(AsyncHandle async) : handle{async.handle} {}

  /// Poll for the response to this request.
  Result<std::optional<Response>> poll();

  /// Block until the response is ready.
  Result<Response, FastlySendError> wait();

  /// Fetch the AsyncHandle for this pending request.
  AsyncHandle async_handle() const;
};

using HttpVersion = uint8_t;

class HttpBase {
public:
  virtual ~HttpBase() = default;

  virtual bool is_valid() const = 0;

  /// Get the http version used for this request.
  virtual Result<HttpVersion> get_version() const = 0;

  virtual Result<std::vector<HostString>> get_header_names() = 0;
  virtual Result<std::optional<std::vector<HostBytes>>>
  get_header_values(std::string_view name) = 0;
  virtual Result<Void> insert_header(std::string_view name, std::span<uint8_t> value) = 0;
  virtual Result<Void> append_header(std::string_view name, std::span<uint8_t> value) = 0;
  virtual Result<Void> remove_header(std::string_view name) = 0;
};

struct TlsVersion {
  uint8_t value = 0;

  explicit TlsVersion(uint8_t raw);

  static TlsVersion version_1();
  static TlsVersion version_1_1();
  static TlsVersion version_1_2();
  static TlsVersion version_1_3();
};

class Secret final {
public:
  using Handle = uint32_t;

  Handle handle = UINT32_MAX - 1;

  Secret() = default;
  explicit Secret(Handle handle) : handle{handle} {}

  Result<std::optional<HostBytes>> plaintext() const;
};

class SecretStore final {
public:
  using Handle = uint32_t;

  Handle handle = UINT32_MAX - 1;

  SecretStore() = default;
  explicit SecretStore(Handle handle) : handle{handle} {}

  static Result<SecretStore> open(std::string_view name);

  Result<std::optional<Secret>> get(std::string_view name);

  static Result<Secret> from_bytes(uint8_t *bytes, size_t len);
};

struct ClientCert {
  HostString cert;
  Secret key;
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

struct Request;

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

  Result<std::vector<HostString>> get_header_names() override;
  Result<std::optional<std::vector<HostBytes>>> get_header_values(std::string_view name) override;
  Result<Void> insert_header(std::string_view name, std::span<uint8_t> value) override;
  Result<Void> append_header(std::string_view name, std::span<uint8_t> value) override;
  Result<Void> remove_header(std::string_view name) override;
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

  Result<std::vector<HostString>> get_header_names() override;
  Result<std::optional<std::vector<HostBytes>>> get_header_values(std::string_view name) override;
  Result<Void> insert_header(std::string_view name, std::span<uint8_t> value) override;
  Result<Void> append_header(std::string_view name, std::span<uint8_t> value) override;
  Result<Void> remove_header(std::string_view name) override;

  /// Get the IP address associated with the response
  Result<std::optional<HostBytes>> get_ip() const;
  /// Get the port associated with the request
  Result<std::optional<uint16_t>> get_port() const;
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

  Handle handle = UINT32_MAX - 1;

  LogEndpoint() = default;
  explicit LogEndpoint(Handle handle) : handle{handle} {}

  static Result<LogEndpoint> get(std::string_view name);

  Result<Void> write(std::string_view msg);
};

class Dict final {
public:
  using Handle = uint32_t;

  Handle handle = UINT32_MAX - 1;

  Dict() = default;
  explicit Dict(Handle handle) : handle{handle} {}

  static Result<Dict> open(std::string_view name);

  Result<std::optional<HostString>> get(std::string_view name);
};

class ConfigStore final {
public:
  using Handle = uint32_t;

  Handle handle = UINT32_MAX - 1;

  ConfigStore() = default;
  explicit ConfigStore(Handle handle) : handle{handle} {}

  static Result<ConfigStore> open(std::string_view name);

  Result<std::optional<HostString>> get(std::string_view name);
};

class ObjectStore final {
public:
  using Handle = uint32_t;

  Handle handle = UINT32_MAX - 1;

  ObjectStore() = default;
  explicit ObjectStore(Handle handle) : handle{handle} {}

  static Result<ObjectStore> open(std::string_view name);

  Result<std::optional<HttpBody>> lookup(std::string_view name);
  Result<AsyncHandle> lookup_async(std::string_view name);
  Result<AsyncHandle> delete_async(std::string_view name);

  Result<Void> insert(std::string_view name, HttpBody body);
};

class ObjectStorePendingLookup final {
public:
  using Handle = uint32_t;

  static constexpr Handle invalid = UINT32_MAX - 1;

  Handle handle = invalid;

  ObjectStorePendingLookup() = default;
  explicit ObjectStorePendingLookup(Handle handle) : handle{handle} {}
  explicit ObjectStorePendingLookup(AsyncHandle async) : handle{async.handle} {}

  /// Block until the response is ready.
  Result<std::optional<HttpBody>, FastlyError> wait();

  /// Fetch the AsyncHandle for this pending request.
  AsyncHandle async_handle() const;
};

class ObjectStorePendingDelete final {
public:
  using Handle = uint32_t;

  static constexpr Handle invalid = UINT32_MAX - 1;

  Handle handle = invalid;

  ObjectStorePendingDelete() = default;
  explicit ObjectStorePendingDelete(Handle handle) : handle{handle} {}
  explicit ObjectStorePendingDelete(AsyncHandle async) : handle{async.handle} {}

  /// Block until the response is ready.
  Result<Void> wait();

  /// Fetch the AsyncHandle for this pending request.
  AsyncHandle async_handle() const;
};

class Random final {
public:
  static Result<HostBytes> get_bytes(size_t num_bytes);

  static Result<uint32_t> get_u32();
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
