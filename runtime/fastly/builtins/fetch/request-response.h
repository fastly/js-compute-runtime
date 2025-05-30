#ifndef FASTLY_REQUEST_RESPONSE
#define FASTLY_REQUEST_RESPONSE

#include "../../../StarlingMonkey/builtins/web/fetch/headers.h"
#include "../../host-api/host_api_fastly.h"

namespace fastly::fetch {

class RequestOrResponse final {
public:
  enum class Slots {
    RequestOrResponse,
    Body,
    BodyStream,
    BodyAllPromise,
    HasBody,
    BodyUsed,
    Headers,
    HeadersGen,
    URL,
    ManualFramingHeaders,
    Backend,
    CacheEntry,
    Count,
  };

  static bool is_instance(JSObject *obj);
  static uint32_t handle(JSObject *obj);
  static bool has_body(JSObject *obj);
  static host_api::HttpBody body_handle(JSObject *obj);
  static JSObject *body_stream(JSObject *obj);
  static JSObject *body_source(JSContext *cx, JS::HandleObject obj);
  static bool body_used(JSObject *obj);
  static bool mark_body_used(JSContext *cx, JS::HandleObject obj);
  static bool move_body_handle(JSContext *cx, JS::HandleObject from, JS::HandleObject to);
  static JS::Value url(JSObject *obj);
  static void set_url(JSObject *obj, JS::Value url);
  static void set_manual_framing_headers(JSContext *cx, JSObject *obj, JS::HandleValue url);
  static bool body_unusable(JSContext *cx, JS::HandleObject body);
  static bool extract_body(JSContext *cx, JS::HandleObject self, JS::HandleValue body_val);
  static bool process_pending_request(JSContext *cx, host_api::HttpPendingReq::Handle handle,
                                      JS::HandleObject context, JS::HandleValue promise);

  /**
   * Returns the RequestOrResponse's Headers if it has been reified, nullptr if
   * not.
   */
  static JSObject *maybe_headers(JSObject *obj);

  /**
   * For Requests and Responses in Mode::ContentOnly, creates a new HostOnly headers
   * object and commits the headers map values.
   */
  static bool commit_headers(JSContext *cx, JS::HandleObject self);

  /**
   * Compare the HeadersGen slot with the current headers generation, returning true if the headers
   * have changed and false otherwise, storing the new generation into the slot.
   * If the headers are undefined, the generation is tracked as null, still supporting comparison.
   */
  static bool compare_bump_headers_gen(JSContext *cx, JS::HandleObject self, bool *changed_out);

  static bool append_body(JSContext *cx, JS::HandleObject self, JS::HandleObject source);

  using ParseBodyCB = bool(JSContext *cx, JS::HandleObject self, JS::UniqueChars buf, size_t len);

  enum class BodyReadResult {
    ArrayBuffer,
    Blob,
    FormData,
    JSON,
    Text,
  };

  template <BodyReadResult result_type>
  static bool parse_body(JSContext *cx, JS::HandleObject self, JS::UniqueChars buf, size_t len);

  static bool content_stream_read_then_handler(JSContext *cx, JS::HandleObject self,
                                               JS::HandleValue extra, JS::CallArgs args);
  static bool content_stream_read_catch_handler(JSContext *cx, JS::HandleObject self,
                                                JS::HandleValue extra, JS::CallArgs args);
  static bool consume_content_stream_for_bodyAll(JSContext *cx, JS::HandleObject self,
                                                 JS::HandleValue stream_val, JS::CallArgs args);
  template <bool async>
  static bool consume_body_handle_for_bodyAll(JSContext *cx, JS::HandleObject self,
                                              JS::HandleValue body_parser, JS::CallArgs args);
  template <RequestOrResponse::BodyReadResult result_type, bool async>
  static bool bodyAll(JSContext *cx, JS::CallArgs args, JS::HandleObject self);
  static bool body_source_cancel_algorithm(JSContext *cx, JS::CallArgs args,
                                           JS::HandleObject stream, JS::HandleObject owner,
                                           JS::HandleValue reason);
  static bool body_source_pull_algorithm(JSContext *cx, JS::CallArgs args, JS::HandleObject source,
                                         JS::HandleObject body_owner, JS::HandleObject controller);
  static bool body_reader_then_handler(JSContext *cx, JS::HandleObject body_owner,
                                       JS::HandleValue extra, JS::CallArgs args);

  static bool body_reader_catch_handler(JSContext *cx, JS::HandleObject body_owner,
                                        JS::HandleValue extra, JS::CallArgs args);

  /**
   * Ensures that the given |body_owner|'s body is properly streamed, if it
   * requires streaming.
   *
   * If streaming is required, starts the process of reading from the
   * ReadableStream representing the body and sets the |requires_streaming| bool
   * to `true`.
   */
  static bool maybe_stream_body(JSContext *cx, JS::HandleObject body_owner,
                                bool *requires_streaming);

  static JSObject *create_body_stream(JSContext *cx, JS::HandleObject owner);

