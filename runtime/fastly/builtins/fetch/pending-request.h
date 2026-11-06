#ifndef FASTLY_PENDING_REQUEST_H
#define FASTLY_PENDING_REQUEST_H

#include "../../../StarlingMonkey/builtins/web/fetch/headers.h"
#include "../../host-api/host_api_fastly.h"
#include "builtin.h"

namespace fastly::fetch {

class PendingRequest final : public builtins::BuiltinNoConstructor<PendingRequest> {
public:
  static constexpr const char *class_name = "FastlyFetchPromiseMetadata";

  enum class Slots : uint8_t {
    Handle,
    Promise,
    Operations,
    Consumed,
    GuestCacheMiss,
    PreserveSurrogateHeaders,
    FetchTask,
    HandedOff,
    Count,
  };

  static const JSFunctionSpec static_methods[];
  static const JSPropertySpec static_properties[];
  static const JSFunctionSpec methods[];
  static const JSPropertySpec properties[];

  static bool init_class(JSContext *cx, JS::HandleObject global);
  static bool install_promise_helpers(JSContext *cx, JS::HandleObject global);
  static JSObject *create_handle(JSContext *cx, host_api::HttpPendingReq::Handle handle,
                                 JS::HandleObject promise);
  static JSObject *create_promise(JSContext *cx, JS::HandleObject promise);
  static JSObject *record_for_promise(JSContext *cx, JS::HandleObject promise);
  static JSObject *ensure_promise_record(JSContext *cx, JS::HandleObject promise);
  static bool is_native(JSObject *self);
  static JSObject *promise(JSObject *self);
  static bool consume(JSContext *cx, JS::HandleObject self);
  static void mark_committed(JSContext *cx, JS::HandleObject self);
  static void set_fetch_task(JSContext *cx, JS::HandleObject self, api::AsyncTask *task);
  static void clear_fetch_task(JSContext *cx, JS::HandleObject self);
  static bool was_handed_off(JSContext *cx, JS::HandleObject promise);
  static bool try_send_to_client(JSContext *cx, JS::HandleObject promise, bool *sent);
  static bool mark_guest_cache_miss_headers(JSContext *cx, JS::HandleObject self,
                                            JS::HandleObject request);
  static bool send_to_client(JSContext *cx, JS::HandleObject self);
  static bool apply_response_headers(JSContext *cx, JS::HandleObject self,
                                     JS::HandleObject response, bool error_response);
  static bool apply_error_headers(JSContext *cx, JS::HandleObject self,
                                  host_api::HttpResp &response);
};

} // namespace fastly::fetch

#endif