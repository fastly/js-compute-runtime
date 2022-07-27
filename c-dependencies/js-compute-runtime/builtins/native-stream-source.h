#ifndef JS_COMPUTE_RUNTIME_NATIVE_STREAM_SOURCE_H
#define JS_COMPUTE_RUNTIME_NATIVE_STREAM_SOURCE_H

namespace NativeStreamSource {
namespace Slots {
enum {
  Owner,          // Request or Response object, or TransformStream.
  Controller,     // The ReadableStreamDefaultController.
  InternalReader, // Only used to lock the stream if it's consumed internally.
  StartPromise,   // Used as the return value of `start`, can be undefined.
                  // Needed to properly implement TransformStream.
  PullAlgorithm,
  CancelAlgorithm,
  PipedToTransformStream, // The TransformStream this source's stream is piped
                          // to, if any. Only applies if the source backs a
                          // RequestOrResponse's body.
  Count
};
};
bool is_instance(JSObject *obj);
// Register the class.
bool init_class(JSContext *cx, JS::HandleObject global);
typedef bool PullAlgorithm(JSContext *cx, JS::CallArgs args, JS::HandleObject stream,
                           JS::HandleObject owner, JS::HandleObject controller);
typedef bool CancelAlgorithm(JSContext *cx, JS::CallArgs args, JS::HandleObject stream,
                             JS::HandleObject owner, JS::HandleValue reason);

JSObject *create(JSContext *cx, JS::HandleObject owner, JS::HandleValue startPromise,
                 PullAlgorithm *pull, CancelAlgorithm *cancel);
JSObject *get_stream_source(JSContext *cx, JS::HandleObject stream);
JSObject *owner(JSObject *self);
JSObject *stream(JSObject *self);
bool stream_has_native_source(JSContext *cx, JS::HandleObject stream);
bool stream_is_body(JSContext *cx, JS::HandleObject stream);
bool lock_stream(JSContext *cx, JS::HandleObject stream);
JSObject *piped_to_transform_stream(JSObject *source);
JSObject *controller(JSObject *self);
void set_stream_piped_to_ts_writable(JSContext *cx, JS::HandleObject stream,
                                     JS::HandleObject writable);
} // namespace NativeStreamSource
#endif