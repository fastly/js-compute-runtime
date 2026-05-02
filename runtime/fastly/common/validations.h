#ifndef FASTLY_VALIDATIONS_H
#define FASTLY_VALIDATIONS_H

#include "builtin.h"
#include "js/GCAPI.h"
#include "mozilla/Maybe.h"
#include <optional>
#include <tuple>

namespace fastly::common {

std::optional<uint32_t> parse_and_validate_timeout(JSContext *cx, JS::HandleValue value,
                                                   const char *subsystem, std::string property_name,
                                                   uint64_t max_timeout);

std::optional<std::tuple<const uint8_t *, size_t, mozilla::Maybe<JS::AutoCheckCannotGC>>>
validate_bytes(JSContext *cx, JS::HandleValue bytes, const char *subsystem);

} // namespace fastly::common

#endif
