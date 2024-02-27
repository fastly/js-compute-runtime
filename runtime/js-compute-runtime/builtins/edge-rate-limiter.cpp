#include "edge-rate-limiter.h"
#include "builtin.h"
#include "core/encode.h"
#include "host_interface/host_api.h"
#include "js-compute-builtins.h"
#include "js/Result.h"
#include <tuple>

namespace builtins {
// add(entry: string, timeToLive: number): void;
bool PenaltyBox::add(JSContext *cx, unsigned argc, JS::Value *vp) {
  REQUEST_HANDLER_ONLY("The PenaltyBox builtin");
  METHOD_HEADER(2);

  // Convert entry parameter into a string
  auto entry = core::encode(cx, args.get(0));
  if (!entry) {
    return false;
  }

  // Convert timeToLive parameter into a number
  double timeToLive;
  if (!JS::ToNumber(cx, args.get(1), &timeToLive)) {
    return false;
  }

  MOZ_ASSERT(JS::GetReservedSlot(self, Slots::Name).isString());
  JS::RootedValue name_val(cx, JS::GetReservedSlot(self, Slots::Name).toString());
  auto name = core::encode(cx, name_val);
  if (!name) {
    return false;
  }

  auto pb = host_api::PenaltyBox(name);

  auto res = pb.add(entry, timeToLive);
  if (auto *err = res.to_err()) {
    HANDLE_ERROR(cx, *err);
    return false;
  }

  args.rval().setUndefined();
  return true;
}

// has(entry: string): boolean;
bool PenaltyBox::has(JSContext *cx, unsigned argc, JS::Value *vp) {
  REQUEST_HANDLER_ONLY("The PenaltyBox builtin");
  METHOD_HEADER(1);

  // Convert entry parameter into a string
  auto entry = core::encode(cx, args.get(0));
  if (!entry) {
    return false;
  }

  MOZ_ASSERT(JS::GetReservedSlot(self, Slots::Name).isString());
  JS::RootedValue name_val(cx, JS::GetReservedSlot(self, Slots::Name).toString());
  auto name = core::encode(cx, name_val);
  if (!name) {
    return false;
  }

  auto pb = host_api::PenaltyBox(name);

  auto res = pb.has(entry);
  if (auto *err = res.to_err()) {
    HANDLE_ERROR(cx, *err);
    return false;
  }

  args.rval().setBoolean(res.unwrap());
  return true;
}

const JSFunctionSpec PenaltyBox::static_methods[] = {
    JS_FN("add", add, 2, JSPROP_ENUMERATE),
    JS_FN("has", has, 1, JSPROP_ENUMERATE),
    JS_FS_END,
};

const JSPropertySpec PenaltyBox::static_properties[] = {
    JS_PS_END,
};

const JSFunctionSpec PenaltyBox::methods[] = {JS_FS_END};

const JSPropertySpec PenaltyBox::properties[] = {
    JS_STRING_SYM_PS(toStringTag, "PenaltyBox", JSPROP_READONLY), JS_PS_END};

// Open a penalty-box identified by the given name
// constructor(name: string);
bool PenaltyBox::constructor(JSContext *cx, unsigned argc, JS::Value *vp) {
  REQUEST_HANDLER_ONLY("The PenaltyBox builtin");
  CTOR_HEADER("PenaltyBox", 1);
  auto name = core::encode(cx, args.get(0));
  if (!name) {
    return false;
  }

  // TODO: Do we want to check if the string is empty?

  JS::RootedObject instance(cx, JS_NewObjectForConstructor(cx, &class_, args));
  if (!instance) {
    return false;
  }
  JS::SetReservedSlot(instance, static_cast<uint32_t>(Slots::Name),
                      JS::StringValue(JS_NewStringCopyZ(cx, name.begin())));
  args.rval().setObject(*instance);
  return true;
}

bool PenaltyBox::init_class(JSContext *cx, JS::HandleObject global) {
  return BuiltinImpl<PenaltyBox>::init_class_impl(cx, global);
}

} // namespace builtins
