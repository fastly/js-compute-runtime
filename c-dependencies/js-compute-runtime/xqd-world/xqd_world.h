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
#define FASTLY_ERROR_MISSING_OPTIONAL 10
#define FASTLY_ERROR_HTTP_HEAD_TOO_LARGE 11
#define FASTLY_ERROR_HTTP_INVALID_STATUS 12
#define FASTLY_RESULT_ERROR_OK 255

typedef struct {
  xqd_world_string_t family;
  xqd_world_string_t major;
  xqd_world_string_t minor;
  xqd_world_string_t patch;
} fastly_user_agent_t;

typedef uint32_t fastly_body_handle_t;

typedef uint8_t fastly_body_write_end_t;

#define FASTLY_BODY_WRITE_END_BACK 0
#define FASTLY_BODY_WRITE_END_FRONT 1
#define FASTLY_RESULT_BODY_WRITE_END_OK 255

typedef uint32_t fastly_log_endpoint_handle_t;

typedef uint32_t fastly_request_handle_t;

typedef uint32_t fastly_pending_request_handle_t;

typedef uint32_t fastly_response_handle_t;

typedef struct {
  fastly_request_handle_t f0;
  fastly_body_handle_t f1;
} fastly_request_t;

typedef struct {
  fastly_response_handle_t f0;
  fastly_body_handle_t f1;
} fastly_response_t;

typedef uint8_t fastly_http_cache_override_tag_t;

#define FASTLY_HTTP_CACHE_OVERRIDE_TAG_NONE (1 << 0)
#define FASTLY_HTTP_CACHE_OVERRIDE_TAG_PASS (1 << 1)
#define FASTLY_HTTP_CACHE_OVERRIDE_TAG_TTL (1 << 2)
#define FASTLY_HTTP_CACHE_OVERRIDE_TAG_STALE_WHILE_REVALIDATE (1 << 3)
#define FASTLY_HTTP_CACHE_OVERRIDE_TAG_PCI (1 << 4)

typedef uint8_t fastly_http_version_t;

#define FASTLY_HTTP_VERSION_HTTP09 0
#define FASTLY_HTTP_VERSION_HTTP10 1
#define FASTLY_HTTP_VERSION_HTTP11 2
#define FASTLY_HTTP_VERSION_H2 3
#define FASTLY_HTTP_VERSION_H3 4
#define FASTLY_RESULT_HTTP_VERSION_OK 255

typedef uint8_t fastly_content_encodings_t;

#define FASTLY_CONTENT_ENCODINGS_GZIP 0
#define FASTLY_RESULT_CONTENT_ENCODINGS_OK 255
// Adjust how this requests's framing headers are determined.

typedef uint8_t fastly_framing_headers_mode_t;

#define FASTLY_FRAMING_HEADERS_MODE_AUTOMATIC 0
#define FASTLY_FRAMING_HEADERS_MODE_MANUALLY_FROM_HEADERS 1
#define FASTLY_RESULT_FRAMING_HEADERS_MODE_OK 255

typedef uint8_t fastly_tls_version_t;

#define FASTLY_TLS_VERSION_TLS1 0
#define FASTLY_TLS_VERSION_TLS11 1
#define FASTLY_TLS_VERSION_TLS12 2
#define FASTLY_TLS_VERSION_TLS13 3
#define FASTLY_RESULT_TLS_VERSION_OK 255

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

typedef uint16_t fastly_http_status_t;

typedef uint32_t fastly_dictionary_handle_t;

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

typedef uint32_t fastly_kv_store_handle_t;

typedef uint32_t fastly_fd_t;

typedef uint32_t fastly_object_store_handle_t;

typedef uint32_t fastly_secret_store_handle_t;

typedef uint32_t fastly_secret_handle_t;

typedef uint8_t fastly_backend_health_t;

#define FASTLY_BACKEND_HEALTH_UNKNOWN 0
#define FASTLY_BACKEND_HEALTH_HEALTHY 1
#define FASTLY_BACKEND_HEALTH_UNHEALTHY 2
#define FASTLY_RESULT_BACKEND_HEALTH_OK 255
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
  xqd_world_string_t id;
} fastly_purge_result_t;

typedef struct {
  bool is_err;
  union {
    fastly_error_t err;
  } val;
} fastly_result_void_error_t;

