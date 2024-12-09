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
    URL,
    ManualFramingHeaders,
    Backend,
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
                                      JS::HandleObject context, JS::HandleObject promise);

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

  static bool append_body(JSContext *cx, JS::HandleObject self, JS::HandleObject source);

  using ParseBodyCB = bool(JSContext *cx, JS::HandleObject self, JS::UniqueChars buf, size_t len);

  enum class BodyReadResult {
    ArrayBuffer,
    Blob,
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
  static bool consume_body_handle_for_bodyAll(JSContext *cx, JS::HandleObject self,
                                              JS::HandleValue body_parser, JS::CallArgs args);
  template <RequestOrResponse::BodyReadResult result_type>
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
    URL = static_cast<int>(RequestOrResponse::Slots::URL),
    ManualFramingHeaders = static_cast<int>(RequestOrResponse::Slots::ManualFramingHeaders),
    Backend = static_cast<int>(RequestOrResponse::Slots::Backend),
    Method = static_cast<int>(RequestOrResponse::Slots::Count),
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
  static bool set_cache_key(JSContext *cx, JS::HandleObject self, JS::HandleValue cache_key_val);
  static bool set_cache_override(JSContext *cx, JS::HandleObject self,
                                 JS::HandleValue cache_override_val);
  static bool apply_cache_override(JSContext *cx, JS::HandleObject self);
  static bool apply_auto_decompress_gzip(JSContext *cx, JS::HandleObject self);

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

class Response final : public builtins::BuiltinImpl<Response> {
  static bool waitUntil(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool ok_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool status_get(JSContext *cx, unsigned argc, JS::Value *vp);
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
    URL = static_cast<int>(RequestOrResponse::Slots::Headers),
    ManualFramingHeaders = static_cast<int>(RequestOrResponse::Slots::ManualFramingHeaders),
    Backend = static_cast<int>(RequestOrResponse::Slots::Backend),
    IsUpstream = static_cast<int>(RequestOrResponse::Slots::Count),
    Status,
    StatusMessage,
    Redirected,
    IsGripUpgrade,
    Count,
  };
  static const JSFunctionSpec static_methods[];
  static const JSPropertySpec static_properties[];
  static const JSFunctionSpec methods[];
  static const JSPropertySpec properties[];

  static const unsigned ctor_length = 1;

  static bool init_class(JSContext *cx, JS::HandleObject global);
  static bool constructor(JSContext *cx, unsigned argc, JS::Value *vp);

  static JSObject *create(JSContext *cx, JS::HandleObject response,
                          host_api::HttpResp response_handle, host_api::HttpBody body_handle,
                          bool is_upstream, bool is_grip_upgrade, JS::HandleString backend);

  /**
   * Returns the RequestOrResponse's Headers, reifying it if necessary.
   */
  static JSObject *headers(JSContext *cx, JS::HandleObject obj);

  static host_api::HttpResp response_handle(JSObject *obj);
  static bool is_upstream(JSObject *obj);
  static bool is_grip_upgrade(JSObject *obj);
  static host_api::HostString backend_str(JSContext *cx, JSObject *obj);
  static uint16_t status(JSObject *obj);
  static JSString *status_message(JSObject *obj);
  static void set_status_message_from_code(JSContext *cx, JSObject *obj, uint16_t code);
};

} // namespace fastly::fetch

#endif
