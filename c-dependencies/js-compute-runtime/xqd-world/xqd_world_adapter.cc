#include "xqd_world_adapter.h"

JSContext *context = nullptr;

void set_js_context(JSContext *cx) { context = cx; }

#ifndef COMPONENT

extern "C" {

__attribute__((weak, export_name("cabi_realloc"))) void *
cabi_realloc(void *ptr, size_t orig_size, size_t align, size_t new_size) {
  return JS_realloc(context, ptr, orig_size, new_size);
}
}

static fastly_error_t convert_result(int res) {
  switch (res) {
  case 0:
    return FASTLY_RESULT_ERROR_OK;
  case 1:
    return FASTLY_ERROR_GENERIC_ERROR;
  case 2:
    return FASTLY_ERROR_INVALID_ARGUMENT;
  case 3:
    return FASTLY_ERROR_BAD_HANDLE;
  case 4:
    return FASTLY_ERROR_BUFFER_LEN;
  case 5:
    return FASTLY_ERROR_UNSUPPORTED;
  case 6:
    return FASTLY_ERROR_BAD_ALIGN;
  case 7:
    return FASTLY_ERROR_HTTP_INVALID;
  case 8:
    return FASTLY_ERROR_HTTP_USER;
  case 9:
    return FASTLY_ERROR_HTTP_INCOMPLETE;
  case 10:
    return FASTLY_ERROR_MISSING_OPTIONAL;
  case 11:
    return FASTLY_ERROR_HTTP_HEAD_TOO_LARGE;
  case 12:
    return FASTLY_ERROR_HTTP_INVALID_STATUS;
  case 100:
    return FASTLY_ERROR_UNKNOWN_ERROR;
  default:
    return FASTLY_ERROR_UNKNOWN_ERROR;
  }
}

fastly_error_t xqd_fastly_http_body_new(fastly_body_handle_t *ret) {
  return convert_result(xqd_body_new(ret));
}

fastly_error_t xqd_fastly_http_body_append(fastly_body_handle_t src, fastly_body_handle_t dest) {
  return convert_result(xqd_body_append(src, dest));
}

fastly_error_t xqd_fastly_http_body_read(fastly_body_handle_t h, uint32_t chunk_size,
                                         fastly_list_u8_t *ret) {
  ret->ptr = static_cast<uint8_t *>(JS_malloc(context, chunk_size));
  return convert_result(xqd_body_read(h, reinterpret_cast<char *>(ret->ptr),
                                      static_cast<size_t>(chunk_size), &ret->len));
}

fastly_error_t xqd_fastly_http_body_write(fastly_body_handle_t h, fastly_list_u8_t *buf,
                                          fastly_body_write_end_t end, uint32_t *ret) {
  return convert_result(xqd_body_write(h, reinterpret_cast<char *>(buf->ptr), buf->len,
                                       end == FASTLY_BODY_WRITE_END_BACK
                                           ? BodyWriteEnd::BodyWriteEndBack
                                           : BodyWriteEnd::BodyWriteEndFront,
                                       reinterpret_cast<size_t *>(ret)));
}

fastly_error_t xqd_fastly_http_body_close(fastly_body_handle_t h) {
  return convert_result(xqd_body_close(h));
}

fastly_error_t xqd_fastly_log_endpoint_get(xqd_world_string_t *name,
                                           fastly_log_endpoint_handle_t *ret) {
  return convert_result(
      xqd_log_endpoint_get(reinterpret_cast<const char *>(name->ptr), name->len, ret));
}

fastly_error_t xqd_fastly_log_write(fastly_log_endpoint_handle_t h, xqd_world_string_t *msg) {
  size_t nwritten = 0;
  return convert_result(
      xqd_log_write(h, reinterpret_cast<const char *>(msg->ptr), msg->len, &nwritten));
}

fastly_error_t xqd_fastly_http_req_body_downstream_get(fastly_request_t *ret) {
  return convert_result(xqd_req_body_downstream_get(&ret->f0, &ret->f1));
}

int convert_tag(fastly_http_cache_override_tag_t tag) {
  switch (tag) {
  case FASTLY_HTTP_CACHE_OVERRIDE_TAG_NONE:
    return CACHE_OVERRIDE_NONE;
  case FASTLY_HTTP_CACHE_OVERRIDE_TAG_PASS:
    return CACHE_OVERRIDE_PASS;
  case FASTLY_HTTP_CACHE_OVERRIDE_TAG_TTL:
    return CACHE_OVERRIDE_TTL;
  case FASTLY_HTTP_CACHE_OVERRIDE_TAG_STALE_WHILE_REVALIDATE:
    return CACHE_OVERRIDE_STALE_WHILE_REVALIDATE;
  case FASTLY_HTTP_CACHE_OVERRIDE_TAG_PCI:
  default:
    return CACHE_OVERRIDE_PCI;
  }
}