  static bool body_get(JSContext *cx, JS::CallArgs args, JS::HandleObject self,
                       bool create_if_undefined);
  static bool backend_get(JSContext *cx, JS::CallArgs args, JS::HandleObject self);
  static JSString *backend(JSObject *obj);

  /**
   * Helper method to get (and possibly unset) the cache entry for a request or response (if any)
   *
   * A Response with a cache entry is a valid CandidateResponse.
   */
  static std::optional<host_api::HttpCacheEntry> cache_entry(JSObject *obj);
  /**
   * Helper method to get and unset the transaction cache entry for a request or response (if any)
   *
   * Unsetting the cache entry on a Response object freezes the CandidateResponse from further
   * modification.
   *
   * A boolean overload for the cache entry disabled case is used to mark if we were cached or not.
   */
  static std::optional<host_api::HttpCacheEntry> take_cache_entry(JSObject *obj,
                                                                  std::optional<bool> mark_cached);
  /**
   * Close the cache entry and clear it from the Response, effectively locking this
   * candidate response object.
   */
  static bool close_if_cache_entry(JSContext *cx, JS::HandleObject self);
};

class Request final : public builtins::BuiltinImpl<Request> {
  static bool method_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool headers_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool url_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool version_get(JSContext *cx, unsigned argc, JS::Value *vp);

  template <RequestOrResponse::BodyReadResult result_type>
  static bool bodyAll(JSContext *cx, unsigned argc, JS::Value *vp);

  static bool backend_get(JSContext *cx, unsigned argc, JS::Value *vp);

  static bool body_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool bodyUsed_get(JSContext *cx, unsigned argc, JS::Value *vp);

