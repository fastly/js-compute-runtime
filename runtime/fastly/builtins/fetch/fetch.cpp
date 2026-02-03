#include "fetch.h"
#include "../../../StarlingMonkey/builtins/web/fetch/headers.h"
#include "../../../StarlingMonkey/builtins/web/streams/native-stream-sink.h"
#include "../../../StarlingMonkey/builtins/web/streams/native-stream-source.h"
#include "../../../StarlingMonkey/builtins/web/streams/transform-stream.h"
#include "../backend.h"
#include "../body.h"
#include "../cache-override.h"
#include "../fastly.h"
#include "../fetch-event.h"
#include "../image-optimizer.h"
#include "./request-response.h"
#include "builtin.h"
#include "encode.h"
#include "extension-api.h"
#include "js/Stream.h"

#include <iostream>

using builtins::web::streams::NativeStreamSink;
using builtins::web::streams::NativeStreamSource;
using builtins::web::streams::TransformStream;
using fastly::FastlyGetErrorMessage;
using fastly::backend::Backend;
using fastly::body::FastlyBody;
using fastly::cache_override::CacheOverride;
using fastly::fastly::Fastly;
using fastly::fetch::ENGINE;
using fastly::fetch::Request;
using fastly::fetch::RequestOrResponse;
using fastly::fetch::Response;
using fastly::fetch_event::FetchEvent;

namespace {

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
    const RootedValue promise(cx, JS::ObjectValue(*promise_));

    return RequestOrResponse::process_pending_request(cx, handle_, request, promise);
  }

  [[nodiscard]] bool cancel(api::Engine *engine) override { return false; }

  void trace(JSTracer *trc) override {
    TraceEdge(trc, &request_, "Fetch request");
    TraceEdge(trc, &promise_, "Fetch promise");
  }
};

template <InternalMethod then_handler, InternalMethod catch_handler>
JSObject *internal_method_then(JSContext *cx, JS::HandleObject promise, JS::HandleObject receiver,
                               const JS::HandleValue extra = UndefinedHandleValue) {
  JS::RootedObject then_handler_obj(cx, create_internal_method<then_handler>(cx, receiver, extra));
  if (!then_handler_obj) {
    return nullptr;
  }
  JS::RootedObject catch_handler_obj(cx,
                                     create_internal_method<catch_handler>(cx, receiver, extra));
  if (!catch_handler_obj) {
    return nullptr;
  }
  JS::RootedObject return_promise(
      cx, JS::CallOriginalPromiseThen(cx, promise, then_handler_obj, catch_handler_obj));
  if (!return_promise) {
    return nullptr;
  }
  return return_promise;
}

JSString *get_backend(JSContext *cx, JS::HandleObject request) {
  RootedString backend(cx, RequestOrResponse::backend(request));
  if (!backend) {
    if (Fastly::allowDynamicBackends) {
      JS::RootedObject dynamicBackend(cx, Backend::create(cx, request));
      if (!dynamicBackend) {
        return nullptr;
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
        return nullptr;
      }
    }
  }
  return backend;
}

bool must_use_guest_caching(JSContext *cx, HandleObject request) {
  JS::RootedObject cache_override(
      cx, JS::GetReservedSlot(request, static_cast<uint32_t>(Request::Slots::CacheOverride))
              .toObjectOrNull());
  if (cache_override) {
    return CacheOverride::beforeSend(cache_override) || CacheOverride::afterSend(cache_override);
  }
  return false;
}

bool http_caching_unsupported = false;
enum CachingMode { Guest, Host, ImageOptimizer };
bool get_caching_mode(JSContext *cx, HandleObject request, CachingMode *caching_mode) {
  *caching_mode = CachingMode::Guest;

  // Check for pass cache override
  MOZ_ASSERT(Request::is_instance(request));
  JS::RootedObject cache_override(
      cx, JS::GetReservedSlot(request, static_cast<uint32_t>(Request::Slots::CacheOverride))
              .toObjectOrNull());
  if (cache_override) {
    if (CacheOverride::mode(cache_override) == CacheOverride::CacheOverrideMode::Pass) {
      // Pass requests have to go through the host for now
      *caching_mode = CachingMode::Host;
      return true;
    }
  }

  // Check for PURGE method
  RootedString method_str(cx, Request::method(cx, request));
  bool is_purge = false;
  if (method_str && !JS_StringEqualsLiteral(cx, method_str, "PURGE", &is_purge)) {
    return false;
  }
  if (is_purge) {
    // We don't yet implement guest-side URL purges
    *caching_mode = CachingMode::Host;
    return true;
  }

  // Requests meant for Image Optimizer should not be cached at this point,
  // as the caching behavior is determined by the origin image, which is
  // fetched after the request reaches the Image Optimizer WASM service.
  // The WASM service uses cache_on_behalf to insert the result into
  // the service's cache.
  auto image_optimizer_opts =
      JS::GetReservedSlot(request, static_cast<uint32_t>(Request::Slots::ImageOptimizerOptions));
  if (!image_optimizer_opts.isNullOrUndefined()) {
    *caching_mode = CachingMode::ImageOptimizer;
    return true;
  }

  // If we previously found guest caching unsupported then remember that
  if (http_caching_unsupported || !fastly::fastly::ENABLE_EXPERIMENTAL_HTTP_CACHE) {
    if (must_use_guest_caching(cx, request)) {
      if (!fastly::fastly::ENABLE_EXPERIMENTAL_HTTP_CACHE) {
        JS_ReportErrorASCII(cx, "HTTP caching API is not enabled for JavaScript; enable it with "
                                "the --enable-http-cache flag "
                                "to the js-compute build command, or contact support for help");
      } else {
        JS_ReportErrorASCII(
            cx,
            "HTTP caching API is not enabled for this service; please contact support for help");
      }
      return false;
    }
    *caching_mode = CachingMode::Host;
    return true;
  }

  // Check if we must use host caching by checking if guest caching is unsupported
  auto request_handle = Request::request_handle(request);
  auto res = request_handle.is_cacheable();
  if (auto *err = res.to_err()) {
    if (host_api::error_is_unsupported(*err)) {
      http_caching_unsupported = true;
      // Guest-side caching is unsupported, so we must use host caching.
      // If we have hooks we must fail since they require guest caching.
      if (must_use_guest_caching(cx, request)) {
        JS_ReportErrorASCII(cx, "HTTP caching API is not enabled; please contact support for help");
        return false;
      }
      *caching_mode = CachingMode::Host;
      return true;
    }
    HANDLE_ERROR(cx, *err);
    return false;
  }

  return true;
}