typedef struct {
  bool is_err;
  union {
    fastly_user_agent_t ok;
    fastly_error_t err;
  } val;
} fastly_result_user_agent_error_t;

typedef struct {
  bool is_err;
  union {
    fastly_body_handle_t ok;
    fastly_error_t err;
  } val;
} fastly_result_body_handle_error_t;

typedef struct {
  uint8_t *ptr;
  size_t len;
} fastly_list_u8_t;

typedef struct {
  bool is_err;
  union {
    fastly_list_u8_t ok;
    fastly_error_t err;
  } val;
} fastly_result_list_u8_error_t;

typedef struct {
  bool is_err;
  union {
    uint32_t ok;
    fastly_error_t err;
  } val;
} fastly_result_u32_error_t;

typedef struct {
  bool is_err;
  union {
    fastly_log_endpoint_handle_t ok;
    fastly_error_t err;
  } val;
} fastly_result_log_endpoint_handle_error_t;

typedef struct {
  bool is_err;
  union {
    fastly_request_t ok;
    fastly_error_t err;
  } val;
} fastly_result_request_error_t;

typedef struct {
  bool is_err;
  union {
    xqd_world_string_t ok;
    fastly_error_t err;
  } val;
} fastly_result_string_error_t;

typedef struct {
  bool is_err;
  union {
    fastly_request_handle_t ok;
    fastly_error_t err;
  } val;
} fastly_result_request_handle_error_t;

typedef struct {
  xqd_world_string_t *ptr;
  size_t len;
} fastly_list_string_t;

typedef struct {
  bool is_err;
  union {
    fastly_list_string_t ok;
    fastly_error_t err;
  } val;
} fastly_result_list_string_error_t;

typedef struct {
  bool is_err;
  union {
    fastly_option_string_t ok;
    fastly_error_t err;
  } val;
} fastly_result_option_string_error_t;

typedef struct {
  bool is_some;
  fastly_list_string_t val;
} fastly_option_list_string_t;

typedef struct {
  bool is_err;
  union {
    fastly_option_list_string_t ok;
    fastly_error_t err;
  } val;
} fastly_result_option_list_string_error_t;

typedef struct {
  bool is_err;
  union {
    fastly_http_version_t ok;
    fastly_error_t err;
  } val;
} fastly_result_http_version_error_t;

typedef struct {
  bool is_err;
  union {
    fastly_response_t ok;
    fastly_error_t err;
  } val;
} fastly_result_response_error_t;

typedef struct {
  bool is_err;
  union {
    fastly_pending_request_handle_t ok;
    fastly_error_t err;
  } val;
} fastly_result_pending_request_handle_error_t;

typedef struct {
  bool is_some;
  fastly_response_t val;
} fastly_option_response_t;

typedef struct {
  bool is_err;
  union {
    fastly_option_response_t ok;
    fastly_error_t err;
  } val;
} fastly_result_option_response_error_t;

typedef struct {
  fastly_pending_request_handle_t *ptr;
  size_t len;
} fastly_list_pending_request_handle_t;

typedef struct {
  uint32_t f0;
  fastly_response_t f1;
} fastly_tuple2_u32_response_t;

typedef struct {
  bool is_err;
  union {
    fastly_tuple2_u32_response_t ok;
    fastly_error_t err;
  } val;
} fastly_result_tuple2_u32_response_error_t;

typedef struct {
  bool is_err;
  union {
    bool ok;
    fastly_error_t err;
  } val;
} fastly_result_bool_error_t;

typedef struct {
  bool is_err;
  union {
    fastly_response_handle_t ok;
    fastly_error_t err;
  } val;
} fastly_result_response_handle_error_t;

typedef struct {
  bool is_err;
  union {
    fastly_http_status_t ok;
    fastly_error_t err;
  } val;
} fastly_result_http_status_error_t;

typedef struct {
  bool is_err;
  union {
    fastly_dictionary_handle_t ok;
    fastly_error_t err;
  } val;
} fastly_result_dictionary_handle_error_t;

typedef struct {
  bool is_err;
  union {
    fastly_kv_store_handle_t ok;
    fastly_error_t err;
  } val;
} fastly_result_kv_store_handle_error_t;

typedef struct {
  bool is_err;
  union {
    fastly_object_store_handle_t ok;
    fastly_error_t err;
  } val;
} fastly_result_object_store_handle_error_t;

