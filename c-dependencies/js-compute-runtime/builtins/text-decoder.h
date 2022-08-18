#ifndef JS_COMPUTE_RUNTIME_TEXT_DECODER_H
#define JS_COMPUTE_RUNTIME_TEXT_DECODER_H

#include "builtin.h"

namespace TextDecoder {
// Register the class.
bool init_class(JSContext *cx, JS::HandleObject global);
} // namespace TextDecoder

#endif
