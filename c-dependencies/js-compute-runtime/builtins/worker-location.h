#ifndef JS_COMPUTE_RUNTIME_WROKER_LOCATION_H
#define JS_COMPUTE_RUNTIME_WROKER_LOCATION_H

#include "builtin.h"

namespace WorkerLocation {
JS::PersistentRooted<JSObject *> url;
// Register the class.
bool init_class(JSContext *cx, JS::HandleObject global);
} // namespace WorkerLocation

#endif
