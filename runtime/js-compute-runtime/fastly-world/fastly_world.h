// Generated by `wit-bindgen` 0.7.0. DO NOT EDIT!
#ifndef __BINDINGS_FASTLY_WORLD_H
#define __BINDINGS_FASTLY_WORLD_H
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
} fastly_world_string_t;

typedef struct {
  fastly_world_string_t family;
  fastly_world_string_t major;
  fastly_world_string_t minor;
  fastly_world_string_t patch;
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

typedef uint8_t fastly_purge_options_mask_t;

#define FASTLY_PURGE_OPTIONS_MASK_SOFT_PURGE (1 << 0)
#define FASTLY_PURGE_OPTIONS_MASK_RET_BUF (1 << 1)

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

// Do not cache the response to this request, regardless of the origin response's headers.
#define FASTLY_HTTP_CACHE_OVERRIDE_TAG_PASS (1 << 0)
#define FASTLY_HTTP_CACHE_OVERRIDE_TAG_TTL (1 << 1)
#define FASTLY_HTTP_CACHE_OVERRIDE_TAG_STALE_WHILE_REVALIDATE (1 << 2)
#define FASTLY_HTTP_CACHE_OVERRIDE_TAG_PCI (1 << 3)

typedef struct {
  bool is_some;
  fastly_world_string_t val;
} fastly_world_option_string_t;

typedef struct {
  bool is_some;
  uint32_t val;
} fastly_world_option_u32_t;

typedef struct {
  bool is_some;
  float val;
} fastly_world_option_float32_t;

typedef struct {
  // * The name of the organization associated with as_number.
  //       *
  //       * For example, fastly is the value given for IP addresses under AS-54113.
  fastly_world_option_string_t as_name;
  // * [Autonomous system](https://en.wikipedia.org/wiki/Autonomous_system_(Internet)) (AS) number.
  fastly_world_option_u32_t as_number;
  // * The telephone area code associated with an IP address.
  //       *
  //       * These are only available for IP addresses in the United States, its territories, and
  //       Canada.
  fastly_world_option_u32_t area_code;
  // * City or town name.
  fastly_world_option_string_t city;
  // * Connection speed.
  fastly_world_option_string_t conn_speed;
  // * Connection type.
  fastly_world_option_string_t conn_type;
  // * Continent.
  fastly_world_option_string_t continent;
  // * A two-character [ISO 3166-1](https://en.wikipedia.org/wiki/ISO_3166-1) country code for the
  // country associated with an IP address.
  //       *
  //       * The US country code is returned for IP addresses associated with overseas United States
  //       military bases.
  //       *
  //       * These values include subdivisions that are assigned their own country codes in ISO
  //       3166-1. For example, subdivisions NO-21 and NO-22 are presented with the country code SJ
  //       for Svalbard and the Jan Mayen Islands.
  fastly_world_option_string_t country_code;
  // * A three-character [ISO 3166-1 alpha-3](https://en.wikipedia.org/wiki/ISO_3166-1_alpha-3)
  // country code for the country associated with the IP address.
  //       *
  //       * The USA country code is returned for IP addresses associated with overseas United
  //       States military bases.
  fastly_world_option_string_t country_code3;
  // * Country name.
  //       *
  //       * This field is the [ISO 3166-1](https://en.wikipedia.org/wiki/ISO_3166-1) English short
  //       name for a country.
  fastly_world_option_string_t country_name;
  // * Time zone offset from Greenwich Mean Time (GMT) for `city`.
  fastly_world_option_string_t gmt_offset;
  // * Latitude, in units of degrees from the equator.
  //       *
  //       * Values range from -90.0 to +90.0 inclusive, and are based on the [WGS
  //       84](https://en.wikipedia.org/wiki/World_Geodetic_System) coordinate reference system.
  fastly_world_option_float32_t latitude;
  // * Longitude, in units of degrees from the [IERS Reference
  // Meridian](https://en.wikipedia.org/wiki/IERS_Reference_Meridian).
  //       *
  //       * Values range from -180.0 to +180.0 inclusive, and are based on the [WGS
  //       84](https://en.wikipedia.org/wiki/World_Geodetic_System) coordinate reference system.
  fastly_world_option_float32_t longitude;
  // * Metro code, representing designated market areas (DMAs) in the United States.
  fastly_world_option_u32_t metro_code;
  // * The postal code associated with the IP address.
  //       *
  //       * These are available for some IP addresses in Australia, Canada, France, Germany, Italy,
  //       Spain, Switzerland, the United Kingdom, and the United States.
  //       *
  //       * For Canadian postal codes, this is the first 3 characters. For the United Kingdom, this
  //       is the first 2-4 characters (outward code). For countries with alphanumeric postal codes,
  //       this field is a lowercase transliteration.
  fastly_world_option_string_t postal_code;
  // * Client proxy description.
  fastly_world_option_string_t proxy_description;
  // * Client proxy type.
  fastly_world_option_string_t proxy_type;
  // * [ISO 3166-2](https://en.wikipedia.org/wiki/ISO_3166-2) country subdivision code.
  //       *
  //       * For countries with multiple levels of subdivision (for example, nations within the
  //       United Kingdom), this variable gives the more specific subdivision.
  //       *
  //       * This field can be None for countries that do not have ISO country subdivision codes.
  //       For example, None is given for IP addresses assigned to the Åland Islands (country code
  //       AX, illustrated below).
  fastly_world_option_string_t region;
  // * Time zone offset from coordinated universal time (UTC) for `city`.
  fastly_world_option_u32_t utc_offset;
} fastly_geo_data_t;

// Adjust how this requests's framing headers are determined.
typedef uint8_t fastly_framing_headers_mode_t;

#define FASTLY_FRAMING_HEADERS_MODE_AUTOMATIC 0
#define FASTLY_FRAMING_HEADERS_MODE_MANUALLY_FROM_HEADERS 1

typedef uint32_t fastly_fd_t;

typedef uint8_t fastly_error_t;

// Unknown error value.
// It should be an internal error if this is returned.
#define FASTLY_ERROR_UNKNOWN_ERROR 0
// Generic error value.
// This means that some unexpected error occurred during a hostcall.
#define FASTLY_ERROR_GENERIC_ERROR 1
// Invalid argument.
#define FASTLY_ERROR_INVALID_ARGUMENT 2
// Invalid handle.
// Thrown when a handle is not valid. E.G. No dictionary exists with the given name.
#define FASTLY_ERROR_BAD_HANDLE 3
// Buffer length error.
// Thrown when a buffer is too long.
#define FASTLY_ERROR_BUFFER_LEN 4
// Unsupported operation error.
// This error is thrown when some operation cannot be performed, because it is not supported.
#define FASTLY_ERROR_UNSUPPORTED 5
// Alignment error.
// This is thrown when a pointer does not point to a properly aligned slice of memory.
#define FASTLY_ERROR_BAD_ALIGN 6
// Invalid HTTP error.
// This can be thrown when a method, URI, header, or status is not valid. This can also
// be thrown if a message head is too large.
#define FASTLY_ERROR_HTTP_INVALID 7
// HTTP user error.
// This is thrown in cases where user code caused an HTTP error. For example, attempt to send
// a 1xx response code, or a request with a non-absolute URI. This can also be caused by
// an unexpected header: both `content-length` and `transfer-encoding`, for example.
#define FASTLY_ERROR_HTTP_USER 8
// HTTP incomplete message error.
// This can be thrown when a stream ended unexpectedly.
#define FASTLY_ERROR_HTTP_INCOMPLETE 9
// A `None` error.
// This status code is used to indicate when an optional value did not exist, as opposed to
// an empty value.
// Note, this value should no longer be used, as we have explicit optional types now.
#define FASTLY_ERROR_OPTIONAL_NONE 10
// Message head too large.
#define FASTLY_ERROR_HTTP_HEAD_TOO_LARGE 11
// Invalid HTTP status.
#define FASTLY_ERROR_HTTP_INVALID_STATUS 12
// Limit exceeded
//
// This is returned when an attempt to allocate a resource has exceeded the maximum number of
// resources permitted. For example, creating too many response handles.
#define FASTLY_ERROR_LIMIT_EXCEEDED 13

typedef struct {
  bool is_some;
  bool val;
} fastly_world_option_bool_t;

typedef struct {
  bool is_some;
  fastly_tls_version_t val;
} fastly_world_option_tls_version_t;

// Create a backend for later use
typedef struct {
  fastly_world_option_string_t host_override;
  fastly_world_option_u32_t connect_timeout;
  fastly_world_option_u32_t first_byte_timeout;
  fastly_world_option_u32_t between_bytes_timeout;
  fastly_world_option_bool_t use_ssl;
  fastly_world_option_tls_version_t ssl_min_version;
  fastly_world_option_tls_version_t ssl_max_version;
  fastly_world_option_string_t cert_hostname;
  fastly_world_option_string_t ca_cert;
  fastly_world_option_string_t ciphers;
  fastly_world_option_string_t sni_hostname;
} fastly_dynamic_backend_config_t;

typedef uint32_t fastly_dictionary_handle_t;

typedef uint8_t fastly_content_encodings_t;

#define FASTLY_CONTENT_ENCODINGS_GZIP (1 << 0)

typedef uint64_t fastly_cache_object_length_t;

// The status of this lookup (and potential transaction)
typedef uint8_t fastly_cache_lookup_state_t;

// a cached object was found
#define FASTLY_CACHE_LOOKUP_STATE_FOUND (1 << 0)
// the cached object is valid to use (implies found)
#define FASTLY_CACHE_LOOKUP_STATE_USABLE (1 << 1)
// the cached object is stale (but may or may not be valid to use)
#define FASTLY_CACHE_LOOKUP_STATE_STALE (1 << 2)
// this client is requested to insert or revalidate an object
#define FASTLY_CACHE_LOOKUP_STATE_MUST_INSERT_OR_UPDATE (1 << 3)

typedef struct {
  bool is_some;
  fastly_request_handle_t val;
} fastly_world_option_request_handle_t;

// Extensible options for cache lookup operations; currently used for both `lookup` and
// `transaction_lookup`.
typedef struct {
  // * A full request handle, but used only for its headers
  fastly_world_option_request_handle_t request_headers;
} fastly_cache_lookup_options_t;

typedef uint64_t fastly_cache_hit_count_t;

// The outcome of a cache lookup (either bare or as part of a cache transaction)
typedef uint32_t fastly_cache_handle_t;

typedef struct {
  uint64_t start;
  uint64_t end;
} fastly_cache_get_body_options_t;

typedef uint64_t fastly_cache_duration_ns_t;

typedef struct {
  uint8_t *ptr;
  size_t len;
} fastly_world_list_u8_t;

// Configuration for several hostcalls that write to the cache:
// - `insert`
// - `transaction-insert`
// - `transaction-insert-and-stream-back`
// - `transaction-update`
//
// Some options are only allowed for certain of these hostcalls; see `cache-write-options-mask`.
typedef struct {
  // this is a required field; there's no flag for it
  fastly_cache_duration_ns_t max_age_ns;
  // a full request handle, but used only for its headers
  fastly_request_handle_t request_headers;
  // a list of header names separated by spaces
  fastly_world_string_t vary_rule;
  // The initial age of the object in nanoseconds (default: 0).
  //
  // This age is used to determine the freshness lifetime of the object as well as to
  // prioritize which variant to return if a subsequent lookup matches more than one vary rule
  fastly_cache_duration_ns_t initial_age_ns;
  fastly_cache_duration_ns_t stale_while_revalidate_ns;
  // a list of surrogate keys separated by spaces
  fastly_world_string_t surrogate_keys;
  fastly_cache_object_length_t length;
  fastly_world_list_u8_t user_metadata;
  bool sensitive_data;
} fastly_cache_write_options_t;

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
  fastly_world_string_t *ptr;
  size_t len;
} fastly_world_list_string_t;

