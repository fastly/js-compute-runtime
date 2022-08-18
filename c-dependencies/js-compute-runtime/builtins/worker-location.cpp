#include "builtin.h"
#include "builtins/url.h"
#include "js-compute-builtins.h"

/**
 * The `WorkerLocation` builtin, added to the global object as the data property
 * `location`.
 * https://html.spec.whatwg.org/multipage/workers.html#worker-locations
 */
namespace WorkerLocation {
namespace Slots {
enum { Count };
};

static JS::PersistentRooted<JSObject *> url;

const unsigned ctor_length = 1;

bool constructor(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS_ReportErrorLatin1(cx, "Illegal constructor WorkerLocation");
  return false;
}

bool check_receiver(JSContext *cx, JS::HandleValue receiver, const char *method_name);

#define ACCESSOR_GET(field)                                                                        \
  bool field##_get(JSContext *cx, unsigned argc, JS::Value *vp) {                                      \
    METHOD_HEADER(0)                                                                               \
    REQUEST_HANDLER_ONLY("location." #field)                                                       \
    return URL::field(cx, url, args.rval());                                                       \
  }

ACCESSOR_GET(href)
ACCESSOR_GET(origin)
ACCESSOR_GET(protocol)
ACCESSOR_GET(host)
ACCESSOR_GET(hostname)
ACCESSOR_GET(port)
ACCESSOR_GET(pathname)
ACCESSOR_GET(search)
ACCESSOR_GET(hash)

#undef ACCESSOR_GET

bool toString(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0)
  return href_get(cx, argc, vp);
}

const JSFunctionSpec methods[] = {JS_FN("toString", toString, 0, JSPROP_ENUMERATE), JS_FS_END};

const JSPropertySpec properties[] = {JS_PSG("href", href_get, JSPROP_ENUMERATE),
                                     JS_PSG("origin", origin_get, JSPROP_ENUMERATE),
                                     JS_PSG("protocol", protocol_get, JSPROP_ENUMERATE),
                                     JS_PSG("host", host_get, JSPROP_ENUMERATE),
                                     JS_PSG("hostname", hostname_get, JSPROP_ENUMERATE),
                                     JS_PSG("port", port_get, JSPROP_ENUMERATE),
                                     JS_PSG("pathname", pathname_get, JSPROP_ENUMERATE),
                                     JS_PSG("search", search_get, JSPROP_ENUMERATE),
                                     JS_PSG("hash", hash_get, JSPROP_ENUMERATE),
                                     JS_STRING_SYM_PS(toStringTag, "Location", JSPROP_READONLY),
                                     JS_PS_END};

CLASS_BOILERPLATE_CUSTOM_INIT(WorkerLocation)

bool init_class(JSContext *cx, JS::HandleObject global) {
  if (!init_class_impl(cx, global)) {
    return false;
  }

  url.init(cx);

  JS::RootedObject location(cx, JS_NewObjectWithGivenProto(cx, &class_, proto_obj));
  if (!location) {
    return false;
  }

  return JS_DefineProperty(cx, global, "location", location, JSPROP_ENUMERATE);
}
} // namespace WorkerLocation