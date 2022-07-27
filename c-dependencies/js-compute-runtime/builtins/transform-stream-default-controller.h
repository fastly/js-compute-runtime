#ifndef JS_COMPUTE_RUNTIME_TRANSFORM_STREAM_DEFAULT_CONTROLLER_H
#define JS_COMPUTE_RUNTIME_TRANSFORM_STREAM_DEFAULT_CONTROLLER_H

#include "builtin.h"

namespace TransformStreamDefaultController {
typedef JSObject *TransformAlgorithm(JSContext *cx, JS::HandleObject controller,
                                     JS::HandleValue chunk);
typedef JSObject *FlushAlgorithm(JSContext *cx, JS::HandleObject controller);

namespace Slots {
enum {
  Stream,
  Transformer,
  TransformAlgorithm,
  TransformInput, // JS::Value to be used by TransformAlgorithm, e.g. a
                  // JSFunction to call.
  FlushAlgorithm,
  FlushInput, // JS::Value to be used by FlushAlgorithm, e.g. a JSFunction to
              // call.
  Count
};
};

// Register the class.
bool init_class(JSContext *cx, JS::HandleObject global);
bool Enqueue(JSContext *cx, JS::HandleObject controller, JS::HandleValue chunk);
JSObject *PerformTransform(JSContext *cx, JS::HandleObject controller, JS::HandleValue chunk);
void ClearAlgorithms(JSObject *controller);
JSObject *SetUpFromTransformer(JSContext *cx, JS::HandleObject stream, JS::HandleValue transformer,
                               JS::HandleObject transformFunction, JS::HandleObject flushFunction);
FlushAlgorithm *flushAlgorithm(JSObject *controller);
} // namespace TransformStreamDefaultController

#endif
