#ifndef JS_COMPUTE_RUNTIME_HOST_API_H
#define JS_COMPUTE_RUNTIME_HOST_API_H

#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "core/allocator.h"
#include "fastly-world/fastly_world.h"
#include "host_interface/host_call.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-offsetof"
#include "js/Utility.h"
#pragma clang diagnostic pop

/// A type to signal that a result produces no value.
struct Void final {};

template <typename T> class Result final {
  /// A private wrapper to distinguish `fastly_error_t` in the private variant.
  struct Error {
    fastly_error_t value;

    explicit Error(fastly_error_t value) : value{value} {}
  };

  std::variant<T, Error> result;

public:
  Result() = default;

  /// Explicitly construct an error.
  static Result err(fastly_error_t err) {
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
  fastly_error_t &emplace_err(fastly_error_t err) & {
    return this->result.template emplace<Error>(err).value;
  }

  /// Construct a value of T in-place.
  template <typename... Args> T &emplace(Args &&...args) {
    return this->result.template emplace<T>(std::forward<Args>(args)...);
  }

  /// True when the result contains an error.
  bool is_err() const { return std::holds_alternative<Error>(this->result); }

  /// Return a pointer to the error value of this result, if the call failed.
  const fastly_error_t *to_err() const {
    return reinterpret_cast<const fastly_error_t *>(std::get_if<Error>(&this->result));
  }

  /// Assume the call was successful, and return a reference to the result.
  T &unwrap() { return std::get<T>(this->result); }
};

/// A string allocated by the host interface. Holds ownership of the data.
struct HostString final {
  JS::UniqueChars ptr;
  size_t len;

  HostString() = default;
  explicit HostString(fastly_world_string_t str) : ptr{str.ptr}, len{str.len} {}
  HostString(JS::UniqueChars ptr, size_t len) : ptr{std::move(ptr)}, len{len} {}

  using iterator = char *;
  using const_iterator = const char *;

  size_t size() const { return this->len; }

  iterator begin() { return this->ptr.get(); }
  iterator end() { return this->begin() + this->len; }

  const_iterator begin() const { return this->ptr.get(); }
  const_iterator end() const { return this->begin() + this->len; }

  /// Conversion to a `std::string_view`.
  operator std::string_view() const { return std::string_view(this->ptr.get(), this->len); }
};

struct HostBytes final {
  std::unique_ptr<uint8_t[]> ptr;
  size_t len;

  HostBytes() = default;
  explicit HostBytes(fastly_world_list_u8_t bytes) : ptr{bytes.ptr}, len{bytes.len} {}

  using iterator = uint8_t *;
  using const_iterator = const uint8_t *;

  size_t size() const { return this->len; }

  iterator begin() { return this->ptr.get(); }
  iterator end() { return this->begin() + this->len; }

  const_iterator begin() const { return this->ptr.get(); }
  const_iterator end() const { return this->begin() + this->len; }

  /// Converstion to a `std::span<uint8_t>`.
  operator std::span<uint8_t>() const { return std::span{this->ptr.get(), this->len}; }
};

/// Common methods for async handles.
class AsyncHandle {
public:
  static constexpr fastly_async_handle_t invalid = UINT32_MAX - 1;

  fastly_async_handle_t handle;

  AsyncHandle() = default;
  explicit AsyncHandle(fastly_async_handle_t handle) : handle{handle} {}

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

/// A convenience wrapper for the host calls involving http bodies.
class HttpBody final {
public:
  static constexpr fastly_body_handle_t invalid = UINT32_MAX - 1;

  /// The handle to use when making host calls, initialized to the special invalid value used by
  /// executed.
  fastly_body_handle_t handle = invalid;

  HttpBody() = default;
  explicit HttpBody(fastly_body_handle_t handle) : handle{handle} {}
  explicit HttpBody(AsyncHandle async) : handle{async.handle} {}

  /// Returns true when this body handle is valid.
  bool valid() const { return this->handle != invalid; }

  /// Make a new body handle.
  static Result<HttpBody> make();

  /// Read a chunk from this handle.
  Result<HostString> read(uint32_t chunk_size) const;

  /// Write a chunk to this handle.
  Result<uint32_t> write(const uint8_t *bytes, size_t len) const;

  /// Writes the given number of bytes from the given buffer to the given handle.
  ///
  /// The host doesn't necessarily write all bytes in any particular call to
  /// `write`, so to ensure all bytes are written, we call it in a loop.
  Result<Void> write_all(const uint8_t *bytes, size_t len) const;

  /// Append another HttpBody to this one.
  Result<Void> append(HttpBody other) const;

  /// Close this handle, and reset internal state to invalid.
  Result<Void> close();

  AsyncHandle async_handle() const;
};

struct Response;

class HttpPendingReq final {
public:
  static constexpr fastly_pending_request_handle_t invalid = UINT32_MAX - 1;

  fastly_pending_request_handle_t handle = invalid;

  HttpPendingReq() = default;
  explicit HttpPendingReq(fastly_pending_request_handle_t handle) : handle{handle} {}
  explicit HttpPendingReq(AsyncHandle async) : handle{async.handle} {}

  /// Poll for the response to this request.
  Result<std::optional<Response>> poll();

  /// Block until the response is ready.
  Result<Response> wait();

  /// Fetch the AsyncHandle for this pending request.
  AsyncHandle async_handle() const;
};

class HttpBase {
public:
  virtual ~HttpBase() = default;

  virtual bool is_valid() const = 0;

  /// Get the http version used for this request.
  virtual Result<fastly_http_version_t> get_version() const = 0;

  virtual Result<std::vector<HostString>> get_header_names() = 0;
  virtual Result<std::optional<std::vector<HostString>>>
  get_header_values(std::string_view name) = 0;
  virtual Result<Void> insert_header(std::string_view name, std::string_view value) = 0;
  virtual Result<Void> append_header(std::string_view name, std::string_view value) = 0;
  virtual Result<Void> remove_header(std::string_view name) = 0;
};

struct BackendConfig {
  std::optional<HostString> host_override;
  std::optional<uint32_t> connect_timeout;
  std::optional<uint32_t> first_byte_timeout;
  std::optional<uint32_t> between_bytes_timeout;
  std::optional<bool> use_ssl;
  std::optional<fastly_tls_version_t> ssl_min_version;
  std::optional<fastly_tls_version_t> ssl_max_version;
  std::optional<HostString> cert_hostname;
  std::optional<HostString> ca_cert;
  std::optional<HostString> ciphers;
  std::optional<HostString> sni_hostname;
};

class HttpReq final : public HttpBase {
public:
  static constexpr fastly_request_handle_t invalid = UINT32_MAX - 1;

  fastly_request_handle_t handle = invalid;

  HttpReq() = default;
  explicit HttpReq(fastly_request_handle_t handle) : handle{handle} {}

  static Result<HttpReq> make();

  static Result<Void> redirect_to_grip_proxy(std::string_view backend);

  static Result<Void> register_dynamic_backend(std::string_view name, std::string_view target,
                                               const BackendConfig &config);

  /// Get the downstream ip address.
  static Result<HostBytes> downstream_client_ip_addr();

  static Result<HostString> http_req_downstream_tls_cipher_openssl_name();

  static Result<HostString> http_req_downstream_tls_protocol();

  static Result<HostBytes> http_req_downstream_tls_client_hello();

  static Result<HostBytes> http_req_downstream_tls_raw_client_certificate();

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
  Result<Void> cache_override(fastly_http_cache_override_tag_t tag, std::optional<uint32_t> ttl,
                              std::optional<uint32_t> stale_while_revalidate,
                              std::optional<std::string_view> surrogate_key);

  bool is_valid() const override;

  Result<fastly_http_version_t> get_version() const override;

  Result<std::vector<HostString>> get_header_names() override;
  Result<std::optional<std::vector<HostString>>> get_header_values(std::string_view name) override;
  Result<Void> insert_header(std::string_view name, std::string_view value) override;
  Result<Void> append_header(std::string_view name, std::string_view value) override;
  Result<Void> remove_header(std::string_view name) override;
};

class HttpResp final : public HttpBase {
public:
  static constexpr fastly_response_handle_t invalid = UINT32_MAX - 1;

  fastly_response_handle_t handle = invalid;

  HttpResp() = default;
  explicit HttpResp(fastly_response_handle_t handle) : handle{handle} {}

  static Result<HttpResp> make();

  /// Get the http status for the response.
  Result<uint16_t> get_status() const;

  /// Set the http status for the response.
  Result<Void> set_status(uint16_t status);

  /// Immediately begin sending this response to the downstream client.
  Result<Void> send_downstream(HttpBody body, bool streaming);

  bool is_valid() const override;

  Result<fastly_http_version_t> get_version() const override;

  Result<std::vector<HostString>> get_header_names() override;
  Result<std::optional<std::vector<HostString>>> get_header_values(std::string_view name) override;
  Result<Void> insert_header(std::string_view name, std::string_view value) override;
  Result<Void> append_header(std::string_view name, std::string_view value) override;
  Result<Void> remove_header(std::string_view name) override;
};

/// The pair of a response and its body.
struct Response {
  HttpResp resp;
  HttpBody body;

  Response() = default;
  explicit Response(fastly_response_t resp) : resp{HttpResp{resp.f0}}, body{HttpBody{resp.f1}} {}
};

class GeoIp final {
  ~GeoIp() = delete;

public:
  /// Lookup information about the ip address provided.
  static Result<HostString> lookup(std::span<uint8_t> bytes);
};

class LogEndpoint final {
public:
  fastly_log_endpoint_handle_t handle = UINT32_MAX - 1;

  LogEndpoint() = default;
  explicit LogEndpoint(fastly_log_endpoint_handle_t handle) : handle{handle} {}

  static Result<LogEndpoint> get(std::string_view name);

  Result<Void> write(std::string_view msg);
};

class Dict final {
public:
  fastly_dictionary_handle_t handle = UINT32_MAX - 1;

  Dict() = default;
  explicit Dict(fastly_dictionary_handle_t handle) : handle{handle} {}

  static Result<Dict> open(std::string_view name);

  Result<std::optional<HostString>> get(std::string_view name);
};

class ObjectStore final {
public:
  fastly_object_store_handle_t handle = UINT32_MAX - 1;

  ObjectStore() = default;
  explicit ObjectStore(fastly_object_store_handle_t handle) : handle{handle} {}

  static Result<ObjectStore> open(std::string_view name);

  Result<std::optional<HttpBody>> lookup(std::string_view name);

  Result<Void> insert(std::string_view name, HttpBody body);
};

namespace host_api {

class Secret final {
public:
  fastly_secret_handle_t handle = UINT32_MAX - 1;

  Secret() = default;
  explicit Secret(fastly_secret_handle_t handle) : handle{handle} {}

  Result<std::optional<HostString>> plaintext() const;
};

class SecretStore final {
public:
  fastly_secret_store_handle_t handle = UINT32_MAX - 1;

  SecretStore() = default;
  explicit SecretStore(fastly_secret_store_handle_t handle) : handle{handle} {}

  static Result<SecretStore> open(std::string_view name);

  Result<std::optional<Secret>> get(std::string_view name);
};

} // namespace host_api

#endif
