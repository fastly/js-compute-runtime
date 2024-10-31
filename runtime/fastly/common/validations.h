#ifndef FASTLY_VALIDATIONS_H
#define FASTLY_VALIDATIONS_H

#include "builtin.h"
#include <optional>

namespace fastly::common {

std::optional<uint32_t> parse_and_validate_timeout(JSContext *cx, JS::HandleValue value,
                                                   const char *subsystem, std::string property_name,
                                                   uint64_t max_timeout);

std::optional<std::tuple<const uint8_t *, size_t>>
validate_bytes(JSContext *cx, JS::HandleValue bytes, const char *subsystem, bool support_strings);

} // namespace fastly::common

#endif
