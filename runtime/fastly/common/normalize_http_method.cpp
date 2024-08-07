#include <string_view>

using namespace std::literals::string_view_literals;

namespace fastly::common {
namespace {
inline char *ascii_uppercase(char *str, size_t length) {
  char *up = reinterpret_cast<char *>(malloc(length + 1));
  int i = 0;
  while (i < length) {
    auto ch = str[i];
    if (ch >= 'a' && ch <= 'z') {
      up[i] = ch & ~0x20;
    } else {
      up[i] = ch;
    }
    i++;
  }
  up[i] = '\0';
  return up;
}
} // namespace

// https://fetch.spec.whatwg.org/#concept-method-normalize
// To normalize a method, if it is a byte-case-insensitive match for `DELETE`, `GET`, `HEAD`,
// `OPTIONS`, `POST`, or `PUT`, byte-uppercase it.
bool normalize_http_method(char *method, size_t length) {
  auto m = std::string_view(method, length);
  // It's quite likely the method is already uppercased,
  // so let's check it first before we create a copy of the method to uppercase it.
  // These have been order by most likely to occur.
  if (m != "GET"sv && m != "HEAD"sv && m != "OPTIONS"sv && m != "POST"sv && m != "PUT"sv &&
      m != "DELETE"sv) [[unlikely]] {
    auto umethod = ascii_uppercase(method, length);
    if (strcmp(umethod, "GET") == 0 || strcmp(umethod, "HEAD") == 0 ||
        strcmp(umethod, "OPTIONS") == 0 || strcmp(umethod, "POST") == 0 ||
        strcmp(umethod, "PUT") == 0 || strcmp(umethod, "DELETE") == 0) {
      strcpy(method, umethod);
      return true;
    }
  }
  return false;
}
} // namespace fastly::common