typedef struct {
  bool is_some;
  fastly_world_list_string_t val;
} fastly_world_option_list_string_t;

typedef struct {
  bool is_some;
  fastly_response_t val;
} fastly_world_option_response_t;

typedef struct {
  fastly_pending_request_handle_t *ptr;
  size_t len;
} fastly_world_list_pending_request_handle_t;

typedef struct {
  uint32_t f0;
  fastly_response_t f1;
} fastly_world_tuple2_u32_response_t;

typedef struct {
  bool is_some;
  fastly_body_handle_t val;
} fastly_world_option_body_handle_t;

typedef struct {
  bool is_some;
  fastly_fd_t val;
} fastly_world_option_fd_t;

typedef struct {
  bool is_some;
  fastly_secret_handle_t val;
} fastly_world_option_secret_handle_t;

typedef struct {
  fastly_async_handle_t *ptr;
  size_t len;
} fastly_world_list_async_handle_t;

typedef struct {
  fastly_body_handle_t f0;
  fastly_cache_handle_t f1;
} fastly_world_tuple2_body_handle_cache_handle_t;

typedef uint32_t compute_at_edge_request_handle_t;

typedef uint32_t compute_at_edge_body_handle_t;

typedef struct {
  compute_at_edge_request_handle_t f0;
  compute_at_edge_body_handle_t f1;
} compute_at_edge_request_t;

