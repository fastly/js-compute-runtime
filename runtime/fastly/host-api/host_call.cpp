
#include <type_traits>

#include "./fastly.h"
#include "./host_api_fastly.h"

using api::AsyncTask;

namespace fastly {
const JSErrorFormatString *FastlyGetErrorMessage(void *userRef, unsigned errorNumber) {
  if (errorNumber > 0 && errorNumber < JSErrNum_Limit) {
    return &fastly_ErrorFormatString[errorNumber];
  }
  return nullptr;
}

} // namespace fastly

namespace host_api {

static_assert(std::is_same_v<APIError, fastly::fastly_host_error>);

bool error_is_generic(APIError e) { return e == FASTLY_HOST_ERROR_GENERIC_ERROR; }

bool error_is_invalid_argument(APIError e) { return e == FASTLY_HOST_ERROR_INVALID_ARGUMENT; }

bool error_is_optional_none(APIError e) { return e == FASTLY_HOST_ERROR_OPTIONAL_NONE; }

bool error_is_bad_handle(APIError e) { return e == FASTLY_HOST_ERROR_BAD_HANDLE; }

bool error_is_unsupported(APIError e) { return e == FASTLY_HOST_ERROR_UNSUPPORTED; }

bool error_is_buffer_len(APIError e) { return e == FASTLY_HOST_ERROR_BUFFER_LEN; }

bool error_is_limit_exceeded(APIError e) { return e == FASTLY_HOST_ERROR_LIMIT_EXCEEDED; }

void handle_kv_error(JSContext *cx, FastlyKVError err, const unsigned int err_type, int line,
                     const char *func) {
  // kv error was a host call error -> report as host error
  if (err.detail == FastlyKVError::detail::host_error) {
    return handle_api_error(cx, err.host_error, line, func);
  }

  // kv error is a normal kv error -> report as KV error
  std::string message = std::move(err.message()).value_or("when attempting to fetch resource.");
  JS_ReportErrorNumberASCII(cx, fastly::FastlyGetErrorMessage, nullptr, err_type, message.c_str());
}

void handle_image_optimizer_error(JSContext *cx, const FastlyImageOptimizerError &err, int line,
                                  const char *func) {
  if (err.is_host_error) {
    return handle_api_error(cx, err.host_err, line, func);
  }

  std::string message = err.message().value();
  JS_ReportErrorUTF8(cx, "[Image Optimizer] %s", message.c_str());
}

/* Returns false if an exception is set on `cx` and the caller should
   immediately return to propagate the exception. */
void handle_api_error(JSContext *cx, APIError err, int line, const char *func) {
  switch (err) {
  case FASTLY_HOST_ERROR_GENERIC_ERROR:
    JS_ReportErrorUTF8(cx,
                       "%s: Generic error value. This means that some unexpected error "
                       "occurred during a hostcall.\n",
                       func);

    break;
  case FASTLY_HOST_ERROR_INVALID_ARGUMENT:
    JS_ReportErrorUTF8(cx, "%s: Invalid argument.\n", func);
    break;
  case FASTLY_HOST_ERROR_BAD_HANDLE:
    JS_ReportErrorUTF8(cx,
                       "%s: Invalid handle. Thrown when a request, response, dictionary, or "
                       "body handle is not valid.\n",
                       func);
    break;
  case FASTLY_HOST_ERROR_BUFFER_LEN:
    JS_ReportErrorUTF8(cx, "%s: Buffer length error. Buffer is too long.\n", func);
    break;
  case FASTLY_HOST_ERROR_UNSUPPORTED:
    JS_ReportErrorUTF8(cx,
                       "%s: Unsupported operation error. This error is thrown "
                       "when some operation cannot be performed, because it is "
                       "not supported.\n",
                       func);
    break;
  case FASTLY_HOST_ERROR_BAD_ALIGN:
    JS_ReportErrorUTF8(cx,
                       "%s: Alignment error. This is thrown when a pointer does not point to "
                       "a properly aligned slice of memory.\n",
                       func);
    break;
  case FASTLY_HOST_ERROR_HTTP_INVALID:
    JS_ReportErrorUTF8(cx,
                       "%s: HTTP parse error. This can be thrown when a method, URI, header, "
                       "or status is not valid. This can also be thrown if a message head is "
                       "too large.\n",
                       func);
    break;
  case FASTLY_HOST_ERROR_HTTP_USER:
    JS_ReportErrorUTF8(cx,
                       "%s: HTTP user error. This is thrown in cases where user code caused "
                       "an HTTP error. For example, attempt to send a 1xx response code, or a "
                       "request with a non-absolute URI. This can also be caused by an "
                       "unexpected header: both `content-length` and `transfer-encoding`, for "
                       "example.\n",
                       func);
    break;
  case FASTLY_HOST_ERROR_HTTP_INCOMPLETE:
    JS_ReportErrorUTF8(cx,
                       "%s: HTTP incomplete message error. A stream ended "
                       "unexpectedly.\n",
                       func);
    break;
  case FASTLY_HOST_ERROR_OPTIONAL_NONE:
    JS_ReportErrorUTF8(cx,
                       "%s: A `None` error. This status code is used to "
                       "indicate when an optional value did not exist, as "
                       "opposed to an empty value.\n",
                       func);
    break;
  case FASTLY_HOST_ERROR_HTTP_HEAD_TOO_LARGE:
    JS_ReportErrorUTF8(cx,
                       "%s: HTTP head too large error. This error will be thrown when the "
                       "message head is too large.\n",
                       func);
    break;
  case FASTLY_HOST_ERROR_HTTP_INVALID_STATUS:
    JS_ReportErrorUTF8(cx,
                       "%s: HTTP invalid status error. This error will be "
                       "thrown when the HTTP message contains an invalid "
                       "status code.\n",
                       func);
    break;
  case FASTLY_HOST_ERROR_LIMIT_EXCEEDED:
    JS_ReportErrorUTF8(cx,
                       "%s: Limit exceeded error. This error will be thrown when an attempt "
                       "to allocate a resource has exceeded the maximum number of resources "
                       "permitted. For example, creating too many response handles.\n",
                       func);
    break;
  default:
    fprintf(stdout, __FILE__ ":%d (%s)\n", line, func);
    JS_ReportErrorUTF8(cx, "Fastly error code %d", err);
  }
}

} // namespace host_api
