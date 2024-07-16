#include <algorithm>
#include <string_view>
#include <vector>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-offsetof"
#include "js/Utility.h"
#include "jsapi.h"
#pragma clang diagnostic pop

#include "../../../StarlingMonkey/runtime/allocator.h"
#include "../fastly.h"
#include "fastly_world.h"

// Ensure that all the things we want to use the hostcall buffer for actually
// fit into the buffer.
#define HOSTCALL_BUFFER_LEN HEADER_MAX_LEN
static_assert(DICTIONARY_ENTRY_MAX_LEN < HOSTCALL_BUFFER_LEN);
static_assert(METHOD_MAX_LEN < HOSTCALL_BUFFER_LEN);
static_assert(URI_MAX_LEN < HOSTCALL_BUFFER_LEN);

#define LIST_ALLOC_SIZE 50

static bool convert_result(int res, fastly_compute_at_edge_types_error_t *err) {
  if (res == 0)
    return true;
  switch (res) {
  case 1:
    *err = FASTLY_COMPUTE_AT_EDGE_TYPES_ERROR_GENERIC_ERROR;
    break;
  case 2:
    *err = FASTLY_COMPUTE_AT_EDGE_TYPES_ERROR_INVALID_ARGUMENT;
    break;
  case 3:
    *err = FASTLY_COMPUTE_AT_EDGE_TYPES_ERROR_BAD_HANDLE;
    break;
  case 4:
    *err = FASTLY_COMPUTE_AT_EDGE_TYPES_ERROR_BUFFER_LEN;
    break;
  case 5:
    *err = FASTLY_COMPUTE_AT_EDGE_TYPES_ERROR_UNSUPPORTED;
    break;
  case 6:
    *err = FASTLY_COMPUTE_AT_EDGE_TYPES_ERROR_BAD_ALIGN;
    break;
  case 7:
    *err = FASTLY_COMPUTE_AT_EDGE_TYPES_ERROR_HTTP_INVALID;
    break;
  case 8:
    *err = FASTLY_COMPUTE_AT_EDGE_TYPES_ERROR_HTTP_USER;
    break;
  case 9:
    *err = FASTLY_COMPUTE_AT_EDGE_TYPES_ERROR_HTTP_INCOMPLETE;
    break;
  case 10:
    *err = FASTLY_COMPUTE_AT_EDGE_TYPES_ERROR_OPTIONAL_NONE;
    break;
  case 11:
    *err = FASTLY_COMPUTE_AT_EDGE_TYPES_ERROR_HTTP_HEAD_TOO_LARGE;
    break;
  case 12:
    *err = FASTLY_COMPUTE_AT_EDGE_TYPES_ERROR_HTTP_INVALID_STATUS;
    break;
  case 13:
    *err = FASTLY_COMPUTE_AT_EDGE_TYPES_ERROR_LIMIT_EXCEEDED;
    break;
  case 100:
    *err = FASTLY_COMPUTE_AT_EDGE_TYPES_ERROR_UNKNOWN_ERROR;
    break;
  default:
    *err = FASTLY_COMPUTE_AT_EDGE_TYPES_ERROR_UNKNOWN_ERROR;
  }
  return false;
}

fastly_compute_at_edge_http_types_http_version_t convert_http_version(uint32_t version) {
  switch (version) {
  case 0:
    return FASTLY_COMPUTE_AT_EDGE_HTTP_TYPES_HTTP_VERSION_HTTP09;
  case 1:
    return FASTLY_COMPUTE_AT_EDGE_HTTP_TYPES_HTTP_VERSION_HTTP10;
  case 2:
    return FASTLY_COMPUTE_AT_EDGE_HTTP_TYPES_HTTP_VERSION_HTTP11;
  case 3:
    return FASTLY_COMPUTE_AT_EDGE_HTTP_TYPES_HTTP_VERSION_H2;
  case 4:
  default:
    return FASTLY_COMPUTE_AT_EDGE_HTTP_TYPES_HTTP_VERSION_H3;
  }
}

bool fastly_compute_at_edge_http_body_new(fastly_compute_at_edge_http_types_body_handle_t *ret,
                                          fastly_compute_at_edge_types_error_t *err) {
  return convert_result(fastly::body_new(ret), err);
}

bool fastly_compute_at_edge_http_body_append(fastly_compute_at_edge_http_types_body_handle_t src,
                                             fastly_compute_at_edge_http_types_body_handle_t dest,
                                             fastly_compute_at_edge_types_error_t *err) {
  return convert_result(fastly::body_append(src, dest), err);
}

bool fastly_compute_at_edge_http_body_read(fastly_compute_at_edge_http_types_body_handle_t h,
                                           uint32_t chunk_size, fastly_world_list_u8_t *ret,
                                           fastly_compute_at_edge_types_error_t *err) {
  ret->ptr = static_cast<uint8_t *>(cabi_malloc(chunk_size, 1));
  return convert_result(fastly::body_read(h, ret->ptr, static_cast<size_t>(chunk_size), &ret->len),
                        err);
}

bool fastly_compute_at_edge_http_body_write(fastly_compute_at_edge_http_types_body_handle_t h,
                                            fastly_world_list_u8_t *buf,
                                            fastly_compute_at_edge_http_body_write_end_t end,
                                            uint32_t *ret,
                                            fastly_compute_at_edge_types_error_t *err) {
  return convert_result(fastly::body_write(h, buf->ptr, buf->len,
                                           end == FASTLY_COMPUTE_AT_EDGE_HTTP_BODY_WRITE_END_BACK
                                               ? fastly::BodyWriteEnd::BodyWriteEndBack
                                               : fastly::BodyWriteEnd::BodyWriteEndFront,
                                           reinterpret_cast<size_t *>(ret)),
                        err);
}

bool fastly_compute_at_edge_http_body_close(fastly_compute_at_edge_http_types_body_handle_t h,
                                            fastly_compute_at_edge_types_error_t *err) {
  return convert_result(fastly::body_close(h), err);
}

bool fastly_compute_at_edge_log_endpoint_get(fastly_world_string_t *name,
                                             fastly_compute_at_edge_log_handle_t *ret,
                                             fastly_compute_at_edge_types_error_t *err) {
  return convert_result(
      fastly::log_endpoint_get(reinterpret_cast<char *>(name->ptr), name->len, ret), err);
}

bool fastly_compute_at_edge_log_write(fastly_compute_at_edge_log_handle_t h,
                                      fastly_world_string_t *msg,
                                      fastly_compute_at_edge_types_error_t *err) {
  size_t nwritten = 0;
  return convert_result(
      fastly::log_write(h, reinterpret_cast<char *>(msg->ptr), msg->len, &nwritten), err);
}

bool fastly_http_req_body_downstream_get(fastly_compute_at_edge_http_types_request_t *ret,
                                         fastly_compute_at_edge_types_error_t *err) {
  return convert_result(fastly::req_body_downstream_get(&ret->f0, &ret->f1), err);
}

bool fastly_compute_at_edge_http_req_redirect_to_grip_proxy(
    fastly_world_string_t *backend, fastly_compute_at_edge_types_error_t *err) {
  return convert_result(
      fastly::req_redirect_to_grip_proxy(reinterpret_cast<char *>(backend->ptr), backend->len),
      err);
}

int convert_tag(fastly_compute_at_edge_http_req_cache_override_tag_t tag) {
  int out_tag = 0;
  if ((tag & FASTLY_COMPUTE_AT_EDGE_HTTP_REQ_CACHE_OVERRIDE_TAG_PASS) > 0) {
    out_tag |= CACHE_OVERRIDE_PASS;
  }
  if ((tag & FASTLY_COMPUTE_AT_EDGE_HTTP_REQ_CACHE_OVERRIDE_TAG_TTL) > 0) {
    out_tag |= CACHE_OVERRIDE_TTL;
  }
  if ((tag & FASTLY_COMPUTE_AT_EDGE_HTTP_REQ_CACHE_OVERRIDE_TAG_STALE_WHILE_REVALIDATE) > 0) {
    out_tag |= CACHE_OVERRIDE_STALE_WHILE_REVALIDATE;
  }
  if ((tag & FASTLY_COMPUTE_AT_EDGE_HTTP_REQ_CACHE_OVERRIDE_TAG_PCI) > 0) {
    out_tag |= CACHE_OVERRIDE_PCI;
  }
  return out_tag;
}

bool fastly_compute_at_edge_http_req_cache_override_set(
    fastly_compute_at_edge_http_types_request_handle_t h,
    fastly_compute_at_edge_http_req_cache_override_tag_t tag, uint32_t *maybe_ttl,
    uint32_t *maybe_stale_while_revalidate, fastly_world_string_t *maybe_sk,
    fastly_compute_at_edge_types_error_t *err) {
  fastly_world_string_t sk_str;
  if (maybe_sk) {
    sk_str = *maybe_sk;
  } else {
    sk_str.len = 0;
    sk_str.ptr = NULL;
  }
  return convert_result(
      fastly::req_cache_override_v2_set(
          h, convert_tag(tag), maybe_ttl == NULL ? 0 : *maybe_ttl,
          maybe_stale_while_revalidate == NULL ? 0 : *maybe_stale_while_revalidate,
          reinterpret_cast<char *>(sk_str.ptr), sk_str.len),
      err);
}

