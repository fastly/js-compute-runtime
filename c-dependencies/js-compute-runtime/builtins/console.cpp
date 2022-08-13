#include "console.h"

namespace builtins {

template <const char *prefix, uint8_t prefix_len>
static bool console_out(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS::CallArgs args = CallArgsFromVp(argc, vp);
  std::string fullLogLine = "";
  auto length = args.length();
  for (int i = 0; i < length; i++) {
    std::string message = "";
    size_t message_len;
    JS::HandleValue arg = args.get(i);
    if (arg.isSymbol()) {
      JS::RootedSymbol sym(cx, arg.toSymbol());
      // auto d = arg.toSymbol();
      JS::RootedString desc(cx, JS::GetSymbolDescription(sym));
      auto msg = encode(cx, desc, &message_len);
      if (!msg) {
        return false;
      }
      message += "Symbol(";
      message += msg.get();
      message += ")";
    } else {
      auto msg = encode(cx, arg, &message_len);
      if (!msg) {
        return false;
      }
      message += msg.get();
    }
    if (fullLogLine.length()) {
      fullLogLine += " ";
      fullLogLine += message;
    } else {
      fullLogLine += message;
    }
  }

  printf("%s: %s\n", prefix, fullLogLine.c_str());
  fflush(stdout);

  args.rval().setUndefined();
  return true;
}

static constexpr char PREFIX_LOG[] = "Log";
static constexpr char PREFIX_DEBUG[] = "Debug";
static constexpr char PREFIX_INFO[] = "Info";
static constexpr char PREFIX_WARN[] = "Warn";
static constexpr char PREFIX_ERROR[] = "Error";

const JSFunctionSpec Console::methods[] = {
    JS_FN("log", (console_out<PREFIX_LOG, 3>), 1, JSPROP_ENUMERATE),
    JS_FN("debug", (console_out<PREFIX_DEBUG, 5>), 1, JSPROP_ENUMERATE),
    JS_FN("info", (console_out<PREFIX_INFO, 4>), 1, JSPROP_ENUMERATE),
    JS_FN("warn", (console_out<PREFIX_WARN, 4>), 1, JSPROP_ENUMERATE),
    JS_FN("error", (console_out<PREFIX_ERROR, 5>), 1, JSPROP_ENUMERATE),
    JS_FS_END};

const JSPropertySpec Console::properties[] = {JS_PS_END};

bool Console::create(JSContext *cx, JS::HandleObject global) {
  JS::RootedObject console(cx, JS_NewPlainObject(cx));
  if (!console)
    return false;
  if (!JS_DefineProperty(cx, global, "console", console, JSPROP_ENUMERATE))
    return false;
  return JS_DefineFunctions(cx, console, methods);
}
} // namespace builtins
