#ifndef JS_COMPUTE_RUNTIME_BUILTIN_H
#define JS_COMPUTE_RUNTIME_BUILTIN_H

#include "js-compute-builtins.h"

/* Returns false if an exception is set on `cx` and the caller should
   immediately return to propagate the exception. */
static inline bool handle_fastly_result(JSContext *cx, int result, int line, const char *func) {
  switch (result) {
  case 0:
    return true;
  case 1:
    JS_ReportErrorUTF8(cx,
                       "%s: Generic error value. This means that some unexpected error "
                       "occurred during a hostcall. - Fastly error code %d\n",
                       func, result);
    return false;
  case 2:
    JS_ReportErrorUTF8(cx, "%s: Invalid argument. - Fastly error code %d\n", func, result);
    return false;
  case 3:
    JS_ReportErrorUTF8(cx,
                       "%s: Invalid handle. Thrown when a request, response, dictionary, or "
                       "body handle is not valid. - Fastly error code %d\n",
                       func, result);
    return false;
  case 4:
    JS_ReportErrorUTF8(cx, "%s: Buffer length error. Buffer is too long. - Fastly error code %d\n",
                       func, result);
    return false;
  case 5:
    JS_ReportErrorUTF8(cx,
                       "%s: Unsupported operation error. This error is thrown "
                       "when some operation cannot be performed, because it is "
                       "not supported. - Fastly error code %d\n",
                       func, result);
    return false;
  case 6:
    JS_ReportErrorUTF8(cx,
                       "%s: Alignment error. This is thrown when a pointer does not point to "
                       "a properly aligned slice of memory. - Fastly error code %d\n",
                       func, result);
    return false;
  case 7:
    JS_ReportErrorUTF8(cx,
                       "%s: HTTP parse error. This can be thrown when a method, URI, header, "
                       "or status is not valid. This can also be thrown if a message head is "
                       "too large. - Fastly error code %d\n",
                       func, result);
    return false;
  case 8:
    JS_ReportErrorUTF8(cx,
                       "%s: HTTP user error. This is thrown in cases where user code caused "
                       "an HTTP error. For example, attempt to send a 1xx response code, or a "
                       "request with a non-absolute URI. This can also be caused by an "
                       "unexpected header: both `content-length` and `transfer-encoding`, for "
                       "example. - Fastly error code %d\n",
                       func, result);
    return false;
  case 9:
    JS_ReportErrorUTF8(cx,
                       "%s: HTTP incomplete message error. A stream ended "
                       "unexpectedly. - Fastly error code %d\n",
                       func, result);
    return false;
  case 10:
    JS_ReportErrorUTF8(cx,
                       "%s: A `None` error. This status code is used to "
                       "indicate when an optional value did not exist, as "
                       "opposed to an empty value. - Fastly error code %d\n",
                       func, result);
    return false;
  case 11:
    JS_ReportErrorUTF8(cx,
                       "%s: HTTP head too large error. This error will be thrown when the "
                       "message head is too large. - Fastly error code %d\n",
                       func, result);
    return false;
  case 12:
    JS_ReportErrorUTF8(cx,
                       "%s: HTTP invalid status error. This error will be "
                       "thrown when the HTTP message contains an invalid "
                       "status code. - Fastly error code %d\n",
                       func, result);
    return false;
  default:
    fprintf(stdout, __FILE__ ":%d (%s) - Fastly error code %d\n", line, func, result);
    JS_ReportErrorUTF8(cx, "Fastly error code %d", result);
    return false;
  }
}

#define HANDLE_RESULT(cx, result) handle_fastly_result(cx, result, __LINE__, __func__)

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
