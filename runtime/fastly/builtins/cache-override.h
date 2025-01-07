#ifndef FASTLY_CACHE_OVERRIDE_H
#define FASTLY_CACHE_OVERRIDE_H

#include "../host-api/host_api_fastly.h"
#include "builtin.h"

namespace fastly::cache_override {

class CacheOverride : public builtins::BuiltinImpl<CacheOverride> {
private:
public:
  static constexpr const char *class_name = "CacheOverride";
  static const int ctor_length = 1;

  // The values stored in these slots are ultimately passed to the host
  // via the fastly_req_cache_override_v2_set hostcall.
  //
  // If `Mode` is not `Override`, all other values are ignored.
  //
  // If `Mode` is `Override`, the values are interpreted in the following way:
  //
  // If `TTL`, `SWR`, or `SurrogateKey` are `undefined`, they're ignored.
  // For each of them, if the value isn't `undefined`, a flag gets set in the
  // hostcall's `tag` parameter, and the value itself is encoded as a uint32
  // parameter.
  //
  // `PCI` is interpreted as a boolean, and a flag gets set in the hostcall's
  // `tag` parameter if `PCI` is true.
  //
  // `BeforeSend` and `AfterSend` are function callbacks that can be set
  // to execute before and after sending the request.
  enum Slots { Mode, TTL, SWR, SurrogateKey, PCI, BeforeSend, AfterSend, Count };

  enum class CacheOverrideMode { None, Pass, Override };

  static const JSFunctionSpec static_methods[];
  static const JSPropertySpec static_properties[];
  static const JSFunctionSpec methods[];
  static const JSPropertySpec properties[];
  static host_api::CacheOverrideTag abi_tag(JSObject *self);
  static JS::Value ttl(JSObject *self);
  static void set_ttl(JSObject *self, uint32_t ttl);
  static JS::Value swr(JSObject *self);
  static void set_swr(JSObject *self, uint32_t swr);
  static JS::Value surrogate_key(JSObject *self);
  static void set_surrogate_key(JSObject *self, JSString *key);
  static JSObject *clone(JSContext *cx, JS::HandleObject self);
  static JS::Value pci(JSObject *self);
  static void set_pci(JSObject *self, bool pci);
  static JSObject *beforeSend(JSObject *self);
  static void set_beforeSend(JSObject *self, JSObject *fn);
  static JSObject *afterSend(JSObject *self);
  static void set_afterSend(JSObject *self, JSObject *fn);
  static CacheOverrideMode mode(JSObject *self);
  static void set_mode(JSObject *self, CacheOverride::CacheOverrideMode mode);
  static bool mode_get(JSContext *cx, JS::HandleObject self, JS::MutableHandleValue rval);
  static bool mode_set(JSContext *cx, JS::HandleObject self, JS::HandleValue val,
                       JS::MutableHandleValue rval);
  static bool ttl_get(JSContext *cx, JS::HandleObject self, JS::MutableHandleValue rval);
  static bool ttl_set(JSContext *cx, JS::HandleObject self, JS::HandleValue val,
                      JS::MutableHandleValue rval);
  static bool swr_get(JSContext *cx, JS::HandleObject self, JS::MutableHandleValue rval);
  static bool swr_set(JSContext *cx, JS::HandleObject self, JS::HandleValue val,
                      JS::MutableHandleValue rval);
  static bool surrogate_key_get(JSContext *cx, JS::HandleObject self, JS::MutableHandleValue rval);
  static bool surrogate_key_set(JSContext *cx, JS::HandleObject self, JS::HandleValue val,
                                JS::MutableHandleValue rval);
  static bool pci_get(JSContext *cx, JS::HandleObject self, JS::MutableHandleValue rval);
  static bool pci_set(JSContext *cx, JS::HandleObject self, JS::HandleValue val,
                      JS::MutableHandleValue rval);
  static bool before_send_get(JSContext *cx, JS::HandleObject self, JS::MutableHandleValue rval);
  static bool before_send_set(JSContext *cx, JS::HandleObject self, JS::HandleValue val,
                              JS::MutableHandleValue rval);
  static bool after_send_get(JSContext *cx, JS::HandleObject self, JS::MutableHandleValue rval);
  static bool after_send_set(JSContext *cx, JS::HandleObject self, JS::HandleValue val,
                             JS::MutableHandleValue rval);
  template <auto accessor_fn> static bool accessor_get(JSContext *cx, unsigned argc, JS::Value *vp);
  template <auto accessor_fn> static bool accessor_set(JSContext *cx, unsigned argc, JS::Value *vp);

  static bool ensure_override(JSContext *cx, JS::HandleObject self, const char *field);

  static bool constructor(JSContext *cx, unsigned argc, JS::Value *vp);
};

} // namespace fastly::cache_override

#endif
