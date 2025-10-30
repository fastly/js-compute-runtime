#ifndef FASTLY_IMAGE_OPTIMIZER_H
#define FASTLY_IMAGE_OPTIMIZER_H

#include "../host-api/host_api_fastly.h"
#include "builtin.h"
#include "encode.h"
#include "extension-api.h"
#include <memory>

#define HANDLE_IMAGE_OPTIMIZER_ERROR(cx, err)                                                      \
  ::host_api::handle_image_optimizer_error(cx, err, __LINE__, __func__)

namespace fastly::image_optimizer {
class EnumOption {
public:
  enum Slots { Count };
  static const JSFunctionSpec static_methods[];
  static const JSFunctionSpec methods[];
  static const JSPropertySpec properties[];
};
#define FASTLY_BEGIN_IMAGE_OPTIMIZER_OPTION_TYPE(type, lowercase)                                  \
  class type : public EnumOption, public builtins::BuiltinNoConstructor<type> {                    \
  public:                                                                                          \
    static const JSPropertySpec static_properties[];                                               \
    static constexpr const char *class_name = #type;                                               \
  };
#define FASTLY_DEFINE_IMAGE_OPTIMIZER_OPTION(name, str, value)
#define FASTLY_END_IMAGE_OPTIMIZER_OPTION_TYPE(type)
#include "image-optimizer-options.inc"

class ImageOptimizerOptions {
public:
  fastly_image_optimizer_transform_config to_config();
  static std::unique_ptr<ImageOptimizerOptions> create(JSContext *cx, JS::HandleValue opts_val);
  std::string to_string() const;

#define FASTLY_BEGIN_IMAGE_OPTIMIZER_OPTION_TYPE(type, lowercase) enum class type {
#define FASTLY_DEFINE_IMAGE_OPTIMIZER_OPTION(name, str, value) name = value,
#define FASTLY_END_IMAGE_OPTIMIZER_OPTION_TYPE(type)                                               \
  }                                                                                                \
  ;
#include "image-optimizer-options.inc"

#define FASTLY_BEGIN_IMAGE_OPTIMIZER_OPTION_TYPE(type, lowercase)                                  \
  static std::optional<type> to_##lowercase(JSContext *cx, JS::HandleValue val) {                  \
    if (val.isUndefined())                                                                         \
      return std::nullopt;                                                                         \
    if (!val.isString()) {                                                                         \
      api::throw_error(cx, api::Errors::TypeError, "imageOptimizerOptions", #type,                 \
                       "must be a string");                                                        \
      return std::nullopt;                                                                         \
    }                                                                                              \
    JS::RootedString str_val(cx, val.toString());                                                  \
    using enum type;
#define FASTLY_DEFINE_IMAGE_OPTIMIZER_OPTION(name, str, value)                                     \
  if (core::encode(cx, str_val) == std::string_view(str)) {                                        \
    return name;                                                                                   \
  }
#define FASTLY_END_IMAGE_OPTIMIZER_OPTION_TYPE(type)                                               \
  JS_ReportErrorUTF8(cx, #type " out of range");                                                   \
  return std::nullopt;                                                                             \
  }
#include "image-optimizer-options.inc"

private:
  ImageOptimizerOptions(Region region, std::optional<Auto> auto_val, std::optional<Format> format)
      : region_(region), auto_(auto_val), format_(format) {}
  Region region_;
  std::optional<Auto> auto_;
  std::optional<Format> format_;
};

inline std::string to_string(const ImageOptimizerOptions &opts) { return opts.to_string(); }

#define FASTLY_BEGIN_IMAGE_OPTIMIZER_OPTION_TYPE(type, lowercase)                                  \
  inline std::string to_string(ImageOptimizerOptions::type val) {                                  \
    using enum ImageOptimizerOptions::type;                                                        \
    std::string prefix = #lowercase;                                                               \
    switch (val) {
#define FASTLY_DEFINE_IMAGE_OPTIMIZER_OPTION(name, str, value)                                     \
  case name:                                                                                       \
    return prefix + '=' + str;
#define FASTLY_END_IMAGE_OPTIMIZER_OPTION_TYPE(type)                                               \
  }                                                                                                \
  }
#include "image-optimizer-options.inc"

} // namespace fastly::image_optimizer
#endif