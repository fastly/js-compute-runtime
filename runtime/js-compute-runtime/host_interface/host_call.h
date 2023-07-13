#ifndef JS_COMPUTE_RUNTIME_HOST_CALL_H
#define JS_COMPUTE_RUNTIME_HOST_CALL_H

#include <cstdint>

// TODO: remove these once the warnings are fixed
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-offsetof"
#pragma clang diagnostic ignored "-Wdeprecated-enum-enum-conversion"
#include "jsapi.h"
#pragma clang diagnostic pop

using FastlyError = uint8_t;

bool error_is_generic(FastlyError e);
bool error_is_invalid_argument(FastlyError e);
bool error_is_optional_none(FastlyError e);
bool error_is_bad_handle(FastlyError e);

void handle_fastly_error(JSContext *cx, FastlyError err, int line, const char *func);

#define HANDLE_ERROR(cx, err) handle_fastly_error(cx, err, __LINE__, __func__)

#define HOSTCALL_BUFFER_LEN HEADER_MAX_LEN

#endif
