#ifndef FASTLY_HEADERS_NORMALIZATION_H
#define FASTLY_HEADERS_NORMALIZATION_H

#include "builtin.h"
#include "host_api.h"

namespace fastly::common {

host_api::HostString normalize_header_name(JSContext *cx, JS::MutableHandleValue name_val,
                                           const char *fun_name);

host_api::HostString normalize_header_value(JSContext *cx, JS::MutableHandleValue value_val,
                                            const char *fun_name);

} // namespace fastly::common

#endif