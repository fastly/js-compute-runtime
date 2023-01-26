#include "xqd_world_adapter.h"

#ifndef COMPONENT

// Ensure that all the things we want to use the hostcall buffer for actually
// fit into the buffer.
#define HOSTCALL_BUFFER_LEN HEADER_MAX_LEN
static_assert(DICTIONARY_ENTRY_MAX_LEN < HOSTCALL_BUFFER_LEN);
static_assert(METHOD_MAX_LEN < HOSTCALL_BUFFER_LEN);
static_assert(URI_MAX_LEN < HOSTCALL_BUFFER_LEN);

#define LIST_ALLOC_SIZE 50

static bool convert_result(int res, fastly_error_t *err) {
  if (res == 0)
    return true;
  switch (res) {
  case 1:
    *err = FASTLY_ERROR_GENERIC_ERROR;
    break;
  case 2:
    *err = FASTLY_ERROR_INVALID_ARGUMENT;
    break;
  case 3:
    *err = FASTLY_ERROR_BAD_HANDLE;
    break;
  case 4:
    *err = FASTLY_ERROR_BUFFER_LEN;
    break;
  case 5:
    *err = FASTLY_ERROR_UNSUPPORTED;
    break;
  case 6:
    *err = FASTLY_ERROR_BAD_ALIGN;
    break;
  case 7:
    *err = FASTLY_ERROR_HTTP_INVALID;
    break;
  case 8:
    *err = FASTLY_ERROR_HTTP_USER;
    break;
  case 9:
    *err = FASTLY_ERROR_HTTP_INCOMPLETE;
    break;
  case 10:
    *err = FASTLY_ERROR_OPTIONAL_NONE;
    break;
  case 11:
    *err = FASTLY_ERROR_HTTP_HEAD_TOO_LARGE;
    break;
  case 12:
    *err = FASTLY_ERROR_HTTP_INVALID_STATUS;
    break;
  case 13:
    *err = FASTLY_ERROR_LIMIT_EXCEEDED;
    break;
  case 100:
    *err = FASTLY_ERROR_UNKNOWN_ERROR;
    break;
  default:
    *err = FASTLY_ERROR_UNKNOWN_ERROR;
  }
  return false;
}

fastly_http_version_t convert_http_version(uint32_t version) {
  switch (version) {
  case 0:
    return FASTLY_HTTP_VERSION_HTTP09;
  case 1:
    return FASTLY_HTTP_VERSION_HTTP10;
  case 2:
    return FASTLY_HTTP_VERSION_HTTP11;
  case 3:
    return FASTLY_HTTP_VERSION_H2;
  case 4:
  default:
    return FASTLY_HTTP_VERSION_H3;
  }
}

bool xqd_fastly_http_body_new(fastly_body_handle_t *ret, fastly_error_t *err) {
  return convert_result(xqd_body_new(ret), err);
}

bool xqd_fastly_http_body_append(fastly_body_handle_t src, fastly_body_handle_t dest,
                                 fastly_error_t *err) {
  return convert_result(xqd_body_append(src, dest), err);
}

bool xqd_fastly_http_body_read(fastly_body_handle_t h, uint32_t chunk_size, fastly_list_u8_t *ret,
                               fastly_error_t *err) {
  ret->ptr = static_cast<uint8_t *>(cabi_malloc(chunk_size, 1));
  return convert_result(xqd_body_read(h, reinterpret_cast<char *>(ret->ptr),
                                      static_cast<size_t>(chunk_size), &ret->len),
                        err);
}

bool xqd_fastly_http_body_write(fastly_body_handle_t h, fastly_list_u8_t *buf,
                                fastly_body_write_end_t end, uint32_t *ret, fastly_error_t *err) {
  return convert_result(xqd_body_write(h, reinterpret_cast<char *>(buf->ptr), buf->len,
                                       end == FASTLY_BODY_WRITE_END_BACK
                                           ? BodyWriteEnd::BodyWriteEndBack
                                           : BodyWriteEnd::BodyWriteEndFront,
                                       reinterpret_cast<size_t *>(ret)),
                        err);
}

bool xqd_fastly_http_body_close(fastly_body_handle_t h, fastly_error_t *err) {
  return convert_result(xqd_body_close(h), err);
}

bool xqd_fastly_log_endpoint_get(xqd_world_string_t *name, fastly_log_endpoint_handle_t *ret,
                                 fastly_error_t *err) {
  return convert_result(
      xqd_log_endpoint_get(reinterpret_cast<const char *>(name->ptr), name->len, ret), err);
}

bool xqd_fastly_log_write(fastly_log_endpoint_handle_t h, xqd_world_string_t *msg,
                          fastly_error_t *err) {
  size_t nwritten = 0;
  return convert_result(
      xqd_log_write(h, reinterpret_cast<const char *>(msg->ptr), msg->len, &nwritten), err);
}

bool xqd_fastly_http_req_body_downstream_get(fastly_request_t *ret, fastly_error_t *err) {
  return convert_result(xqd_req_body_downstream_get(&ret->f0, &ret->f1), err);
}

