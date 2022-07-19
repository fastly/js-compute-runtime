#ifndef JS_COMPUTE_RUNTIME_BUILTIN_H
#define JS_COMPUTE_RUNTIME_BUILTIN_H

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
  static constexpr const JSClassOps class_ops = {};                                                \
  static const uint32_t class_flags = 0;                                                           \
                                                                                                   \
  const JSClass class_ = {#cls, JSCLASS_HAS_RESERVED_SLOTS(Slots::Count) | class_flags,            \
                          &class_ops};                                                             \
  static JS::PersistentRooted<JSObject *> proto_obj;                                               \
                                                                                                   \
  bool is_instance(JSObject *obj) { return !!obj && JS::GetClass(obj) == &class_; }                \
                                                                                                   \
  bool is_instance(JS::Value val) { return val.isObject() && is_instance(&val.toObject()); }       \
                                                                                                   \
  bool check_receiver(JSContext *cx, JS::HandleValue receiver, const char *method_name) {          \
    if (!is_instance(receiver)) {                                                                  \
      JS_ReportErrorUTF8(cx, "Method %s called on receiver that's not an instance of %s\n",        \
                         method_name, class_.name);                                                \
      return false;                                                                                \
    }                                                                                              \
    return true;                                                                                   \
  };                                                                                               \
                                                                                                   \
  bool init_class_impl(JSContext *cx, JS::HandleObject global,                                     \
                       JS::HandleObject parent_proto = nullptr) {                                  \
    proto_obj.init(cx, JS_InitClass(cx, global, parent_proto, &class_, constructor, ctor_length,   \
                                    properties, methods, nullptr, nullptr));                       \
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

#endif
