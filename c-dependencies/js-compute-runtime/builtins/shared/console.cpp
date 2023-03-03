#include "console.h"
#include <js/Array.h>
#include <js/PropertyAndElement.h>
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-offsetof"
#pragma clang diagnostic ignored "-Wdeprecated-enum-enum-conversion"
#include <js/experimental/TypedData.h>
#pragma clang diagnostic pop

JS::Result<mozilla::Ok> ToSource(JSContext *cx, std::string &sourceOut, JS::HandleValue val,
                                 JS::MutableHandleObjectVector visitedObjects);

/**
 * Turn a handle of a Promise into a string which represents the promise.
 * - If the promise is pending this will return "Promise { <pending> }"
 * - If the promise is rejected this will return "Promise { <rejected> (rejected-value)}"
 *  where rejected-value would be the ToSource representation of the rejected value.
 * - If the promise is resolved this will return "Promise { resolved-value}"
 *  where resolved-value would be the ToSource representation of the resolved value.
 */
JS::Result<mozilla::Ok> PromiseToSource(JSContext *cx, std::string &sourceOut, JS::HandleObject obj,
                                        JS::MutableHandleObjectVector visitedObjects) {
  sourceOut += "Promise { ";
  JS::PromiseState state = JS::GetPromiseState(obj);
  switch (state) {
  case JS::PromiseState::Pending: {
    sourceOut += "<pending> }";
    break;
  }
  case JS::PromiseState::Fulfilled: {
    JS::RootedValue value(cx, JS::GetPromiseResult(obj));
    MOZ_TRY(ToSource(cx, sourceOut, value, visitedObjects));
    sourceOut += " }";
    break;
  }
  case JS::PromiseState::Rejected: {
    sourceOut += "<rejected> ";
    JS::RootedValue value(cx, JS::GetPromiseResult(obj));
    MOZ_TRY(ToSource(cx, sourceOut, value, visitedObjects));
    sourceOut += " }";
    break;
  }
  }
  return mozilla::Ok();
}

/**
 * Turn a handle of a Map into a string which represents the map.
 * Each key and value within the map will be converted into it's ToSource representation.
 */
JS::Result<mozilla::Ok> MapToSource(JSContext *cx, std::string &sourceOut, JS::HandleObject obj,
                                    JS::MutableHandleObjectVector visitedObjects) {
  sourceOut += "Map(";
  uint32_t size = JS::MapSize(cx, obj);
  sourceOut += std::to_string(size);
  sourceOut += ") { ";
  JS::Rooted<JS::Value> iterable(cx);
  if (!JS::MapEntries(cx, obj, &iterable)) {
    return JS::Result<mozilla::Ok>(JS::Error());
  }
  JS::ForOfIterator it(cx);
  if (!it.init(iterable)) {
    return JS::Result<mozilla::Ok>(JS::Error());
  }

  JS::RootedObject entry(cx);
  JS::RootedValue entry_val(cx);
  JS::RootedValue name_val(cx);
  JS::RootedValue value_val(cx);
  bool firstValue = true;
  while (true) {
    bool done;
    if (!it.next(&entry_val, &done)) {
      return JS::Result<mozilla::Ok>(JS::Error());
    }

    if (done) {
      break;
    }
    if (firstValue) {
      firstValue = false;
    } else {
      sourceOut += ", ";
    }

    entry = &entry_val.toObject();
    JS_GetElement(cx, entry, 0, &name_val);
    JS_GetElement(cx, entry, 1, &value_val);
    MOZ_TRY(ToSource(cx, sourceOut, name_val, visitedObjects));
    sourceOut += " => ";
    MOZ_TRY(ToSource(cx, sourceOut, value_val, visitedObjects));
  }
  sourceOut += " }";
  return mozilla::Ok();
}

/**
 * Turn a handle of a Set into a string which represents the set.
 * Each value within the set will be converted into it's ToSource representation.
 */
