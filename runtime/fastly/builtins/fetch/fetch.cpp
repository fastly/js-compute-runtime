#include "fetch.h"
#include "../../../StarlingMonkey/builtins/web/fetch/headers.h"
#include "../backend.h"
#include "../cache-override.h"
#include "../fastly.h"
#include "../fetch-event.h"
#include "./request-response.h"
#include "builtin.h"
#include "encode.h"
#include "extension-api.h"

using fastly::FastlyGetErrorMessage;
using fastly::backend::Backend;
using fastly::cache_override::CacheOverride;
using fastly::fastly::Fastly;
using fastly::fetch::Request;
using fastly::fetch_event::FetchEvent;

namespace {

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

} // namespace

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
bool should_use_guest_caching(JSContext *cx, HandleObject request, bool *should_use_cache) {
  *should_use_cache = true;

  // If we previously found guest caching unsupported then remember that
  if (http_caching_unsupported || !fastly::ENABLE_EXPERIMENTAL_HTTP_CACHE) {
    if (must_use_guest_caching(cx, request)) {
      if (!fastly::ENABLE_EXPERIMENTAL_HTTP_CACHE) {
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
    *should_use_cache = false;
    return true;
  }

  // Check for pass cache override
  MOZ_ASSERT(Request::is_instance(request));
  JS::RootedObject cache_override(
      cx, JS::GetReservedSlot(request, static_cast<uint32_t>(Request::Slots::CacheOverride))
              .toObjectOrNull());
  if (cache_override) {
    if (CacheOverride::mode(cache_override) == CacheOverride::CacheOverrideMode::Pass) {
      // Pass requests have to go through the host for now
      *should_use_cache = false;
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
    *should_use_cache = false;
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
      *should_use_cache = false;
      return true;
    }
    HANDLE_ERROR(cx, *err);
    return false;
  }

  return true;
}

// Sends the request body, resolving the response promise with the response
// The without_caching case is effectively pass semantics without cache hooks
template <bool without_caching>
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

  // cache override only applies to requests with caching
  if (!without_caching) {
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

  bool streaming = false;
  if (!RequestOrResponse::maybe_stream_body(cx, request, &streaming)) {
    return false;
  }

  host_api::HttpPendingReq pending_handle;
  {
    auto request_handle = Request::request_handle(request);
    auto body = RequestOrResponse::body_handle(request);
    auto res = !without_caching
                   ? streaming ? request_handle.send_async_streaming(body, backend_chars)
                               : request_handle.send_async(body, backend_chars)
                   : request_handle.send_async_without_caching(body, backend_chars, streaming);

    if (auto *err = res.to_err()) {
      if (host_api::error_is_generic(*err) || host_api::error_is_invalid_argument(*err)) {
        JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr,
                                  JSMSG_REQUEST_BACKEND_DOES_NOT_EXIST, backend_chars.ptr.get());
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
                                  JSMSG_REQUEST_BACKEND_DOES_NOT_EXIST, backend_chars.ptr.get());
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
    RootedObject requestInstance(
        cx, JS_NewObjectWithGivenProto(cx, &Request::class_, Request::proto_obj));
    if (!requestInstance) {
      return false;
    }
    RootedObject candidate_request(cx, Request::create(cx, requestInstance, backend_request_handle,
                                                       host_api::HttpBody{}, false));
    JS::SetReservedSlot(candidate_request,
                        static_cast<uint32_t>(RequestOrResponse::Slots::BodyUsed),
                        JS::BooleanValue(true));
    JS::SetReservedSlot(
        candidate_request, static_cast<uint32_t>(Request::Slots::CacheOverride),
        JS::GetReservedSlot(request, static_cast<uint32_t>(Request::Slots::CacheOverride)));

    JS::RootedValue ret_val(cx);
    JS::RootedValueArray<1> args(cx);
    args[0].set(JS::ObjectValue(*candidate_request));

    // now call before_send with the candidate_request, allowing any async work
    if (!JS::Call(cx, JS::NullHandleValue, before_send, args, &ret_val)) {
      ret_promise.setObject(*PromiseRejectedWithPendingError(cx));
      return true;
    }
    before_send_promise = JS::RootedObject(cx, JS::CallOriginalPromiseResolve(cx, ret_val));
    if (!before_send_promise) {
      return false;
    }
  } else {
    before_send_promise = JS::NewPromiseObject(cx, nullptr);
    JS::ResolvePromise(cx, before_send_promise, JS::UndefinedHandleValue);
  }
  // when we resume, we pick up in fetch_send_body_with_cache_hooks_origin_request
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

bool background_revalidation_then_handler(JSContext *cx, JS::HandleObject request,
                                          JS::HandleValue response, JS::CallArgs args) {
  DEBUG_LOG("background_revalidation_then_handler")
  JSObject *response_obj = &response.toObject();
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
    ENGINE->incr_event_loop_interest();
    // host_api::HttpBody body = body_res.unwrap();
    // TODO: apply body transform through to drive completion
    fprintf(stderr, "TODO: background revalidation body streaming");
    fflush(stderr);
    MOZ_ASSERT(false);
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

bool background_revalidation_catch_handler(JSContext *cx, JS::HandleObject request,
                                           JS::HandleValue promise_val, JS::CallArgs args) {
  DEBUG_LOG("background_revalidation_catch_handler")
  // we follow the Rust implementation calling "close" instead of "transaction_abandon" here
  // this could be reconsidered in future if alternative semantics are required
  RequestOrResponse::close_if_cache_entry(cx, request);
  ENGINE->decr_event_loop_interest();
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

bool stream_back_then_handler(JSContext *cx, JS::HandleObject request, JS::HandleValue extra,
                              JS::CallArgs args) {
  DEBUG_LOG("stream_back_then_handler")
  auto response = args.get(0);
  RootedObject response_obj(cx, &response.toObject());
  auto cache_entry = RequestOrResponse::take_cache_entry(response_obj, false).value();
  auto storage_action = Response::storage_action(response_obj).value();
  // Override cache write options is set to the final cache write options at the end of the response
  // process.
  auto cache_write_options = Response::override_cache_options(response_obj);
  MOZ_ASSERT(cache_write_options);
  switch (storage_action) {
  case host_api::HttpStorageAction::Insert: {
    auto insert_res = cache_entry.transaction_insert_and_stream_back(
        Response::response_handle(response_obj), cache_write_options);
    DEBUG_LOG("stream back transaction insert")
    if (auto *err = insert_res.to_err()) {
      HANDLE_ERROR(cx, *err);
      return false;
    }
    DEBUG_LOG("stream back transaction insert unwrap")
    auto [body, cache_entry] = insert_res.unwrap();
    DEBUG_LOG("stream back transaction insert unwrapped")

    // TODO: body stream handling
    // Stream origin response body insto the insert body
    DEBUG_LOG("stream back transaction insert append")
    auto append_res = body.append(RequestOrResponse::body_handle(response_obj));
    DEBUG_LOG("stream back transaction insert appended")

    if (auto *err = append_res.to_err()) {
      HANDLE_ERROR(cx, *err);
      JSObject *promise = PromiseRejectedWithPendingError(cx);
      if (!promise) {
        return false;
      }
      args.rval().setObject(*promise);
      return true;
    }
    DEBUG_LOG("stream back transaction insert append no error")

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
    DEBUG_LOG("stream back transaction insert all done")
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
  DEBUG_LOG("stream_back_catch_handler")
  // we follow the Rust implementation calling "close" instead of "transaction_abandon" here
  // this could be reconsidered in future if alternative semantics are required
  if (!RequestOrResponse::close_if_cache_entry(cx, request)) {
    return false;
  }
  // "rethrow" the streaming error
  JS_SetPendingException(cx, args.get(0), JS::ExceptionStackBehavior::DoNotCapture);
  return false;
}

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

  if (!RequestOrResponse::commit_headers(cx, request)) {
    return false;
  }

  // Determine if we should use guest-side caching
  bool should_use_guest_caching_out;
  if (!should_use_guest_caching(cx, request, &should_use_guest_caching_out)) {
    return false;
  }
  if (!should_use_guest_caching_out) {
    DEBUG_LOG("Using traditional fetch without cache API")
    return fetch_send_body<false>(cx, request, args.rval());
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
    DEBUG_LOG("Request not cacheable, using non-caching fetch")
    return fetch_send_body<true>(cx, request, args.rval());
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
    DEBUG_LOG("Transaction lookup error")
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

  auto state_res = cache_entry.get_state();
  if (auto *err = state_res.to_err()) {
    DEBUG_LOG("Cache state error")
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
  DEBUG_LOG(state_str)

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
    DEBUG_LOG("Have usable cached response")
    JS::RootedObject cached_response(cx, maybe_response.value());

    if (cache_state.must_insert_or_update()) {
      DEBUG_LOG("Usable response must insert or update background revalidation")
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
      JS::RootedObject ret_promise(cx, internal_method_then<background_revalidation_then_handler,
                                                            background_revalidation_catch_handler>(
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
    DEBUG_LOG("No usable response, and don't need to insert or update -> pass")
    // transaction entry is done
    if (!RequestOrResponse::close_if_cache_entry(cx, request)) {
      return false;
    }
    // request collapsing has been disabled: pass the original request to the origin without
    // updating the cache and without caching
    return fetch_send_body<true>(cx, request, args.rval());
  } else {
    DEBUG_LOG("No usable response, and must insert or update, running origin fetch hooks")
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
  return true;
}

} // namespace fastly::fetch
