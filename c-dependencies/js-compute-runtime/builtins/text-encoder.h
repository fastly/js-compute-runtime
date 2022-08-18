#ifndef JS_COMPUTE_RUNTIME_TEXT_ENCODER_H
#define JS_COMPUTE_RUNTIME_TEXT_ENCODER_H

#include "builtin.h"

namespace TextEncoder {
// Register the class.
bool init_class(JSContext *cx, JS::HandleObject global);
} // namespace TextEncoder

#endif
