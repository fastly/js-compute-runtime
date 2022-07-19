#ifndef JS_COMPUTE_RUNTIME_HOST_CALL_H
#define JS_COMPUTE_RUNTIME_HOST_CALL_H

#include "js-compute-builtins.h"

#include "xqd.h"

/* Returns false if an exception is set on `cx` and the caller should
   immediately return to propagate the exception. */
static inline bool handle_fastly_result(JSContext *cx, int result, int line, const char *func) {
  switch (result) {
  case 0:
    return true;
  case 1:
    JS_ReportErrorUTF8(cx,
                       "%s: Generic error value. This means that some unexpected error "
                       "occurred during a hostcall. - Fastly error code %d\n",
                       func, result);
    return false;
  case 2:
    JS_ReportErrorUTF8(cx, "%s: Invalid argument. - Fastly error code %d\n", func, result);
    return false;
  case 3:
    JS_ReportErrorUTF8(cx,
                       "%s: Invalid handle. Thrown when a request, response, dictionary, or "
                       "body handle is not valid. - Fastly error code %d\n",
                       func, result);
    return false;
  case 4:
    JS_ReportErrorUTF8(cx, "%s: Buffer length error. Buffer is too long. - Fastly error code %d\n",
                       func, result);
    return false;
  case 5:
    JS_ReportErrorUTF8(cx,
                       "%s: Unsupported operation error. This error is thrown "
                       "when some operation cannot be performed, because it is "
                       "not supported. - Fastly error code %d\n",
                       func, result);
    return false;
  case 6:
    JS_ReportErrorUTF8(cx,
                       "%s: Alignment error. This is thrown when a pointer does not point to "
                       "a properly aligned slice of memory. - Fastly error code %d\n",
                       func, result);
    return false;
  case 7:
    JS_ReportErrorUTF8(cx,
                       "%s: HTTP parse error. This can be thrown when a method, URI, header, "
                       "or status is not valid. This can also be thrown if a message head is "
                       "too large. - Fastly error code %d\n",
                       func, result);
    return false;
  case 8:
    JS_ReportErrorUTF8(cx,
                       "%s: HTTP user error. This is thrown in cases where user code caused "
                       "an HTTP error. For example, attempt to send a 1xx response code, or a "
                       "request with a non-absolute URI. This can also be caused by an "
                       "unexpected header: both `content-length` and `transfer-encoding`, for "
                       "example. - Fastly error code %d\n",
                       func, result);
    return false;
  case 9:
    JS_ReportErrorUTF8(cx,
                       "%s: HTTP incomplete message error. A stream ended "
                       "unexpectedly. - Fastly error code %d\n",
                       func, result);
    return false;
  case 10:
    JS_ReportErrorUTF8(cx,
                       "%s: A `None` error. This status code is used to "
                       "indicate when an optional value did not exist, as "
                       "opposed to an empty value. - Fastly error code %d\n",
                       func, result);
    return false;
  case 11:
    JS_ReportErrorUTF8(cx,
                       "%s: HTTP head too large error. This error will be thrown when the "
                       "message head is too large. - Fastly error code %d\n",
                       func, result);
    return false;
  case 12:
    JS_ReportErrorUTF8(cx,
                       "%s: HTTP invalid status error. This error will be "
                       "thrown when the HTTP message contains an invalid "
                       "status code. - Fastly error code %d\n",
                       func, result);
    return false;
  default:
    fprintf(stdout, __FILE__ ":%d (%s) - Fastly error code %d\n", line, func, result);
    JS_ReportErrorUTF8(cx, "Fastly error code %d", result);
    return false;
  }
}

#define HANDLE_RESULT(cx, result) handle_fastly_result(cx, result, __LINE__, __func__)

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
