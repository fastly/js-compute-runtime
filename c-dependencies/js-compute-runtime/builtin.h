#ifndef JS_COMPUTE_RUNTIME_BUILTIN_H
#define JS_COMPUTE_RUNTIME_BUILTIN_H

#include <tuple>

#include "js-compute-builtins.h"

#define DBG(...)                                                                                   \
  printf("%s#%d: ", __func__, __LINE__);                                                           \
  printf(__VA_ARGS__);                                                                             \
  fflush(stdout);

#define MULTI_VALUE_HOSTCALL(op, accum)                                                            \
  uint32_t cursor = 0;                                                                             \
  int64_t ending_cursor = 0;                                                                       \
  size_t nwritten;                                                                                 \
                                                                                                   \
  while (true) {                                                                                   \
    op                                                                                             \
                                                                                                   \
        if (nwritten == 0) {                                                                       \
      break;                                                                                       \
    }                                                                                              \
                                                                                                   \
    accum                                                                                          \
                                                                                                   \
        if (ending_cursor < 0) {                                                                   \
      break;                                                                                       \
    }                                                                                              \
                                                                                                   \
    cursor = (uint32_t)ending_cursor;                                                              \
  }

#define CLASS_BOILERPLATE_CUSTOM_INIT(cls)                                                         \
  constexpr const JSClassOps class_ops = {};                                                       \
  const uint32_t class_flags = 0;                                                                  \
                                                                                                   \
  const JSClass class_ = {#cls, JSCLASS_HAS_RESERVED_SLOTS(Slots::Count) | class_flags,            \
                          &class_ops};                                                             \
  JS::PersistentRooted<JSObject *> proto_obj;                                                      \
                                                                                                   \
  bool is_instance(JSObject *obj) { return !!obj && JS::GetClass(obj) == &class_; }                \
                                                                                                   \
  bool is_instance(JS::Value val) { return val.isObject() && is_instance(&val.toObject()); }       \
                                                                                                   \
  bool check_receiver(JSContext *cx, JS::HandleValue receiver, const char *method_name) {          \
    if (!is_instance(receiver)) {                                                                  \
      JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_INCOMPATIBLE_INSTANCE,         \
                                method_name, class_.name);                                         \
      return false;                                                                                \
    }                                                                                              \
    return true;                                                                                   \
  };                                                                                               \
                                                                                                   \
  bool init_class_impl(JSContext *cx, JS::HandleObject global,                                     \
                       JS::HandleObject parent_proto = nullptr) {                                  \
    proto_obj.init(cx, JS_InitClass(cx, global, &class_, parent_proto, #cls, constructor,          \
                                    ctor_length, properties, methods, nullptr, nullptr));          \
    return proto_obj;                                                                              \
  };

#define CLASS_BOILERPLATE(cls)                                                                     \
  CLASS_BOILERPLATE_CUSTOM_INIT(cls)                                                               \
                                                                                                   \
  bool init_class(JSContext *cx, JS::HandleObject global) { return init_class_impl(cx, global); }

#define CLASS_BOILERPLATE_NO_CTOR(cls)                                                             \
  bool constructor(JSContext *cx, unsigned argc, JS::Value *vp) {                                  \
    JS_ReportErrorUTF8(cx, #cls " can't be instantiated directly");                                \
    return false;                                                                                  \
  }                                                                                                \
                                                                                                   \
  CLASS_BOILERPLATE_CUSTOM_INIT(cls)                                                               \
                                                                                                   \
  bool init_class(JSContext *cx, JS::HandleObject global) {                                        \
    /* Right now, deleting the ctor from the global object after class                             \
       initialization seems to be the best we can do. Not ideal, but works. */                     \
    return init_class_impl(cx, global) && JS_DeleteProperty(cx, global, class_.name);              \
  }

// Define this to make most methods print their name to stderr when invoked.
// #define TRACE_METHOD_CALLS

#ifdef TRACE_METHOD_CALLS
#define TRACE_METHOD(name) DBG("%s\n", name)
#else
#define TRACE_METHOD(name)
#endif

#define METHOD_HEADER_WITH_NAME(required_argc, name)                                               \
  TRACE_METHOD(name)                                                                               \
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);                                                \
  if (!check_receiver(cx, args.thisv(), name))                                                     \
    return false;                                                                                  \
  JS::RootedObject self(cx, &args.thisv().toObject());                                             \
  if (!args.requireAtLeast(cx, name, required_argc))                                               \
    return false;

