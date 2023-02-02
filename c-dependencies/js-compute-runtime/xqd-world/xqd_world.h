#ifndef __BINDINGS_XQD_WORLD_H
#define __BINDINGS_XQD_WORLD_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  char *ptr;
  size_t len;
} xqd_world_string_t;

typedef struct {
  xqd_world_string_t family;
  xqd_world_string_t major;
  xqd_world_string_t minor;
  xqd_world_string_t patch;
} fastly_user_agent_t;

typedef uint8_t fastly_tls_version_t;

#define FASTLY_TLS_VERSION_TLS1 0
#define FASTLY_TLS_VERSION_TLS11 1
#define FASTLY_TLS_VERSION_TLS12 2
#define FASTLY_TLS_VERSION_TLS13 3

typedef uint32_t fastly_secret_store_handle_t;

typedef uint32_t fastly_secret_handle_t;

typedef uint32_t fastly_response_handle_t;

typedef uint32_t fastly_request_handle_t;

typedef struct {
  xqd_world_string_t id;
} fastly_purge_result_t;

typedef uint32_t fastly_pending_request_handle_t;

typedef uint32_t fastly_object_store_handle_t;

typedef uint32_t fastly_log_endpoint_handle_t;

typedef uint8_t fastly_http_version_t;

#define FASTLY_HTTP_VERSION_HTTP09 0
#define FASTLY_HTTP_VERSION_HTTP10 1
#define FASTLY_HTTP_VERSION_HTTP11 2
#define FASTLY_HTTP_VERSION_H2 3
#define FASTLY_HTTP_VERSION_H3 4

typedef uint16_t fastly_http_status_t;

typedef uint8_t fastly_http_cache_override_tag_t;

#define FASTLY_HTTP_CACHE_OVERRIDE_TAG_PASS (1 << 0)
#define FASTLY_HTTP_CACHE_OVERRIDE_TAG_TTL (1 << 1)
#define FASTLY_HTTP_CACHE_OVERRIDE_TAG_STALE_WHILE_REVALIDATE (1 << 2)
#define FASTLY_HTTP_CACHE_OVERRIDE_TAG_PCI (1 << 3)

typedef struct {
  bool is_some;
  xqd_world_string_t val;
} fastly_option_string_t;

typedef struct {
  bool is_some;
  uint32_t val;
} fastly_option_u32_t;

typedef struct {
  bool is_some;
  float val;
} fastly_option_float32_t;

typedef struct {
  fastly_option_string_t as_name;
  fastly_option_u32_t as_number;
  fastly_option_u32_t area_code;
  fastly_option_string_t city;
  fastly_option_string_t conn_speed;
  fastly_option_string_t conn_type;
  fastly_option_string_t continent;
  fastly_option_string_t country_code;
  fastly_option_string_t country_code3;
  fastly_option_string_t country_name;
  fastly_option_string_t gmt_offset;
  fastly_option_float32_t latitude;
  fastly_option_float32_t longitude;
  fastly_option_u32_t metro_code;
  fastly_option_string_t postal_code;
  fastly_option_string_t proxy_description;
  fastly_option_string_t proxy_type;
  fastly_option_string_t region;
  fastly_option_u32_t utc_offset;
} fastly_geo_data_t;

// Adjust how this requests's framing headers are determined.
typedef uint8_t fastly_framing_headers_mode_t;

#define FASTLY_FRAMING_HEADERS_MODE_AUTOMATIC 0
#define FASTLY_FRAMING_HEADERS_MODE_MANUALLY_FROM_HEADERS 1

typedef uint32_t fastly_fd_t;

typedef uint8_t fastly_error_t;

#define FASTLY_ERROR_UNKNOWN_ERROR 0
#define FASTLY_ERROR_GENERIC_ERROR 1
#define FASTLY_ERROR_INVALID_ARGUMENT 2
#define FASTLY_ERROR_BAD_HANDLE 3
#define FASTLY_ERROR_BUFFER_LEN 4
#define FASTLY_ERROR_UNSUPPORTED 5
#define FASTLY_ERROR_BAD_ALIGN 6
#define FASTLY_ERROR_HTTP_INVALID 7
#define FASTLY_ERROR_HTTP_USER 8
#define FASTLY_ERROR_HTTP_INCOMPLETE 9
#define FASTLY_ERROR_OPTIONAL_NONE 10
#define FASTLY_ERROR_HTTP_HEAD_TOO_LARGE 11
#define FASTLY_ERROR_HTTP_INVALID_STATUS 12
#define FASTLY_ERROR_LIMIT_EXCEEDED 13

typedef struct {
  bool is_some;
  bool val;
} fastly_option_bool_t;