JS::Result<mozilla::Ok> SetToSource(JSContext *cx, std::string &sourceOut, JS::HandleObject obj,
                                    JS::MutableHandleObjectVector visitedObjects) {
  sourceOut += "Set(";
  uint32_t size = JS::SetSize(cx, obj);
  sourceOut += std::to_string(size);
  sourceOut += ") { ";
  JS::Rooted<JS::Value> iterable(cx);
  if (!JS::SetValues(cx, obj, &iterable)) {
    return JS::Result<mozilla::Ok>(JS::Error());
  }
  JS::ForOfIterator it(cx);
  if (!it.init(iterable)) {
    return JS::Result<mozilla::Ok>(JS::Error());
  }

  JS::RootedValue entry_val(cx);
  bool firstValue = true;
  while (true) {
    bool done;
    if (!it.next(&entry_val, &done)) {
      return JS::Result<mozilla::Ok>(JS::Error());
    }

    if (done) {
      break;
    }
    if (firstValue) {
      firstValue = false;
    } else {
      sourceOut += ", ";
    }
    MOZ_TRY(ToSource(cx, sourceOut, entry_val, visitedObjects));
  }
  sourceOut += " }";
  return mozilla::Ok();
}

JS::Result<mozilla::Ok> ArrayToSource(JSContext *cx, std::string &sourceOut, JS::HandleObject obj,
                                      JS::MutableHandleObjectVector visitedObjects) {
  sourceOut += "[";
  uint32_t len;
  if (!JS::GetArrayLength(cx, obj, &len)) {
    return JS::Result<mozilla::Ok>(JS::Error());
  }

  for (int i = 0; i < len; i++) {
    JS::RootedValue entry_val(cx);
    JS_GetElement(cx, obj, i, &entry_val);
    if (i > 0) {
      sourceOut += ", ";
    }
    MOZ_TRY(ToSource(cx, sourceOut, entry_val, visitedObjects));
  }
  sourceOut += "]";
  return mozilla::Ok();
}

/*
 * Logs all enumerable properties, except non-own function and symbol properties
 * Includes handling for getters and setters
 */
JS::Result<mozilla::Ok> ObjectToSource(JSContext *cx, std::string &sourceOut, JS::HandleObject obj,
                                       JS::MutableHandleObjectVector visitedObjects) {
  JS::RootedIdVector ids(cx);

  if (!js::GetPropertyKeys(cx, obj, 0, &ids)) {
    return JS::Result<mozilla::Ok>(JS::Error());
  }

  JS::RootedValue value(cx);
  size_t length = ids.length();

  sourceOut += "{";
  bool firstValue = true;
  for (size_t i = 0; i < length; ++i) {
    const auto &id = ids[i];

    JS::Rooted<mozilla::Maybe<JS::PropertyDescriptor>> desc(cx);
    JS_GetOwnPropertyDescriptorById(cx, obj, id, &desc);

    bool getter_setter = !desc.isNothing() && (desc->hasGetter() || desc->hasSetter());

    // retrive the value if not a getter or setter
    if (!getter_setter) {
      if (!JS_GetPropertyById(cx, obj, id, &value)) {
        return JS::Result<mozilla::Ok>(JS::Error());
      }
    }

    // Skip logging non-own function or getter and setter keys
    if (getter_setter || (value.isObject() && JS_ObjectIsFunction(&value.toObject()))) {
      bool own_prop;
      if (!JS_HasOwnPropertyById(cx, obj, id, &own_prop)) {
        return JS::Result<mozilla::Ok>(JS::Error());
      }
      if (!own_prop) {
        continue;
      }
    }

    if (firstValue) {
      firstValue = false;
      sourceOut += " ";
    } else {
      sourceOut += ", ";
    }

    // Key
    if (id.isSymbol()) {
      JS::RootedValue v(cx, SymbolValue(id.toSymbol()));
      MOZ_TRY(ToSource(cx, sourceOut, v, visitedObjects));
    } else {
      size_t message_len;
      JS::RootedValue v(cx, js::IdToValue(id));
      auto msg = encode(cx, v, &message_len);
      if (!msg) {
        return JS::Result<mozilla::Ok>(JS::Error());
      }
      sourceOut += std::string(msg.get(), message_len);
    }

    sourceOut += ": ";

    // Getters and Setters
    if (getter_setter) {
      sourceOut += "[Getter]";
    } else {
      MOZ_TRY(ToSource(cx, sourceOut, value, visitedObjects));
    }
  }

  if (!firstValue) {
    sourceOut += " ";
  }
  sourceOut += "}";

  return mozilla::Ok();
}

