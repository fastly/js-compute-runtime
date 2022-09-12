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

#include "rust-url/rust-url.h"
#include "xqd.h"

struct JSErrorFormatString;

enum JSErrNum {
#define MSG_DEF(name, count, exception, format) name,
#include "./error-numbers.msg"
#undef MSG_DEF
  JSErrNum_Limit
};

const JSErrorFormatString js_ErrorFormatString[JSErrNum_Limit] = {
#define MSG_DEF(name, count, exception, format) {#name, format, count, exception},
#include "./error-numbers.msg"
#undef MSG_DEF
};

const JSErrorFormatString *GetErrorMessage(void *userRef, unsigned errorNumber);

JSObject *PromiseRejectedWithPendingError(JSContext *cx);
inline bool ReturnPromiseRejectedWithPendingError(JSContext *cx, const JS::CallArgs &args) {
  JSObject *promise = PromiseRejectedWithPendingError(cx);
  if (!promise) {
    return false;
  }

  args.rval().setObject(*promise);
  return true;
}

uint8_t *value_to_buffer(JSContext *cx, JS::HandleValue val, const char *val_desc, size_t *len);

typedef bool InternalMethod(JSContext *cx, JS::HandleObject receiver, JS::HandleValue extra,
                            JS::CallArgs args);

template <InternalMethod fun> bool internal_method(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS::CallArgs args = CallArgsFromVp(argc, vp);
  JS::RootedObject self(cx, &js::GetFunctionNativeReserved(&args.callee(), 0).toObject());
  JS::RootedValue extra(cx, js::GetFunctionNativeReserved(&args.callee(), 1));
  return fun(cx, self, extra, args);
}

template <InternalMethod fun>
JSObject *create_internal_method(JSContext *cx, JS::HandleObject receiver,
                                 JS::HandleValue extra = JS::UndefinedHandleValue,
                                 unsigned int nargs = 0, const char *name = "") {
  JSFunction *method = js::NewFunctionWithReserved(cx, internal_method<fun>, 1, 0, name);
  if (!method)
    return nullptr;
  JS::RootedObject method_obj(cx, JS_GetFunctionObject(method));
  js::SetFunctionNativeReserved(method_obj, 0, JS::ObjectValue(*receiver));
  js::SetFunctionNativeReserved(method_obj, 1, extra);
  return method_obj;
}

bool hasWizeningFinished();
bool isWizening();
void markWizeningAsFinished();

bool define_fastly_sys(JSContext *cx, JS::HandleObject global);

namespace RequestOrResponse {
namespace Slots {
enum {
  RequestOrResponse,
  Body,
  BodyStream,
  BodyAllPromise,
  HasBody,
  BodyUsed,
  Headers,
  URL,
  Count
};
};
bool is_instance(JSObject *obj);
JSObject *body_stream(JSObject *obj);
enum class BodyReadResult {
  ArrayBuffer,
  JSON,
  Text,
};

bool body_used(JSObject *obj);
bool body_get(JSContext *cx, JS::CallArgs args, JS::HandleObject self, bool create_if_undefined);
bool body_unusable(JSContext *cx, JS::HandleObject body);
BodyHandle body_handle(JSObject *obj);
template <BodyReadResult result_type>
bool bodyAll(JSContext *cx, JS::CallArgs args, JS::HandleObject self);
JS::Value url(JSObject *obj);
} // namespace RequestOrResponse

int write_to_body_all(BodyHandle handle, const char *buf, size_t len);

bool RejectPromiseWithPendingError(JSContext *cx, JS::HandleObject promise);

namespace URL {
bool is_instance(JS::Value val);
}

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

JS::UniqueChars encode(JSContext *cx, JS::HandleString val, size_t *encoded_len);
JS::UniqueChars encode(JSContext *cx, JS::HandleValue val, size_t *encoded_len);
jsurl::SpecString encode(JSContext *cx, JS::HandleValue val);

bool debug_logging_enabled();
bool dump_value(JSContext *cx, JS::Value value, FILE *fp);
void dump_promise_rejection(JSContext *cx, JS::HandleValue reason, JS::HandleObject promise,
                            FILE *fp);
bool print_stack(JSContext *cx, FILE *fp);
bool print_stack(JSContext *cx, JS::HandleObject stack, FILE *fp);

#endif // fastly_sys_h
