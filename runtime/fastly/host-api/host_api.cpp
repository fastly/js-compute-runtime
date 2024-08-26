#include <algorithm>
#include <type_traits>

#include "../../StarlingMonkey/runtime/allocator.h"
#include "../../StarlingMonkey/runtime/encode.h"
#include "./fastly.h"
#include "./host_api_fastly.h"

#include <algorithm>
#include <arpa/inet.h>

#include <time.h>

using api::FastlyResult;
using fastly::FastlyAPIError;

#define NEVER_HANDLE 0xFFFFFFFE

#define MILLISECS_IN_NANOSECS 1000000
#define SECS_IN_NANOSECS 1000000000

static bool convert_result(int res, fastly::fastly_host_error *err) {
  if (res == 0)
    return true;
  switch (res) {
  case 1:
    *err = FASTLY_HOST_ERROR_GENERIC_ERROR;
    break;
  case 2:
    *err = FASTLY_HOST_ERROR_INVALID_ARGUMENT;
    break;
  case 3:
    *err = FASTLY_HOST_ERROR_BAD_HANDLE;
    break;
  case 4:
    *err = FASTLY_HOST_ERROR_BUFFER_LEN;
    break;
  case 5:
    *err = FASTLY_HOST_ERROR_UNSUPPORTED;
    break;
  case 6:
    *err = FASTLY_HOST_ERROR_BAD_ALIGN;
    break;
  case 7:
    *err = FASTLY_HOST_ERROR_HTTP_INVALID;
    break;
  case 8:
    *err = FASTLY_HOST_ERROR_HTTP_USER;
    break;
  case 9:
    *err = FASTLY_HOST_ERROR_HTTP_INCOMPLETE;
    break;
  case 10:
    *err = FASTLY_HOST_ERROR_OPTIONAL_NONE;
    break;
  case 11:
    *err = FASTLY_HOST_ERROR_HTTP_HEAD_TOO_LARGE;
    break;
  case 12:
    *err = FASTLY_HOST_ERROR_HTTP_INVALID_STATUS;
    break;
  case 13:
    *err = FASTLY_HOST_ERROR_LIMIT_EXCEEDED;
    break;
  case 100:
    *err = FASTLY_HOST_ERROR_UNKNOWN_ERROR;
    break;
  default:
    *err = FASTLY_HOST_ERROR_UNKNOWN_ERROR;
  }
  return false;
}

void sleep_until(uint64_t time_ns, uint64_t now) {
  while (time_ns > now) {
    uint64_t duration = time_ns - now;
    timespec req{.tv_sec = static_cast<time_t>(duration / SECS_IN_NANOSECS),
                 .tv_nsec = static_cast<long>(duration % SECS_IN_NANOSECS)};
    timespec rem;
    nanosleep(&req, &rem);
    now = host_api::MonotonicClock::now();
  }
}

