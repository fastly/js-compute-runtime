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
#define FASTLY_BEGIN_IMAGE_OPTIMIZER_OPTION_TYPE(type, lowercase, str)                             \
  class type : public EnumOption, public builtins::BuiltinNoConstructor<type> {                    \
  public:                                                                                          \
    static const JSPropertySpec static_properties[];                                               \
    static constexpr const char *class_name = #type;                                               \
  };
#include "image-optimizer-options.inc"

class ImageOptimizerOptions {
public:
  fastly_image_optimizer_transform_config to_config();
  static std::unique_ptr<ImageOptimizerOptions> create(JSContext *cx, JS::HandleValue opts_val);
  std::string to_string() const;

#define FASTLY_BEGIN_IMAGE_OPTIMIZER_OPTION_TYPE(type, lowercase, str) enum class type {
#define FASTLY_DEFINE_IMAGE_OPTIMIZER_OPTION(name, str) name,
#define FASTLY_END_IMAGE_OPTIMIZER_OPTION_TYPE(type)                                               \
  }                                                                                                \
  ;
#include "image-optimizer-options.inc"

#define FASTLY_BEGIN_IMAGE_OPTIMIZER_OPTION_TYPE(type, lowercase, str)                             \
  static std::optional<type> to_##lowercase(JSContext *cx, JS::HandleValue val);
#include "image-optimizer-options.inc"

  struct Color {
    host_api::HostString val;
  };
  static std::optional<Color> to_color(JSContext *cx, JS::HandleValue val,
                                       const std::string &field_name);

  struct BGColor {
    Color color;
  };
  static std::optional<BGColor> to_bg_color(JSContext *cx, JS::HandleValue val) {
    auto color = to_color(cx, val, "bgColor");
    if (!color)
      return std::nullopt;
    return BGColor{std::move(*color)};
  }

private:
  ImageOptimizerOptions(Region region, std::optional<Auto> auto_val,
                        std::optional<BGColor> bg_color, std::optional<BWAlgorithm> bw,
                        std::optional<Disable> disable, std::optional<Enable> enable,
                        std::optional<Fit> fit, std::optional<Format> format,
                        std::optional<Metadata> metadata, std::optional<Optimize> optimize,
                        std::optional<Profile> profile, std::optional<ResizeFilter> resize_filter)
      : region_(region), auto_(auto_val), bg_color_(std::move(bg_color)), bw_(bw),
        disable_(disable), enable_(enable), fit_(fit), format_(format), metadata_(metadata),
        optimize_(optimize), profile_(profile), resizeFilter_(resize_filter) {}
  Region region_;
  std::optional<Auto> auto_;
  std::optional<BGColor> bg_color_;
  std::optional<BWAlgorithm> bw_;
  std::optional<Disable> disable_;
  std::optional<Enable> enable_;
  std::optional<Fit> fit_;
  std::optional<Format> format_;
  std::optional<Metadata> metadata_;
  std::optional<Optimize> optimize_;
  std::optional<Profile> profile_;
  std::optional<ResizeFilter> resizeFilter_;
};

inline std::string to_string(const ImageOptimizerOptions &opts) { return opts.to_string(); }

#define FASTLY_BEGIN_IMAGE_OPTIMIZER_OPTION_TYPE(type, lowercase, str)                             \
  inline std::string to_string(ImageOptimizerOptions::type val) {                                  \
    using enum ImageOptimizerOptions::type;                                                        \
    std::string prefix = str;                                                                      \
    switch (val) {
#define FASTLY_DEFINE_IMAGE_OPTIMIZER_OPTION(name, str)                                            \
  case name:                                                                                       \
    return prefix + '=' + str;
#define FASTLY_END_IMAGE_OPTIMIZER_OPTION_TYPE(type)                                               \
  }                                                                                                \
  }
#include "image-optimizer-options.inc"

inline std::string to_string(const ImageOptimizerOptions::Color &c) {
  return std::string{std::string_view(c.val)};
}
inline std::string to_string(const ImageOptimizerOptions::BGColor &bg) {
  return "bg-color=" + to_string(bg.color);
}
} // namespace fastly::image_optimizer
#endif