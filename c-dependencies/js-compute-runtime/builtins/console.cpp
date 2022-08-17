#include "console.h"

JS::Result<std::string> ToSource(JSContext *cx, JS::HandleValue val,
                                 JS::MutableHandleObjectVector visitedObjects);
JS::Result<std::string> PromiseToSource(JSContext *cx, JS::HandleObject obj,
                                        JS::MutableHandleObjectVector visitedObjects) {
  std::string message = "Promise { ";
  JS::PromiseState state = JS::GetPromiseState(obj);
  switch (state) {
  case JS::PromiseState::Pending: {
    message += "<pending> }";
    break;
  }
  case JS::PromiseState::Fulfilled: {
    JS::RootedValue value(cx, JS::GetPromiseResult(obj));
    auto value_source = ToSource(cx, value, visitedObjects);
    if (value_source.isErr()) {
      return JS::Result<std::string>(JS::Error());
    }
    message += value_source.unwrap();
    message += " }";
    break;
  }
  case JS::PromiseState::Rejected: {
    message += "<rejected> ";
    JS::RootedValue value(cx, JS::GetPromiseResult(obj));
    auto value_source = ToSource(cx, value, visitedObjects);
    if (value_source.isErr()) {
      return JS::Result<std::string>(JS::Error());
    }
    message += value_source.unwrap();
    message += " }";
    break;
  }
  }
  return message;
}
JS::Result<std::string> MapToSource(JSContext *cx, JS::HandleObject obj,
                                    JS::MutableHandleObjectVector visitedObjects) {
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
    auto name_source = ToSource(cx, name_val, visitedObjects);
    if (name_source.isErr()) {
      return JS::Result<std::string>(JS::Error());
    }
    message += name_source.unwrap();
    message += " => ";
    auto value_source = ToSource(cx, value_val, visitedObjects);
    if (value_source.isErr()) {
      return JS::Result<std::string>(JS::Error());
    }
    message += value_source.unwrap();
  }
  message += " }";
  return message;
}
JS::Result<std::string> SetToSource(JSContext *cx, JS::HandleObject obj,
                                    JS::MutableHandleObjectVector visitedObjects) {
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
    auto source = ToSource(cx, entry_val, visitedObjects);
    if (source.isErr()) {
      return JS::Result<std::string>(JS::Error());
    }
    if (firstValue) {
      firstValue = false;
    } else {
      message += ", ";
    }
    message += source.unwrap();
  }
  message += " }";
  return message;
}

JS::Result<std::string> ObjectToSource(JSContext *cx, JS::HandleObject obj,
                                       JS::MutableHandleObjectVector visitedObjects) {
  std::string output = "{";
  JS::RootedIdVector ids(cx);
  if (!js::GetPropertyKeys(cx, obj, 0, &ids)) {
    return JS::Result<std::string>(JS::Error());
  }

  JS::RootedValue value(cx);
  size_t length = ids.length();
  for (size_t i = 0; i < length; ++i) {
    const auto &id = ids[i];
    if (!JS_GetPropertyById(cx, obj, id, &value)) {
      return JS::Result<std::string>(JS::Error());
    }

    if (!value.isObject() || !JS_ObjectIsFunction(&value.toObject())) {
      if (id.isSymbol()) {
        JS::RootedValue v(cx, SymbolValue(id.toSymbol()));
        auto idSource = ToSource(cx, v, visitedObjects);
        if (idSource.isErr()) {
          return JS::Result<std::string>(JS::Error());
        }
        output += idSource.unwrap();
      } else {
        JS::RootedValue idValue(cx, js::IdToValue(id));
        JS::Result<std::string> idSource = ToSource(cx, idValue, visitedObjects);
        if (idSource.isErr()) {
          return JS::Result<std::string>(JS::Error());
        }
        output += idSource.unwrap();
      }
      output += ": ";
      JS::Result<std::string> valSource = ToSource(cx, value, visitedObjects);
      if (valSource.isErr()) {
        return JS::Result<std::string>(JS::Error());
      }
      output += valSource.unwrap();
      output += ", ";
    }
  }
  // Remove the last addition of ", " from the output
  if (length) {
    output.pop_back();
    output.pop_back();
  }

  output += "}";

  return output;
}

JS::Result<std::string> ToSource(JSContext *cx, JS::HandleValue val,
                                 JS::MutableHandleObjectVector visitedObjects) {

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

    for (const auto &curObject : visitedObjects) {
      if (obj.get() == curObject) {
        std::string circular = "<Circular>";
        return circular;
      }
    }

    if (!visitedObjects.emplaceBack(obj)) {
      return JS::Result<std::string>(JS::Error());
    }

    if (JS_ObjectIsFunction(obj)) {
      auto msg = JS::InformalValueTypeName(val);
      std::string sourceString(msg);
      return sourceString;
    }
    js::ESClass cls;
    if (!JS::GetBuiltinClass(cx, obj, &cls)) {
      return JS::Result<std::string>(JS::Error());
    }

    if (cls == js::ESClass::Array || cls == js::ESClass::Date || cls == js::ESClass::Error ||
               cls == js::ESClass::RegExp) {
      JS::RootedString source(cx, JS_ValueToSource(cx, val));
      size_t message_len;
      auto msg = encode(cx, source, &message_len);
      if (!msg) {
        return JS::Result<std::string>(JS::Error());
      }
      std::string sourceString(msg.get(), message_len);
      return sourceString;
    } else if (cls == js::ESClass::Set) {
      return SetToSource(cx, obj, visitedObjects);
    } else if (cls == js::ESClass::Map) {
      return MapToSource(cx, obj, visitedObjects);
    } else if (cls == js::ESClass::Promise) {
      return PromiseToSource(cx, obj, visitedObjects);
    } else {
      if (JS::IsWeakMapObject(obj)) {
        std::string sourceString = "WeakMap { <items unknown> }";
        return sourceString;
      }
      auto cls = JS::GetClass(obj);
      std::string className(cls->name);
      if (className == "WeakSet") {
        std::string sourceString = "WeakSet { <items unknown> }";
        return sourceString;
      }
      return ObjectToSource(cx, obj, visitedObjects);
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
  JS::RootedObjectVector visitedObjects(cx);
  for (int i = 0; i < length; i++) {
    JS::HandleValue arg = args.get(i);
    auto source = ToSource(cx, arg, &visitedObjects);
    if (source.isErr()) {
      return false;
    }
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
