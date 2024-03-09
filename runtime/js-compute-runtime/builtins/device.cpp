#include "device.h"
#include "builtin.h"
#include "core/encode.h"
#include "host_interface/host_api.h"
#include "js-compute-builtins.h"
#include "js/JSON.h"

namespace builtins {

namespace {
bool callbackCalled;
bool write_json_to_buf(const char16_t *str, uint32_t strlen, void *out) {
  callbackCalled = true;
  auto outstr = static_cast<std::u16string *>(out);
  outstr->append(str, strlen);

  return true;
}
JSObject *deviceToJSON(JSContext *cx, JS::HandleObject self) {
  MOZ_ASSERT(Device::is_instance(self));

  JS::RootedValue device_info(
      cx, JS::GetReservedSlot(self, static_cast<uint32_t>(Device::Slots::DeviceInfo)));
  JS::RootedValue value(cx);
  if (!device_info.isObject()) {
    return nullptr;
  }

  JS::RootedObject device_info_obj(cx, device_info.toObjectOrNull());

  JS::RootedObject result(cx, JS_NewPlainObject(cx));

  if (!JS_GetProperty(cx, device_info_obj, "name", &value)) {
    return nullptr;
  }
  MOZ_ASSERT(value.isString() || value.isNullOrUndefined());
  if (value.isUndefined()) {
    value.setNull();
  }
  if (!JS_SetProperty(cx, result, "name", value)) {
    return nullptr;
  }

  if (!JS_GetProperty(cx, device_info_obj, "brand", &value)) {
    return nullptr;
  }
  MOZ_ASSERT(value.isString() || value.isNullOrUndefined());
  if (value.isUndefined()) {
    value.setNull();
  }
  if (!JS_SetProperty(cx, result, "brand", value)) {
    return nullptr;
  }

  if (!JS_GetProperty(cx, device_info_obj, "model", &value)) {
    return nullptr;
  }
  MOZ_ASSERT(value.isString() || value.isNullOrUndefined());
  if (value.isUndefined()) {
    value.setNull();
  }
  if (!JS_SetProperty(cx, result, "model", value)) {
    return nullptr;
  }

  if (!JS_GetProperty(cx, device_info_obj, "hardwareType", &value)) {
    return nullptr;
  }
  MOZ_ASSERT(value.isString() || value.isNullOrUndefined());
  if (value.isUndefined()) {
    value.setNull();
  }
  if (!JS_SetProperty(cx, result, "hardwareType", value)) {
    return nullptr;
  }

  if (!JS_GetProperty(cx, device_info_obj, "isDesktop", &value)) {
    return nullptr;
  }
  MOZ_ASSERT(value.isBoolean() || value.isNullOrUndefined());
  if (value.isUndefined()) {
    value.setNull();
  }
  if (!JS_SetProperty(cx, result, "isDesktop", value)) {
    return nullptr;
  }

  if (!JS_GetProperty(cx, device_info_obj, "isGameConsole", &value)) {
    return nullptr;
  }
  MOZ_ASSERT(value.isBoolean() || value.isNullOrUndefined());
  if (value.isUndefined()) {
    value.setNull();
  }
  if (!JS_SetProperty(cx, result, "isGameConsole", value)) {
    return nullptr;
  }

  if (!JS_GetProperty(cx, device_info_obj, "isMediaPlayer", &value)) {
    return nullptr;
  }
  MOZ_ASSERT(value.isBoolean() || value.isNullOrUndefined());
  if (value.isUndefined()) {
    value.setNull();
  }
  if (!JS_SetProperty(cx, result, "isMediaPlayer", value)) {
    return nullptr;
  }

  if (!JS_GetProperty(cx, device_info_obj, "isMobile", &value)) {
    return nullptr;
  }
  MOZ_ASSERT(value.isBoolean() || value.isNullOrUndefined());
  if (value.isUndefined()) {
    value.setNull();
  }
  if (!JS_SetProperty(cx, result, "isMobile", value)) {
    return nullptr;
  }

  if (!JS_GetProperty(cx, device_info_obj, "isSmartTV", &value)) {
    return nullptr;
  }
  MOZ_ASSERT(value.isBoolean() || value.isNullOrUndefined());
  if (value.isUndefined()) {
    value.setNull();
  }
  if (!JS_SetProperty(cx, result, "isSmartTV", value)) {
    return nullptr;
  }

  if (!JS_GetProperty(cx, device_info_obj, "isTablet", &value)) {
    return nullptr;
  }
  MOZ_ASSERT(value.isBoolean() || value.isNullOrUndefined());
  if (value.isUndefined()) {
    value.setNull();
  }
  if (!JS_SetProperty(cx, result, "isTablet", value)) {
    return nullptr;
  }

  if (!JS_GetProperty(cx, device_info_obj, "isTouchscreen", &value)) {
    return nullptr;
  }
  MOZ_ASSERT(value.isBoolean() || value.isNullOrUndefined());
  if (value.isUndefined()) {
    value.setNull();
  }
  if (!JS_SetProperty(cx, result, "isTouchscreen", value)) {
    return nullptr;
  }

  return result;
}

} // namespace

/*
 * This is used by our `Console` implementation and logs all the approproiate properties for a
 * Device instance
 */
JSString *Device::ToSource(JSContext *cx, JS::HandleObject self) {
  MOZ_ASSERT(Device::is_instance(self));
  JS::RootedValue data(cx);
  data.setObjectOrNull(deviceToJSON(cx, self));
  JS::RootedObject replacer(cx);
  JS::RootedValue space(cx);

  std::u16string out;
  // 1. Let bytes the result of running serialize a JavaScript value to JSON bytes on data.
  callbackCalled = false;
  if (!JS::ToJSON(cx, data, replacer, space, &write_json_to_buf, &out)) {
    return nullptr;
  }
  if (!callbackCalled) {
    JS_ReportErrorASCII(cx, "The data is not JSON serializable");
    return nullptr;
  }

  return JS_NewUCStringCopyN(cx, out.c_str(), out.length());
}

bool Device::toJSON(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0)

