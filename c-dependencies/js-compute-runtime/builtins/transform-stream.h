#ifndef JS_COMPUTE_RUNTIME_TRANSFORM_STREAM_H
#define JS_COMPUTE_RUNTIME_TRANSFORM_STREAM_H

#include "builtin.h"

namespace TransformStream {
enum Slots {
  Controller,
  Readable,
  Writable,
  Backpressure,
  BackpressureChangePromise,
  Owner,       // The target RequestOrResponse object if the stream's readable end is
               // used as a body.
  UsedAsMixin, // `true` if the TransformStream is used in another transforming
               // builtin, such as CompressionStream.
  Count
};
bool is_instance(JSObject *obj);
bool is_instance(JS::Value val);
// Register the class.
bool init_class(JSContext *cx, JS::HandleObject global);

JSObject *controller(JSObject *self);
JSObject *readable(JSObject *self);
JSObject *create_rs_proxy(JSContext *cx, JS::HandleObject input_readable);
void set_used_as_mixin(JSObject *self);
JSObject *create(JSContext *cx, double writableHighWaterMark,
                 JS::HandleFunction writableSizeAlgorithm, double readableHighWaterMark,
                 JS::HandleFunction readableSizeAlgorithm, JS::HandleValue transformer,
                 JS::HandleObject startFunction, JS::HandleObject transformFunction,
                 JS::HandleObject flushFunction);
bool Error(JSContext *cx, JS::HandleObject stream, JS::HandleValue error);
bool ErrorWritableAndUnblockWrite(JSContext *cx, JS::HandleObject stream, JS::HandleValue error);
bool SetBackpressure(JSContext *cx, JS::HandleObject stream, bool backpressure);
void set_readable_used_as_body(JSContext *cx, JS::HandleObject readable, JS::HandleObject target);
JSObject *writable(JSObject *self);
bool backpressure(JSObject *self);
bool is_ts_writable(JSContext *cx, JS::HandleObject writable);
bool is_ts_readable(JSContext *cx, JS::HandleObject readable);
JSObject *owner(JSObject *self);
bool readable_used_as_body(JSObject *self);
JSObject *ts_from_readable(JSContext *cx, JS::HandleObject readable);
bool used_as_mixin(JSObject *self);
} // namespace TransformStream

#endif
