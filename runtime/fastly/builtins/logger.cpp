#include "logger.h"
#include "../../../StarlingMonkey/runtime/encode.h"
#include "../host-api/host_api_fastly.h"

using builtins::BuiltinImpl;

namespace fastly::logger {

bool Logger::log(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(1)

  host_api::LogEndpoint endpoint(JS::GetReservedSlot(self, Logger::Slots::Endpoint).toInt32());

  auto msg = core::encode(cx, args.get(0));
  if (!msg) {
    return false;
  }

  auto res = endpoint.write(msg);
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

  auto res = host_api::LogEndpoint::get(std::string_view{name, strlen(name)});
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

  auto name = core::encode(cx, args[0]);
  auto handle_res = host_api::LogEndpoint::get(name);
  if (auto *err = handle_res.to_err()) {
    HANDLE_ERROR(cx, *err);
    return false;
  }

  JS::RootedObject logger(cx, JS_NewObjectForConstructor(cx, &class_, args));
  JS::SetReservedSlot(logger, Slots::Endpoint, JS::Int32Value(handle_res.unwrap().handle));
  args.rval().setObject(*logger);
  return true;
}

bool install(api::Engine *engine) {
  if (!BuiltinImpl<Logger>::init_class_impl(engine->cx(), engine->global())) {
    return false;
  }

  RootedObject logger_ns_obj(engine->cx(), JS_NewObject(engine->cx(), nullptr));
  RootedValue logger_ns_val(engine->cx(), JS::ObjectValue(*logger_ns_obj));
  RootedObject logger_obj(engine->cx(), JS_GetConstructor(engine->cx(), Logger::proto_obj));
  RootedValue logger_val(engine->cx(), ObjectValue(*logger_obj));
  if (!JS_SetProperty(engine->cx(), logger_ns_obj, "Logger", logger_val)) {
    return false;
  }
  if (!engine->define_builtin_module("fastly:logger", logger_ns_val)) {
    return false;
  }

  return true;
}

} // namespace fastly::logger
