#ifndef JS_COMPUTE_RUNTIME_LOGGER_H
#define JS_COMPUTE_RUNTIME_LOGGER_H

#include "builtin.h"

namespace Logger {
// Register the Logger class.
bool init_class(JSContext *cx, JS::HandleObject global);

// Create an instance of the logger class.
JSObject *create(JSContext *cx, const char *name);
} // namespace Logger

#endif
