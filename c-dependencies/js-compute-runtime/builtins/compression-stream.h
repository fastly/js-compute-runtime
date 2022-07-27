#ifndef JS_COMPUTE_RUNTIME_COMPRESSION_STREAM_H
#define JS_COMPUTE_RUNTIME_COMPRESSION_STREAM_H

#include "builtin.h"

namespace CompressionStream {
// Register the class.
bool init_class(JSContext *cx, JS::HandleObject global);
} // namespace CompressionStream

#endif
