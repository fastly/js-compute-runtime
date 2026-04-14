// TODO: remove these once the warnings are fixed
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-offsetof"
#pragma clang diagnostic ignored "-Wdeprecated-enum-enum-conversion"
#include "js/experimental/TypedData.h" // used in "js/Conversions.h"
#pragma clang diagnostic pop

#include "js/Conversions.h"

#include "../../../StarlingMonkey/runtime/encode.h"
#include "cache-override.h"
#include "fastly.h"
#include "host_api.h"

using builtins::BuiltinImpl;
using fastly::FastlyGetErrorMessage;

namespace fastly::cache_override {

namespace {

bool parse_mode(JSContext *cx, JS::HandleValue mode, CacheOverride::CacheOverrideMode *mode_out) {
  auto mode_chars = core::encode(cx, mode);
  if (!mode_chars) {
    return false;
  }

  if (!strcmp(mode_chars.begin(), "none")) {
    *mode_out = CacheOverride::CacheOverrideMode::None;
  } else if (!strcmp(mode_chars.begin(), "pass")) {
    *mode_out = CacheOverride::CacheOverrideMode::Pass;
  } else if (!strcmp(mode_chars.begin(), "override")) {
    *mode_out = CacheOverride::CacheOverrideMode::Override;
  } else {
    JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr, JSMSG_CACHE_OVERRIDE_MODE_INVALID,
                              mode_chars.begin());
    return false;
  }

  return true;
}

} // namespace

CacheOverride::CacheOverrideMode CacheOverride::mode(JSObject *self) {
  return (CacheOverride::CacheOverrideMode)JS::GetReservedSlot(self, Slots::Mode).toInt32();
}

void CacheOverride::set_mode(JSObject *self, CacheOverride::CacheOverrideMode mode) {
  MOZ_ASSERT(is_instance(self));
  JS::SetReservedSlot(self, CacheOverride::Slots::Mode, JS::Int32Value((int32_t)mode));
}

JS::Value CacheOverride::ttl(JSObject *self) {
  MOZ_ASSERT(is_instance(self));
  if (CacheOverride::mode(self) != CacheOverride::CacheOverrideMode::Override) {
    return JS::UndefinedValue();
  }
  return JS::GetReservedSlot(self, Slots::TTL);
}

void CacheOverride::set_ttl(JSObject *self, uint32_t ttl) {
  MOZ_ASSERT(is_instance(self));
  MOZ_ASSERT(mode(self) == CacheOverride::CacheOverrideMode::Override);
  JS::SetReservedSlot(self, CacheOverride::Slots::TTL, JS::Int32Value((int32_t)ttl));
}

JS::Value CacheOverride::staleWhileRevalidate(JSObject *self) {
  MOZ_ASSERT(is_instance(self));
  if (CacheOverride::mode(self) != CacheOverride::CacheOverrideMode::Override)
    return JS::UndefinedValue();
  return JS::GetReservedSlot(self, Slots::StaleWhileRevalidate);
}

void CacheOverride::set_staleWhileRevalidate(JSObject *self, uint32_t swr) {
  MOZ_ASSERT(is_instance(self));
  MOZ_ASSERT(CacheOverride::mode(self) == CacheOverride::CacheOverrideMode::Override);
  JS::SetReservedSlot(self, CacheOverride::Slots::StaleWhileRevalidate,
                      JS::Int32Value((int32_t)swr));
}

JS::Value CacheOverride::staleIfError(JSObject *self) {
  MOZ_ASSERT(is_instance(self));
  if (CacheOverride::mode(self) != CacheOverride::CacheOverrideMode::Override)
    return JS::UndefinedValue();
  return JS::GetReservedSlot(self, Slots::StaleIfError);
}

void CacheOverride::set_staleIfError(JSObject *self, uint32_t sie) {
  MOZ_ASSERT(is_instance(self));
  MOZ_ASSERT(CacheOverride::mode(self) == CacheOverride::CacheOverrideMode::Override);
  JS::SetReservedSlot(self, CacheOverride::Slots::StaleIfError, JS::Int32Value((int32_t)sie));
}