// Imported Functions from `fastly`
bool fastly_abi_init(uint64_t abi_version, fastly_error_t *err);
bool fastly_uap_parse(fastly_world_string_t *user_agent, fastly_user_agent_t *ret,
                      fastly_error_t *err);
bool fastly_http_body_new(fastly_body_handle_t *ret, fastly_error_t *err);
bool fastly_http_body_append(fastly_body_handle_t dest, fastly_body_handle_t src,
                             fastly_error_t *err);
bool fastly_http_body_read(fastly_body_handle_t h, uint32_t chunk_size, fastly_world_list_u8_t *ret,
                           fastly_error_t *err);
bool fastly_http_body_write(fastly_body_handle_t h, fastly_world_list_u8_t *buf,
                            fastly_body_write_end_t end, uint32_t *ret, fastly_error_t *err);
bool fastly_http_body_close(fastly_body_handle_t h, fastly_error_t *err);
bool fastly_log_endpoint_get(fastly_world_string_t *name, fastly_log_endpoint_handle_t *ret,
                             fastly_error_t *err);
bool fastly_log_write(fastly_log_endpoint_handle_t h, fastly_world_string_t *msg,
                      fastly_error_t *err);
bool fastly_http_req_cache_override_set(fastly_request_handle_t h,
                                        fastly_http_cache_override_tag_t tag, uint32_t *maybe_ttl,
                                        uint32_t *maybe_stale_while_revalidate,
                                        fastly_world_string_t *maybe_sk, fastly_error_t *err);
