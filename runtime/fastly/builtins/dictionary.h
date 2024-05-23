#ifndef FASTLYE_DICTIONARY_H
#define FASTLYE_DICTIONARY_H

#include "../host-api/host_api_fastly.h"
#include "builtin.h"
#include "extension-api.h"

namespace fastly::dictionary {

class Dictionary : public builtins::BuiltinImpl<Dictionary> {
private:
public:
  static constexpr const char *class_name = "Dictionary";
  static const int ctor_length = 1;
  enum Slots { Handle, Count };
  static const JSFunctionSpec static_methods[];
  static const JSPropertySpec static_properties[];
  static const JSFunctionSpec methods[];
  static const JSPropertySpec properties[];

  static bool get(JSContext *cx, unsigned argc, JS::Value *vp);

  static host_api::Dict dictionary_handle(JSObject *obj);
  static bool constructor(JSContext *cx, unsigned argc, JS::Value *vp);
};

} // namespace fastly::dictionary

#endif