// Sends the request body, resolving the response promise with the response
template <CachingMode caching_mode>
bool fetch_send_body(JSContext *cx, HandleObject request, JS::MutableHandleValue ret) {
  RootedString backend(cx, get_backend(cx, request));
  if (!backend) {
    JSObject *promise = PromiseRejectedWithPendingError(cx);
    if (!promise) {
      return false;
    }
    ret.setObject(*promise);
    return true;
  }
  host_api::HostString backend_chars = core::encode(cx, backend);
  if (!backend_chars.ptr) {
    JSObject *promise = PromiseRejectedWithPendingError(cx);
    if (!promise) {
      return false;
    }
    ret.setObject(*promise);
    return true;
  }

  // commit the headers into the headers handle
  if (!RequestOrResponse::commit_headers(cx, request)) {
    return false;
  }

  // cache override only applies to requests with caching
  if (caching_mode == CachingMode::Host) {
    if (!Request::apply_cache_override(cx, request)) {
      return false;
    }
  } else {
    // track requests without caching via CacheEntry false convention
    // this is used to track that we later must set Fastly headers in this case
    JS::SetReservedSlot(request, static_cast<uint32_t>(RequestOrResponse::Slots::CacheEntry),
                        JS::BooleanValue(false));
  }

  if (!Request::apply_auto_decompress_gzip(cx, request)) {
    return false;
  }

  RootedObject response_promise(cx, JS::NewPromiseObject(cx, nullptr));
  if (!response_promise) {
    return false;
  }

  // The image optimizer does not support streaming, so never stream in this case
  bool streaming = false;
  if (caching_mode != CachingMode::ImageOptimizer &&
      !RequestOrResponse::maybe_stream_body(cx, request, &streaming)) {
    return false;
  }

  host_api::HttpPendingReq pending_handle;
  {
    auto request_handle = Request::request_handle(request);
    auto body = RequestOrResponse::body_handle(request);
    host_api::Result<host_api::HttpPendingReq> res;
    switch (caching_mode) {
    case CachingMode::Host: {
      if (streaming) {
        res = request_handle.send_async_streaming(body, backend_chars);
      } else {
        res = request_handle.send_async(body, backend_chars);
      }
      break;
    }
    case CachingMode::Guest:
      res = request_handle.send_async_without_caching(body, backend_chars, streaming);
      break;
    case CachingMode::ImageOptimizer: {
      auto config = reinterpret_cast<fastly::image_optimizer::ImageOptimizerOptions *>(
          JS::GetReservedSlot(request, static_cast<uint32_t>(Request::Slots::ImageOptimizerOptions))
              .toPrivate());
      auto config_str = config->to_string();
      auto res = request_handle.send_image_optimizer(body, backend_chars, config_str);
      if (auto *err = res.to_err()) {
        HANDLE_IMAGE_OPTIMIZER_ERROR(cx, *err);
        ret.setObject(*PromiseRejectedWithPendingError(cx));
        return true;
      }

      JS::RootedObject response(cx, Response::create(cx, request, res.unwrap()));
      JS::RootedValue response_val(cx, JS::ObjectValue(*response));
      if (!JS::ResolvePromise(cx, response_promise, response_val)) {
        return false;
      }
      ret.setObject(*response_promise);
      return true;
    }
    }

    if (auto *err = res.to_err()) {
      if (host_api::error_is_generic(*err) || host_api::error_is_invalid_argument(*err)) {
        JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr,
                                  fastly::JSMSG_REQUEST_BACKEND_DOES_NOT_EXIST,
                                  backend_chars.ptr.get());
      } else {
        HANDLE_ERROR(cx, *err);
      }
      ret.setObject(*PromiseRejectedWithPendingError(cx));
      return true;
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
  ret.setObject(*response_promise);
  return true;
}

bool fetch_process_cache_hooks_origin_request(JSContext *cx, JS::HandleObject request,
                                              JS::HandleValue ret_promise, JS::CallArgs args) {
  JS::RootedObject ret_promise_obj(cx, &ret_promise.toObject());

  JS::SetReservedSlot(request, static_cast<uint32_t>(RequestOrResponse::Slots::BodyUsed),
                      JS::BooleanValue(false));

  RootedString backend(cx, get_backend(cx, request));
  if (!backend) {
    RejectPromiseWithPendingError(cx, ret_promise_obj);
    return true;
  }
  host_api::HostString backend_chars = core::encode(cx, backend);
  if (!backend_chars.ptr) {
    RejectPromiseWithPendingError(cx, ret_promise_obj);
    return true;
  }

  if (!RequestOrResponse::commit_headers(cx, request)) {
    return false;
  }

  if (!Request::apply_auto_decompress_gzip(cx, request)) {
    return false;
  }

  bool streaming = false;
  if (!RequestOrResponse::maybe_stream_body(cx, request, &streaming)) {
    RequestOrResponse::close_if_cache_entry(cx, request);
    return false;
  }

  host_api::HttpPendingReq pending_handle;
  {
    auto request_handle = Request::request_handle(request);
    auto body = RequestOrResponse::body_handle(request);
    auto res = request_handle.send_async_without_caching(body, backend_chars, streaming);

    if (auto *err = res.to_err()) {
      if (!RequestOrResponse::close_if_cache_entry(cx, request)) {
        return false;
      }
      if (host_api::error_is_generic(*err) || host_api::error_is_invalid_argument(*err)) {
        JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr,
                                  fastly::JSMSG_REQUEST_BACKEND_DOES_NOT_EXIST,
                                  backend_chars.ptr.get());
      } else {
        HANDLE_ERROR(cx, *err);
      }
      RejectPromiseWithPendingError(cx, ret_promise_obj);
      return true;
    }

    pending_handle = res.unwrap();
  }

  // If the request body is streamed, we need to wait for streaming to complete before marking
  // the request as pending.
  if (!streaming) {
    ENGINE->queue_async_task(new FetchTask(pending_handle.handle, request, ret_promise_obj));
  }

  JS::SetReservedSlot(request, static_cast<uint32_t>(Request::Slots::PendingRequest),
                      JS::Int32Value(pending_handle.handle));
  return true;
}

