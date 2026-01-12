#ifndef fastly_H
#define fastly_H
#ifdef __cplusplus
extern "C" {
namespace fastly {
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct fastly_world_string {
  uint8_t *ptr;
  size_t len;
} fastly_world_string;

typedef struct fastly_world_list_string {
  fastly_world_string *ptr;
  size_t len;
} fastly_world_list_string;

typedef struct fastly_world_list_u8 {
  uint8_t *ptr;
  size_t len;
} fastly_world_list_u8;

typedef struct fastly_world_list_list_u8 {
  fastly_world_list_u8 *ptr;
  size_t len;
} fastly_world_list_list_u8;

typedef struct {
  bool is_some;
  fastly_world_list_list_u8 val;
} fastly_world_option_list_list_u8;

typedef struct fastly_host_http_response {
  uint32_t f0;
  uint32_t f1;
} fastly_host_http_response;

typedef struct fastly_host_http_inspect_options {
  const char *corp;
  uint32_t corp_len;
  const char *workspace;
  uint32_t workspace_len;
  const char *override_client_ip_ptr;
  uint32_t override_client_ip_len;
} fastly_host_http_inspect_options;

typedef fastly_host_http_response fastly_world_tuple2_handle_handle;

#define WASM_IMPORT(module, name) __attribute__((import_module(module), import_name(name)))

// max header size to match vcl
#define HEADER_MAX_LEN 69000
#define METHOD_MAX_LEN 1024
#define URI_MAX_LEN 8192
#define CONFIG_STORE_INITIAL_BUF_LEN 512
#define DICTIONARY_ENTRY_MAX_LEN 8000

// Ensure that all the things we want to use the hostcall buffer for actually
// fit into the buffer.
#define HOSTCALL_BUFFER_LEN HEADER_MAX_LEN
static_assert(DICTIONARY_ENTRY_MAX_LEN < HOSTCALL_BUFFER_LEN);
static_assert(METHOD_MAX_LEN < HOSTCALL_BUFFER_LEN);
static_assert(URI_MAX_LEN < HOSTCALL_BUFFER_LEN);

#define LIST_ALLOC_SIZE 50

typedef uint8_t fastly_host_error;

// Unknown error value.
// It should be an internal error if this is returned.
#define FASTLY_HOST_ERROR_UNKNOWN_ERROR 0
// Generic error value.
// This means that some unexpected error occurred during a hostcall.
#define FASTLY_HOST_ERROR_GENERIC_ERROR 1
// Invalid argument.
#define FASTLY_HOST_ERROR_INVALID_ARGUMENT 2
// Invalid handle.
// Thrown when a handle is not valid. E.G. No dictionary exists with the given name.
#define FASTLY_HOST_ERROR_BAD_HANDLE 3
// Buffer length error.
// Thrown when a buffer is too long.
#define FASTLY_HOST_ERROR_BUFFER_LEN 4
// Unsupported operation error.
// This error is thrown when some operation cannot be performed, because it is not supported.
#define FASTLY_HOST_ERROR_UNSUPPORTED 5
// Alignment error.
// This is thrown when a pointer does not point to a properly aligned slice of memory.
#define FASTLY_HOST_ERROR_BAD_ALIGN 6
// Invalid HTTP error.
// This can be thrown when a method, URI, header, or status is not valid. This can also
// be thrown if a message head is too large.
#define FASTLY_HOST_ERROR_HTTP_INVALID 7
// HTTP user error.
// This is thrown in cases where user code caused an HTTP error. For example, attempt to send
// a 1xx response code, or a request with a non-absolute URI. This can also be caused by
// an unexpected header: both `content-length` and `transfer-encoding`, for example.
#define FASTLY_HOST_ERROR_HTTP_USER 8
// HTTP incomplete message error.
// This can be thrown when a stream ended unexpectedly.
#define FASTLY_HOST_ERROR_HTTP_INCOMPLETE 9
// A `None` error.
// This status code is used to indicate when an optional value did not exist, as opposed to
// an empty value.
// Note, this value should no longer be used, as we have explicit optional types now.
#define FASTLY_HOST_ERROR_OPTIONAL_NONE 10
// Message head too large.
#define FASTLY_HOST_ERROR_HTTP_HEAD_TOO_LARGE 11
// Invalid HTTP status.
#define FASTLY_HOST_ERROR_HTTP_INVALID_STATUS 12
// Limit exceeded
//
// This is returned when an attempt to allocate a resource has exceeded the maximum number of
// resources permitted. For example, creating too many response handles.
#define FASTLY_HOST_ERROR_LIMIT_EXCEEDED 13

typedef uint8_t fastly_host_http_send_error_detail_tag;

// The send-error-detail struct has not been populated.
#define FASTLY_HOST_HTTP_SEND_ERROR_DETAIL_TAG_UNINITIALIZED 0
// There was no send error.
#define FASTLY_HOST_HTTP_SEND_ERROR_DETAIL_TAG_OK 1
// The system encountered a timeout when trying to find an IP address for the backend
// hostname.
#define FASTLY_HOST_HTTP_SEND_ERROR_DETAIL_TAG_DNS_TIMEOUT 2
// The system encountered a DNS error when trying to find an IP address for the backend
// hostname. The fields dns-error-rcode and dns-error-info-code may be set in the
// send-error-detail.
#define FASTLY_HOST_HTTP_SEND_ERROR_DETAIL_TAG_DNS_ERROR 3
// The system cannot determine which backend to use, or the specified backend was invalid.
#define FASTLY_HOST_HTTP_SEND_ERROR_DETAIL_TAG_DESTINATION_NOT_FOUND 4
// The system considers the backend to be unavailable e.g., recent attempts to communicate
// with it may have failed, or a health check may indicate that it is down.
#define FASTLY_HOST_HTTP_SEND_ERROR_DETAIL_TAG_DESTINATION_UNAVAILABLE 5
// The system cannot find a route to the next-hop IP address.
#define FASTLY_HOST_HTTP_SEND_ERROR_DETAIL_TAG_DESTINATION_IP_UNROUTABLE 6
// The system's connection to the backend was refused.
#define FASTLY_HOST_HTTP_SEND_ERROR_DETAIL_TAG_CONNECTION_REFUSED 7
// The system's connection to the backend was closed before a complete response was
// received.
#define FASTLY_HOST_HTTP_SEND_ERROR_DETAIL_TAG_CONNECTION_TERMINATED 8
// The system's attempt to open a connection to the backend timed out.
#define FASTLY_HOST_HTTP_SEND_ERROR_DETAIL_TAG_CONNECTION_TIMEOUT 9
// The system is configured to limit the number of connections it has to the backend, and
// that limit has been exceeded.
#define FASTLY_HOST_HTTP_SEND_ERROR_DETAIL_TAG_CONNECTION_LIMIT_REACHED 10
// The system encountered an error when verifying the certificate presented by the backend.
#define FASTLY_HOST_HTTP_SEND_ERROR_DETAIL_TAG_TLS_CERTIFICATE_ERROR 11
// The system encountered an error with the backend TLS configuration.
#define FASTLY_HOST_HTTP_SEND_ERROR_DETAIL_TAG_TLS_CONFIGURATION_ERROR 12
// The system received an incomplete response to the request from the backend.
#define FASTLY_HOST_HTTP_SEND_ERROR_DETAIL_TAG_HTTP_INCOMPLETE_RESPONSE 13
// The system received a response to the request whose header section was considered too
// large.
#define FASTLY_HOST_HTTP_SEND_ERROR_DETAIL_TAG_HTTP_RESPONSE_HEADER_SECTION_TOO_LARGE 14
// The system received a response to the request whose body was considered too large.
#define FASTLY_HOST_HTTP_SEND_ERROR_DETAIL_TAG_HTTP_RESPONSE_BODY_TOO_LARGE 15
// The system reached a configured time limit waiting for the complete response.
#define FASTLY_HOST_HTTP_SEND_ERROR_DETAIL_TAG_HTTP_RESPONSE_TIMEOUT 16
// The system received a response to the request whose status code or reason phrase was
// invalid.
#define FASTLY_HOST_HTTP_SEND_ERROR_DETAIL_TAG_HTTP_RESPONSE_STATUS_INVALID 17
// The process of negotiating an upgrade of the HTTP version between the system and the
// backend failed.
#define FASTLY_HOST_HTTP_SEND_ERROR_DETAIL_TAG_HTTP_UPGRADE_FAILED 18
// The system encountered an HTTP protocol error when communicating with the backend. This
// error will only be used when a more specific one is not defined.
#define FASTLY_HOST_HTTP_SEND_ERROR_DETAIL_TAG_HTTP_PROTOCOL_ERROR 19
// An invalid cache key was provided for the request.
#define FASTLY_HOST_HTTP_SEND_ERROR_DETAIL_TAG_HTTP_REQUEST_CACHE_KEY_INVALID 20
// An invalid URI was provided for the request.
#define FASTLY_HOST_HTTP_SEND_ERROR_DETAIL_TAG_HTTP_REQUEST_URI_INVALID 21
// The system encountered an unexpected internal error.
#define FASTLY_HOST_HTTP_SEND_ERROR_DETAIL_TAG_INTERNAL_ERROR 22
// The system received a TLS alert from the backend. The field tls-alert-id may be set in
// the send-error-detail.
#define FASTLY_HOST_HTTP_SEND_ERROR_DETAIL_TAG_TLS_ALERT_RECEIVED 23
// The system encountered a TLS error when communicating with the backend, either during
// the handshake or afterwards.
#define FASTLY_HOST_HTTP_SEND_ERROR_DETAIL_TAG_TLS_PROTOCOL_ERROR 24

// Mask representing which fields are understood by the guest, and which have been set by the host.
//
// When the guest calls hostcalls with a mask, it should set every bit in the mask that corresponds
// to a defined flag. This signals the host to write only to fields with a set bit, allowing
// forward compatibility for existing guest programs even after new fields are added to the struct.
typedef uint8_t fastly_host_http_send_error_detail_mask;

#define FASTLY_HOST_HTTP_SEND_ERROR_DETAIL_MASK_RESERVED (1 << 0)
#define FASTLY_HOST_HTTP_SEND_ERROR_DETAIL_MASK_DNS_ERROR_RCODE (1 << 1)
#define FASTLY_HOST_HTTP_SEND_ERROR_DETAIL_MASK_DNS_ERROR_INFO_CODE (1 << 2)
#define FASTLY_HOST_HTTP_SEND_ERROR_DETAIL_MASK_TLS_ALERT_ID (1 << 3)

typedef struct fastly_host_http_send_error_detail {
  fastly_host_http_send_error_detail_tag tag;
  fastly_host_http_send_error_detail_mask mask;
  uint16_t dns_error_rcode;
  uint16_t dns_error_info_code;
  uint8_t tls_alert_id;
} fastly_host_http_send_error_detail;

typedef uint32_t fastly_kv_error;

// The values need to match https://docs.rs/fastly-sys/0.10.5/src/fastly_sys/lib.rs.html#111
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
#define BACKEND_CONFIG_DONT_POOL (1u << 12)
#define BACKEND_CONFIG_CLIENT_CERT (1u << 13)
#define BACKEND_CONFIG_GRPC (1u << 14)
#define BACKEND_CONFIG_KEEPALIVE (1u << 15)

typedef enum BACKEND_HEALTH {
  UNKNOWN = 0,
  HEALTHY = 1,
  UNHEALTHY = 2,
} BACKEND_HEALTH;

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
  const char *client_certificate;
  uint32_t client_certificate_len;
  uint32_t client_key;
  uint32_t http_keepalive_time_ms;
  uint32_t tcp_keepalive_enable;
  uint32_t tcp_keepalive_interval_secs;
  uint32_t tcp_keepalive_probes;
  uint32_t tcp_keepalive_time_secs;
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

typedef uint32_t req_inspect_config_options_mask;

#define FASTLY_HOST_HTTP_REQ_INSPECT_CONFIG_OPTIONS_MASK_RESERVED (1 << 0);
#define FASTLY_HOST_HTTP_REQ_INSPECT_CONFIG_OPTIONS_MASK_CORP (1 << 1);
#define FASTLY_HOST_HTTP_REQ_INSPECT_CONFIG_OPTIONS_MASK_WORKSPACE (1 << 2);
#define FASTLY_HOST_HTTP_REQ_INSPECT_CONFIG_OPTIONS_MASK_OVERRIDE_CLIENT_IP (1 << 3);

WASM_IMPORT("fastly_abi", "init")
int init(uint64_t abi_version);

// Module fastly_http_body
WASM_IMPORT("fastly_http_body", "append")
int body_append(uint32_t dst_handle, uint32_t src_handle);

WASM_IMPORT("fastly_http_body", "new")
int body_new(uint32_t *handle_out);

WASM_IMPORT("fastly_http_body", "read")
int body_read(uint32_t body_handle, uint8_t *buf, size_t buf_len, size_t *nread);

WASM_IMPORT("fastly_http_body", "write")
int body_write(uint32_t body_handle, const uint8_t *buf, size_t buf_len, BodyWriteEnd end,
               size_t *nwritten);

WASM_IMPORT("fastly_http_body", "close")
int body_close(uint32_t body_handle);

WASM_IMPORT("fastly_http_body", "abandon")
int body_abandon(uint32_t body_handle);

WASM_IMPORT("fastly_http_body", "trailer_append")
int body_trailer_append(uint32_t body_handle, const uint8_t *name, size_t name_len,
                        const uint8_t *value, size_t value_len);

WASM_IMPORT("fastly_http_body", "trailer_names_get")
int body_trailer_names_get(uint32_t body_handle, char *buf, size_t buf_len, uint32_t cursor,
                           uint32_t *ending_cursor, size_t *nwritten);

WASM_IMPORT("fastly_http_body", "trailer_value_get")
int body_trailer_value_get(uint32_t body_handle, const uint8_t *name, size_t name_len, char *value,
                           size_t value_max_len, size_t *nwritten);

WASM_IMPORT("fastly_http_body", "trailer_values_get")
int body_trailer_values_get(uint32_t body_handle, const uint8_t *name, size_t name_len, char *buf,
                            size_t buf_len, uint32_t cursor, uint32_t *ending_cursor,
                            size_t *nwritten);

WASM_IMPORT("fastly_http_body", "known_length")
int body_known_length(uint32_t body_handle, uint64_t *length);

// Module fastly_http_cache
// HTTP Cache handle type
typedef uint32_t fastly_http_cache_handle;

// HTTP Cache specific types
typedef uint32_t fastly_is_cacheable;
typedef uint32_t fastly_is_sensitive;

// HTTP storage action enum
typedef uint8_t fastly_http_storage_action;
#define FASTLY_HTTP_STORAGE_ACTION_INSERT 0
#define FASTLY_HTTP_STORAGE_ACTION_UPDATE 1
#define FASTLY_HTTP_STORAGE_ACTION_DO_NOT_STORE 2
#define FASTLY_HTTP_STORAGE_ACTION_RECORD_UNCACHEABLE 3

// HTTP Cache lookup options
typedef struct __attribute__((aligned(4))) fastly_http_cache_lookup_options {
  const char *override_key;
  size_t override_key_len;
} fastly_http_cache_lookup_options;

// HTTP Cache lookup options mask
#define FASTLY_HTTP_CACHE_LOOKUP_OPTIONS_MASK_RESERVED (1 << 0)
#define FASTLY_HTTP_CACHE_LOOKUP_OPTIONS_MASK_OVERRIDE_KEY (1 << 1)
#define FASTLY_HTTP_CACHE_LOOKUP_OPTIONS_MASK_BACKEND_NAME (1 << 2)
#define FASTLY_HTTP_CACHE_LOOKUP_OPTIONS_MASK_ACCEPT_STALE_IF_ERROR (1 << 3)

// HTTP Cache write options
typedef struct __attribute__((aligned(8))) fastly_http_cache_write_options {
  uint64_t max_age_ns;
  const char *vary_rule;
  size_t vary_rule_len;
  uint64_t initial_age_ns;
  uint64_t stale_while_revalidate_ns;
  const char *surrogate_keys;
  size_t surrogate_keys_len;
  uint64_t length;
  uint64_t stale_if_error_ns;
} fastly_http_cache_write_options;

// HTTP Cache write options mask
#define FASTLY_HTTP_CACHE_WRITE_OPTIONS_MASK_RESERVED (1 << 0)
#define FASTLY_HTTP_CACHE_WRITE_OPTIONS_MASK_VARY_RULE (1 << 1)
#define FASTLY_HTTP_CACHE_WRITE_OPTIONS_MASK_INITIAL_AGE_NS (1 << 2)
#define FASTLY_HTTP_CACHE_WRITE_OPTIONS_MASK_STALE_WHILE_REVALIDATE_NS (1 << 3)
#define FASTLY_HTTP_CACHE_WRITE_OPTIONS_MASK_SURROGATE_KEYS (1 << 4)
#define FASTLY_HTTP_CACHE_WRITE_OPTIONS_MASK_LENGTH (1 << 5)
#define FASTLY_HTTP_CACHE_WRITE_OPTIONS_MASK_SENSITIVE_DATA (1 << 6)

// HTTP Cache host calls
WASM_IMPORT("fastly_http_cache", "is_request_cacheable")
int http_cache_is_request_cacheable(uint32_t req_handle, uint32_t *is_cacheable_out);

WASM_IMPORT("fastly_http_cache", "get_suggested_cache_key")
int http_cache_get_suggested_cache_key(uint32_t req_handle, char *key_out, size_t key_out_len,
                                       size_t *nwritten_out);

WASM_IMPORT("fastly_http_cache", "lookup")
int http_cache_lookup(uint32_t req_handle, uint32_t options_mask,
                      fastly_http_cache_lookup_options *options, uint32_t *handle_out);

WASM_IMPORT("fastly_http_cache", "transaction_lookup")
int http_cache_transaction_lookup(uint32_t req_handle, uint32_t options_mask,
                                  fastly_http_cache_lookup_options *options, uint32_t *handle_out);

WASM_IMPORT("fastly_http_cache", "transaction_insert")
int http_cache_transaction_insert(uint32_t handle, uint32_t resp_handle, uint32_t options_mask,
                                  fastly_http_cache_write_options *options,
                                  uint32_t *body_handle_out);

WASM_IMPORT("fastly_http_cache", "transaction_insert_and_stream_back")
int http_cache_transaction_insert_and_stream_back(uint32_t handle, uint32_t resp_handle,
                                                  uint32_t options_mask,
                                                  fastly_http_cache_write_options *options,
                                                  uint32_t *body_handle_out,
                                                  uint32_t *cache_handle_out);

WASM_IMPORT("fastly_http_cache", "transaction_update")
int http_cache_transaction_update(uint32_t handle, uint32_t resp_handle, uint32_t options_mask,
                                  fastly_http_cache_write_options *options);

WASM_IMPORT("fastly_http_cache", "transaction_update_and_return_fresh")
int http_cache_transaction_update_and_return_fresh(uint32_t handle, uint32_t resp_handle,
                                                   uint32_t options_mask,
                                                   fastly_http_cache_write_options *options,
                                                   uint32_t *fresh_handle_out);

WASM_IMPORT("fastly_http_cache", "transaction_record_not_cacheable")
int http_cache_transaction_record_not_cacheable(uint32_t handle, uint32_t options_mask,
                                                fastly_http_cache_write_options *options);

WASM_IMPORT("fastly_http_cache", "transaction_abandon")
int http_cache_transaction_abandon(uint32_t handle);

WASM_IMPORT("fastly_http_cache", "transaction_broadcast_cancel")
int http_cache_transaction_broadcast_cancel(uint32_t handle);

WASM_IMPORT("fastly_http_cache", "close")
int http_cache_close(uint32_t handle);

WASM_IMPORT("fastly_http_cache", "get_suggested_backend_request")
int http_cache_get_suggested_backend_request(uint32_t handle, uint32_t *req_handle_out);

WASM_IMPORT("fastly_http_cache", "get_suggested_cache_options")
int http_cache_get_suggested_cache_options(uint32_t handle, uint32_t response_handle,
                                           uint32_t options_mask,
                                           fastly_http_cache_write_options *options,
                                           uint32_t *options_mask_out,
                                           fastly_http_cache_write_options *options_out);

WASM_IMPORT("fastly_http_cache", "prepare_response_for_storage")
int http_cache_prepare_response_for_storage(uint32_t handle, uint32_t response_handle,
                                            uint8_t *storage_action_out,
                                            uint32_t *updated_resp_handle_out);

WASM_IMPORT("fastly_http_cache", "get_found_response")
int http_cache_get_found_response(uint32_t handle, uint32_t transform_for_client,
                                  uint32_t *resp_handle_out, uint32_t *body_handle_out);

WASM_IMPORT("fastly_http_cache", "get_any_response")
int http_cache_get_any_response(uint32_t handle, uint32_t transform_for_client,
                               uint32_t *resp_handle_out, uint32_t *body_handle_out);

WASM_IMPORT("fastly_http_cache", "get_state")
int http_cache_get_state(uint32_t handle, uint8_t *state_out);

WASM_IMPORT("fastly_http_cache", "get_length")
int http_cache_get_length(uint32_t handle, uint64_t *length_out);

WASM_IMPORT("fastly_http_cache", "get_max_age_ns")
int http_cache_get_max_age_ns(uint32_t handle, uint64_t *max_age_ns_out);

WASM_IMPORT("fastly_http_cache", "get_stale_while_revalidate_ns")
int http_cache_get_stale_while_revalidate_ns(uint32_t handle, uint64_t *swr_ns_out);

WASM_IMPORT("fastly_http_cache", "get_stale_if_error_ns")
int http_cache_get_stale_if_error_ns(uint32_t handle, uint64_t *sie_ns_out);

WASM_IMPORT("fastly_http_cache", "get_age_ns")
int http_cache_get_age_ns(uint32_t handle, uint64_t *age_ns_out);

WASM_IMPORT("fastly_http_cache", "get_hits")
int http_cache_get_hits(uint32_t handle, uint64_t *hits_out);

WASM_IMPORT("fastly_http_cache", "get_sensitive_data")
int http_cache_get_sensitive_data(uint32_t handle, uint32_t *is_sensitive_out);

WASM_IMPORT("fastly_http_cache", "get_surrogate_keys")
int http_cache_get_surrogate_keys(uint32_t handle, char *surrogate_keys_out,
                                  size_t surrogate_keys_out_len, size_t *nwritten_out);

WASM_IMPORT("fastly_http_cache", "get_vary_rule")
int http_cache_get_vary_rule(uint32_t handle, char *vary_rule_out, size_t vary_rule_out_len,
                             size_t *nwritten_out);

// Module fastly_log
WASM_IMPORT("fastly_log", "endpoint_get")
int log_endpoint_get(const char *name, size_t name_len, uint32_t *endpoint_handle);

WASM_IMPORT("fastly_log", "write")
int log_write(uint32_t endpoint_handle, const char *msg, size_t msg_len, size_t *nwritten);

// Module fastly_http_req
WASM_IMPORT("fastly_http_req", "register_dynamic_backend")
int req_register_dynamic_backend(const char *name_prefix, size_t name_prefix_len,
                                 const char *target, size_t target_len,
                                 uint32_t backend_config_mask,
                                 DynamicBackendConfig *backend_configuration);

WASM_IMPORT("fastly_http_req", "body_downstream_get")
int req_body_downstream_get(uint32_t *req_handle_out, uint32_t *body_handle_out);

WASM_IMPORT("fastly_http_req", "redirect_to_grip_proxy_v2")
int req_redirect_to_grip_proxy_v2(uint32_t req_handle, const char *backend_name,
                                  size_t backend_name_len);

WASM_IMPORT("fastly_http_req", "redirect_to_websocket_proxy_v2")
int req_redirect_to_websocket_proxy_v2(uint32_t req_handle, const char *backend_name,
                                       size_t backend_name_len);

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
int req_cache_override_set(uint32_t req_handle, int tag, uint32_t ttl,
                           uint32_t stale_while_revalidate);

WASM_IMPORT("fastly_http_req", "cache_override_v2_set")
int req_cache_override_v2_set(uint32_t req_handle, int tag, uint32_t ttl,
                              uint32_t stale_while_revalidate, const char *surrogate_key,
                              size_t surrogate_key_len);

#define FASTLY_HOST_CONTENT_ENCODINGS_GZIP (1 << 0)

WASM_IMPORT("fastly_http_req", "auto_decompress_response_set")
int req_auto_decompress_response_set(uint32_t req_handle, int tag);

/**
 * `octets` must be a 16-byte array.
 * If, after a successful call, `nwritten` == 4, the value in `octets` is an IPv4 address.
 * Otherwise, if `nwritten` will is `16`, the value in `octets` is an IPv6 address.
 * Otherwise, `nwritten` will be `0`, and no address is available.
 */
WASM_IMPORT("fastly_http_req", "downstream_client_ip_addr")
int req_downstream_client_ip_addr_get(uint8_t *octets, size_t *nwritten);

WASM_IMPORT("fastly_http_req", "downstream_server_ip_addr")
int req_downstream_server_ip_addr_get(uint8_t *octets, size_t *nwritten);

WASM_IMPORT("fastly_http_req", "downstream_tls_cipher_openssl_name")
int req_downstream_tls_cipher_openssl_name(char *ret, size_t ret_len, size_t *nwritten);

WASM_IMPORT("fastly_http_req", "downstream_tls_protocol")
int req_downstream_tls_protocol(char *ret, size_t ret_len, size_t *nwritten);

WASM_IMPORT("fastly_http_req", "downstream_tls_client_hello")
int req_downstream_tls_client_hello(uint8_t *ret, size_t ret_len, size_t *nwritten);

WASM_IMPORT("fastly_http_req", "downstream_tls_raw_client_certificate")
int req_downstream_tls_raw_client_certificate(uint8_t *ret, size_t ret_len, size_t *nwritten);

WASM_IMPORT("fastly_http_req", "downstream_tls_ja3_md5")
int req_downstream_tls_ja3_md5(uint8_t *ret, size_t *nwritten);

WASM_IMPORT("fastly_http_req", "downstream_tls_ja4")
int req_downstream_tls_ja4(uint8_t *ret, size_t ret_len, size_t *nwritten);

WASM_IMPORT("fastly_http_req", "downstream_client_h2_fingerprint")
int req_downstream_client_h2_fingerprint(uint8_t *ret, size_t ret_len, size_t *nwritten);

WASM_IMPORT("fastly_http_req", "downstream_client_oh_fingerprint")
int req_downstream_client_oh_fingerprint(uint8_t *ret, size_t ret_len, size_t *nwritten);

WASM_IMPORT("fastly_http_req", "new")
int req_new(uint32_t *req_handle_out);

WASM_IMPORT("fastly_http_req", "header_names_get")
int req_header_names_get(uint32_t req_handle, uint8_t *buf, size_t buf_len, uint32_t cursor,
                         int64_t *ending_cursor, size_t *nwritten);

WASM_IMPORT("fastly_http_req", "original_header_names_get")
int req_original_header_names_get(uint8_t *buf, size_t buf_len, uint32_t cursor,
                                  int64_t *ending_cursor, size_t *nwritten);

WASM_IMPORT("fastly_http_req", "original_header_count")
int req_original_header_count(uint32_t *count);

WASM_IMPORT("fastly_http_req", "header_value_get")
int req_header_value_get(uint32_t req_handle, const char *name, size_t name_len, uint8_t *value,
                         size_t value_max_len, size_t *nwritten);

WASM_IMPORT("fastly_http_req", "header_values_get")
int req_header_values_get(uint32_t req_handle, const char *name, size_t name_len, uint8_t *buf,
                          size_t buf_len, uint32_t cursor, int64_t *ending_cursor,
                          size_t *nwritten);

WASM_IMPORT("fastly_http_req", "header_insert")
int req_header_insert(uint32_t req_handle, const char *name, size_t name_len, const uint8_t *value,
                      size_t value_len);

WASM_IMPORT("fastly_http_req", "header_append")
int req_header_append(uint32_t req_handle, const char *name, size_t name_len, const uint8_t *value,
                      size_t value_len);

WASM_IMPORT("fastly_http_req", "header_remove")
int req_header_remove(uint32_t req_handle, const char *name, size_t name_len);

WASM_IMPORT("fastly_http_req", "method_get")
int req_method_get(uint32_t req_handle, char *method, size_t method_max_len, size_t *nwritten);

WASM_IMPORT("fastly_http_req", "method_set")
int req_method_set(uint32_t req_handle, const char *method, size_t method_len);

WASM_IMPORT("fastly_http_req", "uri_get")
int req_uri_get(uint32_t req_handle, char *uri, size_t uri_max_len, size_t *nwritten);

WASM_IMPORT("fastly_http_req", "uri_set")
int req_uri_set(uint32_t req_handle, const char *uri, size_t uri_len);

WASM_IMPORT("fastly_http_req", "version_get")
int req_version_get(uint32_t req_handle, uint32_t *version);

WASM_IMPORT("fastly_http_req", "version_set")
int req_version_set(uint32_t req_handle, uint32_t version);

WASM_IMPORT("fastly_http_req", "framing_headers_mode_set")
int req_framing_headers_mode_set(uint32_t req_handle, uint32_t mode);

WASM_IMPORT("fastly_http_req", "send")
int req_send(uint32_t req_handle, uint32_t body_handle, const uint8_t *backend, size_t backend_len,
             uint32_t *resp_handle_out, uint32_t *resp_body_handle_out);

WASM_IMPORT("fastly_http_req", "send_async")
int req_send_async(uint32_t req_handle, uint32_t body_handle, const char *backend,
                   size_t backend_len, uint32_t *pending_req_out);

WASM_IMPORT("fastly_http_req", "send_async_streaming")
int req_send_async_streaming(uint32_t req_handle, uint32_t body_handle, const char *backend,
                             size_t backend_len, uint32_t *pending_req_out);

WASM_IMPORT("fastly_http_req", "send_async_v2")
int req_send_async_v2(uint32_t req_handle, uint32_t body_handle, const char *backend,
                      size_t backend_len, uint32_t streaming, uint32_t *pending_req_out);

WASM_IMPORT("fastly_http_req", "pending_req_poll")
int req_pending_req_poll(uint32_t req_handle, uint32_t *is_done_out, uint32_t *resp_handle_out,
                         uint32_t *resp_body_handle_out);

WASM_IMPORT("fastly_http_req", "pending_req_select")
int req_pending_req_select(uint32_t req_handles[], size_t req_handles_len, uint32_t *done_idx_out,
                           uint32_t *resp_handle_out, uint32_t *resp_body_handle_out);

WASM_IMPORT("fastly_http_req", "pending_req_wait")
int req_pending_req_wait(uint32_t req_handle, uint32_t *resp_handle_out,
                         uint32_t *resp_body_handle_out);

WASM_IMPORT("fastly_http_req", "pending_req_wait_v2")
int req_pending_req_wait_v2(uint32_t req_handle,
                            fastly_host_http_send_error_detail *send_error_detail,
                            uint32_t *resp_handle_out, uint32_t *resp_body_handle_out);

WASM_IMPORT("fastly_http_req", "inspect")
int req_inspect(uint32_t req_handle, uint32_t body_handle,
                req_inspect_config_options_mask config_options_mask,
                fastly_host_http_inspect_options *config, uint8_t *inspect_res_buf,
                uint32_t inspect_res_buf_len, size_t *nwritten_out);

// Module fastly_http_resp
WASM_IMPORT("fastly_http_resp", "new")
int resp_new(uint32_t *resp_handle_out);

WASM_IMPORT("fastly_http_resp", "header_names_get")
int resp_header_names_get(uint32_t resp_handle, uint8_t *buf, size_t buf_len, uint32_t cursor,
                          int64_t *ending_cursor, size_t *nwritten);

WASM_IMPORT("fastly_http_resp", "header_values_get")
int resp_header_values_get(uint32_t resp_handle, const char *name, size_t name_len, uint8_t *buf,
                           size_t buf_len, uint32_t cursor, int64_t *ending_cursor,
                           size_t *nwritten);

WASM_IMPORT("fastly_http_req", "header_values_set")
int req_header_values_set(uint32_t req_handle, const char *name, size_t name_len,
                          const uint8_t *values, size_t values_len);

WASM_IMPORT("fastly_http_resp", "header_insert")
int resp_header_insert(uint32_t resp_handle, const char *name, size_t name_len,
                       const uint8_t *value, size_t value_len);

WASM_IMPORT("fastly_http_resp", "header_append")
int resp_header_append(uint32_t resp_handle, const char *name, size_t name_len,
                       const uint8_t *value, size_t value_len);

WASM_IMPORT("fastly_http_resp", "header_remove")
int resp_header_remove(uint32_t resp_handle, const char *name, size_t name_len);

WASM_IMPORT("fastly_http_resp", "version_get")
int resp_version_get(uint32_t resp_handle, uint32_t *version_out);

WASM_IMPORT("fastly_http_resp", "send_downstream")
int resp_send_downstream(uint32_t resp_handle, uint32_t body_handle, uint32_t streaming);

WASM_IMPORT("fastly_http_resp", "status_get")
int resp_status_get(uint32_t resp_handle, uint16_t *status_out);

WASM_IMPORT("fastly_http_resp", "status_set")
int resp_status_set(uint32_t resp_handle, uint16_t status);

WASM_IMPORT("fastly_http_resp", "framing_headers_mode_set")
int resp_framing_headers_mode_set(uint32_t resp_handle, uint32_t mode);

WASM_IMPORT("fastly_http_resp", "get_addr_dest_ip")
int resp_ip_get(uint32_t resp_handle, uint8_t *ip_out, size_t *nwritten);

WASM_IMPORT("fastly_http_resp", "get_addr_dest_port")
int resp_port_get(uint32_t resp_handle, uint16_t *port_out);

// Module fastly_dictionary
WASM_IMPORT("fastly_dictionary", "open")
int dictionary_open(const char *name, size_t name_len, uint32_t *dict_handle_out);

WASM_IMPORT("fastly_dictionary", "get")
int dictionary_get(uint32_t dict_handle, const char *key, size_t key_len, char *value,
                   size_t value_max_len, size_t *nwritten);

// Module fastly_config_store
WASM_IMPORT("fastly_config_store", "open")
int config_store_open(const char *name, size_t name_len, uint32_t *dict_handle_out);

WASM_IMPORT("fastly_config_store", "get")
int config_store_get(uint32_t dict_handle, const char *key, size_t key_len, char *value,
                     size_t value_max_len, size_t *nwritten);

// Module fastly_secret_store
WASM_IMPORT("fastly_secret_store", "open")
int secret_store_open(const char *name, size_t name_len, uint32_t *dict_handle_out);

WASM_IMPORT("fastly_secret_store", "get")
int secret_store_get(uint32_t dict_handle, const char *key, size_t key_len,
                     uint32_t *opt_secret_handle_out);

WASM_IMPORT("fastly_secret_store", "plaintext")
int secret_store_plaintext(uint32_t secret_handle, char *buf, size_t buf_len, size_t *nwritten);

WASM_IMPORT("fastly_secret_store", "from_bytes")
int secret_store_from_bytes(char *buf, size_t buf_len, uint32_t *opt_secret_handle_out);

// Module fastly_object_store
WASM_IMPORT("fastly_object_store", "open")
int object_store_open(const char *name, size_t name_len, uint32_t *object_store_handle_out);
WASM_IMPORT("fastly_object_store", "lookup")
int object_store_get(uint32_t object_store_handle, const char *key, size_t key_len,
                     uint32_t *opt_body_handle_out);

WASM_IMPORT("fastly_object_store", "lookup_async")
int object_store_get_async(uint32_t object_store_handle, const char *key, size_t key_len,
                           uint32_t *pending_object_store_lookup_handle_out);

WASM_IMPORT("fastly_object_store", "pending_lookup_wait")
int object_store_pending_lookup_wait(uint32_t handle, uint32_t *handle_out);

WASM_IMPORT("fastly_object_store", "delete_async")
int object_store_delete_async(uint32_t object_store_handle, const char *key, size_t key_len,
                              uint32_t *pending_object_store_lookup_handle_out);

WASM_IMPORT("fastly_object_store", "pending_delete_wait")
int object_store_pending_delete_wait(uint32_t handle);

WASM_IMPORT("fastly_object_store", "insert")
int object_store_insert(uint32_t object_store_handle, const char *key, size_t key_len,
                        uint32_t body_handle);

// Module fastly_kv_store
#define KV_LOOKUP_CONFIG_RESERVED (1u << 0)

typedef struct __attribute__((aligned(4))) KVLookupOptions {
  uint32_t reserved;
} KVLookupOptions;

#define KV_INSERT_CONFIG_RESERVED (1u << 0)
#define KV_INSERT_CONFIG_BACKGROUND_FETCH (1u << 1)
#define KV_INSERT_CONFIG_RESERVED_2 (1u << 2)
#define KV_INSERT_CONFIG_METADATA (1u << 3)
#define KV_INSERT_CONFIG_TIME_TO_LIVE_SEC (1u << 4)
#define KV_INSERT_CONFIG_IF_GENERATION_MATCH (1u << 5)

#define KV_INSERT_MODE_OVERWRITE 0u
#define KV_INSERT_MODE_ADD 1u
#define KV_INSERT_MODE_APPEND 2u
#define KV_INSERT_MODE_PREPEND 3u

typedef struct __attribute__((aligned(4))) KVInsertOptions {
  uint32_t mode;
  uint32_t reserved;
  const uint8_t *metadata;
  uint32_t metadata_len;
  uint32_t time_to_live_sec;
  uint64_t if_generation_match;
} KVInsertOptions;

#define KV_DELETE_CONFIG_RESERVED (1u << 0)

typedef struct __attribute__((aligned(4))) KVDeleteOptions {
  uint32_t reserved;
} KVDeleteOptions;

#define KV_LIST_CONFIG_RESERVED (1u << 0)
#define KV_LIST_CONFIG_CURSOR (1u << 1)
#define KV_LIST_CONFIG_LIMIT (1u << 2)
#define KV_LIST_CONFIG_PREFIX (1u << 3)

#define KV_LIST_MODE_STRONG 0u
#define KV_LIST_MODE_EVENTUAL 1u

typedef struct __attribute__((aligned(4))) KVListOptions {
  uint32_t mode;
  const uint8_t *cursor;
  uint32_t cursor_len;
  uint32_t limit;
  const uint8_t *prefix;
  uint32_t prefix_len;
} KVListOptions;

#define KV_ERROR_UNINITIALIZED 0u
#define KV_ERROR_OK 1u
#define KV_ERROR_BAD_REQUEST 2u
#define KV_ERROR_NOT_FOUND 3u
#define KV_ERROR_PRECONDITION_FAILED 4u
#define KV_ERROR_PAYLOAD_TOO_LARGE 5u
#define KV_ERROR_INTERNAL_ERROR 6u
#define KV_ERROR_TOO_MANY_REQUESTS 7u

WASM_IMPORT("fastly_kv_store", "open")
int kv_store_open(const char *name, size_t name_len, uint32_t *kv_store_handle_out);

WASM_IMPORT("fastly_kv_store", "lookup")
int kv_store_lookup(uint32_t kv_store_handle, const char *key, size_t key_len,
                    uint32_t lookup_options_mask, KVLookupOptions *lookup_options,
                    uint32_t *kv_store_lookup_handle_out);

WASM_IMPORT("fastly_kv_store", "lookup_wait")
int kv_store_lookup_wait(uint32_t kv_store_handle_lookup_handle, uint32_t *body_handle_out,
                         uint8_t *metadata_buf_out, size_t metadata_buf_len, size_t *nwritten_out,
                         uint32_t *generation_out, uint32_t *kv_error_out);

WASM_IMPORT("fastly_kv_store", "lookup_wait_v2")
int kv_store_lookup_wait_v2(uint32_t kv_store_handle_lookup_handle, uint32_t *body_handle_out,
                            uint8_t *metadata_buf_out, size_t metadata_buf_len,
                            size_t *nwritten_out, uint64_t *generation_out, uint32_t *kv_error_out);

WASM_IMPORT("fastly_kv_store", "insert")
int kv_store_insert(uint32_t kv_store_handle, const char *key, size_t key_len, uint32_t body_handle,
                    uint32_t insert_options_mask, KVInsertOptions *insert_options,
                    uint32_t *kv_store_insert_handle_out);

WASM_IMPORT("fastly_kv_store", "insert_wait")
int kv_store_insert_wait(uint32_t kv_store_insert_handle, uint32_t *kv_error_out);

WASM_IMPORT("fastly_kv_store", "delete")
int kv_store_delete(uint32_t handle, const char *key, size_t key_len, uint32_t delete_options_mask,
                    KVDeleteOptions *delete_options, uint32_t *kv_store_delete_handle_out);

WASM_IMPORT("fastly_kv_store", "delete_wait")
int kv_store_delete_wait(uint32_t kv_store_delete_handle, uint32_t *kv_error_out);

WASM_IMPORT("fastly_kv_store", "list")
int kv_store_list(uint32_t kv_store_handle, uint32_t list_options_mask, KVListOptions *list_options,
                  uint32_t *kv_store_list_handle_out);

WASM_IMPORT("fastly_kv_store", "list_wait")
int kv_store_list_wait(uint32_t kv_store_list_handle, uint32_t *body_handle_out,
                       uint32_t *kv_error_out);

WASM_IMPORT("fastly_geo", "lookup")
int geo_lookup(const uint8_t *addr_octets, size_t addr_len, char *buf, size_t buf_len,
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
int async_select(uint32_t handles[], size_t handles_len, uint32_t timeout_ms,
                 uint32_t *ready_idx_out);

// Returns 1 if the given async item is "ready" for its associated I/O action, 0 otherwise.
//
// If an object is ready, the I/O action is guaranteed to complete without blocking.
//
// Valid object handles includes bodies and pending requests. See the `async_item_handle`
// definition for more details, including what I/O actions are associated with each handle
// type.
WASM_IMPORT("fastly_async_io", "is_ready")
int async_is_ready(uint32_t handle, uint32_t *is_ready_out);

typedef struct __attribute__((aligned(4))) PurgeOptions {
  uint8_t *ret_buf_ptr;
  size_t ret_buf_len;
  size_t *ret_buf_nwritten_out;
} PurgeOptions;

#define FASTLY_HOST_PURGE_OPTIONS_MASK_SOFT_PURGE (1 << 0)
#define FASTLY_HOST_PURGE_OPTIONS_MASK_RET_BUF (1 << 1)

WASM_IMPORT("fastly_purge", "purge_surrogate_key")
int purge_surrogate_key(char *surrogate_key, size_t surrogate_key_len, uint32_t options_mask,
                        PurgeOptions *purge_options);

#define FASTLY_CACHE_LOOKUP_OPTIONS_MASK_RESERVED (1 << 0)
#define FASTLY_CACHE_LOOKUP_OPTIONS_MASK_REQUEST_HEADERS (1 << 1)

// Extensible options for cache lookup operations currently used for both `lookup` and
// `transaction_lookup`.
typedef struct __attribute__((aligned(4))) fastly_host_cache_lookup_options {
  // * A full request handle, but used only for its headers
  uint32_t request_headers;
} fastly_host_cache_lookup_options;

// Configuration for several hostcalls that write to the cache:
// - `insert`
// - `transaction-insert`
// - `transaction-insert-and-stream-back`
// - `transaction-update`
//
// Some options are only allowed for certain of these hostcalls see `cache-write-options-mask`.
typedef struct fastly_host_cache_write_options {
  // this is a required field there's no flag for it
  uint64_t max_age_ns;
  // a full request handle, but used only for its headers
  uint32_t request_headers;
  // a list of header names separated by spaces
  fastly_world_string vary_rule;
  // The initial age of the object in nanoseconds (default: 0).
  //
  // This age is used to determine the freshness lifetime of the object as well as to
  // prioritize which variant to return if a subsequent lookup matches more than one vary rule
  uint64_t initial_age_ns;
  uint64_t stale_while_revalidate_ns;
  // a list of surrogate keys separated by spaces
  fastly_world_string surrogate_keys;
  uint64_t length;
  fastly_world_list_u8 user_metadata;
  bool sensitive_data;
} fastly_host_cache_write_options;

// a cached object was found
#define FASTLY_HOST_CACHE_LOOKUP_STATE_FOUND (1 << 0)
// the cached object is valid to use (implies found)
#define FASTLY_HOST_CACHE_LOOKUP_STATE_USABLE (1 << 1)
// the cached object is stale (but may or may not be valid to use)
#define FASTLY_HOST_CACHE_LOOKUP_STATE_STALE (1 << 2)
// this client is requested to insert or revalidate an object
#define FASTLY_HOST_CACHE_LOOKUP_STATE_MUST_INSERT_OR_UPDATE (1 << 3)
// a cached object was found and it is only usable if synchronous revalidation fails
#define FASTLY_HOST_CACHE_LOOKUP_STATE_USABLE_IF_ERROR (1 << 4)
// in another client, a synchronous revalidation has failed for this object
#define FASTLY_HOST_CACHE_LOOKUP_STATE_COLLAPSE_ERROR (1 << 5)

WASM_IMPORT("fastly_cache", "lookup")
int cache_lookup(char *cache_key, size_t cache_key_len, uint32_t options_mask,
                 fastly_host_cache_lookup_options *options, uint32_t *ret);

typedef struct __attribute__((aligned(8))) {
  uint64_t max_age_ns;
  uint32_t request_headers;
  const uint8_t *vary_rule_ptr;
  size_t vary_rule_len;
  uint64_t initial_age_ns;
  uint64_t stale_while_revalidate_ns;
  const uint8_t *surrogate_keys_ptr;
  size_t surrogate_keys_len;
  uint64_t length;
  const uint8_t *user_metadata_ptr;
  size_t user_metadata_len;
} CacheWriteOptions;

WASM_IMPORT("fastly_cache", "insert")
int cache_insert(char *cache_key, size_t cache_key_len, uint32_t options_mask,
                 CacheWriteOptions *options, uint32_t *ret);

WASM_IMPORT("fastly_cache", "transaction_lookup")
int cache_transaction_lookup(char *cache_key, size_t cache_key_len, uint32_t options_mask,
                             fastly_host_cache_lookup_options *options, uint32_t *ret);

WASM_IMPORT("fastly_cache", "transaction_insert")
int cache_transaction_insert(uint32_t handle, uint32_t options_mask, CacheWriteOptions *options,
                             uint32_t *ret);

WASM_IMPORT("fastly_cache", "transaction_insert_and_stream_back")
int cache_transaction_insert_and_stream_back(uint32_t handle, uint32_t options_mask,
                                             CacheWriteOptions *options, uint32_t *ret_body,
                                             uint32_t *ret_cache);

WASM_IMPORT("fastly_cache", "transaction_update")
int cache_transaction_update(uint32_t handle, uint32_t options_mask, CacheWriteOptions *options);

WASM_IMPORT("fastly_cache", "transaction_cancel")
int cache_transaction_cancel(uint32_t handle);

WASM_IMPORT("fastly_cache", "close")
int cache_close(uint32_t handle);

WASM_IMPORT("fastly_cache", "get_state")
int cache_get_state(uint32_t handle, uint8_t *ret);

WASM_IMPORT("fastly_cache", "get_user_metadata")
int cache_get_user_metadata(uint32_t handle, char *buf, size_t buf_len, size_t *nread);

#define FASTLY_HOST_CACHE_GET_BODY_OPTIONS_MASK_RESERVED (1 << 0)
#define FASTLY_HOST_CACHE_GET_BODY_OPTIONS_MASK_START (1 << 1)
#define FASTLY_HOST_CACHE_GET_BODY_OPTIONS_MASK_END (1 << 2)

typedef struct fastly_host_cache_get_body_options {
  uint64_t start;
  uint64_t end;
} fastly_host_cache_get_body_options;

WASM_IMPORT("fastly_cache", "get_body")
int cache_get_body(uint32_t handle, uint32_t options_mask,
                   fastly_host_cache_get_body_options *options, uint32_t *ret);

// Returns 1 if a backend with this name exists.
WASM_IMPORT("fastly_backend", "exists")
int backend_exists(const char *name, size_t name_len, uint32_t *exists_out);

#define FASTLY_HOST_BACKEND_BACKEND_HEALTH_UNKNOWN 0
#define FASTLY_HOST_BACKEND_BACKEND_HEALTH_HEALTHY 1
#define FASTLY_HOST_BACKEND_BACKEND_HEALTH_UNHEALTHY 2

// Returns 1 if a backend is healthy.
WASM_IMPORT("fastly_backend", "is_healthy")
int backend_is_healthy(const char *name, size_t name_len, uint32_t *is_healthy_out);

WASM_IMPORT("fastly_backend", "is_dynamic")
int backend_is_dynamic(const char *name, size_t name_len, uint32_t *is_dynamic_out);

WASM_IMPORT("fastly_backend", "get_host")
int backend_get_host(const char *name, size_t name_len, uint8_t *value, size_t value_max_len,
                     size_t *nwritten);

WASM_IMPORT("fastly_backend", "get_override_host")
int backend_get_override_host(const char *name, size_t name_len, uint8_t *value,
                              size_t value_max_len, size_t *nwritten);

WASM_IMPORT("fastly_backend", "get_port")
int backend_get_port(const char *name, size_t name_len, uint16_t *port_out);

WASM_IMPORT("fastly_backend", "get_connect_timeout_ms")
int backend_get_connect_timeout_ms(const char *name, size_t name_len, uint32_t *timeout_ms);

WASM_IMPORT("fastly_backend", "get_first_byte_timeout_ms")
int backend_get_first_byte_timeout_ms(const char *name, size_t name_len, uint32_t *timeout_ms);

WASM_IMPORT("fastly_backend", "get_between_bytes_timeout_ms")
int backend_get_between_bytes_timeout_ms(const char *name, size_t name_len, uint32_t *timeout_ms);

WASM_IMPORT("fastly_backend", "get_http_keepalive_time")
int backend_get_http_keepalive_time(const char *name, size_t name_len, uint32_t *timeout_ms);

WASM_IMPORT("fastly_backend", "get_tcp_keepalive_enable")
int backend_get_tcp_keepalive_enable(const char *name, size_t name_len, uint32_t *is_keepalive);

WASM_IMPORT("fastly_backend", "get_tcp_keepalive_interval")
int backend_get_tcp_keepalive_interval(const char *name, size_t name_len, uint32_t *timeout_ms);

WASM_IMPORT("fastly_backend", "get_tcp_keepalive_probes")
int backend_get_tcp_keepalive_probes(const char *name, size_t name_len, uint32_t *probe_count);

WASM_IMPORT("fastly_backend", "get_tcp_keepalive_time")
int backend_get_tcp_keepalive_time(const char *name, size_t name_len, uint32_t *timeout_secs);

WASM_IMPORT("fastly_backend", "is_ssl")
int backend_is_ssl(const char *name, size_t name_len, uint32_t *is_ssl);

WASM_IMPORT("fastly_backend", "get_ssl_min_version")
int backend_get_ssl_min_version(const char *name, size_t name_len, uint32_t *ssl_min_version);

WASM_IMPORT("fastly_backend", "get_ssl_max_version")
int backend_get_ssl_max_version(const char *name, size_t name_len, uint32_t *ssl_max_version);

WASM_IMPORT("fastly_cache", "get_length")
int cache_get_length(uint32_t handle, uint64_t *ret);

WASM_IMPORT("fastly_cache", "get_max_age_ns")
int cache_get_max_age_ns(uint32_t handle, uint64_t *ret);

WASM_IMPORT("fastly_cache", "get_stale_while_revalidate_ns")
int cache_get_stale_while_revalidate_ns(uint32_t handle, uint64_t *ret);

WASM_IMPORT("fastly_cache", "get_age_ns")
int cache_get_age_ns(uint32_t handle, uint64_t *ret);

WASM_IMPORT("fastly_cache", "get_hits")
int cache_get_hits(uint32_t handle, uint64_t *ret);

WASM_IMPORT("fastly_erl", "check_rate")
int check_rate(const char *rc, size_t rc_len, const char *entry, size_t entry_len, uint32_t delta,
               uint32_t window, uint32_t limit, const char *pb, size_t pb_len, uint32_t ttl,
               bool *blocked_out);

WASM_IMPORT("fastly_erl", "ratecounter_increment")
int ratecounter_increment(const char *rc, size_t rc_len, const char *entry, size_t entry_len,
                          uint32_t delta);

WASM_IMPORT("fastly_erl", "ratecounter_lookup_rate")
int ratecounter_lookup_rate(const char *rc, size_t rc_len, const char *entry, size_t entry_len,
                            uint32_t window, uint32_t *rate_out);

WASM_IMPORT("fastly_erl", "ratecounter_lookup_count")
int ratecounter_lookup_count(const char *rc, size_t rc_len, const char *entry, size_t entry_len,
                             uint32_t duration, uint32_t *count_out);

WASM_IMPORT("fastly_erl", "penaltybox_add")
int penaltybox_add(const char *pb, size_t pb_len, const char *entry, size_t entry_len,
                   uint32_t ttl);

WASM_IMPORT("fastly_erl", "penaltybox_has")
int penaltybox_has(const char *pb, size_t pb_len, const char *entry, size_t entry_len,
                   bool *has_out);

WASM_IMPORT("fastly_device_detection", "lookup")
int device_detection_lookup(const char *user_agent, size_t user_agent_len, const char *buf,
                            size_t buf_len, size_t *nwritten);

WASM_IMPORT("fastly_compute_runtime", "get_vcpu_ms")
int compute_get_vcpu_ms(uint64_t *vcpu_ms);

// ACL handle type
typedef uint32_t fastly_acl_handle;

// ACL error enum
typedef uint32_t fastly_acl_error;

#define FASTLY_ACL_ERROR_UNINITIALIZED 0
#define FASTLY_ACL_ERROR_OK 1
#define FASTLY_ACL_ERROR_NO_CONTENT 2
#define FASTLY_ACL_ERROR_TOO_MANY_REQUESTS 3

// ACL host calls
WASM_IMPORT("fastly_acl", "open")
int acl_open(const char *name, size_t name_len, uint32_t *acl_handle_out);

WASM_IMPORT("fastly_acl", "lookup")
int acl_lookup(uint32_t acl_handle, const uint8_t *ip_octets, size_t ip_len,
               uint32_t *body_handle_out, fastly_acl_error *acl_error_out);

typedef struct __attribute__((aligned(8))) fastly_image_optimizer_transform_config {
  const char *sdk_claims_opts;
  size_t sdk_claims_opts_len;
} fastly_image_optimizer_transform_config;

#define FASTLY_IMAGE_OPTIMIZER_RESERVED (1u << 0)
#define FASTLY_IMAGE_OPTIMIZER_SDK_CLAIMS_OPTS (1u << 1)

#define FASTLY_IMAGE_OPTIMIZER_ERROR_TAG_UNINITIALIZED 0
#define FASTLY_IMAGE_OPTIMIZER_ERROR_TAG_OK 1
#define FASTLY_IMAGE_OPTIMIZER_ERROR_TAG_ERROR 2
#define FASTLY_IMAGE_OPTIMIZER_ERROR_TAG_WARNING 3

typedef struct __attribute__((aligned(8))) fastly_image_optimizer_error_detail {
  uint32_t tag;
  const char *message;
  size_t message_len;
} fastly_image_optimizer_error_detail;

WASM_IMPORT("fastly_image_optimizer", "transform_image_optimizer_request")
int image_optimizer_transform_image_optimizer_request(
    uint32_t req_handle, uint32_t body_handle, const char *backend, size_t backend_len,
    int io_transform_config_mask, fastly_image_optimizer_transform_config *io_transform_config,
    fastly_image_optimizer_error_detail *io_error_detail, uint32_t *resp_handle_out,
    uint32_t *resp_body_handle_out);

#define FASTLY_SHIELDING_SHIELD_BACKEND_OPTIONS_RESERVED (1 << 0)
#define FASTLY_SHIELDING_SHIELD_BACKEND_OPTIONS_CACHE_KEY (1 << 1)
#define FASTLY_SHIELDING_SHIELD_BACKEND_OPTIONS_FIRST_BYTE_TIMEOUT (1 << 2)

struct fastly_shielding_shield_backend_config {
  const char *cache_key;
  uint32_t cache_key_len;
  uint32_t first_byte_timeout_ms;
};

WASM_IMPORT("fastly_shielding", "shield_info")
int fastly_shielding_shield_info(const char *name, size_t name_len, char *info_block,
                                 size_t info_block_len, uint32_t *nwritten_out);

WASM_IMPORT("fastly_shielding", "backend_for_shield")
int fastly_shielding_backend_for_shield(
    const char *name, size_t name_len, uint32_t options_mask,
    const fastly_shielding_shield_backend_config *backend_config, char *backend_name,
    size_t backend_name_len, uint32_t *nwritten_out);

#ifdef __cplusplus
} // namespace fastly
} // extern C
#endif
#endif
