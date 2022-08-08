#ifndef JS_COMPUTE_RUNTIME_BUILTIN_CONSOLE_H
#define JS_COMPUTE_RUNTIME_BUILTIN_CONSOLE_H

#include "builtin.h"

namespace Console {
bool create(JSContext *cx, JS::HandleObject global);
}

#endif