typedef struct {
  bool is_err;
  union {
    fastly_fd_t ok;
    fastly_error_t err;
  } val;
} fastly_result_fd_error_t;

typedef struct {
  bool is_err;
  union {
    fastly_secret_store_handle_t ok;
    fastly_error_t err;
  } val;
} fastly_result_secret_store_handle_error_t;

typedef struct {
  bool is_err;
  union {
    fastly_secret_handle_t ok;
    fastly_error_t err;
  } val;
} fastly_result_secret_handle_error_t;

typedef struct {
  bool is_err;
  union {
    fastly_backend_health_t ok;
    fastly_error_t err;
  } val;
} fastly_result_backend_health_error_t;

typedef struct {
  fastly_async_handle_t *ptr;
  size_t len;
} fastly_list_async_handle_t;

typedef struct {
  bool is_err;
  union {
    fastly_purge_result_t ok;
    fastly_error_t err;
  } val;
} fastly_result_purge_result_error_t;

// Imported Functions from `fastly`

__attribute__((import_module("fastly"),
               import_name("abi-init"))) void __wasm_import_fastly_abi_init(int64_t, int32_t);
fastly_error_t fastly_abi_init(uint64_t abi_version);

__attribute__((import_module("fastly"), import_name("uap-parse"))) void
    __wasm_import_fastly_uap_parse(int32_t, int32_t, int32_t);
fastly_error_t fastly_uap_parse(xqd_world_string_t *user_agent, fastly_user_agent_t *ret);

__attribute__((import_module("fastly"),
               import_name("http-body-new"))) void __wasm_import_fastly_http_body_new(int32_t);
fastly_error_t fastly_http_body_new(fastly_body_handle_t *ret);

__attribute__((import_module("fastly"), import_name("http-body-append"))) void
    __wasm_import_fastly_http_body_append(int32_t, int32_t, int32_t);
fastly_error_t fastly_http_body_append(fastly_body_handle_t dest, fastly_body_handle_t src);

__attribute__((import_module("fastly"), import_name("http-body-read"))) void
    __wasm_import_fastly_http_body_read(int32_t, int32_t, int32_t);
fastly_error_t fastly_http_body_read(fastly_body_handle_t h, uint32_t chunk_size,
                                     fastly_list_u8_t *ret);

__attribute__((import_module("fastly"), import_name("http-body-write"))) void
    __wasm_import_fastly_http_body_write(int32_t, int32_t, int32_t, int32_t, int32_t);
fastly_error_t fastly_http_body_write(fastly_body_handle_t h, fastly_list_u8_t *buf,
                                      fastly_body_write_end_t end, uint32_t *ret);

__attribute__((import_module("fastly"), import_name("http-body-close"))) void
    __wasm_import_fastly_http_body_close(int32_t, int32_t);
fastly_error_t fastly_http_body_close(fastly_body_handle_t h);

__attribute__((import_module("fastly"), import_name("log-endpoint-get"))) void
    __wasm_import_fastly_log_endpoint_get(int32_t, int32_t, int32_t);
fastly_error_t fastly_log_endpoint_get(xqd_world_string_t *name, fastly_log_endpoint_handle_t *ret);

__attribute__((import_module("fastly"), import_name("log-write"))) void
    __wasm_import_fastly_log_write(int32_t, int32_t, int32_t, int32_t);
fastly_error_t fastly_log_write(fastly_log_endpoint_handle_t h, xqd_world_string_t *msg);

__attribute__((import_module("fastly"), import_name("http-req-body-downstream-get"))) void
    __wasm_import_fastly_http_req_body_downstream_get(int32_t);
fastly_error_t fastly_http_req_body_downstream_get(fastly_request_t *ret);

__attribute__((import_module("fastly"), import_name("http-req-cache-override-set"))) void
    __wasm_import_fastly_http_req_cache_override_set(int32_t, int32_t, int32_t, int32_t, int32_t,
                                                     int32_t, int32_t, int32_t, int32_t, int32_t);
fastly_error_t fastly_http_req_cache_override_set(fastly_request_handle_t h,
                                                  fastly_http_cache_override_tag_t tag,
                                                  fastly_option_u32_t *ttl,
                                                  fastly_option_u32_t *stale_while_revalidate,
                                                  fastly_option_string_t *sk);

