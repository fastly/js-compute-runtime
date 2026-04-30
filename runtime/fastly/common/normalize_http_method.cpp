#include <algorithm>
#include <cctype>
#include <string_view>

namespace fastly::common {

// https://fetch.spec.whatwg.org/#concept-method-normalize
// To normalize a method, if it is a byte-case-insensitive match for `DELETE`, `GET`, `HEAD`,
// `OPTIONS`, `POST`, or `PUT`, byte-uppercase it.
bool normalize_http_method(char *method, size_t length) {
  // Ordered by most likely to occur.
  constexpr std::string_view methods[] = {"GET", "HEAD", "OPTIONS", "POST", "PUT", "DELETE"};
  std::string_view m(method, length);

  auto iequal = [](unsigned char a, unsigned char b) {
    return std::toupper(a) == std::toupper(b);
  };

  auto it = std::ranges::find_if(methods, [&](std::string_view candidate) {
    return std::ranges::equal(m, candidate, iequal);
  });

  if (it == std::ranges::end(methods) || *it == m) {
    return false; // not a recognized method, or already normalized
  }

  std::ranges::copy(*it, method); // copy the already-uppercase canonical form
  return true;
}
} // namespace fastly::common