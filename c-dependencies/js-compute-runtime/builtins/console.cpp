#include "js/BigInt.h"
#include "js/RegExp.h"

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
    auto type = arg.type();
    switch (type) {
    case JS::ValueType::Undefined: {
      message += "undefined";
      break;
    }
    case JS::ValueType::Object: {
      JS::RootedObject obj(cx, &arg.toObject());
      js::ESClass cls;
      if (!JS::GetBuiltinClass(cx, obj, &cls)) {
        return false;
      }

      if (cls == js::ESClass::Set) {
        message += "Set(";
        uint32_t size = JS::SetSize(cx, obj);
        message += std::to_string(size);
        message += ") { ";
        JS::Rooted<JS::Value> iterable(cx);
        if (!JS::SetValues(cx, obj, &iterable)) {
          return false;
        }
        JS::ForOfIterator it(cx);
        if (!it.init(iterable)) {
          return false;
        }

        JS::RootedObject entry(cx);
        JS::RootedValue entry_val(cx);
        JS::RootedValue name_val(cx);
        JS::RootedValue value_val(cx);
        bool firstValue = true;
        while (true) {
          bool done;
          if (!it.next(&entry_val, &done)) {return false;}

          if (done){break;}

          JS::RootedString source(cx, JS_ValueToSource(cx, entry_val));
          auto msg = encode(cx, source, &message_len);
          if (!msg) {
            return false;
          }
          if (firstValue) {
            firstValue = false;
          } else {
            message += ", ";
          }
          message += msg.get();
        }
        message += " }";
        break;
      } else if (cls == js::ESClass::Map) {
        message += "Map(";
        uint32_t size = JS::MapSize(cx, obj);
        message += std::to_string(size);
        message += ") { ";
        JS::Rooted<JS::Value> iterable(cx);
        if (!JS::MapEntries(cx, obj, &iterable)) {
          return false;
        }
        JS::ForOfIterator it(cx);
        if (!it.init(iterable)) {
          return false;
        }

        JS::RootedObject entry(cx);
        JS::RootedValue entry_val(cx);
        JS::RootedValue name_val(cx);
        JS::RootedValue value_val(cx);
        bool firstValue = true;
        while (true) {
          bool done;
          if (!it.next(&entry_val, &done)) {return false;}

          if (done){break;}
          if (firstValue) {
            firstValue = false;
          } else {
            message += ", ";
          }

          entry = &entry_val.toObject();
          JS_GetElement(cx, entry, 0, &name_val);
          JS_GetElement(cx, entry, 1, &value_val);
          JS::RootedString name_source(cx, JS_ValueToSource(cx, name_val));
          auto msg = encode(cx, name_source, &message_len);
          if (!msg) {
            return false;
          }
          message += msg.get();
          message += " => ";
          JS::RootedString value_source(cx, JS_ValueToSource(cx, value_val));
          msg = encode(cx, value_source, &message_len);
          if (!msg) {
            return false;
          }
          message += msg.get();
        }
        message += " }";
        break;
      } else {
        JS::RootedString source(cx, JS_ValueToSource(cx, arg));
        auto msg = encode(cx, source, &message_len);
        if (!msg) {
          return false;
        }
        message += msg.get();
        break;
      }
    }
    case JS::ValueType::String: {
      auto msg = encode(cx, arg, &message_len);
      if (!msg) {
        return false;
      }
      message += msg.get();
      break;
    }
    default: {
      JS::RootedString source(cx, JS_ValueToSource(cx, arg));
      auto msg = encode(cx, source, &message_len);
      if (!msg) {
        return false;
      }
      message += msg.get();
      break;
    }
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
