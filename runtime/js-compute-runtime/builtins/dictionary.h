#ifndef JS_COMPUTE_RUNTIME_DICTIONARY_H
#define JS_COMPUTE_RUNTIME_DICTIONARY_H

#include "builtin.h"
#include "js-compute-builtins.h"

namespace builtins {

class Dictionary : public BuiltinImpl<Dictionary> {
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

  static fastly_dictionary_handle_t dictionary_handle(JSObject *obj);
  static bool constructor(JSContext *cx, unsigned argc, JS::Value *vp);

  static bool init_class(JSContext *cx, JS::HandleObject global);
};

} // namespace builtins

#endif
