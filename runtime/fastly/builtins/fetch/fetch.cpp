#include "fetch.h"
#include "../backend.h"
#include "../fastly.h"
#include "../fetch-event.h"
#include "./headers.h"
#include "./request-response.h"
#include "encode.h"
#include "extension-api.h"

using fastly::backend::Backend;
using fastly::fastly::Fastly;
using fastly::fetch::Request;

using fastly::fastly::FastlyGetErrorMessage;

namespace fastly::fetch {

api::Engine *ENGINE;

class FetchTask final : public api::AsyncTask {
  Heap<JSObject *> request_;
  Heap<JSObject *> promise_;

public:
  explicit FetchTask(host_api::HttpPendingReq::Handle handle, JS::HandleObject request,
                     JS::HandleObject promise)
      : request_(request), promise_(promise) {
    if (static_cast<int32_t>(handle) < 0)
      abort();
    handle_ = static_cast<int32_t>(handle);
  }

  [[nodiscard]] bool run(api::Engine *engine) override {
    JSContext *cx = engine->cx();

    const RootedObject request(cx, request_);
    const RootedObject promise(cx, promise_);

    return RequestOrResponse::process_pending_request(cx, handle_, request, promise);
  }

  [[nodiscard]] bool cancel(api::Engine *engine) override { return false; }

  void trace(JSTracer *trc) override {
    TraceEdge(trc, &request_, "Fetch request");
    TraceEdge(trc, &promise_, "Fetch promise");
  }
};

// TODO: throw in all Request methods/getters that rely on host calls once a
// request has been sent. The host won't let us act on them anymore anyway.
/**
 * The `fetch` global function
 * https://fetch.spec.whatwg.org/#fetch-method
 */
bool fetch(JSContext *cx, unsigned argc, Value *vp) {
  CallArgs args = CallArgsFromVp(argc, vp);

  REQUEST_HANDLER_ONLY("fetch")

  if (!args.requireAtLeast(cx, "fetch", 1)) {
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }

  RootedObject requestInstance(
      cx, JS_NewObjectWithGivenProto(cx, &Request::class_, Request::proto_obj));
  if (!requestInstance) {
    return false;
  }

  RootedObject request(cx, Request::create(cx, requestInstance, args[0], args.get(1)));
  if (!request) {
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }

  RootedString backend(cx, Request::backend(request));
  if (!backend) {
    if (Fastly::allowDynamicBackends) {
      JS::RootedObject dynamicBackend(cx, Backend::create(cx, request));
      if (!dynamicBackend) {
        return false;
      }
      backend.set(Backend::name(cx, dynamicBackend));
    } else {
      backend = Fastly::defaultBackend;
      if (!backend) {
        auto handle = Request::request_handle(request);

        auto res = handle.get_uri();
        if (auto *err = res.to_err()) {
          HANDLE_ERROR(cx, *err);
        } else {
          JS_ReportErrorLatin1(cx,
                               "No backend specified for request with url %s. "
                               "Must provide a `backend` property on the `init` object "
                               "passed to either `new Request()` or `fetch`",
                               res.unwrap().begin());
        }
        return ReturnPromiseRejectedWithPendingError(cx, args);
      }
    }
  }

  host_api::HostString backend_chars = core::encode(cx, backend);
  if (!backend_chars.ptr) {
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }

  RootedObject response_promise(cx, JS::NewPromiseObject(cx, nullptr));
  if (!response_promise)
    return ReturnPromiseRejectedWithPendingError(cx, args);

  if (!Request::apply_cache_override(cx, request)) {
    return false;
  }

  if (!Request::apply_auto_decompress_gzip(cx, request)) {
    return false;
  }

  bool streaming = false;
  if (!RequestOrResponse::maybe_stream_body(cx, request, &streaming)) {
    return false;
  }

  host_api::HttpPendingReq pending_handle;
  {
    auto request_handle = Request::request_handle(request);
    auto body = RequestOrResponse::body_handle(request);
    auto res = streaming ? request_handle.send_async_streaming(body, backend_chars)
                         : request_handle.send_async(body, backend_chars);

    if (auto *err = res.to_err()) {
      if (host_api::error_is_generic(*err) || host_api::error_is_invalid_argument(*err)) {
        JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr,
                                  JSMSG_REQUEST_BACKEND_DOES_NOT_EXIST, backend_chars.ptr.get());
      } else {
        HANDLE_ERROR(cx, *err);
      }
      return ReturnPromiseRejectedWithPendingError(cx, args);
    }

    pending_handle = res.unwrap();
  }

  // If the request body is streamed, we need to wait for streaming to complete before marking the
  // request as pending.
  if (!streaming) {
    ENGINE->queue_async_task(new FetchTask(pending_handle.handle, request, response_promise));
  }

  JS::SetReservedSlot(request, static_cast<uint32_t>(Request::Slots::PendingRequest),
                      JS::Int32Value(pending_handle.handle));
  JS::SetReservedSlot(request, static_cast<uint32_t>(Request::Slots::ResponsePromise),
                      JS::ObjectValue(*response_promise));

  args.rval().setObject(*response_promise);
  return true;
}

const JSFunctionSpec methods[] = {JS_FN("fetch", fetch, 2, JSPROP_ENUMERATE), JS_FS_END};

bool install(api::Engine *engine) {
  ENGINE = engine;
  if (!JS_DefineFunctions(engine->cx(), engine->global(), methods)) {
    return false;
  }
  if (!Request::init_class(ENGINE->cx(), ENGINE->global())) {
    return false;
  }
  if (!Response::init_class(ENGINE->cx(), ENGINE->global())) {
    return false;
  }
  if (!Headers::init_class(ENGINE->cx(), ENGINE->global())) {
    return false;
  }
  return true;
}

} // namespace fastly::fetch