__attribute__((import_module("fastly"), import_name("http-req-downstream-client-ip-addr"))) void
    __wasm_import_fastly_http_req_downstream_client_ip_addr(int32_t);
fastly_error_t fastly_http_req_downstream_client_ip_addr(fastly_list_u8_t *ret);

__attribute__((import_module("fastly"),
               import_name("http-req-downstream-client-h2-fingerprint"))) void
    __wasm_import_fastly_http_req_downstream_client_h2_fingerprint(int32_t);
fastly_error_t fastly_http_req_downstream_client_h2_fingerprint(fastly_list_u8_t *ret);

__attribute__((import_module("fastly"),
               import_name("http-req-downstream-tls-cipher-openssl-name"))) void
    __wasm_import_fastly_http_req_downstream_tls_cipher_openssl_name(int32_t);
fastly_error_t fastly_http_req_downstream_tls_cipher_openssl_name(xqd_world_string_t *ret);

__attribute__((import_module("fastly"), import_name("http-req-downstream-tls-protocol"))) void
    __wasm_import_fastly_http_req_downstream_tls_protocol(int32_t);
fastly_error_t fastly_http_req_downstream_tls_protocol(xqd_world_string_t *ret);

__attribute__((import_module("fastly"), import_name("http-req-downstream-tls-client-hello"))) void
    __wasm_import_fastly_http_req_downstream_tls_client_hello(int32_t);
fastly_error_t fastly_http_req_downstream_tls_client_hello(fastly_list_u8_t *ret);

__attribute__((import_module("fastly"),
               import_name("http-req-downstream-tls-client-certificate"))) void
    __wasm_import_fastly_http_req_downstream_tls_client_certificate(int32_t);
fastly_error_t fastly_http_req_downstream_tls_client_certificate(fastly_list_u8_t *ret);

__attribute__((import_module("fastly"),
               import_name("http-req-downstream-tls-client-cert-verify-result"))) void
    __wasm_import_fastly_http_req_downstream_tls_client_cert_verify_result(int32_t);
fastly_error_t fastly_http_req_downstream_tls_client_cert_verify_result(void);

__attribute__((import_module("fastly"), import_name("http-req-downstream-tls-ja3-md5"))) void
    __wasm_import_fastly_http_req_downstream_tls_ja3_md5(int32_t);
fastly_error_t fastly_http_req_downstream_tls_ja3_md5(fastly_list_u8_t *ret);

__attribute__((import_module("fastly"),
               import_name("http-req-new"))) void __wasm_import_fastly_http_req_new(int32_t);
fastly_error_t fastly_http_req_new(fastly_request_handle_t *ret);

__attribute__((import_module("fastly"), import_name("http-req-header-names-get"))) void
    __wasm_import_fastly_http_req_header_names_get(int32_t, int32_t);
fastly_error_t fastly_http_req_header_names_get(fastly_request_handle_t h,
                                                fastly_list_string_t *ret);

__attribute__((import_module("fastly"), import_name("http-req-header-value-get"))) void
    __wasm_import_fastly_http_req_header_value_get(int32_t, int32_t, int32_t, int32_t);
fastly_error_t fastly_http_req_header_value_get(fastly_request_handle_t h, xqd_world_string_t *name,
                                                fastly_option_string_t *ret);

__attribute__((import_module("fastly"), import_name("http-req-header-values-get"))) void
    __wasm_import_fastly_http_req_header_values_get(int32_t, int32_t, int32_t, int32_t);
fastly_error_t fastly_http_req_header_values_get(fastly_request_handle_t h,
                                                 xqd_world_string_t *name,
                                                 fastly_option_list_string_t *ret);

__attribute__((import_module("fastly"), import_name("http-req-header-values-set"))) void
    __wasm_import_fastly_http_req_header_values_set(int32_t, int32_t, int32_t, int32_t, int32_t,
                                                    int32_t);
fastly_error_t fastly_http_req_header_values_set(fastly_request_handle_t h,
                                                 xqd_world_string_t *name,
                                                 fastly_list_string_t *values);

__attribute__((import_module("fastly"), import_name("http-req-header-insert"))) void
    __wasm_import_fastly_http_req_header_insert(int32_t, int32_t, int32_t, int32_t, int32_t,
                                                int32_t);
fastly_error_t fastly_http_req_header_insert(fastly_request_handle_t h, xqd_world_string_t *name,
                                             xqd_world_string_t *value);