JS::Value CacheOverride::surrogate_key(JSObject *self) {
  MOZ_ASSERT(is_instance(self));
  if (CacheOverride::mode(self) != CacheOverride::CacheOverrideMode::Override)
    return JS::UndefinedValue();
  return JS::GetReservedSlot(self, Slots::SurrogateKey);
}

void CacheOverride::set_surrogate_key(JSObject *self, JSString *key) {
  MOZ_ASSERT(is_instance(self));
  MOZ_ASSERT(CacheOverride::mode(self) == CacheOverride::CacheOverrideMode::Override);
  JS::SetReservedSlot(self, CacheOverride::Slots::SurrogateKey, JS::StringValue(key));
}

JS::Value CacheOverride::pci(JSObject *self) {
  MOZ_ASSERT(is_instance(self));
  if (CacheOverride::mode(self) != CacheOverride::CacheOverrideMode::Override)
    return JS::UndefinedValue();
  return JS::GetReservedSlot(self, Slots::PCI);
}

void CacheOverride::set_pci(JSObject *self, bool pci) {
  MOZ_ASSERT(is_instance(self));
  MOZ_ASSERT(CacheOverride::mode(self) == CacheOverride::CacheOverrideMode::Override);
  JS::SetReservedSlot(self, CacheOverride::Slots::PCI, JS::BooleanValue(pci));
}

JSObject *CacheOverride::beforeSend(JSObject *self) {
  MOZ_ASSERT(is_instance(self));
  auto before_send = JS::GetReservedSlot(self, Slots::BeforeSend);
  if (before_send.isUndefined()) {
    return nullptr;
  }
  return before_send.toObjectOrNull();
}

void CacheOverride::set_beforeSend(JSObject *self, JSObject *fn) {
  MOZ_ASSERT(is_instance(self));
  JS::SetReservedSlot(self, Slots::BeforeSend, JS::ObjectValue(*fn));
}

JSObject *CacheOverride::afterSend(JSObject *self) {
  MOZ_ASSERT(is_instance(self));
  auto after_send = JS::GetReservedSlot(self, Slots::AfterSend);
  if (after_send.isUndefined()) {
    return nullptr;
  }
  return after_send.toObjectOrNull();
}

void CacheOverride::set_afterSend(JSObject *self, JSObject *fn) {
  MOZ_ASSERT(is_instance(self));
  JS::SetReservedSlot(self, Slots::AfterSend, JS::ObjectValue(*fn));
}

host_api::CacheOverrideTag CacheOverride::abi_tag(JSObject *self) {
  host_api::CacheOverrideTag tag;

  MOZ_ASSERT(is_instance(self));
  switch (CacheOverride::mode(self)) {
  case CacheOverride::CacheOverrideMode::None:
    return tag;
  case CacheOverride::CacheOverrideMode::Pass:
    tag.set_pass();
    return tag;
  default:;
  }

  if (!ttl(self).isUndefined()) {
    tag.set_ttl();
  }

  if (!staleWhileRevalidate(self).isUndefined()) {
    tag.set_stale_while_revalidate();
  }

  if (!pci(self).isUndefined() && pci(self).toBoolean()) {
    tag.set_pci();
  }

  return tag;
}

bool CacheOverride::mode_get(JSContext *cx, JS::HandleObject self, JS::MutableHandleValue rval) {
  if (self == proto_obj) {
    return api::throw_error(cx, api::Errors::WrongReceiver, "mode get", "CacheOverride");
  }
  const char *mode_chars;
  switch (mode(self)) {
  case CacheOverrideMode::None:
    mode_chars = "none";
    break;
  case CacheOverrideMode::Pass:
    mode_chars = "pass";
    break;
  case CacheOverrideMode::Override:
    mode_chars = "override";
    break;
  }

  JS::RootedString mode_str(cx, JS_NewStringCopyZ(cx, mode_chars));
  if (!mode_str)
    return false;

  rval.setString(mode_str);
  return true;
}

