#ifndef FASTLY_SHIELDING_H
#define FASTLY_SHIELDING_H

#include "../host-api/host_api_fastly.h"
#include "builtin.h"
#include "extension-api.h"

namespace fastly::shielding {

class Shield : public builtins::BuiltinImpl<Shield> {
private:
  static bool backend_for_shield(JSContext *cx, JS::HandleString target,
                                 JS::MutableHandleValue rval);

public:
  static constexpr const char *class_name = "Shield";
  static const int ctor_length = 0;
  enum Slots { IsMe, PlainTarget, SSLTarget, Count };
  static const JSFunctionSpec static_methods[];
  static const JSPropertySpec static_properties[];
  static const JSFunctionSpec methods[];
  static const JSPropertySpec properties[];

  static bool runningOn(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool unencryptedBackend(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool encryptedBackend(JSContext *cx, unsigned argc, JS::Value *vp);

  static bool constructor(JSContext *cx, unsigned argc, JS::Value *vp);
};

} // namespace fastly::shielding

#endif
