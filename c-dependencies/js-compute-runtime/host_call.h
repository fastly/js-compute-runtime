#ifndef JS_COMPUTE_RUNTIME_HOST_CALL_H
#define JS_COMPUTE_RUNTIME_HOST_CALL_H

// TODO: remove these once the warnings are fixed
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-offsetof"
#pragma clang diagnostic ignored "-Wdeprecated-enum-enum-conversion"
#include "jsapi.h"
#pragma clang diagnostic pop

#include "xqd-world/xqd_world_adapter.h"
#include "xqd.h"

enum class FastlyStatus {
  // Success value.
  // This indicates that a hostcall finished successfully.
  Ok = 0,
  // Generic error value.
  // This means that some unexpected error occurred during a hostcall.
  Error = 1,
  // Invalid argument.
  Inval = 2,
  // Invalid handle.
  // Thrown when a handle is not valid. E.G. No dictionary exists with the given name.
  BadF = 3,
  // Buffer length error.
  // Thrown when a buffer is too long.
  BufLen = 4,
  // Unsupported operation error.
  // This error is thrown when some operation cannot be performed, because it is not supported.
  Unsupported = 5,
  // Alignment error.
  // This is thrown when a pointer does not point to a properly aligned slice of memory.
  BadAlign = 6,
  // Invalid HTTP error.
  // This can be thrown when a method, URI, header, or status is not valid. This can also
  // be thrown if a message head is too large.
  HttpInvalid = 7,
  // HTTP user error.
  // This is thrown in cases where user code caused an HTTP error. For example, attempt to send
  // a 1xx response code, or a request with a non-absolute URI. This can also be caused by
  // an unexpected header: both `content-length` and `transfer-encoding`, for example.
  HttpUser = 8,
  // HTTP incomplete message error.
  // This can be thrown when a stream ended unexpectedly.
  HttpIncomplete = 9,
  // A `None` error.
  // This status code is used to indicate when an optional value did not exist, as opposed to
  // an empty value.
  None = 10,
  // Message head too large.
  HttpHeadTooLarge = 11,
  // Invalid HTTP status.
  HttpInvalidStatus = 12,
  // Limit Exceeded
  LimitExceeded = 13,
  // Unknown status.
  Unknown = 100,
};

/* Returns false if an exception is set on `cx` and the caller should
   immediately return to propagate the exception. */
static inline bool handle_fastly_result(JSContext *cx, FastlyStatus result, int line,
                                        const char *func) {
  switch (result) {
  case FastlyStatus::Ok:
    return true;
  case FastlyStatus::Error:
    JS_ReportErrorUTF8(cx,
                       "%s: Generic error value. This means that some unexpected error "
                       "occurred during a hostcall. - Fastly error code %d\n",
                       func, result);
    return false;
  case FastlyStatus::Inval:
    JS_ReportErrorUTF8(cx, "%s: Invalid argument. - Fastly error code %d\n", func, result);
    return false;
  case FastlyStatus::BadF:
    JS_ReportErrorUTF8(cx,
                       "%s: Invalid handle. Thrown when a request, response, dictionary, or "
                       "body handle is not valid. - Fastly error code %d\n",
                       func, result);
    return false;
  case FastlyStatus::BufLen:
    JS_ReportErrorUTF8(cx, "%s: Buffer length error. Buffer is too long. - Fastly error code %d\n",
                       func, result);
    return false;
  case FastlyStatus::Unsupported:
    JS_ReportErrorUTF8(cx,
                       "%s: Unsupported operation error. This error is thrown "
                       "when some operation cannot be performed, because it is "
                       "not supported. - Fastly error code %d\n",
                       func, result);
    return false;
  case FastlyStatus::BadAlign:
    JS_ReportErrorUTF8(cx,
                       "%s: Alignment error. This is thrown when a pointer does not point to "
                       "a properly aligned slice of memory. - Fastly error code %d\n",
                       func, result);
    return false;
  case FastlyStatus::HttpInvalid:
    JS_ReportErrorUTF8(cx,
                       "%s: HTTP parse error. This can be thrown when a method, URI, header, "
                       "or status is not valid. This can also be thrown if a message head is "
                       "too large. - Fastly error code %d\n",
                       func, result);
    return false;
  case FastlyStatus::HttpUser:
    JS_ReportErrorUTF8(cx,
                       "%s: HTTP user error. This is thrown in cases where user code caused "
                       "an HTTP error. For example, attempt to send a 1xx response code, or a "
                       "request with a non-absolute URI. This can also be caused by an "
                       "unexpected header: both `content-length` and `transfer-encoding`, for "
                       "example. - Fastly error code %d\n",
                       func, result);
    return false;
  case FastlyStatus::HttpIncomplete:
    JS_ReportErrorUTF8(cx,
                       "%s: HTTP incomplete message error. A stream ended "
                       "unexpectedly. - Fastly error code %d\n",
                       func, result);
    return false;
  case FastlyStatus::None:
    JS_ReportErrorUTF8(cx,
                       "%s: A `None` error. This status code is used to "
                       "indicate when an optional value did not exist, as "
                       "opposed to an empty value. - Fastly error code %d\n",
                       func, result);
    return false;
  case FastlyStatus::HttpHeadTooLarge:
    JS_ReportErrorUTF8(cx,
                       "%s: HTTP head too large error. This error will be thrown when the "
                       "message head is too large. - Fastly error code %d\n",
                       func, result);
    return false;
  case FastlyStatus::HttpInvalidStatus:
    JS_ReportErrorUTF8(cx,
                       "%s: HTTP invalid status error. This error will be "
                       "thrown when the HTTP message contains an invalid "
                       "status code. - Fastly error code %d\n",
                       func, result);
    return false;
  case FastlyStatus::LimitExceeded:
    JS_ReportErrorUTF8(cx,
                       "%s: Limit exceeded error. This error will be thrown when an attempt"
                       "to allocate a resource has exceeded the maximum number of resources"
                       "permitted. For example, creating too many response handles."
                       " - Fastly error code %d\n",
                       func, result);
    return false;
  default:
    fprintf(stdout, __FILE__ ":%d (%s) - Fastly error code %d\n", line, func, result);
    JS_ReportErrorUTF8(cx, "Fastly error code %d", result);
    return false;
  }
}

FastlyStatus convert_to_fastly_status(bool is_err, fastly_error_t error);

#define HANDLE_RESULT(cx, result, err)                                                             \
  handle_fastly_result(cx, convert_to_fastly_status(result, err), __LINE__, __func__)

#define HOSTCALL_BUFFER_LEN HEADER_MAX_LEN

class OwnedHostCallBuffer {
  static char *hostcall_buffer;
  char *borrowed_buffer;

public:
  static bool initialize(JSContext *cx);

  OwnedHostCallBuffer();
  ~OwnedHostCallBuffer();

  char *get();
};

#endif
