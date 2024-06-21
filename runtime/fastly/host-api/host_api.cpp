#include <algorithm>
#include <type_traits>

#include "../../StarlingMonkey/runtime/allocator.h"
#include "../../StarlingMonkey/runtime/encode.h"
#include "./component/fastly_world.h"
#include "./fastly.h"
#include "./host_api_fastly.h"

#include <algorithm>
#include <arpa/inet.h>

#include <time.h>

using api::FastlyResult;
using fastly::FastlyAPIError;
using host_api::MonotonicClock;
using host_api::Result;

#define NEVER_HANDLE 0xFFFFFFFE

#define MILLISECS_IN_NANOSECS 1000000
#define SECS_IN_NANOSECS 1000000000

void sleep_until(uint64_t time_ns, uint64_t now) {
  while (time_ns > now) {
    uint64_t duration = time_ns - now;
    timespec req{.tv_sec = static_cast<time_t>(duration / SECS_IN_NANOSECS),
                 .tv_nsec = static_cast<long>(duration % SECS_IN_NANOSECS)};
    timespec rem;
    nanosleep(&req, &rem);
    now = MonotonicClock::now();
  }
}

size_t api::AsyncTask::select(std::vector<api::AsyncTask *> *tasks) {
  size_t tasks_len = tasks->size();
  std::vector<fastly_compute_at_edge_async_io_handle_t> handles;
  handles.reserve(tasks_len);
  uint64_t now = 0;
  uint64_t soonest_deadline = 0;
  size_t soonest_deadline_idx = -1;
  for (size_t idx = 0; idx < tasks_len; ++idx) {
    auto *task = tasks->at(idx);
    uint64_t deadline = task->deadline();
    // Select for completed task deadlines before performing the task select host call.
    if (deadline > 0) {
      MOZ_ASSERT(task->id() == NEVER_HANDLE);
      if (now == 0) {
        now = MonotonicClock::now();
        MOZ_ASSERT(now > 0);
      }
      if (deadline <= now) {
        return idx;
      }
      if (soonest_deadline == 0 || deadline < soonest_deadline) {
        soonest_deadline = deadline;
        soonest_deadline_idx = idx;
      }
    } else {
      uint32_t handle = task->id();
      // Timer task handles are skipped and never passed to the host.
      MOZ_ASSERT(handle != NEVER_HANDLE);
      handles.push_back(handle);
    }
  }

  // When there are no async tasks, sleep until the deadline
  if (handles.size() == 0) {
    MOZ_ASSERT(soonest_deadline > 0);
    sleep_until(soonest_deadline, now);
    return soonest_deadline_idx;
  }

  fastly_world_list_handle_t hs{.ptr = handles.data(), .len = handles.size()};
  fastly_world_option_u32_t ret;
  fastly_compute_at_edge_types_error_t err = 0;

  while (true) {
    if (!fastly_compute_at_edge_async_io_select(
            &hs, (soonest_deadline - now) / MILLISECS_IN_NANOSECS, &ret, &err)) {
      abort();
    } else if (ret.is_some) {
      // The host index will be the index in the list of tasks with the timer tasks filtered out.
      // We thus need to offset the host index by any timer tasks appearing before the nth
      // non-timer task.
      size_t task_idx = 0;
      for (size_t idx = 0; idx < tasks_len; ++idx) {
        if (tasks->at(idx)->id() != NEVER_HANDLE) {
          if (ret.val == task_idx) {
            return idx;
          }
          task_idx++;
        }
      }
      abort();
    } else {
      // No value case means a timeout, which means soonest_deadline_idx is set.
      MOZ_ASSERT(soonest_deadline > 0);
      MOZ_ASSERT(soonest_deadline_idx != -1);
      // Verify that the task definitely is ready from a time perspective, and if not loop the host
      // call again.
      now = MonotonicClock::now();
      if (soonest_deadline > now) {
        continue;
      }
      return soonest_deadline_idx;
    }
  }
}

namespace host_api {

namespace {

fastly_world_list_u8_t span_to_list_u8(std::span<uint8_t> span) {
  return {
      .ptr = const_cast<uint8_t *>(span.data()),
      .len = span.size(),
  };
}

fastly_world_string_t string_view_to_world_string(std::string_view str) {
  return {
      .ptr = const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(str.data())),
      .len = str.size(),
  };
}

HostString make_host_string(fastly_world_string_t str) {
  return HostString{JS::UniqueChars{reinterpret_cast<char *>(str.ptr)}, str.len};
}

HostBytes make_host_bytes(fastly_world_list_u8_t str) {
  return HostBytes{std::unique_ptr<uint8_t[]>{str.ptr}, str.len};
}

Response make_response(fastly_compute_at_edge_http_types_response_t &resp) {
  return Response{HttpResp{resp.f0}, HttpBody{resp.f1}};
}

} // namespace

// --- <StarlingMonkey HOST API> ---
Result<HostBytes> Random::get_bytes(size_t num_bytes) {
  Result<HostBytes> res;

  auto ret = HostBytes::with_capacity(num_bytes);
  auto err =
      fastly::random_get(reinterpret_cast<uint32_t>(static_cast<void *>(ret.begin())), num_bytes);
  if (err != 0) {
    res.emplace_err(err);
  } else {
    res.emplace(std::move(ret));
  }

  return res;
}

Result<uint32_t> Random::get_u32() {
  Result<uint32_t> res;

  uint32_t storage;
  auto err = fastly::random_get(reinterpret_cast<uint32_t>(static_cast<void *>(&storage)),
                                sizeof(storage));
  if (err != 0) {
    res.emplace_err(err);
  } else {
    res.emplace(storage);
  }

  return res;
}

uint64_t MonotonicClock::now() { return JS_Now() * 1000; }

uint64_t MonotonicClock::resolution() { return 1000; }

int32_t MonotonicClock::subscribe(const uint64_t when, const bool absolute) { return NEVER_HANDLE; }

void MonotonicClock::unsubscribe(const int32_t handle_id) {}

// --- </StarlingMonkey Host API> ---

// The host interface makes the assumption regularly that uint32_t is sufficient space to store a
// pointer.
static_assert(sizeof(uint32_t) == sizeof(void *));

static_assert(std::is_same_v<HttpVersion, fastly_compute_at_edge_http_types_http_version_t>);
static_assert(std::is_same_v<typeof(CacheOverrideTag::value),
                             fastly_compute_at_edge_http_req_cache_override_tag_t>);
static_assert(
    std::is_same_v<typeof(TlsVersion::value), fastly_compute_at_edge_http_types_tls_version_t>);

static_assert(
    std::is_same_v<typeof(CacheState::state), fastly_compute_at_edge_cache_lookup_state_t>);
static_assert(
    std::is_same_v<typeof(BackendHealth::state), fastly_compute_at_edge_backend_backend_health_t>);

