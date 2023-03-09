#include "host_api.h"
#include "c_at_e_world.h"
#include "c_at_e_world_adapter.h"

Result<HttpBody> HttpBody::make() {
  Result<HttpBody> res;

  fastly_body_handle_t handle;
  fastly_error_t err;
  if (!c_at_e_fastly_http_body_new(&handle, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(handle);
  }

  return res;
}

Result<HttpBodyChunk> HttpBody::read(uint32_t chunk_size) const {
  Result<HttpBodyChunk> res;

  fastly_list_u8_t ret;
  fastly_error_t err;
  if (!c_at_e_fastly_http_body_read(this->handle, chunk_size, &ret, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace(JS::UniqueChars(reinterpret_cast<char *>(ret.ptr)), ret.len);
  }

  return res;
}

Result<uint32_t> HttpBody::write(const uint8_t *ptr, size_t len) const {
  Result<uint32_t> res;

  // The write call doesn't mutate the buffer; the cast is just for the generated c-at-e api.
  fastly_list_u8_t chunk{const_cast<uint8_t *>(ptr), len};

  fastly_error_t err;
  uint32_t written;
  if (!c_at_e_fastly_http_body_write(this->handle, &chunk, FASTLY_BODY_WRITE_END_BACK, &written,
                                     &err)) {
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
  if (!c_at_e_fastly_http_body_append(this->handle, other.handle, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace();
  }

  return res;
}

Result<Void> HttpBody::close() {
  Result<Void> res;

  fastly_error_t err;
  if (!c_at_e_fastly_http_body_close(this->handle, &err)) {
    res.emplace_err(err);
  } else {
    res.emplace();
  }

  return res;
}