mozilla::Maybe<std::string> get_class_name(JSContext *cx, JS::HandleObject obj) {
  mozilla::Maybe<std::string> result = {};
  JS::RootedValue constructorVal(cx);
  if (JS_GetProperty(cx, obj, "constructor", &constructorVal) && constructorVal.isObject()) {
    JS::RootedValue name(cx);
    JS::RootedObject constructorObj(cx, &constructorVal.toObject());
    if (JS_GetProperty(cx, constructorObj, "name", &name) && name.isString()) {
      size_t message_len;
      auto msg = encode(cx, name, &message_len);
      if (!msg) {
        return result;
      }
      std::string name_str(msg.get(), message_len);
      result.emplace(name_str);
    }
  }
  return result;
}

/**
 * Turn a handle of any value into a string which represents it.
 */
JS::Result<mozilla::Ok> ToSource(JSContext *cx, std::string &sourceOut, JS::HandleValue val,
                                 JS::MutableHandleObjectVector visitedObjects) {

  auto type = val.type();
  switch (type) {
  case JS::ValueType::Undefined: {
    sourceOut += "undefined";
    return mozilla::Ok();
  }
  case JS::ValueType::Null: {
    sourceOut += "null";
    return mozilla::Ok();
  }
  case JS::ValueType::Object: {
    JS::RootedObject obj(cx, &val.toObject());

    if (JS_ObjectIsFunction(obj)) {
      sourceOut += "[Function";
      std::string source;
      auto id = JS_GetFunctionId(JS_ValueToFunction(cx, val));
      if (id) {
        sourceOut += " ";
        JS::RootedString name(cx, id);
        size_t name_len;
        auto msg = encode(cx, name, &name_len);
        if (!msg) {
          return JS::Result<mozilla::Ok>(JS::Error());
        }
        sourceOut += std::string(msg.get(), name_len);
      }
      sourceOut += "]";
      return mozilla::Ok();
    }

    if (JS_IsTypedArrayObject(obj)) {
      // Show the typed array type
      mozilla::Maybe<std::string> name_str = get_class_name(cx, obj);
      if (!name_str.isNothing()) {
        sourceOut += *name_str;
        sourceOut += " ";
      }
      MOZ_TRY(ArrayToSource(cx, sourceOut, obj, visitedObjects));
      return mozilla::Ok();
    }

    for (const auto &curObject : visitedObjects) {
      if (obj.get() == curObject) {
        sourceOut += "<Circular>";
        return mozilla::Ok();
      }
    }

    if (!visitedObjects.emplaceBack(obj)) {
      return JS::Result<mozilla::Ok>(JS::Error());
    }

    js::ESClass cls;
    if (!JS::GetBuiltinClass(cx, obj, &cls)) {
      return JS::Result<mozilla::Ok>(JS::Error());
    }

    switch (cls) {
    case js::ESClass::Date:
    case js::ESClass::Error:
    case js::ESClass::RegExp: {
      JS::RootedString source(cx, JS_ValueToSource(cx, val));
      size_t message_len;
      auto msg = encode(cx, source, &message_len);
      if (!msg) {
        return JS::Result<mozilla::Ok>(JS::Error());
      }
      sourceOut += std::string(msg.get(), message_len);
      return mozilla::Ok();
    }
    case js::ESClass::Array: {
      MOZ_TRY(ArrayToSource(cx, sourceOut, obj, visitedObjects));
      return mozilla::Ok();
    }
    case js::ESClass::Set: {
      MOZ_TRY(SetToSource(cx, sourceOut, obj, visitedObjects));
      return mozilla::Ok();
    }
    case js::ESClass::Map: {
      MOZ_TRY(MapToSource(cx, sourceOut, obj, visitedObjects));
      return mozilla::Ok();
    }
    case js::ESClass::Promise: {
      MOZ_TRY(PromiseToSource(cx, sourceOut, obj, visitedObjects));
      return mozilla::Ok();
    }
    default: {
      std::string sourceString;
      if (JS::IsWeakMapObject(obj)) {
        sourceOut += "WeakMap { <items unknown> }";
        return mozilla::Ok();
      }
      auto cls = JS::GetClass(obj);
      std::string className(cls->name);
      if (className == "WeakSet") {
        sourceOut += "WeakSet { <items unknown> }";
        return mozilla::Ok();
      }

      // Lookup the class name if a custom class
      mozilla::Maybe<std::string> name_str = get_class_name(cx, obj);
      if (!name_str.isNothing() && *name_str != "Object") {
        sourceOut += *name_str;
        sourceOut += " ";
      }

      MOZ_TRY(ObjectToSource(cx, sourceOut, obj, visitedObjects));
      return mozilla::Ok();
    }
    }
  }
  case JS::ValueType::String: {
    size_t message_len;
    auto msg = encode(cx, val, &message_len);
    if (!msg) {
      return JS::Result<mozilla::Ok>(JS::Error());
    }
    sourceOut += '"';
    sourceOut += std::string(msg.get(), message_len);
    sourceOut += '"';
    return mozilla::Ok();
  }
  default: {
    JS::RootedString source(cx, JS_ValueToSource(cx, val));
    size_t message_len;
    auto msg = encode(cx, source, &message_len);
    if (!msg) {
      return JS::Result<mozilla::Ok>(JS::Error());
    }
    sourceOut += std::string(msg.get(), message_len);
    return mozilla::Ok();
  }
  }
}

