
#include "host_call.h"

bool handle_fastly_result(JSContext *cx, int result, int line, const char *func) {
  FastlyStatus status = convert_to_fastly_status(result);
  return handle_fastly_result(cx, status, line, func);
}

bool handle_fastly_result(JSContext *cx, fastly_error_t result, int line, const char *func) {
  FastlyStatus status = convert_to_fastly_status(result);
  return handle_fastly_result(cx, status, line, func);
}

FastlyStatus convert_to_fastly_status(int result) {
  switch (result) {
  case 0:
    return FastlyStatus::Ok;
  case 1:
    return FastlyStatus::Error;
  case 2:
    return FastlyStatus::Inval;
  case 3:
    return FastlyStatus::BadF;
  case 4:
    return FastlyStatus::BufLen;
  case 5:
    return FastlyStatus::Unsupported;
  case 6:
    return FastlyStatus::BadAlign;
  case 7:
    return FastlyStatus::HttpInvalid;
  case 8:
    return FastlyStatus::HttpUser;
  case 9:
    return FastlyStatus::HttpIncomplete;
  case 10:
    return FastlyStatus::None;
  case 11:
    return FastlyStatus::HttpHeadTooLarge;
  case 12:
    return FastlyStatus::HttpInvalidStatus;
  default:
    MOZ_ASSERT_UNREACHABLE("coding error");
    return FastlyStatus::Unknown;
  }
}

FastlyStatus convert_to_fastly_status(fastly_error_t result) {
  switch (result) {
  case FASTLY_RESULT_ERROR_OK:
    return FastlyStatus::Ok;
  case FASTLY_ERROR_GENERIC_ERROR:
    return FastlyStatus::Error;
  case FASTLY_ERROR_INVALID_ARGUMENT:
    return FastlyStatus::Inval;
  case FASTLY_ERROR_BAD_HANDLE:
    return FastlyStatus::BadF;
  case FASTLY_ERROR_BUFFER_LEN:
    return FastlyStatus::BufLen;
  case FASTLY_ERROR_UNSUPPORTED:
    return FastlyStatus::Unsupported;
  case FASTLY_ERROR_BAD_ALIGN:
    return FastlyStatus::BadAlign;
  case FASTLY_ERROR_HTTP_INVALID:
    return FastlyStatus::HttpInvalid;
  case FASTLY_ERROR_HTTP_USER:
    return FastlyStatus::HttpUser;
  case FASTLY_ERROR_HTTP_INCOMPLETE:
    return FastlyStatus::HttpIncomplete;
  case FASTLY_ERROR_MISSING_OPTIONAL:
    return FastlyStatus::None;
  case FASTLY_ERROR_HTTP_HEAD_TOO_LARGE:
    return FastlyStatus::HttpHeadTooLarge;
  case FASTLY_ERROR_HTTP_INVALID_STATUS:
    return FastlyStatus::HttpInvalidStatus;
  default:
    MOZ_ASSERT_UNREACHABLE("coding error");
    return FastlyStatus::Unknown;
  }
}

bool OwnedHostCallBuffer::initialize(JSContext *cx) {
  // Ensure the buffer is all zeros so it doesn't add too much to the
  // snapshot.
  hostcall_buffer = (char *)js_calloc(HOSTCALL_BUFFER_LEN);
  return !!hostcall_buffer;
}

OwnedHostCallBuffer::OwnedHostCallBuffer() {
  MOZ_RELEASE_ASSERT(hostcall_buffer != nullptr);
  borrowed_buffer = hostcall_buffer;
  hostcall_buffer = nullptr;
}

char *OwnedHostCallBuffer::get() { return borrowed_buffer; }

OwnedHostCallBuffer::~OwnedHostCallBuffer() {
  // TODO: consider adding a build config that makes this zero the buffer.
  hostcall_buffer = borrowed_buffer;
}

char *OwnedHostCallBuffer::hostcall_buffer;
