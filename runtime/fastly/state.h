#ifndef FASTLY_STATE_H
#define FASTLY_STATE_H

// Global state management for builtins
// In order to use this, the following things need to be in place:
// - The builtin needs:
// -- A nested RequestState type that holds the global state and defines:
// --- An init member for initializing on sandbox startup
// --- A snapshot member for snapshotting/resetting global state
// -- A static request_state member of type state::RequestStateHolder<RequestState> (defined in state.h)
// - The builtin must be added to the global builtins_with_request_state variable in handler.cpp

#include "host-api/host_api_fastly.h"
#include <type_traits>
#include <tuple>
#include <cstdio>

namespace fastly::state {
// Request state is snapshotted before first request,
// then restored to snapshot between requests
template<typename T>
class RequestStateHolder {
private:
  std::optional<T> state_;
  std::optional<T> snapshot_;

public:
  bool init(JSContext *cx) {
    state_.emplace();
    return state_->init(cx);
  }

  // Snapshot must be called exactly once after init, before any reset
  bool snapshot(JSContext *cx) {
    MOZ_ASSERT(!snapshot_.has_value(), "snapshot() should only be called once");
    snapshot_.emplace();
    return state_->snapshot(cx, *snapshot_);
  }

  // Reset restores state from snapshot (snapshot must have been called)
  bool reset(JSContext *cx) {
    MOZ_ASSERT(snapshot_.has_value(), "reset() called before snapshot()");
    state_.emplace();
    return snapshot_->snapshot(cx, *state_);
  }

  T& get() {
    return *state_;
  }

  const T& get() const {
    return *state_;
  }

  T* operator->() {
    return state_ ? &*state_ : nullptr;
  }

  const T* operator->() const {
    return state_ ? &*state_ : nullptr;
  }
};


// Only used for its type to carry around a template parameter pack
template <class...> struct BuiltinsList{};

class Manager {
public:
  // These functions have a bit of template magic, but its just to call the
  // relevant functions on every member of the parameter pack and ensure
  // that they all return true.
  template <class... Builtins>
  static bool reset_all_request_states(BuiltinsList<Builtins...>, JSContext* cx) {
    // Look up "fold expressions" if you don't understand this syntax
    return (reset_state<Builtins>(cx) && ...);
  }

  template <class... Builtins>
  static bool snapshot_all_request_states(BuiltinsList<Builtins...>, JSContext* cx) {
    return (snapshot_state<Builtins>(cx) && ...);
  }
  
private:
  template <class Builtin>
  static bool reset_state(JSContext* cx) {
    if (!Builtin::request_state.reset(cx)) {
      fprintf(stderr, "Failed to reset request state\n");
      return false;
    }
    return true;
  }

  template <class Builtin>
  static bool snapshot_state(JSContext* cx) {
    if (!Builtin::request_state.snapshot(cx)) {
      fprintf(stderr, "Failed to snapshot request state\n");
      return false;
    }
    return true;
  }
};

} // namespace fastly::state

#endif