fastly_error_t xqd_fastly_http_req_cache_override_set(fastly_request_handle_t h,
                                                      fastly_http_cache_override_tag_t tag,
                                                      fastly_option_u32_t *ttl,
                                                      fastly_option_u32_t *stale_while_revalidate,
                                                      fastly_option_string_t *sk) {
  xqd_world_string_t sk_str;
  if (sk->is_some) {
    sk_str = sk->val;
  } else {
    sk_str.len = 0;
    sk_str.ptr = const_cast<char *>("");
  }
  return convert_result(xqd_req_cache_override_v2_set(
      h, convert_tag(tag), ttl->is_some ? ttl->val : 0,
      stale_while_revalidate->is_some ? stale_while_revalidate->val : 0,
      reinterpret_cast<char *>(sk_str.ptr), sk_str.len));
}

fastly_error_t xqd_fastly_http_req_downstream_client_ip_addr(fastly_list_u8_t *ret) {
  ret->ptr = static_cast<uint8_t *>(JS_malloc(context, 16));
  return convert_result(
      xqd_req_downstream_client_ip_addr_get(reinterpret_cast<char *>(&ret->ptr), &ret->len));
}

fastly_error_t xqd_fastly_http_req_new(fastly_request_handle_t *ret) {
  return convert_result(xqd_req_new(ret));
}

fastly_error_t xqd_fastly_http_req_header_names_get(fastly_request_handle_t h,
                                                    fastly_cursor_t cursor,
                                                    fastly_tuple2_string_option_cursor_t *ret) {
  ret->f0.ptr = static_cast<char *>(JS_malloc(context, HEADER_MAX_LEN));
  int64_t next_cursor;
  fastly_error_t result = convert_result(
      xqd_req_header_names_get(h, ret->f0.ptr, HEADER_MAX_LEN, cursor, &next_cursor, &ret->f0.len));
  ret->f0.ptr = static_cast<char *>(JS_realloc(context, ret->f0.ptr, HEADER_MAX_LEN, ret->f0.len));
  if (next_cursor < 0) {
    ret->f1.is_some = false;
  } else {
    ret->f1.is_some = true;
    ret->f1.val = (uint32_t)next_cursor;
  }
  return result;
}

fastly_error_t xqd_fastly_http_req_header_values_get(fastly_request_handle_t h,
                                                     xqd_world_string_t *name,
                                                     fastly_cursor_t cursor,
                                                     fastly_tuple2_string_option_cursor_t *ret) {
  ret->f0.ptr = static_cast<char *>(JS_malloc(context, HEADER_MAX_LEN));
  int64_t next_cursor;
  fastly_error_t result = convert_result(xqd_req_header_values_get(
      h, name->ptr, name->len, ret->f0.ptr, HEADER_MAX_LEN, cursor, &next_cursor, &ret->f0.len));
  if (result != FASTLY_RESULT_ERROR_OK) {
    JS_free(context, ret->f0.ptr);
    return result;
  }
  if (next_cursor < 0) {
    ret->f1.is_some = false;
  } else {
    ret->f1.is_some = true;
    ret->f1.val = (uint32_t)next_cursor;
  }
  ret->f0.ptr = static_cast<char *>(JS_realloc(context, ret->f0.ptr, HEADER_MAX_LEN, ret->f0.len));
  return result;
}

fastly_error_t xqd_fastly_http_req_header_insert(fastly_request_handle_t h,
                                                 xqd_world_string_t *name,
                                                 xqd_world_string_t *value) {
  return convert_result(xqd_req_header_insert(h, name->ptr, name->len, value->ptr, value->len));
}

fastly_error_t xqd_fastly_http_req_header_append(fastly_request_handle_t h,
                                                 xqd_world_string_t *name,
                                                 xqd_world_string_t *value) {
  return convert_result(xqd_req_header_append(h, name->ptr, name->len, value->ptr, value->len));
}

fastly_error_t xqd_fastly_http_req_header_remove(fastly_request_handle_t h,
                                                 xqd_world_string_t *name) {
  return convert_result(xqd_req_header_remove(h, name->ptr, name->len));
}

