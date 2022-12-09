#ifndef xqd_world_adapter_h
#define xqd_world_adapter_h

#include "../host_call.h"
#include "../xqd.h"
#include "js/JSON.h"
#include "xqd_world.h"

static JSContext *CONTEXT = nullptr;

// TODO: remove these once the warnings are fixed
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-offsetof"

#include "jsapi.h"

#pragma clang diagnostic pop

bool xqd_fastly_abi_init(uint64_t abi_version, fastly_error_t *err);
bool xqd_fastly_uap_parse(xqd_world_string_t *user_agent, fastly_user_agent_t *ret,
                          fastly_error_t *err);
bool xqd_fastly_http_body_new(fastly_body_handle_t *ret, fastly_error_t *err);
bool xqd_fastly_http_body_append(fastly_body_handle_t dest, fastly_body_handle_t src,
                                 fastly_error_t *err);
bool xqd_fastly_http_body_read(fastly_body_handle_t h, uint32_t chunk_size, fastly_list_u8_t *ret,
                               fastly_error_t *err);
bool xqd_fastly_http_body_write(fastly_body_handle_t h, fastly_list_u8_t *buf,
                                fastly_body_write_end_t end, uint32_t *ret, fastly_error_t *err);
bool xqd_fastly_http_body_close(fastly_body_handle_t h, fastly_error_t *err);
bool xqd_fastly_log_endpoint_get(xqd_world_string_t *name, fastly_log_endpoint_handle_t *ret,
                                 fastly_error_t *err);
bool xqd_fastly_log_write(fastly_log_endpoint_handle_t h, xqd_world_string_t *msg,
                          fastly_error_t *err);
bool xqd_fastly_http_req_body_downstream_get(fastly_request_t *ret, fastly_error_t *err);
bool xqd_fastly_http_req_cache_override_set(fastly_request_handle_t h,
                                            fastly_http_cache_override_tag_t tag,
                                            fastly_option_u32_t *ttl,
                                            fastly_option_u32_t *stale_while_revalidate,
                                            fastly_option_string_t *sk, fastly_error_t *err);
bool xqd_fastly_http_req_downstream_client_ip_addr(fastly_list_u8_t *ret, fastly_error_t *err);
bool xqd_fastly_http_req_downstream_client_h2_fingerprint(fastly_list_u8_t *ret,
                                                          fastly_error_t *err);
bool xqd_fastly_http_req_downstream_tls_cipher_openssl_name(xqd_world_string_t *ret,
                                                            fastly_error_t *err);
bool xqd_fastly_http_req_downstream_tls_protocol(xqd_world_string_t *ret, fastly_error_t *err);
bool xqd_fastly_http_req_downstream_tls_client_hello(fastly_list_u8_t *ret, fastly_error_t *err);
bool xqd_fastly_http_req_downstream_tls_client_certificate(fastly_list_u8_t *ret,
                                                           fastly_error_t *err);
bool xqd_fastly_http_req_downstream_tls_client_cert_verify_result(fastly_error_t *err);
bool xqd_fastly_http_req_downstream_tls_ja3_md5(fastly_list_u8_t *ret, fastly_error_t *err);
bool xqd_fastly_http_req_new(fastly_request_handle_t *ret, fastly_error_t *err);
bool xqd_fastly_http_req_header_names_get(fastly_request_handle_t h, fastly_list_string_t *ret,
                                          fastly_error_t *err);
bool xqd_fastly_http_req_header_value_get(fastly_request_handle_t h, xqd_world_string_t *name,
                                          fastly_option_string_t *ret, fastly_error_t *err);
bool xqd_fastly_http_req_header_values_get(fastly_request_handle_t h, xqd_world_string_t *name,
                                           fastly_option_list_string_t *ret, fastly_error_t *err);
bool xqd_fastly_http_req_header_values_set(fastly_request_handle_t h, xqd_world_string_t *name,
                                           fastly_list_string_t *values, fastly_error_t *err);
bool xqd_fastly_http_req_header_insert(fastly_request_handle_t h, xqd_world_string_t *name,
                                       xqd_world_string_t *value, fastly_error_t *err);