namespace {

FastlySendError
make_fastly_send_error(fastly_compute_at_edge_http_req_send_error_detail_t &send_error_detail) {
  FastlySendError res;

  switch (send_error_detail.tag) {
  case 0: {
    res.tag = FastlySendError::detail::uninitialized;
    break;
  }
  case 1: {
    res.tag = FastlySendError::detail::ok;
    break;
  }
  case 2: {
    res.tag = FastlySendError::detail::dns_timeout;
    break;
  }
  case 3: {
    res.tag = FastlySendError::detail::dns_error;
    break;
  }
  case 4: {
    res.tag = FastlySendError::detail::destination_not_found;
    break;
  }
  case 5: {
    res.tag = FastlySendError::detail::destination_unavailable;
    break;
  }
  case 6: {
    res.tag = FastlySendError::detail::destination_ip_unroutable;
    break;
  }
  case 7: {
    res.tag = FastlySendError::detail::connection_refused;
    break;
  }
  case 8: {
    res.tag = FastlySendError::detail::connection_terminated;
    break;
  }
  case 9: {
    res.tag = FastlySendError::detail::connection_timeout;
    break;
  }
  case 10: {
    res.tag = FastlySendError::detail::connection_limit_reached;
    break;
  }
  case 11: {
    res.tag = FastlySendError::detail::tls_certificate_error;
    break;
  }
  case 12: {
    res.tag = FastlySendError::detail::tls_configuration_error;
    break;
  }
  case 13: {
    res.tag = FastlySendError::detail::http_incomplete_response;
    break;
  }
  case 14: {
    res.tag = FastlySendError::detail::http_response_header_section_too_large;
    break;
  }
  case 15: {
    res.tag = FastlySendError::detail::http_response_body_too_large;
    break;
  }
  case 16: {
    res.tag = FastlySendError::detail::http_response_timeout;
    break;
  }
  case 17: {
    res.tag = FastlySendError::detail::http_response_status_invalid;
    break;
  }
  case 18: {
    res.tag = FastlySendError::detail::http_upgrade_failed;
    break;
  }
  case 19: {
    res.tag = FastlySendError::detail::http_protocol_error;
    break;
  }
  case 20: {
    res.tag = FastlySendError::detail::http_request_cache_key_invalid;
    break;
  }
  case 21: {
    res.tag = FastlySendError::detail::http_request_uri_invalid;
    break;
  }
  case 22: {
    res.tag = FastlySendError::detail::internal_error;
    break;
  }
  case 23: {
    res.tag = FastlySendError::detail::tls_alert_received;
    break;
  }
  case 24: {
    res.tag = FastlySendError::detail::tls_protocol_error;
    break;
  }
  default: {
    // If we are here, this is either because the host does not provided send error details
    // Or a new error detail tag exists and we don't yet have it implemented
    res.tag = FastlySendError::detail::uninitialized;
  }
  }

  res.dns_error_rcode = send_error_detail.dns_error_rcode;
  res.dns_error_info_code = send_error_detail.dns_error_info_code;
  res.tls_alert_id = send_error_detail.tls_alert_id;

  return res;
}

} // namespace

JSString *get_geo_info(JSContext *cx, JS::HandleString address_str) {
  auto address = core::encode(cx, address_str);
  if (!address) {
    return nullptr;
  }

  // TODO: Remove all of this and rely on the host for validation as the hostcall only takes one
  // user-supplied parameter
  int format = AF_INET;
  size_t octets_len = 4;
  if (std::find(address.begin(), address.end(), ':') != address.end()) {
    format = AF_INET6;
    octets_len = 16;
  }

  uint8_t octets[sizeof(struct in6_addr)];
  if (inet_pton(format, address.begin(), octets) != 1) {
    // While get_geo_info can be invoked through FetchEvent#client.geo, too,
    // that path can't result in an invalid address here, so we can be more
    // specific in the error message.
    // TODO: Make a TypeError
    JS_ReportErrorLatin1(cx, "Invalid address passed to fastly.getGeolocationForIpAddress");
    return nullptr;
  }

  auto res = host_api::GeoIp::lookup(std::span<uint8_t>{octets, octets_len});
  if (auto *err = res.to_err()) {
    HANDLE_ERROR(cx, *err);
    return nullptr;
  }

  auto ret = std::move(res.unwrap());

  return JS_NewStringCopyUTF8N(cx, JS::UTF8Chars(ret.ptr.release(), ret.len));
}

