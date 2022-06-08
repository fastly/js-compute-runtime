#ifndef fastly_sys_h
#define fastly_sys_h

// TODO: remove these once the warnings are fixed
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-offsetof"

#include "jsapi.h"
#include "jsfriendapi.h"

#include "js/ForOfIterator.h"
#include "js/Object.h"
#include "js/Promise.h"

#pragma clang diagnostic pop

bool define_fastly_sys(JSContext *cx, JS::HandleObject global);

namespace FetchEvent {
enum class State {
  unhandled,
  waitToRespond,
  responseStreaming,
  responseDone,
  responsedWithError,
};

JSObject *create(JSContext *cx);
bool init_request(JSContext *cx, JS::HandleObject self);

// There can only ever be a single FetchEvent instance in a service, so we can
// treat it as a singleton for easy access. Returns a nullptr if the FetchEvent
// hasn't been created yet.
JS::HandleObject instance();

State state(JSObject *self);
void set_state(JSObject *self, State state);

// https://w3c.github.io/ServiceWorker/#extendableevent-active
bool is_active(JSObject *self);

bool is_dispatching(JSObject *self);
void start_dispatching(JSObject *self);
void stop_dispatching(JSObject *self);

bool response_started(JSObject *self);
bool respondWithError(JSContext *cx, JS::HandleObject self);
} // namespace FetchEvent

bool has_pending_requests();
bool process_network_io(JSContext *cx);

JS::UniqueChars encode(JSContext *cx, JS::HandleValue val, size_t *encoded_len);

bool debug_logging_enabled();
bool dump_value(JSContext *cx, JS::Value value, FILE *fp);
void dump_promise_rejection(JSContext *cx, JS::HandleValue reason, JS::HandleObject promise,
                            FILE *fp);
bool print_stack(JSContext *cx, FILE *fp);
bool print_stack(JSContext *cx, JS::HandleObject stack, FILE *fp);

#endif // fastly_sys_h
