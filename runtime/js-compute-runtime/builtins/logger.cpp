#include "logger.h"
#include "core/encode.h"
#include "host_interface/host_api.h"

namespace builtins {

bool Logger::log(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(1)

  JS::RootedString endpoint_name(cx, JS::GetReservedSlot(self, Slots::EndpointName).toString());
  auto endpoint_name_str = core::encode(cx, endpoint_name);
  if (!endpoint_name_str) {
    return false;
  }

  JS::RootedValue endpoint_id(cx, JS::GetReservedSlot(self, Slots::Endpoint));

  // If the endpoint has not yet been loaded up, do it now, throwing any endpoint error for the
  // first log.
  if (endpoint_id.isNull()) {
    auto res = host_api::LogEndpoint::get(
        std::string_view{endpoint_name_str.ptr.get(), endpoint_name_str.len});
    if (auto *err = res.to_err()) {
      HANDLE_ERROR(cx, *err);
      return false;
    }

    endpoint_id.set(JS::Int32Value(res.unwrap().handle));
    JS::SetReservedSlot(self, Slots::Endpoint, endpoint_id);

    MOZ_ASSERT(endpoint_id.isInt32());
  }

  host_api::LogEndpoint endpoint(endpoint_id.toInt32());

  auto msg = core::encode(cx, args.get(0));
  if (!msg) {
    return false;
  }

  fprintf(stdout, "Log[%s]: %s\n", endpoint_name_str.ptr.get(), msg.ptr.get());
  fflush(stdout);

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

JSObject *Logger::create(JSContext *cx, JS::HandleValue endpoint_name) {
  JS::RootedObject logger(cx, JS_NewObjectWithGivenProto(cx, &class_, proto_obj));
  if (!logger) {
    return nullptr;
  }
  JS::SetReservedSlot(logger, Slots::Endpoint, JS::NullValue());
  JS::SetReservedSlot(logger, Slots::EndpointName, endpoint_name);
  return logger;
}

bool Logger::constructor(JSContext *cx, unsigned argc, JS::Value *vp) {
  REQUEST_HANDLER_ONLY("The Logger builtin");
  CTOR_HEADER("Logger", 1);
  auto logger = Logger::create(cx, args[0]);
  args.rval().setObject(*logger);
  return true;
}

bool Logger::init_class(JSContext *cx, JS::HandleObject global) {
  return BuiltinImpl<Logger>::init_class_impl(cx, global);
}

} // namespace builtins