bool fetch_process_cache_hooks_before_send_reject(JSContext *cx, JS::HandleObject request,
                                                  JS::HandleValue ret_promise, JS::CallArgs args) {
  if (!RequestOrResponse::close_if_cache_entry(cx, request)) {
    return false;
  }
  JS::RootedObject ret_promise_obj(cx, &ret_promise.toObject());
  JS::RejectPromise(cx, ret_promise_obj, args.get(0));
  return true;
}

// Sends the request body, applying the beforeSend and afterSend HTTP caching hook lifecycle
bool fetch_send_body_with_cache_hooks(JSContext *cx, HandleObject request,
                                      host_api::HttpCacheEntry &cache_entry,
                                      JS::MutableHandleValue ret_promise) {
  auto suggested_backend_request_res = cache_entry.get_suggested_backend_request();
  if (auto *err = suggested_backend_request_res.to_err()) {
    HANDLE_ERROR(cx, *err);
    ret_promise.setObject(*PromiseRejectedWithPendingError(cx));
    return true;
  }
  auto backend_request_handle = suggested_backend_request_res.unwrap();

  // We carefully patch just the request handle on the original request to refer to the new request
  // handle. This retains the normal fetch logic that the body will stream then wait for the request
  // and use the request response, even though the other request details will be different under the
  // hood, but it is the simplest approach to ensure the original body runs through the correct
  // request logic. Setting the cache handle on the request indicates this is the situation for a
  // cache origin request.
  JS::SetReservedSlot(request, static_cast<uint32_t>(Request::Slots::Request),
                      JS::Int32Value(backend_request_handle.handle));
  JS::SetReservedSlot(request, static_cast<uint32_t>(RequestOrResponse::Slots::CacheEntry),
                      JS::Int32Value(cache_entry.handle));

  // Next, if we have a beforeSend hook, we invoke this hook prior to sending to the backend, with
  // a newly created request object to match this request. This ensures it gets its headers freshly
  // linked to the new host request correctly without the previous header cache state from the
  // original request. We lock the body on this request as it is still owned by the initiating
  // request object. If there is no beforeSend hook, we don't need to create this request.
  RootedObject cache_override(
      cx, JS::GetReservedSlot(request, static_cast<uint32_t>(Request::Slots::CacheOverride))
              .toObjectOrNull());
  RootedObject before_send(cx);
  if (cache_override) {
    before_send.set(CacheOverride::beforeSend(cache_override));
  }

  JS::RootedObject before_send_promise(cx);
  if (before_send) {
    JS::RootedValue ret_val(cx);
    JS::RootedValueArray<1> args(cx);
    args[0].set(JS::ObjectValue(*request));

    // lock the body temporarily (throwing if already used here, earlier than usual, instead)
    if (RequestOrResponse::body_used(request)) {
      JS_ReportErrorASCII(cx, "Body has already been consumed");
      ret_promise.setObject(*PromiseRejectedWithPendingError(cx));
    }
    JS::SetReservedSlot(request, static_cast<uint32_t>(RequestOrResponse::Slots::BodyUsed),
                        JS::BooleanValue(true));

    // now call before_send with the candidate_request, allowing any async work
    if (!JS::Call(cx, JS::NullHandleValue, before_send, args, &ret_val)) {
      ret_promise.setObject(*PromiseRejectedWithPendingError(cx));
      return true;
    }
    before_send_promise.set(JS::CallOriginalPromiseResolve(cx, ret_val));
    if (!before_send_promise) {
      return false;
    }
  } else {
    before_send_promise.set(JS::NewPromiseObject(cx, nullptr));
    if (!before_send_promise) {
      return false;
    }
    if (!JS::ResolvePromise(cx, before_send_promise, JS::UndefinedHandleValue)) {
      return false;
    }
  }
  // when we resume, we pick up in fetch_send_body_with_cache_hooks_origin_request
  ret_promise.setObject(*JS::NewPromiseObject(cx, nullptr));
  JS::RootedObject then_handler_obj(
      cx,
      create_internal_method<fetch_process_cache_hooks_origin_request>(cx, request, ret_promise));
  if (!then_handler_obj) {
    return false;
  }
  JS::RootedObject catch_handler_obj(
      cx, create_internal_method<fetch_process_cache_hooks_before_send_reject>(cx, request,
                                                                               ret_promise));
  if (!catch_handler_obj) {
    return false;
  }
  return JS::AddPromiseReactions(cx, before_send_promise, then_handler_obj, catch_handler_obj);
}

bool apply_body_transform_transform_then(JSContext *cx, JS::HandleObject fastly_body,
                                         JS::HandleValue response, JS::CallArgs args) {
  JS::RootedValue transformed_array_buffer(cx, args.get(0));
  JS::RootedObject transformed_array_buffer_obj(cx);
  bool valid = false;
  if (transformed_array_buffer.isObject()) {
    transformed_array_buffer_obj.set(&transformed_array_buffer.toObject());
    if (JS_IsArrayBufferViewObject(transformed_array_buffer_obj) ||
        JS_IsTypedArrayObject(transformed_array_buffer_obj)) {
      valid = true;
    }
  }
  if (!valid) {
    JS::RootedObject response_obj(cx, &response.toObject());
    JS::RootedValue result_promise_val(
        cx, JS::GetReservedSlot(response_obj,
                                static_cast<uint32_t>(RequestOrResponse::Slots::BodyAllPromise)));
    JS_ReportErrorASCII(
        cx, "transformBodyFn did not return a valid array buffer or promise for an array buffer");
    return false;
  }

  JS::RootedValueArray<1> append_args(cx);
  append_args[0].setObject(*transformed_array_buffer_obj);
  JS::RootedValue rval(cx);
  if (!JS::Call(cx, fastly_body, "append", append_args, &rval)) {
    return false;
  }
  if (!JS::Call(cx, fastly_body, "close", JS::HandleValueArray::empty(), &rval)) {
    return false;
  }
  return true;
}

