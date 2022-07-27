// TODO: remove these once the warnings are fixed
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-offsetof"
#include "js/experimental/TypedData.h" // used within "js/Stream.h"
#pragma clang diagnostic pop

#include "js/Stream.h"

#include "builtin.h"
#include "builtins/native-stream-sink.h"
#include "js-compute-builtins.h"

// A JS class to use as the underlying sink for native writable streams, used
// for TransformStream.
namespace NativeStreamSink {
namespace Slots {
enum {
  Owner,          // TransformStream.
  Controller,     // The WritableStreamDefaultController.
  InternalWriter, // Only used to lock the stream if it's consumed internally.
  StartPromise,   // Used as the return value of `start`, can be undefined.
                  // Needed to properly implement TransformStream.
  WriteAlgorithm,
  AbortAlgorithm,
  CloseAlgorithm,
  // AbortAlgorithm, TODO: implement
  Count
};
};

JSObject *owner(JSObject *self) { return &JS::GetReservedSlot(self, Slots::Owner).toObject(); }

JS::Value startPromise(JSObject *self) { return JS::GetReservedSlot(self, Slots::StartPromise); }

WriteAlgorithm *writeAlgorithm(JSObject *self) {
  return (WriteAlgorithm *)JS::GetReservedSlot(self, Slots::WriteAlgorithm).toPrivate();
}

AbortAlgorithm *abortAlgorithm(JSObject *self) {
  return (AbortAlgorithm *)JS::GetReservedSlot(self, Slots::AbortAlgorithm).toPrivate();
}

CloseAlgorithm *closeAlgorithm(JSObject *self) {
  return (CloseAlgorithm *)JS::GetReservedSlot(self, Slots::CloseAlgorithm).toPrivate();
}

JSObject *controller(JSObject *self) {
  return &JS::GetReservedSlot(self, Slots::Controller).toObject();
}

/**
 * Returns the underlying sink for the given controller iff it's an object,
 * nullptr otherwise.
 */
static JSObject *get_controller_sink(JSContext *cx, JS::HandleObject controller) {
  JS::RootedValue sink(cx, JS::WritableStreamControllerGetUnderlyingSink(cx, controller));
  return sink.isObject() ? &sink.toObject() : nullptr;
}

JSObject *get_stream_sink(JSContext *cx, JS::HandleObject stream) {
  JS::RootedObject controller(cx, JS::WritableStreamGetController(cx, stream));
  return get_controller_sink(cx, controller);
}

bool stream_has_native_sink(JSContext *cx, JS::HandleObject stream) {
  MOZ_RELEASE_ASSERT(JS::IsWritableStream(stream));

  JSObject *sink = get_stream_sink(cx, stream);
  return is_instance(sink);
}

const unsigned ctor_length = 0;

bool check_receiver(JSContext *cx, JS::HandleValue receiver, const char *method_name);

bool start(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(1)

  MOZ_ASSERT(args[0].isObject());
  JS::RootedObject controller(cx, &args[0].toObject());
  MOZ_ASSERT(get_controller_sink(cx, controller) == self);

  JS::SetReservedSlot(self, Slots::Controller, args[0]);

  // For TransformStream, StartAlgorithm returns the same Promise for both the
  // readable and writable stream. All other native initializations of
  // WritableStream have StartAlgorithm return undefined.
  //
  // Instead of introducing both the StartAlgorithm as a pointer and
  // startPromise as a value, we just store the latter or undefined, and always
  // return it.
  args.rval().set(startPromise(self));
  return true;
}

bool write(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(1)

  JS::RootedObject owner(cx, NativeStreamSink::owner(self));
  JS::HandleValue chunk(args[0]);

  WriteAlgorithm *write = writeAlgorithm(self);
  return write(cx, args, self, owner, chunk);
}

bool abort(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(1)

  JS::RootedObject owner(cx, NativeStreamSink::owner(self));
  JS::HandleValue reason(args[0]);

  AbortAlgorithm *abort = abortAlgorithm(self);
  return abort(cx, args, self, owner, reason);
}

bool close(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0)

  JS::RootedObject owner(cx, NativeStreamSink::owner(self));

  CloseAlgorithm *close = closeAlgorithm(self);
  return close(cx, args, self, owner);
}

const JSFunctionSpec methods[] = {JS_FN("start", start, 1, 0), JS_FN("write", write, 2, 0),
                                  JS_FN("abort", abort, 2, 0), JS_FN("close", close, 1, 0),
                                  JS_FS_END};

const JSPropertySpec properties[] = {JS_PS_END};

CLASS_BOILERPLATE_NO_CTOR(NativeStreamSink)

JSObject *create(JSContext *cx, JS::HandleObject owner, JS::HandleValue startPromise,
                 WriteAlgorithm *write, CloseAlgorithm *close, AbortAlgorithm *abort) {
  JS::RootedObject sink(cx, JS_NewObjectWithGivenProto(cx, &class_, proto_obj));
  if (!sink)
    return nullptr;

  JS::SetReservedSlot(sink, Slots::Owner, JS::ObjectValue(*owner));
  JS::SetReservedSlot(sink, Slots::StartPromise, startPromise);
  JS::SetReservedSlot(sink, Slots::WriteAlgorithm, JS::PrivateValue((void *)write));
  JS::SetReservedSlot(sink, Slots::AbortAlgorithm, JS::PrivateValue((void *)abort));
  JS::SetReservedSlot(sink, Slots::CloseAlgorithm, JS::PrivateValue((void *)close));
  return sink;
}
} // namespace NativeStreamSink