__attribute__((import_module("fastly"), import_name("http-req-header-append"))) void
    __wasm_import_fastly_http_req_header_append(int32_t, int32_t, int32_t, int32_t, int32_t,
                                                int32_t);
fastly_error_t fastly_http_req_header_append(fastly_request_handle_t h, xqd_world_string_t *name,
                                             xqd_world_string_t *value);

__attribute__((import_module("fastly"), import_name("http-req-header-remove"))) void
    __wasm_import_fastly_http_req_header_remove(int32_t, int32_t, int32_t, int32_t);
fastly_error_t fastly_http_req_header_remove(fastly_request_handle_t h, xqd_world_string_t *name);

__attribute__((import_module("fastly"), import_name("http-req-method-get"))) void
    __wasm_import_fastly_http_req_method_get(int32_t, int32_t);
fastly_error_t fastly_http_req_method_get(fastly_request_handle_t h, xqd_world_string_t *ret);

__attribute__((import_module("fastly"), import_name("http-req-method-set"))) void
    __wasm_import_fastly_http_req_method_set(int32_t, int32_t, int32_t, int32_t);
fastly_error_t fastly_http_req_method_set(fastly_request_handle_t h, xqd_world_string_t *method);

__attribute__((import_module("fastly"), import_name("http-req-uri-get"))) void
    __wasm_import_fastly_http_req_uri_get(int32_t, int32_t);
fastly_error_t fastly_http_req_uri_get(fastly_request_handle_t h, xqd_world_string_t *ret);

__attribute__((import_module("fastly"), import_name("http-req-uri-set"))) void
    __wasm_import_fastly_http_req_uri_set(int32_t, int32_t, int32_t, int32_t);
fastly_error_t fastly_http_req_uri_set(fastly_request_handle_t h, xqd_world_string_t *uri);

__attribute__((import_module("fastly"), import_name("http-req-version-get"))) void
    __wasm_import_fastly_http_req_version_get(int32_t, int32_t);
fastly_error_t fastly_http_req_version_get(fastly_request_handle_t h, fastly_http_version_t *ret);

__attribute__((import_module("fastly"), import_name("http-req-version-set"))) void
    __wasm_import_fastly_http_req_version_set(int32_t, int32_t, int32_t);
fastly_error_t fastly_http_req_version_set(fastly_request_handle_t h,
                                           fastly_http_version_t version);

__attribute__((import_module("fastly"), import_name("http-req-send"))) void
    __wasm_import_fastly_http_req_send(int32_t, int32_t, int32_t, int32_t, int32_t);
fastly_error_t fastly_http_req_send(fastly_request_handle_t h, fastly_body_handle_t b,
                                    xqd_world_string_t *backend, fastly_response_t *ret);

__attribute__((import_module("fastly"), import_name("http-req-send-async"))) void
    __wasm_import_fastly_http_req_send_async(int32_t, int32_t, int32_t, int32_t, int32_t);
fastly_error_t fastly_http_req_send_async(fastly_request_handle_t h, fastly_body_handle_t b,
                                          xqd_world_string_t *backend,
                                          fastly_pending_request_handle_t *ret);

__attribute__((import_module("fastly"), import_name("http-req-send-async-streaming"))) void
    __wasm_import_fastly_http_req_send_async_streaming(int32_t, int32_t, int32_t, int32_t, int32_t);
fastly_error_t fastly_http_req_send_async_streaming(fastly_request_handle_t h,
                                                    fastly_body_handle_t b,
                                                    xqd_world_string_t *backend,
                                                    fastly_pending_request_handle_t *ret);

__attribute__((import_module("fastly"), import_name("http-req-pending-req-poll"))) void
    __wasm_import_fastly_http_req_pending_req_poll(int32_t, int32_t);
fastly_error_t fastly_http_req_pending_req_poll(fastly_pending_request_handle_t h,
                                                fastly_option_response_t *ret);

__attribute__((import_module("fastly"), import_name("http-req-pending-req-wait"))) void
    __wasm_import_fastly_http_req_pending_req_wait(int32_t, int32_t);
fastly_error_t fastly_http_req_pending_req_wait(fastly_pending_request_handle_t h,
                                                fastly_response_t *ret);

__attribute__((import_module("fastly"), import_name("http-req-pending-req-select"))) void
    __wasm_import_fastly_http_req_pending_req_select(int32_t, int32_t, int32_t);