  args.rval().setObjectOrNull(deviceToJSON(cx, self));

  return true;
}

// get name(): string | null;
bool Device::device_name_get(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0)

  JS::RootedValue device_info(cx,
                              JS::GetReservedSlot(self, static_cast<uint32_t>(Slots::DeviceInfo)));
  if (!device_info.isObject()) {
    args.rval().setNull();
    return true;
  }
  JS::RootedObject device_info_obj(cx, device_info.toObjectOrNull());

  JS::RootedValue device_name(cx);
  if (!JS_GetProperty(cx, device_info_obj, "name", &device_name)) {
    return false;
  }
  MOZ_ASSERT(device_name.isString() || device_name.isNullOrUndefined());
  if (device_name.isUndefined()) {
    args.rval().setNull();
  } else {
    args.rval().set(device_name);
  }
  return true;
}

// get brand(): string | null;
bool Device::brand_get(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0)

  JS::RootedValue device_info(cx,
                              JS::GetReservedSlot(self, static_cast<uint32_t>(Slots::DeviceInfo)));
  if (!device_info.isObject()) {
    args.rval().setNull();
    return true;
  }
  JS::RootedObject device_info_obj(cx, device_info.toObjectOrNull());

  JS::RootedValue device_brand(cx);
  if (!JS_GetProperty(cx, device_info_obj, "brand", &device_brand)) {
    return false;
  }
  MOZ_ASSERT(device_brand.isString() || device_brand.isNullOrUndefined());
  if (device_brand.isUndefined()) {
    args.rval().setNull();
  } else {
    args.rval().set(device_brand);
  }
  return true;
}

