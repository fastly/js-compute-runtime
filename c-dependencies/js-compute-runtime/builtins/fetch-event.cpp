#include "builtins/fetch-event.h"
#include "builtins/client-info.h"
#include "builtins/fastly.h"
#include "builtins/request-response.h"
#include "builtins/shared/url.h"
#include "builtins/worker-location.h"
#include "host_interface/host_api.h"
#include "xqd-world/xqd_world_adapter.h"

namespace builtins {

namespace {

JS::PersistentRooted<JSObject *> INSTANCE;

void inc_pending_promise_count(JSObject *self) {
  MOZ_ASSERT(FetchEvent::is_instance(self));
  auto count =
      JS::GetReservedSlot(self, static_cast<uint32_t>(FetchEvent::Slots::PendingPromiseCount))
          .toInt32();
  count++;
  MOZ_ASSERT(count > 0);
  JS::SetReservedSlot(self, static_cast<uint32_t>(FetchEvent::Slots::PendingPromiseCount),
                      JS::Int32Value(count));
}

void dec_pending_promise_count(JSObject *self) {
  MOZ_ASSERT(FetchEvent::is_instance(self));
  auto count =
      JS::GetReservedSlot(self, static_cast<uint32_t>(FetchEvent::Slots::PendingPromiseCount))
          .toInt32();
  MOZ_ASSERT(count > 0);
  count--;
  JS::SetReservedSlot(self, static_cast<uint32_t>(FetchEvent::Slots::PendingPromiseCount),
                      JS::Int32Value(count));
}

bool add_pending_promise(JSContext *cx, JS::HandleObject self, JS::HandleObject promise) {
  MOZ_ASSERT(FetchEvent::is_instance(self));
  MOZ_ASSERT(JS::IsPromiseObject(promise));

  JS::RootedObject handler(cx);
  handler = &JS::GetReservedSlot(
                 self, static_cast<uint32_t>(FetchEvent::Slots::DecPendingPromiseCountFunc))
                 .toObject();
  if (!JS::AddPromiseReactions(cx, promise, handler, handler))
    return false;

  inc_pending_promise_count(self);
  return true;
}

} // namespace

bool FetchEvent::client_get(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0)

  JS::RootedValue clientInfo(cx,
                             JS::GetReservedSlot(self, static_cast<uint32_t>(Slots::ClientInfo)));

  if (clientInfo.isUndefined()) {
    JS::RootedObject obj(cx, ClientInfo::create(cx));
    if (!obj)
      return false;
    clientInfo.setObject(*obj);
    JS::SetReservedSlot(self, static_cast<uint32_t>(Slots::ClientInfo), clientInfo);
  }

  args.rval().set(clientInfo);
  return true;
}

JSObject *FetchEvent::prepare_downstream_request(JSContext *cx) {
  JS::RootedObject requestInstance(
      cx, JS_NewObjectWithGivenProto(cx, &Request::class_, Request::proto_obj));
  if (!requestInstance)
    return nullptr;
  return Request::create(cx, requestInstance, INVALID_HANDLE, INVALID_HANDLE, true);
}