// These are constructed as thens that can return promises to extend the outer promise
bool apply_body_transform_response_array_buffer_then(JSContext *cx, JS::HandleObject fastly_body,
                                                     JS::HandleValue response, JS::CallArgs args) {
  JS::RootedObject response_obj(cx, &response.toObject());
  JS::RootedObject response_transform(
      cx,
      &JS::GetReservedSlot(response_obj, static_cast<uint32_t>(Response::Slots::CacheBodyTransform))
           .toObject());
  MOZ_ASSERT(response_transform);

  JS::RootedObject array_buffer(cx, &args.get(0).toObject());
  JS::RootedObject transform_fn_obj(
      cx,
      &JS::GetReservedSlot(response_obj, static_cast<uint32_t>(Response::Slots::CacheBodyTransform))
           .toObject());

  JS::RootedValueArray<1> transform_args(cx);
  transform_args[0].setObject(*array_buffer);
  JS::RootedValue transform_ret(cx);
  if (!JS::Call(cx, JS::NullHandleValue, transform_fn_obj, transform_args, &transform_ret)) {
    return false;
  }
  JS::RootedObject transform_ret_promise(cx, JS::CallOriginalPromiseResolve(cx, transform_ret));
  if (!transform_ret_promise) {
    return false;
  }

  JS::RootedObject transform_then(
      cx, create_internal_method<apply_body_transform_transform_then>(cx, fastly_body, response));
  if (!transform_then) {
    return false;
  }

  JS::RootedObject ret_promise(
      cx, JS::CallOriginalPromiseThen(cx, transform_ret_promise, transform_then, nullptr));
  args.rval().setObject(*ret_promise);
  return true;
}

bool apply_body_transform_catch_handler(JSContext *cx, JS::HandleObject fastly_body,
                                        JS::HandleValue response, JS::CallArgs args) {
  JS::RootedValue rval(cx);
  if (!JS::Call(cx, fastly_body, "abandon", JS::HandleValueArray::empty(), &rval)) {
    return false;
  }
  // "rethrow" the error
  JS_SetPendingException(cx, args.get(0), JS::ExceptionStackBehavior::DoNotCapture);
  return false;
}

bool apply_body_transform(JSContext *cx, JS::HandleValue response, host_api::HttpBody into_body,
                          JS::MutableHandleObject ret_promise) {
  JS::RootedObject response_obj(cx, &response.toObject());
  // Get the entire body from the response (asynchronously)

  JS::Value array_buffer_ret;
  JS::CallArgs args = JS::CallArgsFromVp(0, &array_buffer_ret);
  if (!RequestOrResponse::bodyAll<RequestOrResponse::BodyReadResult::ArrayBuffer, false>(
          cx, args, response_obj)) {
    return false;
  }
  JS::RootedValue array_buffer(cx, array_buffer_ret);
  JS::RootedObject array_buffer_promise(cx, JS::CallOriginalPromiseResolve(cx, array_buffer));
  if (!array_buffer_promise) {
    return false;
  }

  JS::RootedObject fastly_body(
      cx, FastlyBody::create(cx, reinterpret_cast<uint32_t>(into_body.handle)));
  if (!fastly_body) {
    return false;
  }

  JS::RootedObject response_array_buffer_then_obj(
      cx, create_internal_method<apply_body_transform_response_array_buffer_then>(cx, fastly_body,
                                                                                  response));
  if (!response_array_buffer_then_obj) {
    return false;
  }
  JS::RootedObject catch_handler_obj(
      cx, create_internal_method<apply_body_transform_catch_handler>(cx, fastly_body, response));
  if (!catch_handler_obj) {
    return false;
  }

  JS::RootedObject body_transform_promise(
      cx, JS::CallOriginalPromiseThen(cx, array_buffer_promise, response_array_buffer_then_obj,
                                      nullptr));
  if (!body_transform_promise) {
    return false;
  }

  JS::RootedObject body_transform_promise_with_exception_handling(
      cx, JS::CallOriginalPromiseThen(cx, body_transform_promise, nullptr, catch_handler_obj));
  if (!body_transform_promise_with_exception_handling) {
    return false;
  }

  ret_promise.set(body_transform_promise_with_exception_handling);

  return true;

  // Ideally we would support transform streams here in future...
  // // Create the stream source from the response body
  // JS::RootedObject body_stream_in(cx, RequestOrResponse::create_body_stream(cx, response));
  // if (!body_stream_in) {
  //   return false;
  // }

  // // Create the transform stream, and extract its readable and writable end objects
  // JS::RootedValue transform_stream(
  //     cx,
  //     JS::GetReservedSlot(response,
  //     static_cast<uint32_t>(Response::Slots::CacheBodyTransform)));
  // JS::RootedObject transform_stream_obj(cx, &transform_stream.toObject());
  // JS::RootedValue transform_readable(cx);
  // if (!JS_GetProperty(cx, transform_stream_obj, "readable", &transform_readable)) {
  //   return false;
  // }

  // JS::RootedValue transform_writable(cx);
  // if (!JS_GetProperty(cx, transform_stream_obj, "writable", &transform_writable)) {
  //   return false;
  // }

  // // Pipe the incoming body stream into the writable end of the transform stream
  // JS::RootedValueArray<1> args(cx);
  // args[0].set(transform_writable);
  // JS::RootedValue pipe_ret(cx);
  // if (!JS::Call(cx, body_stream_in, "pipeTo", args, &pipe_ret)) {
  //   return false;
  // }

  // // Pipe the readable end of the transform stream into the into_body, with the completion
  // handler
  // // on error or success

  // JS::RootedObject fastly_body(
  //     cx, FastlyBody::create(cx, reinterpret_cast<uint32_t>(into_body.handle)));
  // if (!fastly_body) {
  //   return false;
  // }
  // JS::RootedObject into_response(
  //     cx, JS_NewObjectWithGivenProto(cx, &Response::class_, Response::proto_obj));
  // if (!into_response) {
  //   return false;
  // }
  // if (!Response::create(
  //         cx, into_response, Response::response_handle(response), into_body, true, nullptr,
  //         JS::GetReservedSlot(response, static_cast<uint32_t>(Slots::Backend)).toString())) {
  //   return false;
  // }
  // JS::RootedObject body_stream(cx, RequestOrResponse::body_stream(self));
  // if (!body_stream) {
  //   body_stream = RequestOrResponse::create_body_stream(cx, self);
  //   if (!body_stream)
  //     return false;
  // }
  // // JS::RootedObject sink(cx, NativeStreamSink::create(cx, fastly_body,
  // JS::UndefinedHandleValue,
  // //                                                    body_sink_write_algorithm,
  // //                                                    body_sink_close_algorithm,
  // //                                                    body_sink_abort_algorithm));
  // // if (!sink) {
  // //   return false;
  // // }
  // JS::RootedObject transform_readable_obj(cx, &transform_readable.toObject());
  // JS::RootedValueArray<1> pipeToArgs(cx);
  // // pipeToArgs[0].setObject(*sink);
  // pipeToArgs[0].setObject(*insert_response);
  // JS::RootedValue pipe_promise(cx);
  // if (!JS::Call(cx, transform_readable_obj, "pipeTo", pipeToArgs, &pipe_promise)) {
  //   return false;
  // }
  // JS::RootedObject pipe_promise_obj(cx, &pipe_promise.toObject());

  // if (then_or_catch_handler != nullptr) {
  //   JS::RootedObject then_or_catch_handler_obj(
  //       cx, create_internal_method<then_or_catch_handler>(cx, response));
  //   if (!then_or_catch_handler_obj) {
  //     return false;
  //   }
  //   if (!JS::AddPromiseReactions(cx, pipe_promise_obj, then_or_catch_handler_obj,
  //                                then_or_catch_handler_obj)) {
  //     return false;
  //   }
  // }
  // return true;
}

