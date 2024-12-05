#include "logger.h"
#include "../../../StarlingMonkey/runtime/encode.h"
#include "../host-api/host_api_fastly.h"

namespace builtins::web::console {

class Console : public BuiltinNoConstructor<Console> {
private:
public:
  static constexpr const char *class_name = "Console";
  enum LogType {
    Log,
    Info,
    Debug,
    Warn,
    Error,
  };
  enum Slots { Count };
  static const JSFunctionSpec methods[];
  static const JSPropertySpec properties[];
};

bool write_stderr = false;
bool write_prefix = false;

void builtin_impl_console_log(Console::LogType log_ty, const char *msg) {
  FILE *output = stdout;
  if (write_stderr) {
    if (log_ty == Console::LogType::Warn || log_ty == Console::LogType::Error) {
      output = stderr;
    }
  }
  if (write_prefix) {
    const char *prefix = "";
    switch (log_ty) {
    case Console::LogType::Log:
      prefix = "Log";
      break;
    case Console::LogType::Debug:
      prefix = "Debug";
      break;
    case Console::LogType::Info:
      prefix = "Info";
      break;
    case Console::LogType::Warn:
      prefix = "Warn";
      break;
    case Console::LogType::Error:
      prefix = "Error";
      break;
    }
    fprintf(output, "%s: %s\n", prefix, msg);
    fflush(output);
  } else {
    fprintf(output, "%s\n", msg);
    fflush(output);
  }
}

} // namespace builtins::web::console

namespace fastly::logger {

bool Logger::log(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(1)

  JS::RootedValue endpoint_id(cx, JS::GetReservedSlot(self, Slots::Endpoint));

  // If the endpoint has not yet been loaded up, do it now, throwing any endpoint error for the
  // first log.
  if (endpoint_id.isNull()) {
    JS::RootedString endpoint_name(cx, JS::GetReservedSlot(self, Slots::EndpointName).toString());
    auto endpoint_name_str = core::encode(cx, endpoint_name);
    if (!endpoint_name_str) {
      return false;
    }

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
  CTOR_HEADER("Logger", 1);
  auto logger = Logger::create(cx, args[0]);
  args.rval().setObject(*logger);
  return true;
}

bool configure_console(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

  // Check if we have at least one argument and it's an object
  if (args.length() < 1 || !args[0].isObject()) {
    JS_ReportErrorUTF8(cx, "configureConsole requires an options object as its argument");
    return false;
  }

  // Get the options object
  JS::RootedObject options(cx, &args[0].toObject());
  JS::RootedValue val(cx);

  // Handle prefixing option
  if (JS_GetProperty(cx, options, "prefixing", &val)) {
    if (!val.isUndefined()) {
      if (!val.isBoolean()) {
        JS_ReportErrorUTF8(cx, "prefixing option must be a boolean");
        return false;
      }
      builtins::web::console::write_prefix = val.toBoolean();
    }
  } else {
    return false;
  }

  // Handle stderr option
  if (JS_GetProperty(cx, options, "stderr", &val)) {
    if (!val.isUndefined()) {
      if (!val.isBoolean()) {
        JS_ReportErrorUTF8(cx, "stderr option must be a boolean");
        return false;
      }
      builtins::web::console::write_stderr = val.toBoolean();
    }
  } else {
    return false;
  }

  // Set the return value to undefined
  args.rval().setUndefined();
  return true;
}

bool install(api::Engine *engine) {
  if (!Logger::init_class_impl(engine->cx(), engine->global())) {
    return false;
  }

  RootedObject logger_ns_obj(engine->cx(), JS_NewObject(engine->cx(), nullptr));
  RootedValue logger_ns_val(engine->cx(), JS::ObjectValue(*logger_ns_obj));
  RootedObject logger_obj(engine->cx(), JS_GetConstructor(engine->cx(), Logger::proto_obj));
  RootedValue logger_val(engine->cx(), ObjectValue(*logger_obj));
  if (!JS_SetProperty(engine->cx(), logger_ns_obj, "Logger", logger_val)) {
    return false;
  }
  auto configure_console_fn =
      JS_NewFunction(engine->cx(), &configure_console, 1, 0, "configureConsole");
  RootedObject configure_console_obj(engine->cx(), JS_GetFunctionObject(configure_console_fn));
  RootedValue configure_console_val(engine->cx(), ObjectValue(*configure_console_obj));
  if (!JS_SetProperty(engine->cx(), logger_ns_obj, "configureConsole", configure_console_val)) {
    return false;
  }
  if (!JS_SetProperty(engine->cx(), logger_obj, "configureConsole", configure_console_val)) {
    return false;
  }
  if (!engine->define_builtin_module("fastly:logger", logger_ns_val)) {
    return false;
  }

  return true;
}

} // namespace fastly::logger
