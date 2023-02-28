#ifndef JS_COMPUTE_RUNTIME_OBJECT_STORE_H
#define JS_COMPUTE_RUNTIME_OBJECT_STORE_H

#include "builtin.h"
#include "js-compute-builtins.h"

namespace ObjectStoreEntry {
// Register the class.
bool init_class(JSContext *cx, JS::HandleObject global);
bool is_instance(JSObject *obj);
bool is_instance(JS::Value val);
} // namespace ObjectStoreEntry

namespace ObjectStore {
// Register the class.
bool init_class(JSContext *cx, JS::HandleObject global);
} // namespace ObjectStore

#endif
