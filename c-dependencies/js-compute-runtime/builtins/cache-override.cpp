// TODO: remove these once the warnings are fixed
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-offsetof"
#pragma clang diagnostic ignored "-Wdeprecated-enum-enum-conversion"
#include "js/experimental/TypedData.h" // used in "js/Conversions.h"
#pragma clang diagnostic pop

#include "js/Conversions.h"

#include "cache-override.h"
#include "host_interface/host_call.h"
#include "js-compute-builtins.h"

namespace builtins {

CacheOverride::CacheOverrideMode CacheOverride::mode(JSObject *self) {
  return (CacheOverride::CacheOverrideMode)JS::GetReservedSlot(self, Slots::Mode).toInt32();
}

void CacheOverride::set_mode(JSObject *self, CacheOverride::CacheOverrideMode mode) {
  MOZ_ASSERT(is_instance(self));
  JS::SetReservedSlot(self, CacheOverride::Slots::Mode, JS::Int32Value((int32_t)mode));
}

JS::Value CacheOverride::ttl(JSObject *self) {
  MOZ_ASSERT(is_instance(self));
  if (CacheOverride::mode(self) != CacheOverride::CacheOverrideMode::Override)
    return JS::UndefinedValue();
  return JS::GetReservedSlot(self, Slots::TTL);
}

void CacheOverride::set_ttl(JSObject *self, uint32_t ttl) {
  MOZ_ASSERT(is_instance(self));
  MOZ_RELEASE_ASSERT(mode(self) == CacheOverride::CacheOverrideMode::Override);
  JS::SetReservedSlot(self, CacheOverride::Slots::TTL, JS::Int32Value((int32_t)ttl));
}

JS::Value CacheOverride::swr(JSObject *self) {
  MOZ_ASSERT(is_instance(self));
  if (CacheOverride::mode(self) != CacheOverride::CacheOverrideMode::Override)
    return JS::UndefinedValue();
  return JS::GetReservedSlot(self, Slots::SWR);
}

void CacheOverride::set_swr(JSObject *self, uint32_t swr) {
  MOZ_ASSERT(is_instance(self));
  MOZ_RELEASE_ASSERT(CacheOverride::mode(self) == CacheOverride::CacheOverrideMode::Override);
  JS::SetReservedSlot(self, CacheOverride::Slots::SWR, JS::Int32Value((int32_t)swr));
}

JS::Value CacheOverride::surrogate_key(JSObject *self) {
  MOZ_ASSERT(is_instance(self));
  if (CacheOverride::mode(self) != CacheOverride::CacheOverrideMode::Override)
    return JS::UndefinedValue();
  return JS::GetReservedSlot(self, Slots::SurrogateKey);
}

void CacheOverride::set_surrogate_key(JSObject *self, JSString *key) {
  MOZ_ASSERT(is_instance(self));
  MOZ_RELEASE_ASSERT(CacheOverride::mode(self) == CacheOverride::CacheOverrideMode::Override);
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
  MOZ_RELEASE_ASSERT(CacheOverride::mode(self) == CacheOverride::CacheOverrideMode::Override);
  JS::SetReservedSlot(self, CacheOverride::Slots::PCI, JS::BooleanValue(pci));
}

fastly_http_cache_override_tag_t CacheOverride::abi_tag(JSObject *self) {
  MOZ_ASSERT(is_instance(self));
  switch (CacheOverride::mode(self)) {
  case CacheOverride::CacheOverrideMode::None:
    return 0;
  case CacheOverride::CacheOverrideMode::Pass:
    return FASTLY_HTTP_CACHE_OVERRIDE_TAG_PASS;
  default:;
  }

  fastly_http_cache_override_tag_t tag = 0;
  if (!ttl(self).isUndefined())
    tag |= FASTLY_HTTP_CACHE_OVERRIDE_TAG_TTL;
  if (!swr(self).isUndefined())
    tag |= FASTLY_HTTP_CACHE_OVERRIDE_TAG_STALE_WHILE_REVALIDATE;
  if (!pci(self).isUndefined())
    tag |= FASTLY_HTTP_CACHE_OVERRIDE_TAG_PCI;

  return tag;
}

bool CacheOverride::mode_get(JSContext *cx, JS::HandleObject self, JS::MutableHandleValue rval) {
  if (self == proto_obj) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_INVALID_INTERFACE, "mode get",
                              "CacheOverride");
    return false;
  }
  const char *mode_chars;
  switch (CacheOverride::mode(self)) {
  case CacheOverride::CacheOverrideMode::None:
    mode_chars = "none";
    break;
  case CacheOverride::CacheOverrideMode::Pass:
    mode_chars = "pass";
    break;
  case CacheOverride::CacheOverrideMode::Override:
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
  if (CacheOverride::mode(self) == CacheOverride::CacheOverrideMode::Override)
    return true;

  JS_ReportErrorUTF8(cx,
                     "Can't set %s on CacheOverride object whose mode "
                     "isn't \"override\"",
                     field);
  return false;
}