// get model(): string | null;
bool Device::model_get(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0)

  JS::RootedValue device_info(cx,
                              JS::GetReservedSlot(self, static_cast<uint32_t>(Slots::DeviceInfo)));
  if (!device_info.isObject()) {
    args.rval().setNull();
    return true;
  }
  JS::RootedObject device_info_obj(cx, device_info.toObjectOrNull());

  JS::RootedValue device_model(cx);
  if (!JS_GetProperty(cx, device_info_obj, "model", &device_model)) {
    return false;
  }
  MOZ_ASSERT(device_model.isString() || device_model.isNullOrUndefined());
  if (device_model.isUndefined()) {
    args.rval().setNull();
  } else {
    args.rval().set(device_model);
  }
  return true;
}

// get hardwareType(): string | null;
bool Device::hardware_type_get(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0)

  JS::RootedValue device_info(cx,
                              JS::GetReservedSlot(self, static_cast<uint32_t>(Slots::DeviceInfo)));
  if (!device_info.isObject()) {
    args.rval().setNull();
    return true;
  }
  JS::RootedObject device_info_obj(cx, device_info.toObjectOrNull());

  JS::RootedValue device_hwtype(cx);
  if (!JS_GetProperty(cx, device_info_obj, "hwtype", &device_hwtype)) {
    return false;
  }
  MOZ_ASSERT(device_hwtype.isString() || device_hwtype.isNullOrUndefined());
  if (device_hwtype.isUndefined()) {
    args.rval().setNull();
  } else {
    args.rval().set(device_hwtype);
  }
  return true;
}

// get isDesktop(): boolean | null;
bool Device::is_desktop_get(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0)

  JS::RootedValue device_info(cx,
                              JS::GetReservedSlot(self, static_cast<uint32_t>(Slots::DeviceInfo)));
  if (!device_info.isObject()) {
    args.rval().setBoolean(false);
    return true;
  }
  JS::RootedObject device_info_obj(cx, device_info.toObjectOrNull());

  JS::RootedValue device_is_desktop(cx);
  if (!JS_GetProperty(cx, device_info_obj, "is_desktop", &device_is_desktop)) {
    return false;
  }
  MOZ_ASSERT(device_is_desktop.isBoolean() || device_is_desktop.isNullOrUndefined());
  if (device_is_desktop.isUndefined()) {
    args.rval().setNull();
  } else {
    args.rval().set(device_is_desktop);
  }
  return true;
}

// get isGameConsole(): boolean | null;
bool Device::is_gameconsole_get(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0)

  JS::RootedValue device_info(cx,
                              JS::GetReservedSlot(self, static_cast<uint32_t>(Slots::DeviceInfo)));
  if (!device_info.isObject()) {
    args.rval().setBoolean(false);
    return true;
  }
  JS::RootedObject device_info_obj(cx, device_info.toObjectOrNull());

  JS::RootedValue device_is_gameconsole(cx);
  if (!JS_GetProperty(cx, device_info_obj, "is_gameconsole", &device_is_gameconsole)) {
    return false;
  }
  MOZ_ASSERT(device_is_gameconsole.isBoolean() || device_is_gameconsole.isNullOrUndefined());
  if (device_is_gameconsole.isUndefined()) {
    args.rval().setNull();
  } else {
    args.rval().set(device_is_gameconsole);
  }
  return true;
}

// get isMediaPlayer(): boolean | null;
bool Device::is_mediaplayer_get(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0)

  JS::RootedValue device_info(cx,
                              JS::GetReservedSlot(self, static_cast<uint32_t>(Slots::DeviceInfo)));
  if (!device_info.isObject()) {
    args.rval().setBoolean(false);
    return true;
  }
  JS::RootedObject device_info_obj(cx, device_info.toObjectOrNull());

  JS::RootedValue device_is_mediaplayer(cx);
  if (!JS_GetProperty(cx, device_info_obj, "is_mediaplayer", &device_is_mediaplayer)) {
    return false;
  }
  MOZ_ASSERT(device_is_mediaplayer.isBoolean() || device_is_mediaplayer.isNullOrUndefined());
  if (device_is_mediaplayer.isUndefined()) {
    args.rval().setNull();
  } else {
    args.rval().set(device_is_mediaplayer);
  }
  return true;
}

