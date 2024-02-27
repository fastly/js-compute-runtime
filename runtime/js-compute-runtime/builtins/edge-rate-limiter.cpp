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


// increment(entry: string, delta: number): void;
bool RateCounter::increment(JSContext *cx, unsigned argc, JS::Value *vp) {
  REQUEST_HANDLER_ONLY("The RateCounter builtin");
  METHOD_HEADER(2);

  // Convert entry parameter into a string
  auto entry = core::encode(cx, args.get(0));
  if (!entry) {
    return false;
  }

  // Convert delta parameter into a number
  double delta;
  if (!JS::ToNumber(cx, args.get(1), &delta)) {
    return false;
  }

  MOZ_ASSERT(JS::GetReservedSlot(self, Slots::Name).isString());
  JS::RootedValue name_val(cx, JS::GetReservedSlot(self, Slots::Name).toString());
  auto name = core::encode(cx, name_val);
  if (!name) {
    return false;
  }

  auto rc = host_api::RateCounter(name);

  auto res = rc.increment(entry, delta);
  if (auto *err = res.to_err()) {
    HANDLE_ERROR(cx, *err);
    return false;
  }

  args.rval().setUndefined();
  return true;
}

// lookupRate(entry: string, window: [1, 10, 60]): number;
bool RateCounter::lookupRate(JSContext *cx, unsigned argc, JS::Value *vp) {
  REQUEST_HANDLER_ONLY("The RateCounter builtin");
  METHOD_HEADER(1);

  // Convert entry parameter into a string
  auto entry = core::encode(cx, args.get(0));
  if (!entry) {
    return false;
  }

  // Convert window parameter into a number
  double window;
  if (!JS::ToNumber(cx, args.get(1), &window)) {
    return false;
  }

  if (window != 1 || window != 10 || window != 60) {
    JS_ReportErrorASCII(cx, "window parameter must be either: 1, 10, or 60");
    return false;
  }

  MOZ_ASSERT(JS::GetReservedSlot(self, Slots::Name).isString());
  JS::RootedValue name_val(cx, JS::GetReservedSlot(self, Slots::Name).toString());
  auto name = core::encode(cx, name_val);
  if (!name) {
    return false;
  }

  auto rc = host_api::RateCounter(name);

  auto res = rc.lookup_rate(entry, window);
  if (auto *err = res.to_err()) {
    HANDLE_ERROR(cx, *err);
    return false;
  }

  JS::RootedValue rate(cx, JS::NumberValue(res.unwrap()));
  args.rval().set(rate);
  return true;
}

// lookupCount(entry: string, duration: [10, 20, 30, 40, 50, 60]): number;
bool RateCounter::lookupCount(JSContext *cx, unsigned argc, JS::Value *vp) {
  REQUEST_HANDLER_ONLY("The RateCounter builtin");
  METHOD_HEADER(1);

  // Convert entry parameter into a string
  auto entry = core::encode(cx, args.get(0));
  if (!entry) {
    return false;
  }

  // Convert duration parameter into a number
  double duration;
  if (!JS::ToNumber(cx, args.get(1), &duration)) {
    return false;
  }

  if (duration != 10 || duration != 20 || duration != 30 || duration != 40 || duration != 50 || duration != 60) {
    JS_ReportErrorASCII(cx, "duration parameter must be either: 10, 20, 30, 40, 50, or 60");
    return false;
  }

  MOZ_ASSERT(JS::GetReservedSlot(self, Slots::Name).isString());
  JS::RootedValue name_val(cx, JS::GetReservedSlot(self, Slots::Name).toString());
  auto name = core::encode(cx, name_val);
  if (!name) {
    return false;
  }

  auto rc = host_api::RateCounter(name);

  auto res = rc.lookup_count(entry, duration);
  if (auto *err = res.to_err()) {
    HANDLE_ERROR(cx, *err);
    return false;
  }

  JS::RootedValue rate(cx, JS::NumberValue(res.unwrap()));
  args.rval().set(rate);
  return true;
}

const JSFunctionSpec RateCounter::static_methods[] = {
    JS_FN("increment", increment, 2, JSPROP_ENUMERATE),
    JS_FN("lookupRate", lookupRate, 2, JSPROP_ENUMERATE),
    JS_FN("lookupCount", lookupCount, 2, JSPROP_ENUMERATE),
    JS_FS_END,
};

const JSPropertySpec RateCounter::static_properties[] = {
    JS_PS_END,
};

const JSFunctionSpec RateCounter::methods[] = {JS_FS_END};

const JSPropertySpec RateCounter::properties[] = {
    JS_STRING_SYM_PS(toStringTag, "RateCounter", JSPROP_READONLY), JS_PS_END};

// Open a RateCounter instance with the given name
// constructor(name: string);
bool RateCounter::constructor(JSContext *cx, unsigned argc, JS::Value *vp) {
  REQUEST_HANDLER_ONLY("The RateCounter builtin");
  CTOR_HEADER("RateCounter", 1);
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

bool RateCounter::init_class(JSContext *cx, JS::HandleObject global) {
  return BuiltinImpl<RateCounter>::init_class_impl(cx, global);
}

} // namespace builtins