fastly_error_t xqd_fastly_http_req_method_get(fastly_request_handle_t h, xqd_world_string_t *ret) {
  ret->ptr = static_cast<char *>(JS_malloc(context, METHOD_MAX_LEN));
  return convert_result(
      xqd_req_method_get(h, reinterpret_cast<char *>(ret->ptr), METHOD_MAX_LEN, &ret->len));
}

fastly_error_t xqd_fastly_http_req_method_set(fastly_request_handle_t h,
                                              xqd_world_string_t *method) {
  return convert_result(
      xqd_req_method_set(h, reinterpret_cast<const char *>(method->ptr), method->len));
}

fastly_error_t xqd_fastly_http_req_uri_get(fastly_request_handle_t h, xqd_world_string_t *ret) {
  ret->ptr = static_cast<char *>(JS_malloc(context, URI_MAX_LEN));
  return convert_result(xqd_req_uri_get(h, ret->ptr, URI_MAX_LEN, &ret->len));
}

fastly_error_t xqd_fastly_http_req_uri_set(fastly_request_handle_t h, xqd_world_string_t *uri) {
  return convert_result(xqd_req_uri_set(h, uri->ptr, uri->len));
}

fastly_error_t xqd_fastly_http_req_version_get(fastly_request_handle_t h,
                                               fastly_http_version_t *ret) {
  return convert_result(xqd_req_version_get(h, reinterpret_cast<uint32_t *>(ret)));
}

fastly_error_t xqd_fastly_http_req_send(fastly_request_handle_t h, fastly_body_handle_t b,
                                        xqd_world_string_t *backend, fastly_response_t *ret) {
  return convert_result(xqd_req_send(h, b, backend->ptr, backend->len, &ret->f0, &ret->f1));
}

fastly_error_t xqd_fastly_http_req_send_async(fastly_request_handle_t h, fastly_body_handle_t b,
                                              xqd_world_string_t *backend,
                                              fastly_pending_request_handle_t *ret) {
  return convert_result(xqd_req_send_async(h, b, backend->ptr, backend->len, ret));
}

fastly_error_t xqd_fastly_http_req_send_async_streaming(fastly_request_handle_t h,
                                                        fastly_body_handle_t b,
                                                        xqd_world_string_t *backend,
                                                        fastly_pending_request_handle_t *ret) {
  return convert_result(xqd_req_send_async_streaming(h, b, backend->ptr, backend->len, ret));
}

fastly_error_t xqd_fastly_http_req_pending_req_select(fastly_list_pending_request_handle_t *h,
                                                      fastly_tuple2_u32_response_t *ret) {
  return convert_result(
      xqd_req_pending_req_select(h->ptr, h->len, &ret->f0, &ret->f1.f0, &ret->f1.f1));
}

fastly_error_t xqd_fastly_http_req_pending_req_wait(fastly_pending_request_handle_t h,
                                                    fastly_response_t *ret) {
  return convert_result(xqd_req_pending_req_wait(h, &ret->f0, &ret->f1));
}

fastly_error_t
xqd_fastly_http_req_register_dynamic_backend(xqd_world_string_t *prefix, xqd_world_string_t *target,
                                             fastly_dynamic_backend_config_t *config) {
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
                                                         &backend_configuration));
}

fastly_error_t xqd_fastly_http_resp_new(fastly_response_handle_t *ret) {
  return convert_result(xqd_resp_new(ret));
}

fastly_error_t xqd_fastly_http_resp_header_names_get(fastly_request_handle_t h,
                                                     fastly_cursor_t cursor,
                                                     fastly_tuple2_string_option_cursor_t *ret) {
  ret->f0.ptr = static_cast<char *>(JS_malloc(context, HEADER_MAX_LEN));
  int64_t next_cursor;
  fastly_error_t result = convert_result(xqd_resp_header_names_get(
      h, ret->f0.ptr, HEADER_MAX_LEN, cursor, &next_cursor, &ret->f0.len));
  if (result != FASTLY_RESULT_ERROR_OK) {
    JS_free(context, ret->f0.ptr);
    return result;
  }
  ret->f0.ptr = static_cast<char *>(JS_realloc(context, ret->f0.ptr, HEADER_MAX_LEN, ret->f0.len));
  if (next_cursor < 0) {
    ret->f1.is_some = false;
  } else {
    ret->f1.is_some = true;
    ret->f1.val = (uint32_t)next_cursor;
  }
  return result;
}