bool FetchEvent::init_downstream_request(JSContext *cx, JS::HandleObject request) {
  MOZ_ASSERT(Request::request_handle(request) == INVALID_HANDLE);

  fastly_request_t req;
  fastly_error_t err;
  if (!xqd_fastly_http_req_body_downstream_get(&req, &err)) {
    HANDLE_ERROR(cx, err);
    return false;
  }

  fastly_request_handle_t request_handle = req.f0;
  fastly_body_handle_t body_handle = req.f1;

  JS::SetReservedSlot(request, static_cast<uint32_t>(Request::Slots::Request),
                      JS::Int32Value(request_handle));
  JS::SetReservedSlot(request, static_cast<uint32_t>(Request::Slots::Body),
                      JS::Int32Value(body_handle));

  // Set the method.
  xqd_world_string_t method_str;
  if (!xqd_fastly_http_req_method_get(request_handle, &method_str, &err)) {
    HANDLE_ERROR(cx, err);
    return false;
  }

  bool is_get = strncmp(method_str.ptr, "GET", method_str.len) == 0;
  bool is_head = strncmp(method_str.ptr, "HEAD", method_str.len) == 0;

  if (!is_get) {
    JS::RootedString method(cx, JS_NewStringCopyN(cx, method_str.ptr, method_str.len));
    JS_free(cx, method_str.ptr);
    if (!method) {
      return false;
    }

    JS::SetReservedSlot(request, static_cast<uint32_t>(Request::Slots::Method),
                        JS::StringValue(method));
  }

  // Set whether we have a body depending on the method.
  // TODO: verify if that's right. I.e. whether we should treat all requests
  // that are not GET or HEAD as having a body, which might just be 0-length.
  // It's not entirely clear what else we even could do here though.
  if (!is_get && !is_head) {
    JS::SetReservedSlot(request, static_cast<uint32_t>(Request::Slots::HasBody), JS::TrueValue());
  }

  xqd_world_string_t uri_str;
  if (!xqd_fastly_http_req_uri_get(request_handle, &uri_str, &err)) {
    HANDLE_ERROR(cx, err);
    return false;
  }

  JS::RootedString url(cx, JS_NewStringCopyN(cx, uri_str.ptr, uri_str.len));
  if (!url) {
    JS_free(cx, uri_str.ptr);
    return false;
  }
  JS::SetReservedSlot(request, static_cast<uint32_t>(Request::Slots::URL), JS::StringValue(url));

  // Set the URL for `globalThis.location` to the client request's URL.
  JS::RootedObject url_instance(
      cx, JS_NewObjectWithGivenProto(cx, &builtins::URL::class_, builtins::URL::proto_obj));
  if (!url_instance) {
    JS_free(cx, uri_str.ptr);
    return false;
  }

  jsurl::SpecString spec(reinterpret_cast<uint8_t *>(uri_str.ptr), uri_str.len, uri_str.len);
  builtins::WorkerLocation::url = builtins::URL::create(cx, url_instance, spec);
  JS_free(cx, uri_str.ptr);
  if (!builtins::WorkerLocation::url) {
    return false;
  }

  // Set `fastly.baseURL` to the origin of the client request's URL.
  // Note that this only happens if baseURL hasn't already been set to another
  // value explicitly.
  if (!builtins::Fastly::baseURL.get()) {
    JS::RootedObject url_instance(
        cx, JS_NewObjectWithGivenProto(cx, &builtins::URL::class_, builtins::URL::proto_obj));
    if (!url_instance)
      return false;

    builtins::Fastly::baseURL = builtins::URL::create(
        cx, url_instance, builtins::URL::origin(cx, builtins::WorkerLocation::url));
    if (!builtins::Fastly::baseURL)
      return false;
  }

  return true;
}

bool FetchEvent::request_get(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0)

  args.rval().set(JS::GetReservedSlot(self, static_cast<uint32_t>(Slots::Request)));
  return true;
}

namespace {

bool start_response(JSContext *cx, JS::HandleObject response_obj, bool streaming) {
  fastly_response_handle_t response = Response::response_handle(response_obj);
  fastly_body_handle_t body = RequestOrResponse::body_handle(response_obj);

  fastly_error_t err;
  if (!xqd_fastly_http_resp_send_downstream(response, body, streaming, &err)) {
    HANDLE_ERROR(cx, err);
    return false;
  }
  return true;
}

// Steps in this function refer to the spec at
// https://w3c.github.io/ServiceWorker/#fetch-event-respondwith
bool response_promise_then_handler(JSContext *cx, JS::HandleObject event, JS::HandleValue extra,
                                   JS::CallArgs args) {
  // Step 10.1
  // Note: the `then` handler is only invoked after all Promise resolution has
  // happened. (Even if there were multiple Promises to unwrap first.) That
  // means that at this point we're guaranteed to have the final value instead
  // of a Promise wrapping it, so either the value is a Response, or we have to
  // bail.
  if (!Response::is_instance(args.get(0))) {
    JS_ReportErrorUTF8(cx, "FetchEvent#respondWith must be called with a Response "
                           "object or a Promise resolving to a Response object as "
                           "the first argument");
    JS::RootedObject rejection(cx, PromiseRejectedWithPendingError(cx));
    if (!rejection)
      return false;
    args.rval().setObject(*rejection);
    return FetchEvent::respondWithError(cx, event);
  }

  // Step 10.2 (very roughly: the way we handle responses and their bodies is
  // very different.)
  JS::RootedObject response_obj(cx, &args[0].toObject());

  // Ensure that all headers are stored client-side, so we retain access to them
  // after sending the response off.
  if (Response::is_upstream(response_obj)) {
    JS::RootedObject headers(cx);
    headers =
        RequestOrResponse::headers<builtins::Headers::Mode::ProxyToResponse>(cx, response_obj);
    if (!builtins::Headers::delazify(cx, headers))
      return false;
  }

  bool streaming = false;
  if (!RequestOrResponse::maybe_stream_body(cx, response_obj, &streaming)) {
    return false;
  }

  FetchEvent::set_state(event, streaming ? FetchEvent::State::responseStreaming
                                         : FetchEvent::State::responseDone);
  return start_response(cx, response_obj, streaming);
}

// Steps in this function refer to the spec at
// https://w3c.github.io/ServiceWorker/#fetch-event-respondwith
bool response_promise_catch_handler(JSContext *cx, JS::HandleObject event,
                                    JS::HandleValue promise_val, JS::CallArgs args) {
  JS::RootedObject promise(cx, &promise_val.toObject());

  fprintf(stderr, "Error while running request handler: ");
  dump_promise_rejection(cx, args.get(0), promise, stderr);

  // TODO: verify that this is the right behavior.
  // Steps 9.1-2
  return FetchEvent::respondWithError(cx, event);
}

} // namespace

