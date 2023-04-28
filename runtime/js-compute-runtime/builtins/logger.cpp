#include "logger.h"
#include "host_interface/host_call.h"

namespace builtins {

bool Logger::log(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(1)

  auto endpoint =
      (fastly_log_endpoint_handle_t)JS::GetReservedSlot(self, Logger::Slots::Endpoint).toInt32();

  size_t msg_len;
  JS::UniqueChars msg = encode(cx, args.get(0), &msg_len);
  if (!msg)
    return false;

  fastly_error_t err;
  fastly_world_string_t msg_str = {msg.get(), msg_len};
  if (!fastly_log_write(endpoint, &msg_str, &err)) {
    HANDLE_ERROR(cx, err);
    return false;
  }

  args.rval().setUndefined();
  return true;
}

const JSFunctionSpec Logger::static_methods[] = {
    JS_FS_END,
};

const JSPropertySpec Logger::static_properties[] = {
    JS_PS_END,
};

const JSFunctionSpec Logger::methods[] = {JS_FN("log", log, 1, JSPROP_ENUMERATE), JS_FS_END};

const JSPropertySpec Logger::properties[] = {JS_PS_END};

JSObject *Logger::create(JSContext *cx, const char *name) {
  JS::RootedObject logger(cx, JS_NewObjectWithGivenProto(cx, &class_, proto_obj));
  if (!logger)
    return nullptr;

  fastly_log_endpoint_handle_t handle;
  fastly_world_string_t name_str = {const_cast<char *>(name), strlen(name)};
  fastly_error_t err;
  if (!fastly_log_endpoint_get(&name_str, &handle, &err)) {
    HANDLE_ERROR(cx, err);
    return nullptr;
  }

  JS::SetReservedSlot(logger, Slots::Endpoint, JS::Int32Value(handle));

  return logger;
}

bool Logger::constructor(JSContext *cx, unsigned argc, JS::Value *vp) {
  REQUEST_HANDLER_ONLY("The Logger builtin");
  CTOR_HEADER("Logger", 1);

  fastly_world_string_t name_str;
  JS::UniqueChars name = encode(cx, args[0], &name_str.len);
  name_str.ptr = name.get();
  JS::RootedObject logger(cx, JS_NewObjectForConstructor(cx, &class_, args));
  fastly_log_endpoint_handle_t handle = INVALID_HANDLE;
  fastly_error_t err;

  if (!fastly_log_endpoint_get(&name_str, &handle, &err)) {
    HANDLE_ERROR(cx, err);
    return false;
  }

  JS::SetReservedSlot(logger, Slots::Endpoint, JS::Int32Value(handle));
  args.rval().setObject(*logger);
  return true;
}

bool Logger::init_class(JSContext *cx, JS::HandleObject global) {
  return BuiltinImpl<Logger>::init_class_impl(cx, global);
}

} // namespace builtins