bool fastly_compute_at_edge_http_req_auto_decompress_response_set(
    fastly_compute_at_edge_http_types_request_handle_t h,
    fastly_compute_at_edge_http_types_content_encodings_t encodings,
    fastly_compute_at_edge_types_error_t *err) {
  return convert_result(fastly::req_auto_decompress_response_set(h, encodings), err);
}

bool fastly_compute_at_edge_http_req_downstream_client_ip_addr(
    fastly_world_list_u8_t *ret, fastly_compute_at_edge_types_error_t *err) {
  ret->ptr = static_cast<uint8_t *>(cabi_malloc(16, 1));
  return convert_result(fastly::req_downstream_client_ip_addr_get(ret->ptr, &ret->len), err);
}

bool fastly_compute_at_edge_http_req_downstream_tls_cipher_openssl_name(
    fastly_world_string_t *ret, fastly_compute_at_edge_types_error_t *err) {
  auto default_size = 128;
  ret->ptr = static_cast<uint8_t *>(cabi_malloc(default_size, 4));
  auto status = fastly::req_downstream_tls_cipher_openssl_name(reinterpret_cast<char *>(ret->ptr),
                                                               default_size, &ret->len);
  if (status == FASTLY_COMPUTE_AT_EDGE_TYPES_ERROR_BUFFER_LEN) {
    ret->ptr = static_cast<uint8_t *>(cabi_realloc(ret->ptr, default_size, 4, ret->len));
    status = fastly::req_downstream_tls_cipher_openssl_name(reinterpret_cast<char *>(ret->ptr),
                                                            ret->len, &ret->len);
  }
  return convert_result(status, err);
}

bool fastly_compute_at_edge_http_req_downstream_tls_protocol(
    fastly_world_string_t *ret, fastly_compute_at_edge_types_error_t *err) {
  auto default_size = 32;
  ret->ptr = static_cast<uint8_t *>(cabi_malloc(default_size, 4));
  auto status = fastly::req_downstream_tls_protocol(reinterpret_cast<char *>(ret->ptr),
                                                    default_size, &ret->len);
  if (status == FASTLY_COMPUTE_AT_EDGE_TYPES_ERROR_BUFFER_LEN) {
    ret->ptr = static_cast<uint8_t *>(cabi_realloc(ret->ptr, default_size, 4, ret->len));
    status = fastly::req_downstream_tls_protocol(reinterpret_cast<char *>(ret->ptr), ret->len,
                                                 &ret->len);
  }
  return convert_result(status, err);
}

bool fastly_compute_at_edge_http_req_downstream_tls_raw_client_certificate(
    fastly_world_list_u8_t *ret, fastly_compute_at_edge_types_error_t *err) {
  auto default_size = 4096;
  ret->ptr = static_cast<uint8_t *>(cabi_malloc(default_size, 4));
  auto status =
      fastly::req_downstream_tls_raw_client_certificate(ret->ptr, default_size, &ret->len);
  if (status == FASTLY_COMPUTE_AT_EDGE_TYPES_ERROR_BUFFER_LEN) {
    ret->ptr = static_cast<uint8_t *>(cabi_realloc(ret->ptr, default_size, 4, ret->len));
    status = fastly::req_downstream_tls_raw_client_certificate(ret->ptr, ret->len, &ret->len);
  }
  return convert_result(status, err);
}

bool fastly_compute_at_edge_http_req_downstream_tls_ja3_md5(
    fastly_world_list_u8_t *ret, fastly_compute_at_edge_types_error_t *err) {
  auto default_size = 16;
  ret->ptr = static_cast<uint8_t *>(cabi_malloc(default_size, 4));
  auto status = fastly::req_downstream_tls_ja3_md5(ret->ptr, &ret->len);
  if (status == FASTLY_COMPUTE_AT_EDGE_TYPES_ERROR_BUFFER_LEN) {
    ret->ptr = static_cast<uint8_t *>(cabi_realloc(ret->ptr, default_size, 4, ret->len));
    status = fastly::req_downstream_tls_ja3_md5(ret->ptr, &ret->len);
  }
  return convert_result(status, err);
}
bool fastly_compute_at_edge_http_req_downstream_tls_client_hello(
    fastly_world_list_u8_t *ret, fastly_compute_at_edge_types_error_t *err) {
  auto default_size = 512;
  ret->ptr = static_cast<uint8_t *>(cabi_malloc(default_size, 4));
  auto status = fastly::req_downstream_tls_client_hello(ret->ptr, default_size, &ret->len);
  if (status == FASTLY_COMPUTE_AT_EDGE_TYPES_ERROR_BUFFER_LEN) {
    ret->ptr = static_cast<uint8_t *>(cabi_realloc(ret->ptr, default_size, 4, ret->len));
    status = fastly::req_downstream_tls_client_hello(ret->ptr, ret->len, &ret->len);
  }
  return convert_result(status, err);
}