fastly_error_t fastly_http_req_pending_req_select(fastly_list_pending_request_handle_t *h,
                                                  fastly_tuple2_u32_response_t *ret);

__attribute__((import_module("fastly"), import_name("http-req-key-is-valid"))) void
    __wasm_import_fastly_http_req_key_is_valid(int32_t);
fastly_error_t fastly_http_req_key_is_valid(bool *ret);

__attribute__((import_module("fastly"), import_name("http-req-close"))) void
    __wasm_import_fastly_http_req_close(int32_t, int32_t);
fastly_error_t fastly_http_req_close(fastly_request_handle_t h);

__attribute__((import_module("fastly"), import_name("http-req-auto-decompress-response-set"))) void
    __wasm_import_fastly_http_req_auto_decompress_response_set(int32_t, int32_t, int32_t);
fastly_error_t fastly_http_req_auto_decompress_response_set(fastly_request_handle_t h,
                                                            fastly_content_encodings_t encodings);

__attribute__((import_module("fastly"), import_name("http-req-upgrade-websocket"))) void
    __wasm_import_fastly_http_req_upgrade_websocket(int32_t, int32_t, int32_t);
fastly_error_t fastly_http_req_upgrade_websocket(xqd_world_string_t *backend);

__attribute__((import_module("fastly"), import_name("http-req-redirect-to-websocket-proxy"))) void
    __wasm_import_fastly_http_req_redirect_to_websocket_proxy(int32_t, int32_t, int32_t);
fastly_error_t fastly_http_req_redirect_to_websocket_proxy(xqd_world_string_t *backend);

__attribute__((import_module("fastly"), import_name("http-req-redirect-to-grip-proxy"))) void
    __wasm_import_fastly_http_req_redirect_to_grip_proxy(int32_t, int32_t, int32_t);
fastly_error_t fastly_http_req_redirect_to_grip_proxy(xqd_world_string_t *backend);

__attribute__((import_module("fastly"), import_name("http-req-framing-headers-mode-set"))) void
    __wasm_import_fastly_http_req_framing_headers_mode_set(int32_t, int32_t, int32_t);
fastly_error_t fastly_http_req_framing_headers_mode_set(fastly_request_handle_t h,
                                                        fastly_framing_headers_mode_t mode);

__attribute__((import_module("fastly"), import_name("http-req-register-dynamic-backend"))) void
    __wasm_import_fastly_http_req_register_dynamic_backend(int32_t, int32_t);
fastly_error_t fastly_http_req_register_dynamic_backend(xqd_world_string_t *prefix,
                                                        xqd_world_string_t *target,
                                                        fastly_dynamic_backend_config_t *config);

__attribute__((import_module("fastly"),
               import_name("http-resp-new"))) void __wasm_import_fastly_http_resp_new(int32_t);
fastly_error_t fastly_http_resp_new(fastly_response_handle_t *ret);

__attribute__((import_module("fastly"), import_name("http-resp-header-names-get"))) void
    __wasm_import_fastly_http_resp_header_names_get(int32_t, int32_t);
fastly_error_t fastly_http_resp_header_names_get(fastly_response_handle_t h,
                                                 fastly_list_string_t *ret);

__attribute__((import_module("fastly"), import_name("http-resp-header-value-get"))) void
    __wasm_import_fastly_http_resp_header_value_get(int32_t, int32_t, int32_t, int32_t);
fastly_error_t fastly_http_resp_header_value_get(fastly_response_handle_t h,
                                                 xqd_world_string_t *name,
                                                 fastly_option_string_t *ret);

__attribute__((import_module("fastly"), import_name("http-resp-header-values-get"))) void
    __wasm_import_fastly_http_resp_header_values_get(int32_t, int32_t, int32_t, int32_t);
fastly_error_t fastly_http_resp_header_values_get(fastly_response_handle_t h,
                                                  xqd_world_string_t *name,
                                                  fastly_option_list_string_t *ret);

__attribute__((import_module("fastly"), import_name("http-resp-header-values-set"))) void
    __wasm_import_fastly_http_resp_header_values_set(int32_t, int32_t, int32_t, int32_t, int32_t,
                                                     int32_t);
fastly_error_t fastly_http_resp_header_values_set(fastly_response_handle_t h,
                                                  xqd_world_string_t *name,
                                                  fastly_list_string_t *values);