// Steps in this function refer to the spec at
// https://w3c.github.io/ServiceWorker/#fetch-event-respondwith
bool FetchEvent::respondWith(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(1)

  // Coercion of argument `r` to a Promise<Response>
  JS::RootedObject response_promise(cx, JS::CallOriginalPromiseResolve(cx, args.get(0)));
  if (!response_promise)
    return false;

  // Step 2
  if (!is_dispatching(self)) {
    JS_ReportErrorUTF8(cx, "FetchEvent#respondWith must be called synchronously from "
                           "within a FetchEvent handler");
    return false;
  }

  // Step 3
  if (state(self) != State::unhandled) {
    JS_ReportErrorUTF8(cx, "FetchEvent#respondWith can't be called twice on the same event");
    return false;
  }

  // Step 4
  add_pending_promise(cx, self, response_promise);

  // Steps 5-7 (very roughly)
  set_state(self, State::waitToRespond);

  // Step 9 (continued in `response_promise_catch_handler` above)
  JS::RootedObject catch_handler(cx);
  JS::RootedValue extra(cx, JS::ObjectValue(*response_promise));
  catch_handler = create_internal_method<response_promise_catch_handler>(cx, self, extra);
  if (!catch_handler)
    return false;

  // Step 10 (continued in `response_promise_then_handler` above)
  JS::RootedObject then_handler(cx);
  then_handler = create_internal_method<response_promise_then_handler>(cx, self);
  if (!then_handler)
    return false;

  if (!JS::AddPromiseReactions(cx, response_promise, then_handler, catch_handler))
    return false;

  args.rval().setUndefined();
  return true;
}

bool FetchEvent::respondWithError(JSContext *cx, JS::HandleObject self) {
  MOZ_RELEASE_ASSERT(state(self) == State::unhandled || state(self) == State::waitToRespond);
  set_state(self, State::responsedWithError);
  fastly_response_handle_t response = INVALID_HANDLE;
  fastly_error_t err;

  if (!xqd_fastly_http_resp_new(&response, &err)) {
    HANDLE_ERROR(cx, err);
    return false;
  }

  auto make_res = HttpBody::make();
  if (auto *err = make_res.to_err()) {
    HANDLE_ERROR(cx, *err);
    return false;
  }

  auto body = make_res.unwrap();
  if (!xqd_fastly_http_resp_status_set(response, 500, &err) ||
      !xqd_fastly_http_resp_send_downstream(response, body.handle, false, &err)) {
    HANDLE_ERROR(cx, err);
    return false;
  }
  return true;
}

namespace {

// Step 5 of https://w3c.github.io/ServiceWorker/#wait-until-method
bool dec_pending_promise_count(JSContext *cx, JS::HandleObject event, JS::HandleValue extra,
                               JS::CallArgs args) {
  // Step 5.1
  dec_pending_promise_count(event);

  // Note: step 5.2 not relevant to our implementation.
  return true;
}

} // namespace

// Steps in this function refer to the spec at
// https://w3c.github.io/ServiceWorker/#wait-until-method
bool FetchEvent::waitUntil(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(1)

  JS::RootedObject promise(cx, JS::CallOriginalPromiseResolve(cx, args.get(0)));
  if (!promise)
    return false;

  // Step 2
  if (!is_active(self)) {
    JS_ReportErrorUTF8(cx, "FetchEvent#waitUntil called on inactive event");
    return false;
  }

  // Steps 3-4
  add_pending_promise(cx, self, promise);

  // Note: step 5 implemented in dec_pending_promise_count

  args.rval().setUndefined();
  return true;
}