bool background_cleanup_handler(JSContext *cx, JS::HandleObject request,
                                JS::HandleValue promise_val, JS::CallArgs args) {
  // we follow the Rust implementation calling "close" instead of "transaction_abandon" here
  // this could be reconsidered in future if alternative semantics are required
  RequestOrResponse::close_if_cache_entry(cx, request);
  ENGINE->decr_event_loop_interest();
  return true;
}

bool background_revalidation_then_handler(JSContext *cx, JS::HandleObject request,
                                          JS::HandleValue extra, JS::CallArgs args) {
  auto response = args.get(0);
  RootedObject response_obj(cx, &response.toObject());
  auto cache_entry = RequestOrResponse::take_cache_entry(response_obj, std::nullopt).value();
  auto storage_action = Response::storage_action(response_obj).value();
  auto cache_write_options = Response::override_cache_options(response_obj);
  MOZ_ASSERT(cache_write_options);
  // Mark interest as complete for this phase. We will create new event interest for the body
  // streaming promise shortly if needed to simplify return and error paths in this function.
  ENGINE->decr_event_loop_interest();
  switch (storage_action) {
  case host_api::HttpStorageAction::Insert: {
    auto body_res = cache_entry.transaction_insert(Response::response_handle(response_obj),
                                                   cache_write_options);
    if (auto *err = body_res.to_err()) {
      HANDLE_ERROR(cx, *err);
      return false;
    }

    auto body = body_res.unwrap();

    if (!Response::has_body_transform(response_obj)) {
      auto append_res = body.append(RequestOrResponse::body_handle(response_obj));

      if (auto *err = append_res.to_err()) {
        HANDLE_ERROR(cx, *err);
        JSObject *promise = PromiseRejectedWithPendingError(cx);
        if (!promise) {
          return false;
        }
        args.rval().setObject(*promise);
        return true;
      }

      auto close_res = body.close();
      if (auto *err = close_res.to_err()) {
        HANDLE_ERROR(cx, *err);
        JSObject *promise = PromiseRejectedWithPendingError(cx);
        if (!promise) {
          return false;
        }
        args.rval().setObject(*promise);
        return true;
      }

      return true;
    }

    JS::RootedObject ret_promise(cx);
    if (!apply_body_transform(cx, response, body, &ret_promise)) {
      return false;
    }

    JS::RootedObject then_or_catch_handler_obj(
        cx, create_internal_method<background_cleanup_handler>(cx, response_obj));
    if (!then_or_catch_handler_obj) {
      return false;
    }
    if (!JS::AddPromiseReactions(cx, ret_promise, then_or_catch_handler_obj,
                                 then_or_catch_handler_obj)) {
      return false;
    }

    break;
  }
  case host_api::HttpStorageAction::Update: {
    auto res = cache_entry.transaction_update(Response::response_handle(response_obj),
                                              cache_write_options);
    if (auto *err = res.to_err()) {
      HANDLE_ERROR(cx, *err);
      return false;
    }
    break;
  }
  case host_api::HttpStorageAction::DoNotStore: {
    auto res = cache_entry.transaction_abandon();
    if (auto *err = res.to_err()) {
      HANDLE_ERROR(cx, *err);
      return false;
    }
    break;
  }
  case host_api::HttpStorageAction::RecordUncacheable: {
    auto res = cache_entry.transaction_record_not_cacheable(cache_write_options->max_age_ns.value(),
                                                            cache_write_options->vary_rule);
    if (auto *err = res.to_err()) {
      HANDLE_ERROR(cx, *err);
      return false;
    }
    break;
  }
  default:
    MOZ_ASSERT_UNREACHABLE();
  }
  return true;
}

std::optional<JSObject *> get_found_response(JSContext *cx, host_api::HttpCacheEntry &cache_entry,
                                             JS::HandleObject request,
                                             JS::HandleValue maybe_candidate_response,
                                             bool transform_for_client) {
  auto found_res = cache_entry.get_found_response(transform_for_client);
  if (auto *err = found_res.to_err()) {
    HANDLE_ERROR(cx, *err);
    return nullptr;
  }
  if (!found_res.unwrap().has_value()) {
    return std::nullopt;
  }
  auto found = found_res.unwrap().value();
  RootedObject response(cx, Response::create(cx, request, found));
  if (!response) {
    return nullptr;
  }
  // copy cache options from candidate response to response
  host_api::HttpCacheWriteOptions *override_cache_options;
  if (maybe_candidate_response.isObject()) {
    override_cache_options =
        Response::take_override_cache_options(&maybe_candidate_response.toObject());
  } else {
    // Perhaps we can consider making these hostcalls lazy, requires a Response state enum to know
    // we are in a state we can do this and then keeping the cache handle around, where it is not
    // yet clear if holding handles for longer periods on responses is okay.
    override_cache_options = new host_api::HttpCacheWriteOptions();
    auto age_res = cache_entry.get_age_ns();
    if (auto *err = age_res.to_err()) {
      HANDLE_ERROR(cx, *err);
      return nullptr;
    }
    override_cache_options->initial_age_ns = age_res.unwrap();
    auto max_age_ns = cache_entry.get_max_age_ns();
    if (auto *err = max_age_ns.to_err()) {
      HANDLE_ERROR(cx, *err);
      return nullptr;
    }
    override_cache_options->max_age_ns = max_age_ns.unwrap();
    auto swr_res = cache_entry.get_stale_while_revalidate_ns();
    if (auto *err = swr_res.to_err()) {
      HANDLE_ERROR(cx, *err);
      return nullptr;
    }
    override_cache_options->stale_while_revalidate_ns = swr_res.unwrap();
    auto length_res = cache_entry.get_length();
    if (auto *err = length_res.to_err()) {
      HANDLE_ERROR(cx, *err);
      return nullptr;
    }
    override_cache_options->length = length_res.unwrap();
    auto sensitive_res = cache_entry.get_sensitive_data();
    if (auto *err = sensitive_res.to_err()) {
      HANDLE_ERROR(cx, *err);
      return nullptr;
    }
    override_cache_options->sensitive_data = sensitive_res.unwrap();
    auto vary_res = cache_entry.get_vary_rule();
    if (auto *err = vary_res.to_err()) {
      HANDLE_ERROR(cx, *err);
      return nullptr;
    }
    override_cache_options->vary_rule = std::move(vary_res.unwrap());
    auto surrogate_keys_res = cache_entry.get_surrogate_keys();
    if (auto *err = surrogate_keys_res.to_err()) {
      HANDLE_ERROR(cx, *err);
      return nullptr;
    }
    override_cache_options->surrogate_keys = std::move(surrogate_keys_res.unwrap());
  }
  JS::SetReservedSlot(response, static_cast<uint32_t>(Response::Slots::OverrideCacheWriteOptions),
                      JS::PrivateValue(reinterpret_cast<uint32_t>(override_cache_options)));

  return response;
}