// get isMobile(): boolean | null;
bool Device::is_mobile_get(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0)

  JS::RootedValue device_info(cx,
                              JS::GetReservedSlot(self, static_cast<uint32_t>(Slots::DeviceInfo)));
  if (!device_info.isObject()) {
    args.rval().setBoolean(false);
    return true;
  }
  JS::RootedObject device_info_obj(cx, device_info.toObjectOrNull());

  JS::RootedValue device_is_mobile(cx);
  if (!JS_GetProperty(cx, device_info_obj, "is_mobile", &device_is_mobile)) {
    return false;
  }
  MOZ_ASSERT(device_is_mobile.isBoolean() || device_is_mobile.isNullOrUndefined());
  if (device_is_mobile.isUndefined()) {
    args.rval().setNull();
  } else {
    args.rval().set(device_is_mobile);
  }
  return true;
}

// get isSmartTV(): boolean | null;
bool Device::is_smarttv_get(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0)

  JS::RootedValue device_info(cx,
                              JS::GetReservedSlot(self, static_cast<uint32_t>(Slots::DeviceInfo)));
  if (!device_info.isObject()) {
    args.rval().setBoolean(false);
    return true;
  }
  JS::RootedObject device_info_obj(cx, device_info.toObjectOrNull());

  JS::RootedValue device_is_smarttv(cx);
  if (!JS_GetProperty(cx, device_info_obj, "is_smarttv", &device_is_smarttv)) {
    return false;
  }
  MOZ_ASSERT(device_is_smarttv.isBoolean() || device_is_smarttv.isNullOrUndefined());
  if (device_is_smarttv.isUndefined()) {
    args.rval().setNull();
  } else {
    args.rval().set(device_is_smarttv);
  }
  return true;
}

// get isTablet(): boolean | null;
bool Device::is_tablet_get(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0)

  JS::RootedValue device_info(cx,
                              JS::GetReservedSlot(self, static_cast<uint32_t>(Slots::DeviceInfo)));
  if (!device_info.isObject()) {
    args.rval().setBoolean(false);
    return true;
  }
  JS::RootedObject device_info_obj(cx, device_info.toObjectOrNull());

  JS::RootedValue device_is_tablet(cx);
  if (!JS_GetProperty(cx, device_info_obj, "is_tablet", &device_is_tablet)) {
    return false;
  }
  MOZ_ASSERT(device_is_tablet.isBoolean() || device_is_tablet.isNullOrUndefined());
  if (device_is_tablet.isUndefined()) {
    args.rval().setNull();
  } else {
    args.rval().set(device_is_tablet);
  }
  return true;
}

// get isTouchscreen(): boolean | null;
bool Device::is_touchscreen_get(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0)

  JS::RootedValue device_info(cx,
                              JS::GetReservedSlot(self, static_cast<uint32_t>(Slots::DeviceInfo)));
  if (!device_info.isObject()) {
    args.rval().setBoolean(false);
    return true;
  }
  JS::RootedObject device_info_obj(cx, device_info.toObjectOrNull());

  JS::RootedValue device_is_touchscreen(cx);
  if (!JS_GetProperty(cx, device_info_obj, "is_touchscreen", &device_is_touchscreen)) {
    return false;
  }
  MOZ_ASSERT(device_is_touchscreen.isBoolean() || device_is_touchscreen.isNullOrUndefined());
  if (device_is_touchscreen.isUndefined()) {
    args.rval().setNull();
  } else {
    args.rval().set(device_is_touchscreen);
  }
  return true;
}

