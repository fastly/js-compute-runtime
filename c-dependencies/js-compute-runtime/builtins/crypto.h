#ifndef JS_COMPUTE_RUNTIME_BUILTIN_CRYPTO_H
#define JS_COMPUTE_RUNTIME_BUILTIN_CRYPTO_H

#include "builtin.h"

namespace builtins {

class Crypto : public BuiltinNoConstructor<Crypto> {
private:
public:
  static constexpr const char *class_name = "Crypto";
  enum Slots { Count };
  static const JSFunctionSpec methods[];
  static const JSPropertySpec properties[];

  static bool get_random_values(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool random_uuid(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool create(JSContext *cx, JS::HandleObject global);
};

} // namespace builtins

#endif