fastly_error_t xqd_fastly_http_resp_header_values_get(fastly_request_handle_t h,
                                                      xqd_world_string_t *name,
                                                      fastly_cursor_t cursor,
                                                      fastly_tuple2_string_option_cursor_t *ret) {
  ret->f0.ptr = static_cast<char *>(JS_malloc(context, HEADER_MAX_LEN));
  int64_t next_cursor;
  fastly_error_t result = convert_result(xqd_resp_header_values_get(
      h, name->ptr, name->len, ret->f0.ptr, HEADER_MAX_LEN, cursor, &next_cursor, &ret->f0.len));
  if (next_cursor < 0) {
    ret->f1.is_some = false;
  } else {
    ret->f1.is_some = true;
    ret->f1.val = (uint32_t)next_cursor;
  }
  ret->f0.ptr = static_cast<char *>(JS_realloc(context, ret->f0.ptr, HEADER_MAX_LEN, ret->f0.len));
  return result;
}

fastly_error_t xqd_fastly_http_resp_header_insert(fastly_request_handle_t h,
                                                  xqd_world_string_t *name,
                                                  xqd_world_string_t *value) {
  return convert_result(xqd_resp_header_insert(h, name->ptr, name->len, value->ptr, value->len));
}

fastly_error_t xqd_fastly_http_resp_header_append(fastly_request_handle_t h,
                                                  xqd_world_string_t *name,
                                                  xqd_world_string_t *value) {
  return convert_result(xqd_resp_header_append(h, name->ptr, name->len, value->ptr, value->len));
}