typedef struct {
  bool is_some;
  fastly_tls_version_t val;
} fastly_option_tls_version_t;

// Create a backend for later use
typedef struct {
  fastly_option_string_t host_override;
  fastly_option_u32_t connect_timeout;
  fastly_option_u32_t first_byte_timeout;
  fastly_option_u32_t between_bytes_timeout;
  fastly_option_bool_t use_ssl;
  fastly_option_tls_version_t ssl_min_version;
  fastly_option_tls_version_t ssl_max_version;
  fastly_option_string_t cert_hostname;
  fastly_option_string_t ca_cert;
  fastly_option_string_t ciphers;
  fastly_option_string_t sni_hostname;
} fastly_dynamic_backend_config_t;

typedef uint32_t fastly_dictionary_handle_t;

typedef uint8_t fastly_content_encodings_t;

#define FASTLY_CONTENT_ENCODINGS_GZIP 0

typedef uint8_t fastly_body_write_end_t;

#define FASTLY_BODY_WRITE_END_BACK 0
#define FASTLY_BODY_WRITE_END_FRONT 1

typedef uint32_t fastly_body_handle_t;

typedef struct {
  fastly_response_handle_t f0;
  fastly_body_handle_t f1;
} fastly_response_t;

typedef struct {
  fastly_request_handle_t f0;
  fastly_body_handle_t f1;
} fastly_request_t;

// A handle to an object supporting generic async operations.
// Can be either a `BodyHandle` or a `PendingRequestHandle`.
//
// Each async item has an associated I/O action:
//
// * Pending requests: awaiting the response headers / `Response` object
// * Normal bodies: reading bytes from the body
// * Streaming bodies: writing bytes to the body
//
// For writing bytes, note that there is a large host-side buffer that bytes can eagerly be written
// into, even before the origin itself consumes that data.
typedef uint32_t fastly_async_handle_t;

typedef struct {
  uint8_t *ptr;
  size_t len;
} fastly_list_u8_t;

typedef struct {
  xqd_world_string_t *ptr;
  size_t len;
} fastly_list_string_t;

typedef struct {
  bool is_some;
  fastly_list_string_t val;
} fastly_option_list_string_t;

typedef struct {
  bool is_some;
  fastly_response_t val;
} fastly_option_response_t;

typedef struct {
  fastly_pending_request_handle_t *ptr;
  size_t len;
} fastly_list_pending_request_handle_t;

typedef struct {
  uint32_t f0;
  fastly_response_t f1;
} fastly_tuple2_u32_response_t;

typedef struct {
  bool is_some;
  fastly_body_handle_t val;
} fastly_option_body_handle_t;

typedef struct {
  bool is_some;
  fastly_fd_t val;
} fastly_option_fd_t;

typedef struct {
  bool is_some;
  fastly_secret_handle_t val;
} fastly_option_secret_handle_t;

typedef struct {
  fastly_async_handle_t *ptr;
  size_t len;
} fastly_list_async_handle_t;

// Imported Functions from `fastly`
bool fastly_abi_init(uint64_t abi_version, fastly_error_t *err);
bool fastly_uap_parse(xqd_world_string_t *user_agent, fastly_user_agent_t *ret,
                      fastly_error_t *err);
bool fastly_http_body_new(fastly_body_handle_t *ret, fastly_error_t *err);
bool fastly_http_body_append(fastly_body_handle_t dest, fastly_body_handle_t src,
                             fastly_error_t *err);
bool fastly_http_body_read(fastly_body_handle_t h, uint32_t chunk_size, fastly_list_u8_t *ret,
                           fastly_error_t *err);
bool fastly_http_body_write(fastly_body_handle_t h, fastly_list_u8_t *buf,
                            fastly_body_write_end_t end, uint32_t *ret, fastly_error_t *err);
bool fastly_http_body_close(fastly_body_handle_t h, fastly_error_t *err);
bool fastly_log_endpoint_get(xqd_world_string_t *name, fastly_log_endpoint_handle_t *ret,
                             fastly_error_t *err);
bool fastly_log_write(fastly_log_endpoint_handle_t h, xqd_world_string_t *msg, fastly_error_t *err);
bool fastly_http_req_body_downstream_get(fastly_request_t *ret, fastly_error_t *err);
bool fastly_http_req_cache_override_set(fastly_request_handle_t h,
                                        fastly_http_cache_override_tag_t tag, uint32_t *maybe_ttl,
                                        uint32_t *maybe_stale_while_revalidate,
                                        xqd_world_string_t *maybe_sk, fastly_error_t *err);