__attribute__((import_module("fastly"), import_name("http-resp-header-insert"))) void
    __wasm_import_fastly_http_resp_header_insert(int32_t, int32_t, int32_t, int32_t, int32_t,
                                                 int32_t);
fastly_error_t fastly_http_resp_header_insert(fastly_response_handle_t h, xqd_world_string_t *name,
                                              xqd_world_string_t *value);

__attribute__((import_module("fastly"), import_name("http-resp-header-append"))) void
    __wasm_import_fastly_http_resp_header_append(int32_t, int32_t, int32_t, int32_t, int32_t,
                                                 int32_t);
fastly_error_t fastly_http_resp_header_append(fastly_response_handle_t h, xqd_world_string_t *name,
                                              xqd_world_string_t *value);

__attribute__((import_module("fastly"), import_name("http-resp-header-remove"))) void
    __wasm_import_fastly_http_resp_header_remove(int32_t, int32_t, int32_t, int32_t);
fastly_error_t fastly_http_resp_header_remove(fastly_response_handle_t h, xqd_world_string_t *name);

__attribute__((import_module("fastly"), import_name("http-resp-version-get"))) void
    __wasm_import_fastly_http_resp_version_get(int32_t, int32_t);
fastly_error_t fastly_http_resp_version_get(fastly_response_handle_t h, fastly_http_version_t *ret);

__attribute__((import_module("fastly"), import_name("http-resp-version-set"))) void
    __wasm_import_fastly_http_resp_version_set(int32_t, int32_t, int32_t);
fastly_error_t fastly_http_resp_version_set(fastly_response_handle_t h,
                                            fastly_http_version_t version);

__attribute__((import_module("fastly"), import_name("http-resp-send-downstream"))) void
    __wasm_import_fastly_http_resp_send_downstream(int32_t, int32_t, int32_t, int32_t);
fastly_error_t fastly_http_resp_send_downstream(fastly_response_handle_t h, fastly_body_handle_t b,
                                                bool streaming);

__attribute__((import_module("fastly"), import_name("http-resp-status-get"))) void
    __wasm_import_fastly_http_resp_status_get(int32_t, int32_t);
fastly_error_t fastly_http_resp_status_get(fastly_response_handle_t h, fastly_http_status_t *ret);

__attribute__((import_module("fastly"), import_name("http-resp-status-set"))) void
    __wasm_import_fastly_http_resp_status_set(int32_t, int32_t, int32_t);
fastly_error_t fastly_http_resp_status_set(fastly_response_handle_t h, fastly_http_status_t status);

__attribute__((import_module("fastly"), import_name("http-resp-close"))) void
    __wasm_import_fastly_http_resp_close(int32_t, int32_t);
fastly_error_t fastly_http_resp_close(fastly_response_handle_t h);

__attribute__((import_module("fastly"), import_name("http-resp-framing-headers-mode-set"))) void
    __wasm_import_fastly_http_resp_framing_headers_mode_set(int32_t, int32_t, int32_t);
fastly_error_t fastly_http_resp_framing_headers_mode_set(fastly_response_handle_t h,
                                                         fastly_framing_headers_mode_t mode);

__attribute__((import_module("fastly"), import_name("dictionary-open"))) void
    __wasm_import_fastly_dictionary_open(int32_t, int32_t, int32_t);
fastly_error_t fastly_dictionary_open(xqd_world_string_t *name, fastly_dictionary_handle_t *ret);

__attribute__((import_module("fastly"), import_name("dictionary-get"))) void
    __wasm_import_fastly_dictionary_get(int32_t, int32_t, int32_t, int32_t);
fastly_error_t fastly_dictionary_get(fastly_dictionary_handle_t h, xqd_world_string_t *key,
                                     xqd_world_string_t *ret);

__attribute__((import_module("fastly"), import_name("geo-lookup"))) void
    __wasm_import_fastly_geo_lookup(int32_t, int32_t, int32_t);
fastly_error_t fastly_geo_lookup(fastly_list_u8_t *addr_octets, xqd_world_string_t *ret);

__attribute__((import_module("fastly"), import_name("kv-open"))) void
    __wasm_import_fastly_kv_open(int32_t, int32_t, int32_t);
fastly_error_t fastly_kv_open(xqd_world_string_t *name, fastly_kv_store_handle_t *ret);