namespace builtins {

template <Console::LogType log_ty>
static bool console_out(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS::CallArgs args = CallArgsFromVp(argc, vp);
  std::string fullLogLine = "";
  auto length = args.length();
  JS::RootedObjectVector visitedObjects(cx);
  for (int i = 0; i < length; i++) {
    JS::HandleValue arg = args.get(i);
    std::string source = "";
    auto result = ToSource(cx, source, arg, &visitedObjects);
    if (result.isErr()) {
      return false;
    }
    // strip quotes for direct string logs
    if (source[0] == '"' && source[source.length() - 1] == '"') {
      source = source.substr(1, source.length() - 2);
    }
    if (fullLogLine.length()) {
      fullLogLine += " ";
      fullLogLine += source;
    } else {
      fullLogLine += source;
    }
  }

  builtin_impl_console_log(log_ty, fullLogLine.c_str());

  args.rval().setUndefined();
  return true;
}

const JSFunctionSpec Console::methods[] = {
    JS_FN("log", (console_out<Console::LogType::Log>), 1, JSPROP_ENUMERATE),
    JS_FN("debug", (console_out<Console::LogType::Debug>), 1, JSPROP_ENUMERATE),
    JS_FN("info", (console_out<Console::LogType::Info>), 1, JSPROP_ENUMERATE),
    JS_FN("warn", (console_out<Console::LogType::Warn>), 1, JSPROP_ENUMERATE),
    JS_FN("error", (console_out<Console::LogType::Error>), 1, JSPROP_ENUMERATE),
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