bool fastly_http_req_downstream_client_ip_addr(fastly_list_u8_t *ret, fastly_error_t *err);
bool fastly_http_req_downstream_client_h2_fingerprint(fastly_list_u8_t *ret, fastly_error_t *err);
bool fastly_http_req_downstream_tls_cipher_openssl_name(xqd_world_string_t *ret,
                                                        fastly_error_t *err);
bool fastly_http_req_downstream_tls_protocol(xqd_world_string_t *ret, fastly_error_t *err);
bool fastly_http_req_downstream_tls_client_hello(fastly_list_u8_t *ret, fastly_error_t *err);
bool fastly_http_req_downstream_tls_client_certificate(fastly_list_u8_t *ret, fastly_error_t *err);
bool fastly_http_req_downstream_tls_client_cert_verify_result(fastly_error_t *err);
bool fastly_http_req_downstream_tls_ja3_md5(fastly_list_u8_t *ret, fastly_error_t *err);
bool fastly_http_req_new(fastly_request_handle_t *ret, fastly_error_t *err);
bool fastly_http_req_header_names_get(fastly_request_handle_t h, fastly_list_string_t *ret,
                                      fastly_error_t *err);
bool fastly_http_req_header_value_get(fastly_request_handle_t h, xqd_world_string_t *name,
                                      fastly_option_string_t *ret, fastly_error_t *err);
bool fastly_http_req_header_values_get(fastly_request_handle_t h, xqd_world_string_t *name,
                                       fastly_option_list_string_t *ret, fastly_error_t *err);
bool fastly_http_req_header_values_set(fastly_request_handle_t h, xqd_world_string_t *name,
                                       fastly_list_string_t *values, fastly_error_t *err);
bool fastly_http_req_header_insert(fastly_request_handle_t h, xqd_world_string_t *name,
                                   xqd_world_string_t *value, fastly_error_t *err);
bool fastly_http_req_header_append(fastly_request_handle_t h, xqd_world_string_t *name,
                                   xqd_world_string_t *value, fastly_error_t *err);
bool fastly_http_req_header_remove(fastly_request_handle_t h, xqd_world_string_t *name,
                                   fastly_error_t *err);
bool fastly_http_req_method_get(fastly_request_handle_t h, xqd_world_string_t *ret,
                                fastly_error_t *err);
bool fastly_http_req_method_set(fastly_request_handle_t h, xqd_world_string_t *method,
                                fastly_error_t *err);
bool fastly_http_req_uri_get(fastly_request_handle_t h, xqd_world_string_t *ret,
                             fastly_error_t *err);
bool fastly_http_req_uri_set(fastly_request_handle_t h, xqd_world_string_t *uri,
                             fastly_error_t *err);
bool fastly_http_req_version_get(fastly_request_handle_t h, fastly_http_version_t *ret,
                                 fastly_error_t *err);
bool fastly_http_req_version_set(fastly_request_handle_t h, fastly_http_version_t version,
                                 fastly_error_t *err);
bool fastly_http_req_send(fastly_request_handle_t h, fastly_body_handle_t b,
                          xqd_world_string_t *backend, fastly_response_t *ret, fastly_error_t *err);
bool fastly_http_req_send_async(fastly_request_handle_t h, fastly_body_handle_t b,
                                xqd_world_string_t *backend, fastly_pending_request_handle_t *ret,
                                fastly_error_t *err);
bool fastly_http_req_send_async_streaming(fastly_request_handle_t h, fastly_body_handle_t b,
                                          xqd_world_string_t *backend,
                                          fastly_pending_request_handle_t *ret,
                                          fastly_error_t *err);
bool fastly_http_req_pending_req_poll(fastly_pending_request_handle_t h,
                                      fastly_option_response_t *ret, fastly_error_t *err);
bool fastly_http_req_pending_req_wait(fastly_pending_request_handle_t h, fastly_response_t *ret,
                                      fastly_error_t *err);
bool fastly_http_req_pending_req_select(fastly_list_pending_request_handle_t *h,
                                        fastly_tuple2_u32_response_t *ret, fastly_error_t *err);
bool fastly_http_req_key_is_valid(bool *ret, fastly_error_t *err);
bool fastly_http_req_close(fastly_request_handle_t h, fastly_error_t *err);
bool fastly_http_req_auto_decompress_response_set(fastly_request_handle_t h,
                                                  fastly_content_encodings_t encodings,
                                                  fastly_error_t *err);
bool fastly_http_req_upgrade_websocket(xqd_world_string_t *backend, fastly_error_t *err);
bool fastly_http_req_redirect_to_websocket_proxy(xqd_world_string_t *backend, fastly_error_t *err);
bool fastly_http_req_redirect_to_grip_proxy(xqd_world_string_t *backend, fastly_error_t *err);
bool fastly_http_req_framing_headers_mode_set(fastly_request_handle_t h,
                                              fastly_framing_headers_mode_t mode,
                                              fastly_error_t *err);
