#ifndef JS_COMPUTE_RUNTIME_OBJECT_STORE_H
#define JS_COMPUTE_RUNTIME_OBJECT_STORE_H

#include "builtin.h"
namespace ObjectStoreEntry {
// Register the class.
bool init_class(JSContext *cx, JS::HandleObject global);
} // namespace ObjectStoreEntry

namespace ObjectStore {
// Register the class.
bool init_class(JSContext *cx, JS::HandleObject global);
} // namespace ObjectStore

#endif