bool stream_back_transform_fulfill(JSContext *cx, JS::HandleObject found_response,
                                   JS::HandleValue extra, JS::CallArgs args) {
  args.rval().setObject(*found_response);
  return true;
}

bool stream_back_then_handler(JSContext *cx, JS::HandleObject request, JS::HandleValue extra,
                              JS::CallArgs args) {
  JS::RootedValue response(cx, args.get(0));
  RootedObject response_obj(cx, &response.toObject());
  auto cache_entry = RequestOrResponse::take_cache_entry(response_obj, false).value();
  auto storage_action = Response::storage_action(response_obj).value();
  // Override cache write options is set to the final cache write options at the end of the
  // response process.
  auto cache_write_options = Response::override_cache_options(response_obj);
  MOZ_ASSERT(cache_write_options);
  switch (storage_action) {
  case host_api::HttpStorageAction::Insert: {
    auto insert_res = cache_entry.transaction_insert_and_stream_back(
        Response::response_handle(response_obj), cache_write_options);
    if (auto *err = insert_res.to_err()) {
      HANDLE_ERROR(cx, *err);
      return false;
    }

    auto [body, cache_entry] = insert_res.unwrap();

    if (!Response::has_body_transform(response_obj)) {
      auto append_res = body.append(RequestOrResponse::body_handle(response_obj));

      if (auto *err = append_res.to_err()) {
        HANDLE_ERROR(cx, *err);
        JSObject *promise = PromiseRejectedWithPendingError(cx);
        if (!promise) {
          return false;
        }
        args.rval().setObject(*promise);
        return true;
      }

      auto close_res = body.close();
      if (auto *err = close_res.to_err()) {
        HANDLE_ERROR(cx, *err);
        JSObject *promise = PromiseRejectedWithPendingError(cx);
        if (!promise) {
          return false;
        }
        args.rval().setObject(*promise);
        return true;
      }

      auto found_response = get_found_response(cx, cache_entry, request, response, false);
      MOZ_ASSERT(found_response.has_value());
      if (!found_response.value()) {
        JSObject *promise = PromiseRejectedWithPendingError(cx);
        if (!promise) {
          return false;
        }
        args.rval().setObject(*promise);
        return true;
      }

      // update response to be the new response, effectively disposing the candidate response
      response_obj.set(found_response.value());

      // Return cached response regardless of revalidation status
      RootedObject response_promise(cx, JS::NewPromiseObject(cx, nullptr));
      JS::RootedValue response_val(cx, JS::ObjectValue(*response_obj));
      args.rval().setObject(*response_promise);
      if (!JS::ResolvePromise(cx, response_promise, response_val)) {
        return false;
      }
      break;
    }

    // Body transfom
    // in order to stream from the response object we must unlock it
    JS::SetReservedSlot(response_obj, static_cast<size_t>(Response::Slots::HasBody),
                        JS::TrueValue());

    JS::RootedObject ret_promise(cx);
    if (!apply_body_transform(cx, response, body, &ret_promise)) {
      return false;
    }

    auto found_response = get_found_response(cx, cache_entry, request, response, false);
    MOZ_ASSERT(found_response.has_value());
    if (!found_response.value()) {
      JSObject *promise = PromiseRejectedWithPendingError(cx);
      if (!promise) {
        return false;
      }
      args.rval().setObject(*promise);
      return true;
    }

    JS::RootedObject found_response_obj(cx, found_response.value());
    JS::RootedObject then_handler_obj(
        cx, create_internal_method<stream_back_transform_fulfill>(cx, found_response_obj));
    if (!then_handler_obj) {
      return false;
    }

    JS::RootedObject final_promise(
        cx, JS::CallOriginalPromiseThen(cx, ret_promise, then_handler_obj, nullptr));
    if (!final_promise) {
      return false;
    }
    args.rval().setObject(*final_promise);
    break;
  }
  case host_api::HttpStorageAction::Update: {
    auto update_res = cache_entry.transaction_update_and_return_fresh(
        Response::response_handle(response_obj), cache_write_options);
    if (auto *err = update_res.to_err()) {
      HANDLE_ERROR(cx, *err);
      return false;
    }
    auto cache_entry = update_res.unwrap();

    auto found_response = get_found_response(cx, cache_entry, request, response, true);
    MOZ_ASSERT(found_response.has_value());
    if (!found_response.value()) {
      JSObject *promise = PromiseRejectedWithPendingError(cx);
      if (!promise) {
        return false;
      }
      args.rval().setObject(*promise);
      return true;
    }

    // response becomes a new response
    response_obj.set(found_response.value());

    // Return cached response regardless of revalidation status
    RootedObject response_promise(cx, JS::NewPromiseObject(cx, nullptr));
    JS::RootedValue response_val(cx, JS::ObjectValue(*response_obj));
    args.rval().setObject(*response_promise);
    if (!JS::ResolvePromise(cx, response_promise, response_val)) {
      return false;
    }
    break;
  }
  case host_api::HttpStorageAction::DoNotStore: {
    // promote the CandidateResponse -> body is now readable
    JS::SetReservedSlot(response_obj, static_cast<size_t>(Response::Slots::HasBody),
                        JS::TrueValue());
    auto res = cache_entry.transaction_abandon();
    if (auto *err = res.to_err()) {
      HANDLE_ERROR(cx, *err);
      return false;
    }
    args.rval().setObject(*response_obj);
    break;
  }
  case host_api::HttpStorageAction::RecordUncacheable: {
    // promote the CandidateResponse -> body is now readable
    JS::SetReservedSlot(response_obj, static_cast<size_t>(Response::Slots::HasBody),
                        JS::TrueValue());
    auto res = cache_entry.transaction_record_not_cacheable(cache_write_options->max_age_ns.value(),
                                                            cache_write_options->vary_rule);
    if (auto *err = res.to_err()) {
      HANDLE_ERROR(cx, *err);
      return false;
    }
    args.rval().setObject(*response_obj);
    break;
  }
  default:
    MOZ_ASSERT_UNREACHABLE();
  }
  // note the storage action on the final response, since the response went through the cache,
  // distinct from responses that did not go through this cache lifecycle.
  // TODO: work out why / if this is needed?
  // JS::SetReservedSlot(response_obj, static_cast<uint32_t>(Response::Slots::StorageAction),
  //                     JS::Int32Value(static_cast<uint32_t>(storage_action)));
  // in all cases, we must add the fastly cache headers
  if (!Response::add_fastly_cache_headers(cx, response_obj, request, cache_entry,
                                          "cached response")) {
    return false;
  }
  return true;
}