bool fastly_http_req_register_dynamic_backend(xqd_world_string_t *prefix,
                                              xqd_world_string_t *target,
                                              fastly_dynamic_backend_config_t *config,
                                              fastly_error_t *err);
bool fastly_http_resp_new(fastly_response_handle_t *ret, fastly_error_t *err);
bool fastly_http_resp_header_names_get(fastly_response_handle_t h, fastly_list_string_t *ret,
                                       fastly_error_t *err);
bool fastly_http_resp_header_value_get(fastly_response_handle_t h, xqd_world_string_t *name,
                                       fastly_option_string_t *ret, fastly_error_t *err);
bool fastly_http_resp_header_values_get(fastly_response_handle_t h, xqd_world_string_t *name,
                                        fastly_option_list_string_t *ret, fastly_error_t *err);
bool fastly_http_resp_header_values_set(fastly_response_handle_t h, xqd_world_string_t *name,
                                        fastly_list_string_t *values, fastly_error_t *err);
bool fastly_http_resp_header_insert(fastly_response_handle_t h, xqd_world_string_t *name,
                                    xqd_world_string_t *value, fastly_error_t *err);
bool fastly_http_resp_header_append(fastly_response_handle_t h, xqd_world_string_t *name,
                                    xqd_world_string_t *value, fastly_error_t *err);
bool fastly_http_resp_header_remove(fastly_response_handle_t h, xqd_world_string_t *name,
                                    fastly_error_t *err);
bool fastly_http_resp_version_get(fastly_response_handle_t h, fastly_http_version_t *ret,
                                  fastly_error_t *err);
bool fastly_http_resp_version_set(fastly_response_handle_t h, fastly_http_version_t version,
                                  fastly_error_t *err);
bool fastly_http_resp_send_downstream(fastly_response_handle_t h, fastly_body_handle_t b,
                                      bool streaming, fastly_error_t *err);
bool fastly_http_resp_status_get(fastly_response_handle_t h, fastly_http_status_t *ret,
                                 fastly_error_t *err);
bool fastly_http_resp_status_set(fastly_response_handle_t h, fastly_http_status_t status,
                                 fastly_error_t *err);
bool fastly_http_resp_close(fastly_response_handle_t h, fastly_error_t *err);
bool fastly_http_resp_framing_headers_mode_set(fastly_response_handle_t h,
                                               fastly_framing_headers_mode_t mode,
                                               fastly_error_t *err);
bool fastly_dictionary_open(xqd_world_string_t *name, fastly_dictionary_handle_t *ret,
                            fastly_error_t *err);
bool fastly_dictionary_get(fastly_dictionary_handle_t h, xqd_world_string_t *key,
                           fastly_option_string_t *ret, fastly_error_t *err);
bool fastly_geo_lookup(fastly_list_u8_t *addr_octets, xqd_world_string_t *ret, fastly_error_t *err);
bool fastly_object_store_open(xqd_world_string_t *name, fastly_object_store_handle_t *ret,
                              fastly_error_t *err);
bool fastly_object_store_lookup(fastly_object_store_handle_t store, xqd_world_string_t *key,
                                fastly_option_body_handle_t *ret, fastly_error_t *err);
bool fastly_object_store_lookup_as_fd(fastly_object_store_handle_t store, xqd_world_string_t *key,
                                      fastly_option_fd_t *ret, fastly_error_t *err);
bool fastly_object_store_insert(fastly_object_store_handle_t store, xqd_world_string_t *key,
                                fastly_body_handle_t body_handle, fastly_error_t *err);
bool fastly_secret_store_open(xqd_world_string_t *name, fastly_secret_store_handle_t *ret,
                              fastly_error_t *err);
bool fastly_secret_store_get(fastly_secret_store_handle_t store, xqd_world_string_t *key,
                             fastly_option_secret_handle_t *ret, fastly_error_t *err);
bool fastly_secret_store_plaintext(fastly_secret_handle_t secret, fastly_option_string_t *ret,
                                   fastly_error_t *err);
bool fastly_async_io_select(fastly_list_async_handle_t *hs, uint32_t timeout_ms,
                            fastly_option_u32_t *ret, fastly_error_t *err);
bool fastly_async_io_is_ready(fastly_async_handle_t handle, bool *ret, fastly_error_t *err);
bool fastly_purge_surrogate_key(xqd_world_string_t *surrogate_key, bool soft_purge,
                                fastly_purge_result_t *ret, fastly_error_t *err);

// Exported Functions from `fastly-runtime`
void fastly_runtime_serve_sync(void);

#ifdef __cplusplus
}
#endif
#endif
