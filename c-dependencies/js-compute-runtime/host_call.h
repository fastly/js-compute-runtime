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

/* Returns false if an exception is set on `cx` and the caller should
   immediately return to propagate the exception. */
static inline void handle_fastly_error(JSContext *cx, fastly_error_t err, int line,
                                       const char *func) {
  switch (err) {
  case FASTLY_ERROR_GENERIC_ERROR:
    JS_ReportErrorUTF8(cx,
                       "%s: Generic error value. This means that some unexpected error "
                       "occurred during a hostcall. - Fastly error code %d\n",
                       func, err);

    break;
  case FASTLY_ERROR_INVALID_ARGUMENT:
    JS_ReportErrorUTF8(cx, "%s: Invalid argument. - Fastly error code %d\n", func, err);
    break;
  case FASTLY_ERROR_BAD_HANDLE:
    JS_ReportErrorUTF8(cx,
                       "%s: Invalid handle. Thrown when a request, response, dictionary, or "
                       "body handle is not valid. - Fastly error code %d\n",
                       func, err);
    break;
  case FASTLY_ERROR_BUFFER_LEN:
    JS_ReportErrorUTF8(cx, "%s: Buffer length error. Buffer is too long. - Fastly error code %d\n",
                       func, err);
    break;
  case FASTLY_ERROR_UNSUPPORTED:
    JS_ReportErrorUTF8(cx,
                       "%s: Unsupported operation error. This error is thrown "
                       "when some operation cannot be performed, because it is "
                       "not supported. - Fastly error code %d\n",
                       func, err);
    break;
  case FASTLY_ERROR_BAD_ALIGN:
    JS_ReportErrorUTF8(cx,
                       "%s: Alignment error. This is thrown when a pointer does not point to "
                       "a properly aligned slice of memory. - Fastly error code %d\n",
                       func, err);
    break;
  case FASTLY_ERROR_HTTP_INVALID:
    JS_ReportErrorUTF8(cx,
                       "%s: HTTP parse error. This can be thrown when a method, URI, header, "
                       "or status is not valid. This can also be thrown if a message head is "
                       "too large. - Fastly error code %d\n",
                       func, err);
    break;
  case FASTLY_ERROR_HTTP_USER:
    JS_ReportErrorUTF8(cx,
                       "%s: HTTP user error. This is thrown in cases where user code caused "
                       "an HTTP error. For example, attempt to send a 1xx response code, or a "
                       "request with a non-absolute URI. This can also be caused by an "
                       "unexpected header: both `content-length` and `transfer-encoding`, for "
                       "example. - Fastly error code %d\n",
                       func, err);
    break;
  case FASTLY_ERROR_HTTP_INCOMPLETE:
    JS_ReportErrorUTF8(cx,
                       "%s: HTTP incomplete message error. A stream ended "
                       "unexpectedly. - Fastly error code %d\n",
                       func, err);
    break;
  case FASTLY_ERROR_OPTIONAL_NONE:
    JS_ReportErrorUTF8(cx,
                       "%s: A `None` error. This status code is used to "
                       "indicate when an optional value did not exist, as "
                       "opposed to an empty value. - Fastly error code %d\n",
                       func, err);
    break;
  case FASTLY_ERROR_HTTP_HEAD_TOO_LARGE:
    JS_ReportErrorUTF8(cx,
                       "%s: HTTP head too large error. This error will be thrown when the "
                       "message head is too large. - Fastly error code %d\n",
                       func, err);
    break;
  case FASTLY_ERROR_HTTP_INVALID_STATUS:
    JS_ReportErrorUTF8(cx,
                       "%s: HTTP invalid status error. This error will be "
                       "thrown when the HTTP message contains an invalid "
                       "status code. - Fastly error code %d\n",
                       func, err);
    break;
  case FASTLY_ERROR_LIMIT_EXCEEDED:
    JS_ReportErrorUTF8(cx,
                       "%s: Limit exceeded error. This error will be thrown when an attempt"
                       "to allocate a resource has exceeded the maximum number of resources"
                       "permitted. For example, creating too many response handles."
                       " - Fastly error code %d\n",
                       func, err);
    break;
  default:
    fprintf(stdout, __FILE__ ":%d (%s) - Fastly error code %d\n", line, func, err);
    JS_ReportErrorUTF8(cx, "Fastly error code %d", err);
  }
}

#define HANDLE_ERROR(cx, err) handle_fastly_error(cx, err, __LINE__, __func__)

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
