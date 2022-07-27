#ifndef JS_COMPUTE_RUNTIME_DECOMPRESSION_STREAM_H
#define JS_COMPUTE_RUNTIME_DECOMPRESSION_STREAM_H

#include "builtin.h"

namespace DecompressionStream {
// Register the class.
bool init_class(JSContext *cx, JS::HandleObject global);
} // namespace DecompressionStream

#endif
