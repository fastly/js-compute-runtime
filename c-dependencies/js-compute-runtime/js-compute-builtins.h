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

bool define_fastly_sys(JSContext* cx, JS::HandleObject global);

JSObject* create_downstream_request_event(JSContext* cx);

bool did_send_response();
bool send_error_response(JSContext* cx);

bool has_pending_requests();
bool process_network_io(JSContext* cx);

JS::UniqueChars encode(JSContext* cx, JS::HandleValue val, size_t* encoded_len);

bool debug_logging_enabled();
bool dump_value(JSContext* cx, JS::Value value, FILE* fp);
bool print_stack(JSContext* cx, FILE* fp);
bool print_stack(JSContext* cx, JS::HandleObject stack, FILE* fp);

#endif // fastly_sys_h