bool CacheOverride::ensure_override(JSContext *cx, JS::HandleObject self, const char *field) {
  if (mode(self) == CacheOverrideMode::Override)
    return true;

  JS_ReportErrorUTF8(cx,
                     "Can't set %s on CacheOverride object whose mode "
                     "isn't \"override\"",
                     field);
  return false;
}

bool CacheOverride::mode_set(JSContext *cx, JS::HandleObject self, JS::HandleValue val,
                             JS::MutableHandleValue ret) {
  if (self == proto_obj) {
    return api::throw_error(cx, api::Errors::WrongReceiver, "mode get", "CacheOverride");
  }

  CacheOverrideMode mode;
  if (!parse_mode(cx, val, &mode)) {
    return false;
  }

  set_mode(self, mode);
  return true;
}

bool CacheOverride::ttl_get(JSContext *cx, JS::HandleObject self, JS::MutableHandleValue rval) {
  if (self == proto_obj) {
    return api::throw_error(cx, api::Errors::WrongReceiver, "ttl get", "CacheOverride");
  }
  rval.set(ttl(self));
  return true;
}

bool CacheOverride::ttl_set(JSContext *cx, JS::HandleObject self, JS::HandleValue val,
                            JS::MutableHandleValue rval) {
  if (self == proto_obj) {
    return api::throw_error(cx, api::Errors::WrongReceiver, "ttl set", "CacheOverride");
  }
  if (!ensure_override(cx, self, "a TTL"))
    return false;

  if (val.isUndefined()) {
    JS::SetReservedSlot(self, Slots::TTL, val);
  } else {
    int32_t ttl;
    if (!JS::ToInt32(cx, val, &ttl))
      return false;

    set_ttl(self, ttl);
  }
  rval.set(ttl(self));
  return true;
}

bool CacheOverride::staleWhileRevalidate_get(JSContext *cx, JS::HandleObject self,
                                             JS::MutableHandleValue rval) {
  if (self == proto_obj) {
    return api::throw_error(cx, api::Errors::WrongReceiver, "staleWhileRevalidate get",
                            "CacheOverride");
  }
  rval.set(staleWhileRevalidate(self));
  return true;
}

bool CacheOverride::staleWhileRevalidate_set(JSContext *cx, JS::HandleObject self,
                                             JS::HandleValue val, JS::MutableHandleValue rval) {
  if (self == proto_obj) {
    return api::throw_error(cx, api::Errors::WrongReceiver, "staleWhileRevalidate set",
                            "CacheOverride");
  }
  if (!ensure_override(cx, self, "staleWhileRevalidate"))
    return false;

  if (val.isUndefined()) {
    JS::SetReservedSlot(self, Slots::StaleWhileRevalidate, val);
  } else {
    int32_t swr;
    if (!JS::ToInt32(cx, val, &swr))
      return false;

    set_staleWhileRevalidate(self, swr);
  }
  rval.set(staleWhileRevalidate(self));
  return true;
}

bool CacheOverride::staleIfError_get(JSContext *cx, JS::HandleObject self,
                                     JS::MutableHandleValue rval) {
  if (self == proto_obj) {
    return api::throw_error(cx, api::Errors::WrongReceiver, "staleIfError get", "CacheOverride");
  }
  rval.set(staleIfError(self));
  return true;
}

bool CacheOverride::staleIfError_set(JSContext *cx, JS::HandleObject self, JS::HandleValue val,
                                     JS::MutableHandleValue rval) {
  if (self == proto_obj) {
    return api::throw_error(cx, api::Errors::WrongReceiver, "staleIfError set", "CacheOverride");
  }
  if (!ensure_override(cx, self, "staleIfError"))
    return false;

  if (val.isUndefined()) {
    JS::SetReservedSlot(self, Slots::StaleIfError, val);
  } else {
    int32_t swr;
    if (!JS::ToInt32(cx, val, &swr))
      return false;

    set_staleIfError(self, swr);
  }
  rval.set(staleIfError(self));
  return true;
}

