#ifndef FASTLY_NORMALIZE_HTTP_METHOD_H
#define FASTLY_NORMALIZE_HTTP_METHOD_H

namespace fastly::common {
// https://fetch.spec.whatwg.org/#concept-method-normalize
// To normalize a method, if it is a byte-case-insensitive match for `DELETE`, `GET`, `HEAD`,
// `OPTIONS`, `POST`, or `PUT`, byte-uppercase it.
// Returns `true` if the method name was normalized, `false` otherwise.
bool normalize_http_method(char *method, size_t length);
} // namespace fastly::common

#endif
