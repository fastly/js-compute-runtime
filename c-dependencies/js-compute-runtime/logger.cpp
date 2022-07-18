#include "logger.h"
#include "../xqd.h"
#include "builtin.h"

namespace Logger {
namespace Slots {
enum { Endpoint, Count };
};

const unsigned ctor_length = 1;

bool check_receiver(JSContext *cx, JS::HandleValue receiver, const char *method_name);

static bool log(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(1)

  auto endpoint = LogEndpointHandle{(uint32_t)JS::GetReservedSlot(self, Slots::Endpoint).toInt32()};

  size_t msg_len;
  JS::UniqueChars msg = encode(cx, args.get(0), &msg_len);
  if (!msg)
    return false;

  size_t nwritten;
  if (!HANDLE_RESULT(cx, xqd_log_write(endpoint, msg.get(), msg_len, &nwritten)))
    return false;

  args.rval().setUndefined();
  return true;
}

const JSFunctionSpec methods[] = {JS_FN("log", log, 1, JSPROP_ENUMERATE), JS_FS_END};

const JSPropertySpec properties[] = {JS_PS_END};

CLASS_BOILERPLATE_NO_CTOR(Logger)

JSObject *create(JSContext *cx, const char *name) {
  JS::RootedObject logger(cx, JS_NewObjectWithGivenProto(cx, &class_, proto_obj));
  if (!logger)
    return nullptr;

  auto handle = LogEndpointHandle{INVALID_HANDLE};

  if (!HANDLE_RESULT(cx, xqd_log_endpoint_get(name, strlen(name), &handle)))
    return nullptr;

  JS::SetReservedSlot(logger, Slots::Endpoint, JS::Int32Value(handle.handle));

  return logger;
}
} // namespace Logger