bool CacheOverride::surrogate_key_get(JSContext *cx, JS::HandleObject self,
                                      JS::MutableHandleValue rval) {
  if (self == proto_obj) {
    return api::throw_error(cx, api::Errors::WrongReceiver, "surrogate_key get", "CacheOverride");
  }
  rval.set(surrogate_key(self));
  return true;
}

bool CacheOverride::surrogate_key_set(JSContext *cx, JS::HandleObject self, JS::HandleValue val,
                                      JS::MutableHandleValue rval) {
  if (self == proto_obj) {
    return api::throw_error(cx, api::Errors::WrongReceiver, "surrogate_key set", "CacheOverride");
  }
  if (!ensure_override(cx, self, "a surrogate key"))
    return false;

  if (val.isUndefined()) {
    JS::SetReservedSlot(self, Slots::SurrogateKey, val);
  } else {
    JS::RootedString surrogate_key(cx, JS::ToString(cx, val));
    if (!surrogate_key)
      return false;

    set_surrogate_key(self, surrogate_key);
  }
  rval.set(surrogate_key(self));
  return true;
}

bool CacheOverride::pci_get(JSContext *cx, JS::HandleObject self, JS::MutableHandleValue rval) {
  if (self == proto_obj) {
    return api::throw_error(cx, api::Errors::WrongReceiver, "pci get", "CacheOverride");
  }
  rval.set(pci(self));
  return true;
}

bool CacheOverride::pci_set(JSContext *cx, JS::HandleObject self, JS::HandleValue val,
                            JS::MutableHandleValue rval) {
  if (self == proto_obj) {
    return api::throw_error(cx, api::Errors::WrongReceiver, "pci set", "CacheOverride");
  }
  if (!ensure_override(cx, self, "PCI"))
    return false;

  if (val.isUndefined()) {
    JS::SetReservedSlot(self, Slots::PCI, val);
  } else {
    bool pci = JS::ToBoolean(val);
    set_pci(self, pci);
  }
  rval.set(pci(self));
  return true;
}

bool CacheOverride::before_send_get(JSContext *cx, JS::HandleObject self,
                                    JS::MutableHandleValue rval) {
  if (self == proto_obj) {
    return api::throw_error(cx, api::Errors::WrongReceiver, "beforeSend get", "CacheOverride");
  }
  JSObject *beforeSend(self);
  if (!beforeSend) {
    rval.setUndefined();
    return true;
  }
  rval.setObject(*beforeSend);
  return true;
}

bool CacheOverride::before_send_set(JSContext *cx, JS::HandleObject self, JS::HandleValue val,
                                    JS::MutableHandleValue rval) {
  if (self == proto_obj) {
    return api::throw_error(cx, api::Errors::WrongReceiver, "beforeSend set", "CacheOverride");
  }
  if (!ensure_override(cx, self, "beforeSend"))
    return false;
  if (val.isUndefined()) {
    JS::SetReservedSlot(self, Slots::BeforeSend, JS::UndefinedValue());
    rval.setUndefined();
    return true;
  } else if (!val.isObject() || !JS::IsCallable(&val.toObject())) {
    JS_ReportErrorUTF8(cx, "CacheOverride: beforeSend must be a function");
    return false;
  } else {
    set_beforeSend(self, &val.toObject());
  }

  rval.setObject(*beforeSend(self));
  return true;
}

bool CacheOverride::after_send_get(JSContext *cx, JS::HandleObject self,
                                   JS::MutableHandleValue rval) {
  if (self == proto_obj) {
    return api::throw_error(cx, api::Errors::WrongReceiver, "afterSend get", "CacheOverride");
  }
  JSObject *afterSend(self);
  if (!afterSend) {
    rval.setUndefined();
    return true;
  }
  rval.setObject(*afterSend);
  return true;
}