// static lookup(useragent: string): Device;
bool Device::lookup(JSContext *cx, unsigned argc, JS::Value *vp) {
  REQUEST_HANDLER_ONLY("The Device builtin");
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  if (!args.requireAtLeast(cx, "Device.lookup", 1)) {
    return false;
  }

  // Convert key parameter into a string and check the value adheres to our validation rules.
  auto key = core::encode(cx, args[0]);
  if (!key) {
    return false;
  }

  if (key.len == 0) {
    JS_ReportErrorASCII(cx, "Device.lookup: useragent parameter can not be an empty string");
    return false;
  }

  auto lookup_res = host_api::DeviceDetection::lookup(key);
  if (auto *err = lookup_res.to_err()) {
    if (host_api::error_is_optional_none(*err)) {
      args.rval().setNull();
      return true;
    }
    HANDLE_ERROR(cx, *err);
    return false;
  }
  auto result = std::move(lookup_res.unwrap());

  JS::RootedString device_info_str(cx, JS_NewStringCopyN(cx, result.ptr.release(), result.len));
  if (!device_info_str) {
    return false;
  }

  JS::RootedValue device_info(cx);

  if (!JS_ParseJSON(cx, device_info_str, &device_info)) {
    return false;
  }

  MOZ_ASSERT(device_info.isObject());

  JS::RootedObject device_info_obj(cx, device_info.toObjectOrNull());

  args.rval().setObjectOrNull(Device::create(cx, device_info_obj));

  return true;
}

const JSFunctionSpec Device::static_methods[] = {
    JS_FN("lookup", lookup, 1, JSPROP_ENUMERATE),
    JS_FS_END,
};

const JSPropertySpec Device::static_properties[] = {
    JS_PS_END,
};

const JSFunctionSpec Device::methods[] = {JS_FN("toJSON", toJSON, 0, JSPROP_ENUMERATE), JS_FS_END};

const JSPropertySpec Device::properties[] = {
    JS_PSG("name", Device::device_name_get, JSPROP_ENUMERATE),
    JS_PSG("brand", Device::brand_get, JSPROP_ENUMERATE),
    JS_PSG("model", Device::model_get, JSPROP_ENUMERATE),
    JS_PSG("hardwareType", Device::hardware_type_get, JSPROP_ENUMERATE),
    JS_PSG("isDesktop", Device::is_desktop_get, JSPROP_ENUMERATE),
    JS_PSG("isGameConsole", Device::is_gameconsole_get, JSPROP_ENUMERATE),
    JS_PSG("isMediaPlayer", Device::is_mediaplayer_get, JSPROP_ENUMERATE),
    JS_PSG("isMobile", Device::is_mobile_get, JSPROP_ENUMERATE),
    JS_PSG("isSmartTV", Device::is_smarttv_get, JSPROP_ENUMERATE),
    JS_PSG("isTablet", Device::is_tablet_get, JSPROP_ENUMERATE),
    JS_PSG("isTouchscreen", Device::is_touchscreen_get, JSPROP_ENUMERATE),
    JS_STRING_SYM_PS(toStringTag, "Device", JSPROP_READONLY),
    JS_PS_END};

bool Device::constructor(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS_ReportErrorNumberASCII(cx, GetErrorMessageBuiltin, nullptr, JSMSG_ILLEGAL_CTOR);
  return false;
}

bool Device::init_class(JSContext *cx, JS::HandleObject global) {
  return BuiltinImpl<Device>::init_class_impl(cx, global);
}

JSObject *Device::create(JSContext *cx, JS::HandleObject device_info) {
  JS::RootedObject instance(cx, JS_NewObjectWithGivenProto(cx, &Device::class_, Device::proto_obj));

  JS::RootedValue device(cx);
  if (!JS_GetProperty(cx, device_info, "device", &device)) {
    return nullptr;
  }
  MOZ_ASSERT(device.isObject());

  JS::SetReservedSlot(instance, static_cast<uint32_t>(Slots::DeviceInfo), device);

  return instance;
}

} // namespace builtins
