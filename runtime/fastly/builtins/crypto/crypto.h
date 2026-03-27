#ifndef BUILTINS_WEB_FASTLY_CRYPTO_H
#define BUILTINS_WEB_FASTLY_CRYPTO_H

#include "builtin.h"

namespace fastly::crypto {

class Crypto : public builtins::BuiltinNoConstructor<Crypto> {
private:
public:
  static constexpr const char *class_name = "Crypto";
  static const int ctor_length = 0;

  static JS::PersistentRooted<JSObject *> subtle;

  enum Slots { Count };
  static const JSFunctionSpec static_methods[];
  static const JSPropertySpec static_properties[];
  static const JSFunctionSpec methods[];
  static const JSPropertySpec properties[];

  static bool subtle_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool get_random_values(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool random_uuid(JSContext *cx, unsigned argc, JS::Value *vp);

  static bool init_class(JSContext *cx, JS::HandleObject global);
};

bool install(api::Engine *engine);

} // namespace fastly::crypto

#endif