bool CacheOverride::after_send_set(JSContext *cx, JS::HandleObject self, JS::HandleValue val,
                                   JS::MutableHandleValue rval) {
  if (self == proto_obj) {
    return api::throw_error(cx, api::Errors::WrongReceiver, "afterSend set", "CacheOverride");
  }
  if (!ensure_override(cx, self, "afterSend"))
    return false;
  if (val.isUndefined()) {
    JS::SetReservedSlot(self, Slots::AfterSend, JS::UndefinedValue());
    rval.setUndefined();
    return true;
  } else if (!val.isObject() || !JS::IsCallable(&val.toObject())) {
    JS_ReportErrorUTF8(cx, "CacheOverride: afterSend must be a function");
    return false;
  } else {
    set_afterSend(self, &val.toObject());
  }

  rval.setObject(*afterSend(self));
  return true;
}

template <auto accessor_fn>
bool CacheOverride::accessor_get(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0)
  return accessor_fn(cx, self, args.rval());
}

template <auto accessor_fn>
bool CacheOverride::accessor_set(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(1)
  return accessor_fn(cx, self, args[0], args.rval());
}

const JSFunctionSpec CacheOverride::static_methods[] = {JS_FS_END};
const JSPropertySpec CacheOverride::static_properties[] = {JS_PS_END};
const JSFunctionSpec CacheOverride::methods[] = {JS_FS_END};

const JSPropertySpec CacheOverride::properties[] = {
    JS_PSGS("mode", accessor_get<mode_get>, accessor_set<mode_set>, JSPROP_ENUMERATE),
    JS_PSGS("ttl", accessor_get<ttl_get>, accessor_set<ttl_set>, JSPROP_ENUMERATE),
    JS_PSGS("swr", accessor_get<staleWhileRevalidate_get>, accessor_set<staleWhileRevalidate_set>,
            JSPROP_ENUMERATE),
    JS_PSGS("staleWhileRevalidate", accessor_get<staleWhileRevalidate_get>,
            accessor_set<staleWhileRevalidate_set>, JSPROP_ENUMERATE),
    JS_PSGS("surrogateKey", accessor_get<surrogate_key_get>, accessor_set<surrogate_key_set>,
            JSPROP_ENUMERATE),
    JS_PSGS("pci", accessor_get<pci_get>, accessor_set<pci_set>, JSPROP_ENUMERATE),
    JS_PSGS("beforeSend", accessor_get<before_send_get>, accessor_set<before_send_set>,
            JSPROP_ENUMERATE),
    JS_PSGS("afterSend", accessor_get<after_send_get>, accessor_set<after_send_set>,
            JSPROP_ENUMERATE),
    JS_PSGS("staleIfError", accessor_get<staleIfError_get>, accessor_set<staleIfError_set>,
            JSPROP_ENUMERATE),
    JS_STRING_SYM_PS(toStringTag, "CacheOverride", JSPROP_READONLY),
    JS_PS_END};

