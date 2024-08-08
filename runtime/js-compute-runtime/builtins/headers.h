#ifndef BUILTINS_HEADERS_H
#define BUILTINS_HEADERS_H

#include "builtin.h"
#include "host_interface/host_api.h"
namespace builtins {

const char VALID_NAME_CHARS[128] = {
    0, 0, 0, 0, 0, 0, 0, 0, //   0
    0, 0, 0, 0, 0, 0, 0, 0, //   8
    0, 0, 0, 0, 0, 0, 0, 0, //  16
    0, 0, 0, 0, 0, 0, 0, 0, //  24

    0, 1, 0, 1, 1, 1, 1, 1, //  32
    0, 0, 1, 1, 0, 1, 1, 0, //  40
    1, 1, 1, 1, 1, 1, 1, 1, //  48
    1, 1, 0, 0, 0, 0, 0, 0, //  56

    0, 1, 1, 1, 1, 1, 1, 1, //  64
    1, 1, 1, 1, 1, 1, 1, 1, //  72
    1, 1, 1, 1, 1, 1, 1, 1, //  80
    1, 1, 1, 0, 0, 0, 1, 1, //  88

    1, 1, 1, 1, 1, 1, 1, 1, //  96
    1, 1, 1, 1, 1, 1, 1, 1, // 104
    1, 1, 1, 1, 1, 1, 1, 1, // 112
    1, 1, 1, 0, 1, 0, 1, 0  // 120
};

host_api::HostString normalize_header_name(JSContext *cx, JS::MutableHandleValue name_val,
                                           const char *fun_name);

host_api::HostString normalize_header_value(JSContext *cx, JS::MutableHandleValue value_val,
                                            const char *fun_name);

class Headers final : public BuiltinImpl<Headers> {
  static bool get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool set(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool has(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool append(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool delete_(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool forEach(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool entries(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool keys(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool values(JSContext *cx, unsigned argc, JS::Value *vp);

public:
  enum class Mode : int32_t {
    Standalone,
    ProxyToRequest,
    ProxyToResponse,
  };

  static constexpr const char *class_name = "Headers";

  enum class Slots {
    BackingMap,
    Handle,
    Mode,
    HasLazyValues,
    Immutable,
    Count,
  };

  static bool is_immutable(JS::HandleObject self);

  static bool delazify(JSContext *cx, JS::HandleObject headers);

  /**
   * Adds the given header name/value to `self`'s list of headers if `self`
   * doesn't already contain a header with that name.
   *
   * Assumes that both the name and value are valid and normalized.
   * TODO(performance): fully skip normalization.
   * https://github.com/fastly/js-compute-runtime/issues/221
   */
  static bool maybe_add(JSContext *cx, JS::HandleObject self, const char *name, const char *value);

  // Appends a non-normalized value for a non-normalized header name to both
  // the JS side Map and, in non-standalone mode, the host.
  //
  // Verifies and normalizes the name and value.
  static bool append_header_value(JSContext *cx, JS::HandleObject self, JS::HandleValue name,
                                  JS::HandleValue value, const char *fun_name);

  static const JSFunctionSpec static_methods[];
  static const JSPropertySpec static_properties[];
  static const JSFunctionSpec methods[];
  static const JSPropertySpec properties[];

  static const unsigned ctor_length = 1;

  static bool init_class(JSContext *cx, JS::HandleObject global);
  static bool constructor(JSContext *cx, unsigned argc, JS::Value *vp);

  static JSObject *create(JSContext *cx, JS::HandleObject headers, enum Mode mode,
                          JS::HandleObject owner, JS::HandleObject init_headers, bool immutable);
  static JSObject *create(JSContext *cx, JS::HandleObject headers, enum Mode mode,
                          JS::HandleObject owner, JS::HandleValue initv, bool immutable);
  static JSObject *create(JSContext *cx, JS::HandleObject self, enum Mode mode,
                          JS::HandleObject owner, bool immutable);
};

} // namespace builtins

#endif
