#ifndef BUILTIN_REQUEST_RESPONSE
#define BUILTIN_REQUEST_RESPONSE

#include "builtin.h"
#include "builtins/headers.h"
#include "c-at-e-world/c_at_e_world.h"

namespace builtins {

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
    Count,
  };

  static bool is_instance(JSObject *obj);
  static uint32_t handle(JSObject *obj);
  static bool has_body(JSObject *obj);
  static fastly_body_handle_t body_handle(JSObject *obj);
  static JSObject *body_stream(JSObject *obj);
  static JSObject *body_source(JSContext *cx, JS::HandleObject obj);
  static bool body_used(JSObject *obj);
  static bool mark_body_used(JSContext *cx, JS::HandleObject obj);
  static bool move_body_handle(JSContext *cx, JS::HandleObject from, JS::HandleObject to);
  static JS::Value url(JSObject *obj);
  static void set_url(JSObject *obj, JS::Value url);
  static bool body_unusable(JSContext *cx, JS::HandleObject body);
  static bool extract_body(JSContext *cx, JS::HandleObject self, JS::HandleValue body_val);

  /**
   * Returns the RequestOrResponse's Headers if it has been reified, nullptr if
   * not.
   */
  static JSObject *maybe_headers(JSObject *obj);

  /**
   * Returns the RequestOrResponse's Headers, reifying it if necessary.
   */
  template <Headers::Mode mode> static JSObject *headers(JSContext *cx, JS::HandleObject obj);

  static bool append_body(JSContext *cx, JS::HandleObject self, JS::HandleObject source);

  using ParseBodyCB = bool(JSContext *cx, JS::HandleObject self, JS::UniqueChars buf, size_t len);

  enum class BodyReadResult {
    ArrayBuffer,
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
};

class Request final : public BuiltinImpl<Request> {
  static bool method_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool headers_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool url_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool version_get(JSContext *cx, unsigned argc, JS::Value *vp);

  template <RequestOrResponse::BodyReadResult result_type>
  static bool bodyAll(JSContext *cx, unsigned argc, JS::Value *vp);

  static bool body_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool bodyUsed_get(JSContext *cx, unsigned argc, JS::Value *vp);

  static bool setCacheOverride(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool setCacheKey(JSContext *cx, unsigned argc, JS::Value *vp);
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
    Backend = static_cast<int>(RequestOrResponse::Slots::Count),
    Method,
    CacheOverride,
    PendingRequest,
    ResponsePromise,
    IsDownstream,
    Count,
  };

  static JSObject *response_promise(JSObject *obj);
  static JSString *method(JSContext *cx, JS::HandleObject obj);
  static bool set_cache_key(JSContext *cx, JS::HandleObject self, JS::HandleValue cache_key_val);
  static bool set_cache_override(JSContext *cx, JS::HandleObject self,
                                 JS::HandleValue cache_override_val);
  static bool apply_cache_override(JSContext *cx, JS::HandleObject self);

  static fastly_request_handle_t request_handle(JSObject *obj);
  static fastly_pending_request_handle_t pending_handle(JSObject *obj);
  static bool is_downstream(JSObject *obj);
  static JSString *backend(JSObject *obj);

  static const JSFunctionSpec methods[];
  static const JSPropertySpec properties[];

  static const unsigned ctor_length = 1;

  static bool init_class(JSContext *cx, JS::HandleObject global);
  static bool constructor(JSContext *cx, unsigned argc, JS::Value *vp);

  static JSObject *create(JSContext *cx, JS::HandleObject requestInstance,
                          fastly_request_handle_t request_handle, fastly_body_handle_t body_handle,
                          bool is_downstream);
  static JSObject *create(JSContext *cx, JS::HandleObject requestInstance, JS::HandleValue input,
                          JS::HandleValue init_val);

  static JSObject *create_instance(JSContext *cx);
};

class Response final : public BuiltinImpl<Response> {
  static bool waitUntil(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool ok_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool status_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool statusText_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool url_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool version_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool type_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool headers_get(JSContext *cx, unsigned argc, JS::Value *vp);

  template <RequestOrResponse::BodyReadResult result_type>
  static bool bodyAll(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool body_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool bodyUsed_get(JSContext *cx, unsigned argc, JS::Value *vp);

public:
  static constexpr const char *class_name = "Response";

  enum class Slots {
    Response = static_cast<int>(RequestOrResponse::Slots::RequestOrResponse),
    Body = static_cast<int>(RequestOrResponse::Slots::Body),
    BodyStream = static_cast<int>(RequestOrResponse::Slots::BodyStream),
    HasBody = static_cast<int>(RequestOrResponse::Slots::HasBody),
    BodyUsed = static_cast<int>(RequestOrResponse::Slots::BodyUsed),
    Headers = static_cast<int>(RequestOrResponse::Slots::Headers),
    IsUpstream = static_cast<int>(RequestOrResponse::Slots::Count),
    Status,
    StatusMessage,
    Count,
  };

  static const JSFunctionSpec methods[];
  static const JSPropertySpec properties[];

  static const unsigned ctor_length = 1;

  static bool init_class(JSContext *cx, JS::HandleObject global);
  static bool constructor(JSContext *cx, unsigned argc, JS::Value *vp);

  static JSObject *create(JSContext *cx, JS::HandleObject response,
                          fastly_response_handle_t response_handle,
                          fastly_body_handle_t body_handle, bool is_upstream);

  static fastly_response_handle_t response_handle(JSObject *obj);
  static bool is_upstream(JSObject *obj);
  static uint16_t status(JSObject *obj);
  static JSString *status_message(JSObject *obj);
  static void set_status_message_from_code(JSContext *cx, JSObject *obj, uint16_t code);
};

} // namespace builtins

#endif