bool fastly_http_req_downstream_client_ip_addr(fastly_world_list_u8_t *ret, fastly_error_t *err);
bool fastly_http_req_downstream_client_h2_fingerprint(fastly_world_list_u8_t *ret,
                                                      fastly_error_t *err);
bool fastly_http_req_downstream_tls_cipher_openssl_name(fastly_world_string_t *ret,
                                                        fastly_error_t *err);
bool fastly_http_req_downstream_tls_protocol(fastly_world_string_t *ret, fastly_error_t *err);
bool fastly_http_req_downstream_tls_client_hello(fastly_world_list_u8_t *ret, fastly_error_t *err);
bool fastly_http_req_downstream_tls_client_certificate(fastly_world_list_u8_t *ret,
                                                       fastly_error_t *err);
bool fastly_http_req_downstream_tls_client_cert_verify_result(fastly_error_t *err);
bool fastly_http_req_downstream_tls_ja3_md5(fastly_world_list_u8_t *ret, fastly_error_t *err);
bool fastly_http_req_new(fastly_request_handle_t *ret, fastly_error_t *err);
bool fastly_http_req_header_names_get(fastly_request_handle_t h, fastly_world_list_string_t *ret,
                                      fastly_error_t *err);
bool fastly_http_req_header_value_get(fastly_request_handle_t h, fastly_world_string_t *name,
                                      fastly_world_option_string_t *ret, fastly_error_t *err);
