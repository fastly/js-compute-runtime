#ifndef FASTLY_ACL_H
#define FASTLY_ACL_H

#include "builtin.h"
#include "extension-api.h"

namespace fastly::acl {

class Acl : public builtins::BuiltinNoConstructor<Acl> {
private:
public:
  static constexpr const char *class_name = "Acl";
  static const int ctor_length = 1;
  enum Slots { HostAcl, Count };

  static const JSFunctionSpec static_methods[];
  static const JSPropertySpec static_properties[];

  static const JSFunctionSpec methods[];
  static const JSPropertySpec properties[];

  static bool open(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool lookup(JSContext *cx, unsigned argc, JS::Value *vp);
};

} // namespace fastly::acl

#endif
