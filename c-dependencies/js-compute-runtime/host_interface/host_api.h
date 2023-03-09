#ifndef JS_COMPUTE_RUNTIME_HOST_API_H
#define JS_COMPUTE_RUNTIME_HOST_API_H

#include <memory>
#include <variant>

#include "c-at-e-world/c_at_e_world.h"
#include "core/allocator.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-offsetof"
#include "js/Utility.h"
#pragma clang diagnostic pop

/// A type to signal that a result produces no value.
struct Void final {};

template <typename T> class Result final {
  /// A private wrapper to distinguish `fastly_error_t` in the private variant.
  struct Error {
    fastly_error_t value;

    explicit Error(fastly_error_t value) : value{value} {}
  };

  std::variant<T, Error> result;

public:
  Result() = default;

  /// Explicitly construct an error.
  static Result err(fastly_error_t err) {
    Result res;
    res.emplace_err(err);
    return res;
  }

  /// Explicitly construct a successful result.
  template <typename... Args> static Result ok(Args &&...args) {
    Result res;
    res.emplace(std::forward<Args>(args)...);
    return res;
  }

  /// Construct an error in-place.
  fastly_error_t &emplace_err(fastly_error_t err) & {
    return this->result.template emplace<Error>(err).value;
  }

  /// Construct a value of T in-place.
  template <typename... Args> T &emplace(Args &&...args) {
    return this->result.template emplace<T>(std::forward<Args>(args)...);
  }

  /// True when the result contains an error.
  bool is_err() const { return std::holds_alternative<Error>(this->result); }

  /// Return a pointer to the error value of this result, if the call failed.
  const fastly_error_t *to_err() const {
    return reinterpret_cast<const fastly_error_t *>(std::get_if<Error>(&this->result));
  }

  /// Assume the call was successful, and return a reference to the result.
  T &unwrap() { return std::get<T>(this->result); }
};

/// A single chunk read from an HttpBody.
struct HttpBodyChunk {
  JS::UniqueChars ptr;
  size_t len;

  HttpBodyChunk() = default;
  HttpBodyChunk(JS::UniqueChars ptr, size_t len) : ptr{std::move(ptr)}, len{len} {}
};

/// A convenience wrapper for the host calls involving http bodies.
class HttpBody final {
public:
  static constexpr fastly_body_handle_t invalid = UINT32_MAX - 1;

  /// The handle to use when making host calls, initialized to the special invalid value used by
  /// executed.
  fastly_body_handle_t handle = invalid;

  HttpBody() = default;
  explicit HttpBody(fastly_body_handle_t handle) : handle{handle} {}

  /// Returns true when this body handle is valid.
  bool valid() const { return this->handle != invalid; }

  /// Make a new body handle.
  static Result<HttpBody> make();

  /// Read a chunk from this handle.
  Result<HttpBodyChunk> read(uint32_t chunk_size) const;

  /// Write a chunk to this handle.
  Result<uint32_t> write(const uint8_t *bytes, size_t len) const;

  /// Writes the given number of bytes from the given buffer to the given handle.
  ///
  /// The host doesn't necessarily write all bytes in any particular call to
  /// `write`, so to ensure all bytes are written, we call it in a loop.
  Result<Void> write_all(const uint8_t *bytes, size_t len) const;

  /// Append another HttpBody to this one.
  Result<Void> append(HttpBody other) const;

  /// Close this handle, and reset internal state to invalid.
  Result<Void> close();
};

#endif