bool stream_back_catch_handler(JSContext *cx, JS::HandleObject request, JS::HandleValue promise_val,
                               JS::CallArgs args) {
  // we follow the Rust implementation calling "close" instead of "transaction_abandon" here
  // this could be reconsidered in future if alternative semantics are required
  if (!RequestOrResponse::close_if_cache_entry(cx, request)) {
    return false;
  }
  // "rethrow" the streaming error
  JS_SetPendingException(cx, args.get(0), JS::ExceptionStackBehavior::DoNotCapture);
  return false;
}

} // namespace

namespace fastly::fetch {

api::Engine *ENGINE;

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

  // Determine if we should use guest-side caching
  CachingMode caching_mode;
  if (!get_caching_mode(cx, request, &caching_mode)) {
    return false;
  }
  if (caching_mode == CachingMode::Host) {
    DEBUG_LOG("HTTP Cache: Using traditional fetch without cache API")
    return fetch_send_body<CachingMode::Host>(cx, request, args.rval());
  } else if (caching_mode == CachingMode::ImageOptimizer) {
    return fetch_send_body<CachingMode::ImageOptimizer>(cx, request, args.rval());
  }

  // Check if request is actually cacheable
  bool is_cacheable = false;
  {
    auto request_handle = Request::request_handle(request);
    auto res = request_handle.is_cacheable();
    if (auto *err = res.to_err()) {
      HANDLE_ERROR(cx, *err);
      JSObject *promise = PromiseRejectedWithPendingError(cx);
      if (!promise) {
        return false;
      }
      args.rval().setObject(*promise);
      return true;
    }
    is_cacheable = res.unwrap();
  }

  // If not cacheable, fallback to non-caching path
  if (!is_cacheable) {
    DEBUG_LOG("HTTP Cache: Request not cacheable, using non-caching fetch")
    return fetch_send_body<CachingMode::Guest>(cx, request, args.rval());
  }

  // Lookup in cache
  auto request_handle = Request::request_handle(request);

  // Convert override cache key to span if present
  host_api::HostString override_key_str;
  std::span<uint8_t> override_key_span = {};
  JS::RootedValue override_cache_key(
      cx, JS::GetReservedSlot(request, static_cast<uint32_t>(Request::Slots::OverrideCacheKey)));
  if (override_cache_key.isString()) {
    override_key_str = core::encode(cx, override_cache_key);
    override_key_span = std::span<uint8_t>(reinterpret_cast<uint8_t *>(override_key_str.ptr.get()),
                                           override_key_str.size());
  }

  auto transaction_res =
      host_api::HttpCacheEntry::transaction_lookup(request_handle, override_key_span);
  if (auto *err = transaction_res.to_err()) {
    DEBUG_LOG("HTTP Cache: Transaction lookup error")
    if (host_api::error_is_limit_exceeded(*err)) {
      JS_ReportErrorASCII(cx, "HTTP caching limit exceeded");
    } else {
      HANDLE_ERROR(cx, *err);
    }
    JSObject *promise = PromiseRejectedWithPendingError(cx);
    if (!promise) {
      return false;
    }
    args.rval().setObject(*promise);
    return true;
  }
  host_api::HttpCacheEntry cache_entry = transaction_res.unwrap();

  // This forces synchronous lookups, but we should be able to abstract a new CacheLookupTask, which
  // could be fully async based on its handle as an async select task handle, and then we could
  // support multiple cache lookups in parallel together.
  auto state_res = cache_entry.get_state();
  if (auto *err = state_res.to_err()) {
    HANDLE_ERROR(cx, *err);
    JSObject *promise = PromiseRejectedWithPendingError(cx);
    if (!promise) {
      return false;
    }
    args.rval().setObject(*promise);
    return true;
  }
  auto cache_state = state_res.unwrap();
  std::string state_str = std::to_string(cache_state.state);

  // Check for usable cached response
  JS::RootedValue no_candidate(cx);
  auto maybe_response = get_found_response(cx, cache_entry, request, no_candidate, true);
  if (maybe_response.has_value() && !maybe_response.value()) {
    JSObject *promise = PromiseRejectedWithPendingError(cx);
    if (!promise) {
      return false;
    }
    args.rval().setObject(*promise);
    return true;
  }

