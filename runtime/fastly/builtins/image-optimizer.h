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

  struct Blur {
    double value;
    bool is_percentage;
  };
  static std::optional<Blur> to_blur(JSContext *cx, JS::HandleValue val);

  struct Brightness {
    double value;
  };
  static std::optional<Brightness> to_brightness(JSContext *cx, JS::HandleValue val);

  struct Color {
    host_api::HostString val;
  };
  static std::optional<Color> to_color(JSContext *cx, JS::HandleValue val,
                                       const std::string &field_name);

  struct Contrast {
    double value;
  };
  static std::optional<Contrast> to_contrast(JSContext *cx, JS::HandleValue val);

  struct Dpr {
    double value;
  };
  static std::optional<Dpr> to_dpr(JSContext *cx, JS::HandleValue val);

  struct Frame {
    int32_t value;
  };
  static std::optional<Frame> to_frame(JSContext *cx, JS::HandleValue val);

  struct PixelsOrPercentage {
    union {
      int pixels;
      double percentage;
    };
    bool is_percentage;
  };
  static std::optional<PixelsOrPercentage>
  to_pixels_or_percentage(JSContext *cx, JS::HandleValue val, const std::string &field_name);

  struct Height {
    PixelsOrPercentage value;
  };
  static std::optional<Height> to_height(JSContext *cx, JS::HandleValue val) {
    auto ret = to_pixels_or_percentage(cx, val, "height");
    if (!ret)
      return std::nullopt;
    return Height{*ret};
  }

  struct Level {
    host_api::HostString value;
  };
  static std::optional<Level> to_level(JSContext *cx, JS::HandleValue val);

  struct Saturation {
    double value;
  };
  static std::optional<Saturation> to_saturation(JSContext *cx, JS::HandleValue val);

  struct Sharpen {
    double amount;
    double radius;
    int32_t threshold;
  };
  static std::optional<Sharpen> to_sharpen(JSContext *cx, JS::HandleValue val);

  struct Viewbox {
    int32_t value;
  };
  static std::optional<Viewbox> to_viewbox(JSContext *cx, JS::HandleValue val);

  struct Width {
    PixelsOrPercentage value;
  };
  static std::optional<Width> to_width(JSContext *cx, JS::HandleValue val) {
    auto ret = to_pixels_or_percentage(cx, val, "width");
    if (!ret)
      return std::nullopt;
    return Width{*ret};
  }

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
                        std::optional<BGColor> bg_color, std::optional<Blur> blur,
                        std::optional<Brightness> brightness, std::optional<BWAlgorithm> bw,
                        std::optional<Contrast> contrast, std::optional<Disable> disable,
                        std::optional<Dpr> dpr, std::optional<Enable> enable,
                        std::optional<Fit> fit, std::optional<Format> format,
                        std::optional<Frame> frame, std::optional<Height> height,
                        std::optional<Level> level, std::optional<Metadata> metadata,
                        std::optional<Optimize> optimize, std::optional<Orient> orient,
                        std::optional<Profile> profile, std::optional<ResizeFilter> resize_filter,
                        std::optional<Saturation> saturation, std::optional<Sharpen> sharpen,
                        std::optional<Viewbox> viewbox, std::optional<Width> width)
      : region_(region), auto_(auto_val), bg_color_(std::move(bg_color)), blur_(blur), bw_(bw),
        contrast_(contrast), disable_(disable), dpr_(dpr), enable_(enable), fit_(fit),
        format_(format), frame_(frame), height_(height), level_(std::move(level)),
        metadata_(metadata), optimize_(optimize), orient_(orient), profile_(profile),
        resizeFilter_(resize_filter), saturation_(saturation), sharpen_(sharpen), viewbox_(viewbox),
        width_(width) {}
  Region region_;
  std::optional<Auto> auto_;
  std::optional<BGColor> bg_color_;
  std::optional<Blur> blur_;
  std::optional<Brightness> brightness_;
  std::optional<BWAlgorithm> bw_;
  std::optional<Contrast> contrast_;
  std::optional<Disable> disable_;
  std::optional<Dpr> dpr_;
  std::optional<Enable> enable_;
  std::optional<Fit> fit_;
  std::optional<Format> format_;
  std::optional<Frame> frame_;
  std::optional<Height> height_;
  std::optional<Level> level_;
  std::optional<Metadata> metadata_;
  std::optional<Optimize> optimize_;
  std::optional<Orient> orient_;
  std::optional<Profile> profile_;
  std::optional<ResizeFilter> resizeFilter_;
  std::optional<Saturation> saturation_;
  std::optional<Sharpen> sharpen_;
  std::optional<Viewbox> viewbox_;
  std::optional<Width> width_;
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
inline std::string to_string(const ImageOptimizerOptions::Blur &blur) {
  auto ret = "blur=" + std::to_string(blur.value);
  if (blur.is_percentage) {
    ret += 'p';
  }
  return ret;
}
inline std::string to_string(const ImageOptimizerOptions::Brightness &brightness) {
  return "brightness=" + std::to_string(brightness.value);
}
inline std::string to_string(const ImageOptimizerOptions::Contrast &contrast) {
  return "contrast=" + std::to_string(contrast.value);
}
inline std::string to_string(const ImageOptimizerOptions::Dpr &dpr) {
  return "dpr=" + std::to_string(dpr.value);
}
inline std::string to_string(const ImageOptimizerOptions::Frame &frame) {
  return "frame=" + std::to_string(frame.value);
}
inline std::string to_string(const ImageOptimizerOptions::PixelsOrPercentage &value) {
  if (value.is_percentage) {
    return std::to_string(value.percentage) + 'p';
  }
  return std::to_string(value.pixels);
}
inline std::string to_string(const ImageOptimizerOptions::Height &height) {
  return "height=" + to_string(height.value);
}
inline std::string to_string(const ImageOptimizerOptions::Level &level) {
  return "level=" + std::string(std::string_view(level.value));
}
inline std::string to_string(const ImageOptimizerOptions::Saturation &saturation) {
  return "saturation=" + std::to_string(saturation.value);
}
inline std::string to_string(const ImageOptimizerOptions::Sharpen &sharpen) {
  return "sharpen=a" + std::to_string(sharpen.amount) + ",r" + std::to_string(sharpen.radius) +
         ",t" + std::to_string(sharpen.threshold);
}
inline std::string to_string(const ImageOptimizerOptions::Viewbox &viewbox) {
  return "viewbox=" + std::to_string(viewbox.value);
}
inline std::string to_string(const ImageOptimizerOptions::Width &width) {
  return "width=" + to_string(width.value);
}
} // namespace fastly::image_optimizer
#endif