size_t api::AsyncTask::select(std::vector<api::AsyncTask *> &tasks) {
  size_t tasks_len = tasks.size();
  std::vector<api::FastlyAsyncTask::Handle> handles;
  handles.reserve(tasks_len);
  uint64_t now = 0;
  uint64_t soonest_deadline = 0;
  size_t soonest_deadline_idx = -1;
  for (size_t idx = 0; idx < tasks_len; ++idx) {
    auto *task = tasks.at(idx);
    uint64_t deadline = task->deadline();
    // Select for completed task deadlines before performing the task select host call.
    if (deadline > 0) {
      MOZ_ASSERT(task->id() == NEVER_HANDLE);
      if (now == 0) {
        now = host_api::MonotonicClock::now();
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
    MOZ_ASSERT(soonest_deadline > now);
    sleep_until(soonest_deadline, now);
    return soonest_deadline_idx;
  }

  uint32_t ret = UINT32_MAX;
  fastly::fastly_host_error err = 0;

  while (true) {
    MOZ_ASSERT(soonest_deadline == 0 || soonest_deadline > now);
    // timeout value of 0 means no timeout for async_select
    uint32_t timeout = soonest_deadline > 0 ? (soonest_deadline - now) / MILLISECS_IN_NANOSECS : 0;
    if (!convert_result(fastly::async_select(handles.data(), handles.size(), timeout, &ret),
                        &err)) {
      if (host_api::error_is_bad_handle(err)) {
        fprintf(stderr, "Critical Error: An invalid handle was provided to async_select.\n");
      } else {
        fprintf(stderr, "Critical Error: An unknown error occurred in async_select.\n");
      }
      abort();
    }

    // The result is only valid if the timeout didn't expire.
    if (ret != UINT32_MAX) {
      // The host index will be the index in the list of tasks with the timer tasks filtered out.
      // We thus need to offset the host index by any timer tasks appearing before the nth
      // non-timer task.
      size_t task_idx = 0;
      for (size_t idx = 0; idx < tasks_len; ++idx) {
        if (tasks.at(idx)->id() != NEVER_HANDLE) {
          if (ret == task_idx) {
            return idx;
          }
          task_idx++;
        }
      }
      abort();
    } else if (soonest_deadline > 0) {
      MOZ_ASSERT(soonest_deadline > now);
      MOZ_ASSERT(soonest_deadline_idx != -1);
      // Verify that the task definitely is ready from a time perspective, and if not loop the host
      // call again.
      now = host_api::MonotonicClock::now();
      if (soonest_deadline > now) {
        err = 0;
        continue;
      }
      return soonest_deadline_idx;
    } else {
      abort();
    }
  }
}

namespace host_api {

namespace {

fastly::fastly_world_list_u8 span_to_list_u8(std::span<uint8_t> span) {
  return {
      .ptr = const_cast<uint8_t *>(span.data()),
      .len = span.size(),
  };
}

fastly::fastly_world_string string_view_to_world_string(std::string_view str) {
  return {
      .ptr = const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(str.data())),
      .len = str.size(),
  };
}

HostString make_host_string(fastly::fastly_world_string str) {
  return HostString{JS::UniqueChars{reinterpret_cast<char *>(str.ptr)}, str.len};
}

HostString make_host_string(fastly::fastly_world_list_u8 str) {
  return HostString{JS::UniqueChars{reinterpret_cast<char *>(str.ptr)}, str.len};
}

HostBytes make_host_bytes(fastly::fastly_world_list_u8 str) {
  return HostBytes{std::unique_ptr<uint8_t[]>{str.ptr}, str.len};
}

Response make_response(fastly::fastly_host_http_response &resp) {
  return Response{HttpResp{resp.f0}, HttpBody{resp.f1}};
}

template <auto header_names_get>
Result<std::vector<HostString>> generic_get_header_names(auto handle) {
  Result<std::vector<HostString>> res;

  fastly::fastly_world_list_string ret;
  fastly::fastly_host_error err;
  fastly::fastly_world_string *strs = static_cast<fastly::fastly_world_string *>(
      cabi_malloc(LIST_ALLOC_SIZE * sizeof(fastly::fastly_world_string), 1));
  size_t str_max = LIST_ALLOC_SIZE;
  size_t str_cnt = 0;
  size_t nwritten;
  uint8_t *buf = static_cast<uint8_t *>(cabi_malloc(HOSTCALL_BUFFER_LEN, 1));
  uint32_t cursor = 0;
  int64_t next_cursor = 0;
  while (true) {
    if (!convert_result(
            header_names_get(handle, buf, HEADER_MAX_LEN, cursor, &next_cursor, &nwritten), &err)) {
      cabi_free(buf);
      res.emplace_err(err);
      return res;
    }
    if (nwritten == 0) {
      break;
    }
    uint32_t offset = 0;
    for (size_t i = 0; i < nwritten; i++) {
      if (buf[i] != '\0')
        continue;
      if (str_cnt == str_max) {
        strs = static_cast<fastly::fastly_world_string *>(
            cabi_realloc(strs, str_max * sizeof(fastly::fastly_world_string), 1,
                         (str_max + LIST_ALLOC_SIZE) * sizeof(fastly::fastly_world_string)));
        str_max += LIST_ALLOC_SIZE;
      }
      size_t len = i - offset;
      strs[str_cnt].ptr = static_cast<uint8_t *>(cabi_malloc(len, 1));
      strs[str_cnt].len = len;
      memcpy(strs[str_cnt].ptr, buf + offset, len);
      offset = i + 1;
      str_cnt++;
    }
    if (next_cursor < 0)
      break;
    cursor = (uint32_t)next_cursor;
  }
  cabi_free(buf);
  if (str_cnt != 0) {
    strs = static_cast<fastly::fastly_world_string *>(
        cabi_realloc(strs, str_max * sizeof(fastly::fastly_world_string), 1,
                     str_cnt * sizeof(fastly::fastly_world_string)));
  }
  ret.ptr = strs;
  ret.len = str_cnt;
  std::vector<HostString> names;

  for (int i = 0; i < ret.len; i++) {
    names.emplace_back(make_host_string(ret.ptr[i]));
  }

  // Free the vector of string pointers, but leave the individual strings alone.
  cabi_free(ret.ptr);

  res.emplace(std::move(names));

  return res;
}

struct Chunk {
  JS::UniqueChars buffer;
  size_t length;

  static Chunk make(std::string_view data) {
    Chunk res{JS::UniqueChars{static_cast<char *>(cabi_malloc(data.size(), 1))}, data.size()};
    std::copy(data.begin(), data.end(), res.buffer.get());
    return res;
  }
};

template <auto header_values_get>
Result<std::optional<std::vector<HostString>>> generic_get_header_values(auto handle,
                                                                         std::string_view name) {
  Result<std::optional<std::vector<HostString>>> res;

  fastly::fastly_world_string hdr = string_view_to_world_string(name);
  fastly::fastly_world_option_list_list_u8 ret;
  fastly::fastly_host_error err;
  std::vector<Chunk> header_values;
  JS::UniqueLatin1Chars buffer(static_cast<unsigned char *>(cabi_malloc(HEADER_MAX_LEN, 1)));
  uint32_t cursor = 0;
  while (true) {
    int64_t ending_cursor = 0;
    size_t length = 0;
    if (!convert_result(header_values_get(handle, reinterpret_cast<char *>(hdr.ptr), hdr.len,
                                          buffer.get(), HEADER_MAX_LEN, cursor, &ending_cursor,
                                          &length),
                        &err)) {
      res.emplace_err(err);
      return res;
    }

    if (length == 0) {
      break;
    }

    std::string_view result{reinterpret_cast<char *>(buffer.get()), length};
    while (!result.empty()) {
      auto end = result.find('\0');
      header_values.emplace_back(Chunk::make(result.substr(0, end)));
      if (end == result.npos) {
        break;
      }

      result = result.substr(end + 1);
    }

    if (ending_cursor < 0) {
      break;
    }
  }

  if (header_values.empty()) {
    ret.is_some = false;
  } else {
    ret.is_some = true;
    ret.val.len = header_values.size();
    ret.val.ptr = static_cast<fastly::fastly_world_list_u8 *>(
        cabi_malloc(header_values.size() * sizeof(fastly::fastly_world_list_u8),
                    alignof(fastly::fastly_world_list_u8)));
    auto *next = ret.val.ptr;
    for (auto &chunk : header_values) {
      next->len = chunk.length;
      next->ptr = reinterpret_cast<uint8_t *>(chunk.buffer.release());
      ++next;
    }
  }

  if (ret.is_some) {
    std::vector<HostString> names;

    for (int i = 0; i < ret.val.len; i++) {
      names.emplace_back(make_host_string(ret.val.ptr[i]));
    }

    // Free the vector of string pointers, but leave the individual strings alone.
    cabi_free(ret.val.ptr);

    res.emplace(std::move(names));
  } else {
    res.emplace(std::nullopt);
  }

  return res;
}

template <auto header_op>
Result<Void> generic_header_op(auto handle, std::string_view name, std::span<uint8_t> value) {
  Result<Void> res;

  fastly::fastly_world_string hdr = string_view_to_world_string(name);
  fastly::fastly_world_list_u8 val = span_to_list_u8(value);
  fastly::fastly_host_error err;
  if (!convert_result(
          header_op(handle, reinterpret_cast<char *>(hdr.ptr), hdr.len, val.ptr, val.len), &err)) {
    res.emplace_err(err);
  }

  return res;
}

template <auto remove_header>
Result<Void> generic_header_remove(auto handle, std::string_view name) {
  Result<Void> res;

  fastly::fastly_world_string hdr = string_view_to_world_string(name);
  fastly::fastly_host_error err;
  if (!convert_result(remove_header(handle, reinterpret_cast<char *>(hdr.ptr), hdr.len), &err)) {
    if (host_api::error_is_invalid_argument(err)) {
      return Result<Void>::ok();
    }
    res.emplace_err(err);
  }

  return res;
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

// HttpHeaders and HttpHeadersReadOnly extend Resource.
// Resource provdes handle_state_ which is a HandleState
// which gets to be fully host-defined.
Resource::~Resource() {
  if (handle_state_ != nullptr) {
    handle_state_ = nullptr;
  }
};

// Fastly handle state is currently just a wrapper around
// an arbitrary fastly handle, along with a bit indicating
// if it is a request or response.
class HandleState {
protected:
  api::FastlyAsyncTask::Handle handle_;
  bool is_req_;

public:
  explicit HandleState(api::FastlyAsyncTask::Handle handle, bool is_request) {
    handle_ = handle;
    is_req_ = is_request;
  }
  api::FastlyAsyncTask::Handle handle() { return handle_; }
  bool is_req() { return is_req_; }
  bool valid() const { return true; }
};

HttpHeaders *HttpHeadersReadOnly::clone() { return new HttpHeaders(*this); }

const std::vector<const char *> forbidden_request_headers = {};
const std::vector<const char *> forbidden_response_headers = {};

const std::vector<const char *> &HttpHeaders::get_forbidden_request_headers() {
  return forbidden_request_headers;
}
const std::vector<const char *> &HttpHeaders::get_forbidden_response_headers() {
  return forbidden_response_headers;
}

Result<vector<tuple<HostString, HostString>>> HttpHeadersReadOnly::entries() const {
  Result<vector<tuple<HostString, HostString>>> res;

  Result<std::vector<HostString>> names_res;
  if (this->handle_state_.get()->is_req()) {
    names_res =
        generic_get_header_names<fastly::req_header_names_get>(this->handle_state_.get()->handle());
  } else {
    names_res = generic_get_header_names<fastly::resp_header_names_get>(
        this->handle_state_.get()->handle());
  }
  if (const auto err = names_res.to_err()) {
    return Result<vector<tuple<HostString, HostString>>>::err(*err);
  }

  vector<tuple<HostString, HostString>> entries_vec;
  for (auto &name : names_res.unwrap()) {
    auto values_res = HttpHeadersReadOnly::get(name);
    if (auto err = values_res.to_err()) {
      return Result<vector<tuple<HostString, HostString>>>::err(*err);
    }
    auto &values = values_res.unwrap();
    if (!values.has_value()) {
      // original js-compute-runtime also skipped here, but should this be an error or empty entry?
      continue;
    }
    auto last_val = &(*values.value().end());
    for (auto &value : values.value()) {
      if (&value == last_val) {
        entries_vec.emplace_back(std::move(name), std::move(value));
      } else {
        std::string_view host_name_view(name);
        entries_vec.emplace_back(host_api::HostString(host_name_view), std::move(value));
      }
    }
  }

  res.emplace(std::move(entries_vec));
  return res;
}

Result<optional<vector<HostString>>> HttpHeadersReadOnly::get(string_view name) const {
  Result<optional<vector<HostString>>> res;
  if (this->handle_state_.get()->is_req()) {
    return generic_get_header_values<fastly::req_header_values_get>(
        this->handle_state_.get()->handle(), name);
  } else {
    return generic_get_header_values<fastly::resp_header_values_get>(
        this->handle_state_.get()->handle(), name);
  }
}

Result<bool> HttpHeadersReadOnly::has(string_view name) const {
  auto get_res = get(name);
  if (const auto err = get_res.to_err()) {
    return Result<bool>::err(*err);
  }
  return Result<bool>::ok(get_res.unwrap().has_value());
}

HttpHeaders::HttpHeaders(std::unique_ptr<HandleState> state)
    : HttpHeadersReadOnly(std::move(state)) {}

HttpHeaders::HttpHeaders() { handle_state_ = nullptr; }

// TODO(guybedford): ensure this actually clones, or ban it entirely?
HttpHeaders::HttpHeaders(const HttpHeadersReadOnly &headers) : HttpHeadersReadOnly(nullptr) {
  auto handle_state =
      new HandleState(headers.handle_state_.get()->handle(), headers.handle_state_.get()->is_req());
  this->handle_state_ = std::unique_ptr<HandleState>(handle_state);
}

// This is only used by the HttpHeaders subclass, and immediately assigns the state after.
HttpHeadersReadOnly::HttpHeadersReadOnly() { handle_state_ = nullptr; }
HttpHeadersReadOnly::HttpHeadersReadOnly(std::unique_ptr<HandleState> state) {
  handle_state_ = std::move(state);
}

// This call corresponds to a state transition in switch_mode from Mode::ContentOnly to
// Mode::CachedInContent in StarlingMonkey, which occurs when cloning a headers object, which we do
// not call.
//
// That is, we have:
// - Desynchronize from host: Mode::CachedInContent -> Mode::ContentOnly (create local mutations)
// - Resynchronize to host  : Mode::ContentOnly -> Mode::CachedInContent (commit local mutations)
//
// With these state transitions permitted arbitrarily.
//
// Fastly's headers implementation ties headers to request and response handles, as opposed to
// being able to exist as free handles for headers. We therefore avoid the headers cloning operation
// Mode::ContentOnly -> Mode::CachedInContent transition and as a result this FromEntries function
// is never called.
//
// Instead we use a separate RequestOrResponse::commit_headers() implementation for fetch requests
// and fetch-event responses, leaving the former Mode::ContentOnly as the cached value to achieve
// the exact same result.
Result<HttpHeaders *> HttpHeaders::FromEntries(vector<tuple<HostString, HostString>> &entries) {
  MOZ_RELEASE_ASSERT(false);
}

// Instead, we use write_headers to write into an existing headers object.
Result<Void>
write_headers(HttpHeaders *headers,
              std::vector<std::tuple<host_api::HostString, host_api::HostString>> &list) {
  for (const auto &[name, value] : list) {
    auto res = headers->append(name, value);
    if (res.is_err()) {
      return res;
    }
  }
  return Result<Void>::ok();
}

Result<Void> HttpHeaders::remove(string_view name) {
  if (this->handle_state_.get()->is_req()) {
    return generic_header_remove<fastly::req_header_remove>(this->handle_state_.get()->handle(),
                                                            name);
  } else {
    return generic_header_remove<fastly::resp_header_remove>(this->handle_state_.get()->handle(),
                                                             name);
  }
}

Result<Void> HttpHeaders::set(string_view name, string_view value) {
  std::span<uint8_t> value_span = {reinterpret_cast<uint8_t *>(const_cast<char *>(value.data())),
                                   value.size()};
  if (this->handle_state_.get()->is_req()) {
    return generic_header_op<fastly::req_header_insert>(this->handle_state_.get()->handle(), name,
                                                        value_span);
  } else {
    return generic_header_op<fastly::resp_header_insert>(this->handle_state_.get()->handle(), name,
                                                         value_span);
  }
}
Result<Void> HttpHeaders::append(string_view name, string_view value) {
  std::span<uint8_t> value_span = {reinterpret_cast<uint8_t *>(const_cast<char *>(value.data())),
                                   value.size()};
  if (this->handle_state_.get()->is_req()) {
    return generic_header_op<fastly::req_header_append>(this->handle_state_.get()->handle(), name,
                                                        value_span);
  } else {
    return generic_header_op<fastly::resp_header_append>(this->handle_state_.get()->handle(), name,
                                                         value_span);
  }
}

// --- </StarlingMonkey Host API> ---

// The host interface makes the assumption regularly that uint32_t is sufficient space to store a
// pointer.
static_assert(sizeof(uint32_t) == sizeof(void *));

namespace {

FastlySendError
make_fastly_send_error(fastly::fastly_host_http_send_error_detail &send_error_detail) {
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

  auto res = GeoIp::lookup(std::span<uint8_t>{octets, octets_len});
  if (auto *err = res.to_err()) {
    HANDLE_ERROR(cx, *err);
    return nullptr;
  }

  auto ret = std::move(res.unwrap());

  return JS_NewStringCopyUTF8N(cx, JS::UTF8Chars(ret.ptr.release(), ret.len));
}

Result<HttpBody> HttpBody::make() {
  Result<HttpBody> res;

  HttpBody::Handle handle;
  fastly::fastly_host_error err;
  if (!convert_result(fastly::body_new(&handle), &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(handle);
  }

  return res;
}

Result<HostString> HttpBody::read(uint32_t chunk_size) const {
  Result<HostString> res;

  fastly::fastly_world_list_u8 ret;
  ret.ptr = static_cast<uint8_t *>(cabi_malloc(chunk_size, 1));
  fastly::fastly_host_error err;
  if (!convert_result(
          fastly::body_read(this->handle, ret.ptr, static_cast<size_t>(chunk_size), &ret.len),
          &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(JS::UniqueChars(reinterpret_cast<char *>(ret.ptr)), ret.len);
  }

  return res;
}

Result<uint32_t> HttpBody::write_front(const uint8_t *ptr, size_t len) const {
  Result<uint32_t> res;

  // The write call doesn't mutate the buffer; the cast is just for the generated fastly api.
  fastly::fastly_world_list_u8 chunk{const_cast<uint8_t *>(ptr), len};

  fastly::fastly_host_error err;
  uint32_t written;

  if (!convert_result(fastly::body_write(this->handle, chunk.ptr, chunk.len,
                                         fastly::BodyWriteEnd::BodyWriteEndFront,
                                         reinterpret_cast<size_t *>(&written)),
                      &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(written);
  }

  return res;
}

Result<uint32_t> HttpBody::write_back(const uint8_t *ptr, size_t len) const {
  Result<uint32_t> res;

  // The write call doesn't mutate the buffer; the cast is just for the generated fastly api.
  fastly::fastly_world_list_u8 chunk{const_cast<uint8_t *>(ptr), len};

  fastly::fastly_host_error err;
  uint32_t written;
  if (!convert_result(fastly::body_write(this->handle, chunk.ptr, chunk.len,
                                         fastly::BodyWriteEnd::BodyWriteEndBack,
                                         reinterpret_cast<size_t *>(&written)),
                      &err)) {
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

  fastly::fastly_host_error err;
  if (!convert_result(fastly::body_append(this->handle, other.handle), &err)) {
    res.emplace_err(err);
  } else {
    res.emplace();
  }

  return res;
}

Result<Void> HttpBody::close() {
  Result<Void> res;

  fastly::fastly_host_error err;
  if (!convert_result(fastly::body_close(this->handle), &err)) {
    res.emplace_err(err);
  } else {
    res.emplace();
  }

  return res;
}

FastlyAsyncTask::Handle HttpBody::async_handle() const {
  return FastlyAsyncTask::Handle{this->handle};
}

FastlyResult<Response, FastlySendError> HttpPendingReq::wait() {
  FastlyResult<Response, FastlySendError> res;

  fastly::fastly_host_http_send_error_detail s;
  std::memset(&s, 0, sizeof(s));
  s.mask |= FASTLY_HOST_HTTP_SEND_ERROR_DETAIL_MASK_DNS_ERROR_RCODE;
  s.mask |= FASTLY_HOST_HTTP_SEND_ERROR_DETAIL_MASK_DNS_ERROR_INFO_CODE;
  s.mask |= FASTLY_HOST_HTTP_SEND_ERROR_DETAIL_MASK_TLS_ALERT_ID;

  fastly::fastly_host_error err;
  fastly::fastly_host_http_response ret;

  if (!convert_result(fastly::req_pending_req_wait_v2(this->handle, &s, &ret.f0, &ret.f1), &err)) {
    res.emplace_err(make_fastly_send_error(s));
  } else {
    res.emplace(make_response(ret));
  }

  return res;
}

FastlyAsyncTask::Handle HttpPendingReq::async_handle() const {
  return FastlyAsyncTask::Handle{this->handle};
}

void CacheOverrideTag::set_pass() { this->value |= CACHE_OVERRIDE_PASS; }

void CacheOverrideTag::set_ttl() { this->value |= CACHE_OVERRIDE_TTL; }

void CacheOverrideTag::set_stale_while_revalidate() {
  this->value |= CACHE_OVERRIDE_STALE_WHILE_REVALIDATE;
}

void CacheOverrideTag::set_pci() { this->value |= CACHE_OVERRIDE_PCI; }

TlsVersion::TlsVersion(uint8_t raw) : value{raw} {
  switch (raw) {
  case fastly::TLS::VERSION_1:
  case fastly::TLS::VERSION_1_1:
  case fastly::TLS::VERSION_1_2:
  case fastly::TLS::VERSION_1_3:
    break;

  default:
    MOZ_ASSERT(false, "Making a TlsValue from an invalid raw value");
  }
}

uint8_t TlsVersion::get_version() const { return this->value; }

TlsVersion TlsVersion::version_1() { return TlsVersion{fastly::TLS::VERSION_1}; }

TlsVersion TlsVersion::version_1_1() { return TlsVersion{fastly::TLS::VERSION_1_1}; }

TlsVersion TlsVersion::version_1_2() { return TlsVersion{fastly::TLS::VERSION_1_2}; }

TlsVersion TlsVersion::version_1_3() { return TlsVersion{fastly::TLS::VERSION_1_3}; }

Result<HttpReq> HttpReq::make() {
  Result<HttpReq> res;

  HttpReq::Handle handle;
  fastly::fastly_host_error err;
  if (!convert_result(fastly::req_new(&handle), &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(handle);
  }

  return res;
}

Result<Void> HttpReq::redirect_to_grip_proxy(std::string_view backend) {
  Result<Void> res;

  fastly::fastly_host_error err;
  fastly::fastly_world_string backend_str = string_view_to_world_string(backend);
  if (!convert_result(fastly::req_redirect_to_grip_proxy(reinterpret_cast<char *>(backend_str.ptr),
                                                         backend_str.len),
                      &err)) {
    res.emplace_err(err);
  } else {
    res.emplace();
  }

  return res;
}

Result<Void> HttpReq::auto_decompress_gzip() {
  Result<Void> res;

  fastly::fastly_host_error err;
  int encodings_to_decompress = 0 | FASTLY_HOST_CONTENT_ENCODINGS_GZIP;
  if (!convert_result(
          fastly::req_auto_decompress_response_set(this->handle, encodings_to_decompress), &err)) {
    res.emplace_err(err);
  } else {
    res.emplace();
  }

  return res;
}

Result<Void> HttpReq::register_dynamic_backend(std::string_view name, std::string_view target,
                                               const BackendConfig &config) {
  Result<Void> res;

  fastly::DynamicBackendConfig backend_configuration;
  memset(&backend_configuration, 0, sizeof(backend_configuration));
  uint32_t backend_config_mask = 0;

  if (auto &val = config.host_override) {
    backend_config_mask |= BACKEND_CONFIG_HOST_OVERRIDE;
    auto host_override = string_view_to_world_string(*val);
    backend_configuration.host_override = reinterpret_cast<char *>(host_override.ptr);
    backend_configuration.host_override_len = host_override.len;
  }

  if (config.connect_timeout.has_value()) {
    backend_config_mask |= BACKEND_CONFIG_CONNECT_TIMEOUT;
    backend_configuration.connect_timeout_ms = *config.connect_timeout;
  }

  if (config.first_byte_timeout.has_value()) {
    backend_config_mask |= BACKEND_CONFIG_FIRST_BYTE_TIMEOUT;
    backend_configuration.first_byte_timeout_ms = *config.first_byte_timeout;
  }

  if (config.between_bytes_timeout.has_value()) {
    backend_config_mask |= BACKEND_CONFIG_BETWEEN_BYTES_TIMEOUT;
    backend_configuration.between_bytes_timeout_ms = *config.between_bytes_timeout;
  }

  if (config.use_ssl.value_or(false)) {
    backend_config_mask |= BACKEND_CONFIG_USE_SSL;
  }

  if (config.dont_pool.value_or(false)) {
    backend_config_mask |= BACKEND_CONFIG_DONT_POOL;
  }

  if (config.ssl_min_version.has_value()) {
    backend_config_mask |= BACKEND_CONFIG_SSL_MIN_VERSION;
    backend_configuration.ssl_min_version = config.ssl_min_version->get_version();
  }

  if (config.ssl_max_version.has_value()) {
    backend_config_mask |= BACKEND_CONFIG_SSL_MAX_VERSION;
    backend_configuration.ssl_max_version = config.ssl_max_version->get_version();
  }

  if (auto &val = config.cert_hostname) {
    backend_config_mask |= BACKEND_CONFIG_CERT_HOSTNAME;
    auto cert_hostname = string_view_to_world_string(*val);
    backend_configuration.cert_hostname = reinterpret_cast<char *>(cert_hostname.ptr);
    backend_configuration.cert_hostname_len = cert_hostname.len;
  }

  if (auto &val = config.ca_cert) {
    backend_config_mask |= BACKEND_CONFIG_CA_CERT;
    auto ca_cert = string_view_to_world_string(*val);
    backend_configuration.ca_cert = reinterpret_cast<char *>(ca_cert.ptr);
    backend_configuration.ca_cert_len = ca_cert.len;
  }

  if (auto &val = config.ciphers) {
    backend_config_mask |= BACKEND_CONFIG_CIPHERS;
    auto ciphers = string_view_to_world_string(*val);
    backend_configuration.ciphers = reinterpret_cast<char *>(ciphers.ptr);
    backend_configuration.ciphers_len = ciphers.len;
  }

  if (auto &val = config.sni_hostname) {
    backend_config_mask |= BACKEND_CONFIG_SNI_HOSTNAME;
    auto sni_hostname = string_view_to_world_string(*val);
    backend_configuration.sni_hostname = reinterpret_cast<char *>(sni_hostname.ptr);
    backend_configuration.sni_hostname_len = sni_hostname.len;
  }

  if (auto &val = config.client_cert) {
    backend_config_mask |= BACKEND_CONFIG_CLIENT_CERT;
    auto client_cert = string_view_to_world_string(val->cert);
    backend_configuration.client_certificate = reinterpret_cast<char *>(client_cert.ptr);
    backend_configuration.client_certificate_len = client_cert.len;
    backend_configuration.client_key = config.client_cert->key;
  }

  auto name_str = string_view_to_world_string(name);
  auto target_str = string_view_to_world_string(target);
  fastly::fastly_host_error err;
  if (!convert_result(fastly::req_register_dynamic_backend(
                          reinterpret_cast<char *>(name_str.ptr), name_str.len,
                          reinterpret_cast<char *>(target_str.ptr), target_str.len,
                          backend_config_mask, &backend_configuration),
                      &err)) {
    res.emplace_err(err);
  } else {
    res.emplace();
  }

  return res;
}

Result<HttpPendingReq> HttpReq::send_async(HttpBody body, std::string_view backend) {
  Result<HttpPendingReq> res;

  fastly::fastly_host_error err;
  HttpPendingReq::Handle ret;
  fastly::fastly_world_string backend_str = string_view_to_world_string(backend);
  if (!convert_result(fastly::req_send_async(this->handle, body.handle,
                                             reinterpret_cast<char *>(backend_str.ptr),
                                             backend_str.len, &ret),
                      &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(ret);
  }

  return res;
}

Result<HttpPendingReq> HttpReq::send_async_streaming(HttpBody body, std::string_view backend) {
  Result<HttpPendingReq> res;

  fastly::fastly_host_error err;
  HttpPendingReq::Handle ret;
  fastly::fastly_world_string backend_str = string_view_to_world_string(backend);
  if (!convert_result(fastly::req_send_async_streaming(this->handle, body.handle,
                                                       reinterpret_cast<char *>(backend_str.ptr),
                                                       backend_str.len, &ret),
                      &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(ret);
  }

  return res;
}

Result<Void> HttpReq::set_method(std::string_view method) {
  Result<Void> res;

  fastly::fastly_host_error err;
  fastly::fastly_world_string str = string_view_to_world_string(method);
  if (!convert_result(
          fastly::req_method_set(this->handle, reinterpret_cast<char *>(str.ptr), str.len), &err)) {
    res.emplace_err(err);
  }

  return res;
}

Result<HostString> HttpReq::get_method() const {
  Result<HostString> res;

  fastly::fastly_host_error err;
  fastly::fastly_world_string method;
  method.ptr = static_cast<uint8_t *>(cabi_malloc(METHOD_MAX_LEN, 1));
  if (!convert_result(fastly::req_method_get(this->handle, reinterpret_cast<char *>(method.ptr),
                                             METHOD_MAX_LEN, &method.len),
                      &err)) {
    cabi_free(method.ptr);
    res.emplace_err(err);
  } else {
    method.ptr = static_cast<uint8_t *>(cabi_realloc(method.ptr, METHOD_MAX_LEN, 1, method.len));
    res.emplace(make_host_string(method));
  }

  return res;
}

Result<Void> HttpReq::set_uri(std::string_view str) {
  Result<Void> res;

  fastly::fastly_host_error err;
  fastly::fastly_world_string uri = string_view_to_world_string(str);
  if (!convert_result(fastly::req_uri_set(this->handle, reinterpret_cast<char *>(uri.ptr), uri.len),
                      &err)) {
    res.emplace_err(err);
  } else {
    res.emplace();
  }

  return res;
}

Result<HostString> HttpReq::get_uri() const {
  Result<HostString> res;

  fastly::fastly_host_error err;
  fastly::fastly_world_string uri;
  uri.ptr = static_cast<uint8_t *>(cabi_malloc(URI_MAX_LEN, 1));
  if (!convert_result(fastly::req_uri_get(this->handle, reinterpret_cast<char *>(uri.ptr),
                                          URI_MAX_LEN, &uri.len),
                      &err)) {
    cabi_free(uri.ptr);
    res.emplace_err(err);
  } else {
    uri.ptr = static_cast<uint8_t *>(cabi_realloc(uri.ptr, URI_MAX_LEN, 1, uri.len));
    res.emplace(make_host_string(uri));
  }

  return res;
}

Result<Void> HttpReq::cache_override(CacheOverrideTag tag, std::optional<uint32_t> opt_ttl,
                                     std::optional<uint32_t> opt_swr,
                                     std::optional<std::string_view> opt_sk) {
  Result<Void> res;

  fastly::fastly_world_string sk;
  if (opt_sk.has_value()) {
    sk = string_view_to_world_string(opt_sk.value());
  } else {
    sk.ptr = nullptr;
    sk.len = 0;
  }

  fastly::fastly_host_error err;
  if (!convert_result(fastly::req_cache_override_v2_set(this->handle, tag.value,
                                                        opt_ttl.value_or(0), opt_swr.value_or(0),
                                                        reinterpret_cast<char *>(sk.ptr), sk.len),
                      &err)) {
    res.emplace_err(err);
  } else {
    res.emplace();
  }

  return res;
}

Result<Void> HttpReq::set_framing_headers_mode(FramingHeadersMode mode) {
  Result<Void> res;

  fastly::fastly_host_error err;
  if (!convert_result(
          fastly::req_framing_headers_mode_set(this->handle, static_cast<uint32_t>(mode)), &err)) {
    res.emplace_err(err);
  } else {
    res.emplace();
  }

  return res;
}

Result<HostBytes> HttpReq::downstream_client_ip_addr() {
  Result<HostBytes> res;

  fastly::fastly_world_list_u8 octets;
  octets.ptr = static_cast<uint8_t *>(cabi_malloc(16, 1));
  fastly::fastly_host_error err;
  if (!convert_result(fastly::req_downstream_client_ip_addr_get(octets.ptr, &octets.len), &err)) {
    cabi_free(octets.ptr);
    res.emplace_err(err);
  } else {
    res.emplace(make_host_bytes(octets));
  }

  return res;
}

Result<HostBytes> HttpReq::downstream_server_ip_addr() {
  Result<HostBytes> res;

  fastly::fastly_world_list_u8 octets;
  octets.ptr = static_cast<uint8_t *>(cabi_malloc(16, 1));
  fastly::fastly_host_error err;
  if (!convert_result(fastly::req_downstream_server_ip_addr_get(octets.ptr, &octets.len), &err)) {
    cabi_free(octets.ptr);
    res.emplace_err(err);
  } else {
    res.emplace(make_host_bytes(octets));
  }

  return res;
}

// http-req-downstream-tls-cipher-openssl-name: func() -> result<string, error>
Result<HostString> HttpReq::http_req_downstream_tls_cipher_openssl_name() {
  Result<HostString> res;

  fastly::fastly_host_error err;
  fastly::fastly_world_string ret;
  auto default_size = 128;
  ret.ptr = static_cast<uint8_t *>(cabi_malloc(default_size, 4));
  auto status = fastly::req_downstream_tls_cipher_openssl_name(reinterpret_cast<char *>(ret.ptr),
                                                               default_size, &ret.len);
  if (status == FASTLY_HOST_ERROR_BUFFER_LEN) {
    ret.ptr = static_cast<uint8_t *>(cabi_realloc(ret.ptr, default_size, 4, ret.len));
    status = fastly::req_downstream_tls_cipher_openssl_name(reinterpret_cast<char *>(ret.ptr),
                                                            ret.len, &ret.len);
  }

  if (!convert_result(status, &err)) {
    cabi_free(ret.ptr);
    res.emplace_err(err);
  } else {
    res.emplace(make_host_string(ret));
  }

  return res;
}

// http-req-downstream-tls-protocol: func() -> result<string, error>
Result<HostString> HttpReq::http_req_downstream_tls_protocol() {
  Result<HostString> res;

  fastly::fastly_host_error err;
  fastly::fastly_world_string ret;
  auto default_size = 32;
  ret.ptr = static_cast<uint8_t *>(cabi_malloc(default_size, 4));
  auto status = fastly::req_downstream_tls_protocol(reinterpret_cast<char *>(ret.ptr), default_size,
                                                    &ret.len);
  if (status == FASTLY_HOST_ERROR_BUFFER_LEN) {
    ret.ptr = static_cast<uint8_t *>(cabi_realloc(ret.ptr, default_size, 4, ret.len));
    status =
        fastly::req_downstream_tls_protocol(reinterpret_cast<char *>(ret.ptr), ret.len, &ret.len);
  }
  if (!convert_result(status, &err)) {
    cabi_free(ret.ptr);
    res.emplace_err(err);
  } else {
    res.emplace(make_host_string(ret));
  }

  return res;
}

// http-req-downstream-tls-client-hello: func() -> result<list<u8>, error>
Result<HostBytes> HttpReq::http_req_downstream_tls_client_hello() {
  Result<HostBytes> res;

  fastly::fastly_world_list_u8 ret;
  fastly::fastly_host_error err;
  auto default_size = 512;
  ret.ptr = static_cast<uint8_t *>(cabi_malloc(default_size, 4));
  auto status = fastly::req_downstream_tls_client_hello(ret.ptr, default_size, &ret.len);
  if (status == FASTLY_HOST_ERROR_BUFFER_LEN) {
    ret.ptr = static_cast<uint8_t *>(cabi_realloc(ret.ptr, default_size, 4, ret.len));
    status = fastly::req_downstream_tls_client_hello(ret.ptr, ret.len, &ret.len);
  }

  if (!convert_result(status, &err)) {
    cabi_free(ret.ptr);
    res.emplace_err(err);
  } else {
    res.emplace(make_host_bytes(ret));
  }

  return res;
}

// http-req-downstream-tls-raw-client-certificate: func() -> result<list<u8>, error>
Result<HostBytes> HttpReq::http_req_downstream_tls_raw_client_certificate() {
  Result<HostBytes> res;

  fastly::fastly_world_list_u8 ret;
  fastly::fastly_host_error err;
  auto default_size = 4096;
  ret.ptr = static_cast<uint8_t *>(cabi_malloc(default_size, 4));
  auto status = fastly::req_downstream_tls_raw_client_certificate(ret.ptr, default_size, &ret.len);
  if (status == FASTLY_HOST_ERROR_BUFFER_LEN) {
    ret.ptr = static_cast<uint8_t *>(cabi_realloc(ret.ptr, default_size, 4, ret.len));
    status = fastly::req_downstream_tls_raw_client_certificate(ret.ptr, ret.len, &ret.len);
  }
  if (!convert_result(status, &err)) {
    cabi_free(ret.ptr);
    res.emplace_err(err);
  } else {
    res.emplace(make_host_bytes(ret));
  }

  return res;
}

// http-req-downstream-tls-ja3-md5: func() -> result<list<u8>, error>
Result<HostBytes> HttpReq::http_req_downstream_tls_ja3_md5() {
  Result<HostBytes> res;

  fastly::fastly_world_list_u8 ret;
  fastly::fastly_host_error err;
  auto default_size = 16;
  ret.ptr = static_cast<uint8_t *>(cabi_malloc(default_size, 4));
  auto status = fastly::req_downstream_tls_ja3_md5(ret.ptr, &ret.len);
  if (status == FASTLY_HOST_ERROR_BUFFER_LEN) {
    ret.ptr = static_cast<uint8_t *>(cabi_realloc(ret.ptr, default_size, 4, ret.len));
    status = fastly::req_downstream_tls_ja3_md5(ret.ptr, &ret.len);
  }
  if (!convert_result(status, &err)) {
    cabi_free(ret.ptr);
    res.emplace_err(err);
  } else {
    res.emplace(make_host_bytes(ret));
  }

  return res;
}

bool HttpReq::is_valid() const { return this->handle != HttpReq::invalid; }

Result<HttpVersion> HttpReq::get_version() const {
  Result<uint8_t> res;

  fastly::fastly_host_error err;
  uint32_t fastly_http_version;
  if (!convert_result(fastly::req_version_get(this->handle, &fastly_http_version), &err)) {
    res.emplace_err(err);
  } else {
    MOZ_ASSERT(fastly_http_version <= static_cast<int>(std::numeric_limits<uint8_t>::max()));
    res.emplace(fastly_http_version);
  }

  return res;
}

HttpHeadersReadOnly *HttpReq::headers() {
  return new HttpHeadersReadOnly(std::unique_ptr<HandleState>(new HandleState(this->handle, true)));
}

HttpHeaders *HttpReq::headers_writable() { return headers()->clone(); }

Result<HttpResp> HttpResp::make() {
  Result<HttpResp> res;

  HttpResp::Handle handle;
  fastly::fastly_host_error err;
  if (!convert_result(fastly::resp_new(&handle), &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(handle);
  }

  return res;
}

Result<uint16_t> HttpResp::get_status() const {
  Result<uint16_t> res;

  uint16_t ret;
  fastly::fastly_host_error err;
  if (!convert_result(fastly::resp_status_get(this->handle, &ret), &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(ret);
  }

  return res;
}

Result<Void> HttpResp::set_status(uint16_t status) {
  Result<Void> res;

  fastly::fastly_host_error err;
  if (!convert_result(fastly::resp_status_set(this->handle, status), &err)) {
    res.emplace_err(err);
  } else {
    res.emplace();
  }

  return res;
}

Result<Void> HttpResp::send_downstream(HttpBody body, bool streaming) {
  Result<Void> res;

  fastly::fastly_host_error err;
  if (!convert_result(fastly::resp_send_downstream(this->handle, body.handle, streaming), &err)) {
    res.emplace_err(err);
  } else {
    res.emplace();
  }

  return res;
}

Result<Void> HttpResp::set_framing_headers_mode(FramingHeadersMode mode) {
  Result<Void> res;

  fastly::fastly_host_error err;
  if (!convert_result(
          fastly::resp_framing_headers_mode_set(this->handle, static_cast<uint32_t>(mode)), &err)) {
    res.emplace_err(err);
  } else {
    res.emplace();
  }

  return res;
}

bool HttpResp::is_valid() const { return this->handle != HttpResp::invalid; }

Result<HttpVersion> HttpResp::get_version() const {
  Result<HttpVersion> res;

  fastly::fastly_host_error err;
  uint32_t fastly_http_version;
  if (!convert_result(fastly::resp_version_get(this->handle, &fastly_http_version), &err)) {
    res.emplace_err(err);
  } else {
    MOZ_ASSERT(fastly_http_version <= static_cast<int>(std::numeric_limits<uint8_t>::max()));
    res.emplace(fastly_http_version);
  }

  return res;
}

HttpHeadersReadOnly *HttpResp::headers() {
  return new HttpHeadersReadOnly(
      std::unique_ptr<HandleState>(new HandleState(this->handle, false)));
}

HttpHeaders *HttpResp::headers_writable() { return headers()->clone(); }

Result<std::optional<HostBytes>> HttpResp::get_ip() const {
  Result<std::optional<HostBytes>> res;

  fastly::fastly_host_error err;
  fastly::fastly_world_list_u8 ret;

  ret.ptr = static_cast<uint8_t *>(cabi_malloc(16, 1));
  if (!convert_result(fastly::resp_ip_get(this->handle, ret.ptr, &ret.len), &err)) {
    if (error_is_optional_none(err)) {
      res.emplace(std::nullopt);
    } else {
      cabi_free(ret.ptr);
      res.emplace_err(err);
    }
  } else {
    res.emplace(make_host_bytes(ret));
  }
  return res;
}

Result<std::optional<uint16_t>> HttpResp::get_port() const {
  Result<std::optional<uint16_t>> res;

  fastly::fastly_host_error err;
  uint16_t ret;
  if (!convert_result(fastly::resp_port_get(this->handle, &ret), &err)) {
    if (error_is_optional_none(err)) {
      res.emplace(std::nullopt);
    } else {
      res.emplace_err(err);
    }
  } else {
    res.emplace(ret);
  }

  return res;
}

Result<HostString> GeoIp::lookup(std::span<uint8_t> bytes) {
  Result<HostString> res;

  fastly::fastly_world_list_u8 octets_list{const_cast<uint8_t *>(bytes.data()), bytes.size()};
  fastly::fastly_world_string ret;
  fastly::fastly_host_error err;
  ret.ptr = static_cast<uint8_t *>(cabi_malloc(HOSTCALL_BUFFER_LEN, 1));
  if (!convert_result(fastly::geo_lookup(octets_list.ptr, octets_list.len,
                                         reinterpret_cast<char *>(ret.ptr), HOSTCALL_BUFFER_LEN,
                                         &ret.len),
                      &err)) {
    cabi_free(ret.ptr);
    res.emplace_err(err);
  } else {
    ret.ptr = static_cast<uint8_t *>(cabi_realloc(ret.ptr, HOSTCALL_BUFFER_LEN, 1, ret.len));
    res.emplace(make_host_string(ret));
  }

  return res;
}

Result<LogEndpoint> LogEndpoint::get(std::string_view name) {
  Result<LogEndpoint> res;

  auto name_str = string_view_to_world_string(name);
  LogEndpoint::Handle handle;
  fastly::fastly_host_error err;
  if (!convert_result(
          fastly::log_endpoint_get(reinterpret_cast<char *>(name_str.ptr), name_str.len, &handle),
          &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(LogEndpoint{handle});
  }

  return res;
}

Result<Void> LogEndpoint::write(std::string_view msg) {
  Result<Void> res;

  auto msg_str = string_view_to_world_string(msg);
  fastly::fastly_host_error err;
  size_t nwritten = 0;
  if (!convert_result(fastly::log_write(this->handle, reinterpret_cast<char *>(msg_str.ptr),
                                        msg_str.len, &nwritten),
                      &err)) {
    res.emplace_err(err);
  } else {
    res.emplace();
  }

  return res;
}

Result<Dict> Dict::open(std::string_view name) {
  Result<Dict> res;

  auto name_str = string_view_to_world_string(name);
  Dict::Handle ret;
  fastly::fastly_host_error err;
  if (!convert_result(
          fastly::dictionary_open(reinterpret_cast<char *>(name_str.ptr), name_str.len, &ret),
          &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(ret);
  }

  return res;
}

Result<std::optional<HostString>> Dict::get(std::string_view name) {
  Result<std::optional<HostString>> res;

  auto name_str = string_view_to_world_string(name);
  fastly::fastly_world_string ret;
  fastly::fastly_host_error err;

  ret.ptr = static_cast<uint8_t *>(cabi_malloc(DICTIONARY_ENTRY_MAX_LEN, 1));
  if (!convert_result(fastly::dictionary_get(this->handle, reinterpret_cast<char *>(name_str.ptr),
                                             name_str.len, reinterpret_cast<char *>(ret.ptr),
                                             DICTIONARY_ENTRY_MAX_LEN, &ret.len),
                      &err)) {
    if (error_is_optional_none(err)) {
      res.emplace(std::nullopt);
    } else {
      cabi_free(ret.ptr);
      res.emplace_err(err);
    }
  } else {
    ret.ptr = static_cast<uint8_t *>(cabi_realloc(ret.ptr, DICTIONARY_ENTRY_MAX_LEN, 1, ret.len));
    res.emplace(make_host_string(ret));
  }

  return res;
}

Result<ConfigStore> ConfigStore::open(std::string_view name) {
  Result<ConfigStore> res;

  auto name_str = string_view_to_world_string(name);
  ConfigStore::Handle ret;
  fastly::fastly_host_error err;
  if (!convert_result(
          fastly::config_store_open(reinterpret_cast<char *>(name_str.ptr), name_str.len, &ret),
          &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(ret);
  }

  return res;
}

Result<std::optional<HostString>> ConfigStore::get(std::string_view name) {
  Result<std::optional<HostString>> res;

  auto name_str = string_view_to_world_string(name);
  fastly::fastly_world_string ret;
  fastly::fastly_host_error err;
  ret.ptr = static_cast<uint8_t *>(cabi_malloc(CONFIG_STORE_ENTRY_MAX_LEN, 1));
  if (!convert_result(fastly::config_store_get(this->handle, reinterpret_cast<char *>(name_str.ptr),
                                               name_str.len, reinterpret_cast<char *>(ret.ptr),
                                               CONFIG_STORE_ENTRY_MAX_LEN, &ret.len),
                      &err)) {
    if (error_is_optional_none(err)) {
      res.emplace(std::nullopt);
    } else {
      res.emplace_err(err);
      cabi_free(ret.ptr);
    }
  } else {
    ret.ptr = static_cast<uint8_t *>(cabi_realloc(ret.ptr, CONFIG_STORE_ENTRY_MAX_LEN, 1, ret.len));
    res.emplace(make_host_string(ret));
  }

  return res;
}

Result<ObjectStore> ObjectStore::open(std::string_view name) {
  Result<ObjectStore> res;

  auto name_str = string_view_to_world_string(name);
  ObjectStore::Handle ret;
  fastly::fastly_host_error err;
  if (!convert_result(
          fastly::object_store_open(reinterpret_cast<char *>(name_str.ptr), name_str.len, &ret),
          &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(ret);
  }

  return res;
}

Result<std::optional<HttpBody>> ObjectStore::lookup(std::string_view name) {
  Result<std::optional<HttpBody>> res;

  auto name_str = string_view_to_world_string(name);
  ObjectStore::Handle ret;
  fastly::fastly_host_error err;
  bool ok =
      convert_result(fastly::object_store_get(this->handle, reinterpret_cast<char *>(name_str.ptr),
                                              name_str.len, &ret),
                     &err);
  if ((!ok && error_is_optional_none(err)) || ret == INVALID_HANDLE) {
    res.emplace(std::nullopt);
  } else {
    res.emplace(ret);
  }

  return res;
}

Result<ObjectStorePendingLookup::Handle> ObjectStore::lookup_async(std::string_view name) {
  Result<ObjectStorePendingLookup::Handle> res;

  auto name_str = string_view_to_world_string(name);
  ObjectStorePendingLookup::Handle ret;
  fastly::fastly_host_error err;
  if (!convert_result(fastly::object_store_get_async(
                          this->handle, reinterpret_cast<char *>(name_str.ptr), name_str.len, &ret),
                      &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(ret);
  }

  return res;
}

Result<ObjectStorePendingDelete::Handle> ObjectStore::delete_async(std::string_view name) {
  Result<ObjectStorePendingDelete::Handle> res;

  auto name_str = string_view_to_world_string(name);
  ObjectStorePendingDelete::Handle ret;
  fastly::fastly_host_error err;
  if (!convert_result(fastly::object_store_delete_async(
                          this->handle, reinterpret_cast<char *>(name_str.ptr), name_str.len, &ret),
                      &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(ret);
  }

  return res;
}

Result<Void> ObjectStore::insert(std::string_view name, HttpBody body) {
  Result<Void> res;

  auto name_str = string_view_to_world_string(name);
  fastly::fastly_host_error err;
  if (!convert_result(fastly::object_store_insert(this->handle,
                                                  reinterpret_cast<char *>(name_str.ptr),
                                                  name_str.len, body.handle),
                      &err)) {
    res.emplace_err(err);
  } else {
    res.emplace();
  }

  return res;
}

FastlyResult<std::optional<HttpBody>, FastlyAPIError> ObjectStorePendingLookup::wait() {
  FastlyResult<std::optional<HttpBody>, FastlyAPIError> res;

  fastly::fastly_host_error err;
  HttpBody::Handle ret = INVALID_HANDLE;
  bool ok = convert_result(fastly::object_store_pending_lookup_wait(this->handle, &ret), &err);
  if ((!ok && error_is_optional_none(err)) || ret == INVALID_HANDLE) {
    res.emplace(std::nullopt);
  } else {
    res.emplace(ret);
  }

  return res;
}

FastlyAsyncTask::Handle ObjectStorePendingLookup::async_handle() const {
  return FastlyAsyncTask::Handle{this->handle};
}

Result<Void> ObjectStorePendingDelete::wait() {
  Result<Void> res;

  fastly::fastly_host_error err;
  if (!convert_result(fastly::object_store_pending_delete_wait(this->handle), &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(Void{});
  }

  return res;
}

FastlyAsyncTask::Handle ObjectStorePendingDelete::async_handle() const {
  return FastlyAsyncTask::Handle{this->handle};
}

Result<std::optional<HostBytes>> Secret::plaintext() const {
  Result<std::optional<HostBytes>> res;

  fastly::fastly_world_list_u8 ret;
  fastly::fastly_host_error err;
  ret.ptr = static_cast<uint8_t *>(JS_malloc(CONTEXT, DICTIONARY_ENTRY_MAX_LEN));
  if (!convert_result(fastly::secret_store_plaintext(this->handle,
                                                     reinterpret_cast<char *>(ret.ptr),
                                                     DICTIONARY_ENTRY_MAX_LEN, &ret.len),
                      &err)) {
    if (error_is_optional_none(err)) {
      res.emplace(std::nullopt);
    } else {
      JS_free(CONTEXT, ret.ptr);
      res.emplace_err(err);
    }
  } else {
    ret.ptr =
        static_cast<uint8_t *>(JS_realloc(CONTEXT, ret.ptr, DICTIONARY_ENTRY_MAX_LEN, ret.len));
    res.emplace(make_host_bytes(ret));
  }

  return res;
}

Result<SecretStore> SecretStore::open(std::string_view name) {
  Result<SecretStore> res;

  auto name_str = string_view_to_world_string(name);
  SecretStore::Handle ret;
  fastly::fastly_host_error err;
  if (!convert_result(
          fastly::secret_store_open(reinterpret_cast<char *>(name_str.ptr), name_str.len, &ret),
          &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(ret);
  }

  return res;
}

Result<std::optional<Secret>> SecretStore::get(std::string_view name) {
  Result<std::optional<Secret>> res;

  auto name_str = string_view_to_world_string(name);
  Secret::Handle ret = INVALID_HANDLE;
  fastly::fastly_host_error err;
  bool ok =
      convert_result(fastly::secret_store_get(this->handle, reinterpret_cast<char *>(name_str.ptr),
                                              name_str.len, &ret),
                     &err);
  if ((!ok && error_is_optional_none(err)) || ret == INVALID_HANDLE) {
    res.emplace(std::nullopt);
  } else {
    res.emplace(ret);
  }

  return res;
}

Result<Secret> SecretStore::from_bytes(uint8_t *bytes, size_t len) {
  Result<Secret> res;

  fastly::fastly_world_list_u8 bytes_list{const_cast<uint8_t *>(bytes), len};
  Secret::Handle ret = INVALID_HANDLE;
  fastly::fastly_host_error err;
  bool ok = convert_result(fastly::secret_store_from_bytes(reinterpret_cast<char *>(bytes_list.ptr),
                                                           bytes_list.len, &ret),
                           &err);
  if (!ok || ret == INVALID_HANDLE) {
    res.emplace_err(err);
  } else {
    res.emplace(ret);
  }

  return res;
}

bool CacheState::is_found() const { return this->state & FASTLY_HOST_CACHE_LOOKUP_STATE_FOUND; }

bool CacheState::is_usable() const { return this->state & FASTLY_HOST_CACHE_LOOKUP_STATE_USABLE; }

bool CacheState::is_stale() const { return this->state & FASTLY_HOST_CACHE_LOOKUP_STATE_STALE; }

bool CacheState::must_insert_or_update() const {
  return this->state & FASTLY_HOST_CACHE_LOOKUP_STATE_MUST_INSERT_OR_UPDATE;
}

Result<CacheHandle> CacheHandle::lookup(std::string_view key, const CacheLookupOptions &opts) {
  Result<CacheHandle> res;

  auto key_str = string_view_to_world_string(key);

  fastly::fastly_host_error err;
  CacheHandle::Handle handle;
  fastly::fastly_host_cache_lookup_options os;
  memset(&os, 0, sizeof(os));

  uint8_t options_mask = 0;
  if (opts.request_headers.is_valid()) {
    os.request_headers = opts.request_headers.handle;
    options_mask |= FASTLY_CACHE_LOOKUP_OPTIONS_MASK_REQUEST_HEADERS;
  }

  if (!convert_result(fastly::cache_lookup(reinterpret_cast<char *>(key_str.ptr), key_str.len,
                                           options_mask, &os, &handle),
                      &err)) {
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

  fastly::fastly_host_error err;
  CacheHandle::Handle handle;
  fastly::fastly_host_cache_lookup_options os;
  memset(&os, 0, sizeof(os));

  uint32_t options_mask = 0;
  if (opts.request_headers.is_valid()) {
    os.request_headers = opts.request_headers.handle;
    options_mask |= FASTLY_CACHE_LOOKUP_OPTIONS_MASK_REQUEST_HEADERS;
  }

  if (!convert_result(fastly::cache_transaction_lookup(reinterpret_cast<char *>(key_str.ptr),
                                                       key_str.len, options_mask, &os, &handle),
                      &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(handle);
  }

  return res;
}

namespace {

void init_write_options(fastly::fastly_host_cache_write_options &options,
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

#define FASTLY_CACHE_WRITE_OPTIONS_MASK_RESERVED (1 << 0)
#define FASTLY_CACHE_WRITE_OPTIONS_MASK_REQUEST_HEADERS (1 << 1)
#define FASTLY_CACHE_WRITE_OPTIONS_MASK_VARY_RULE (1 << 2)
#define FASTLY_CACHE_WRITE_OPTIONS_MASK_INITIAL_AGE_NS (1 << 3)
#define FASTLY_CACHE_WRITE_OPTIONS_MASK_STALE_WHILE_REVALIDATE_NS (1 << 4)
#define FASTLY_CACHE_WRITE_OPTIONS_MASK_SURROGATE_KEYS (1 << 5)
#define FASTLY_CACHE_WRITE_OPTIONS_MASK_LENGTH (1 << 6)
#define FASTLY_CACHE_WRITE_OPTIONS_MASK_USER_METADATA (1 << 7)
#define FASTLY_CACHE_WRITE_OPTIONS_MASK_SENSITIVE_DATA (1 << 8)

Result<HttpBody> CacheHandle::insert(std::string_view key, const CacheWriteOptions &os) {
  Result<HttpBody> res;

  fastly::fastly_host_cache_write_options options;
  init_write_options(options, os);

  fastly::fastly_host_error err;
  HttpBody::Handle ret;
  auto host_key = string_view_to_world_string(key);

  uint16_t options_mask = 0;
  fastly::CacheWriteOptions opts;
  std::memset(&opts, 0, sizeof(opts));
  opts.max_age_ns = options.max_age_ns;

  if (options.request_headers != INVALID_HANDLE && options.request_headers != 0) {
    options_mask |= FASTLY_CACHE_WRITE_OPTIONS_MASK_REQUEST_HEADERS;
    opts.request_headers = options.request_headers;
  }
  if (options.vary_rule.ptr != nullptr) {
    options_mask |= FASTLY_CACHE_WRITE_OPTIONS_MASK_VARY_RULE;
    opts.vary_rule_len = options.vary_rule.len;
    opts.vary_rule_ptr = reinterpret_cast<uint8_t *>(options.vary_rule.ptr);
  }
  if (options.initial_age_ns != 0) {
    options_mask |= FASTLY_CACHE_WRITE_OPTIONS_MASK_INITIAL_AGE_NS;
    opts.initial_age_ns = options.initial_age_ns;
  }
  if (options.stale_while_revalidate_ns != 0) {
    options_mask |= FASTLY_CACHE_WRITE_OPTIONS_MASK_STALE_WHILE_REVALIDATE_NS;
    opts.stale_while_revalidate_ns = options.stale_while_revalidate_ns;
  }
  if (options.surrogate_keys.ptr != nullptr) {
    options_mask |= FASTLY_CACHE_WRITE_OPTIONS_MASK_SURROGATE_KEYS;
    opts.surrogate_keys_len = options.surrogate_keys.len;
    opts.surrogate_keys_ptr = reinterpret_cast<uint8_t *>(options.surrogate_keys.ptr);
  }
  if (options.length != 0) {
    options_mask |= FASTLY_CACHE_WRITE_OPTIONS_MASK_LENGTH;
    opts.length = options.length;
  }
  if (options.user_metadata.ptr != nullptr) {
    options_mask |= FASTLY_CACHE_WRITE_OPTIONS_MASK_USER_METADATA;
    opts.user_metadata_len = options.user_metadata.len;
    opts.user_metadata_ptr = options.user_metadata.ptr;
  }
  if (options.sensitive_data) {
    options_mask |= FASTLY_CACHE_WRITE_OPTIONS_MASK_SENSITIVE_DATA;
  }

  if (!convert_result(fastly::cache_insert(reinterpret_cast<char *>(host_key.ptr), host_key.len,
                                           options_mask, &opts, &ret),
                      &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(HttpBody{ret});
  }

  return res;
}

Result<HttpBody> CacheHandle::transaction_insert(const CacheWriteOptions &os) {
  Result<HttpBody> res;

  fastly::fastly_host_cache_write_options options;
  init_write_options(options, os);

  fastly::fastly_host_error err;
  HttpBody::Handle ret;

  uint16_t options_mask = 0;
  fastly::CacheWriteOptions opts;
  std::memset(&opts, 0, sizeof(opts));
  opts.max_age_ns = options.max_age_ns;

  if (options.request_headers != INVALID_HANDLE && options.request_headers != 0) {
    options_mask |= FASTLY_CACHE_WRITE_OPTIONS_MASK_REQUEST_HEADERS;
    opts.request_headers = options.request_headers;
  }
  if (options.vary_rule.len > 0) {
    options_mask |= FASTLY_CACHE_WRITE_OPTIONS_MASK_VARY_RULE;
    opts.vary_rule_len = options.vary_rule.len;
    opts.vary_rule_ptr = reinterpret_cast<uint8_t *>(options.vary_rule.ptr);
  }
  if (options.initial_age_ns != 0) {
    options_mask |= FASTLY_CACHE_WRITE_OPTIONS_MASK_INITIAL_AGE_NS;
    opts.initial_age_ns = options.initial_age_ns;
  }
  if (options.stale_while_revalidate_ns != 0) {
    options_mask |= FASTLY_CACHE_WRITE_OPTIONS_MASK_STALE_WHILE_REVALIDATE_NS;
    opts.stale_while_revalidate_ns = options.stale_while_revalidate_ns;
  }
  if (options.surrogate_keys.ptr != nullptr) {
    options_mask |= FASTLY_CACHE_WRITE_OPTIONS_MASK_SURROGATE_KEYS;
    opts.surrogate_keys_len = options.surrogate_keys.len;
    opts.surrogate_keys_ptr = reinterpret_cast<uint8_t *>(options.surrogate_keys.ptr);
  }
  if (options.length != 0) {
    options_mask |= FASTLY_CACHE_WRITE_OPTIONS_MASK_LENGTH;
    opts.length = options.length;
  }
  if (options.user_metadata.ptr != nullptr) {
    options_mask |= FASTLY_CACHE_WRITE_OPTIONS_MASK_USER_METADATA;
    opts.user_metadata_len = options.user_metadata.len;
    opts.user_metadata_ptr = options.user_metadata.ptr;
  }
  if (options.sensitive_data) {
    options_mask |= FASTLY_CACHE_WRITE_OPTIONS_MASK_SENSITIVE_DATA;
  }

  if (!convert_result(fastly::cache_transaction_insert(this->handle, options_mask, &opts, &ret),
                      &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(HttpBody{ret});
  }

  return res;
}

Result<Void> CacheHandle::transaction_update(const CacheWriteOptions &os) {
  Result<Void> res;

  fastly::fastly_host_cache_write_options options;
  init_write_options(options, os);

  fastly::fastly_host_error err;

  uint16_t options_mask = 0;
  fastly::CacheWriteOptions opts;
  std::memset(&opts, 0, sizeof(opts));
  opts.max_age_ns = options.max_age_ns;

  if (options.request_headers != INVALID_HANDLE && options.request_headers != 0) {
    options_mask |= FASTLY_CACHE_WRITE_OPTIONS_MASK_REQUEST_HEADERS;
    opts.request_headers = options.request_headers;
  }
  if (options.vary_rule.ptr != nullptr) {
    options_mask |= FASTLY_CACHE_WRITE_OPTIONS_MASK_VARY_RULE;
    opts.vary_rule_len = options.vary_rule.len;
    opts.vary_rule_ptr = reinterpret_cast<uint8_t *>(options.vary_rule.ptr);
  }
  if (options.initial_age_ns != 0) {
    options_mask |= FASTLY_CACHE_WRITE_OPTIONS_MASK_INITIAL_AGE_NS;
    opts.initial_age_ns = options.initial_age_ns;
  }
  if (options.stale_while_revalidate_ns != 0) {
    options_mask |= FASTLY_CACHE_WRITE_OPTIONS_MASK_STALE_WHILE_REVALIDATE_NS;
    opts.stale_while_revalidate_ns = options.stale_while_revalidate_ns;
  }
  if (options.surrogate_keys.ptr != nullptr) {
    options_mask |= FASTLY_CACHE_WRITE_OPTIONS_MASK_SURROGATE_KEYS;
    opts.surrogate_keys_len = options.surrogate_keys.len;
    opts.surrogate_keys_ptr = reinterpret_cast<uint8_t *>(options.surrogate_keys.ptr);
  }
  if (options.length != 0) {
    options_mask |= FASTLY_CACHE_WRITE_OPTIONS_MASK_LENGTH;
    opts.length = options.length;
  }
  if (options.user_metadata.ptr != nullptr) {
    options_mask |= FASTLY_CACHE_WRITE_OPTIONS_MASK_USER_METADATA;
    opts.user_metadata_len = options.user_metadata.len;
    opts.user_metadata_ptr = options.user_metadata.ptr;
  }
  if (options.sensitive_data) {
    options_mask |= FASTLY_CACHE_WRITE_OPTIONS_MASK_SENSITIVE_DATA;
  }

  if (!convert_result(fastly::cache_transaction_update(this->handle, options_mask, &opts), &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(Void{});
  }

  return res;
}

Result<std::tuple<HttpBody, CacheHandle>>
CacheHandle::transaction_insert_and_stream_back(const CacheWriteOptions &os) {
  Result<std::tuple<HttpBody, CacheHandle>> res;

  fastly::fastly_host_cache_write_options options;
  init_write_options(options, os);

  fastly::fastly_host_error err;
  fastly::fastly_world_tuple2_handle_handle ret;

  uint16_t options_mask = 0;
  fastly::CacheWriteOptions opts;
  std::memset(&opts, 0, sizeof(opts));
  opts.max_age_ns = options.max_age_ns;

  MOZ_ASSERT(options.request_headers == INVALID_HANDLE || options.request_headers == 0);

  if (options.vary_rule.ptr != nullptr) {
    options_mask |= FASTLY_CACHE_WRITE_OPTIONS_MASK_VARY_RULE;
    opts.vary_rule_len = options.vary_rule.len;
    opts.vary_rule_ptr = options.vary_rule.ptr;
  }
  if (options.initial_age_ns != 0) {
    options_mask |= FASTLY_CACHE_WRITE_OPTIONS_MASK_INITIAL_AGE_NS;
    opts.initial_age_ns = options.initial_age_ns;
  }
  if (options.stale_while_revalidate_ns != 0) {
    options_mask |= FASTLY_CACHE_WRITE_OPTIONS_MASK_STALE_WHILE_REVALIDATE_NS;
    opts.stale_while_revalidate_ns = options.stale_while_revalidate_ns;
  }
  if (options.surrogate_keys.ptr != nullptr) {
    options_mask |= FASTLY_CACHE_WRITE_OPTIONS_MASK_SURROGATE_KEYS;
    opts.surrogate_keys_len = options.surrogate_keys.len;
    opts.surrogate_keys_ptr = options.surrogate_keys.ptr;
  }
  if (options.length != 0) {
    options_mask |= FASTLY_CACHE_WRITE_OPTIONS_MASK_LENGTH;
    opts.length = options.length;
  }
  if (options.user_metadata.ptr != nullptr) {
    options_mask |= FASTLY_CACHE_WRITE_OPTIONS_MASK_USER_METADATA;
    opts.user_metadata_len = options.user_metadata.len;
    opts.user_metadata_ptr = options.user_metadata.ptr;
  }
  if (options.sensitive_data) {
    options_mask |= FASTLY_CACHE_WRITE_OPTIONS_MASK_SENSITIVE_DATA;
  }

  if (!convert_result(fastly::cache_transaction_insert_and_stream_back(this->handle, options_mask,
                                                                       &opts, &ret.f0, &ret.f1),
                      &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(HttpBody{ret.f0}, CacheHandle{ret.f1});
  }

  return res;
}

Result<Void> CacheHandle::transaction_cancel() {
  Result<Void> res;

  fastly::fastly_host_error err;
  if (!convert_result(fastly::cache_transaction_cancel(this->handle), &err)) {
    res.emplace_err(err);
  } else {
    res.emplace();
  }

  return res;
}

Result<HttpBody> CacheHandle::get_body(const CacheGetBodyOptions &opts) {
  Result<HttpBody> res;

  fastly::fastly_host_cache_get_body_options options{};
  uint32_t options_mask = 0;
  if (opts.start.has_value()) {
    options_mask |= FASTLY_HOST_CACHE_GET_BODY_OPTIONS_MASK_START;
    options.start = opts.start.value();
  }
  if (opts.end.has_value()) {
    options_mask |= FASTLY_HOST_CACHE_GET_BODY_OPTIONS_MASK_END;
    options.end = opts.end.value();
  }

  HttpBody::Handle body = INVALID_HANDLE;
  fastly::fastly_host_error err;

  bool ok =
      convert_result(fastly::cache_get_body(this->handle, options_mask, &options, &body), &err);
  if (!ok && !error_is_optional_none(err)) {
    res.emplace_err(err);
  } else {
    res.emplace(HttpBody{body});
  }

  return res;
}

Result<CacheState> CacheHandle::get_state() {
  Result<CacheState> res;

  fastly::fastly_host_error err;
  alignas(4) uint8_t state;
  if (!convert_result(fastly::cache_get_state(this->handle, &state), &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(CacheState{state});
  }

  return res;
}

Result<Void> CacheHandle::close() {

  fastly::fastly_host_error err;
  if (!convert_result(fastly::cache_close(this->handle), &err)) {
    return Result<Void>::err(err);
  }

  return Result<Void>::ok();
  ;
}

Result<HostBytes> CacheHandle::get_user_metadata() {
  Result<HostBytes> res;

  fastly::fastly_world_list_u8 ret;
  fastly::fastly_host_error err;

  size_t default_size = 16 * 1024;
  ret.ptr = static_cast<uint8_t *>(cabi_malloc(default_size, 4));
  auto status = fastly::cache_get_user_metadata(handle, reinterpret_cast<char *>(ret.ptr),
                                                default_size, &ret.len);
  if (status == FASTLY_HOST_ERROR_BUFFER_LEN) {
    ret.ptr = static_cast<uint8_t *>(cabi_realloc(ret.ptr, default_size, 4, ret.len));
    status = fastly::cache_get_user_metadata(handle, reinterpret_cast<char *>(ret.ptr), ret.len,
                                             &ret.len);
  }

  if (!convert_result(status, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(make_host_bytes(ret));
  }

  return res;
}

Result<uint64_t> CacheHandle::get_length() {
  Result<uint64_t> res;

  uint64_t ret;
  fastly::fastly_host_error err;
  if (!convert_result(fastly::cache_get_length(this->handle, &ret), &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(ret);
  }

  return res;
}

Result<uint64_t> CacheHandle::get_max_age_ns() {
  Result<uint64_t> res;

  uint64_t ret;
  fastly::fastly_host_error err;
  if (!convert_result(fastly::cache_get_max_age_ns(this->handle, &ret), &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(ret);
  }

  return res;
}

Result<uint64_t> CacheHandle::get_stale_while_revalidate_ns() {
  Result<uint64_t> res;

  uint64_t ret;
  fastly::fastly_host_error err;
  if (!convert_result(fastly::cache_get_stale_while_revalidate_ns(this->handle, &ret), &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(ret);
  }

  return res;
}

Result<uint64_t> CacheHandle::get_age_ns() {
  Result<uint64_t> res;

  uint64_t ret;
  fastly::fastly_host_error err;
  if (!convert_result(fastly::cache_get_age_ns(this->handle, &ret), &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(ret);
  }

  return res;
}

Result<uint64_t> CacheHandle::get_hits() {
  Result<uint64_t> res;

  uint64_t ret;
  fastly::fastly_host_error err;
  if (!convert_result(fastly::cache_get_hits(this->handle, &ret), &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(ret);
  }

  return res;
}

Result<std::optional<HostString>> Fastly::purge_surrogate_key(std::string_view key) {
  Result<std::optional<HostString>> res;

  auto host_key = string_view_to_world_string(key);
  fastly::fastly_host_error err;
  // TODO: we don't currently define any meaningful options in fastly.wit
  uint32_t options_mask = 0;

  fastly::PurgeOptions options{nullptr, 0, nullptr};

  // Currently this host-call has been implemented to support the `SimpleCache.delete(key)` method,
  // which uses hard-purging and not soft-purging.
  // TODO: Create a JS API for this hostcall which supports hard-purging and another which supports
  // soft-purging. E.G. `fastly.purgeSurrogateKey(key)` and `fastly.softPurgeSurrogateKey(key)`
  MOZ_ASSERT(!(options_mask & FASTLY_HOST_PURGE_OPTIONS_MASK_SOFT_PURGE));
  MOZ_ASSERT(!(options_mask & FASTLY_HOST_PURGE_OPTIONS_MASK_RET_BUF));

  if (!convert_result(fastly::purge_surrogate_key(reinterpret_cast<char *>(host_key.ptr),
                                                  host_key.len, options_mask, &options),
                      &err)) {
    res.emplace_err(err);
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
  case dns_error: {
    // allocate maximum len of error message
    char buf[34 + 10 + 1];
    int written = snprintf(buf, sizeof(buf), "DNS error (rcode=%d, info_code=%d)",
                           this->dns_error_rcode, this->dns_error_info_code);
    MOZ_ASSERT(written > 34);
    return std::string(buf, written);
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
    // allocate maximum len of error message
    char buf[34 + 1];
    int written =
        snprintf(buf, sizeof(buf), "TLS alert received (alert_id=%d)", this->tls_alert_id);
    MOZ_ASSERT(written > 35);
    return std::string(buf, written);
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
  return this->state & FASTLY_HOST_BACKEND_BACKEND_HEALTH_UNKNOWN;
}

bool BackendHealth::is_healthy() const {
  return this->state & FASTLY_HOST_BACKEND_BACKEND_HEALTH_HEALTHY;
}

bool BackendHealth::is_unhealthy() const {
  return this->state & FASTLY_HOST_BACKEND_BACKEND_HEALTH_UNHEALTHY;
}

Result<bool> Backend::exists(std::string_view name) {
  Result<bool> res;

  auto name_str = string_view_to_world_string(name);
  alignas(4) bool ret;
  fastly::fastly_host_error err;
  if (!convert_result(fastly::backend_exists(reinterpret_cast<char *>(name_str.ptr), name_str.len,
                                             (uint32_t *)&ret),
                      &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(ret);
  }

  return res;
}

Result<BackendHealth> Backend::health(std::string_view name) {
  Result<BackendHealth> res;

  auto name_str = string_view_to_world_string(name);
  fastly::fastly_host_error err;

  uint32_t fastly_backend_health;
  if (!convert_result(fastly::backend_is_healthy(reinterpret_cast<char *>(name_str.ptr),
                                                 name_str.len, &fastly_backend_health),
                      &err)) {
    res.emplace_err(err);
  } else {
    switch (fastly_backend_health) {
    case FASTLY_HOST_BACKEND_BACKEND_HEALTH_UNKNOWN:
    case FASTLY_HOST_BACKEND_BACKEND_HEALTH_HEALTHY:
    case FASTLY_HOST_BACKEND_BACKEND_HEALTH_UNHEALTHY: {
      res.emplace(BackendHealth(fastly_backend_health));
      break;
    }
    default: {
      MOZ_CRASH("Making a BackendHealth from an invalid value");
    }
    }
  }

  return res;
}

Result<Void> RateCounter::increment(std::string_view name, std::string_view entry, uint32_t delta) {
  auto name_str = string_view_to_world_string(name);
  auto entry_str = string_view_to_world_string(entry);
  fastly::fastly_host_error err;
  if (!convert_result(fastly::ratecounter_increment(
                          reinterpret_cast<char *>(name_str.ptr), name_str.len,
                          reinterpret_cast<char *>(entry_str.ptr), entry_str.len, delta),
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
  fastly::fastly_host_error err;
  if (!convert_result(fastly::ratecounter_lookup_rate(
                          reinterpret_cast<char *>(name_str.ptr), name_str.len,
                          reinterpret_cast<char *>(entry_str.ptr), entry_str.len, window, &ret),
                      &err)) {
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
  fastly::fastly_host_error err;
  if (!convert_result(fastly::ratecounter_lookup_count(
                          reinterpret_cast<char *>(name_str.ptr), name_str.len,
                          reinterpret_cast<char *>(entry_str.ptr), entry_str.len, duration, &ret),
                      &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(ret);
  }

  return res;
}

Result<Void> PenaltyBox::add(std::string_view name, std::string_view entry, uint32_t time_to_live) {
  auto name_str = string_view_to_world_string(name);
  auto entry_str = string_view_to_world_string(entry);
  fastly::fastly_host_error err;
  if (!convert_result(fastly::penaltybox_add(reinterpret_cast<char *>(name_str.ptr), name_str.len,
                                             reinterpret_cast<char *>(entry_str.ptr), entry_str.len,
                                             time_to_live),
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
  fastly::fastly_host_error err;
  if (!convert_result(fastly::penaltybox_has(reinterpret_cast<char *>(name_str.ptr), name_str.len,
                                             reinterpret_cast<char *>(entry_str.ptr), entry_str.len,
                                             &ret),
                      &err)) {
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
  fastly::fastly_host_error err;
  if (!convert_result(fastly::check_rate(reinterpret_cast<char *>(rate_counter_name_str.ptr),
                                         rate_counter_name_str.len,
                                         reinterpret_cast<char *>(entry_str.ptr), entry_str.len,
                                         delta, window, limit,
                                         reinterpret_cast<char *>(penalty_box_name_str.ptr),
                                         penalty_box_name_str.len, time_to_live, &ret),
                      &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(ret);
  }
  return res;
}

Result<HostString> DeviceDetection::lookup(std::string_view user_agent) {
  Result<HostString> res;

  auto user_agent_str = string_view_to_world_string(user_agent);
  fastly::fastly_host_error err;
  fastly::fastly_world_string ret;

  auto default_size = 1024;
  ret.ptr = static_cast<uint8_t *>(cabi_malloc(default_size, 4));
  auto status = fastly::device_detection_lookup(
      reinterpret_cast<char *>(user_agent_str.ptr), user_agent_str.len,
      reinterpret_cast<char *>(ret.ptr), default_size, &ret.len);
  if (status == FASTLY_HOST_ERROR_BUFFER_LEN) {
    ret.ptr = static_cast<uint8_t *>(cabi_realloc(ret.ptr, default_size, 4, ret.len));
    status = fastly::device_detection_lookup(reinterpret_cast<char *>(user_agent_str.ptr),
                                             user_agent_str.len, reinterpret_cast<char *>(ret.ptr),
                                             ret.len, &ret.len);
  }

  if (!convert_result(status, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(make_host_string(ret));
  }
  return res;
}

} // namespace host_api