bool CacheOverride::mode_set(JSContext *cx, JS::HandleObject self, JS::HandleValue val,
                             JS::MutableHandleValue rval) {
  if (self == proto_obj) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_INVALID_INTERFACE, "mode set",
                              "CacheOverride");
    return false;
  }
  size_t mode_len;
  JS::UniqueChars mode_chars = encode(cx, val, &mode_len);
  if (!mode_chars)
    return false;

  CacheOverride::CacheOverrideMode mode;
  if (!strcmp(mode_chars.get(), "none")) {
    mode = CacheOverride::CacheOverrideMode::None;
  } else if (!strcmp(mode_chars.get(), "pass")) {
    mode = CacheOverride::CacheOverrideMode::Pass;
  } else if (!strcmp(mode_chars.get(), "override")) {
    mode = CacheOverride::CacheOverrideMode::Override;
  } else {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_CACHE_OVERRIDE_MODE_INVALID,
                              mode_chars.get());
    return false;
  }

  CacheOverride::set_mode(self, mode);
  return true;
}

bool CacheOverride::ttl_get(JSContext *cx, JS::HandleObject self, JS::MutableHandleValue rval) {
  if (self == proto_obj) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_INVALID_INTERFACE, "ttl get",
                              "CacheOverride");
    return false;
  }
  rval.set(CacheOverride::ttl(self));
  return true;
}

bool CacheOverride::ttl_set(JSContext *cx, JS::HandleObject self, JS::HandleValue val,
                            JS::MutableHandleValue rval) {
  if (self == proto_obj) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_INVALID_INTERFACE, "ttl set",
                              "CacheOverride");
    return false;
  }
  if (!CacheOverride::ensure_override(cx, self, "a TTL"))
    return false;

  if (val.isUndefined()) {
    JS::SetReservedSlot(self, CacheOverride::Slots::TTL, val);
  } else {
    int32_t ttl;
    if (!JS::ToInt32(cx, val, &ttl))
      return false;

    CacheOverride::set_ttl(self, ttl);
  }
  rval.set(CacheOverride::ttl(self));
  return true;
}

bool CacheOverride::swr_get(JSContext *cx, JS::HandleObject self, JS::MutableHandleValue rval) {
  if (self == proto_obj) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_INVALID_INTERFACE, "swr get",
                              "CacheOverride");
    return false;
  }
  rval.set(CacheOverride::swr(self));
  return true;
}

bool CacheOverride::swr_set(JSContext *cx, JS::HandleObject self, JS::HandleValue val,
                            JS::MutableHandleValue rval) {
  if (self == proto_obj) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_INVALID_INTERFACE, "swr set",
                              "CacheOverride");
    return false;
  }
  if (!CacheOverride::ensure_override(cx, self, "SWR"))
    return false;

  if (val.isUndefined()) {
    JS::SetReservedSlot(self, CacheOverride::Slots::SWR, val);
  } else {
    int32_t swr;
    if (!JS::ToInt32(cx, val, &swr))
      return false;

    CacheOverride::set_swr(self, swr);
  }
  rval.set(CacheOverride::swr(self));
  return true;
}

bool CacheOverride::surrogate_key_get(JSContext *cx, JS::HandleObject self,
                                      JS::MutableHandleValue rval) {
  if (self == proto_obj) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_INVALID_INTERFACE,
                              "surrogate_key get", "CacheOverride");
    return false;
  }
  rval.set(CacheOverride::surrogate_key(self));
  return true;
}