bool fastly_http_req_header_values_get(fastly_request_handle_t h, fastly_world_string_t *name,
                                       fastly_world_option_list_string_t *ret, fastly_error_t *err);
bool fastly_http_req_header_values_set(fastly_request_handle_t h, fastly_world_string_t *name,
                                       fastly_world_list_string_t *values, fastly_error_t *err);
bool fastly_http_req_header_insert(fastly_request_handle_t h, fastly_world_string_t *name,
                                   fastly_world_string_t *value, fastly_error_t *err);
bool fastly_http_req_header_append(fastly_request_handle_t h, fastly_world_string_t *name,
                                   fastly_world_string_t *value, fastly_error_t *err);
bool fastly_http_req_header_remove(fastly_request_handle_t h, fastly_world_string_t *name,
                                   fastly_error_t *err);
bool fastly_http_req_method_get(fastly_request_handle_t h, fastly_world_string_t *ret,
                                fastly_error_t *err);
bool fastly_http_req_method_set(fastly_request_handle_t h, fastly_world_string_t *method,
                                fastly_error_t *err);
bool fastly_http_req_uri_get(fastly_request_handle_t h, fastly_world_string_t *ret,
                             fastly_error_t *err);
bool fastly_http_req_uri_set(fastly_request_handle_t h, fastly_world_string_t *uri,
                             fastly_error_t *err);
bool fastly_http_req_version_get(fastly_request_handle_t h, fastly_http_version_t *ret,
                                 fastly_error_t *err);
bool fastly_http_req_version_set(fastly_request_handle_t h, fastly_http_version_t version,
                                 fastly_error_t *err);
bool fastly_http_req_send(fastly_request_handle_t h, fastly_body_handle_t b,
                          fastly_world_string_t *backend, fastly_response_t *ret,
                          fastly_error_t *err);
bool fastly_http_req_send_async(fastly_request_handle_t h, fastly_body_handle_t b,
                                fastly_world_string_t *backend,
                                fastly_pending_request_handle_t *ret, fastly_error_t *err);
bool fastly_http_req_send_async_streaming(fastly_request_handle_t h, fastly_body_handle_t b,
                                          fastly_world_string_t *backend,
                                          fastly_pending_request_handle_t *ret,
                                          fastly_error_t *err);
bool fastly_http_req_pending_req_poll(fastly_pending_request_handle_t h,
                                      fastly_world_option_response_t *ret, fastly_error_t *err);
bool fastly_http_req_pending_req_wait(fastly_pending_request_handle_t h, fastly_response_t *ret,
                                      fastly_error_t *err);
bool fastly_http_req_pending_req_select(fastly_world_list_pending_request_handle_t *h,
                                        fastly_world_tuple2_u32_response_t *ret,
                                        fastly_error_t *err);
// Returns whether or not the original client request arrived with a
// Fastly-Key belonging to a user with the rights to purge content on this
// service.
bool fastly_http_req_key_is_valid(bool *ret, fastly_error_t *err);
bool fastly_http_req_close(fastly_request_handle_t h, fastly_error_t *err);
bool fastly_http_req_auto_decompress_response_set(fastly_request_handle_t h,
                                                  fastly_content_encodings_t encodings,
                                                  fastly_error_t *err);
