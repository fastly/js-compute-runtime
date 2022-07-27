// TODO: remove these once the warnings are fixed
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-offsetof"
#include "js/experimental/TypedData.h" // used within "js/Stream.h"
#pragma clang diagnostic pop

#include "js/Stream.h"

#include "builtin.h"
#include "builtins/native-stream-sink.h"
#include "builtins/native-stream-source.h"
#include "js-compute-builtins.h"

// A JS class to use as the underlying source for native readable streams, used
// for Request/Response bodies and TransformStream.
namespace NativeStreamSource {

JSObject *owner(JSObject *self) {
  MOZ_ASSERT(is_instance(self));
  return &JS::GetReservedSlot(self, Slots::Owner).toObject();
}

JSObject *stream(JSObject *self) { return RequestOrResponse::body_stream(owner(self)); }

JS::Value startPromise(JSObject *self) {
  MOZ_ASSERT(is_instance(self));
  return JS::GetReservedSlot(self, Slots::StartPromise);
}

PullAlgorithm *pullAlgorithm(JSObject *self) {
  MOZ_ASSERT(is_instance(self));
  return (PullAlgorithm *)JS::GetReservedSlot(self, Slots::PullAlgorithm).toPrivate();
}

CancelAlgorithm *cancelAlgorithm(JSObject *self) {
  MOZ_ASSERT(is_instance(self));
  return (CancelAlgorithm *)JS::GetReservedSlot(self, Slots::CancelAlgorithm).toPrivate();
}

JSObject *controller(JSObject *self) {
  MOZ_ASSERT(is_instance(self));
  return &JS::GetReservedSlot(self, Slots::Controller).toObject();
}

/**
 * Returns the underlying source for the given controller iff it's an object,
 * nullptr otherwise.
 */
static JSObject *get_controller_source(JSContext *cx, JS::HandleObject controller) {
  JS::RootedValue source(cx);
  bool success __attribute__((unused));
  success = JS::ReadableStreamControllerGetUnderlyingSource(cx, controller, &source);
  MOZ_ASSERT(success);
  return source.isObject() ? &source.toObject() : nullptr;
}

JSObject *get_stream_source(JSContext *cx, JS::HandleObject stream) {
  MOZ_ASSERT(JS::IsReadableStream(stream));
  JS::RootedObject controller(cx, JS::ReadableStreamGetController(cx, stream));
  return get_controller_source(cx, controller);
}

bool stream_has_native_source(JSContext *cx, JS::HandleObject stream) {
  JSObject *source = get_stream_source(cx, stream);
  return is_instance(source);
}

bool stream_is_body(JSContext *cx, JS::HandleObject stream) {
  JSObject *stream_source = get_stream_source(cx, stream);
  return NativeStreamSource::is_instance(stream_source) &&
         RequestOrResponse::is_instance(owner(stream_source));
}

void set_stream_piped_to_ts_writable(JSContext *cx, JS::HandleObject stream,
                                     JS::HandleObject writable) {
  JS::RootedObject source(cx, NativeStreamSource::get_stream_source(cx, stream));
  MOZ_ASSERT(is_instance(source));
  JS::RootedObject sink(cx, NativeStreamSink::get_stream_sink(cx, writable));
  JS::RootedObject transform_stream(cx, NativeStreamSink::owner(sink));
  MOZ_ASSERT(transform_stream);
  JS::SetReservedSlot(source, Slots::PipedToTransformStream, JS::ObjectValue(*transform_stream));
}

JSObject *piped_to_transform_stream(JSObject *self) {
  MOZ_ASSERT(is_instance(self));
  return JS::GetReservedSlot(self, Slots::PipedToTransformStream).toObjectOrNull();
}

bool lock_stream(JSContext *cx, JS::HandleObject stream) {
  MOZ_ASSERT(JS::IsReadableStream(stream));

  bool locked;
  JS::ReadableStreamIsLocked(cx, stream, &locked);
  if (locked) {
    JS_ReportErrorLatin1(cx, "Can't lock an already locked ReadableStream");
    return false;
  }

  JS::RootedObject self(cx, get_stream_source(cx, stream));
  MOZ_ASSERT(is_instance(self));

  auto mode = JS::ReadableStreamReaderMode::Default;
  JS::RootedObject reader(cx, JS::ReadableStreamGetReader(cx, stream, mode));
  if (!reader)
    return false;

  JS::SetReservedSlot(self, Slots::InternalReader, JS::ObjectValue(*reader));
  return true;
}

const unsigned ctor_length = 0;

bool check_receiver(JSContext *cx, JS::HandleValue receiver, const char *method_name);

bool start(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(1)

  MOZ_ASSERT(args[0].isObject());
  JS::RootedObject controller(cx, &args[0].toObject());
  MOZ_ASSERT(get_controller_source(cx, controller) == self);

  JS::SetReservedSlot(self, NativeStreamSource::Slots::Controller, args[0]);

  // For TransformStream, StartAlgorithm returns the same Promise for both the
  // readable and writable stream. All other native initializations of
  // ReadableStream have StartAlgorithm return undefined. Instead of introducing
  // both the StartAlgorithm as a pointer and startPromise as a value, we just
  // store the latter or undefined, and always return it.
  args.rval().set(startPromise(self));
  return true;
}

bool pull(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(1)

  JS::RootedObject owner(cx, NativeStreamSource::owner(self));
  JS::RootedObject controller(cx, &args[0].toObject());
  MOZ_ASSERT(controller == NativeStreamSource::controller(self));
  MOZ_ASSERT(get_controller_source(cx, controller) == self.get());

  PullAlgorithm *pull = pullAlgorithm(self);
  return pull(cx, args, self, owner, controller);
}

bool cancel(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0)

  JS::RootedObject owner(cx, NativeStreamSource::owner(self));
  JS::HandleValue reason(args.get(0));

  CancelAlgorithm *cancel = cancelAlgorithm(self);
  return cancel(cx, args, self, owner, reason);
}

const JSFunctionSpec methods[] = {JS_FN("start", start, 1, 0), JS_FN("pull", pull, 1, 0),
                                  JS_FN("cancel", cancel, 1, 0), JS_FS_END};

const JSPropertySpec properties[] = {JS_PS_END};

CLASS_BOILERPLATE_NO_CTOR(NativeStreamSource)

JSObject *create(JSContext *cx, JS::HandleObject owner, JS::HandleValue startPromise,
                 PullAlgorithm *pull, CancelAlgorithm *cancel) {
  JS::RootedObject source(cx, JS_NewObjectWithGivenProto(cx, &class_, proto_obj));
  if (!source)
    return nullptr;

  JS::SetReservedSlot(source, Slots::Owner, JS::ObjectValue(*owner));
  JS::SetReservedSlot(source, Slots::StartPromise, startPromise);
  JS::SetReservedSlot(source, Slots::PullAlgorithm, JS::PrivateValue((void *)pull));
  JS::SetReservedSlot(source, Slots::CancelAlgorithm, JS::PrivateValue((void *)cancel));
  JS::SetReservedSlot(source, Slots::PipedToTransformStream, JS::NullValue());
  return source;
}
} // namespace NativeStreamSource