bool CacheOverride::surrogate_key_set(JSContext *cx, JS::HandleObject self, JS::HandleValue val,
                                      JS::MutableHandleValue rval) {
  if (self == proto_obj) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_INVALID_INTERFACE,
                              "surrogate_key set", "CacheOverride");
    return false;
  }
  if (!CacheOverride::ensure_override(cx, self, "a surrogate key"))
    return false;

  if (val.isUndefined()) {
    JS::SetReservedSlot(self, CacheOverride::Slots::SurrogateKey, val);
  } else {
    JS::RootedString surrogate_key(cx, JS::ToString(cx, val));
    if (!surrogate_key)
      return false;

    CacheOverride::set_surrogate_key(self, surrogate_key);
  }
  rval.set(CacheOverride::surrogate_key(self));
  return true;
}

bool CacheOverride::pci_get(JSContext *cx, JS::HandleObject self, JS::MutableHandleValue rval) {
  if (self == proto_obj) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_INVALID_INTERFACE, "pci get",
                              "CacheOverride");
    return false;
  }
  rval.set(CacheOverride::pci(self));
  return true;
}

bool CacheOverride::pci_set(JSContext *cx, JS::HandleObject self, JS::HandleValue val,
                            JS::MutableHandleValue rval) {
  if (self == proto_obj) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_INVALID_INTERFACE, "pci set",
                              "CacheOverride");
    return false;
  }
  if (!CacheOverride::ensure_override(cx, self, "PCI"))
    return false;

  if (val.isUndefined()) {
    JS::SetReservedSlot(self, CacheOverride::Slots::PCI, val);
  } else {
    bool pci = JS::ToBoolean(val);
    CacheOverride::set_pci(self, pci);
  }
  rval.set(CacheOverride::pci(self));
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
    JS_PSGS("swr", accessor_get<swr_get>, accessor_set<swr_set>, JSPROP_ENUMERATE),
    JS_PSGS("surrogateKey", accessor_get<surrogate_key_get>, accessor_set<surrogate_key_set>,
            JSPROP_ENUMERATE),
    JS_PSGS("pci", accessor_get<pci_get>, accessor_set<pci_set>, JSPROP_ENUMERATE),
    JS_STRING_SYM_PS(toStringTag, "CacheOverride", JSPROP_READONLY),
    JS_PS_END};

bool CacheOverride::constructor(JSContext *cx, unsigned argc, JS::Value *vp) {
  CTOR_HEADER("CacheOverride", 1);

  JS::RootedObject self(cx, JS_NewObjectForConstructor(cx, &class_, args));

  JS::RootedValue val(cx);
  if (!mode_set(cx, self, args[0], &val))
    return false;

  if (CacheOverride::mode(self) == CacheOverride::CacheOverrideMode::Override) {
    if (!args.get(1).isObject()) {
      JS_ReportErrorUTF8(cx, "Creating a CacheOverride object with mode \"override\" requires "
                             "an init object for the override parameters as the second argument");
      return false;
    }

    JS::RootedObject override_init(cx, &args[1].toObject());

    if (!JS_GetProperty(cx, override_init, "ttl", &val) || !ttl_set(cx, self, val, &val)) {
      return false;
    }

    if (!JS_GetProperty(cx, override_init, "swr", &val) || !swr_set(cx, self, val, &val)) {
      return false;
    }

    if (!JS_GetProperty(cx, override_init, "surrogateKey", &val) ||
        !surrogate_key_set(cx, self, val, &val)) {
      return false;
    }

    if (!JS_GetProperty(cx, override_init, "pci", &val) || !pci_set(cx, self, val, &val)) {
      return false;
    }
  }

  args.rval().setObject(*self);
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
    MOZ_ASSERT(!val.isObject());
    JS::SetReservedSlot(result, i, val);
  }

  return result;
}

bool CacheOverride::init_class(JSContext *cx, JS::HandleObject global) {
  return BuiltinImpl<CacheOverride>::init_class_impl(cx, global);
}

} // namespace builtins