#define METHOD_HEADER(required_argc) METHOD_HEADER_WITH_NAME(required_argc, __func__)

#define CTOR_HEADER(name, required_argc)                                                           \
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);                                                \
  if (!ThrowIfNotConstructing(cx, args, name)) {                                                   \
    return false;                                                                                  \
  }                                                                                                \
  if (!args.requireAtLeast(cx, name " constructor", required_argc)) {                              \
    return false;                                                                                  \
  }

#define REQUEST_HANDLER_ONLY(name)                                                                 \
  if (isWizening()) {                                                                              \
    JS_ReportErrorUTF8(cx,                                                                         \
                       "%s can only be used during request handling, "                             \
                       "not during initialization",                                                \
                       name);                                                                      \
    return false;                                                                                  \
  }

#define INIT_ONLY(name)                                                                            \
  if (hasWizeningFinished()) {                                                                     \
    JS_ReportErrorUTF8(cx,                                                                         \
                       "%s can only be used during initialization, "                               \
                       "not during request handling",                                              \
                       name);                                                                      \
    return false;                                                                                  \
  }

inline bool ThrowIfNotConstructing(JSContext *cx, const JS::CallArgs &args,
                                   const char *builtinName) {
  if (args.isConstructing()) {
    return true;
  }

  JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_BUILTIN_CTOR_NO_NEW, builtinName);
  return false;
}
namespace builtins {

template <typename Impl> class BuiltinImpl {
private:
  static constexpr const JSClassOps class_ops{};
  static constexpr const uint32_t class_flags = 0;

public:
  static constexpr JSClass class_{
      Impl::class_name,
      JSCLASS_HAS_RESERVED_SLOTS(Impl::Slots::Count) | class_flags,
      &class_ops,
  };

  static inline JS::Result<std::tuple<JS::CallArgs, JS::Rooted<JSObject *> *>>
  MethodHeaderWithName(int required_argc, JSContext *cx, unsigned argc, JS::Value *vp,
                       const char *name) {
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    if (!check_receiver(cx, args.thisv(), name)) {
      return JS::Result<std::tuple<JS::CallArgs, JS::Rooted<JSObject *> *>>(JS::Error());
    }
    JS::Rooted<JSObject *> self(cx, &args.thisv().toObject());
    if (!args.requireAtLeast(cx, name, required_argc)) {
      return JS::Result<std::tuple<JS::CallArgs, JS::Rooted<JSObject *> *>>(JS::Error());
    }

    return JS::Result<std::tuple<JS::CallArgs, JS::Rooted<JSObject *> *>>(
        std::make_tuple(args, &self));
  }

  static JS::PersistentRooted<JSObject *> proto_obj;

  static bool is_instance(JSObject *obj) { return obj != nullptr && JS::GetClass(obj) == &class_; }

  static bool is_instance(JS::Value val) { return val.isObject() && is_instance(&val.toObject()); }

  static bool check_receiver(JSContext *cx, JS::HandleValue receiver, const char *method_name) {
    if (!Impl::is_instance(receiver)) {
      JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_INCOMPATIBLE_INSTANCE,
                                method_name, Impl::class_.name);
      return false;
    }

    return true;
  }

  static bool init_class_impl(JSContext *cx, JS::HandleObject global,
                              JS::HandleObject parent_proto = nullptr) {
    proto_obj.init(cx, JS_InitClass(cx, global, &class_, parent_proto, Impl::class_name,
                                    Impl::constructor, Impl::ctor_length, Impl::properties,
                                    Impl::methods, nullptr, nullptr));

    return proto_obj != nullptr;
  }
};

template <typename Impl> JS::PersistentRooted<JSObject *> BuiltinImpl<Impl>::proto_obj{};

template <typename Impl> class BuiltinNoConstructor : public BuiltinImpl<Impl> {
public:
  static const int ctor_length = 1;

  static bool constructor(JSContext *cx, unsigned argc, JS::Value *vp) {
    JS_ReportErrorUTF8(cx, "%s can't be instantiated directly", Impl::class_name);
    return false;
  }

  static bool init_class(JSContext *cx, JS::HandleObject global) {
    return BuiltinImpl<Impl>::init_class_impl(cx, global) &&
           JS_DeleteProperty(cx, global, BuiltinImpl<Impl>::class_.name);
  }
};

} // namespace builtins

#endif