__attribute__((import_module("fastly"), import_name("kv-lookup"))) void
    __wasm_import_fastly_kv_lookup(int32_t, int32_t, int32_t, int32_t);
fastly_error_t fastly_kv_lookup(fastly_kv_store_handle_t store, fastly_list_u8_t *key,
                                fastly_body_handle_t *ret);

__attribute__((import_module("fastly"), import_name("kv-insert"))) void
    __wasm_import_fastly_kv_insert(int32_t, int32_t, int32_t, int32_t, int32_t, int32_t);
fastly_error_t fastly_kv_insert(fastly_kv_store_handle_t store, fastly_list_u8_t *key,
                                fastly_body_handle_t body_handle, uint32_t max_age, bool *ret);

__attribute__((import_module("fastly"), import_name("object-store-open"))) void
    __wasm_import_fastly_object_store_open(int32_t, int32_t, int32_t);
fastly_error_t fastly_object_store_open(xqd_world_string_t *name,
                                        fastly_object_store_handle_t *ret);

__attribute__((import_module("fastly"), import_name("object-store-lookup"))) void
    __wasm_import_fastly_object_store_lookup(int32_t, int32_t, int32_t, int32_t);
fastly_error_t fastly_object_store_lookup(fastly_object_store_handle_t store,
                                          xqd_world_string_t *key, fastly_body_handle_t *ret);

__attribute__((import_module("fastly"), import_name("object-store-lookup-as-fd"))) void
    __wasm_import_fastly_object_store_lookup_as_fd(int32_t, int32_t, int32_t, int32_t);
fastly_error_t fastly_object_store_lookup_as_fd(fastly_object_store_handle_t store,
                                                xqd_world_string_t *key, fastly_fd_t *ret);

__attribute__((import_module("fastly"), import_name("object-store-insert"))) void
    __wasm_import_fastly_object_store_insert(int32_t, int32_t, int32_t, int32_t, int32_t);
fastly_error_t fastly_object_store_insert(fastly_object_store_handle_t store,
                                          xqd_world_string_t *key,
                                          fastly_body_handle_t body_handle);

__attribute__((import_module("fastly"), import_name("secret-store-open"))) void
    __wasm_import_fastly_secret_store_open(int32_t, int32_t, int32_t);
fastly_error_t fastly_secret_store_open(xqd_world_string_t *name,
                                        fastly_secret_store_handle_t *ret);

__attribute__((import_module("fastly"), import_name("secret-store-get"))) void
    __wasm_import_fastly_secret_store_get(int32_t, int32_t, int32_t, int32_t);
fastly_error_t fastly_secret_store_get(fastly_secret_store_handle_t store, xqd_world_string_t *key,
                                       fastly_secret_handle_t *ret);

__attribute__((import_module("fastly"), import_name("secret-store-plaintext"))) void
    __wasm_import_fastly_secret_store_plaintext(int32_t, int32_t);
fastly_error_t fastly_secret_store_plaintext(fastly_secret_handle_t secret,
                                             xqd_world_string_t *ret);

__attribute__((import_module("fastly"), import_name("backend-is-healthy"))) void
    __wasm_import_fastly_backend_is_healthy(int32_t, int32_t, int32_t);
fastly_error_t fastly_backend_is_healthy(xqd_world_string_t *backend, fastly_backend_health_t *ret);

__attribute__((import_module("fastly"), import_name("async-io-select"))) void
    __wasm_import_fastly_async_io_select(int32_t, int32_t, int32_t, int32_t);
fastly_error_t fastly_async_io_select(fastly_list_async_handle_t *hs, uint32_t timeout_ms,
                                      uint32_t *ret);

__attribute__((import_module("fastly"), import_name("async-io-is-ready"))) void
    __wasm_import_fastly_async_io_is_ready(int32_t, int32_t);
fastly_error_t fastly_async_io_is_ready(fastly_async_handle_t handle, bool *ret);

__attribute__((import_module("fastly"), import_name("purge-surrogate-key"))) void
    __wasm_import_fastly_purge_surrogate_key(int32_t, int32_t, int32_t, int32_t);
fastly_error_t fastly_purge_surrogate_key(xqd_world_string_t *surrogate_key, bool soft_purge,
                                          fastly_purge_result_t *ret);

#ifdef __cplusplus
}
#endif
#endif