bool fastly_http_req_upgrade_websocket(fastly_world_string_t *backend, fastly_error_t *err);
bool fastly_http_req_redirect_to_websocket_proxy(fastly_world_string_t *backend,
                                                 fastly_error_t *err);
bool fastly_http_req_redirect_to_grip_proxy(fastly_world_string_t *backend, fastly_error_t *err);
bool fastly_http_req_framing_headers_mode_set(fastly_request_handle_t h,
                                              fastly_framing_headers_mode_t mode,
                                              fastly_error_t *err);
bool fastly_http_req_register_dynamic_backend(fastly_world_string_t *prefix,
                                              fastly_world_string_t *target,
                                              fastly_dynamic_backend_config_t *config,
                                              fastly_error_t *err);
bool fastly_http_resp_new(fastly_response_handle_t *ret, fastly_error_t *err);
bool fastly_http_resp_header_names_get(fastly_response_handle_t h, fastly_world_list_string_t *ret,
                                       fastly_error_t *err);
bool fastly_http_resp_header_value_get(fastly_response_handle_t h, fastly_world_string_t *name,
                                       fastly_world_option_string_t *ret, fastly_error_t *err);
bool fastly_http_resp_header_values_get(fastly_response_handle_t h, fastly_world_string_t *name,
                                        fastly_world_option_list_string_t *ret,
                                        fastly_error_t *err);
bool fastly_http_resp_header_values_set(fastly_response_handle_t h, fastly_world_string_t *name,
                                        fastly_world_list_string_t *values, fastly_error_t *err);
bool fastly_http_resp_header_insert(fastly_response_handle_t h, fastly_world_string_t *name,
                                    fastly_world_string_t *value, fastly_error_t *err);
bool fastly_http_resp_header_append(fastly_response_handle_t h, fastly_world_string_t *name,
                                    fastly_world_string_t *value, fastly_error_t *err);
