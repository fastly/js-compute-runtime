#include "logger.h"
#include "host_interface/host_api.h"

namespace builtins {

bool Logger::log(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(1)

  LogEndpoint endpoint(JS::GetReservedSlot(self, Logger::Slots::Endpoint).toInt32());

  size_t msg_len;
  JS::UniqueChars msg = encode(cx, args.get(0), &msg_len);
  if (!msg) {
    return false;
  }

  auto res = endpoint.write(std::string_view{msg.get(), msg_len});
  if (auto *err = res.to_err()) {
    HANDLE_ERROR(cx, *err);
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
  if (!logger) {
    return nullptr;
  }

  auto res = LogEndpoint::get(std::string_view{name, strlen(name)});
  if (auto *err = res.to_err()) {
    HANDLE_ERROR(cx, *err);
    return nullptr;
  }

  JS::SetReservedSlot(logger, Slots::Endpoint, JS::Int32Value(res.unwrap().handle));

  return logger;
}

bool Logger::constructor(JSContext *cx, unsigned argc, JS::Value *vp) {
  REQUEST_HANDLER_ONLY("The Logger builtin");
  CTOR_HEADER("Logger", 1);

  size_t name_len;
  JS::UniqueChars name = encode(cx, args[0], &name_len);

  auto handle_res = LogEndpoint::get(std::string_view{name.get(), name_len});
  if (auto *err = handle_res.to_err()) {
    HANDLE_ERROR(cx, *err);
    return false;
  }

  JS::RootedObject logger(cx, JS_NewObjectForConstructor(cx, &class_, args));
  JS::SetReservedSlot(logger, Slots::Endpoint, JS::Int32Value(handle_res.unwrap().handle));
  args.rval().setObject(*logger);
  return true;
}

bool Logger::init_class(JSContext *cx, JS::HandleObject global) {
  return BuiltinImpl<Logger>::init_class_impl(cx, global);
}

} // namespace builtins
