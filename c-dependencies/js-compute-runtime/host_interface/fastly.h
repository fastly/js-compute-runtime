#ifndef fastly_H
#define fastly_H
#ifdef __cplusplus
extern "C" {
#endif

#include "fastly-world/fastly_world.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

namespace fastly {

#define WASM_IMPORT(module, name) __attribute__((import_module(module), import_name(name)))

// max header size to match vcl
#define HEADER_MAX_LEN 69000
#define METHOD_MAX_LEN 1024
#define URI_MAX_LEN 8192
#define CONFIG_STORE_ENTRY_MAX_LEN 8000
#define DICTIONARY_ENTRY_MAX_LEN CONFIG_STORE_ENTRY_MAX_LEN

// The values need to match https://docs.rs/fastly-sys/0.8.7/src/fastly_sys/lib.rs.html#86-108
#define BACKEND_CONFIG_RESERVED (1u << 0)
#define BACKEND_CONFIG_HOST_OVERRIDE (1u << 1)
#define BACKEND_CONFIG_CONNECT_TIMEOUT (1u << 2)
#define BACKEND_CONFIG_FIRST_BYTE_TIMEOUT (1u << 3)
#define BACKEND_CONFIG_BETWEEN_BYTES_TIMEOUT (1u << 4)
#define BACKEND_CONFIG_USE_SSL (1u << 5)
#define BACKEND_CONFIG_SSL_MIN_VERSION (1u << 6)
#define BACKEND_CONFIG_SSL_MAX_VERSION (1u << 7)
#define BACKEND_CONFIG_CERT_HOSTNAME (1u << 8)
#define BACKEND_CONFIG_CA_CERT (1u << 9)
#define BACKEND_CONFIG_CIPHERS (1u << 10)
#define BACKEND_CONFIG_SNI_HOSTNAME (1u << 11)

typedef enum TLS {
  VERSION_1 = 0,
  VERSION_1_1 = 1,
  VERSION_1_2 = 2,
  VERSION_1_3 = 3,
} TLS;

typedef struct DynamicBackendConfig {
  const char *host_override;
  uint32_t host_override_len;
  uint32_t connect_timeout_ms;
  uint32_t first_byte_timeout_ms;
  uint32_t between_bytes_timeout_ms;
  uint32_t ssl_min_version;
  uint32_t ssl_max_version;
  const char *cert_hostname;
  uint32_t cert_hostname_len;
  const char *ca_cert;
  uint32_t ca_cert_len;
  const char *ciphers;
  uint32_t ciphers_len;
  const char *sni_hostname;
  uint32_t sni_hostname_len;
} DynamicBackendConfig;

#define INVALID_HANDLE (UINT32_MAX - 1)

typedef enum BodyWriteEnd {
  BodyWriteEndBack = 0,
  BodyWriteEndFront = 1,
} BodyWriteEnd;

#define CACHE_OVERRIDE_NONE (0u)
#define CACHE_OVERRIDE_PASS (1u << 0)
#define CACHE_OVERRIDE_TTL (1u << 1)
#define CACHE_OVERRIDE_STALE_WHILE_REVALIDATE (1u << 2)
#define CACHE_OVERRIDE_PCI (1u << 3)

WASM_IMPORT("fastly_abi", "init")
int init(uint64_t abi_version);

// Module fastly_http_body
WASM_IMPORT("fastly_http_body", "append")
int body_append(fastly_body_handle_t dst_handle, fastly_body_handle_t src_handle);

WASM_IMPORT("fastly_http_body", "new")
int body_new(fastly_body_handle_t *handle_out);

WASM_IMPORT("fastly_http_body", "read")
int body_read(fastly_body_handle_t body_handle, char *buf, size_t buf_len, size_t *nread);

WASM_IMPORT("fastly_http_body", "write")
int body_write(fastly_body_handle_t body_handle, const char *buf, size_t buf_len, BodyWriteEnd end,
               size_t *nwritten);

WASM_IMPORT("fastly_http_body", "close")
int body_close(fastly_body_handle_t body_handle);

// Module fastly_log
WASM_IMPORT("fastly_log", "endpoint_get")
int log_endpoint_get(const char *name, size_t name_len,
                     fastly_log_endpoint_handle_t *endpoint_handle);

WASM_IMPORT("fastly_log", "write")
int log_write(fastly_log_endpoint_handle_t endpoint_handle, const char *msg, size_t msg_len,
              size_t *nwritten);

// Module fastly_http_req
WASM_IMPORT("fastly_http_req", "register_dynamic_backend")
int req_register_dynamic_backend(const char *name_prefix, size_t name_prefix_len,
                                 const char *target, size_t target_len,
                                 uint32_t backend_config_mask,
                                 DynamicBackendConfig *backend_configuration);

WASM_IMPORT("fastly_http_req", "body_downstream_get")
int req_body_downstream_get(fastly_request_handle_t *req_handle_out,
                            fastly_body_handle_t *body_handle_out);

/**
 * Set the cache override behavior for this request.
 *
 * The default behavior, equivalent to `CACHE_OVERRIDE_NONE`, respects the cache control headers
 * from the origin's response.
 *
 * Calling this function with `CACHE_OVERRIDE_PASS` will ignore the subsequent arguments and Pass
 * unconditionally.
 *
 * To override, TTL, stale-while-revalidate, or stale-with-error, set the appropriate bits in the
 * tag using the corresponding constants, and pass the override values in the appropriate arguments.
 *
 * fastly_req_cache_override_v2_set also includes an optional Surrogate-Key which will be set or
 * added to any received from the origin.
 */

WASM_IMPORT("fastly_http_req", "cache_override_set")
int req_cache_override_set(fastly_request_handle_t req_handle, int tag, uint32_t ttl,
                           uint32_t stale_while_revalidate);

WASM_IMPORT("fastly_http_req", "cache_override_v2_set")
int req_cache_override_v2_set(fastly_request_handle_t req_handle, int tag, uint32_t ttl,
                              uint32_t stale_while_revalidate, const char *surrogate_key,
                              size_t surrogate_key_len);

/**
 * `octets` must be a 16-byte array.
 * If, after a successful call, `nwritten` == 4, the value in `octets` is an IPv4 address.
 * Otherwise, if `nwritten` will is `16`, the value in `octets` is an IPv6 address.
 * Otherwise, `nwritten` will be `0`, and no address is available.
 */
WASM_IMPORT("fastly_http_req", "downstream_client_ip_addr")
int req_downstream_client_ip_addr_get(char *octets, size_t *nwritten);

// TODO:

// (@interface func (export "downstream_tls_cipher_openssl_name")
//    (param $cipher_out (@witx pointer char8))
//    (param $cipher_max_len (@witx usize))
//    (param $nwritten_out (@witx pointer (@witx usize)))
//    (result $err $fastly_status)
// )

// (@interface func (export "downstream_tls_protocol")
//    (param $protocol_out (@witx pointer char8))
//    (param $protocol_max_len (@witx usize))
//    (param $nwritten_out (@witx pointer (@witx usize)))
//    (result $err $fastly_status)
// )

// (@interface func (export "downstream_tls_client_hello")
//    (param $chello_out (@witx pointer char8))
//    (param $chello_max_len (@witx usize))
//    (param $nwritten_out (@witx pointer (@witx usize)))
//    (result $err $fastly_status)
// )

WASM_IMPORT("fastly_http_req", "new")
int req_new(fastly_request_handle_t *req_handle_out);

WASM_IMPORT("fastly_http_req", "header_names_get")
int req_header_names_get(fastly_request_handle_t req_handle, char *buf, size_t buf_len,
                         uint32_t cursor, int64_t *ending_cursor, size_t *nwritten);

WASM_IMPORT("fastly_http_req", "original_header_names_get")
int req_original_header_names_get(char *buf, size_t buf_len, uint32_t cursor,
                                  int64_t *ending_cursor, size_t *nwritten);

WASM_IMPORT("fastly_http_req", "original_header_count")
int req_original_header_count(uint32_t *count);

WASM_IMPORT("fastly_http_req", "header_value_get")
int req_header_value_get(fastly_request_handle_t req_handle, const char *name, size_t name_len,
                         char *value, size_t value_max_len, size_t *nwritten);

WASM_IMPORT("fastly_http_req", "header_values_get")
int req_header_values_get(fastly_request_handle_t req_handle, const char *name, size_t name_len,
                          char *buf, size_t buf_len, uint32_t cursor, int64_t *ending_cursor,
                          size_t *nwritten);

WASM_IMPORT("fastly_http_req", "header_insert")
int req_header_insert(fastly_request_handle_t req_handle, const char *name, size_t name_len,
                      const char *value, size_t value_len);

WASM_IMPORT("fastly_http_req", "header_append")
int req_header_append(fastly_request_handle_t req_handle, const char *name, size_t name_len,
                      const char *value, size_t value_len);

WASM_IMPORT("fastly_http_req", "header_remove")
int req_header_remove(fastly_request_handle_t req_handle, const char *name, size_t name_len);

WASM_IMPORT("fastly_http_req", "method_get")
int req_method_get(fastly_request_handle_t req_handle, char *method, size_t method_max_len,
                   size_t *nwritten);

WASM_IMPORT("fastly_http_req", "method_set")
int req_method_set(fastly_request_handle_t req_handle, const char *method, size_t method_len);

WASM_IMPORT("fastly_http_req", "uri_get")
int req_uri_get(fastly_request_handle_t req_handle, char *uri, size_t uri_max_len,
                size_t *nwritten);

WASM_IMPORT("fastly_http_req", "uri_set")
int req_uri_set(fastly_request_handle_t req_handle, const char *uri, size_t uri_len);

WASM_IMPORT("fastly_http_req", "version_get")
int req_version_get(fastly_request_handle_t req_handle, uint32_t *version);

WASM_IMPORT("fastly_http_req", "version_set")
int req_version_set(fastly_request_handle_t req_handle, uint32_t version);

WASM_IMPORT("fastly_http_req", "send")
int req_send(fastly_request_handle_t req_handle, fastly_body_handle_t body_handle,
             const char *backend, size_t backend_len, fastly_response_handle_t *resp_handle_out,
             fastly_body_handle_t *resp_body_handle_out);

WASM_IMPORT("fastly_http_req", "send_async")
int req_send_async(fastly_request_handle_t req_handle, fastly_body_handle_t body_handle,
                   const char *backend, size_t backend_len,
                   fastly_pending_request_handle_t *pending_req_out);

WASM_IMPORT("fastly_http_req", "send_async_streaming")
int req_send_async_streaming(fastly_request_handle_t req_handle, fastly_body_handle_t body_handle,
                             const char *backend, size_t backend_len,
                             fastly_pending_request_handle_t *pending_req_out);

WASM_IMPORT("fastly_http_req", "pending_req_poll")
int req_pending_req_poll(fastly_pending_request_handle_t req_handle, uint32_t *is_done_out,
                         fastly_response_handle_t *resp_handle_out,
                         fastly_body_handle_t *resp_body_handle_out);

WASM_IMPORT("fastly_http_req", "pending_req_select")
int req_pending_req_select(fastly_pending_request_handle_t req_handles[], size_t req_handles_len,
                           uint32_t *done_idx_out, fastly_response_handle_t *resp_handle_out,
                           fastly_body_handle_t *resp_body_handle_out);

WASM_IMPORT("fastly_http_req", "pending_req_wait")
int req_pending_req_wait(fastly_pending_request_handle_t req_handle,
                         fastly_response_handle_t *resp_handle_out,
                         fastly_body_handle_t *resp_body_handle_out);

// Module fastly_http_resp
WASM_IMPORT("fastly_http_resp", "new")
int resp_new(fastly_response_handle_t *resp_handle_out);

WASM_IMPORT("fastly_http_resp", "header_names_get")
int resp_header_names_get(fastly_response_handle_t resp_handle, char *buf, size_t buf_len,
                          uint32_t cursor, int64_t *ending_cursor, size_t *nwritten);

WASM_IMPORT("fastly_http_resp", "header_values_get")
int resp_header_values_get(fastly_response_handle_t resp_handle, const char *name, size_t name_len,
                           char *buf, size_t buf_len, uint32_t cursor, int64_t *ending_cursor,
                           size_t *nwritten);

WASM_IMPORT("fastly_http_req", "header_values_set")
int req_header_values_set(fastly_request_handle_t req_handle, const char *name, size_t name_len,
                          const char *values, size_t values_len);

WASM_IMPORT("fastly_http_resp", "header_insert")
int resp_header_insert(fastly_response_handle_t resp_handle, const char *name, size_t name_len,
                       const char *value, size_t value_len);

WASM_IMPORT("fastly_http_resp", "header_append")
int resp_header_append(fastly_response_handle_t resp_handle, const char *name, size_t name_len,
                       const char *value, size_t value_len);

WASM_IMPORT("fastly_http_resp", "header_remove")
int resp_header_remove(fastly_response_handle_t resp_handle, const char *name, size_t name_len);

WASM_IMPORT("fastly_http_resp", "version_get")
int resp_version_get(fastly_response_handle_t resp_handle, uint32_t *version_out);

WASM_IMPORT("fastly_http_resp", "send_downstream")
int resp_send_downstream(fastly_response_handle_t resp_handle, fastly_body_handle_t body_handle,
                         uint32_t streaming);

WASM_IMPORT("fastly_http_resp", "status_get")
int resp_status_get(fastly_response_handle_t resp_handle, uint16_t *status_out);

WASM_IMPORT("fastly_http_resp", "status_set")
int resp_status_set(fastly_response_handle_t resp_handle, uint16_t status);

// Module fastly_dictionary
WASM_IMPORT("fastly_dictionary", "open")
int dictionary_open(const char *name, size_t name_len, fastly_dictionary_handle_t *dict_handle_out);

WASM_IMPORT("fastly_dictionary", "get")
int dictionary_get(fastly_dictionary_handle_t dict_handle, const char *key, size_t key_len,
                   char *value, size_t value_max_len, size_t *nwritten);

// Module fastly_secret_store
WASM_IMPORT("fastly_secret_store", "open")
int secret_store_open(const char *name, size_t name_len,
                      fastly_secret_store_handle_t *dict_handle_out);

WASM_IMPORT("fastly_secret_store", "get")
int secret_store_get(fastly_secret_store_handle_t dict_handle, const char *key, size_t key_len,
                     fastly_secret_handle_t *opt_secret_handle_out);

WASM_IMPORT("fastly_secret_store", "plaintext")
int secret_store_plaintext(fastly_secret_handle_t secret_handle, char *buf, size_t buf_len,
                           size_t *nwritten);

// Module fastly_object_store
WASM_IMPORT("fastly_object_store", "open")
int object_store_open(const char *name, size_t name_len,
                      fastly_object_store_handle_t *object_store_handle_out);
WASM_IMPORT("fastly_object_store", "lookup")
int object_store_get(fastly_object_store_handle_t object_store_handle, const char *key,
                     size_t key_len, fastly_body_handle_t *opt_body_handle_out);
WASM_IMPORT("fastly_object_store", "lookup_async")
int object_store_get_async(
    fastly_object_store_handle_t object_store_handle, const char *key, size_t key_len,
    fastly_pending_object_store_lookup_handle_t *pending_object_store_lookup_handle_out);

WASM_IMPORT("fastly_object_store", "lookup_wait")
int object_store_lookup_wait(fastly_pending_object_store_lookup_handle_t handle,
                             fastly_option_body_handle_t *handle_out);

WASM_IMPORT("fastly_object_store", "insert")
int object_store_insert(fastly_object_store_handle_t object_store_handle, const char *key,
                        size_t key_len, fastly_body_handle_t body_handle);
WASM_IMPORT("fastly_geo", "lookup")
int geo_lookup(const char *addr_octets, size_t addr_len, char *buf, size_t buf_len,
               size_t *nwritten);

WASM_IMPORT("wasi_snapshot_preview1", "random_get")
int32_t random_get(int32_t arg0, int32_t arg1);

// Blocks until one of the given objects is ready for I/O, or the optional timeout expires.
//
// Valid object handles includes bodies and pending requests. See the `async_item_handle`
// definition for more details, including what I/O actions are associated with each handle
// type.
//
// The timeout is specified in milliseconds, or 0 if no timeout is desired.
//
// Returns the _index_ (not handle!) of the first object that is ready, or u32::MAX if the
// timeout expires before any objects are ready for I/O.
WASM_IMPORT("fastly_async_io", "select")
int async_select(fastly_async_handle_t handles[], size_t handles_len, uint32_t timeout_ms,
                 uint32_t *ready_idx_out);

// Returns 1 if the given async item is "ready" for its associated I/O action, 0 otherwise.
//
// If an object is ready, the I/O action is guaranteed to complete without blocking.
//
// Valid object handles includes bodies and pending requests. See the `async_item_handle`
// definition for more details, including what I/O actions are associated with each handle
// type.
WASM_IMPORT("fastly_async_io", "is_ready")
int async_is_ready(fastly_async_handle_t handle, uint32_t *is_ready_out);
} // namespace fastly
#ifdef __cplusplus
}
#endif
#endif