bool fastly_compute_at_edge_http_req_new(fastly_compute_at_edge_http_types_request_handle_t *ret,
                                         fastly_compute_at_edge_types_error_t *err) {
  return convert_result(fastly::req_new(ret), err);
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

bool fastly_compute_at_edge_http_req_header_names_get(
    fastly_compute_at_edge_http_types_request_handle_t h, fastly_world_list_string_t *ret,
    fastly_compute_at_edge_types_error_t *err) {
  std::vector<Chunk> header_names;
  {
    JS::UniqueChars buf{static_cast<char *>(cabi_malloc(HOSTCALL_BUFFER_LEN, 1))};
    uint32_t cursor = 0;
    while (true) {
      size_t length = 0;
      int64_t ending_cursor = 0;
      auto res = fastly::req_header_names_get(h, reinterpret_cast<uint8_t *>(buf.get()),
                                              HEADER_MAX_LEN, cursor, &ending_cursor, &length);
      if (!convert_result(res, err)) {
        return false;
      }

      if (length == 0) {
        break;
      }

      std::string_view result{buf.get(), length};
      while (!result.empty()) {
        auto end = result.find('\0');
        header_names.emplace_back(Chunk::make(result.substr(0, end)));
        if (end == result.npos) {
          break;
        }

        result = result.substr(end + 1);
      }

      if (ending_cursor < 0) {
        break;
      }

      cursor = ending_cursor;
    }
  }

  ret->len = header_names.size();
  ret->ptr = static_cast<fastly_world_string_t *>(cabi_malloc(
      header_names.size() * sizeof(fastly_world_string_t), alignof(fastly_world_string_t)));
  auto *next = ret->ptr;
  for (auto &chunk : header_names) {
    next->len = chunk.length;
    next->ptr = reinterpret_cast<uint8_t *>(chunk.buffer.release());
    ++next;
  }

  return true;
}

bool fastly_compute_at_edge_http_req_header_values_get(
    fastly_compute_at_edge_http_types_request_handle_t h, fastly_world_string_t *name,
    fastly_world_option_list_list_u8_t *ret, fastly_compute_at_edge_types_error_t *err) {

  std::vector<Chunk> header_values;

  {
    JS::UniqueLatin1Chars buffer(static_cast<unsigned char *>(cabi_malloc(HEADER_MAX_LEN, 1)));
    uint32_t cursor = 0;
    while (true) {
      int64_t ending_cursor = 0;
      size_t length = 0;
      auto res = fastly::req_header_values_get(h, reinterpret_cast<char *>(name->ptr), name->len,
                                               buffer.get(), HEADER_MAX_LEN, cursor, &ending_cursor,
                                               &length);
      if (!convert_result(res, err)) {
        return false;
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
  }

  if (header_values.empty()) {
    ret->is_some = false;
  } else {
    ret->is_some = true;
    ret->val.len = header_values.size();
    ret->val.ptr = static_cast<fastly_world_list_u8_t *>(cabi_malloc(
        header_values.size() * sizeof(fastly_world_list_u8_t), alignof(fastly_world_list_u8_t)));
    auto *next = ret->val.ptr;
    for (auto &chunk : header_values) {
      next->len = chunk.length;
      next->ptr = reinterpret_cast<uint8_t *>(chunk.buffer.release());
      ++next;
    }
  }

  return true;
}

bool fastly_compute_at_edge_http_req_framing_headers_mode_set(
    fastly_compute_at_edge_http_req_request_handle_t h,
    fastly_compute_at_edge_http_req_framing_headers_mode_t mode,
    fastly_compute_at_edge_http_req_error_t *err) {
  return convert_result(fastly::req_framing_headers_mode_set(h, mode), err);
}

bool fastly_compute_at_edge_http_req_header_insert(
    fastly_compute_at_edge_http_types_request_handle_t h, fastly_world_string_t *name,
    fastly_world_list_u8_t *value, fastly_compute_at_edge_types_error_t *err) {
  return convert_result(fastly::req_header_insert(h, reinterpret_cast<char *>(name->ptr), name->len,
                                                  value->ptr, value->len),
                        err);
}

bool fastly_compute_at_edge_http_req_header_append(
    fastly_compute_at_edge_http_types_request_handle_t h, fastly_world_string_t *name,
    fastly_world_list_u8_t *value, fastly_compute_at_edge_types_error_t *err) {
  return convert_result(fastly::req_header_append(h, reinterpret_cast<char *>(name->ptr), name->len,
                                                  value->ptr, value->len),
                        err);
}

bool fastly_compute_at_edge_http_req_header_remove(
    fastly_compute_at_edge_http_types_request_handle_t h, fastly_world_string_t *name,
    fastly_compute_at_edge_types_error_t *err) {
  return convert_result(
      fastly::req_header_remove(h, reinterpret_cast<char *>(name->ptr), name->len), err);
}

bool fastly_compute_at_edge_http_req_method_get(
    fastly_compute_at_edge_http_types_request_handle_t h, fastly_world_string_t *ret,
    fastly_compute_at_edge_types_error_t *err) {
  ret->ptr = static_cast<uint8_t *>(cabi_malloc(METHOD_MAX_LEN, 1));
  return convert_result(
      fastly::req_method_get(h, reinterpret_cast<char *>(ret->ptr), METHOD_MAX_LEN, &ret->len),
      err);
}

bool fastly_compute_at_edge_http_req_method_set(
    fastly_compute_at_edge_http_types_request_handle_t h, fastly_world_string_t *method,
    fastly_compute_at_edge_types_error_t *err) {
  return convert_result(
      fastly::req_method_set(h, reinterpret_cast<char *>(method->ptr), method->len), err);
}

bool fastly_compute_at_edge_http_req_uri_get(fastly_compute_at_edge_http_types_request_handle_t h,
                                             fastly_world_string_t *ret,
                                             fastly_compute_at_edge_types_error_t *err) {
  ret->ptr = static_cast<uint8_t *>(cabi_malloc(URI_MAX_LEN, 1));
  if (!convert_result(
          fastly::req_uri_get(h, reinterpret_cast<char *>(ret->ptr), URI_MAX_LEN, &ret->len),
          err)) {
    cabi_free(ret->ptr);
    return false;
  }
  ret->ptr = static_cast<uint8_t *>(cabi_realloc(ret->ptr, URI_MAX_LEN, 1, ret->len));
  return true;
}

bool fastly_compute_at_edge_http_req_uri_set(fastly_compute_at_edge_http_types_request_handle_t h,
                                             fastly_world_string_t *uri,
                                             fastly_compute_at_edge_types_error_t *err) {
  return convert_result(fastly::req_uri_set(h, reinterpret_cast<char *>(uri->ptr), uri->len), err);
}

bool fastly_compute_at_edge_http_req_version_get(
    fastly_compute_at_edge_http_types_request_handle_t h,
    fastly_compute_at_edge_http_types_http_version_t *ret,
    fastly_compute_at_edge_types_error_t *err) {
  uint32_t fastly_http_version;
  if (!convert_result(fastly::req_version_get(h, &fastly_http_version), err)) {
    return false;
  }
  *ret = convert_http_version(fastly_http_version);
  return true;
}

bool fastly_compute_at_edge_http_req_send_async(
    fastly_compute_at_edge_http_types_request_handle_t h,
    fastly_compute_at_edge_http_types_body_handle_t b, fastly_world_string_t *backend,
    fastly_compute_at_edge_http_types_pending_request_handle_t *ret,
    fastly_compute_at_edge_types_error_t *err) {
  return convert_result(
      fastly::req_send_async(h, b, reinterpret_cast<char *>(backend->ptr), backend->len, ret), err);
}

bool fastly_compute_at_edge_http_req_send_async_streaming(
    fastly_compute_at_edge_http_types_request_handle_t h,
    fastly_compute_at_edge_http_types_body_handle_t b, fastly_world_string_t *backend,
    fastly_compute_at_edge_http_types_pending_request_handle_t *ret,
    fastly_compute_at_edge_types_error_t *err) {
  return convert_result(fastly::req_send_async_streaming(
                            h, b, reinterpret_cast<char *>(backend->ptr), backend->len, ret),
                        err);
}

bool fastly_compute_at_edge_http_req_pending_req_wait(
    fastly_compute_at_edge_http_types_pending_request_handle_t h,
    fastly_compute_at_edge_http_types_response_t *ret, fastly_compute_at_edge_types_error_t *err) {
  return convert_result(fastly::req_pending_req_wait(h, &ret->f0, &ret->f1), err);
}

bool fastly_compute_at_edge_http_req_pending_req_wait_v2(
    fastly_compute_at_edge_http_req_pending_request_handle_t h,
    fastly_compute_at_edge_http_req_send_error_detail_t *s,
    fastly_compute_at_edge_http_req_response_t *ret, fastly_compute_at_edge_http_req_error_t *err) {
  fastly_compute_at_edge_types_error_t host_err;

  s->mask |= FASTLY_COMPUTE_AT_EDGE_HTTP_TYPES_SEND_ERROR_DETAIL_MASK_DNS_ERROR_RCODE;
  s->mask |= FASTLY_COMPUTE_AT_EDGE_HTTP_TYPES_SEND_ERROR_DETAIL_MASK_DNS_ERROR_INFO_CODE;
  s->mask |= FASTLY_COMPUTE_AT_EDGE_HTTP_TYPES_SEND_ERROR_DETAIL_MASK_TLS_ALERT_ID;

  return convert_result(fastly::req_pending_req_wait_v2(h, s, &ret->f0, &ret->f1), &host_err);
}

bool fastly_compute_at_edge_http_req_register_dynamic_backend(
    fastly_world_string_t *prefix, fastly_world_string_t *target,
    fastly_compute_at_edge_http_types_dynamic_backend_config_t *config,
    fastly_compute_at_edge_types_error_t *err) {
  uint32_t backend_config_mask = 0;

  if (config->use_ssl.is_some && config->use_ssl.val) {
    backend_config_mask |= BACKEND_CONFIG_USE_SSL;
  }
  if (config->dont_pool.is_some && config->dont_pool.val) {
    backend_config_mask |= BACKEND_CONFIG_DONT_POOL;
  }
  if (config->host_override.is_some) {
    backend_config_mask |= BACKEND_CONFIG_HOST_OVERRIDE;
  }
  if (config->connect_timeout.is_some) {
    backend_config_mask |= BACKEND_CONFIG_CONNECT_TIMEOUT;
  }
  if (config->first_byte_timeout.is_some) {
    backend_config_mask |= BACKEND_CONFIG_FIRST_BYTE_TIMEOUT;
  }
  if (config->between_bytes_timeout.is_some) {
    backend_config_mask |= BACKEND_CONFIG_BETWEEN_BYTES_TIMEOUT;
  }
  if (config->ssl_min_version.is_some) {
    backend_config_mask |= BACKEND_CONFIG_SSL_MIN_VERSION;
  }
  if (config->ssl_max_version.is_some) {
    backend_config_mask |= BACKEND_CONFIG_SSL_MAX_VERSION;
  }
  if (config->cert_hostname.is_some) {
    backend_config_mask |= BACKEND_CONFIG_CERT_HOSTNAME;
  }
  if (config->ca_cert.is_some) {
    backend_config_mask |= BACKEND_CONFIG_CA_CERT;
  }
  if (config->ciphers.is_some) {
    backend_config_mask |= BACKEND_CONFIG_CIPHERS;
  }
  if (config->sni_hostname.is_some) {
    backend_config_mask |= BACKEND_CONFIG_SNI_HOSTNAME;
  }
  if (config->client_cert.is_some) {
    backend_config_mask |= BACKEND_CONFIG_CLIENT_CERT;
  }
  fastly::DynamicBackendConfig backend_configuration{
      .host_override = reinterpret_cast<char *>(config->host_override.val.ptr),
      .host_override_len = config->host_override.val.len,
      .connect_timeout_ms = config->connect_timeout.val,
      .first_byte_timeout_ms = config->first_byte_timeout.val,
      .between_bytes_timeout_ms = config->between_bytes_timeout.val,
      .ssl_min_version = config->ssl_min_version.val,
      .ssl_max_version = config->ssl_max_version.val,
      .cert_hostname = reinterpret_cast<char *>(config->cert_hostname.val.ptr),
      .cert_hostname_len = config->cert_hostname.val.len,
      .ca_cert = reinterpret_cast<char *>(config->ca_cert.val.ptr),
      .ca_cert_len = config->ca_cert.val.len,
      .ciphers = reinterpret_cast<char *>(config->ciphers.val.ptr),
      .ciphers_len = config->ciphers.val.len,
      .sni_hostname = reinterpret_cast<char *>(config->sni_hostname.val.ptr),
      .sni_hostname_len = config->sni_hostname.val.len,
      .client_certificate = reinterpret_cast<char *>(config->client_cert.val.client_cert.ptr),
      .client_certificate_len = config->client_cert.val.client_cert.len,
      .client_key = config->client_cert.val.client_key};
  return convert_result(
      fastly::req_register_dynamic_backend(reinterpret_cast<char *>(prefix->ptr), prefix->len,
                                           reinterpret_cast<char *>(target->ptr), target->len,
                                           backend_config_mask, &backend_configuration),
      err);
}

bool fastly_compute_at_edge_http_resp_new(fastly_compute_at_edge_http_types_response_handle_t *ret,
                                          fastly_compute_at_edge_types_error_t *err) {
  return convert_result(fastly::resp_new(ret), err);
}

bool fastly_compute_at_edge_http_resp_header_names_get(
    fastly_compute_at_edge_http_types_response_handle_t h, fastly_world_list_string_t *ret,
    fastly_compute_at_edge_types_error_t *err) {
  fastly_world_string_t *strs = static_cast<fastly_world_string_t *>(
      cabi_malloc(LIST_ALLOC_SIZE * sizeof(fastly_world_string_t), 1));
  size_t str_max = LIST_ALLOC_SIZE;
  size_t str_cnt = 0;
  size_t nwritten;
  uint8_t *buf = static_cast<uint8_t *>(cabi_malloc(HOSTCALL_BUFFER_LEN, 1));
  uint32_t cursor = 0;
  int64_t next_cursor = 0;
  while (true) {
    if (!convert_result(
            fastly::resp_header_names_get(h, buf, HEADER_MAX_LEN, cursor, &next_cursor, &nwritten),
            err)) {
      cabi_free(buf);
      return false;
    }
    if (nwritten == 0) {
      break;
    }
    uint32_t offset = 0;
    for (size_t i = 0; i < nwritten; i++) {
      if (buf[i] != '\0')
        continue;
      if (str_cnt == str_max) {
        strs = static_cast<fastly_world_string_t *>(
            cabi_realloc(strs, str_max * sizeof(fastly_world_string_t), 1,
                         (str_max + LIST_ALLOC_SIZE) * sizeof(fastly_world_string_t)));
        str_max += LIST_ALLOC_SIZE;
      }
      strs[str_cnt].ptr = static_cast<uint8_t *>(cabi_malloc(i - offset + 1, 1));
      strs[str_cnt].len = i - offset;
      memcpy(strs[str_cnt].ptr, buf + offset, i - offset + 1);
      offset = i + 1;
      str_cnt++;
    }
    if (next_cursor < 0)
      break;
    cursor = (uint32_t)next_cursor;
  }
  cabi_free(buf);
  if (str_cnt != 0) {
    strs = static_cast<fastly_world_string_t *>(cabi_realloc(
        strs, str_max * sizeof(fastly_world_string_t), 1, str_cnt * sizeof(fastly_world_string_t)));
  }
  ret->ptr = strs;
  ret->len = str_cnt;
  return true;
}

bool fastly_compute_at_edge_http_resp_header_values_get(
    fastly_compute_at_edge_http_types_response_handle_t h, fastly_world_string_t *name,
    fastly_world_option_list_list_u8_t *ret, fastly_compute_at_edge_types_error_t *err) {
  size_t str_max = LIST_ALLOC_SIZE;
  fastly_world_list_u8_t *strs = static_cast<fastly_world_list_u8_t *>(
      cabi_malloc(str_max * sizeof(fastly_world_list_u8_t), 1));
  size_t str_cnt = 0;
  size_t nwritten;
  uint8_t *buf = static_cast<uint8_t *>(cabi_malloc(HOSTCALL_BUFFER_LEN, 1));
  uint32_t cursor = 0;
  int64_t next_cursor = 0;
  while (true) {
    if (!convert_result(fastly::resp_header_values_get(h, reinterpret_cast<char *>(name->ptr),
                                                       name->len, buf, HEADER_MAX_LEN, cursor,
                                                       &next_cursor, &nwritten),
                        err)) {
      cabi_free(buf);
      return false;
    }
    if (nwritten == 0)
      break;
    uint32_t offset = 0;
    for (size_t i = 0; i < nwritten; i++) {
      if (buf[i] != '\0')
        continue;
      if (str_cnt == str_max) {
        strs = static_cast<fastly_world_list_u8_t *>(
            cabi_realloc(strs, str_max * sizeof(fastly_world_list_u8_t), 1,
                         (str_max + LIST_ALLOC_SIZE) * sizeof(fastly_world_list_u8_t)));
        str_max += LIST_ALLOC_SIZE;
      }
      strs[str_cnt].ptr = static_cast<uint8_t *>(cabi_malloc(i - offset + 1, 1));
      strs[str_cnt].len = i - offset;
      memcpy(strs[str_cnt].ptr, buf + offset, i - offset + 1);
      offset = i + 1;
      str_cnt++;
    }
    if (next_cursor < 0)
      break;
    cursor = static_cast<uint32_t>(next_cursor);
  }
  cabi_free(buf);
  if (str_cnt > 0) {
    ret->is_some = true;
    ret->val.ptr = strs;
    ret->val.len = str_cnt;
    strs = static_cast<fastly_world_list_u8_t *>(
        cabi_realloc(strs, str_max * sizeof(fastly_world_list_u8_t), 1,
                     str_cnt * sizeof(fastly_world_list_u8_t)));
  } else {
    ret->is_some = false;
    cabi_free(strs);
  }
  return true;
}

bool fastly_compute_at_edge_http_resp_header_insert(
    fastly_compute_at_edge_http_types_response_handle_t h, fastly_world_string_t *name,
    fastly_world_list_u8_t *value, fastly_compute_at_edge_types_error_t *err) {
  return convert_result(fastly::resp_header_insert(h, reinterpret_cast<char *>(name->ptr),
                                                   name->len, value->ptr, value->len),
                        err);
}

bool fastly_compute_at_edge_http_resp_header_append(
    fastly_compute_at_edge_http_types_response_handle_t h, fastly_world_string_t *name,
    fastly_world_list_u8_t *value, fastly_compute_at_edge_types_error_t *err) {
  return convert_result(fastly::resp_header_append(h, reinterpret_cast<char *>(name->ptr),
                                                   name->len, value->ptr, value->len),
                        err);
}

bool fastly_compute_at_edge_http_resp_header_remove(
    fastly_compute_at_edge_http_types_response_handle_t h, fastly_world_string_t *name,
    fastly_compute_at_edge_types_error_t *err) {
  return convert_result(
      fastly::resp_header_remove(h, reinterpret_cast<char *>(name->ptr), name->len), err);
}

bool fastly_compute_at_edge_http_resp_version_get(
    fastly_compute_at_edge_http_types_response_handle_t h,
    fastly_compute_at_edge_http_types_http_version_t *ret,
    fastly_compute_at_edge_types_error_t *err) {
  uint32_t fastly_http_version;
  if (!convert_result(fastly::resp_version_get(h, &fastly_http_version), err)) {
    return false;
  }
  *ret = convert_http_version(fastly_http_version);
  return true;
}

bool fastly_compute_at_edge_http_resp_framing_headers_mode_set(
    fastly_compute_at_edge_http_resp_response_handle_t h,
    fastly_compute_at_edge_http_resp_framing_headers_mode_t mode,
    fastly_compute_at_edge_http_resp_error_t *err) {
  return convert_result(fastly::resp_framing_headers_mode_set(h, mode), err);
}

bool fastly_compute_at_edge_http_resp_send_downstream(
    fastly_compute_at_edge_http_types_response_handle_t h,
    fastly_compute_at_edge_http_types_body_handle_t b, bool streaming,
    fastly_compute_at_edge_types_error_t *err) {
  return convert_result(fastly::resp_send_downstream(h, b, streaming), err);
}

bool fastly_compute_at_edge_http_resp_status_get(
    fastly_compute_at_edge_http_types_response_handle_t h,
    fastly_compute_at_edge_http_types_http_status_t *ret,
    fastly_compute_at_edge_types_error_t *err) {
  return convert_result(fastly::resp_status_get(h, ret), err);
}

bool fastly_compute_at_edge_http_resp_status_set(
    fastly_compute_at_edge_http_types_response_handle_t h,
    fastly_compute_at_edge_http_types_http_status_t status,
    fastly_compute_at_edge_types_error_t *err) {
  return convert_result(fastly::resp_status_set(h, status), err);
}

bool fastly_compute_at_edge_http_resp_ip_get(fastly_compute_at_edge_http_resp_response_handle_t h,
                                             fastly_world_option_list_u8_t *ret,
                                             fastly_compute_at_edge_http_resp_error_t *err) {
  ret->val.ptr = static_cast<uint8_t *>(cabi_malloc(16, 1));
  if (!convert_result(fastly::resp_ip_get(h, ret->val.ptr, &ret->val.len), err)) {
    if (*err == FASTLY_COMPUTE_AT_EDGE_TYPES_ERROR_OPTIONAL_NONE) {
      ret->is_some = false;
      return true;
    } else {
      cabi_free(ret->val.ptr);
      return false;
    }
  }
  ret->is_some = true;
  return true;
}

bool fastly_compute_at_edge_http_resp_port_get(fastly_compute_at_edge_http_resp_response_handle_t h,
                                               fastly_world_option_u16_t *ret,
                                               fastly_compute_at_edge_http_resp_error_t *err) {
  if (!convert_result(fastly::resp_port_get(h, &ret->val), err)) {
    if (*err == FASTLY_COMPUTE_AT_EDGE_TYPES_ERROR_OPTIONAL_NONE) {
      ret->is_some = false;
      return true;
    } else {
      return false;
    }
  }
  ret->is_some = true;
  return true;
}

bool fastly_compute_at_edge_dictionary_open(fastly_world_string_t *name,
                                            fastly_compute_at_edge_dictionary_handle_t *ret,
                                            fastly_compute_at_edge_types_error_t *err) {
  return convert_result(
      fastly::dictionary_open(reinterpret_cast<char *>(name->ptr), name->len, ret), err);
}

bool fastly_compute_at_edge_dictionary_get(fastly_compute_at_edge_dictionary_handle_t h,
                                           fastly_world_string_t *key,
                                           fastly_world_option_string_t *ret,
                                           fastly_compute_at_edge_types_error_t *err) {
  ret->val.ptr = static_cast<uint8_t *>(cabi_malloc(DICTIONARY_ENTRY_MAX_LEN, 1));
  if (!convert_result(fastly::dictionary_get(h, reinterpret_cast<char *>(key->ptr), key->len,
                                             reinterpret_cast<char *>(ret->val.ptr),
                                             DICTIONARY_ENTRY_MAX_LEN, &ret->val.len),
                      err)) {
    if (*err == FASTLY_COMPUTE_AT_EDGE_TYPES_ERROR_OPTIONAL_NONE) {
      ret->is_some = false;
      return true;
    } else {
      cabi_free(ret->val.ptr);
      return false;
    }
  }
  ret->is_some = true;
  ret->val.ptr =
      static_cast<uint8_t *>(cabi_realloc(ret->val.ptr, DICTIONARY_ENTRY_MAX_LEN, 1, ret->val.len));
  return true;
}

bool fastly_compute_at_edge_config_store_open(fastly_world_string_t *name,
                                              fastly_compute_at_edge_config_store_handle_t *ret,
                                              fastly_compute_at_edge_types_error_t *err) {
  return convert_result(
      fastly::config_store_open(reinterpret_cast<char *>(name->ptr), name->len, ret), err);
}

bool fastly_compute_at_edge_config_store_get(fastly_compute_at_edge_config_store_handle_t h,
                                             fastly_world_string_t *key,
                                             fastly_world_option_string_t *ret,
                                             fastly_compute_at_edge_types_error_t *err) {
  ret->val.ptr = static_cast<uint8_t *>(cabi_malloc(CONFIG_STORE_ENTRY_MAX_LEN, 1));
  if (!convert_result(fastly::config_store_get(h, reinterpret_cast<char *>(key->ptr), key->len,
                                               reinterpret_cast<char *>(ret->val.ptr),
                                               CONFIG_STORE_ENTRY_MAX_LEN, &ret->val.len),
                      err)) {
    if (*err == FASTLY_COMPUTE_AT_EDGE_TYPES_ERROR_OPTIONAL_NONE) {
      ret->is_some = false;
      return true;
    } else {
      cabi_free(ret->val.ptr);
      return false;
    }
  }
  ret->is_some = true;
  ret->val.ptr = static_cast<uint8_t *>(
      cabi_realloc(ret->val.ptr, CONFIG_STORE_ENTRY_MAX_LEN, 1, ret->val.len));
  return true;
}

bool fastly_compute_at_edge_secret_store_open(
    fastly_world_string_t *name, fastly_compute_at_edge_secret_store_store_handle_t *ret,
    fastly_compute_at_edge_types_error_t *err) {
  return convert_result(
      fastly::secret_store_open(reinterpret_cast<char *>(name->ptr), name->len, ret), err);
}

bool fastly_compute_at_edge_secret_store_get(
    fastly_compute_at_edge_secret_store_store_handle_t store, fastly_world_string_t *key,
    fastly_world_option_secret_handle_t *ret, fastly_compute_at_edge_types_error_t *err) {
  ret->val = INVALID_HANDLE;
  bool ok = convert_result(
      fastly::secret_store_get(store, reinterpret_cast<char *>(key->ptr), key->len, &ret->val),
      err);
  if ((!ok && *err == FASTLY_COMPUTE_AT_EDGE_TYPES_ERROR_OPTIONAL_NONE) ||
      ret->val == INVALID_HANDLE) {
    ret->is_some = false;
    return true;
  }
  ret->is_some = true;
  return ok;
}

bool fastly_compute_at_edge_secret_store_plaintext(
    fastly_compute_at_edge_secret_store_secret_handle_t h, fastly_world_option_list_u8_t *ret,
    fastly_compute_at_edge_secret_store_error_t *err) {
  ret->val.ptr = static_cast<uint8_t *>(JS_malloc(CONTEXT, DICTIONARY_ENTRY_MAX_LEN));
  if (!convert_result(fastly::secret_store_plaintext(h, reinterpret_cast<char *>(ret->val.ptr),
                                                     DICTIONARY_ENTRY_MAX_LEN, &ret->val.len),
                      err)) {
    if (*err == FASTLY_COMPUTE_AT_EDGE_TYPES_ERROR_OPTIONAL_NONE) {
      ret->is_some = false;
      return true;
    } else {
      JS_free(CONTEXT, ret->val.ptr);
      return false;
    }
  }
  ret->is_some = true;
  ret->val.ptr = static_cast<uint8_t *>(
      JS_realloc(CONTEXT, ret->val.ptr, DICTIONARY_ENTRY_MAX_LEN, ret->val.len));
  return true;
}

bool fastly_compute_at_edge_secret_store_from_bytes(
    fastly_world_list_u8_t *bytes, fastly_compute_at_edge_secret_store_secret_handle_t *ret,
    fastly_compute_at_edge_secret_store_error_t *err) {
  *ret = INVALID_HANDLE;
  bool ok = convert_result(
      fastly::secret_store_from_bytes(reinterpret_cast<char *>(bytes->ptr), bytes->len, ret), err);
  if (!ok || *ret == INVALID_HANDLE) {
    return false;
  }
  return true;
}

bool fastly_compute_at_edge_geo_lookup(fastly_world_list_u8_t *addr_octets,
                                       fastly_world_string_t *ret,
                                       fastly_compute_at_edge_types_error_t *err) {
  ret->ptr = static_cast<uint8_t *>(cabi_malloc(HOSTCALL_BUFFER_LEN, 1));
  if (!convert_result(fastly::geo_lookup(addr_octets->ptr, addr_octets->len,
                                         reinterpret_cast<char *>(ret->ptr), HOSTCALL_BUFFER_LEN,
                                         &ret->len),
                      err)) {
    cabi_free(ret->ptr);
    return false;
  }
  ret->ptr = static_cast<uint8_t *>(cabi_realloc(ret->ptr, HOSTCALL_BUFFER_LEN, 1, ret->len));
  return true;
}

bool fastly_compute_at_edge_object_store_open(fastly_world_string_t *name,
                                              fastly_compute_at_edge_object_store_handle_t *ret,
                                              fastly_compute_at_edge_types_error_t *err) {
  return convert_result(
      fastly::object_store_open(reinterpret_cast<char *>(name->ptr), name->len, ret), err);
}

bool fastly_compute_at_edge_object_store_lookup(fastly_compute_at_edge_object_store_handle_t store,
                                                fastly_world_string_t *key,
                                                fastly_world_option_body_handle_t *ret,
                                                fastly_compute_at_edge_types_error_t *err) {
  ret->val = INVALID_HANDLE;
  bool ok = convert_result(
      fastly::object_store_get(store, reinterpret_cast<char *>(key->ptr), key->len, &ret->val),
      err);
  if ((!ok && *err == FASTLY_COMPUTE_AT_EDGE_TYPES_ERROR_OPTIONAL_NONE) ||
      ret->val == INVALID_HANDLE) {
    ret->is_some = false;
    return true;
  }
  ret->is_some = true;
  return ok;
}

bool fastly_compute_at_edge_object_store_lookup_async(
    fastly_compute_at_edge_object_store_handle_t store, fastly_world_string_t *key,
    fastly_compute_at_edge_object_store_pending_handle_t *ret,
    fastly_compute_at_edge_object_store_error_t *err) {
  return convert_result(
      fastly::object_store_get_async(store, reinterpret_cast<char *>(key->ptr), key->len, ret),
      err);
}

bool fastly_compute_at_edge_object_store_pending_lookup_wait(
    fastly_compute_at_edge_object_store_pending_handle_t h, fastly_world_option_body_handle_t *ret,
    fastly_compute_at_edge_object_store_error_t *err) {
  ret->val = INVALID_HANDLE;
  bool ok = convert_result(fastly::object_store_pending_lookup_wait(h, &ret->val), err);
  if ((!ok && *err == FASTLY_COMPUTE_AT_EDGE_TYPES_ERROR_OPTIONAL_NONE) ||
      ret->val == INVALID_HANDLE) {
    ret->is_some = false;
    return true;
  }
  ret->is_some = true;
  return ok;
}

bool fastly_compute_at_edge_object_store_delete_async(
    fastly_compute_at_edge_object_store_handle_t store, fastly_world_string_t *key,
    fastly_compute_at_edge_object_store_pending_handle_t *ret,
    fastly_compute_at_edge_object_store_error_t *err) {
  return convert_result(
      fastly::object_store_delete_async(store, reinterpret_cast<char *>(key->ptr), key->len, ret),
      err);
}

bool fastly_compute_at_edge_object_store_pending_delete_wait(
    fastly_compute_at_edge_object_store_pending_handle_t h,
    fastly_compute_at_edge_object_store_error_t *err) {
  return convert_result(fastly::object_store_pending_delete_wait(h), err);
}

bool fastly_compute_at_edge_object_store_insert(
    fastly_compute_at_edge_object_store_handle_t store, fastly_world_string_t *key,
    fastly_compute_at_edge_http_types_body_handle_t body_handle,
    fastly_compute_at_edge_types_error_t *err) {
  return convert_result(
      fastly::object_store_insert(store, reinterpret_cast<char *>(key->ptr), key->len, body_handle),
      err);
}

bool fastly_compute_at_edge_async_io_select(fastly_world_list_handle_t *hs, uint32_t timeout_ms,
                                            fastly_world_option_u32_t *ret,
                                            fastly_compute_at_edge_types_error_t *err) {
  if (!convert_result(fastly::async_select(hs->ptr, hs->len, timeout_ms, &ret->val), err)) {
    if (*err == FASTLY_COMPUTE_AT_EDGE_TYPES_ERROR_OPTIONAL_NONE) {
      ret->is_some = false;
      return true;
    }
    return false;
  }

  // The result is only valid if the timeout didn't expire.
  ret->is_some = ret->val != UINT32_MAX;

  return true;
}
bool fastly_compute_at_edge_async_io_is_ready(fastly_compute_at_edge_async_io_handle_t handle,
                                              bool *ret,
                                              fastly_compute_at_edge_types_error_t *err) {
  uint32_t ret_int;
  if (!convert_result(fastly::async_is_ready(handle, &ret_int), err)) {
    return false;
  }
  *ret = (bool)ret_int;
  return true;
}

bool fastly_compute_at_edge_purge_surrogate_key(
    fastly_world_string_t *surrogate_key, fastly_compute_at_edge_purge_options_mask_t options_mask,
    fastly_world_option_string_t *ret, fastly_compute_at_edge_types_error_t *err) {
  fastly::PurgeOptions options{nullptr, 0, nullptr};

  // Currently this host-call has been implemented to support the `SimpleCache.delete(key)` method,
  // which uses hard-purging and not soft-purging.
  // TODO: Create a JS API for this hostcall which supports hard-purging and another which supports
  // soft-purging. E.G. `fastly.purgeSurrogateKey(key)` and `fastly.softPurgeSurrogateKey(key)`
  MOZ_ASSERT(!(options_mask & FASTLY_COMPUTE_AT_EDGE_PURGE_OPTIONS_MASK_SOFT_PURGE));
  MOZ_ASSERT(!(options_mask & FASTLY_COMPUTE_AT_EDGE_PURGE_OPTIONS_MASK_RET_BUF));

  ret->is_some = false;

  return convert_result(fastly::purge_surrogate_key(reinterpret_cast<char *>(surrogate_key->ptr),
                                                    surrogate_key->len, options_mask, &options),
                        err);
}

#define FASTLY_CACHE_LOOKUP_OPTIONS_MASK_RESERVED (1 << 0)
#define FASTLY_CACHE_LOOKUP_OPTIONS_MASK_REQUEST_HEADERS (1 << 1)

bool fastly_compute_at_edge_cache_lookup(fastly_world_string_t *cache_key,
                                         fastly_compute_at_edge_cache_lookup_options_t *options,
                                         fastly_compute_at_edge_cache_handle_t *ret,
                                         fastly_compute_at_edge_types_error_t *err) {
  uint8_t options_mask = 0;
  if (options->request_headers.is_some) {
    options_mask |= FASTLY_CACHE_LOOKUP_OPTIONS_MASK_REQUEST_HEADERS;
  }
  return convert_result(fastly::cache_lookup(reinterpret_cast<char *>(cache_key->ptr),
                                             cache_key->len, options_mask, options, ret),
                        err);
}

#define FASTLY_CACHE_WRITE_OPTIONS_MASK_RESERVED (1 << 0)
#define FASTLY_CACHE_WRITE_OPTIONS_MASK_REQUEST_HEADERS (1 << 1)
#define FASTLY_CACHE_WRITE_OPTIONS_MASK_VARY_RULE (1 << 2)
#define FASTLY_CACHE_WRITE_OPTIONS_MASK_INITIAL_AGE_NS (1 << 3)
#define FASTLY_CACHE_WRITE_OPTIONS_MASK_STALE_WHILE_REVALIDATE_NS (1 << 4)
#define FASTLY_CACHE_WRITE_OPTIONS_MASK_SURROGATE_KEYS (1 << 5)
#define FASTLY_CACHE_WRITE_OPTIONS_MASK_LENGTH (1 << 6)
#define FASTLY_CACHE_WRITE_OPTIONS_MASK_USER_METADATA (1 << 7)
#define FASTLY_CACHE_WRITE_OPTIONS_MASK_SENSITIVE_DATA (1 << 8)

bool fastly_compute_at_edge_cache_insert(fastly_world_string_t *cache_key,
                                         fastly_compute_at_edge_cache_write_options_t *options,
                                         fastly_compute_at_edge_http_types_body_handle_t *ret,
                                         fastly_compute_at_edge_types_error_t *err) {
  uint16_t options_mask = 0;
  fastly::CacheWriteOptions opts;
  std::memset(&opts, 0, sizeof(opts));
  opts.max_age_ns = options->max_age_ns;

  if (options->request_headers != INVALID_HANDLE && options->request_headers != 0) {
    options_mask |= FASTLY_CACHE_WRITE_OPTIONS_MASK_REQUEST_HEADERS;
    opts.request_headers = options->request_headers;
  }
  if (options->vary_rule.ptr != nullptr) {
    options_mask |= FASTLY_CACHE_WRITE_OPTIONS_MASK_VARY_RULE;
    opts.vary_rule_len = options->vary_rule.len;
    opts.vary_rule_ptr = reinterpret_cast<uint8_t *>(options->vary_rule.ptr);
  }
  if (options->initial_age_ns != 0) {
    options_mask |= FASTLY_CACHE_WRITE_OPTIONS_MASK_INITIAL_AGE_NS;
    opts.initial_age_ns = options->initial_age_ns;
  }
  if (options->stale_while_revalidate_ns != 0) {
    options_mask |= FASTLY_CACHE_WRITE_OPTIONS_MASK_STALE_WHILE_REVALIDATE_NS;
    opts.stale_while_revalidate_ns = options->stale_while_revalidate_ns;
  }
  if (options->surrogate_keys.ptr != nullptr) {
    options_mask |= FASTLY_CACHE_WRITE_OPTIONS_MASK_SURROGATE_KEYS;
    opts.surrogate_keys_len = options->surrogate_keys.len;
    opts.surrogate_keys_ptr = reinterpret_cast<uint8_t *>(options->surrogate_keys.ptr);
  }
  if (options->length != 0) {
    options_mask |= FASTLY_CACHE_WRITE_OPTIONS_MASK_LENGTH;
    opts.length = options->length;
  }
  if (options->user_metadata.ptr != nullptr) {
    options_mask |= FASTLY_CACHE_WRITE_OPTIONS_MASK_USER_METADATA;
    opts.user_metadata_len = options->user_metadata.len;
    opts.user_metadata_ptr = options->user_metadata.ptr;
  }
  if (options->sensitive_data) {
    options_mask |= FASTLY_CACHE_WRITE_OPTIONS_MASK_SENSITIVE_DATA;
  }
  return convert_result(fastly::cache_insert(reinterpret_cast<char *>(cache_key->ptr),
                                             cache_key->len, options_mask, &opts, ret),
                        err);
}

bool fastly_compute_at_edge_cache_transaction_insert(
    fastly_compute_at_edge_cache_handle_t handle,
    fastly_compute_at_edge_cache_write_options_t *options,
    fastly_compute_at_edge_cache_body_handle_t *ret, fastly_compute_at_edge_cache_error_t *err) {
  uint16_t options_mask = 0;
  fastly::CacheWriteOptions opts;
  std::memset(&opts, 0, sizeof(opts));
  opts.max_age_ns = options->max_age_ns;

  if (options->request_headers != INVALID_HANDLE && options->request_headers != 0) {
    options_mask |= FASTLY_CACHE_WRITE_OPTIONS_MASK_REQUEST_HEADERS;
    opts.request_headers = options->request_headers;
  }
  if (options->vary_rule.len > 0) {
    options_mask |= FASTLY_CACHE_WRITE_OPTIONS_MASK_VARY_RULE;
    opts.vary_rule_len = options->vary_rule.len;
    opts.vary_rule_ptr = reinterpret_cast<uint8_t *>(options->vary_rule.ptr);
  }
  if (options->initial_age_ns != 0) {
    options_mask |= FASTLY_CACHE_WRITE_OPTIONS_MASK_INITIAL_AGE_NS;
    opts.initial_age_ns = options->initial_age_ns;
  }
  if (options->stale_while_revalidate_ns != 0) {
    options_mask |= FASTLY_CACHE_WRITE_OPTIONS_MASK_STALE_WHILE_REVALIDATE_NS;
    opts.stale_while_revalidate_ns = options->stale_while_revalidate_ns;
  }
  if (options->surrogate_keys.ptr != nullptr) {
    options_mask |= FASTLY_CACHE_WRITE_OPTIONS_MASK_SURROGATE_KEYS;
    opts.surrogate_keys_len = options->surrogate_keys.len;
    opts.surrogate_keys_ptr = reinterpret_cast<uint8_t *>(options->surrogate_keys.ptr);
  }
  if (options->length != 0) {
    options_mask |= FASTLY_CACHE_WRITE_OPTIONS_MASK_LENGTH;
    opts.length = options->length;
  }
  if (options->user_metadata.ptr != nullptr) {
    options_mask |= FASTLY_CACHE_WRITE_OPTIONS_MASK_USER_METADATA;
    opts.user_metadata_len = options->user_metadata.len;
    opts.user_metadata_ptr = options->user_metadata.ptr;
  }
  if (options->sensitive_data) {
    options_mask |= FASTLY_CACHE_WRITE_OPTIONS_MASK_SENSITIVE_DATA;
  }
  return convert_result(fastly::cache_transaction_insert(handle, options_mask, &opts, ret), err);
}

bool fastly_compute_at_edge_cache_transaction_update(
    fastly_compute_at_edge_cache_handle_t handle,
    fastly_compute_at_edge_cache_write_options_t *options,
    fastly_compute_at_edge_cache_error_t *err) {
  uint16_t options_mask = 0;
  fastly::CacheWriteOptions opts;
  std::memset(&opts, 0, sizeof(opts));
  opts.max_age_ns = options->max_age_ns;

  if (options->request_headers != INVALID_HANDLE && options->request_headers != 0) {
    options_mask |= FASTLY_CACHE_WRITE_OPTIONS_MASK_REQUEST_HEADERS;
    opts.request_headers = options->request_headers;
  }
  if (options->vary_rule.ptr != nullptr) {
    options_mask |= FASTLY_CACHE_WRITE_OPTIONS_MASK_VARY_RULE;
    opts.vary_rule_len = options->vary_rule.len;
    opts.vary_rule_ptr = reinterpret_cast<uint8_t *>(options->vary_rule.ptr);
  }
  if (options->initial_age_ns != 0) {
    options_mask |= FASTLY_CACHE_WRITE_OPTIONS_MASK_INITIAL_AGE_NS;
    opts.initial_age_ns = options->initial_age_ns;
  }
  if (options->stale_while_revalidate_ns != 0) {
    options_mask |= FASTLY_CACHE_WRITE_OPTIONS_MASK_STALE_WHILE_REVALIDATE_NS;
    opts.stale_while_revalidate_ns = options->stale_while_revalidate_ns;
  }
  if (options->surrogate_keys.ptr != nullptr) {
    options_mask |= FASTLY_CACHE_WRITE_OPTIONS_MASK_SURROGATE_KEYS;
    opts.surrogate_keys_len = options->surrogate_keys.len;
    opts.surrogate_keys_ptr = reinterpret_cast<uint8_t *>(options->surrogate_keys.ptr);
  }
  if (options->length != 0) {
    options_mask |= FASTLY_CACHE_WRITE_OPTIONS_MASK_LENGTH;
    opts.length = options->length;
  }
  if (options->user_metadata.ptr != nullptr) {
    options_mask |= FASTLY_CACHE_WRITE_OPTIONS_MASK_USER_METADATA;
    opts.user_metadata_len = options->user_metadata.len;
    opts.user_metadata_ptr = options->user_metadata.ptr;
  }
  if (options->sensitive_data) {
    options_mask |= FASTLY_CACHE_WRITE_OPTIONS_MASK_SENSITIVE_DATA;
  }
  return convert_result(fastly::cache_transaction_update(handle, options_mask, &opts), err);
}

bool fastly_compute_at_edge_cache_get_body(
    fastly_compute_at_edge_cache_handle_t handle,
    fastly_compute_at_edge_cache_get_body_options_t *options,
    fastly_compute_at_edge_cache_get_body_options_mask_t options_mask,
    fastly_compute_at_edge_cache_body_handle_t *ret, fastly_compute_at_edge_cache_error_t *err) {
  bool ok = convert_result(fastly::cache_get_body(handle, options_mask, options, ret), err);
  if (!ok && *err == FASTLY_COMPUTE_AT_EDGE_TYPES_ERROR_OPTIONAL_NONE) {
    *ret = INVALID_HANDLE;
    return true;
  }
  return ok;
}
bool fastly_compute_at_edge_cache_transaction_lookup(
    fastly_world_string_t *cache_key, fastly_compute_at_edge_cache_lookup_options_t *options,
    fastly_compute_at_edge_cache_handle_t *ret, fastly_compute_at_edge_types_error_t *err) {
  uint32_t options_mask = 0;
  if (options->request_headers.is_some) {
    options_mask |= FASTLY_CACHE_LOOKUP_OPTIONS_MASK_REQUEST_HEADERS;
  }
  return convert_result(fastly::cache_transaction_lookup(reinterpret_cast<char *>(cache_key->ptr),
                                                         cache_key->len, options_mask, options,
                                                         ret),
                        err);
}
bool fastly_compute_at_edge_cache_transaction_insert_and_stream_back(
    fastly_compute_at_edge_cache_handle_t handle,
    fastly_compute_at_edge_cache_write_options_t *options,
    fastly_world_tuple2_body_handle_handle_t *ret, fastly_compute_at_edge_types_error_t *err) {
  uint16_t options_mask = 0;
  fastly::CacheWriteOptions opts;
  std::memset(&opts, 0, sizeof(opts));
  opts.max_age_ns = options->max_age_ns;

  MOZ_ASSERT(options->request_headers == INVALID_HANDLE || options->request_headers == 0);

  if (options->vary_rule.ptr != nullptr) {
    options_mask |= FASTLY_CACHE_WRITE_OPTIONS_MASK_VARY_RULE;
    opts.vary_rule_len = options->vary_rule.len;
    opts.vary_rule_ptr = reinterpret_cast<uint8_t *>(options->vary_rule.ptr);
  }
  if (options->initial_age_ns != 0) {
    options_mask |= FASTLY_CACHE_WRITE_OPTIONS_MASK_INITIAL_AGE_NS;
    opts.initial_age_ns = options->initial_age_ns;
  }
  if (options->stale_while_revalidate_ns != 0) {
    options_mask |= FASTLY_CACHE_WRITE_OPTIONS_MASK_STALE_WHILE_REVALIDATE_NS;
    opts.stale_while_revalidate_ns = options->stale_while_revalidate_ns;
  }
  if (options->surrogate_keys.ptr != nullptr) {
    options_mask |= FASTLY_CACHE_WRITE_OPTIONS_MASK_SURROGATE_KEYS;
    opts.surrogate_keys_len = options->surrogate_keys.len;
    opts.surrogate_keys_ptr = reinterpret_cast<uint8_t *>(options->surrogate_keys.ptr);
  }
  if (options->length != 0) {
    options_mask |= FASTLY_CACHE_WRITE_OPTIONS_MASK_LENGTH;
    opts.length = options->length;
  }
  if (options->user_metadata.ptr != nullptr) {
    options_mask |= FASTLY_CACHE_WRITE_OPTIONS_MASK_USER_METADATA;
    opts.user_metadata_len = options->user_metadata.len;
    opts.user_metadata_ptr = options->user_metadata.ptr;
  }
  if (options->sensitive_data) {
    options_mask |= FASTLY_CACHE_WRITE_OPTIONS_MASK_SENSITIVE_DATA;
  }
  return convert_result(fastly::cache_transaction_insert_and_stream_back(handle, options_mask,
                                                                         &opts, &ret->f0, &ret->f1),
                        err);
}

/// Cancel an obligation to provide an object to the cache.
///
/// Useful if there is an error before streaming is possible, e.g. if a backend is unreachable.
bool fastly_compute_at_edge_cache_transaction_cancel(fastly_compute_at_edge_cache_handle_t handle,
                                                     fastly_compute_at_edge_types_error_t *err) {
  return convert_result(fastly::cache_transaction_cancel(handle), err);
}

bool fastly_compute_at_edge_cache_get_state(fastly_compute_at_edge_cache_handle_t handle,
                                            fastly_compute_at_edge_cache_lookup_state_t *ret,
                                            fastly_compute_at_edge_types_error_t *err) {
  return convert_result(fastly::cache_get_state(handle, ret), err);
}

/// Cancel an obligation to provide an object to the cache.
///
/// Useful if there is an error before streaming is possible, e.g. if a backend is unreachable.
bool fastly_compute_at_edge_transaction_cancel(fastly_compute_at_edge_cache_handle_t handle,
                                               fastly_compute_at_edge_types_error_t *err) {
  return convert_result(fastly::cache_transaction_cancel(handle), err);
}

bool fastly_compute_at_edge_cache_close(fastly_compute_at_edge_cache_handle_t handle,
                                        fastly_compute_at_edge_types_error_t *err) {
  return true;
}
bool fastly_compute_at_edge_cache_get_user_metadata(fastly_compute_at_edge_cache_handle_t handle,
                                                    fastly_world_list_u8_t *ret,
                                                    fastly_compute_at_edge_cache_error_t *err) {
  size_t default_size = 16 * 1024;
  ret->ptr = static_cast<uint8_t *>(cabi_malloc(default_size, 4));
  auto status = fastly::cache_get_user_metadata(handle, reinterpret_cast<char *>(ret->ptr),
                                                default_size, &ret->len);
  if (status == FASTLY_COMPUTE_AT_EDGE_TYPES_ERROR_BUFFER_LEN) {
    ret->ptr = static_cast<uint8_t *>(cabi_realloc(ret->ptr, default_size, 4, ret->len));
    status = fastly::cache_get_user_metadata(handle, reinterpret_cast<char *>(ret->ptr), ret->len,
                                             &ret->len);
  }
  return convert_result(status, err);
}
bool fastly_compute_at_edge_cache_get_length(fastly_compute_at_edge_cache_handle_t handle,
                                             uint64_t *ret,
                                             fastly_compute_at_edge_types_error_t *err) {
  return convert_result(fastly::cache_get_length(handle, ret), err);
}
bool fastly_compute_at_edge_cache_get_max_age_ns(fastly_compute_at_edge_cache_handle_t handle,
                                                 uint64_t *ret,
                                                 fastly_compute_at_edge_types_error_t *err) {
  return convert_result(fastly::cache_get_max_age_ns(handle, ret), err);
}
bool fastly_compute_at_edge_cache_get_stale_while_revalidate_ns(
    fastly_compute_at_edge_cache_handle_t handle, uint64_t *ret,
    fastly_compute_at_edge_types_error_t *err) {
  return convert_result(fastly::cache_get_stale_while_revalidate_ns(handle, ret), err);
}
bool fastly_compute_at_edge_cache_get_age_ns(fastly_compute_at_edge_cache_handle_t handle,
                                             uint64_t *ret,
                                             fastly_compute_at_edge_types_error_t *err) {
  return convert_result(fastly::cache_get_age_ns(handle, ret), err);
}
bool fastly_compute_at_edge_cache_get_hits(fastly_compute_at_edge_cache_handle_t handle,
                                           uint64_t *ret,
                                           fastly_compute_at_edge_types_error_t *err) {
  return convert_result(fastly::cache_get_hits(handle, ret), err);
}

/*
 * Fastly Backend
 */
bool fastly_compute_at_edge_backend_exists(fastly_world_string_t *backend, bool *ret,
                                           fastly_compute_at_edge_types_error_t *err) {
  uint32_t ret_int;
  if (!convert_result(
          fastly::backend_exists(reinterpret_cast<char *>(backend->ptr), backend->len, &ret_int),
          err)) {
    return false;
  }
  *ret = (bool)ret_int;
  return true;
}

fastly_compute_at_edge_backend_backend_health_t
convert_fastly_backend_health(fastly::BACKEND_HEALTH version) {
  switch (version) {
  case fastly::BACKEND_HEALTH::UNKNOWN:
    return FASTLY_COMPUTE_AT_EDGE_BACKEND_BACKEND_HEALTH_UNKNOWN;
  case fastly::BACKEND_HEALTH::HEALTHY:
    return FASTLY_COMPUTE_AT_EDGE_BACKEND_BACKEND_HEALTH_HEALTHY;
  case fastly::BACKEND_HEALTH::UNHEALTHY:
    return FASTLY_COMPUTE_AT_EDGE_BACKEND_BACKEND_HEALTH_UNHEALTHY;
  default:
    return FASTLY_COMPUTE_AT_EDGE_BACKEND_BACKEND_HEALTH_UNKNOWN;
  }
}

bool fastly_compute_at_edge_backend_is_healthy(fastly_world_string_t *backend,
                                               fastly_compute_at_edge_backend_backend_health_t *ret,
                                               fastly_compute_at_edge_types_error_t *err) {
  fastly::BACKEND_HEALTH fastly_backend_health;
  if (!convert_result(
          fastly::backend_is_healthy(reinterpret_cast<char *>(backend->ptr), backend->len,
                                     reinterpret_cast<uint32_t *>(&fastly_backend_health)),
          err)) {
    return false;
  }
  *ret = convert_fastly_backend_health(fastly_backend_health);
  return true;
}

bool fastly_compute_at_edge_edge_rate_limiter_check_rate(
    fastly_world_string_t *rate_counter_name, fastly_world_string_t *entry, uint32_t delta,
    uint32_t window, uint32_t limit, fastly_world_string_t *penalty_box_name, uint32_t time_to_live,
    bool *ret, fastly_compute_at_edge_edge_rate_limiter_error_t *err) {
  return convert_result(
      fastly::check_rate(reinterpret_cast<char *>(rate_counter_name->ptr), rate_counter_name->len,
                         reinterpret_cast<char *>(entry->ptr), entry->len, delta, window, limit,
                         reinterpret_cast<char *>(penalty_box_name->ptr), penalty_box_name->len,
                         time_to_live, ret),
      err);
}

bool fastly_compute_at_edge_edge_rate_limiter_ratecounter_increment(
    fastly_world_string_t *rate_counter_name, fastly_world_string_t *entry, uint32_t delta,
    fastly_compute_at_edge_edge_rate_limiter_error_t *err) {
  return convert_result(
      fastly::ratecounter_increment(reinterpret_cast<char *>(rate_counter_name->ptr),
                                    rate_counter_name->len, reinterpret_cast<char *>(entry->ptr),
                                    entry->len, delta),
      err);
}

bool fastly_compute_at_edge_edge_rate_limiter_ratecounter_lookup_rate(
    fastly_world_string_t *rate_counter_name, fastly_world_string_t *entry, uint32_t window,
    uint32_t *ret, fastly_compute_at_edge_edge_rate_limiter_error_t *err) {
  return convert_result(
      fastly::ratecounter_lookup_rate(reinterpret_cast<char *>(rate_counter_name->ptr),
                                      rate_counter_name->len, reinterpret_cast<char *>(entry->ptr),
                                      entry->len, window, ret),
      err);
}

bool fastly_compute_at_edge_edge_rate_limiter_ratecounter_lookup_count(
    fastly_world_string_t *rate_counter_name, fastly_world_string_t *entry, uint32_t duration,
    uint32_t *ret, fastly_compute_at_edge_edge_rate_limiter_error_t *err) {
  return convert_result(
      fastly::ratecounter_lookup_count(reinterpret_cast<char *>(rate_counter_name->ptr),
                                       rate_counter_name->len, reinterpret_cast<char *>(entry->ptr),
                                       entry->len, duration, ret),
      err);
}

bool fastly_compute_at_edge_edge_rate_limiter_penaltybox_add(
    fastly_world_string_t *penalty_box_name, fastly_world_string_t *entry, uint32_t time_to_live,
    fastly_compute_at_edge_edge_rate_limiter_error_t *err) {
  return convert_result(
      fastly::penaltybox_add(reinterpret_cast<char *>(penalty_box_name->ptr), penalty_box_name->len,
                             reinterpret_cast<char *>(entry->ptr), entry->len, time_to_live),
      err);
}

bool fastly_compute_at_edge_edge_rate_limiter_penaltybox_has(
    fastly_world_string_t *penalty_box_name, fastly_world_string_t *entry, bool *ret,
    fastly_compute_at_edge_edge_rate_limiter_error_t *err) {
  return convert_result(
      fastly::penaltybox_has(reinterpret_cast<char *>(penalty_box_name->ptr), penalty_box_name->len,
                             reinterpret_cast<char *>(entry->ptr), entry->len, ret),
      err);
}

bool fastly_compute_at_edge_device_detection_lookup(
    fastly_world_string_t *user_agent, fastly_world_string_t *ret,
    fastly_compute_at_edge_device_detection_error_t *err) {
  auto default_size = 1024;
  ret->ptr = static_cast<uint8_t *>(cabi_malloc(default_size, 4));
  auto status =
      fastly::device_detection_lookup(reinterpret_cast<char *>(user_agent->ptr), user_agent->len,
                                      reinterpret_cast<char *>(ret->ptr), default_size, &ret->len);
  if (status == FASTLY_COMPUTE_AT_EDGE_TYPES_ERROR_BUFFER_LEN) {
    ret->ptr = static_cast<uint8_t *>(cabi_realloc(ret->ptr, default_size, 4, ret->len));
    status =
        fastly::device_detection_lookup(reinterpret_cast<char *>(user_agent->ptr), user_agent->len,
                                        reinterpret_cast<char *>(ret->ptr), ret->len, &ret->len);
  }
  return convert_result(status, err);
}