fastly_error_t xqd_fastly_http_resp_header_remove(fastly_request_handle_t h,
                                                  xqd_world_string_t *name) {
  return convert_result(xqd_resp_header_remove(h, name->ptr, name->len));
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

fastly_error_t xqd_fastly_http_resp_version_get(fastly_response_handle_t h,
                                                fastly_http_version_t *ret) {
  uint32_t xqd_http_version;
  fastly_error_t result = convert_result(xqd_resp_version_get(h, &xqd_http_version));
  if (result != FASTLY_RESULT_ERROR_OK)
    return result;
  *ret = convert_http_version(xqd_http_version);
  return result;
}

fastly_error_t xqd_fastly_http_resp_send_downstream(fastly_response_handle_t h,
                                                    fastly_body_handle_t b, bool streaming) {
  return convert_result(xqd_resp_send_downstream(h, b, streaming));
}

fastly_error_t xqd_fastly_http_resp_status_get(fastly_response_handle_t h,
                                               fastly_http_status_t *ret) {
  return convert_result(xqd_resp_status_get(h, ret));
}

fastly_error_t xqd_fastly_http_resp_status_set(fastly_response_handle_t h,
                                               fastly_http_status_t status) {
  return convert_result(xqd_resp_status_set(h, status));
}

fastly_error_t xqd_fastly_dictionary_open(xqd_world_string_t *name,
                                          fastly_dictionary_handle_t *ret) {
  return convert_result(xqd_dictionary_open(name->ptr, name->len, ret));
}

fastly_error_t xqd_fastly_dictionary_get(fastly_dictionary_handle_t h, xqd_world_string_t *key,
                                         xqd_world_string_t *ret) {
  ret->ptr = static_cast<char *>(JS_malloc(context, DICTIONARY_ENTRY_MAX_LEN));
  fastly_error_t result = convert_result(
      xqd_dictionary_get(h, key->ptr, key->len, ret->ptr, DICTIONARY_ENTRY_MAX_LEN, &ret->len));
  if (result != FASTLY_RESULT_ERROR_OK) {
    JS_free(context, ret->ptr);
    return result;
  }
  ret->ptr = static_cast<char *>(JS_realloc(context, ret->ptr, HEADER_MAX_LEN, ret->len));
  return result;
}

fastly_error_t xqd_fastly_geo_lookup(fastly_list_u8_t *addr_octets, xqd_world_string_t *ret) {
  ret->ptr = static_cast<char *>(JS_malloc(context, HOSTCALL_BUFFER_LEN));
  fastly_error_t result =
      convert_result(xqd_geo_lookup(reinterpret_cast<char *>(addr_octets->ptr), addr_octets->len,
                                    ret->ptr, HOSTCALL_BUFFER_LEN, &ret->len));
  if (result != FASTLY_RESULT_ERROR_OK) {
    JS_free(context, ret->ptr);
    return result;
  }
  return result;
}

fastly_error_t xqd_fastly_object_store_open(xqd_world_string_t *name,
                                            fastly_object_store_handle_t *ret) {
  return convert_result(xqd_object_store_open(name->ptr, name->len, ret));
}

fastly_error_t xqd_fastly_object_store_lookup(fastly_object_store_handle_t store,
                                              xqd_world_string_t *key, fastly_body_handle_t *ret) {
  return convert_result(xqd_object_store_get(store, key->ptr, key->len, ret));
}

fastly_error_t xqd_fastly_object_store_insert(fastly_object_store_handle_t store,
                                              xqd_world_string_t *key,
                                              fastly_body_handle_t body_handle) {
  return convert_result(xqd_object_store_insert(store, key->ptr, key->len, body_handle));
}

#else

fastly_error_t xqd_fastly_abi_init(uint64_t abi_version) { return fastly_abi_init(abi_version); }

fastly_error_t xqd_fastly_uap_parse(xqd_world_string_t *user_agent, fastly_user_agent_t *ret) {
  return fastly_uap_parse(user_agent, ret);
}

fastly_error_t xqd_fastly_http_body_new(fastly_body_handle_t *ret) {
  return fastly_http_body_new(ret);
}

fastly_error_t xqd_fastly_http_body_append(fastly_body_handle_t dest, fastly_body_handle_t src) {
  return fastly_http_body_append(dest, src);
}

fastly_error_t xqd_fastly_http_body_read(fastly_body_handle_t h, uint32_t chunk_size,
                                         fastly_list_u8_t *ret) {
  return fastly_http_body_read(h, chunk_size, ret);
}

fastly_error_t xqd_fastly_http_body_write(fastly_body_handle_t h, fastly_list_u8_t *buf,
                                          fastly_body_write_end_t end, uint32_t *ret) {
  return fastly_http_body_write(h, buf, end, ret);
}

fastly_error_t xqd_fastly_http_body_close(fastly_body_handle_t h) {
  return fastly_http_body_close(h);
}

fastly_error_t xqd_fastly_log_endpoint_get(xqd_world_string_t *name,
                                           fastly_log_endpoint_handle_t *ret) {
  return fastly_log_endpoint_get(name, ret);
}

fastly_error_t xqd_fastly_log_write(fastly_log_endpoint_handle_t h, xqd_world_string_t *msg) {
  return fastly_log_write(h, msg);
}

fastly_error_t xqd_fastly_http_req_body_downstream_get(fastly_request_t *ret) {
  return fastly_http_req_body_downstream_get(ret);
}

fastly_error_t xqd_fastly_http_req_cache_override_set(fastly_request_handle_t h,
                                                      fastly_http_cache_override_tag_t tag,
                                                      fastly_option_u32_t *ttl,
                                                      fastly_option_u32_t *stale_while_revalidate,
                                                      fastly_option_string_t *sk) {
  return fastly_http_req_cache_override_set(h, tag, ttl, stale_while_revalidate, sk);
}

fastly_error_t xqd_fastly_http_req_downstream_client_ip_addr(fastly_list_u8_t *ret) {
  return fastly_http_req_downstream_client_ip_addr(ret);
}

fastly_error_t xqd_fastly_http_req_downstream_client_h2_fingerprint(fastly_list_u8_t *ret) {
  return fastly_http_req_downstream_client_h2_fingerprint(ret);
}

fastly_error_t xqd_fastly_http_req_downstream_tls_cipher_openssl_name(xqd_world_string_t *ret) {
  return fastly_http_req_downstream_tls_cipher_openssl_name(ret);
}

fastly_error_t xqd_fastly_http_req_downstream_tls_protocol(xqd_world_string_t *ret) {
  return fastly_http_req_downstream_tls_protocol(ret);
}

fastly_error_t xqd_fastly_http_req_downstream_tls_client_hello(fastly_list_u8_t *ret) {
  return fastly_http_req_downstream_tls_client_hello(ret);
}

fastly_error_t xqd_fastly_http_req_downstream_tls_client_certificate(fastly_list_u8_t *ret) {
  return fastly_http_req_downstream_tls_client_certificate(ret);
}

fastly_error_t xqd_fastly_http_req_downstream_tls_client_cert_verify_result(void) {
  return fastly_http_req_downstream_tls_client_cert_verify_result();
}

fastly_error_t xqd_fastly_http_req_downstream_tls_ja3_md5(fastly_list_u8_t *ret) {
  return fastly_http_req_downstream_tls_ja3_md5(ret);
}

fastly_error_t xqd_fastly_http_req_new(fastly_request_handle_t *ret) {
  return fastly_http_req_new(ret);
}

fastly_error_t xqd_fastly_http_req_header_names_get(fastly_request_handle_t h,
                                                    fastly_cursor_t cursor,
                                                    fastly_tuple2_string_option_cursor_t *ret) {
  return fastly_http_req_header_names_get(h, cursor, ret);
}

fastly_error_t xqd_fastly_http_req_header_value_get(fastly_request_handle_t h,
                                                    xqd_world_string_t *name,
                                                    xqd_world_string_t *ret) {
  return fastly_http_req_header_value_get(h, name, ret);
}

fastly_error_t xqd_fastly_http_req_header_values_get(fastly_request_handle_t h,
                                                     xqd_world_string_t *name,
                                                     fastly_cursor_t cursor,
                                                     fastly_tuple2_string_option_cursor_t *ret) {
  return fastly_http_req_header_values_get(h, name, cursor, ret);
}

fastly_error_t xqd_fastly_http_req_header_values_set(fastly_request_handle_t h,
                                                     xqd_world_string_t *name,
                                                     fastly_list_string_t *values) {
  return fastly_http_req_header_values_set(h, name, values);
}

fastly_error_t xqd_fastly_http_req_header_insert(fastly_request_handle_t h,
                                                 xqd_world_string_t *name,
                                                 xqd_world_string_t *value) {
  return fastly_http_req_header_insert(h, name, value);
}

fastly_error_t xqd_fastly_http_req_header_append(fastly_request_handle_t h,
                                                 xqd_world_string_t *name,
                                                 xqd_world_string_t *value) {
  return fastly_http_req_header_append(h, name, value);
}

fastly_error_t xqd_fastly_http_req_header_remove(fastly_request_handle_t h,
                                                 xqd_world_string_t *name) {
  return fastly_http_req_header_remove(h, name);
}

fastly_error_t xqd_fastly_http_req_method_get(fastly_request_handle_t h, xqd_world_string_t *ret) {
  return fastly_http_req_method_get(h, ret);
}

fastly_error_t xqd_fastly_http_req_method_set(fastly_request_handle_t h,
                                              xqd_world_string_t *method) {
  return fastly_http_req_method_set(h, method);
}

fastly_error_t xqd_fastly_http_req_uri_get(fastly_request_handle_t h, xqd_world_string_t *ret) {
  return fastly_http_req_uri_get(h, ret);
}

fastly_error_t xqd_fastly_http_req_uri_set(fastly_request_handle_t h, xqd_world_string_t *uri) {
  return fastly_http_req_uri_set(h, uri);
}

fastly_error_t xqd_fastly_http_req_version_get(fastly_request_handle_t h,
                                               fastly_http_version_t *ret) {
  return fastly_http_req_version_get(h, ret);
}

fastly_error_t xqd_fastly_http_req_version_set(fastly_request_handle_t h,
                                               fastly_http_version_t version) {
  return fastly_http_req_version_set(h, version);
}

fastly_error_t xqd_fastly_http_req_send(fastly_request_handle_t h, fastly_body_handle_t b,
                                        xqd_world_string_t *backend, fastly_response_t *ret) {
  return fastly_http_req_send(h, b, backend, ret);
}

fastly_error_t xqd_fastly_http_req_send_async(fastly_request_handle_t h, fastly_body_handle_t b,
                                              xqd_world_string_t *backend,
                                              fastly_pending_request_handle_t *ret) {
  return fastly_http_req_send_async(h, b, backend, ret);
}

fastly_error_t xqd_fastly_http_req_send_async_streaming(fastly_request_handle_t h,
                                                        fastly_body_handle_t b,
                                                        xqd_world_string_t *backend,
                                                        fastly_pending_request_handle_t *ret) {
  return fastly_http_req_send_async_streaming(h, b, backend, ret);
}

fastly_error_t xqd_fastly_http_req_pending_req_poll(fastly_pending_request_handle_t h,
                                                    fastly_option_response_t *ret) {
  return fastly_http_req_pending_req_poll(h, ret);
}

fastly_error_t xqd_fastly_http_req_pending_req_wait(fastly_pending_request_handle_t h,
                                                    fastly_response_t *ret) {
  return fastly_http_req_pending_req_wait(h, ret);
}

fastly_error_t xqd_fastly_http_req_pending_req_select(fastly_list_pending_request_handle_t *h,
                                                      fastly_tuple2_u32_response_t *ret) {
  return fastly_http_req_pending_req_select(h, ret);
}

fastly_error_t xqd_fastly_http_req_key_is_valid(bool *ret) {
  return fastly_http_req_key_is_valid(ret);
}

fastly_error_t xqd_fastly_http_req_close(fastly_request_handle_t h) {
  return fastly_http_req_close(h);
}

fastly_error_t
xqd_fastly_http_req_auto_decompress_response_set(fastly_request_handle_t h,
                                                 fastly_content_encodings_t encodings) {
  return fastly_http_req_auto_decompress_response_set(h, encodings);
}

fastly_error_t xqd_fastly_http_req_upgrade_websocket(xqd_world_string_t *backend) {
  return fastly_http_req_upgrade_websocket(backend);
}

fastly_error_t xqd_fastly_http_req_redirect_to_websocket_proxy(xqd_world_string_t *backend) {
  return fastly_http_req_redirect_to_websocket_proxy(backend);
}

fastly_error_t xqd_fastly_http_req_redirect_to_grip_proxy(xqd_world_string_t *backend) {
  return fastly_http_req_redirect_to_grip_proxy(backend);
}

fastly_error_t xqd_fastly_http_req_framing_headers_mode_set(fastly_request_handle_t h,
                                                            fastly_framing_headers_mode_t mode) {
  return fastly_http_req_framing_headers_mode_set(h, mode);
}

fastly_error_t
xqd_fastly_http_req_register_dynamic_backend(xqd_world_string_t *prefix, xqd_world_string_t *target,
                                             fastly_dynamic_backend_config_t *config) {
  return fastly_http_req_register_dynamic_backend(prefix, target, config);
}

fastly_error_t xqd_fastly_http_resp_new(fastly_response_handle_t *ret) {
  return fastly_http_resp_new(ret);
}
fastly_error_t xqd_fastly_http_resp_header_names_get(fastly_response_handle_t h,
                                                     fastly_cursor_t cursor,
                                                     fastly_tuple2_string_option_cursor_t *ret) {
  return fastly_http_resp_header_names_get(h, cursor, ret);
}
fastly_error_t xqd_fastly_http_resp_header_value_get(fastly_response_handle_t h,
                                                     xqd_world_string_t *name,
                                                     xqd_world_string_t *ret) {
  return fastly_http_resp_header_value_get(h, name, ret);
}
fastly_error_t xqd_fastly_http_resp_header_values_get(fastly_response_handle_t h,
                                                      xqd_world_string_t *name,
                                                      fastly_cursor_t cursor,
                                                      fastly_tuple2_string_option_cursor_t *ret) {
  return fastly_http_resp_header_values_get(h, name, cursor, ret);
}
fastly_error_t xqd_fastly_http_resp_header_values_set(fastly_response_handle_t h,
                                                      xqd_world_string_t *name,
                                                      fastly_list_string_t *values) {
  return fastly_http_resp_header_values_set(h, name, values);
}
fastly_error_t xqd_fastly_http_resp_header_insert(fastly_response_handle_t h,
                                                  xqd_world_string_t *name,
                                                  xqd_world_string_t *value) {
  return fastly_http_resp_header_insert(h, name, value);
}
fastly_error_t xqd_fastly_http_resp_header_append(fastly_response_handle_t h,
                                                  xqd_world_string_t *name,
                                                  xqd_world_string_t *value) {
  return fastly_http_resp_header_append(h, name, value);
}
fastly_error_t xqd_fastly_http_resp_header_remove(fastly_response_handle_t h,
                                                  xqd_world_string_t *name) {
  return fastly_http_resp_header_remove(h, name);
}
fastly_error_t xqd_fastly_http_resp_version_get(fastly_response_handle_t h,
                                                fastly_http_version_t *ret) {
  return fastly_http_resp_version_get(h, ret);
}
fastly_error_t xqd_fastly_http_resp_version_set(fastly_response_handle_t h,
                                                fastly_http_version_t version) {
  return fastly_http_resp_version_set(h, version);
}
fastly_error_t xqd_fastly_http_resp_send_downstream(fastly_response_handle_t h,
                                                    fastly_body_handle_t b, bool streaming) {
  return fastly_http_resp_send_downstream(h, b, streaming);
}
fastly_error_t xqd_fastly_http_resp_status_get(fastly_response_handle_t h,
                                               fastly_http_status_t *ret) {
  return fastly_http_resp_status_get(h, ret);
}
fastly_error_t xqd_fastly_http_resp_status_set(fastly_response_handle_t h,
                                               fastly_http_status_t status) {
  return fastly_http_resp_status_set(h, status);
}
fastly_error_t xqd_fastly_http_resp_close(fastly_response_handle_t h) {
  return fastly_http_resp_close(h);
}
fastly_error_t xqd_fastly_http_resp_framing_headers_mode_set(fastly_response_handle_t h,
                                                             fastly_framing_headers_mode_t mode) {
  return fastly_http_resp_framing_headers_mode_set(h, mode);
}
fastly_error_t xqd_fastly_dictionary_open(xqd_world_string_t *name,
                                          fastly_dictionary_handle_t *ret) {
  return fastly_dictionary_open(name, ret);
}
fastly_error_t xqd_fastly_dictionary_get(fastly_dictionary_handle_t h, xqd_world_string_t *key,
                                         xqd_world_string_t *ret) {
  return fastly_dictionary_get(h, key, ret);
}
fastly_error_t xqd_fastly_geo_lookup(fastly_list_u8_t *addr_octets, xqd_world_string_t *ret) {
  return fastly_geo_lookup(addr_octets, ret);
}
fastly_error_t xqd_fastly_kv_open(xqd_world_string_t *name, fastly_kv_store_handle_t *ret) {
  return fastly_kv_open(name, ret);
}
fastly_error_t xqd_fastly_kv_lookup(fastly_kv_store_handle_t store, fastly_list_u8_t *key,
                                    fastly_body_handle_t *ret) {
  return fastly_kv_lookup(store, key, ret);
}
fastly_error_t xqd_fastly_kv_insert(fastly_kv_store_handle_t store, fastly_list_u8_t *key,
                                    fastly_body_handle_t body_handle, uint32_t max_age, bool *ret) {
  return fastly_kv_insert(store, key, body_handle, max_age, ret);
}
fastly_error_t xqd_fastly_object_store_open(xqd_world_string_t *name,
                                            fastly_object_store_handle_t *ret) {
  return fastly_object_store_open(name, ret);
}
fastly_error_t xqd_fastly_object_store_lookup(fastly_object_store_handle_t store,
                                              xqd_world_string_t *key, fastly_body_handle_t *ret) {
  return fastly_object_store_lookup(store, key, ret);
}
fastly_error_t xqd_fastly_object_store_lookup_as_fd(fastly_object_store_handle_t store,
                                                    xqd_world_string_t *key, fastly_fd_t *ret) {
  return fastly_object_store_lookup_as_fd(store, key, ret);
}
fastly_error_t xqd_fastly_object_store_insert(fastly_object_store_handle_t store,
                                              xqd_world_string_t *key,
                                              fastly_body_handle_t body_handle) {
  return fastly_object_store_insert(store, key, body_handle);
}
fastly_error_t xqd_fastly_secret_store_open(xqd_world_string_t *name,
                                            fastly_secret_store_handle_t *ret) {
  return fastly_secret_store_open(name, ret);
}
fastly_error_t xqd_fastly_secret_store_get(fastly_secret_store_handle_t store,
                                           xqd_world_string_t *key, fastly_secret_handle_t *ret) {
  return fastly_secret_store_get(store, key, ret);
}
fastly_error_t xqd_fastly_secret_store_plaintext(fastly_secret_handle_t secret,
                                                 xqd_world_string_t *ret) {
  return fastly_secret_store_plaintext(secret, ret);
}
fastly_error_t xqd_fastly_backend_is_healthy(xqd_world_string_t *backend,
                                             fastly_backend_health_t *ret) {
  return fastly_backend_is_healthy(backend, ret);
}
fastly_error_t xqd_fastly_async_io_select(fastly_list_async_handle_t *hs, uint32_t timeout_ms,
                                          uint32_t *ret) {
  return fastly_async_io_select(hs, timeout_ms, ret);
}
fastly_error_t xqd_fastly_async_io_is_ready(fastly_async_handle_t handle, bool *ret) {
  return fastly_async_io_is_ready(handle, ret);
}
fastly_error_t xqd_fastly_purge_surrogate_key(xqd_world_string_t *surrogate_key, bool soft_purge,
                                              fastly_purge_result_t *ret) {
  return fastly_purge_surrogate_key(surrogate_key, soft_purge, ret);
}

#endif
