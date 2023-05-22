#include <algorithm>

#include "core/allocator.h"
#include "fastly-world/fastly_world.h"
#include "host_interface/host_api.h"

namespace {

fastly_world_string_t string_view_to_world_string(std::string_view str) {
  return {
      .ptr = const_cast<char *>(str.data()),
      .len = str.size(),
  };
}

} // namespace

Result<bool> AsyncHandle::is_ready() const {
  Result<bool> res;

  fastly_error_t err;
  bool ret;
  if (!fastly_async_io_is_ready(this->handle, &ret, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(ret);
  }

  return res;
}

Result<std::optional<uint32_t>> AsyncHandle::select(const std::vector<AsyncHandle> &handles,
                                                    uint32_t timeout_ms) {
  Result<std::optional<uint32_t>> res;

  static_assert(sizeof(AsyncHandle) == sizeof(fastly_async_handle_t));
  fastly_list_async_handle_t hs{
      .ptr = reinterpret_cast<fastly_async_handle_t *>(const_cast<AsyncHandle *>(handles.data())),
      .len = handles.size()};
  fastly_option_u32_t ret;
  fastly_error_t err;
  if (!fastly_async_io_select(&hs, timeout_ms, &ret, &err)) {
    res.emplace_err(err);
  } else if (ret.is_some) {
    res.emplace(ret.val);
  } else {
    res.emplace(std::nullopt);
  }

  return res;
}

Result<HttpBody> HttpBody::make() {
  Result<HttpBody> res;

  fastly_body_handle_t handle;
  fastly_error_t err;
  if (!fastly_http_body_new(&handle, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(handle);
  }

  return res;
}

Result<HostString> HttpBody::read(uint32_t chunk_size) const {
  Result<HostString> res;

  fastly_list_u8_t ret;
  fastly_error_t err;
  if (!fastly_http_body_read(this->handle, chunk_size, &ret, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(JS::UniqueChars(reinterpret_cast<char *>(ret.ptr)), ret.len);
  }

  return res;
}

Result<uint32_t> HttpBody::write(const uint8_t *ptr, size_t len) const {
  Result<uint32_t> res;

  // The write call doesn't mutate the buffer; the cast is just for the generated fastly api.
  fastly_list_u8_t chunk{const_cast<uint8_t *>(ptr), len};

  fastly_error_t err;
  uint32_t written;
  if (!fastly_http_body_write(this->handle, &chunk, FASTLY_BODY_WRITE_END_BACK, &written, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(written);
  }

  return res;
}

Result<Void> HttpBody::write_all(const uint8_t *ptr, size_t len) const {
  while (len > 0) {
    auto write_res = this->write(ptr, len);
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

  fastly_error_t err;
  if (!fastly_http_body_append(this->handle, other.handle, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace();
  }

  return res;
}

Result<Void> HttpBody::close() {
  Result<Void> res;

  fastly_error_t err;
  if (!fastly_http_body_close(this->handle, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace();
  }

  return res;
}

AsyncHandle HttpBody::async_handle() const { return AsyncHandle{this->handle}; }

namespace {

template <auto header_names_get>
Result<std::vector<HostString>> generic_get_header_names(auto handle) {
  Result<std::vector<HostString>> res;

  fastly_list_string_t ret;
  fastly_error_t err;
  if (!header_names_get(handle, &ret, &err)) {
    res.emplace_err(err);
  } else {
    std::vector<HostString> names;

    for (int i = 0; i < ret.len; i++) {
      names.emplace_back(HostString{ret.ptr[i]});
    }

    // Free the vector of string pointers, but leave the individual strings alone.
    cabi_free(ret.ptr);

    res.emplace(std::move(names));
  }

  return res;
}

template <auto header_values_get>
Result<std::optional<std::vector<HostString>>> generic_get_header_values(auto handle,
                                                                         std::string_view name) {
  Result<std::optional<std::vector<HostString>>> res;

  fastly_world_string_t hdr = string_view_to_world_string(name);
  fastly_option_list_string_t ret;
  fastly_error_t err;
  if (!header_values_get(handle, &hdr, &ret, &err)) {
    res.emplace_err(err);
  } else {

    if (ret.is_some) {
      std::vector<HostString> names;

      for (int i = 0; i < ret.val.len; i++) {
        names.emplace_back(HostString{ret.val.ptr[i]});
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
Result<Void> generic_header_op(auto handle, std::string_view name, std::string_view value) {
  Result<Void> res;

  fastly_world_string_t hdr = string_view_to_world_string(name);
  fastly_world_string_t val = string_view_to_world_string(value);
  fastly_error_t err;
  if (!header_op(handle, &hdr, &val, &err)) {
    res.emplace_err(err);
  }

  return res;
}

template <auto remove_header>
Result<Void> generic_header_remove(auto handle, std::string_view name) {
  Result<Void> res;

  fastly_world_string_t hdr = string_view_to_world_string(name);
  fastly_error_t err;
  if (!remove_header(handle, &hdr, &err)) {
    res.emplace_err(err);
  }

  return res;
}

} // namespace

Result<std::optional<Response>> HttpPendingReq::poll() {
  Result<std::optional<Response>> res;

  fastly_error_t err;
  fastly_option_response_t ret;
  if (!fastly_http_req_pending_req_poll(this->handle, &ret, &err)) {
    res.emplace_err(err);
  } else if (ret.is_some) {
    res.emplace(ret.val);
  } else {
    res.emplace(std::nullopt);
  }

  return res;
}

Result<Response> HttpPendingReq::wait() {
  Result<Response> res;

  fastly_error_t err;
  fastly_response_t ret;
  if (!fastly_http_req_pending_req_wait(this->handle, &ret, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(ret);
  }

  return res;
}

AsyncHandle HttpPendingReq::async_handle() const { return AsyncHandle{this->handle}; }

Result<HttpReq> HttpReq::make() {
  Result<HttpReq> res;

  fastly_request_handle_t handle;
  fastly_error_t err;
  if (!fastly_http_req_new(&handle, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(handle);
  }

  return res;
}

Result<Void> HttpReq::redirect_to_grip_proxy(std::string_view backend) {
  Result<Void> res;

  fastly_error_t err;
  fastly_world_string_t backend_str = string_view_to_world_string(backend);
  if (!fastly_http_req_redirect_to_grip_proxy(&backend_str, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace();
  }

  return res;
}

Result<Response> HttpReq::send(HttpBody body, std::string_view backend) {
  Result<Response> res;

  fastly_error_t err;
  fastly_response_t ret;
  fastly_world_string_t backend_str = string_view_to_world_string(backend);
  if (!fastly_http_req_send(this->handle, body.handle, &backend_str, &ret, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(ret);
  }

  return res;
}

Result<HttpPendingReq> HttpReq::send_async(HttpBody body, std::string_view backend) {
  Result<HttpPendingReq> res;

  fastly_error_t err;
  fastly_pending_request_handle_t ret;
  fastly_world_string_t backend_str = string_view_to_world_string(backend);
  if (!fastly_http_req_send_async(this->handle, body.handle, &backend_str, &ret, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(ret);
  }

  return res;
}

Result<HttpPendingReq> HttpReq::send_async_streaming(HttpBody body, std::string_view backend) {
  Result<HttpPendingReq> res;

  fastly_error_t err;
  fastly_pending_request_handle_t ret;
  fastly_world_string_t backend_str = string_view_to_world_string(backend);
  if (!fastly_http_req_send_async_streaming(this->handle, body.handle, &backend_str, &ret, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(ret);
  }

  return res;
}

Result<Void> HttpReq::set_method(std::string_view method) {
  Result<Void> res;

  fastly_error_t err;
  fastly_world_string_t str = string_view_to_world_string(method);
  if (!fastly_http_req_method_set(this->handle, &str, &err)) {
    res.emplace_err(err);
  }

  return res;
}

Result<HostString> HttpReq::get_method() const {
  Result<HostString> res;

  fastly_error_t err;
  fastly_world_string_t ret;
  if (!fastly_http_req_method_get(this->handle, &ret, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(ret);
  }

  return res;
}

Result<Void> HttpReq::set_uri(std::string_view str) {
  Result<Void> res;

  fastly_error_t err;
  fastly_world_string_t uri = string_view_to_world_string(str);
  if (!fastly_http_req_uri_set(this->handle, &uri, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace();
  }

  return res;
}

Result<HostString> HttpReq::get_uri() const {
  Result<HostString> res;

  fastly_error_t err;
  fastly_world_string_t uri;
  if (!fastly_http_req_uri_get(this->handle, &uri, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(uri);
  }

  return res;
}

Result<Void> HttpReq::cache_override(fastly_http_cache_override_tag_t tag,
                                     std::optional<uint32_t> opt_ttl,
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

  fastly_error_t err;
  if (!fastly_http_req_cache_override_set(this->handle, tag, ttl, swr, &sk, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace();
  }

  return res;
}

bool HttpReq::is_valid() const { return this->handle != HttpReq::invalid; }

Result<fastly_http_version_t> HttpReq::get_version() const {
  Result<fastly_http_version_t> res;

  fastly_error_t err;
  fastly_http_version_t ret;
  if (!fastly_http_req_version_get(this->handle, &ret, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(ret);
  }

  return res;
}

Result<std::vector<HostString>> HttpReq::get_header_names() {
  return generic_get_header_names<fastly_http_req_header_names_get>(this->handle);
}

Result<std::optional<std::vector<HostString>>> HttpReq::get_header_values(std::string_view name) {
  return generic_get_header_values<fastly_http_req_header_values_get>(this->handle, name);
}

Result<Void> HttpReq::insert_header(std::string_view name, std::string_view value) {
  return generic_header_op<fastly_http_req_header_insert>(this->handle, name, value);
}

Result<Void> HttpReq::append_header(std::string_view name, std::string_view value) {
  return generic_header_op<fastly_http_req_header_append>(this->handle, name, value);
}

Result<Void> HttpReq::remove_header(std::string_view name) {
  return generic_header_remove<fastly_http_req_header_remove>(this->handle, name);
}

Result<HttpResp> HttpResp::make() {
  Result<HttpResp> res;

  fastly_response_handle_t handle;
  fastly_error_t err;
  if (!fastly_http_resp_new(&handle, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(handle);
  }

  return res;
}

Result<uint16_t> HttpResp::get_status() const {
  Result<uint16_t> res;

  fastly_http_status_t ret;
  fastly_error_t err;
  if (!fastly_http_resp_status_get(this->handle, &ret, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(ret);
  }

  return res;
}

Result<Void> HttpResp::set_status(uint16_t status) {
  Result<Void> res;

  fastly_error_t err;
  if (!fastly_http_resp_status_set(this->handle, status, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace();
  }

  return res;
}

Result<Void> HttpResp::send_downstream(HttpBody body, bool streaming) {
  Result<Void> res;

  fastly_error_t err;
  if (!fastly_http_resp_send_downstream(this->handle, body.handle, streaming, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace();
  }

  return res;
}

bool HttpResp::is_valid() const { return this->handle != HttpResp::invalid; }

Result<fastly_http_version_t> HttpResp::get_version() const {
  Result<fastly_http_version_t> res;

  fastly_error_t err;
  fastly_http_version_t ret;
  if (!fastly_http_resp_version_get(this->handle, &ret, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(ret);
  }

  return res;
}

Result<std::vector<HostString>> HttpResp::get_header_names() {
  return generic_get_header_names<fastly_http_resp_header_names_get>(this->handle);
}

Result<std::optional<std::vector<HostString>>> HttpResp::get_header_values(std::string_view name) {
  return generic_get_header_values<fastly_http_resp_header_values_get>(this->handle, name);
}

Result<Void> HttpResp::insert_header(std::string_view name, std::string_view value) {
  return generic_header_op<fastly_http_resp_header_insert>(this->handle, name, value);
}

Result<Void> HttpResp::append_header(std::string_view name, std::string_view value) {
  return generic_header_op<fastly_http_resp_header_append>(this->handle, name, value);
}

Result<Void> HttpResp::remove_header(std::string_view name) {
  return generic_header_remove<fastly_http_resp_header_remove>(this->handle, name);
}