bool fastly_http_resp_header_remove(fastly_response_handle_t h, fastly_world_string_t *name,
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
// Adjust how this response's framing headers are determined.
bool fastly_http_resp_framing_headers_mode_set(fastly_response_handle_t h,
                                               fastly_framing_headers_mode_t mode,
                                               fastly_error_t *err);
bool fastly_dictionary_open(fastly_world_string_t *name, fastly_dictionary_handle_t *ret,
                            fastly_error_t *err);
bool fastly_dictionary_get(fastly_dictionary_handle_t h, fastly_world_string_t *key,
                           fastly_world_option_string_t *ret, fastly_error_t *err);
// JSON string for now
bool fastly_geo_lookup(fastly_world_list_u8_t *addr_octets, fastly_world_string_t *ret,
                       fastly_error_t *err);
bool fastly_object_store_open(fastly_world_string_t *name, fastly_object_store_handle_t *ret,
                              fastly_error_t *err);
bool fastly_object_store_lookup(fastly_object_store_handle_t store, fastly_world_string_t *key,
                                fastly_world_option_body_handle_t *ret, fastly_error_t *err);
bool fastly_object_store_lookup_as_fd(fastly_object_store_handle_t store,
                                      fastly_world_string_t *key, fastly_world_option_fd_t *ret,
                                      fastly_error_t *err);
bool fastly_object_store_insert(fastly_object_store_handle_t store, fastly_world_string_t *key,
                                fastly_body_handle_t body_handle, fastly_error_t *err);
bool fastly_secret_store_open(fastly_world_string_t *name, fastly_secret_store_handle_t *ret,
                              fastly_error_t *err);
bool fastly_secret_store_get(fastly_secret_store_handle_t store, fastly_world_string_t *key,
                             fastly_world_option_secret_handle_t *ret, fastly_error_t *err);
bool fastly_secret_store_plaintext(fastly_secret_handle_t secret, fastly_world_option_string_t *ret,
                                   fastly_error_t *err);
// Blocks until one of the given objects is ready for I/O, or the optional timeout expires.
//
// Valid object handles includes bodies and pending requests. See the `async_item_handle`
// definition for more details, including what I/O actions are associated with each handle
// type.
//
// The timeout is specified in milliseconds, or 0 if no timeout is desired.
//
// Returns the _index_ (not handle!) of the first object that is ready, or
// none if the timeout expires before any objects are ready for I/O.
bool fastly_async_io_select(fastly_world_list_async_handle_t *hs, uint32_t timeout_ms,
                            fastly_world_option_u32_t *ret, fastly_error_t *err);
// Returns 1 if the given async item is "ready" for its associated I/O action, 0 otherwise.
//
// If an object is ready, the I/O action is guaranteed to complete without blocking.
//
// Valid object handles includes bodies and pending requests. See the `async_item_handle`
// definition for more details, including what I/O actions are associated with each handle
// type.
bool fastly_async_io_is_ready(fastly_async_handle_t handle, bool *ret, fastly_error_t *err);
bool fastly_purge_surrogate_key(fastly_world_string_t *surrogate_keys,
                                fastly_purge_options_mask_t purge_options,
                                fastly_world_option_string_t *ret, fastly_error_t *err);
// Performs a non-request-collapsing cache lookup.
//
// Returns a result without waiting for any request collapsing that may be ongoing.
bool fastly_cache_lookup(fastly_world_string_t *cache_key, fastly_cache_lookup_options_t *options,
                         fastly_cache_handle_t *ret, fastly_error_t *err);
// Performs a non-request-collapsing cache insertion (or update).
//
// The returned handle is to a streaming body that is used for writing the object into
// the cache.
bool fastly_cache_insert(fastly_world_string_t *cache_key, fastly_cache_write_options_t *options,
                         fastly_body_handle_t *ret, fastly_error_t *err);
// The entrypoint to the request-collapsing cache transaction API.
//
// This operation always participates in request collapsing and may return stale objects. To bypass
// request collapsing, use `lookup` and `insert` instead.
bool fastly_transaction_lookup(fastly_world_string_t *cache_key,
                               fastly_cache_lookup_options_t *options, fastly_cache_handle_t *ret,
                               fastly_error_t *err);
// Insert an object into the cache with the given metadata, and return a readable stream of the
// bytes as they are stored.
//
// This helps avoid the "slow reader" problem on a teed stream, for example when a program wishes
// to store a backend request in the cache while simultaneously streaming to a client in an HTTP
// response.
//
// The returned body handle is to a streaming body that is used for writing the object _into_
// the cache. The returned cache handle provides a separate transaction for reading out the
// newly cached object to send elsewhere.
bool fastly_transaction_insert_and_stream_back(fastly_cache_handle_t handle,
                                               fastly_cache_write_options_t *options,
                                               fastly_world_tuple2_body_handle_cache_handle_t *ret,
                                               fastly_error_t *err);
// Cancel an obligation to provide an object to the cache.
//
// Useful if there is an error before streaming is possible, e.g. if a backend is unreachable.
bool fastly_transaction_cancel(fastly_cache_handle_t handle, fastly_error_t *err);
bool fastly_cache_get_state(fastly_cache_handle_t handle, fastly_cache_lookup_state_t *ret,
                            fastly_error_t *err);
// Gets a range of the found object body, returning the `$none` error if there
// was no found object.
//
// The returned `body_handle` must be closed before calling this function again on the same
// `cache_handle`.
//
// Note: until the CacheD protocol is adjusted to fully support this functionality,
// the body of objects that are past the stale-while-revalidate period will not
// be available, even when other metadata is.
bool fastly_cache_get_body(fastly_cache_handle_t handle, fastly_cache_get_body_options_t *options,
                           fastly_body_handle_t *ret, fastly_error_t *err);

// Exported Functions from `compute-at-edge`
bool compute_at_edge_serve(compute_at_edge_request_t *req);

#ifdef __cplusplus
}
#endif
#endif
