#include "logger.h"
#include "host_call.h"

namespace builtins {

bool Logger::log(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(1)

  auto endpoint =
      LogEndpointHandle{(uint32_t)JS::GetReservedSlot(self, Logger::Slots::Endpoint).toInt32()};

  size_t msg_len;
  JS::UniqueChars msg = encode(cx, args.get(0), &msg_len);
  if (!msg)
    return false;

  size_t nwritten;
  if (!HANDLE_RESULT(cx, xqd_log_write(endpoint, msg.get(), msg_len, &nwritten)))
    return false;

  args.rval().setUndefined();
  return true;
}

const JSFunctionSpec Logger::methods[] = {JS_FN("log", log, 1, JSPROP_ENUMERATE), JS_FS_END};

const JSPropertySpec Logger::properties[] = {JS_PS_END};

JSObject *Logger::create(JSContext *cx, const char *name) {
  JS::RootedObject logger(cx, JS_NewObjectWithGivenProto(cx, &class_, proto_obj));
  if (!logger)
    return nullptr;

  auto handle = LogEndpointHandle{INVALID_HANDLE};

  if (!HANDLE_RESULT(cx, xqd_log_endpoint_get(name, strlen(name), &handle)))
    return nullptr;

  JS::SetReservedSlot(logger, Slots::Endpoint, JS::Int32Value(handle.handle));

  return logger;
}

bool Logger::constructor(JSContext *cx, unsigned argc, JS::Value *vp) {
  REQUEST_HANDLER_ONLY("The Logger builtin");
  CTOR_HEADER("Logger", 1);

  size_t name_len;
  JS::UniqueChars name = encode(cx, args[0], &name_len);
  JS::RootedObject logger(cx, JS_NewObjectForConstructor(cx, &class_, args));
  auto handle = LogEndpointHandle{INVALID_HANDLE};

  if (!HANDLE_RESULT(cx, xqd_log_endpoint_get(name.get(), name_len, &handle))) {
    return false;
  }

  JS::SetReservedSlot(logger, Slots::Endpoint, JS::Int32Value(handle.handle));
  args.rval().setObject(*logger);
  return true;
}

bool Logger::init_class(JSContext *cx, JS::HandleObject global) {
  return BuiltinImpl<Logger>::init_class_impl(cx, global);
}

} // namespace builtins