  static bool setCacheOverride(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool setCacheKey(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool setManualFramingHeaders(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool clone(JSContext *cx, unsigned argc, JS::Value *vp);

public:
  static constexpr const char *class_name = "Request";

  enum class Slots {
    Request = static_cast<int>(RequestOrResponse::Slots::RequestOrResponse),
    Body = static_cast<int>(RequestOrResponse::Slots::Body),
    BodyStream = static_cast<int>(RequestOrResponse::Slots::BodyStream),
    HasBody = static_cast<int>(RequestOrResponse::Slots::HasBody),
    BodyUsed = static_cast<int>(RequestOrResponse::Slots::BodyUsed),
    Headers = static_cast<int>(RequestOrResponse::Slots::Headers),
    HeadersGen = static_cast<int>(RequestOrResponse::Slots::HeadersGen),
    URL = static_cast<int>(RequestOrResponse::Slots::URL),
    ManualFramingHeaders = static_cast<int>(RequestOrResponse::Slots::ManualFramingHeaders),
    Backend = static_cast<int>(RequestOrResponse::Slots::Backend),
    CacheEntry = static_cast<int>(RequestOrResponse::Slots::CacheEntry),
    Method = static_cast<int>(RequestOrResponse::Slots::Count),
    OverrideCacheKey,
    CacheOverride,
    PendingRequest,
    ResponsePromise,
    IsDownstream,
    AutoDecompressGzip,
    Count,
  };

  /**
   * Returns the RequestOrResponse's Headers, reifying it if necessary.
   */
  static JSObject *headers(JSContext *cx, JS::HandleObject obj);

  static JSObject *response_promise(JSObject *obj);
  static JSString *method(JSContext *cx, JS::HandleObject obj);
  // static JSString *override_cache_key(JSContext *cx)
  static bool set_cache_key(JSContext *cx, JS::HandleObject self, JS::HandleValue cache_key_val);
  static bool set_cache_override(JSContext *cx, JS::HandleObject self,
                                 JS::HandleValue cache_override_val);
  static bool apply_cache_override(JSContext *cx, JS::HandleObject self);
  static bool apply_auto_decompress_gzip(JSContext *cx, JS::HandleObject self);

  static bool isCacheable_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static host_api::HttpReq request_handle(JSObject *obj);
  static host_api::HttpPendingReq pending_handle(JSObject *obj);
  static bool is_downstream(JSObject *obj);
  static const JSFunctionSpec static_methods[];
  static const JSPropertySpec static_properties[];
  static const JSFunctionSpec methods[];
  static const JSPropertySpec properties[];

  static const unsigned ctor_length = 1;

  static bool init_class(JSContext *cx, JS::HandleObject global);
  static bool constructor(JSContext *cx, unsigned argc, JS::Value *vp);

  static JSObject *create(JSContext *cx, JS::HandleObject requestInstance,
                          host_api::HttpReq request_handle, host_api::HttpBody body_handle,
                          bool is_downstream);
  static JSObject *create(JSContext *cx, JS::HandleObject requestInstance, JS::HandleValue input,
                          JS::HandleValue init_val);

  static JSObject *create_instance(JSContext *cx);
};

class Response final : public builtins::FinalizableBuiltinImpl<Response> {
  static bool waitUntil(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool ok_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool status_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool status_set(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool statusText_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool url_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool version_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool type_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool headers_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool redirected_get(JSContext *cx, unsigned argc, JS::Value *vp);

  template <RequestOrResponse::BodyReadResult result_type>
  static bool bodyAll(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool backend_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool body_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool bodyUsed_get(JSContext *cx, unsigned argc, JS::Value *vp);

  static bool ip_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool port_get(JSContext *cx, unsigned argc, JS::Value *vp);

  static bool redirect(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool json(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool setManualFramingHeaders(JSContext *cx, unsigned argc, JS::Value *vp);

public:
  static constexpr const char *class_name = "Response";

  enum class Slots {
    Response = static_cast<int>(RequestOrResponse::Slots::RequestOrResponse),
    Body = static_cast<int>(RequestOrResponse::Slots::Body),
    BodyStream = static_cast<int>(RequestOrResponse::Slots::BodyStream),
    HasBody = static_cast<int>(RequestOrResponse::Slots::HasBody),
    BodyUsed = static_cast<int>(RequestOrResponse::Slots::BodyUsed),
    Headers = static_cast<int>(RequestOrResponse::Slots::Headers),
    HeadersGen = static_cast<int>(RequestOrResponse::Slots::HeadersGen),
    URL = static_cast<int>(RequestOrResponse::Slots::Headers),
    ManualFramingHeaders = static_cast<int>(RequestOrResponse::Slots::ManualFramingHeaders),
    Backend = static_cast<int>(RequestOrResponse::Slots::Backend),
    CacheEntry = static_cast<int>(RequestOrResponse::Slots::CacheEntry),
    IsUpstream = static_cast<int>(RequestOrResponse::Slots::Count),
    Status,
    StatusMessage,
    Redirected,
    GripUpgradeRequest,
    WebsocketUpgradeRequest,
    StorageAction,
    SuggestedCacheWriteOptions,
    OverrideCacheWriteOptions,
    CacheBodyTransform,
    Count,
  };
  static const JSFunctionSpec static_methods[];
  static const JSPropertySpec static_properties[];
  static const JSFunctionSpec methods[];
  static const JSPropertySpec properties[];

  static const unsigned ctor_length = 1;

  static bool init_class(JSContext *cx, JS::HandleObject global);
  static bool constructor(JSContext *cx, unsigned argc, JS::Value *vp);

  /**
   * Create a response for a request, used for the fetch response flow.
   */
  static JSObject *create(JSContext *cx, HandleObject request, host_api::Response res);

  /**
   * Base-level response creation handler, for both upstream and downstream requests.
   */
  static JSObject *create(JSContext *cx, JS::HandleObject response,
                          host_api::HttpResp response_handle, host_api::HttpBody body_handle,
                          bool is_upstream, JSObject *grip_upgrade_request,
                          JSObject *websocket_upgrade_request, JS::HandleString backend);

  static host_api::HttpResp response_handle(JSObject *obj);

  /**
   * Returns the RequestOrResponse's Headers, reifying it if necessary.
   */
  static JSObject *headers(JSContext *cx, JS::HandleObject obj);

  /**
   * Get the storage action for the response.
   */
  static std::optional<host_api::HttpStorageAction> storage_action(JSObject *obj);

  static bool is_upstream(JSObject *obj);
  static std::optional<host_api::HttpReq> grip_upgrade_request(JSObject *obj);
  static std::optional<host_api::HttpReq> websocket_upgrade_request(JSObject *obj);
  static host_api::HostString backend_str(JSContext *cx, JSObject *obj);
  static uint16_t status(JSObject *obj);
  static JSString *status_message(JSObject *obj);
  static void set_status_message_from_code(JSContext *cx, JSObject *obj, uint16_t code);

  static bool add_fastly_cache_headers(JSContext *cx, JS::HandleObject self,
                                       JS::HandleObject request,
                                       std::optional<host_api::HttpCacheEntry> cache_entry,
                                       const char *fun_name);

  static bool has_body_transform(JSObject *self);

  /**
   * Override cache options set by the user & suggested options, or final cache options if
   * finalized.
   */
  static host_api::HttpCacheWriteOptions *override_cache_options(JSObject *response);

  /**
   * Takes the override cache options field.
   */
  static host_api::HttpCacheWriteOptions *take_override_cache_options(JSObject *response);

  /**
   * Suggested cache options as provided by the host for the request/response pair, and
   * computed lazily (fallible).
   */
  static host_api::HttpCacheWriteOptions *suggested_cache_options(JSContext *cx,
                                                                  HandleObject response);

  static bool isCacheable_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool cached_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool stale_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool ttl_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool ttl_set(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool age_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool swr_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool swr_set(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool vary_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool vary_set(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool surrogateKeys_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool surrogateKeys_set(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool pci_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool pci_set(JSContext *cx, unsigned argc, JS::Value *vp);

  static void finalize(JS::GCContext *gcx, JSObject *self);
};

} // namespace fastly::fetch

#endif