bool xqd_fastly_http_req_header_append(fastly_request_handle_t h, xqd_world_string_t *name,
                                       xqd_world_string_t *value, fastly_error_t *err);
bool xqd_fastly_http_req_header_remove(fastly_request_handle_t h, xqd_world_string_t *name,
                                       fastly_error_t *err);
bool xqd_fastly_http_req_method_get(fastly_request_handle_t h, xqd_world_string_t *ret,
                                    fastly_error_t *err);
bool xqd_fastly_http_req_method_set(fastly_request_handle_t h, xqd_world_string_t *method,
                                    fastly_error_t *err);
bool xqd_fastly_http_req_uri_get(fastly_request_handle_t h, xqd_world_string_t *ret,
                                 fastly_error_t *err);
bool xqd_fastly_http_req_uri_set(fastly_request_handle_t h, xqd_world_string_t *uri,
                                 fastly_error_t *err);
bool xqd_fastly_http_req_version_get(fastly_request_handle_t h, fastly_http_version_t *ret,
                                     fastly_error_t *err);
bool xqd_fastly_http_req_version_set(fastly_request_handle_t h, fastly_http_version_t version,
                                     fastly_error_t *err);
bool xqd_fastly_http_req_send(fastly_request_handle_t h, fastly_body_handle_t b,
                              xqd_world_string_t *backend, fastly_response_t *ret,
                              fastly_error_t *err);
bool xqd_fastly_http_req_send_async(fastly_request_handle_t h, fastly_body_handle_t b,
                                    xqd_world_string_t *backend,
                                    fastly_pending_request_handle_t *ret, fastly_error_t *err);
bool xqd_fastly_http_req_send_async_streaming(fastly_request_handle_t h, fastly_body_handle_t b,
                                              xqd_world_string_t *backend,
                                              fastly_pending_request_handle_t *ret,
                                              fastly_error_t *err);
bool xqd_fastly_http_req_pending_req_poll(fastly_pending_request_handle_t h,
                                          fastly_option_response_t *ret, fastly_error_t *err);
bool xqd_fastly_http_req_pending_req_wait(fastly_pending_request_handle_t h, fastly_response_t *ret,
                                          fastly_error_t *err);
bool xqd_fastly_http_req_pending_req_select(fastly_list_pending_request_handle_t *h,
                                            fastly_tuple2_u32_response_t *ret, fastly_error_t *err);
bool xqd_fastly_http_req_key_is_valid(bool *ret, fastly_error_t *err);
bool xqd_fastly_http_req_close(fastly_request_handle_t h, fastly_error_t *err);
bool xqd_fastly_http_req_auto_decompress_response_set(fastly_request_handle_t h,
                                                      fastly_content_encodings_t encodings,
                                                      fastly_error_t *err);
bool xqd_fastly_http_req_upgrade_websocket(xqd_world_string_t *backend, fastly_error_t *err);
bool xqd_fastly_http_req_redirect_to_websocket_proxy(xqd_world_string_t *backend,
                                                     fastly_error_t *err);
bool xqd_fastly_http_req_redirect_to_grip_proxy(xqd_world_string_t *backend, fastly_error_t *err);
bool xqd_fastly_http_req_framing_headers_mode_set(fastly_request_handle_t h,
                                                  fastly_framing_headers_mode_t mode,
                                                  fastly_error_t *err);
bool xqd_fastly_http_req_register_dynamic_backend(xqd_world_string_t *prefix,
                                                  xqd_world_string_t *target,
                                                  fastly_dynamic_backend_config_t *config,
                                                  fastly_error_t *err);
bool xqd_fastly_http_resp_new(fastly_response_handle_t *ret, fastly_error_t *err);
bool xqd_fastly_http_resp_header_names_get(fastly_response_handle_t h, fastly_list_string_t *ret,
                                           fastly_error_t *err);
bool xqd_fastly_http_resp_header_value_get(fastly_response_handle_t h, xqd_world_string_t *name,
                                           fastly_option_string_t *ret, fastly_error_t *err);
