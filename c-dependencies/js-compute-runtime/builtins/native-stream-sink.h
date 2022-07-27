#ifndef JS_COMPUTE_RUNTIME_NATIVE_STREAM_SINK_H
#define JS_COMPUTE_RUNTIME_NATIVE_STREAM_SINK_H

namespace NativeStreamSink {
typedef bool WriteAlgorithm(JSContext *cx, JS::CallArgs args, JS::HandleObject stream,
                            JS::HandleObject owner, JS::HandleValue chunk);
typedef bool AbortAlgorithm(JSContext *cx, JS::CallArgs args, JS::HandleObject stream,
                            JS::HandleObject owner, JS::HandleValue reason);
typedef bool CloseAlgorithm(JSContext *cx, JS::CallArgs args, JS::HandleObject stream,
                            JS::HandleObject owner);
bool is_instance(JSObject *obj);
// Register the class.
bool init_class(JSContext *cx, JS::HandleObject global);
JSObject *create(JSContext *cx, JS::HandleObject owner, JS::HandleValue startPromise,
                 WriteAlgorithm *write, CloseAlgorithm *close, AbortAlgorithm *abort);
JSObject *get_stream_sink(JSContext *cx, JS::HandleObject stream);
JSObject *owner(JSObject *self);

} // namespace NativeStreamSink
#endif