bool xqd_fastly_http_req_cache_override_set(fastly_request_handle_t h,
                                            fastly_http_cache_override_tag_t tag,
                                            uint32_t *maybe_ttl,
                                            uint32_t *maybe_stale_while_revalidate,
                                            xqd_world_string_t *maybe_sk, fastly_error_t *err) {
  xqd_world_string_t sk_str;
  if (maybe_sk) {
    sk_str = *maybe_sk;
  } else {
    sk_str.len = 0;
    sk_str.ptr = NULL;
  }
  return convert_result(
      xqd_req_cache_override_v2_set(
          h, (int)(tag), maybe_ttl == NULL ? 0 : *maybe_ttl,
          maybe_stale_while_revalidate == NULL ? 0 : *maybe_stale_while_revalidate,
          reinterpret_cast<char *>(sk_str.ptr), sk_str.len),
      err);
}

bool xqd_fastly_http_req_downstream_client_ip_addr(fastly_list_u8_t *ret, fastly_error_t *err) {
  ret->ptr = static_cast<uint8_t *>(cabi_malloc(16, 1));
  return convert_result(
      xqd_req_downstream_client_ip_addr_get(reinterpret_cast<char *>(ret->ptr), &ret->len), err);
}

bool xqd_fastly_http_req_new(fastly_request_handle_t *ret, fastly_error_t *err) {
  return convert_result(xqd_req_new(ret), err);
}

