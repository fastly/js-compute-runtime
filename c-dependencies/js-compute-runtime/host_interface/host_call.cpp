
#include "host_interface/host_call.h"

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