  if (maybe_response.has_value()) {
    JS::RootedObject cached_response(cx, maybe_response.value());

    if (cache_state.must_insert_or_update()) {
      // Need to start background revalidation
      // Queue async task to handle background cache revalidation, ensuring it blocks process
      // completion
      RootedValue background_revalidation_promise(cx);
      if (!fetch_send_body_with_cache_hooks(cx, request, cache_entry,
                                            &background_revalidation_promise)) {
        RequestOrResponse::close_if_cache_entry(cx, request);
        return false;
      }
      JS::RootedObject background_revalidation_promise_obj(
          cx, &background_revalidation_promise.toObject());
      JS::RootedObject ret_promise(
          cx,
          internal_method_then<background_revalidation_then_handler, background_cleanup_handler>(
              cx, background_revalidation_promise_obj, request));
      if (!ret_promise) {
        RequestOrResponse::close_if_cache_entry(cx, request);
        return false;
      }
      args.rval().setObject(*ret_promise);
      JS::SetReservedSlot(request, static_cast<uint32_t>(Request::Slots::ResponsePromise),
                          JS::ObjectValue(*ret_promise));
      // keep the event loop alive until background revalidation completes or errors
      ENGINE->incr_event_loop_interest();
    } else {
      if (!RequestOrResponse::close_if_cache_entry(cx, request)) {
        return false;
      }
    }

    // mark the response cache entry as cached for the cached getter
    RequestOrResponse::take_cache_entry(cached_response, true);

    // Return cached response regardless of revalidation status
    if (!Response::add_fastly_cache_headers(cx, cached_response, request, cache_entry,
                                            "cached response")) {
      return false;
    }

    RootedObject response_promise(cx, JS::NewPromiseObject(cx, nullptr));
    JS::RootedValue response_val(cx, JS::ObjectValue(*cached_response));
    args.rval().setObject(*response_promise);
    return JS::ResolvePromise(cx, response_promise, response_val);
  }

  // No valid cached response, need to make backend request
  if (!cache_state.must_insert_or_update()) {
    // transaction entry is done
    if (!RequestOrResponse::close_if_cache_entry(cx, request)) {
      return false;
    }
    // request collapsing has been disabled: pass the original request to the origin without
    // updating the cache and without caching
    return fetch_send_body<CachingMode::Guest>(cx, request, args.rval());
  } else {
    JS::RootedValue stream_back_promise(cx, JS::ObjectValue(*JS::NewPromiseObject(cx, nullptr)));
    if (!fetch_send_body_with_cache_hooks(cx, request, cache_entry, &stream_back_promise)) {
      RequestOrResponse::close_if_cache_entry(cx, request);
      return false;
    }
    JS::RootedObject stream_back_promise_obj(cx, &stream_back_promise.toObject());
    JS::RootedObject ret_promise(
        cx, internal_method_then<stream_back_then_handler, stream_back_catch_handler>(
                cx, stream_back_promise_obj, request));
    if (!ret_promise) {
      RequestOrResponse::close_if_cache_entry(cx, request);
      return false;
    }
    JS::SetReservedSlot(request, static_cast<uint32_t>(Request::Slots::ResponsePromise),
                        JS::ObjectValue(*ret_promise));
    args.rval().setObject(*ret_promise);
  }

  return true;
}

const JSFunctionSpec methods[] = {JS_FN("fetch", fetch, 2, JSPROP_ENUMERATE), JS_FS_END};

namespace ReadableStream_fastly_adds {

bool is_instance(JSObject *obj) { return JS::IsReadableStream(obj); }

bool is_instance(JS::Value val) { return val.isObject() && is_instance(&val.toObject()); }

bool check_receiver(JSContext *cx, JS::HandleValue receiver, const char *method_name) {
  if (!is_instance(receiver)) {
    return api::throw_error(cx, api::Errors::WrongReceiver, method_name, "ReadableStream");
  }
  return true;
};

static JS::PersistentRooted<JSObject *> rs_proto_obj;
static JS::PersistentRooted<JS::Value> original_pipeTo;
static JS::PersistentRooted<JS::Value> overridden_pipeTo;

bool pipeTo_wrapper(JSContext *cx, unsigned argc, JS::Value *vp) {
  __builtin_trap();
  std::cerr << "wrapper getting called???" << std::endl;
  METHOD_HEADER(1)

  bool ret(JS::Call(cx, args.thisv(), original_pipeTo, JS::HandleValueArray(args), args.rval()));

  JS::RootedObject target(cx, args[0].isObject() ? &args[0].toObject() : nullptr);
  if (target) {
    JS::RootedObject source(cx, NativeStreamSource::get_stream_source(cx, self));
    if (NativeStreamSource::is_instance(source)) {
      JS::RootedObject src_owner(cx, NativeStreamSource::owner(source));
      JS::RootedObject proxy(
          cx, JS::GetReservedSlot(source, NativeStreamSource::Slots::PipedToTransformStream)
                  .toObjectOrNull());
      if (proxy) {
        JS::RootedObject proxy_owner(cx, TransformStream::owner(proxy));
        if (RequestOrResponse::is_instance(proxy_owner)) {
          std::cout << "wrapper getting called???" << std::endl;
          JS::SetReservedSlot(proxy_owner,
                              static_cast<uint32_t>(RequestOrResponse::Slots::ShortcuttedVia),
                              ObjectValue(*src_owner));
        }
      }
    }
  }

  return ret;
}

bool initialize_overridden_pipeTo(JSContext *cx, JS::HandleObject global) {
  JS::RootedValue val(cx);
  if (!JS_GetProperty(cx, global, "ReadableStream", &val)) {
    return false;
  }
  JS::RootedObject readableStream_builtin(cx, &val.toObject());

  if (!JS_GetProperty(cx, readableStream_builtin, "prototype", &val)) {
    return false;
  }
  rs_proto_obj.init(cx, &val.toObject());
  MOZ_ASSERT(rs_proto_obj);

  original_pipeTo.init(cx);
  overridden_pipeTo.init(cx);
  if (!JS_GetProperty(cx, rs_proto_obj, "pipeTo", &original_pipeTo)) {
    return false;
  }
  MOZ_ASSERT(JS::IsCallable(&original_pipeTo.toObject()));

  std::cerr << "overriding pipeTo" << std::endl;
  JSFunction *pipeTo_fun =
      JS_DefineFunction(cx, rs_proto_obj, "pipeTo", pipeTo_wrapper, 1, JSPROP_ENUMERATE);
  if (!pipeTo_fun) {
    return false;
  }

  overridden_pipeTo.setObject(*JS_GetFunctionObject(pipeTo_fun));

  return true;
}

} // namespace ReadableStream_fastly_adds

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
  if (!builtins::web::fetch::Headers::init_class(ENGINE->cx(), ENGINE->global())) {
    return false;
  }
  if (!ReadableStream_fastly_adds::initialize_overridden_pipeTo(engine->cx(), engine->global())) {
    return false;
  }
  return true;
}

} // namespace fastly::fetch