bool xqd_fastly_http_req_header_names_get(fastly_request_handle_t h, fastly_list_string_t *ret,
                                          fastly_error_t *err) {
  size_t str_max = LIST_ALLOC_SIZE;

  xqd_world_string_t *strs =
      static_cast<xqd_world_string_t *>(cabi_malloc(str_max * sizeof(xqd_world_string_t), 1));
  size_t str_cnt = 0;
  size_t nwritten;
  char *buf = static_cast<char *>(cabi_malloc(HOSTCALL_BUFFER_LEN, 1));
  uint32_t cursor = 0;
  int64_t next_cursor = 0;
  while (true) {
    if (!convert_result(
            xqd_req_header_names_get(h, buf, HEADER_MAX_LEN, cursor, &next_cursor, &nwritten),
            err)) {
      cabi_free(strs);
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
        strs = static_cast<xqd_world_string_t *>(
            cabi_realloc(strs, str_max * sizeof(xqd_world_string_t), 1,
                         (str_max + LIST_ALLOC_SIZE) * sizeof(xqd_world_string_t)));
        str_max += LIST_ALLOC_SIZE;
      }
      strs[str_cnt].ptr = static_cast<char *>(cabi_malloc(i - offset + 1, 1));
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
  strs = static_cast<xqd_world_string_t *>(cabi_realloc(strs, str_max * sizeof(xqd_world_string_t),
                                                        1, str_cnt * sizeof(xqd_world_string_t)));
  ret->ptr = strs;
  ret->len = str_cnt;
  return true;
}

bool xqd_fastly_http_req_header_values_get(fastly_request_handle_t h, xqd_world_string_t *name,
                                           fastly_option_list_string_t *ret, fastly_error_t *err) {
  size_t str_max = LIST_ALLOC_SIZE;
  xqd_world_string_t *strs =
      static_cast<xqd_world_string_t *>(cabi_malloc(str_max * sizeof(xqd_world_string_t), 1));
  size_t str_cnt = 0;
  size_t nwritten;
  char *buf = static_cast<char *>(cabi_malloc(HOSTCALL_BUFFER_LEN, 1));
  uint32_t cursor = 0;
  int64_t next_cursor = 0;
  while (true) {
    if (!convert_result(xqd_req_header_values_get(h, name->ptr, name->len, buf, HEADER_MAX_LEN,
                                                  cursor, &next_cursor, &nwritten),
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
        strs = static_cast<xqd_world_string_t *>(
            cabi_realloc(strs, str_max * sizeof(xqd_world_string_t), 1,
                         (str_max + LIST_ALLOC_SIZE) * sizeof(xqd_world_string_t)));
        str_max += LIST_ALLOC_SIZE;
      }
      strs[str_cnt].ptr = static_cast<char *>(cabi_malloc(i - offset + 1, 1));
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
  if (str_cnt > 0) {
    ret->is_some = true;
    ret->val.ptr = strs;
    ret->val.len = str_cnt;
    strs = static_cast<xqd_world_string_t *>(cabi_realloc(
        strs, str_max * sizeof(xqd_world_string_t), 1, str_cnt * sizeof(xqd_world_string_t)));
  } else {
    ret->is_some = false;
    cabi_free(strs);
  }
  return true;
}

bool xqd_fastly_http_req_header_insert(fastly_request_handle_t h, xqd_world_string_t *name,
                                       xqd_world_string_t *value, fastly_error_t *err) {
  return convert_result(xqd_req_header_insert(h, name->ptr, name->len, value->ptr, value->len),
                        err);
}

bool xqd_fastly_http_req_header_append(fastly_request_handle_t h, xqd_world_string_t *name,
                                       xqd_world_string_t *value, fastly_error_t *err) {
  return convert_result(xqd_req_header_append(h, name->ptr, name->len, value->ptr, value->len),
                        err);
}

bool xqd_fastly_http_req_header_remove(fastly_request_handle_t h, xqd_world_string_t *name,
                                       fastly_error_t *err) {
  return convert_result(xqd_req_header_remove(h, name->ptr, name->len), err);
}

bool xqd_fastly_http_req_method_get(fastly_request_handle_t h, xqd_world_string_t *ret,
                                    fastly_error_t *err) {
  ret->ptr = static_cast<char *>(cabi_malloc(METHOD_MAX_LEN, 1));
  return convert_result(
      xqd_req_method_get(h, reinterpret_cast<char *>(ret->ptr), METHOD_MAX_LEN, &ret->len), err);
}

bool xqd_fastly_http_req_method_set(fastly_request_handle_t h, xqd_world_string_t *method,
                                    fastly_error_t *err) {
  return convert_result(
      xqd_req_method_set(h, reinterpret_cast<const char *>(method->ptr), method->len), err);
}

bool xqd_fastly_http_req_uri_get(fastly_request_handle_t h, xqd_world_string_t *ret,
                                 fastly_error_t *err) {
  ret->ptr = static_cast<char *>(cabi_malloc(URI_MAX_LEN, 1));
  if (!convert_result(xqd_req_uri_get(h, ret->ptr, URI_MAX_LEN, &ret->len), err)) {
    cabi_free(ret->ptr);
    return false;
  }
  ret->ptr = static_cast<char *>(cabi_realloc(ret->ptr, URI_MAX_LEN, 1, ret->len));
  return true;
}

bool xqd_fastly_http_req_uri_set(fastly_request_handle_t h, xqd_world_string_t *uri,
                                 fastly_error_t *err) {
  return convert_result(xqd_req_uri_set(h, uri->ptr, uri->len), err);
}

bool xqd_fastly_http_req_version_get(fastly_request_handle_t h, fastly_http_version_t *ret,
                                     fastly_error_t *err) {
  uint32_t xqd_http_version;
  if (!convert_result(xqd_req_version_get(h, &xqd_http_version), err)) {
    return false;
  }
  *ret = convert_http_version(xqd_http_version);
  return true;
}

bool xqd_fastly_http_req_send_async(fastly_request_handle_t h, fastly_body_handle_t b,
                                    xqd_world_string_t *backend,
                                    fastly_pending_request_handle_t *ret, fastly_error_t *err) {
  return convert_result(xqd_req_send_async(h, b, backend->ptr, backend->len, ret), err);
}

bool xqd_fastly_http_req_send_async_streaming(fastly_request_handle_t h, fastly_body_handle_t b,
                                              xqd_world_string_t *backend,
                                              fastly_pending_request_handle_t *ret,
                                              fastly_error_t *err) {
  return convert_result(xqd_req_send_async_streaming(h, b, backend->ptr, backend->len, ret), err);
}

bool xqd_fastly_http_req_pending_req_wait(fastly_pending_request_handle_t h, fastly_response_t *ret,
                                          fastly_error_t *err) {
  return convert_result(xqd_req_pending_req_wait(h, &ret->f0, &ret->f1), err);
}

bool xqd_fastly_http_req_register_dynamic_backend(xqd_world_string_t *prefix,
                                                  xqd_world_string_t *target,
                                                  fastly_dynamic_backend_config_t *config,
                                                  fastly_error_t *err) {
  uint32_t backend_config_mask = 0;

  if (config->host_override.is_some)
    backend_config_mask |= BACKEND_CONFIG_HOST_OVERRIDE;
  if (config->connect_timeout.is_some)
    backend_config_mask |= BACKEND_CONFIG_CONNECT_TIMEOUT;
  if (config->use_ssl.is_some)
    backend_config_mask |= BACKEND_CONFIG_USE_SSL;
  if (config->ssl_min_version.is_some)
    backend_config_mask |= BACKEND_CONFIG_SSL_MIN_VERSION;
  if (config->ssl_max_version.is_some)
    backend_config_mask |= BACKEND_CONFIG_SSL_MAX_VERSION;
  if (config->cert_hostname.is_some)
    backend_config_mask |= BACKEND_CONFIG_CERT_HOSTNAME;
  if (config->ca_cert.is_some)
    backend_config_mask |= BACKEND_CONFIG_CA_CERT;
  if (config->ciphers.is_some)
    backend_config_mask |= BACKEND_CONFIG_CIPHERS;
  if (config->sni_hostname.is_some)
    backend_config_mask |= BACKEND_CONFIG_SNI_HOSTNAME;
  DynamicBackendConfig backend_configuration{
      .host_override = config->host_override.val.ptr,
      .host_override_len = config->host_override.val.len,
      .connect_timeout_ms = config->connect_timeout.val,
      .first_byte_timeout_ms = config->first_byte_timeout.val,
      .between_bytes_timeout_ms = config->between_bytes_timeout.val,
      .ssl_min_version = config->ssl_min_version.val,
      .ssl_max_version = config->ssl_max_version.val,
      .cert_hostname = config->cert_hostname.val.ptr,
      .cert_hostname_len = config->cert_hostname.val.len,
      .ca_cert = config->ca_cert.val.ptr,
      .ca_cert_len = config->ca_cert.val.len,
      .ciphers = config->ciphers.val.ptr,
      .ciphers_len = config->ciphers.val.len,
      .sni_hostname = config->sni_hostname.val.ptr,
      .sni_hostname_len = config->sni_hostname.val.len,
  };
  return convert_result(xqd_req_register_dynamic_backend(prefix->ptr, prefix->len, target->ptr,
                                                         target->len, backend_config_mask,
                                                         &backend_configuration),
                        err);
}

bool xqd_fastly_http_resp_new(fastly_response_handle_t *ret, fastly_error_t *err) {
  return convert_result(xqd_resp_new(ret), err);
}

bool xqd_fastly_http_resp_header_names_get(fastly_response_handle_t h, fastly_list_string_t *ret,
                                           fastly_error_t *err) {
  xqd_world_string_t *strs = static_cast<xqd_world_string_t *>(
      cabi_malloc(LIST_ALLOC_SIZE * sizeof(xqd_world_string_t), 1));
  size_t str_max = LIST_ALLOC_SIZE;
  size_t str_cnt = 0;
  size_t nwritten;
  char *buf = static_cast<char *>(cabi_malloc(HOSTCALL_BUFFER_LEN, 1));
  uint32_t cursor = 0;
  int64_t next_cursor = 0;
  while (true) {
    if (!convert_result(
            xqd_resp_header_names_get(h, buf, HEADER_MAX_LEN, cursor, &next_cursor, &nwritten),
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
        strs = static_cast<xqd_world_string_t *>(
            cabi_realloc(strs, str_max * sizeof(xqd_world_string_t), 1,
                         (str_max + LIST_ALLOC_SIZE) * sizeof(xqd_world_string_t)));
        str_max += LIST_ALLOC_SIZE;
      }
      strs[str_cnt].ptr = static_cast<char *>(cabi_malloc(i - offset + 1, 1));
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
  strs = static_cast<xqd_world_string_t *>(cabi_realloc(strs, str_max * sizeof(xqd_world_string_t),
                                                        1, str_cnt * sizeof(xqd_world_string_t)));
  ret->ptr = strs;
  ret->len = str_cnt;
  return true;
}

bool xqd_fastly_http_resp_header_values_get(fastly_response_handle_t h, xqd_world_string_t *name,
                                            fastly_option_list_string_t *ret, fastly_error_t *err) {
  size_t str_max = LIST_ALLOC_SIZE;
  xqd_world_string_t *strs = static_cast<xqd_world_string_t *>(cabi_malloc(str_max, 1));
  size_t str_cnt = 0;
  size_t nwritten;
  char *buf = static_cast<char *>(cabi_malloc(HOSTCALL_BUFFER_LEN, 1));
  uint32_t cursor = 0;
  int64_t next_cursor = 0;
  while (true) {
    if (!convert_result(xqd_resp_header_values_get(h, name->ptr, name->len, buf, HEADER_MAX_LEN,
                                                   cursor, &next_cursor, &nwritten),
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
        strs = static_cast<xqd_world_string_t *>(
            cabi_realloc(strs, str_max * sizeof(xqd_world_string_t), 1,
                         (str_max + LIST_ALLOC_SIZE) * sizeof(xqd_world_string_t)));
        str_max += LIST_ALLOC_SIZE;
      }
      strs[str_cnt].ptr = static_cast<char *>(cabi_malloc(i - offset + 1, 1));
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
  if (str_cnt > 0) {
    ret->is_some = true;
    ret->val.ptr = strs;
    ret->val.len = str_cnt;
    strs = static_cast<xqd_world_string_t *>(cabi_realloc(
        strs, str_max * sizeof(xqd_world_string_t), 1, str_cnt * sizeof(xqd_world_string_t)));
  } else {
    ret->is_some = false;
    cabi_free(strs);
  }
  return true;
}

bool xqd_fastly_http_resp_header_insert(fastly_response_handle_t h, xqd_world_string_t *name,
                                        xqd_world_string_t *value, fastly_error_t *err) {
  return convert_result(xqd_resp_header_insert(h, name->ptr, name->len, value->ptr, value->len),
                        err);
}

bool xqd_fastly_http_resp_header_append(fastly_response_handle_t h, xqd_world_string_t *name,
                                        xqd_world_string_t *value, fastly_error_t *err) {
  return convert_result(xqd_resp_header_append(h, name->ptr, name->len, value->ptr, value->len),
                        err);
}

bool xqd_fastly_http_resp_header_remove(fastly_response_handle_t h, xqd_world_string_t *name,
                                        fastly_error_t *err) {
  return convert_result(xqd_resp_header_remove(h, name->ptr, name->len), err);
}

bool xqd_fastly_http_resp_version_get(fastly_response_handle_t h, fastly_http_version_t *ret,
                                      fastly_error_t *err) {
  uint32_t xqd_http_version;
  if (!convert_result(xqd_resp_version_get(h, &xqd_http_version), err)) {
    return false;
  }
  *ret = convert_http_version(xqd_http_version);
  return true;
}

bool xqd_fastly_http_resp_send_downstream(fastly_response_handle_t h, fastly_body_handle_t b,
                                          bool streaming, fastly_error_t *err) {
  return convert_result(xqd_resp_send_downstream(h, b, streaming), err);
}

bool xqd_fastly_http_resp_status_get(fastly_response_handle_t h, fastly_http_status_t *ret,
                                     fastly_error_t *err) {
  return convert_result(xqd_resp_status_get(h, ret), err);
}

bool xqd_fastly_http_resp_status_set(fastly_response_handle_t h, fastly_http_status_t status,
                                     fastly_error_t *err) {
  return convert_result(xqd_resp_status_set(h, status), err);
}

bool xqd_fastly_dictionary_open(xqd_world_string_t *name, fastly_dictionary_handle_t *ret,
                                fastly_error_t *err) {
  return convert_result(xqd_dictionary_open(name->ptr, name->len, ret), err);
}

bool xqd_fastly_dictionary_get(fastly_dictionary_handle_t h, xqd_world_string_t *key,
                               fastly_option_string_t *ret, fastly_error_t *err) {
  ret->val.ptr = static_cast<char *>(cabi_malloc(DICTIONARY_ENTRY_MAX_LEN, 1));
  if (!convert_result(xqd_dictionary_get(h, key->ptr, key->len, ret->val.ptr,
                                         DICTIONARY_ENTRY_MAX_LEN, &ret->val.len),
                      err)) {
    if (*err == FASTLY_ERROR_OPTIONAL_NONE) {
      ret->is_some = false;
      return true;
    } else {
      cabi_free(ret->val.ptr);
      return false;
    }
  }
  ret->is_some = true;
  ret->val.ptr =
      static_cast<char *>(cabi_realloc(ret->val.ptr, DICTIONARY_ENTRY_MAX_LEN, 1, ret->val.len));
  return true;
}

bool xqd_fastly_geo_lookup(fastly_list_u8_t *addr_octets, xqd_world_string_t *ret,
                           fastly_error_t *err) {
  ret->ptr = static_cast<char *>(cabi_malloc(HOSTCALL_BUFFER_LEN, 1));
  if (!convert_result(xqd_geo_lookup(reinterpret_cast<char *>(addr_octets->ptr), addr_octets->len,
                                     ret->ptr, HOSTCALL_BUFFER_LEN, &ret->len),
                      err)) {
    cabi_free(ret->ptr);
    return false;
  }
  ret->ptr = static_cast<char *>(cabi_realloc(ret->ptr, HOSTCALL_BUFFER_LEN, 1, ret->len));
  return true;
}

bool xqd_fastly_object_store_open(xqd_world_string_t *name, fastly_object_store_handle_t *ret,
                                  fastly_error_t *err) {
  return convert_result(xqd_object_store_open(name->ptr, name->len, ret), err);
}

bool xqd_fastly_object_store_lookup(fastly_object_store_handle_t store, xqd_world_string_t *key,
                                    fastly_option_body_handle_t *ret, fastly_error_t *err) {
  ret->val = INVALID_HANDLE;
  bool ok = convert_result(xqd_object_store_get(store, key->ptr, key->len, &ret->val), err);
  if ((!ok && *err == FASTLY_ERROR_OPTIONAL_NONE) || ret->val == INVALID_HANDLE) {
    ret->is_some = false;
    return true;
  }
  ret->is_some = true;
  return ok;
}

bool xqd_fastly_object_store_insert(fastly_object_store_handle_t store, xqd_world_string_t *key,
                                    fastly_body_handle_t body_handle, fastly_error_t *err) {
  return convert_result(xqd_object_store_insert(store, key->ptr, key->len, body_handle), err);
}

bool xqd_fastly_async_io_select(fastly_list_async_handle_t *hs, uint32_t timeout_ms,
                                fastly_option_u32_t *ret, fastly_error_t *err) {
  if (!convert_result(xqd_async_select(hs->ptr, hs->len, timeout_ms, &ret->val), err)) {
    if (*err == FASTLY_ERROR_OPTIONAL_NONE) {
      ret->is_some = false;
      return true;
    }
    return false;
  }
  ret->is_some = true;
  return true;
}
bool xqd_fastly_async_io_is_ready(fastly_async_handle_t handle, bool *ret, fastly_error_t *err) {
  uint32_t ret_int;
  if (!convert_result(xqd_async_is_ready(handle, &ret_int), err)) {
    return false;
  }
  *ret = (bool)ret_int;
  return true;
}

#else

bool xqd_fastly_abi_init(uint64_t abi_version, fastly_error_t *err) {
  return fastly_abi_init(abi_version, err);
}

bool xqd_fastly_uap_parse(xqd_world_string_t *user_agent, fastly_user_agent_t *ret,
                          fastly_error_t *err) {
  return fastly_uap_parse(user_agent, ret, err);
}

bool xqd_fastly_http_body_new(fastly_body_handle_t *ret, fastly_error_t *err) {
  return fastly_http_body_new(ret, err);
}

bool xqd_fastly_http_body_append(fastly_body_handle_t dest, fastly_body_handle_t src,
                                 fastly_error_t *err) {
  return fastly_http_body_append(dest, src, err);
}

bool xqd_fastly_http_body_read(fastly_body_handle_t h, uint32_t chunk_size, fastly_list_u8_t *ret,
                               fastly_error_t *err) {
  return fastly_http_body_read(h, chunk_size, ret, err);
}

bool xqd_fastly_http_body_write(fastly_body_handle_t h, fastly_list_u8_t *buf,
                                fastly_body_write_end_t end, uint32_t *ret, fastly_error_t *err) {
  return fastly_http_body_write(h, buf, end, ret, err);
}

bool xqd_fastly_http_body_close(fastly_body_handle_t h, fastly_error_t *err) {
  return fastly_http_body_close(h, err);
}

bool xqd_fastly_log_endpoint_get(xqd_world_string_t *name, fastly_log_endpoint_handle_t *ret,
                                 fastly_error_t *err) {
  return fastly_log_endpoint_get(name, ret, err);
}

bool xqd_fastly_log_write(fastly_log_endpoint_handle_t h, xqd_world_string_t *msg,
                          fastly_error_t *err) {
  return fastly_log_write(h, msg, err);
}

bool xqd_fastly_http_req_body_downstream_get(fastly_request_t *ret, fastly_error_t *err) {
  return fastly_http_req_body_downstream_get(ret, err);
}

bool xqd_fastly_http_req_cache_override_set(fastly_request_handle_t h,
                                            fastly_http_cache_override_tag_t tag,
                                            uint32_t *maybe_ttl,
                                            uint32_t *maybe_stale_while_revalidate,
                                            xqd_world_string_t *maybe_sk, fastly_error_t *err) {
  return fastly_http_req_cache_override_set(h, tag, maybe_ttl, maybe_stale_while_revalidate,
                                            maybe_sk, err);
}

bool xqd_fastly_http_req_downstream_client_ip_addr(fastly_list_u8_t *ret, fastly_error_t *err) {
  return fastly_http_req_downstream_client_ip_addr(ret, err);
}

bool xqd_fastly_http_req_downstream_client_h2_fingerprint(fastly_list_u8_t *ret,
                                                          fastly_error_t *err) {
  return fastly_http_req_downstream_client_h2_fingerprint(ret, err);
}

bool xqd_fastly_http_req_downstream_tls_cipher_openssl_name(xqd_world_string_t *ret,
                                                            fastly_error_t *err) {
  return fastly_http_req_downstream_tls_cipher_openssl_name(ret, err);
}

bool xqd_fastly_http_req_downstream_tls_protocol(xqd_world_string_t *ret, fastly_error_t *err) {
  return fastly_http_req_downstream_tls_protocol(ret, err);
}

bool xqd_fastly_http_req_downstream_tls_client_hello(fastly_list_u8_t *ret, fastly_error_t *err) {
  return fastly_http_req_downstream_tls_client_hello(ret, err);
}

bool xqd_fastly_http_req_downstream_tls_client_certificate(fastly_list_u8_t *ret,
                                                           fastly_error_t *err) {
  return fastly_http_req_downstream_tls_client_certificate(ret, err);
}

bool xqd_fastly_http_req_downstream_tls_client_cert_verify_result(fastly_error_t *err) {
  return fastly_http_req_downstream_tls_client_cert_verify_result(err);
}

bool xqd_fastly_http_req_downstream_tls_ja3_md5(fastly_list_u8_t *ret, fastly_error_t *err) {
  return fastly_http_req_downstream_tls_ja3_md5(ret, err);
}

bool xqd_fastly_http_req_new(fastly_request_handle_t *ret, fastly_error_t *err) {
  return fastly_http_req_new(ret, err);
}

bool xqd_fastly_http_req_header_names_get(fastly_request_handle_t h, fastly_list_string_t *ret,
                                          fastly_error_t *err) {
  return fastly_http_req_header_names_get(h, ret, err);
}

bool xqd_fastly_http_req_header_value_get(fastly_request_handle_t h, xqd_world_string_t *name,
                                          fastly_option_string_t *ret, fastly_error_t *err) {
  return fastly_http_req_header_value_get(h, name, ret, err);
}

bool xqd_fastly_http_req_header_values_get(fastly_request_handle_t h, xqd_world_string_t *name,
                                           fastly_option_list_string_t *ret, fastly_error_t *err) {
  return fastly_http_req_header_values_get(h, name, ret, err);
}

bool xqd_fastly_http_req_header_values_set(fastly_request_handle_t h, xqd_world_string_t *name,
                                           fastly_list_string_t *values, fastly_error_t *err) {
  return fastly_http_req_header_values_set(h, name, values, err);
}

bool xqd_fastly_http_req_header_insert(fastly_request_handle_t h, xqd_world_string_t *name,
                                       xqd_world_string_t *value, fastly_error_t *err) {
  return fastly_http_req_header_insert(h, name, value, err);
}

bool xqd_fastly_http_req_header_append(fastly_request_handle_t h, xqd_world_string_t *name,
                                       xqd_world_string_t *value, fastly_error_t *err) {
  return fastly_http_req_header_append(h, name, value, err);
}

bool xqd_fastly_http_req_header_remove(fastly_request_handle_t h, xqd_world_string_t *name,
                                       fastly_error_t *err) {
  return fastly_http_req_header_remove(h, name, err);
}

bool xqd_fastly_http_req_method_get(fastly_request_handle_t h, xqd_world_string_t *ret,
                                    fastly_error_t *err) {
  return fastly_http_req_method_get(h, ret, err);
}

bool xqd_fastly_http_req_method_set(fastly_request_handle_t h, xqd_world_string_t *method,
                                    fastly_error_t *err) {
  return fastly_http_req_method_set(h, method, err);
}

bool xqd_fastly_http_req_uri_get(fastly_request_handle_t h, xqd_world_string_t *ret,
                                 fastly_error_t *err) {
  return fastly_http_req_uri_get(h, ret, err);
}

bool xqd_fastly_http_req_uri_set(fastly_request_handle_t h, xqd_world_string_t *uri,
                                 fastly_error_t *err) {
  return fastly_http_req_uri_set(h, uri, err);
}

bool xqd_fastly_http_req_version_get(fastly_request_handle_t h, fastly_http_version_t *ret,
                                     fastly_error_t *err) {
  return fastly_http_req_version_get(h, ret, err);
}

bool xqd_fastly_http_req_version_set(fastly_request_handle_t h, fastly_http_version_t version,
                                     fastly_error_t *err) {
  return fastly_http_req_version_set(h, version, err);
}

bool xqd_fastly_http_req_send(fastly_request_handle_t h, fastly_body_handle_t b,
                              xqd_world_string_t *backend, fastly_response_t *ret,
                              fastly_error_t *err) {
  return fastly_http_req_send(h, b, backend, ret, err);
}

bool xqd_fastly_http_req_send_async(fastly_request_handle_t h, fastly_body_handle_t b,
                                    xqd_world_string_t *backend,
                                    fastly_pending_request_handle_t *ret, fastly_error_t *err) {
  return fastly_http_req_send_async(h, b, backend, ret, err);
}

bool xqd_fastly_http_req_send_async_streaming(fastly_request_handle_t h, fastly_body_handle_t b,
                                              xqd_world_string_t *backend,
                                              fastly_pending_request_handle_t *ret,
                                              fastly_error_t *err) {
  return fastly_http_req_send_async_streaming(h, b, backend, ret, err);
}

bool xqd_fastly_http_req_pending_req_poll(fastly_pending_request_handle_t h,
                                          fastly_option_response_t *ret, fastly_error_t *err) {
  return fastly_http_req_pending_req_poll(h, ret, err);
}

bool xqd_fastly_http_req_pending_req_wait(fastly_pending_request_handle_t h, fastly_response_t *ret,
                                          fastly_error_t *err) {
  return fastly_http_req_pending_req_wait(h, ret, err);
}

bool xqd_fastly_http_req_pending_req_select(fastly_list_pending_request_handle_t *h,
                                            fastly_tuple2_u32_response_t *ret,
                                            fastly_error_t *err) {
  return fastly_http_req_pending_req_select(h, ret, err);
}

bool xqd_fastly_http_req_key_is_valid(bool *ret, fastly_error_t *err) {
  return fastly_http_req_key_is_valid(ret, err);
}

bool xqd_fastly_http_req_close(fastly_request_handle_t h, fastly_error_t *err) {
  return fastly_http_req_close(h, err);
}

bool xqd_fastly_http_req_auto_decompress_response_set(fastly_request_handle_t h,
                                                      fastly_content_encodings_t encodings,
                                                      fastly_error_t *err) {
  return fastly_http_req_auto_decompress_response_set(h, encodings, err);
}

bool xqd_fastly_http_req_upgrade_websocket(xqd_world_string_t *backend, fastly_error_t *err) {
  return fastly_http_req_upgrade_websocket(backend, err);
}

bool xqd_fastly_http_req_redirect_to_websocket_proxy(xqd_world_string_t *backend,
                                                     fastly_error_t *err) {
  return fastly_http_req_redirect_to_websocket_proxy(backend, err);
}

bool xqd_fastly_http_req_redirect_to_grip_proxy(xqd_world_string_t *backend, fastly_error_t *err) {
  return fastly_http_req_redirect_to_grip_proxy(backend, err);
}

bool xqd_fastly_http_req_framing_headers_mode_set(fastly_request_handle_t h,
                                                  fastly_framing_headers_mode_t mode,
                                                  fastly_error_t *err) {
  return fastly_http_req_framing_headers_mode_set(h, mode, err);
}

bool xqd_fastly_http_req_register_dynamic_backend(xqd_world_string_t *prefix,
                                                  xqd_world_string_t *target,
                                                  fastly_dynamic_backend_config_t *config,
                                                  fastly_error_t *err) {
  return fastly_http_req_register_dynamic_backend(prefix, target, config, err);
}

bool xqd_fastly_http_resp_new(fastly_response_handle_t *ret, fastly_error_t *err) {
  return fastly_http_resp_new(ret, err);
}
bool xqd_fastly_http_resp_header_names_get(fastly_response_handle_t h, fastly_list_string_t *ret,
                                           fastly_error_t *err) {
  return fastly_http_resp_header_names_get(h, ret, err);
}
bool xqd_fastly_http_resp_header_value_get(fastly_response_handle_t h, xqd_world_string_t *name,
                                           fastly_option_string_t *ret, fastly_error_t *err) {
  return fastly_http_resp_header_value_get(h, name, ret, err);
}
bool xqd_fastly_http_resp_header_values_get(fastly_response_handle_t h, xqd_world_string_t *name,
                                            fastly_option_list_string_t *ret, fastly_error_t *err) {
  return fastly_http_resp_header_values_get(h, name, ret, err);
}
bool xqd_fastly_http_resp_header_values_set(fastly_response_handle_t h, xqd_world_string_t *name,
                                            fastly_list_string_t *values, fastly_error_t *err) {
  return fastly_http_resp_header_values_set(h, name, values, err);
}
bool xqd_fastly_http_resp_header_insert(fastly_response_handle_t h, xqd_world_string_t *name,
                                        xqd_world_string_t *value, fastly_error_t *err) {
  return fastly_http_resp_header_insert(h, name, value, err);
}
bool xqd_fastly_http_resp_header_append(fastly_response_handle_t h, xqd_world_string_t *name,
                                        xqd_world_string_t *value, fastly_error_t *err) {
  return fastly_http_resp_header_append(h, name, value, err);
}
bool xqd_fastly_http_resp_header_remove(fastly_response_handle_t h, xqd_world_string_t *name,
                                        fastly_error_t *err) {
  return fastly_http_resp_header_remove(h, name, err);
}
bool xqd_fastly_http_resp_version_get(fastly_response_handle_t h, fastly_http_version_t *ret,
                                      fastly_error_t *err) {
  return fastly_http_resp_version_get(h, ret, err);
}
bool xqd_fastly_http_resp_version_set(fastly_response_handle_t h, fastly_http_version_t version,
                                      fastly_error_t *err) {
  return fastly_http_resp_version_set(h, version, err);
}
bool xqd_fastly_http_resp_send_downstream(fastly_response_handle_t h, fastly_body_handle_t b,
                                          bool streaming, fastly_error_t *err) {
  return fastly_http_resp_send_downstream(h, b, streaming, err);
}
bool xqd_fastly_http_resp_status_get(fastly_response_handle_t h, fastly_http_status_t *ret,
                                     fastly_error_t *err) {
  return fastly_http_resp_status_get(h, ret, err);
}
bool xqd_fastly_http_resp_status_set(fastly_response_handle_t h, fastly_http_status_t status,
                                     fastly_error_t *err) {
  return fastly_http_resp_status_set(h, status, err);
}
bool xqd_fastly_http_resp_close(fastly_response_handle_t h, fastly_error_t *err) {
  return fastly_http_resp_close(h, err);
}
bool xqd_fastly_http_resp_framing_headers_mode_set(fastly_response_handle_t h,
                                                   fastly_framing_headers_mode_t mode,
                                                   fastly_error_t *err) {
  return fastly_http_resp_framing_headers_mode_set(h, mode, err);
}
bool xqd_fastly_dictionary_open(xqd_world_string_t *name, fastly_dictionary_handle_t *ret,
                                fastly_error_t *err) {
  return fastly_dictionary_open(name, ret, err);
}
bool xqd_fastly_dictionary_get(fastly_dictionary_handle_t h, xqd_world_string_t *key,
                               fastly_option_string_t *ret, fastly_error_t *err) {
  return fastly_dictionary_get(h, key, ret, err);
}
bool xqd_fastly_geo_lookup(fastly_list_u8_t *addr_octets, xqd_world_string_t *ret,
                           fastly_error_t *err) {
  return fastly_geo_lookup(addr_octets, ret, err);
}
bool xqd_fastly_object_store_open(xqd_world_string_t *name, fastly_object_store_handle_t *ret,
                                  fastly_error_t *err) {
  return fastly_object_store_open(name, ret, err);
}
bool xqd_fastly_object_store_lookup(fastly_object_store_handle_t store, xqd_world_string_t *key,
                                    fastly_option_body_handle_t *ret, fastly_error_t *err) {
  return fastly_object_store_lookup(store, key, ret, err);
}
bool xqd_fastly_object_store_lookup_as_fd(fastly_object_store_handle_t store,
                                          xqd_world_string_t *key, fastly_option_fd_t *ret,
                                          fastly_error_t *err) {
  return fastly_object_store_lookup_as_fd(store, key, ret, err);
}
bool xqd_fastly_object_store_insert(fastly_object_store_handle_t store, xqd_world_string_t *key,
                                    fastly_body_handle_t body_handle, fastly_error_t *err) {
  return fastly_object_store_insert(store, key, body_handle, err);
}
bool xqd_fastly_secret_store_open(xqd_world_string_t *name, fastly_secret_store_handle_t *ret,
                                  fastly_error_t *err) {
  return fastly_secret_store_open(name, ret, err);
}
bool xqd_fastly_secret_store_get(fastly_secret_store_handle_t store, xqd_world_string_t *key,
                                 fastly_option_secret_handle_t *ret, fastly_error_t *err) {
  return fastly_secret_store_get(store, key, ret, err);
}
bool xqd_fastly_secret_store_plaintext(fastly_secret_handle_t secret, fastly_option_string_t *ret,
                                       fastly_error_t *err) {
  return fastly_secret_store_plaintext(secret, ret, err);
}
bool xqd_fastly_async_io_select(fastly_list_async_handle_t *hs, uint32_t timeout_ms,
                                fastly_option_u32_t *ret, fastly_error_t *err) {
  return fastly_async_io_select(hs, timeout_ms, ret, err);
}
bool xqd_fastly_async_io_is_ready(fastly_async_handle_t handle, bool *ret, fastly_error_t *err) {
  return fastly_async_io_is_ready(handle, ret, err);
}
bool xqd_fastly_purge_surrogate_key(xqd_world_string_t *surrogate_key, bool soft_purge,
                                    fastly_purge_result_t *ret, fastly_error_t *err) {
  return fastly_purge_surrogate_key(surrogate_key, soft_purge, ret, err);
}

#endif
