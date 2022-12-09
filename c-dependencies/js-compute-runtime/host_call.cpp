
#include "host_call.h"

FastlyStatus convert_to_fastly_status(bool is_err, fastly_error_t error) {
  if (!is_err)
    return FastlyStatus::Ok;
  switch (error) {
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
  case FASTLY_ERROR_OPTIONAL_NONE:
    return FastlyStatus::None;
  case FASTLY_ERROR_HTTP_HEAD_TOO_LARGE:
    return FastlyStatus::HttpHeadTooLarge;
  case FASTLY_ERROR_HTTP_INVALID_STATUS:
    return FastlyStatus::HttpInvalidStatus;
  case FASTLY_ERROR_LIMIT_EXCEEDED:
    return FastlyStatus::LimitExceeded;
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