Result<HttpBody> HttpBody::make() {
  Result<HttpBody> res;

  fastly_compute_at_edge_http_types_body_handle_t handle;
  fastly_compute_at_edge_types_error_t err;
  if (!fastly_compute_at_edge_http_body_new(&handle, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(handle);
  }

  return res;
}

Result<HostString> HttpBody::read(uint32_t chunk_size) const {
  Result<HostString> res;

  fastly_world_list_u8_t ret;
  fastly_compute_at_edge_types_error_t err;
  if (!fastly_compute_at_edge_http_body_read(this->handle, chunk_size, &ret, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(JS::UniqueChars(reinterpret_cast<char *>(ret.ptr)), ret.len);
  }

  return res;
}

Result<uint32_t> HttpBody::write_front(const uint8_t *ptr, size_t len) const {
  Result<uint32_t> res;

  // The write call doesn't mutate the buffer; the cast is just for the generated fastly api.
  fastly_world_list_u8_t chunk{const_cast<uint8_t *>(ptr), len};

  fastly_compute_at_edge_types_error_t err;
  uint32_t written;
  if (!fastly_compute_at_edge_http_body_write(
          this->handle, &chunk, FASTLY_COMPUTE_AT_EDGE_HTTP_BODY_WRITE_END_FRONT, &written, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(written);
  }

  return res;
}

Result<uint32_t> HttpBody::write_back(const uint8_t *ptr, size_t len) const {
  Result<uint32_t> res;

  // The write call doesn't mutate the buffer; the cast is just for the generated fastly api.
  fastly_world_list_u8_t chunk{const_cast<uint8_t *>(ptr), len};

  fastly_compute_at_edge_types_error_t err;
  uint32_t written;
  if (!fastly_compute_at_edge_http_body_write(
          this->handle, &chunk, FASTLY_COMPUTE_AT_EDGE_HTTP_BODY_WRITE_END_BACK, &written, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(written);
  }

  return res;
}

Result<Void> HttpBody::write_all_front(const uint8_t *ptr, size_t len) const {
  while (len > 0) {
    auto write_res = this->write_front(ptr, len);
    if (auto *err = write_res.to_err()) {
      return Result<Void>::err(*err);
    }

    auto written = write_res.unwrap();
    ptr += written;
    MOZ_ASSERT(written <= len);
    len -= static_cast<size_t>(written);
  }

  return Result<Void>::ok();
}

Result<Void> HttpBody::write_all_back(const uint8_t *ptr, size_t len) const {
  while (len > 0) {
    auto write_res = this->write_back(ptr, len);
    if (auto *err = write_res.to_err()) {
      return Result<Void>::err(*err);
    }

    auto written = write_res.unwrap();
    ptr += written;
    len -= std::min(len, static_cast<size_t>(written));
  }

  return Result<Void>::ok();
}

Result<Void> HttpBody::append(HttpBody other) const {
  Result<Void> res;

  fastly_compute_at_edge_types_error_t err;
  if (!fastly_compute_at_edge_http_body_append(this->handle, other.handle, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace();
  }

  return res;
}

Result<Void> HttpBody::close() {
  Result<Void> res;

  fastly_compute_at_edge_types_error_t err;
  if (!fastly_compute_at_edge_http_body_close(this->handle, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace();
  }

  return res;
}

FastlyHandle HttpBody::async_handle() const { return FastlyHandle{this->handle}; }

namespace {

template <auto header_names_get>
Result<std::vector<HostString>> generic_get_header_names(auto handle) {
  Result<std::vector<HostString>> res;

  fastly_world_list_string_t ret;
  fastly_compute_at_edge_types_error_t err;
  if (!header_names_get(handle, &ret, &err)) {
    res.emplace_err(err);
  } else {
    std::vector<HostString> names;

    for (int i = 0; i < ret.len; i++) {
      names.emplace_back(make_host_string(ret.ptr[i]));
    }

    // Free the vector of string pointers, but leave the individual strings alone.
    cabi_free(ret.ptr);

    res.emplace(std::move(names));
  }

  return res;
}

template <auto header_values_get>
Result<std::optional<std::vector<HostBytes>>> generic_get_header_values(auto handle,
                                                                        std::string_view name) {
  Result<std::optional<std::vector<HostBytes>>> res;

  fastly_world_string_t hdr = string_view_to_world_string(name);
  fastly_world_option_list_list_u8_t ret;
  fastly_compute_at_edge_types_error_t err;
  if (!header_values_get(handle, &hdr, &ret, &err)) {
    res.emplace_err(err);
  } else {

    if (ret.is_some) {
      std::vector<HostBytes> names;

      for (int i = 0; i < ret.val.len; i++) {
        names.emplace_back(make_host_bytes(ret.val.ptr[i]));
      }

      // Free the vector of string pointers, but leave the individual strings alone.
      cabi_free(ret.val.ptr);

      res.emplace(std::move(names));
    } else {
      res.emplace(std::nullopt);
    }
  }

  return res;
}

template <auto header_op>
Result<Void> generic_header_op(auto handle, std::string_view name, std::span<uint8_t> value) {
  Result<Void> res;

  fastly_world_string_t hdr = string_view_to_world_string(name);
  fastly_world_list_u8_t val = span_to_list_u8(value);
  fastly_compute_at_edge_types_error_t err;
  if (!header_op(handle, &hdr, &val, &err)) {
    res.emplace_err(err);
  }

  return res;
}

template <auto remove_header>
Result<Void> generic_header_remove(auto handle, std::string_view name) {
  Result<Void> res;

  fastly_world_string_t hdr = string_view_to_world_string(name);
  fastly_compute_at_edge_types_error_t err;
  if (!remove_header(handle, &hdr, &err)) {
    res.emplace_err(err);
  }

  return res;
}

} // namespace

Result<std::optional<Response>> HttpPendingReq::poll() {
  Result<std::optional<Response>> res;

  fastly_compute_at_edge_types_error_t err;
  fastly_world_option_response_t ret;
  if (!fastly_compute_at_edge_http_req_pending_req_poll(this->handle, &ret, &err)) {
    res.emplace_err(err);
  } else if (ret.is_some) {
    res.emplace(make_response(ret.val));
  } else {
    res.emplace(std::nullopt);
  }

  return res;
}

api::FastlyResult<Response, FastlySendError> HttpPendingReq::wait() {
  api::FastlyResult<Response, FastlySendError> res;

  fastly_compute_at_edge_http_req_send_error_detail_t s;
  std::memset(&s, 0, sizeof(s));
  fastly_compute_at_edge_http_req_error_t err;
  fastly_compute_at_edge_http_types_response_t ret;
  if (!fastly_compute_at_edge_http_req_pending_req_wait_v2(this->handle, &s, &ret, &err)) {
    res.emplace_err(make_fastly_send_error(s));
  } else {
    res.emplace(make_response(ret));
  }

  return res;
}

FastlyHandle HttpPendingReq::async_handle() const { return FastlyHandle{this->handle}; }

void CacheOverrideTag::set_pass() {
  this->value |= FASTLY_COMPUTE_AT_EDGE_HTTP_REQ_CACHE_OVERRIDE_TAG_PASS;
}

void CacheOverrideTag::set_ttl() {
  this->value |= FASTLY_COMPUTE_AT_EDGE_HTTP_REQ_CACHE_OVERRIDE_TAG_TTL;
}

void CacheOverrideTag::set_stale_while_revalidate() {
  this->value |= FASTLY_COMPUTE_AT_EDGE_HTTP_REQ_CACHE_OVERRIDE_TAG_STALE_WHILE_REVALIDATE;
}

void CacheOverrideTag::set_pci() {
  this->value |= FASTLY_COMPUTE_AT_EDGE_HTTP_REQ_CACHE_OVERRIDE_TAG_PCI;
}

TlsVersion::TlsVersion(uint8_t raw) : value{raw} {
  switch (raw) {
  case FASTLY_COMPUTE_AT_EDGE_HTTP_TYPES_TLS_VERSION_TLS1:
  case FASTLY_COMPUTE_AT_EDGE_HTTP_TYPES_TLS_VERSION_TLS11:
  case FASTLY_COMPUTE_AT_EDGE_HTTP_TYPES_TLS_VERSION_TLS12:
  case FASTLY_COMPUTE_AT_EDGE_HTTP_TYPES_TLS_VERSION_TLS13:
    break;

  default:
    MOZ_ASSERT(false, "Making a TlsValue from an invalid raw value");
  }
}

TlsVersion TlsVersion::version_1() {
  return TlsVersion{FASTLY_COMPUTE_AT_EDGE_HTTP_TYPES_TLS_VERSION_TLS1};
}

TlsVersion TlsVersion::version_1_1() {
  return TlsVersion{FASTLY_COMPUTE_AT_EDGE_HTTP_TYPES_TLS_VERSION_TLS11};
}

TlsVersion TlsVersion::version_1_2() {
  return TlsVersion{FASTLY_COMPUTE_AT_EDGE_HTTP_TYPES_TLS_VERSION_TLS12};
}

TlsVersion TlsVersion::version_1_3() {
  return TlsVersion{FASTLY_COMPUTE_AT_EDGE_HTTP_TYPES_TLS_VERSION_TLS13};
}

Result<HttpReq> HttpReq::make() {
  Result<HttpReq> res;

  fastly_compute_at_edge_http_types_request_handle_t handle;
  fastly_compute_at_edge_types_error_t err;
  if (!fastly_compute_at_edge_http_req_new(&handle, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(handle);
  }

  return res;
}

Result<Void> HttpReq::redirect_to_grip_proxy(std::string_view backend) {
  Result<Void> res;

  fastly_compute_at_edge_types_error_t err;
  fastly_world_string_t backend_str = string_view_to_world_string(backend);
  if (!fastly_compute_at_edge_http_req_redirect_to_grip_proxy(&backend_str, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace();
  }

  return res;
}

Result<Void> HttpReq::auto_decompress_gzip() {
  Result<Void> res;

  fastly_compute_at_edge_types_error_t err;
  fastly_compute_at_edge_http_types_content_encodings_t encodings_to_decompress = 0;
  encodings_to_decompress |= FASTLY_COMPUTE_AT_EDGE_HTTP_TYPES_CONTENT_ENCODINGS_GZIP;
  if (!fastly_compute_at_edge_http_req_auto_decompress_response_set(
          this->handle, encodings_to_decompress, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace();
  }

  return res;
}

Result<Void> HttpReq::register_dynamic_backend(std::string_view name, std::string_view target,
                                               const BackendConfig &config) {
  Result<Void> res;

  fastly_compute_at_edge_http_types_dynamic_backend_config_t backend_config;
  memset(&backend_config, 0, sizeof(backend_config));

  if (auto &val = config.host_override) {
    backend_config.host_override.is_some = true;
    backend_config.host_override.val = string_view_to_world_string(*val);
  }

  if (auto &val = config.connect_timeout) {
    backend_config.connect_timeout.is_some = true;
    backend_config.connect_timeout.val = *val;
  }

  if (auto &val = config.first_byte_timeout) {
    backend_config.first_byte_timeout.is_some = true;
    backend_config.first_byte_timeout.val = *val;
  }

  if (auto &val = config.between_bytes_timeout) {
    backend_config.between_bytes_timeout.is_some = true;
    backend_config.between_bytes_timeout.val = *val;
  }

  if (auto &val = config.use_ssl) {
    backend_config.use_ssl.is_some = true;
    backend_config.use_ssl.val = *val;
  }

  if (auto &val = config.dont_pool) {
    backend_config.dont_pool.is_some = true;
    backend_config.dont_pool.val = *val;
  }

  if (auto &val = config.ssl_min_version) {
    backend_config.ssl_min_version.is_some = true;
    backend_config.ssl_min_version.val = val->value;
  }

  if (auto &val = config.ssl_max_version) {
    backend_config.ssl_max_version.is_some = true;
    backend_config.ssl_max_version.val = val->value;
  }

  if (auto &val = config.cert_hostname) {
    backend_config.cert_hostname.is_some = true;
    backend_config.cert_hostname.val = string_view_to_world_string(*val);
  }

  if (auto &val = config.ca_cert) {
    backend_config.ca_cert.is_some = true;
    backend_config.ca_cert.val = string_view_to_world_string(*val);
  }

  if (auto &val = config.ciphers) {
    backend_config.ciphers.is_some = true;
    backend_config.ciphers.val = string_view_to_world_string(*val);
  }

  if (auto &val = config.sni_hostname) {
    backend_config.sni_hostname.is_some = true;
    backend_config.sni_hostname.val = string_view_to_world_string(*val);
  }

  if (auto &val = config.client_cert) {
    backend_config.client_cert.is_some = true;
    backend_config.client_cert.val.client_cert = string_view_to_world_string(val->cert);
    backend_config.client_cert.val.client_key = val->key;
  }

  auto name_str = string_view_to_world_string(name);
  auto target_str = string_view_to_world_string(target);
  fastly_compute_at_edge_types_error_t err;
  if (!fastly_compute_at_edge_http_req_register_dynamic_backend(&name_str, &target_str,
                                                                &backend_config, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace();
  }

  return res;
}

Result<Response> HttpReq::send(HttpBody body, std::string_view backend) {
  Result<Response> res;

  fastly_compute_at_edge_types_error_t err;
  fastly_compute_at_edge_http_types_response_t ret;
  fastly_world_string_t backend_str = string_view_to_world_string(backend);
  if (!fastly_compute_at_edge_http_req_send(this->handle, body.handle, &backend_str, &ret, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(make_response(ret));
  }

  return res;
}

Result<HttpPendingReq> HttpReq::send_async(HttpBody body, std::string_view backend) {
  Result<HttpPendingReq> res;

  fastly_compute_at_edge_types_error_t err;
  fastly_compute_at_edge_http_types_pending_request_handle_t ret;
  fastly_world_string_t backend_str = string_view_to_world_string(backend);
  if (!fastly_compute_at_edge_http_req_send_async(this->handle, body.handle, &backend_str, &ret,
                                                  &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(ret);
  }

  return res;
}

Result<HttpPendingReq> HttpReq::send_async_streaming(HttpBody body, std::string_view backend) {
  Result<HttpPendingReq> res;

  fastly_compute_at_edge_types_error_t err;
  fastly_compute_at_edge_http_types_pending_request_handle_t ret;
  fastly_world_string_t backend_str = string_view_to_world_string(backend);
  if (!fastly_compute_at_edge_http_req_send_async_streaming(this->handle, body.handle, &backend_str,
                                                            &ret, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(ret);
  }

  return res;
}

Result<Void> HttpReq::set_method(std::string_view method) {
  Result<Void> res;

  fastly_compute_at_edge_types_error_t err;
  fastly_world_string_t str = string_view_to_world_string(method);
  if (!fastly_compute_at_edge_http_req_method_set(this->handle, &str, &err)) {
    res.emplace_err(err);
  }

  return res;
}

Result<HostString> HttpReq::get_method() const {
  Result<HostString> res;

  fastly_compute_at_edge_types_error_t err;
  fastly_world_string_t ret;
  if (!fastly_compute_at_edge_http_req_method_get(this->handle, &ret, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(make_host_string(ret));
  }

  return res;
}

Result<Void> HttpReq::set_uri(std::string_view str) {
  Result<Void> res;

  fastly_compute_at_edge_types_error_t err;
  fastly_world_string_t uri = string_view_to_world_string(str);
  if (!fastly_compute_at_edge_http_req_uri_set(this->handle, &uri, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace();
  }

  return res;
}

Result<HostString> HttpReq::get_uri() const {
  Result<HostString> res;

  fastly_compute_at_edge_types_error_t err;
  fastly_world_string_t uri;
  if (!fastly_compute_at_edge_http_req_uri_get(this->handle, &uri, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(make_host_string(uri));
  }

  return res;
}

Result<Void> HttpReq::cache_override(CacheOverrideTag tag, std::optional<uint32_t> opt_ttl,
                                     std::optional<uint32_t> opt_swr,
                                     std::optional<std::string_view> opt_sk) {
  Result<Void> res;

  uint32_t *ttl = nullptr;
  if (opt_ttl.has_value()) {
    ttl = &opt_ttl.value();
  }

  uint32_t *swr = nullptr;
  if (opt_swr.has_value()) {
    swr = &opt_swr.value();
  }

  fastly_world_string_t sk{nullptr, 0};
  if (opt_sk.has_value()) {
    sk = string_view_to_world_string(opt_sk.value());
  }

  fastly_compute_at_edge_types_error_t err;
  if (!fastly_compute_at_edge_http_req_cache_override_set(this->handle, tag.value, ttl, swr, &sk,
                                                          &err)) {
    res.emplace_err(err);
  } else {
    res.emplace();
  }

  return res;
}

Result<Void> HttpReq::set_framing_headers_mode(FramingHeadersMode mode) {
  Result<Void> res;

  fastly_compute_at_edge_http_req_framing_headers_mode_t m;

  switch (mode) {
  case FramingHeadersMode::Automatic:
    m = FASTLY_COMPUTE_AT_EDGE_HTTP_TYPES_FRAMING_HEADERS_MODE_AUTOMATIC;
    break;
  case FramingHeadersMode::ManuallyFromHeaders:
    m = FASTLY_COMPUTE_AT_EDGE_HTTP_TYPES_FRAMING_HEADERS_MODE_MANUALLY_FROM_HEADERS;
    break;
  }

  fastly_compute_at_edge_types_error_t err;
  if (!fastly_compute_at_edge_http_req_framing_headers_mode_set(this->handle, m, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace();
  }

  return res;
}

Result<HostBytes> HttpReq::downstream_client_ip_addr() {
  Result<HostBytes> res;

  fastly_world_list_u8_t octets;
  fastly_compute_at_edge_types_error_t err;
  if (!fastly_compute_at_edge_http_req_downstream_client_ip_addr(&octets, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(make_host_bytes(octets));
  }

  return res;
}

// http-req-downstream-tls-cipher-openssl-name: func() -> result<string, error>
Result<HostString> HttpReq::http_req_downstream_tls_cipher_openssl_name() {
  Result<HostString> res;

  fastly_compute_at_edge_types_error_t err;
  fastly_world_string_t ret;
  if (!fastly_compute_at_edge_http_req_downstream_tls_cipher_openssl_name(&ret, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(make_host_string(ret));
  }

  return res;
}

// http-req-downstream-tls-protocol: func() -> result<string, error>
Result<HostString> HttpReq::http_req_downstream_tls_protocol() {
  Result<HostString> res;

  fastly_compute_at_edge_types_error_t err;
  fastly_world_string_t ret;
  if (!fastly_compute_at_edge_http_req_downstream_tls_protocol(&ret, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(make_host_string(ret));
  }

  return res;
}

// http-req-downstream-tls-client-hello: func() -> result<list<u8>, error>
Result<HostBytes> HttpReq::http_req_downstream_tls_client_hello() {
  Result<HostBytes> res;

  fastly_world_list_u8_t ret;
  fastly_compute_at_edge_types_error_t err;
  if (!fastly_compute_at_edge_http_req_downstream_tls_client_hello(&ret, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(make_host_bytes(ret));
  }

  return res;
}

// http-req-downstream-tls-raw-client-certificate: func() -> result<list<u8>, error>
Result<HostBytes> HttpReq::http_req_downstream_tls_raw_client_certificate() {
  Result<HostBytes> res;

  fastly_world_list_u8_t ret;
  fastly_compute_at_edge_types_error_t err;
  if (!fastly_compute_at_edge_http_req_downstream_tls_raw_client_certificate(&ret, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(make_host_bytes(ret));
  }

  return res;
}

// http-req-downstream-tls-ja3-md5: func() -> result<list<u8>, error>
Result<HostBytes> HttpReq::http_req_downstream_tls_ja3_md5() {
  Result<HostBytes> res;

  fastly_world_list_u8_t ret;
  fastly_compute_at_edge_types_error_t err;
  if (!fastly_compute_at_edge_http_req_downstream_tls_ja3_md5(&ret, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(make_host_bytes(ret));
  }

  return res;
}

bool HttpReq::is_valid() const { return this->handle != HttpReq::invalid; }

Result<fastly_compute_at_edge_http_types_http_version_t> HttpReq::get_version() const {
  Result<fastly_compute_at_edge_http_types_http_version_t> res;

  fastly_compute_at_edge_types_error_t err;
  fastly_compute_at_edge_http_types_http_version_t ret;
  if (!fastly_compute_at_edge_http_req_version_get(this->handle, &ret, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(ret);
  }

  return res;
}

Result<std::vector<HostString>> HttpReq::get_header_names() {
  return generic_get_header_names<fastly_compute_at_edge_http_req_header_names_get>(this->handle);
}

Result<std::optional<std::vector<HostBytes>>> HttpReq::get_header_values(std::string_view name) {
  return generic_get_header_values<fastly_compute_at_edge_http_req_header_values_get>(this->handle,
                                                                                      name);
}

Result<Void> HttpReq::insert_header(std::string_view name, std::span<uint8_t> value) {
  return generic_header_op<fastly_compute_at_edge_http_req_header_insert>(this->handle, name,
                                                                          value);
}

Result<Void> HttpReq::append_header(std::string_view name, std::span<uint8_t> value) {
  return generic_header_op<fastly_compute_at_edge_http_req_header_append>(this->handle, name,
                                                                          value);
}

Result<Void> HttpReq::remove_header(std::string_view name) {
  return generic_header_remove<fastly_compute_at_edge_http_req_header_remove>(this->handle, name);
}

Result<HttpResp> HttpResp::make() {
  Result<HttpResp> res;

  fastly_compute_at_edge_http_types_response_handle_t handle;
  fastly_compute_at_edge_types_error_t err;
  if (!fastly_compute_at_edge_http_resp_new(&handle, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(handle);
  }

  return res;
}

Result<uint16_t> HttpResp::get_status() const {
  Result<uint16_t> res;

  fastly_compute_at_edge_http_types_http_status_t ret;
  fastly_compute_at_edge_types_error_t err;
  if (!fastly_compute_at_edge_http_resp_status_get(this->handle, &ret, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(ret);
  }

  return res;
}

Result<Void> HttpResp::set_status(uint16_t status) {
  Result<Void> res;

  fastly_compute_at_edge_types_error_t err;
  if (!fastly_compute_at_edge_http_resp_status_set(this->handle, status, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace();
  }

  return res;
}

Result<Void> HttpResp::send_downstream(HttpBody body, bool streaming) {
  Result<Void> res;

  fastly_compute_at_edge_types_error_t err;
  if (!fastly_compute_at_edge_http_resp_send_downstream(this->handle, body.handle, streaming,
                                                        &err)) {
    res.emplace_err(err);
  } else {
    res.emplace();
  }

  return res;
}

Result<Void> HttpResp::set_framing_headers_mode(FramingHeadersMode mode) {
  Result<Void> res;

  fastly_compute_at_edge_http_resp_framing_headers_mode_t m;

  switch (mode) {
  case FramingHeadersMode::Automatic:
    m = FASTLY_COMPUTE_AT_EDGE_HTTP_TYPES_FRAMING_HEADERS_MODE_AUTOMATIC;
    break;
  case FramingHeadersMode::ManuallyFromHeaders:
    m = FASTLY_COMPUTE_AT_EDGE_HTTP_TYPES_FRAMING_HEADERS_MODE_MANUALLY_FROM_HEADERS;
    break;
  }

  fastly_compute_at_edge_types_error_t err;
  if (!fastly_compute_at_edge_http_resp_framing_headers_mode_set(this->handle, m, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace();
  }

  return res;
}

bool HttpResp::is_valid() const { return this->handle != HttpResp::invalid; }

Result<fastly_compute_at_edge_http_types_http_version_t> HttpResp::get_version() const {
  Result<fastly_compute_at_edge_http_types_http_version_t> res;

  fastly_compute_at_edge_types_error_t err;
  fastly_compute_at_edge_http_types_http_version_t ret;
  if (!fastly_compute_at_edge_http_resp_version_get(this->handle, &ret, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(ret);
  }

  return res;
}

Result<std::vector<HostString>> HttpResp::get_header_names() {
  return generic_get_header_names<fastly_compute_at_edge_http_resp_header_names_get>(this->handle);
}

Result<std::optional<std::vector<HostBytes>>> HttpResp::get_header_values(std::string_view name) {
  return generic_get_header_values<fastly_compute_at_edge_http_resp_header_values_get>(this->handle,
                                                                                       name);
}

Result<Void> HttpResp::insert_header(std::string_view name, std::span<uint8_t> value) {
  return generic_header_op<fastly_compute_at_edge_http_resp_header_insert>(this->handle, name,
                                                                           value);
}

Result<Void> HttpResp::append_header(std::string_view name, std::span<uint8_t> value) {
  return generic_header_op<fastly_compute_at_edge_http_resp_header_append>(this->handle, name,
                                                                           value);
}

Result<Void> HttpResp::remove_header(std::string_view name) {
  return generic_header_remove<fastly_compute_at_edge_http_resp_header_remove>(this->handle, name);
}

Result<HostString> GeoIp::lookup(std::span<uint8_t> bytes) {
  Result<HostString> res;

  fastly_world_list_u8_t octets_list{const_cast<uint8_t *>(bytes.data()), bytes.size()};
  fastly_world_string_t ret;
  fastly_compute_at_edge_types_error_t err;
  if (!fastly_compute_at_edge_geo_lookup(&octets_list, &ret, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(make_host_string(ret));
  }

  return res;
}

Result<LogEndpoint> LogEndpoint::get(std::string_view name) {
  Result<LogEndpoint> res;

  auto name_str = string_view_to_world_string(name);
  fastly_compute_at_edge_log_handle_t handle;
  fastly_compute_at_edge_types_error_t err;
  if (!fastly_compute_at_edge_log_endpoint_get(&name_str, &handle, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(LogEndpoint{handle});
  }

  return res;
}

Result<Void> LogEndpoint::write(std::string_view msg) {
  Result<Void> res;

  auto msg_str = string_view_to_world_string(msg);
  fastly_compute_at_edge_types_error_t err;
  if (!fastly_compute_at_edge_log_write(this->handle, &msg_str, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace();
  }

  return res;
}

Result<Dict> Dict::open(std::string_view name) {
  Result<Dict> res;

  auto name_str = string_view_to_world_string(name);
  fastly_compute_at_edge_dictionary_handle_t ret;
  fastly_compute_at_edge_types_error_t err;
  if (!fastly_compute_at_edge_dictionary_open(&name_str, &ret, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(ret);
  }

  return res;
}

Result<std::optional<HostString>> Dict::get(std::string_view name) {
  Result<std::optional<HostString>> res;

  auto name_str = string_view_to_world_string(name);
  fastly_world_option_string_t ret;
  fastly_compute_at_edge_types_error_t err;
  if (!fastly_compute_at_edge_dictionary_get(this->handle, &name_str, &ret, &err)) {
    res.emplace_err(err);
  } else if (ret.is_some) {
    res.emplace(make_host_string(ret.val));
  } else {
    res.emplace(std::nullopt);
  }

  return res;
}

Result<ConfigStore> ConfigStore::open(std::string_view name) {
  Result<ConfigStore> res;

  auto name_str = string_view_to_world_string(name);
  fastly_compute_at_edge_config_store_handle_t ret;
  fastly_compute_at_edge_types_error_t err;
  if (!fastly_compute_at_edge_config_store_open(&name_str, &ret, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(ret);
  }

  return res;
}

Result<std::optional<HostString>> ConfigStore::get(std::string_view name) {
  Result<std::optional<HostString>> res;

  auto name_str = string_view_to_world_string(name);
  fastly_world_option_string_t ret;
  fastly_compute_at_edge_types_error_t err;
  if (!fastly_compute_at_edge_config_store_get(this->handle, &name_str, &ret, &err)) {
    res.emplace_err(err);
  } else if (ret.is_some) {
    res.emplace(make_host_string(ret.val));
  } else {
    res.emplace(std::nullopt);
  }

  return res;
}

Result<ObjectStore> ObjectStore::open(std::string_view name) {
  Result<ObjectStore> res;

  auto name_str = string_view_to_world_string(name);
  fastly_compute_at_edge_object_store_handle_t ret;
  fastly_compute_at_edge_types_error_t err;
  if (!fastly_compute_at_edge_object_store_open(&name_str, &ret, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(ret);
  }

  return res;
}

Result<std::optional<HttpBody>> ObjectStore::lookup(std::string_view name) {
  Result<std::optional<HttpBody>> res;

  auto name_str = string_view_to_world_string(name);
  fastly_world_option_body_handle_t ret;
  fastly_compute_at_edge_types_error_t err;
  if (!fastly_compute_at_edge_object_store_lookup(this->handle, &name_str, &ret, &err)) {
    res.emplace_err(err);
  } else if (ret.is_some) {
    res.emplace(ret.val);
  } else {
    res.emplace(std::nullopt);
  }

  return res;
}

Result<FastlyHandle> ObjectStore::lookup_async(std::string_view name) {
  Result<FastlyHandle> res;

  auto name_str = string_view_to_world_string(name);
  fastly_compute_at_edge_object_store_pending_handle_t ret;
  fastly_compute_at_edge_types_error_t err;
  if (!fastly_compute_at_edge_object_store_lookup_async(this->handle, &name_str, &ret, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(ret);
  }

  return res;
}

Result<FastlyHandle> ObjectStore::delete_async(std::string_view name) {
  Result<FastlyHandle> res;

  auto name_str = string_view_to_world_string(name);
  fastly_compute_at_edge_object_store_pending_handle_t ret;
  fastly_compute_at_edge_types_error_t err;
  if (!fastly_compute_at_edge_object_store_delete_async(this->handle, &name_str, &ret, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(ret);
  }

  return res;
}

Result<Void> ObjectStore::insert(std::string_view name, HttpBody body) {
  Result<Void> res;

  auto name_str = string_view_to_world_string(name);
  fastly_compute_at_edge_types_error_t err;
  if (!fastly_compute_at_edge_object_store_insert(this->handle, &name_str, body.handle, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace();
  }

  return res;
}

FastlyResult<std::optional<HttpBody>, FastlyAPIError> ObjectStorePendingLookup::wait() {
  FastlyResult<std::optional<HttpBody>, FastlyAPIError> res;

  fastly_compute_at_edge_types_error_t err;
  fastly_world_option_body_handle_t ret;
  if (!fastly_compute_at_edge_object_store_pending_lookup_wait(this->handle, &ret, &err)) {
    res.emplace_err(static_cast<FastlyAPIError>(err));
  } else if (ret.is_some) {
    res.emplace(ret.val);
  } else {
    res.emplace(std::nullopt);
  }

  return res;
}

FastlyHandle ObjectStorePendingLookup::async_handle() const { return FastlyHandle{this->handle}; }

Result<Void> ObjectStorePendingDelete::wait() {
  Result<Void> res;

  fastly_compute_at_edge_types_error_t err;
  if (!fastly_compute_at_edge_object_store_pending_delete_wait(this->handle, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(Void{});
  }

  return res;
}

FastlyHandle ObjectStorePendingDelete::async_handle() const { return FastlyHandle{this->handle}; }

static_assert(std::is_same_v<FastlyHandle, fastly_compute_at_edge_secret_store_secret_handle_t>);
static_assert(std::is_same_v<FastlyHandle, fastly_compute_at_edge_secret_store_store_handle_t>);

Result<std::optional<HostBytes>> Secret::plaintext() const {
  Result<std::optional<HostBytes>> res;

  fastly_world_option_list_u8_t ret;
  fastly_compute_at_edge_types_error_t err;
  if (!fastly_compute_at_edge_secret_store_plaintext(this->handle, &ret, &err)) {
    res.emplace_err(err);
  } else if (ret.is_some) {
    res.emplace(make_host_bytes(ret.val));
  } else {
    res.emplace(std::nullopt);
  }

  return res;
}

Result<SecretStore> SecretStore::open(std::string_view name) {
  Result<SecretStore> res;

  auto name_str = string_view_to_world_string(name);
  fastly_compute_at_edge_secret_store_store_handle_t ret;
  fastly_compute_at_edge_types_error_t err;
  if (!fastly_compute_at_edge_secret_store_open(&name_str, &ret, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(ret);
  }

  return res;
}

Result<std::optional<Secret>> SecretStore::get(std::string_view name) {
  Result<std::optional<Secret>> res;

  auto name_str = string_view_to_world_string(name);
  fastly_world_option_secret_handle_t ret;
  fastly_compute_at_edge_types_error_t err;
  if (!fastly_compute_at_edge_secret_store_get(this->handle, &name_str, &ret, &err)) {
    res.emplace_err(err);
  } else if (ret.is_some) {
    res.emplace(ret.val);
  } else {
    res.emplace(std::nullopt);
  }

  return res;
}

Result<Secret> SecretStore::from_bytes(uint8_t *bytes, size_t len) {
  Result<Secret> res;

  fastly_world_list_u8_t bytes_list{const_cast<uint8_t *>(bytes), len};
  fastly_compute_at_edge_secret_store_secret_handle_t ret;
  fastly_compute_at_edge_types_error_t err;
  if (!fastly_compute_at_edge_secret_store_from_bytes(&bytes_list, &ret, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(ret);
  }

  return res;
}

bool CacheState::is_found() const {
  return this->state & FASTLY_COMPUTE_AT_EDGE_CACHE_LOOKUP_STATE_FOUND;
}

bool CacheState::is_usable() const {
  return this->state & FASTLY_COMPUTE_AT_EDGE_CACHE_LOOKUP_STATE_USABLE;
}

bool CacheState::is_stale() const {
  return this->state & FASTLY_COMPUTE_AT_EDGE_CACHE_LOOKUP_STATE_STALE;
}

bool CacheState::must_insert_or_update() const {
  return this->state & FASTLY_COMPUTE_AT_EDGE_CACHE_LOOKUP_STATE_MUST_INSERT_OR_UPDATE;
}

Result<CacheHandle> CacheHandle::lookup(std::string_view key, const CacheLookupOptions &opts) {
  Result<CacheHandle> res;

  auto key_str = string_view_to_world_string(key);

  fastly_compute_at_edge_cache_lookup_options_t os;
  memset(&os, 0, sizeof(os));

  if (opts.request_headers.is_valid()) {
    os.request_headers.is_some = true;
    os.request_headers.val = opts.request_headers.handle;
  }

  fastly_compute_at_edge_types_error_t err;
  fastly_compute_at_edge_cache_handle_t handle;
  if (!fastly_compute_at_edge_cache_lookup(&key_str, &os, &handle, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(handle);
  }

  return res;
}

Result<CacheHandle> CacheHandle::transaction_lookup(std::string_view key,
                                                    const CacheLookupOptions &opts) {
  Result<CacheHandle> res;

  auto key_str = string_view_to_world_string(key);

  fastly_compute_at_edge_cache_lookup_options_t os;
  memset(&os, 0, sizeof(os));

  if (opts.request_headers.is_valid()) {
    os.request_headers.is_some = true;
    os.request_headers.val = opts.request_headers.handle;
  }

  fastly_compute_at_edge_types_error_t err;
  fastly_compute_at_edge_cache_handle_t handle;
  if (!fastly_compute_at_edge_cache_transaction_lookup(&key_str, &os, &handle, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(handle);
  }

  return res;
}

namespace {

void init_write_options(fastly_compute_at_edge_cache_write_options_t &options,
                        const CacheWriteOptions &opts) {
  memset(&options, 0, sizeof(options));

  options.max_age_ns = opts.max_age_ns;
  options.request_headers = opts.request_headers.handle;

  if (opts.vary_rule.empty()) {
    options.vary_rule.ptr = 0;
    options.vary_rule.len = 0;
  } else {
    options.vary_rule = string_view_to_world_string(opts.vary_rule);
  }

  options.initial_age_ns = opts.initial_age_ns;
  options.stale_while_revalidate_ns = opts.stale_while_revalidate_ns;

  if (opts.surrogate_keys.empty()) {
    options.surrogate_keys.ptr = 0;
    options.surrogate_keys.len = 0;
  } else {
    options.surrogate_keys = string_view_to_world_string(opts.surrogate_keys);
  }

  options.length = opts.length;

  if (!opts.metadata.len || !opts.metadata.ptr) {
    options.user_metadata.ptr = 0;
    options.user_metadata.len = 0;
  } else {
    options.user_metadata = span_to_list_u8(opts.metadata);
  }

  options.sensitive_data = opts.sensitive;
}

} // namespace

Result<HttpBody> CacheHandle::insert(std::string_view key, const CacheWriteOptions &opts) {
  Result<HttpBody> res;

  fastly_compute_at_edge_cache_write_options_t options;
  init_write_options(options, opts);

  fastly_compute_at_edge_types_error_t err;
  fastly_compute_at_edge_http_types_body_handle_t ret;
  auto host_key = string_view_to_world_string(key);
  if (!fastly_compute_at_edge_cache_insert(&host_key, &options, &ret, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(HttpBody{ret});
  }

  return res;
}

Result<HttpBody> CacheHandle::transaction_insert(const CacheWriteOptions &opts) {
  Result<HttpBody> res;

  fastly_compute_at_edge_cache_write_options_t options;
  init_write_options(options, opts);

  fastly_compute_at_edge_types_error_t err;
  fastly_compute_at_edge_http_types_body_handle_t ret;
  if (!fastly_compute_at_edge_cache_transaction_insert(this->handle, &options, &ret, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(HttpBody{ret});
  }

  return res;
}

Result<Void> CacheHandle::transaction_update(const CacheWriteOptions &opts) {
  Result<Void> res;

  fastly_compute_at_edge_cache_write_options_t options;
  init_write_options(options, opts);

  fastly_compute_at_edge_types_error_t err;
  if (!fastly_compute_at_edge_cache_transaction_update(this->handle, &options, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(Void{});
  }

  return res;
}

Result<std::tuple<HttpBody, CacheHandle>>
CacheHandle::transaction_insert_and_stream_back(const CacheWriteOptions &opts) {
  Result<std::tuple<HttpBody, CacheHandle>> res;

  fastly_compute_at_edge_cache_write_options_t options;
  init_write_options(options, opts);

  fastly_compute_at_edge_types_error_t err;
  fastly_world_tuple2_body_handle_handle_t ret;
  if (!fastly_compute_at_edge_cache_transaction_insert_and_stream_back(this->handle, &options, &ret,
                                                                       &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(HttpBody{ret.f0}, CacheHandle{ret.f1});
  }

  return res;
}

Result<Void> CacheHandle::transaction_cancel() {
  Result<Void> res;

  fastly_compute_at_edge_types_error_t err;
  if (!fastly_compute_at_edge_cache_transaction_cancel(this->handle, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace();
  }

  return res;
}

Result<HttpBody> CacheHandle::get_body(const CacheGetBodyOptions &opts) {
  Result<HttpBody> res;

  fastly_compute_at_edge_cache_get_body_options_t options{};
  uint32_t options_mask = 0;
  if (opts.start.has_value()) {
    options_mask |= FASTLY_COMPUTE_AT_EDGE_CACHE_GET_BODY_OPTIONS_MASK_START;
    options.start = opts.start.value();
  }
  if (opts.end.has_value()) {
    options_mask |= FASTLY_COMPUTE_AT_EDGE_CACHE_GET_BODY_OPTIONS_MASK_END;
    options.end = opts.end.value();
  }

  fastly_compute_at_edge_http_types_body_handle_t body;
  fastly_compute_at_edge_types_error_t err;
  if (!fastly_compute_at_edge_cache_get_body(this->handle, &options, options_mask, &body, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(HttpBody{body});
  }

  return res;
}

Result<CacheState> CacheHandle::get_state() {
  Result<CacheState> res;

  fastly_compute_at_edge_types_error_t err;
  alignas(4) fastly_compute_at_edge_cache_lookup_state_t state;
  if (!fastly_compute_at_edge_cache_get_state(this->handle, &state, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(CacheState{state});
  }

  return res;
}

Result<Void> CacheHandle::close() {

  fastly_compute_at_edge_types_error_t err;
  if (!fastly_compute_at_edge_cache_close(this->handle, &err)) {
    return Result<Void>::err(err);
  }

  return Result<Void>::ok();
  ;
}

Result<HostBytes> CacheHandle::get_user_metadata() {
  Result<HostBytes> res;

  fastly_world_list_u8_t ret;
  fastly_compute_at_edge_types_error_t err;
  if (!fastly_compute_at_edge_cache_get_user_metadata(this->handle, &ret, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(make_host_bytes(ret));
  }

  return res;
}

Result<uint64_t> CacheHandle::get_length() {
  Result<uint64_t> res;

  uint64_t ret;
  fastly_compute_at_edge_types_error_t err;
  if (!fastly_compute_at_edge_cache_get_length(this->handle, &ret, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(ret);
  }

  return res;
}

Result<uint64_t> CacheHandle::get_max_age_ns() {
  Result<uint64_t> res;

  uint64_t ret;
  fastly_compute_at_edge_types_error_t err;
  if (!fastly_compute_at_edge_cache_get_max_age_ns(this->handle, &ret, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(ret);
  }

  return res;
}

Result<uint64_t> CacheHandle::get_stale_while_revalidate_ns() {
  Result<uint64_t> res;

  uint64_t ret;
  fastly_compute_at_edge_types_error_t err;
  if (!fastly_compute_at_edge_cache_get_stale_while_revalidate_ns(this->handle, &ret, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(ret);
  }

  return res;
}

Result<uint64_t> CacheHandle::get_age_ns() {
  Result<uint64_t> res;

  uint64_t ret;
  fastly_compute_at_edge_types_error_t err;
  if (!fastly_compute_at_edge_cache_get_age_ns(this->handle, &ret, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(ret);
  }

  return res;
}

Result<uint64_t> CacheHandle::get_hits() {
  Result<uint64_t> res;

  uint64_t ret;
  fastly_compute_at_edge_types_error_t err;
  if (!fastly_compute_at_edge_cache_get_hits(this->handle, &ret, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(ret);
  }

  return res;
}

Result<std::optional<HostString>> Fastly::purge_surrogate_key(std::string_view key) {
  Result<std::optional<HostString>> res;

  auto host_key = string_view_to_world_string(key);
  fastly_world_option_string_t ret;
  fastly_compute_at_edge_types_error_t err;
  // TODO: we don't currently define any meaningful options in fastly.wit
  fastly_compute_at_edge_purge_options_mask_t purge_options = 0;
  if (!fastly_compute_at_edge_purge_surrogate_key(&host_key, purge_options, &ret, &err)) {
    res.emplace_err(err);
  } else if (ret.is_some) {
    res.emplace(make_host_string(ret.val));
  } else {
    res.emplace(std::nullopt);
  }

  return res;
}

const std::optional<std::string> FastlySendError::message() const {
  switch (this->tag) {
  /// The send-error-detail struct has not been populated.
  /// We will our generic error message in this situation.
  case uninitialized: {
    return "NetworkError when attempting to fetch resource.";
  }
  /// There was no send error.
  case ok: {
    return std::nullopt;
  }
  /// The system encountered a timeout when trying to find an IP address for the backend
  /// hostname.
  case dns_timeout: {
    return "DNS timeout";
  }
  /// The system encountered a DNS error when trying to find an IP address for the backend
  /// hostname. The fields dns_error_rcode and dns_error_info_code may be set in the
  /// send_error_detail.
  // TODO(GB): reenable DNS error codes
  case dns_error: {
    return "DNS error (rcode={}, info_code={})" /*, this->dns_error_rcode,
                        this->dns_error_info_code*/
        ;
  }
  /// The system cannot determine which backend to use, or the specified backend was invalid.
  case destination_not_found: {
    return "Destination not found";
  }
  /// The system considers the backend to be unavailable; e.g., recent attempts to communicate
  /// with it may have failed, or a health check may indicate that it is down.
  case destination_unavailable: {
    return "Destination unavailable";
  }
  /// The system cannot find a route to the next_hop IP address.
  case destination_ip_unroutable: {
    return "Destination IP unroutable";
  }
  /// The system's connection to the backend was refused.
  case connection_refused: {
    return "Connection refused";
  }
  /// The system's connection to the backend was closed before a complete response was
  /// received.
  case connection_terminated: {
    return "Connection terminated";
  }
  /// The system's attempt to open a connection to the backend timed out.
  case connection_timeout: {
    return "Connection timeout";
  }
  /// The system is configured to limit the number of connections it has to the backend, and
  /// that limit has been exceeded.
  case connection_limit_reached: {
    return "Connection limit reached";
  }
  /// The system encountered an error when verifying the certificate presented by the backend.
  case tls_certificate_error: {
    return "TLS certificate error";
  }
  /// The system encountered an error with the backend TLS configuration.
  case tls_configuration_error: {
    return "TLS configuration error";
  }
  /// The system received an incomplete response to the request from the backend.
  case http_incomplete_response: {
    return "Incomplete HTTP response";
  }
  /// The system received a response to the request whose header section was considered too
  /// large.
  case http_response_header_section_too_large: {
    return "HTTP response header section too large";
  }
  /// The system received a response to the request whose body was considered too large.
  case http_response_body_too_large: {
    return "HTTP response body too large";
  }
  /// The system reached a configured time limit waiting for the complete response.
  case http_response_timeout: {
    return "HTTP response timeout";
  }
  /// The system received a response to the request whose status code or reason phrase was
  /// invalid.
  case http_response_status_invalid: {
    return "HTTP response status invalid";
  }
  /// The process of negotiating an upgrade of the HTTP version between the system and the
  /// backend failed.
  case http_upgrade_failed: {
    return "HTTP upgrade failed";
  }
  /// The system encountered an HTTP protocol error when communicating with the backend. This
  /// error will only be used when a more specific one is not defined.
  case http_protocol_error: {
    return "HTTP protocol error";
  }
  /// An invalid cache key was provided for the request.
  case http_request_cache_key_invalid: {
    return "HTTP request cache key invalid";
  }
  /// An invalid URI was provided for the request.
  case http_request_uri_invalid: {
    return "HTTP request URI invalid";
  }
  /// The system encountered an unexpected internal error.
  case internal_error: {
    return "Internal error";
  }
  /// The system received a TLS alert from the backend. The field tls_alert_id may be set in
  /// the send_error_detail.
  case tls_alert_received: {
    return /*fmt::format(*/ "TLS alert received (alert_id={})" /*, this->tls_alert_id)*/;
  }
  /// The system encountered a TLS error when communicating with the backend, either during
  /// the handshake or afterwards.
  case tls_protocol_error: {
    return "TLS protocol error";
  }
  }
  return "NetworkError when attempting to fetch resource.";
}

bool BackendHealth::is_unknown() const {
  return this->state & FASTLY_COMPUTE_AT_EDGE_BACKEND_BACKEND_HEALTH_UNKNOWN;
}

bool BackendHealth::is_healthy() const {
  return this->state & FASTLY_COMPUTE_AT_EDGE_BACKEND_BACKEND_HEALTH_HEALTHY;
}

bool BackendHealth::is_unhealthy() const {
  return this->state & FASTLY_COMPUTE_AT_EDGE_BACKEND_BACKEND_HEALTH_UNHEALTHY;
}

Result<bool> Backend::exists(std::string_view name) {
  Result<bool> res;

  auto name_str = string_view_to_world_string(name);
  bool ret;
  fastly_compute_at_edge_types_error_t err;
  if (!fastly_compute_at_edge_backend_exists(&name_str, &ret, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(ret);
  }

  return res;
}

Result<BackendHealth> Backend::health(std::string_view name) {
  Result<BackendHealth> res;

  auto name_str = string_view_to_world_string(name);
  fastly_compute_at_edge_backend_backend_health_t ret;
  fastly_compute_at_edge_types_error_t err;
  if (!fastly_compute_at_edge_backend_is_healthy(&name_str, &ret, &err)) {
    res.emplace_err(err);
  } else {
    switch (ret) {
    case FASTLY_COMPUTE_AT_EDGE_BACKEND_BACKEND_HEALTH_UNKNOWN:
    case FASTLY_COMPUTE_AT_EDGE_BACKEND_BACKEND_HEALTH_HEALTHY:
    case FASTLY_COMPUTE_AT_EDGE_BACKEND_BACKEND_HEALTH_UNHEALTHY: {
      res.emplace(BackendHealth(ret));
      break;
    }
    default: {
      MOZ_ASSERT_UNREACHABLE("Making a BackendHealth from an invalid value");
    }
    }
  }

  return res;
}

Result<Void> RateCounter::increment(std::string_view name, std::string_view entry, uint32_t delta) {
  auto name_str = string_view_to_world_string(name);
  auto entry_str = string_view_to_world_string(entry);
  fastly_compute_at_edge_types_error_t err;
  if (!fastly_compute_at_edge_edge_rate_limiter_ratecounter_increment(&name_str, &entry_str, delta,
                                                                      &err)) {
    return Result<Void>::err(err);
  }

  return Result<Void>::ok();
}

Result<uint32_t> RateCounter::lookup_rate(std::string_view name, std::string_view entry,
                                          uint32_t window) {
  Result<uint32_t> res;

  auto name_str = string_view_to_world_string(name);
  auto entry_str = string_view_to_world_string(entry);
  uint32_t ret;
  fastly_compute_at_edge_types_error_t err;
  if (!fastly_compute_at_edge_edge_rate_limiter_ratecounter_lookup_rate(&name_str, &entry_str,
                                                                        window, &ret, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(ret);
  }

  return res;
}

Result<uint32_t> RateCounter::lookup_count(std::string_view name, std::string_view entry,
                                           uint32_t duration) {
  Result<uint32_t> res;

  auto name_str = string_view_to_world_string(name);
  auto entry_str = string_view_to_world_string(entry);
  uint32_t ret;
  fastly_compute_at_edge_types_error_t err;
  if (!fastly_compute_at_edge_edge_rate_limiter_ratecounter_lookup_count(&name_str, &entry_str,
                                                                         duration, &ret, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(ret);
  }

  return res;
}

Result<Void> PenaltyBox::add(std::string_view name, std::string_view entry, uint32_t timeToLive) {
  auto name_str = string_view_to_world_string(name);
  auto entry_str = string_view_to_world_string(entry);
  fastly_compute_at_edge_types_error_t err;
  if (!fastly_compute_at_edge_edge_rate_limiter_penaltybox_add(&name_str, &entry_str, timeToLive,
                                                               &err)) {
    return Result<Void>::err(err);
  }

  return Result<Void>::ok();
}

Result<bool> PenaltyBox::has(std::string_view name, std::string_view entry) {
  Result<bool> res;

  auto name_str = string_view_to_world_string(name);
  auto entry_str = string_view_to_world_string(entry);
  alignas(4) bool ret;
  fastly_compute_at_edge_types_error_t err;
  if (!fastly_compute_at_edge_edge_rate_limiter_penaltybox_has(&name_str, &entry_str, &ret, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(ret);
  }
  return res;
}

Result<bool> EdgeRateLimiter::check_rate(std::string_view rate_counter_name, std::string_view entry,
                                         uint32_t delta, uint32_t window, uint32_t limit,
                                         std::string_view penalty_box_name, uint32_t time_to_live) {
  Result<bool> res;

  auto rate_counter_name_str = string_view_to_world_string(rate_counter_name);
  auto entry_str = string_view_to_world_string(entry);
  auto penalty_box_name_str = string_view_to_world_string(penalty_box_name);
  alignas(4) bool ret;
  fastly_compute_at_edge_types_error_t err;
  if (!fastly_compute_at_edge_edge_rate_limiter_check_rate(
          &rate_counter_name_str, &entry_str, delta, window, limit, &penalty_box_name_str,
          time_to_live, &ret, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(ret);
  }
  return res;
}

Result<HostString> DeviceDetection::lookup(std::string_view user_agent) {
  Result<HostString> res;

  auto user_agent_str = string_view_to_world_string(user_agent);
  fastly_compute_at_edge_types_error_t err;
  fastly_world_string_t ret;
  if (!fastly_compute_at_edge_device_detection_lookup(&user_agent_str, &ret, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(JS::UniqueChars(reinterpret_cast<char *>(ret.ptr)), ret.len);
  }
  return res;
}

} // namespace host_api
