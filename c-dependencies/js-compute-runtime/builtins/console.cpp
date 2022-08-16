#include "js/BigInt.h"
#include "js/RegExp.h"

#include "console.h"

JS::Result<std::string> PromiseToSource(JSContext *cx, JS::HandleObject obj) {
  size_t message_len;
  std::string message = "Promise { ";
  JS::PromiseState state = JS::GetPromiseState(obj);
  switch (state) {
  case JS::PromiseState::Pending: {
    message += "<pending> }";
    break;
  }
  case JS::PromiseState::Fulfilled: {
    JS::RootedValue value(cx, JS::GetPromiseResult(obj));
    JS::RootedString value_source(cx, JS_ValueToSource(cx, value));
    auto msg = encode(cx, value_source, &message_len);
    if (!msg) {
      return JS::Result<std::string>(JS::Error());
    }
    message += msg.get();
    message += " }";
    break;
  }
  case JS::PromiseState::Rejected: {
    message += "<rejected> ";
    JS::RootedValue value(cx, JS::GetPromiseResult(obj));
    JS::RootedString value_source(cx, JS_ValueToSource(cx, value));
    auto msg = encode(cx, value_source, &message_len);
    if (!msg) {
      return JS::Result<std::string>(JS::Error());
    }
    message += msg.get();
    message += " }";
    break;
  }
  }
  return message;
}
JS::Result<std::string> MapToSource(JSContext *cx, JS::HandleObject obj) {
  size_t message_len;
  std::string message = "Map(";
  uint32_t size = JS::MapSize(cx, obj);
  message += std::to_string(size);
  message += ") { ";
  JS::Rooted<JS::Value> iterable(cx);
  if (!JS::MapEntries(cx, obj, &iterable)) {
    return JS::Result<std::string>(JS::Error());
  }
  JS::ForOfIterator it(cx);
  if (!it.init(iterable)) {
    return JS::Result<std::string>(JS::Error());
  }

  JS::RootedObject entry(cx);
  JS::RootedValue entry_val(cx);
  JS::RootedValue name_val(cx);
  JS::RootedValue value_val(cx);
  bool firstValue = true;
  while (true) {
    bool done;
    if (!it.next(&entry_val, &done)) {
      return JS::Result<std::string>(JS::Error());
    }

    if (done) {
      break;
    }
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
      return JS::Result<std::string>(JS::Error());
    }
    message += msg.get();
    message += " => ";
    JS::RootedString value_source(cx, JS_ValueToSource(cx, value_val));
    msg = encode(cx, value_source, &message_len);
    if (!msg) {
      return JS::Result<std::string>(JS::Error());
    }
    message += msg.get();
  }
  message += " }";
  return message;
}
JS::Result<std::string> SetToSource(JSContext *cx, JS::HandleObject obj) {
  size_t message_len;
  std::string message = "Set(";
  uint32_t size = JS::SetSize(cx, obj);
  message += std::to_string(size);
  message += ") { ";
  JS::Rooted<JS::Value> iterable(cx);
  if (!JS::SetValues(cx, obj, &iterable)) {
    return JS::Result<std::string>(JS::Error());
  }
  JS::ForOfIterator it(cx);
  if (!it.init(iterable)) {
    return JS::Result<std::string>(JS::Error());
  }

  JS::RootedValue entry_val(cx);
  bool firstValue = true;
  while (true) {
    bool done;
    if (!it.next(&entry_val, &done)) {
      return JS::Result<std::string>(JS::Error());
    }

    if (done) {
      break;
    }

    JS::RootedString source(cx, JS_ValueToSource(cx, entry_val));
    auto msg = encode(cx, source, &message_len);
    if (!msg) {
      return JS::Result<std::string>(JS::Error());
    }
    if (firstValue) {
      firstValue = false;
    } else {
      message += ", ";
    }
    message += msg.get();
  }
  message += " }";
  return message;
}

JS::Result<std::string> ToSource(JSContext *cx, JS::HandleValue val) {
  auto type = val.type();
  switch (type) {
    case JS::ValueType::Undefined: {
      std::string sourceString = "undefined";
      return sourceString;
    }
    case JS::ValueType::Null: {
      std::string sourceString = "null";
      return sourceString;
    }
    case JS::ValueType::Object: {
      JS::RootedObject obj(cx, &val.toObject());
      js::ESClass cls;
      if (!JS::GetBuiltinClass(cx, obj, &cls)) {
        return JS::Result<std::string>(JS::Error());
      }

      if (cls == js::ESClass::Set) {
        return SetToSource(cx, obj);
      } else if (cls == js::ESClass::Map) {
        return MapToSource(cx, obj);
      } else if (cls == js::ESClass::Promise) {
        return PromiseToSource(cx, obj);
      } else if (JS::IsWeakMapObject(obj)) {
        std::string sourceString = "WeakMap { <items unknown> }";
        return sourceString;
      } else {
        JS::RootedString source(cx, JS_ValueToSource(cx, val));
        size_t message_len;
        auto msg = encode(cx, source, &message_len);
        if (!msg) {
          return JS::Result<std::string>(JS::Error());
        }
        std::string sourceString(msg.get(), message_len);
        return sourceString;
      }
    }
    case JS::ValueType::String: {
      size_t message_len;
      auto msg = encode(cx, val, &message_len);
      if (!msg) {
        return JS::Result<std::string>(JS::Error());
      }
      std::string sourceString(msg.get(), message_len);
      return sourceString;
    }
    default: {
      JS::RootedString source(cx, JS_ValueToSource(cx, val));
      size_t message_len;
      auto msg = encode(cx, source, &message_len);
      if (!msg) {
        return JS::Result<std::string>(JS::Error());
      }
      std::string sourceString(msg.get(), message_len);
      return sourceString;
    }
  }
}

namespace builtins {


template <const char *prefix, uint8_t prefix_len>
static bool console_out(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS::CallArgs args = CallArgsFromVp(argc, vp);
  std::string fullLogLine = "";
  auto length = args.length();
  for (int i = 0; i < length; i++) {
    JS::HandleValue arg = args.get(i);
    auto source = ToSource(cx, arg);
    if (source.isErr()) {return false;}
    std::string message = source.unwrap();
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
