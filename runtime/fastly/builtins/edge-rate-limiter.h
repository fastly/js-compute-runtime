#ifndef FASTLY_EDGE_RATE_LIMITER_H
#define FASTLY_EDGE_RATE_LIMITER_H

#include "builtin.h"
#include "extension-api.h"

namespace fastly::edge_rate_limiter {

class PenaltyBox final : public builtins::BuiltinImpl<PenaltyBox> {
  static bool add(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool has(JSContext *cx, unsigned argc, JS::Value *vp);

public:
  static constexpr const char *class_name = "PenaltyBox";
  enum Slots { Name, Count };
  static const JSFunctionSpec static_methods[];
  static const JSPropertySpec static_properties[];
  static const JSFunctionSpec methods[];
  static const JSPropertySpec properties[];

  static const unsigned ctor_length = 0;

  static bool constructor(JSContext *cx, unsigned argc, JS::Value *vp);

  static JSString *get_name(JSObject *self);
};

class RateCounter final : public builtins::BuiltinImpl<RateCounter> {
  static bool increment(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool lookupRate(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool lookupCount(JSContext *cx, unsigned argc, JS::Value *vp);

public:
  static constexpr const char *class_name = "RateCounter";
  enum Slots { Name, Count };
  static const JSFunctionSpec static_methods[];
  static const JSPropertySpec static_properties[];
  static const JSFunctionSpec methods[];
  static const JSPropertySpec properties[];

  static const unsigned ctor_length = 0;

  static bool constructor(JSContext *cx, unsigned argc, JS::Value *vp);

  static JSString *get_name(JSObject *self);
};

class EdgeRateLimiter final : public builtins::BuiltinImpl<EdgeRateLimiter> {
  static bool checkRate(JSContext *cx, unsigned argc, JS::Value *vp);

public:
  static constexpr const char *class_name = "EdgeRateLimiter";
  enum Slots { RateCounterName, PenaltyBoxName, Count };
  static const JSFunctionSpec static_methods[];
  static const JSPropertySpec static_properties[];
  static const JSFunctionSpec methods[];
  static const JSPropertySpec properties[];

  static const unsigned ctor_length = 0;

  static bool constructor(JSContext *cx, unsigned argc, JS::Value *vp);
};

} // namespace fastly::edge_rate_limiter

#endif