const JSFunctionSpec FetchEvent::methods[] = {
    JS_FN("respondWith", respondWith, 1, JSPROP_ENUMERATE),
    JS_FN("waitUntil", waitUntil, 1, JSPROP_ENUMERATE),
    JS_FS_END,
};

const JSPropertySpec FetchEvent::properties[] = {
    JS_PSG("client", client_get, JSPROP_ENUMERATE),
    JS_PSG("request", request_get, JSPROP_ENUMERATE),
    JS_PS_END,
};

JSObject *FetchEvent::create(JSContext *cx) {
  JS::RootedObject self(cx, JS_NewObjectWithGivenProto(cx, &class_, proto_obj));
  if (!self)
    return nullptr;

  JS::RootedObject request(cx, prepare_downstream_request(cx));
  if (!request)
    return nullptr;

  JS::RootedObject dec_count_handler(cx,
                                     create_internal_method<dec_pending_promise_count>(cx, self));
  if (!dec_count_handler)
    return nullptr;

  JS::SetReservedSlot(self, static_cast<uint32_t>(Slots::Request), JS::ObjectValue(*request));
  JS::SetReservedSlot(self, static_cast<uint32_t>(Slots::Dispatch), JS::FalseValue());
  JS::SetReservedSlot(self, static_cast<uint32_t>(Slots::State),
                      JS::Int32Value((int)State::unhandled));
  JS::SetReservedSlot(self, static_cast<uint32_t>(Slots::PendingPromiseCount), JS::Int32Value(0));
  JS::SetReservedSlot(self, static_cast<uint32_t>(Slots::DecPendingPromiseCountFunc),
                      JS::ObjectValue(*dec_count_handler));

  INSTANCE.init(cx, self);
  return self;
}

JS::HandleObject FetchEvent::instance() { return INSTANCE; }

bool FetchEvent::init_request(JSContext *cx, JS::HandleObject self) {
  JS::RootedObject request(
      cx, &JS::GetReservedSlot(self, static_cast<uint32_t>(Slots::Request)).toObject());
  return init_downstream_request(cx, request);
}

bool FetchEvent::is_active(JSObject *self) {
  MOZ_ASSERT(is_instance(self));
  // Note: we also treat the FetchEvent as active if it's in `responseStreaming`
  // state because that requires us to extend the service's lifetime as well. In
  // the spec this is achieved using individual promise counts for the body read
  // operations.
  return JS::GetReservedSlot(self, static_cast<uint32_t>(Slots::Dispatch)).toBoolean() ||
         state(self) == State::responseStreaming ||
         JS::GetReservedSlot(self, static_cast<uint32_t>(Slots::PendingPromiseCount)).toInt32() > 0;
}

bool FetchEvent::is_dispatching(JSObject *self) {
  MOZ_ASSERT(is_instance(self));
  return JS::GetReservedSlot(self, static_cast<uint32_t>(Slots::Dispatch)).toBoolean();
}

void FetchEvent::start_dispatching(JSObject *self) {
  MOZ_ASSERT(!is_dispatching(self));
  JS::SetReservedSlot(self, static_cast<uint32_t>(Slots::Dispatch), JS::TrueValue());
}

void FetchEvent::stop_dispatching(JSObject *self) {
  MOZ_ASSERT(is_dispatching(self));
  JS::SetReservedSlot(self, static_cast<uint32_t>(Slots::Dispatch), JS::FalseValue());
}

FetchEvent::State FetchEvent::state(JSObject *self) {
  MOZ_ASSERT(is_instance(self));
  return static_cast<State>(
      JS::GetReservedSlot(self, static_cast<uint32_t>(Slots::State)).toInt32());
}

void FetchEvent::set_state(JSObject *self, State new_state) {
  MOZ_ASSERT(is_instance(self));
  MOZ_ASSERT((uint8_t)new_state > (uint8_t)state(self));
  JS::SetReservedSlot(self, static_cast<uint32_t>(Slots::State),
                      JS::Int32Value(static_cast<int32_t>(new_state)));
}

bool FetchEvent::response_started(JSObject *self) {
  auto current_state = state(self);
  return current_state != State::unhandled && current_state != State::waitToRespond;
}

} // namespace builtins