JSObject *CacheOverride::create(JSContext *cx, JS::HandleValue override) {
  JS::RootedObject self(cx, JS_NewObjectWithGivenProto(cx, &class_, proto_obj));
  JS::RootedValue val(cx);

  if (override.isString()) {
    CacheOverrideMode mode;
    if (!parse_mode(cx, override, &mode)) {
      return nullptr;
    }
    set_mode(self, mode);
    return self;
  } else if (!override.isObject()) {
    JS_ReportErrorUTF8(cx, "CacheOverride must be created from a string mode or override object");
    return nullptr;
  }

  JS::RootedObject override_obj(cx, &override.toObject());

  set_mode(self, CacheOverrideMode::Override);

  if (!JS_GetProperty(cx, override_obj, "ttl", &val) || !ttl_set(cx, self, val, &val)) {
    return nullptr;
  }

  if (!JS_GetProperty(cx, override_obj, "swr", &val) ||
      !staleWhileRevalidate_set(cx, self, val, &val)) {
    return nullptr;
  }

  if (!JS_GetProperty(cx, override_obj, "staleWhileRevalidate", &val)) {
    return nullptr;
  }
  if (!val.isNullOrUndefined()) {
    if (!staleWhileRevalidate_set(cx, self, val, &val)) {
      return nullptr;
    }
  }

  if (!JS_GetProperty(cx, override_obj, "staleIfError", &val) ||
      !staleIfError_set(cx, self, val, &val)) {
    return nullptr;
  }

  if (!JS_GetProperty(cx, override_obj, "surrogateKey", &val) ||
      !surrogate_key_set(cx, self, val, &val)) {
    return nullptr;
  }

  if (!JS_GetProperty(cx, override_obj, "pci", &val) || !pci_set(cx, self, val, &val)) {
    return nullptr;
  }

  if (!JS_GetProperty(cx, override_obj, "beforeSend", &val) ||
      !before_send_set(cx, self, val, &val)) {
    return nullptr;
  }

  if (!JS_GetProperty(cx, override_obj, "afterSend", &val) ||
      !after_send_set(cx, self, val, &val)) {
    return nullptr;
  }

  return self;
}

bool CacheOverride::constructor(JSContext *cx, unsigned argc, JS::Value *vp) {
  CTOR_HEADER("CacheOverride", 1);

  JS::RootedValue init(cx);
  CacheOverrideMode mode;
  if (args[0].isObject()) {
    init.setObject(args[0].toObject());
    mode = CacheOverrideMode::Override;
  } else {
    if (!parse_mode(cx, args[0], &mode)) {
      return false;
    }
    if (args.length() > 1) {
      init.set(args[1]);
    }
  }

  if (mode == CacheOverride::CacheOverrideMode::Override) {
    if (!init.isObject()) {
      JS_ReportErrorUTF8(cx, "Creating a CacheOverride object with mode \"override\" requires "
                             "an init object for the override parameters as the second argument");
      return false;
    }

    JSObject *cache_override = create(cx, init);
    if (!cache_override) {
      return false;
    }
    args.rval().setObject(*cache_override);
    return true;
  }

  JS::RootedObject cache_override(cx, JS_NewObjectWithGivenProto(cx, &class_, proto_obj));
  if (!cache_override) {
    return false;
  }
  set_mode(cache_override, mode);
  args.rval().setObject(*cache_override);
  return true;
}

/**
 * Clone a CacheOverride instance by copying all its reserved slots.
 *
 * This works because CacheOverride slots only contain primitive values.
 */
JSObject *CacheOverride::clone(JSContext *cx, JS::HandleObject self) {
  MOZ_ASSERT(is_instance(self));
  JS::RootedObject result(cx, JS_NewObjectWithGivenProto(cx, &class_, proto_obj));
  if (!result) {
    return nullptr;
  }

  for (size_t i = 0; i < Slots::Count; i++) {
    JS::Value val = JS::GetReservedSlot(self, i);
    JS::SetReservedSlot(result, i, val);
  }

  return result;
}

bool install(api::Engine *engine) {
  if (!CacheOverride::init_class_impl(engine->cx(), engine->global())) {
    return false;
  }
  RootedObject cache_override(engine->cx(), JS_NewObject(engine->cx(), nullptr));
  RootedValue cache_override_class(engine->cx());
  if (!JS_GetProperty(engine->cx(), engine->global(), "CacheOverride", &cache_override_class)) {
    return false;
  }
  if (!JS_SetProperty(engine->cx(), cache_override, "CacheOverride", cache_override_class)) {
    return false;
  }
  RootedValue cache_override_val(engine->cx(), JS::ObjectValue(*cache_override));
  if (!engine->define_builtin_module("fastly:cache-override", cache_override_val)) {
    return false;
  }
  return true;
}

} // namespace fastly::cache_override
