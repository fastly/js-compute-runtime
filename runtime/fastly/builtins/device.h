#ifndef FASTLY_DEVICE_H
#define FASTLY_DEVICE_H

#include "builtin.h"
#include "extension-api.h"

namespace fastly::device {

class Device : public builtins::BuiltinNoConstructor<Device> {
private:
public:
  static constexpr const char *class_name = "Device";
  static const int ctor_length = 0;
  //   {
  //     "brand": null,
  //     "hwtype": null,
  //     "is_desktop": false,
  //     "is_ereader": false,
  //     "is_gameconsole": false,
  //     "is_mediaplayer": false,
  //     "is_mobile": false,
  //     "is_smarttv": false,
  //     "is_tablet": false,
  //     "is_touchscreen": false,
  //     "is_tvplayer": false,
  //     "model": null,
  //     "name": null
  // }
  enum Slots { DeviceInfo, Count };

  static const JSFunctionSpec static_methods[];
  static const JSPropertySpec static_properties[];
  static const JSFunctionSpec methods[];
  static const JSPropertySpec properties[];

  static bool device_name_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool brand_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool model_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool hardware_type_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool is_gameconsole_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool is_mediaplayer_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool is_mobile_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool is_smarttv_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool is_tablet_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool is_desktop_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool is_touchscreen_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool is_bot_get(JSContext *cx, unsigned argc, JS::Value *vp);

  static bool toJSON(JSContext *cx, unsigned argc, JS::Value *vp);

  static bool lookup(JSContext *cx, unsigned argc, JS::Value *vp);

  static JSObject *create(JSContext *cx, JS::HandleObject deviceInfo);

  static JSString *ToSource(JSContext *cx, JS::HandleObject self);
};

} // namespace fastly::device

#endif
