#ifndef JS_COMPUTE_RUNTIME_ENCODE_H
#define JS_COMPUTE_RUNTIME_ENCODE_H

#include "host_interface/host_api.h"
#include "rust-url/rust-url.h"

namespace fastly::core {

// TODO(performance): introduce a version that writes into an existing buffer, and use that
// with the hostcall buffer where possible.
// https://github.com/fastly/js-compute-runtime/issues/215
HostString encode(JSContext *cx, JS::HandleString str);
HostString encode(JSContext *cx, JS::HandleValue val);

jsurl::SpecString encode_spec_string(JSContext *cx, JS::HandleValue val);

} // namespace fastly::core

#endif
