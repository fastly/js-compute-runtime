#ifndef fastly_sys_h
#define fastly_sys_h

#include <optional>
#include <span>

// TODO: remove these once the warnings are fixed
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-offsetof"
#pragma clang diagnostic ignored "-Wdeprecated-enum-enum-conversion"
#include "jsapi.h"
#include "jsfriendapi.h"

#include "js/ForOfIterator.h"
#include "js/Object.h"
#include "js/Promise.h"

#pragma clang diagnostic pop

#include "host_interface/c-at-e.h"
#include "host_interface/host_call.h"
#include "rust-url/rust-url.h"

struct JSErrorFormatString;

extern JS::PersistentRootedObjectVector *pending_async_tasks;

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

std::optional<std::span<uint8_t>> value_to_buffer(JSContext *cx, JS::HandleValue val,
                                                  const char *val_desc);

using InternalMethod = bool(JSContext *cx, JS::HandleObject receiver, JS::HandleValue extra,
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

namespace GlobalProperties {
// Maps an encoded character to a value in the Base64 alphabet, per
// RFC 4648, Table 1. Invalid input characters map to UINT8_MAX.
// https://datatracker.ietf.org/doc/html/rfc4648#section-4

constexpr uint8_t nonAlphabet = 255;

static const uint8_t base64DecodeTable[] = {
    // clang-format off
  /* 0 */  nonAlphabet, nonAlphabet, nonAlphabet, nonAlphabet, nonAlphabet, nonAlphabet, nonAlphabet, nonAlphabet,
  /* 8 */  nonAlphabet, nonAlphabet, nonAlphabet, nonAlphabet, nonAlphabet, nonAlphabet, nonAlphabet, nonAlphabet,
  /* 16 */ nonAlphabet, nonAlphabet, nonAlphabet, nonAlphabet, nonAlphabet, nonAlphabet, nonAlphabet, nonAlphabet,
  /* 24 */ nonAlphabet, nonAlphabet, nonAlphabet, nonAlphabet, nonAlphabet, nonAlphabet, nonAlphabet, nonAlphabet,
  /* 32 */ nonAlphabet, nonAlphabet, nonAlphabet, nonAlphabet, nonAlphabet, nonAlphabet, nonAlphabet, nonAlphabet,
  /* 40 */ nonAlphabet, nonAlphabet, nonAlphabet,
  62 /* + */,
  nonAlphabet, nonAlphabet, nonAlphabet,
  63 /* / */,

  /* 48 */ /* 0 - 9 */ 52, 53, 54, 55, 56, 57, 58, 59,
  /* 56 */ 60, 61, nonAlphabet, nonAlphabet, nonAlphabet, nonAlphabet, nonAlphabet, nonAlphabet,

  /* 64 */ nonAlphabet, /* A - Z */ 0, 1, 2, 3, 4, 5, 6,
  /* 72 */ 7, 8, 9, 10, 11, 12, 13, 14,
  /* 80 */ 15, 16, 17, 18, 19, 20, 21, 22,
  /* 88 */ 23, 24, 25, nonAlphabet, nonAlphabet, nonAlphabet, nonAlphabet, nonAlphabet,
  /* 96 */ nonAlphabet, /* a - z */ 26, 27, 28, 29, 30, 31, 32,
  /* 104 */ 33, 34, 35, 36, 37, 38, 39, 40,
  /* 112 */ 41, 42, 43, 44, 45, 46, 47, 48,
  /* 120 */ 49, 50, 51, nonAlphabet, nonAlphabet, nonAlphabet, nonAlphabet, nonAlphabet,
};

// clang-format off
static const uint8_t base64URLDecodeTable[] = { 
  /* 0 */    nonAlphabet, nonAlphabet, nonAlphabet, nonAlphabet, nonAlphabet, nonAlphabet, nonAlphabet, nonAlphabet, 
  /* 8 */    nonAlphabet, nonAlphabet, nonAlphabet, nonAlphabet, nonAlphabet, nonAlphabet, nonAlphabet, nonAlphabet, 
  /* 16 */   nonAlphabet, nonAlphabet, nonAlphabet, nonAlphabet, nonAlphabet, nonAlphabet, nonAlphabet, nonAlphabet, 
  /* 24 */   nonAlphabet, nonAlphabet, nonAlphabet, nonAlphabet, nonAlphabet, nonAlphabet, nonAlphabet, nonAlphabet, 
  /* 32 */   nonAlphabet, nonAlphabet, nonAlphabet, nonAlphabet, nonAlphabet, nonAlphabet, nonAlphabet, nonAlphabet,
  /* 40 */   nonAlphabet, nonAlphabet, nonAlphabet,          62, nonAlphabet,          62, nonAlphabet,          63,
  /* 48 */            52,          53,          54,          55,          56,          57,          58,          59,
  /* 56 */            60,          61, nonAlphabet, nonAlphabet, nonAlphabet, nonAlphabet, nonAlphabet, nonAlphabet,
  /* 64 */   nonAlphabet,           0,           1,           2,           3,           4,           5,           6,
  /* 72 */             7,           8,           9,          10,          11,          12,          13,          14,
  /* 80 */            15,          16,          17,          18,          19,          20,          21,          22,
  /* 88 */            23,          24,          25, nonAlphabet, nonAlphabet, nonAlphabet, nonAlphabet,          63,
  /* 96 */   nonAlphabet,          26,          27,          28,          29,          30,          31,          32,
  /* 104 */           33,          34,          35,          36,          37,          38,          39,          40,
  /* 112 */           41,          42,          43,          44,          45,          46,          47,          48,
  /* 120 */           49,          50,          51, nonAlphabet, nonAlphabet, nonAlphabet, nonAlphabet, nonAlphabet
};

static const char base64EncodeTable[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                      "abcdefghijklmnopqrstuvwxyz"
                      "0123456789+/";

static const char base64URLEncodeTable[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                      "abcdefghijklmnopqrstuvwxyz"
                      "0123456789-_";

// clang-format on

std::string forgivingBase64Encode(std::string_view data, const char *encodeTable);
JS::Result<std::string> forgivingBase64Decode(std::string_view data, const uint8_t *decodeTable);
} // namespace GlobalProperties

bool has_pending_async_tasks();
bool process_pending_async_tasks(JSContext *cx);

JS::UniqueChars encode(JSContext *cx, JS::HandleString val, size_t *encoded_len);
JS::UniqueChars encode(JSContext *cx, JS::HandleValue val, size_t *encoded_len);
jsurl::SpecString encode(JSContext *cx, JS::HandleValue val);

bool debug_logging_enabled();
bool dump_value(JSContext *cx, JS::Value value, FILE *fp);
void dump_promise_rejection(JSContext *cx, JS::HandleValue reason, JS::HandleObject promise,
                            FILE *fp);
bool print_stack(JSContext *cx, FILE *fp);
bool print_stack(JSContext *cx, JS::HandleObject stack, FILE *fp);

JS::Result<std::string> forgivingBase64Decode(std::string_view data);
JS::Result<std::string> ConvertJSValueToByteString(JSContext *cx, JS::Handle<JS::Value> v);
JS::Result<std::string> ConvertJSValueToByteString(JSContext *cx, std::string v);

#endif // fastly_sys_h