bool xqd_fastly_http_resp_header_values_get(fastly_response_handle_t h, xqd_world_string_t *name,
                                            fastly_option_list_string_t *ret, fastly_error_t *err);
bool xqd_fastly_http_resp_header_values_set(fastly_response_handle_t h, xqd_world_string_t *name,
                                            fastly_list_string_t *values, fastly_error_t *err);
bool xqd_fastly_http_resp_header_insert(fastly_response_handle_t h, xqd_world_string_t *name,
                                        xqd_world_string_t *value, fastly_error_t *err);
bool xqd_fastly_http_resp_header_append(fastly_response_handle_t h, xqd_world_string_t *name,
                                        xqd_world_string_t *value, fastly_error_t *err);
bool xqd_fastly_http_resp_header_remove(fastly_response_handle_t h, xqd_world_string_t *name,
                                        fastly_error_t *err);
bool xqd_fastly_http_resp_version_get(fastly_response_handle_t h, fastly_http_version_t *ret,
                                      fastly_error_t *err);
bool xqd_fastly_http_resp_version_set(fastly_response_handle_t h, fastly_http_version_t version,
                                      fastly_error_t *err);
bool xqd_fastly_http_resp_send_downstream(fastly_response_handle_t h, fastly_body_handle_t b,
                                          bool streaming, fastly_error_t *err);
bool xqd_fastly_http_resp_status_get(fastly_response_handle_t h, fastly_http_status_t *ret,
                                     fastly_error_t *err);
bool xqd_fastly_http_resp_status_set(fastly_response_handle_t h, fastly_http_status_t status,
                                     fastly_error_t *err);
bool xqd_fastly_http_resp_close(fastly_response_handle_t h, fastly_error_t *err);
bool xqd_fastly_http_resp_framing_headers_mode_set(fastly_response_handle_t h,
                                                   fastly_framing_headers_mode_t mode,
                                                   fastly_error_t *err);
bool xqd_fastly_dictionary_open(xqd_world_string_t *name, fastly_dictionary_handle_t *ret,
                                fastly_error_t *err);
bool xqd_fastly_dictionary_get(fastly_dictionary_handle_t h, xqd_world_string_t *key,
                               fastly_option_string_t *ret, fastly_error_t *err);
bool xqd_fastly_geo_lookup(fastly_list_u8_t *addr_octets, xqd_world_string_t *ret,
                           fastly_error_t *err);
bool xqd_fastly_object_store_open(xqd_world_string_t *name, fastly_object_store_handle_t *ret,
                                  fastly_error_t *err);
bool xqd_fastly_object_store_lookup(fastly_object_store_handle_t store, xqd_world_string_t *key,
                                    fastly_option_body_handle_t *ret, fastly_error_t *err);
bool xqd_fastly_object_store_lookup_as_fd(fastly_object_store_handle_t store,
                                          xqd_world_string_t *key, fastly_option_fd_t *ret,
                                          fastly_error_t *err);
bool xqd_fastly_object_store_insert(fastly_object_store_handle_t store, xqd_world_string_t *key,
                                    fastly_body_handle_t body_handle, fastly_error_t *err);
bool xqd_fastly_secret_store_open(xqd_world_string_t *name, fastly_secret_store_handle_t *ret,
                                  fastly_error_t *err);
bool xqd_fastly_secret_store_get(fastly_secret_store_handle_t store, xqd_world_string_t *key,
                                 fastly_option_secret_handle_t *ret, fastly_error_t *err);
bool xqd_fastly_secret_store_plaintext(fastly_secret_handle_t secret, fastly_option_string_t *ret,
                                       fastly_error_t *err);
bool xqd_fastly_async_io_select(fastly_list_async_handle_t *hs, uint32_t timeout_ms,
                                fastly_option_u32_t *ret, fastly_error_t *err);
bool xqd_fastly_async_io_is_ready(fastly_async_handle_t handle, bool *ret, fastly_error_t *err);
bool xqd_fastly_purge_surrogate_key(xqd_world_string_t *surrogate_key, bool soft_purge,
                                    fastly_purge_result_t *ret, fastly_error_t *err);

#endif
