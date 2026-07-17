#ifndef FASTLY_IMAGE_OPTIMIZER_H
#define FASTLY_IMAGE_OPTIMIZER_H

#include "../host-api/host_api_fastly.h"
#include "builtin.h"
#include "encode.h"
#include "extension-api.h"
#include <memory>
#include <variant>

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
  static bool optionsToQueryString(JSContext *cx, unsigned argc, JS::Value *vp);
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

  struct PixelsOrPercentage {
    union {
      int pixels;
      double percentage;
    };
    bool is_percentage;
  };
  static std::optional<PixelsOrPercentage> to_pixels_or_percentage(JSContext *cx,
                                                                   JS::HandleValue val);

  struct Size {
    struct Absolute {
      PixelsOrPercentage width;
      PixelsOrPercentage height;
    };
    struct Ratio {
      double width_ratio;
      double height_ratio;
    };
    std::variant<Absolute, Ratio> value;
  };
  static std::optional<Size> to_size(JSContext *cx, JS::HandleValue val);

  struct Position {
    // Absolute or offset
    std::variant<PixelsOrPercentage, double> x;
    std::variant<PixelsOrPercentage, double> y;
  };
  static std::optional<Position> to_position(JSContext *cx, JS::HandleValue val);
  struct Canvas {
    Size size;
    std::optional<Position> position;
  };
  static std::optional<Canvas> to_canvas(JSContext *cx, JS::HandleValue val);

  struct Color {
    host_api::HostString val;
  };
  static std::optional<Color> to_color(JSContext *cx, JS::HandleValue val);

  struct Contrast {
    double value;
  };
  static std::optional<Contrast> to_contrast(JSContext *cx, JS::HandleValue val);

  // Deduplicates between Crop and Precrop
  struct CropSpec {
    Size size;
    std::optional<Position> position;
    std::optional<CropMode> mode;
  };
  static std::optional<CropSpec> to_crop_spec(JSContext *cx, JS::HandleValue val);

  struct Crop {
    CropSpec value;
  };
  static std::optional<Crop> to_crop(JSContext *cx, JS::HandleValue val) {
    auto value = to_crop_spec(cx, val);
    if (!value)
      return std::nullopt;
    return Crop{*value};
  }

  struct Dpr {
    double value;
  };
  static std::optional<Dpr> to_dpr(JSContext *cx, JS::HandleValue val);

  struct Frame {
    int32_t value;
  };
  static std::optional<Frame> to_frame(JSContext *cx, JS::HandleValue val);

  struct Height {
    PixelsOrPercentage value;
  };
  static std::optional<Height> to_height(JSContext *cx, JS::HandleValue val) {
    auto ret = to_pixels_or_percentage(cx, val);
    if (!ret) {
      api::throw_error(cx, api::Errors::TypeError, "imageOptimizerOptions", "height",
                       "must be an integer pixel value or a string percentage value");
      return std::nullopt;
    }
    return Height{*ret};
  }

  struct Level {
    host_api::HostString value;
  };
  static std::optional<Level> to_level(JSContext *cx, JS::HandleValue val);

  struct Sides {
    PixelsOrPercentage top, right, bottom, left;
  };
  static std::optional<Sides> to_sides(JSContext *cx, JS::HandleValue val);

  struct Pad {
    Sides value;
  };
  static std::optional<Pad> to_pad(JSContext *cx, JS::HandleValue val) {
    auto sides = to_sides(cx, val);
    if (!sides) {
      api::throw_error(
          cx, api::Errors::TypeError, "imageOptimizerOptions", "trim",
          "must be an object with top, right, bottom, and left elements, each being an "
          "integer or a string percentage value");
      return std::nullopt;
    }
    return Pad{*sides};
  }

  struct Precrop {
    CropSpec value;
  };
  static std::optional<Precrop> to_precrop(JSContext *cx, JS::HandleValue val) {
    auto value = to_crop_spec(cx, val);
    if (!value)
      return std::nullopt;
    return Precrop{*value};
  }

  struct Quality {
    uint32_t value;
  };
  static std::optional<Quality> to_quality(JSContext *cx, JS::HandleValue val);

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

  struct Trim {
    Sides value;
  };
  static std::optional<Trim> to_trim(JSContext *cx, JS::HandleValue val) {
    auto sides = to_sides(cx, val);
    if (!sides) {
      api::throw_error(
          cx, api::Errors::TypeError, "imageOptimizerOptions", "trim",
          "must be an object with top, right, bottom, and left elements, each being an "
          "integer or a string percentage value");
      return std::nullopt;
    }
    return Trim{*sides};
  }

  struct TrimColor {
    Color color;
    std::optional<double> threshold;
  };
  static std::optional<TrimColor> to_trim_color(JSContext *cx, JS::HandleValue val);

  struct Viewbox {
    int32_t value;
  };
  static std::optional<Viewbox> to_viewbox(JSContext *cx, JS::HandleValue val);

  struct Width {
    PixelsOrPercentage value;
  };
  static std::optional<Width> to_width(JSContext *cx, JS::HandleValue val) {
    auto ret = to_pixels_or_percentage(cx, val);
    if (!ret) {
      api::throw_error(cx, api::Errors::TypeError, "imageOptimizerOptions", "width",
                       "must be an integer pixel value or a string percentage value");
      return std::nullopt;
    }
    return Width{*ret};
  }

  struct BGColor {
    Color color;
  };
  static std::optional<BGColor> to_bg_color(JSContext *cx, JS::HandleValue val) {
    auto color = to_color(cx, val);
    if (!color) {
      api::throw_error(cx, api::Errors::TypeError, "imageOptimizerOptions", "bgColor",
                       "must be a 3/6 character hex string or RGB(A) object");
      return std::nullopt;
    }
    return BGColor{std::move(*color)};
  }

private:
  ImageOptimizerOptions(
      Region region, std::optional<Auto> auto_val, std::optional<BGColor> bg_color,
      std::optional<Blur> blur, std::optional<Brightness> brightness, std::optional<BWAlgorithm> bw,
      std::optional<Canvas> canvas, std::optional<Contrast> contrast, std::optional<Crop> crop,
      std::optional<Disable> disable, std::optional<Dpr> dpr, std::optional<Enable> enable,
      std::optional<Fit> fit, std::optional<Format> format, std::optional<Frame> frame,
      std::optional<Height> height, std::optional<Level> level, std::optional<Metadata> metadata,
      std::optional<Optimize> optimize, std::optional<Orient> orient, std::optional<Pad> pad,
      std::optional<Precrop> precrop, std::optional<Profile> profile,
      std::optional<Quality> quality, std::optional<ResizeFilter> resize_filter,
      std::optional<Saturation> saturation, std::optional<Sharpen> sharpen,
      std::optional<Trim> trim, std::optional<TrimColor> trim_color, std::optional<Viewbox> viewbox,
      std::optional<Width> width)
      : region_(region), auto_(auto_val), bg_color_(std::move(bg_color)), blur_(blur),
        brightness_(brightness), bw_(bw), canvas_(canvas), contrast_(contrast), crop_(crop),
        disable_(disable), dpr_(dpr), enable_(enable), fit_(fit), format_(format), frame_(frame),
        height_(height), level_(std::move(level)), metadata_(metadata), optimize_(optimize),
        orient_(orient), pad_(pad), precrop_(precrop), profile_(profile), quality_(quality),
        resizeFilter_(resize_filter), saturation_(saturation), sharpen_(sharpen), trim_(trim),
        trim_color_(std::move(trim_color)), viewbox_(viewbox), width_(width) {}
  Region region_;
  std::optional<Auto> auto_;
  std::optional<BGColor> bg_color_;
  std::optional<Blur> blur_;
  std::optional<Brightness> brightness_;
  std::optional<BWAlgorithm> bw_;
  std::optional<Canvas> canvas_;
  std::optional<Contrast> contrast_;
  std::optional<Crop> crop_;
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
  std::optional<Pad> pad_;
  std::optional<Precrop> precrop_;
  std::optional<Profile> profile_;
  std::optional<Quality> quality_;
  std::optional<ResizeFilter> resizeFilter_;
  std::optional<Saturation> saturation_;
  std::optional<Sharpen> sharpen_;
  std::optional<Trim> trim_;
  std::optional<TrimColor> trim_color_;
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
inline std::string to_string(const ImageOptimizerOptions::PixelsOrPercentage &value) {
  if (value.is_percentage) {
    return std::to_string(value.percentage) + 'p';
  }
  return std::to_string(value.pixels);
}
inline std::string to_string(const ImageOptimizerOptions::Size &size) {
  if (auto abs = std::get_if<ImageOptimizerOptions::Size::Absolute>(&size.value)) {
    return to_string(abs->width) + ',' + to_string(abs->height);
  }
  auto ratio = std::get<ImageOptimizerOptions::Size::Ratio>(size.value);
  return std::to_string(ratio.width_ratio) + ':' + std::to_string(ratio.height_ratio);
}
inline std::string to_string(const ImageOptimizerOptions::Position &position) {
  std::string ret;
  if (auto value = std::get_if<ImageOptimizerOptions::PixelsOrPercentage>(&position.x)) {
    ret += 'x' + to_string(*value);
  } else {
    auto dbl = std::get<double>(position.x);
    ret += "offset-x" + std::to_string(dbl);
  }
  ret += ',';
  if (auto value = std::get_if<ImageOptimizerOptions::PixelsOrPercentage>(&position.y)) {
    ret += 'y' + to_string(*value);
  } else {
    auto dbl = std::get<double>(position.y);
    ret += "offset-y" + std::to_string(dbl);
  }
  return ret;
}
inline std::string to_string(const ImageOptimizerOptions::Canvas &canvas) {
  std::string ret = "canvas=" + to_string(canvas.size);
  if (canvas.position) {
    ret += ',' + to_string(*canvas.position);
  }
  return ret;
}
inline std::string to_string(const ImageOptimizerOptions::Contrast &contrast) {
  return "contrast=" + std::to_string(contrast.value);
}
inline std::string to_string(const ImageOptimizerOptions::CropSpec &crop) {
  std::string ret = to_string(crop.size);
  if (crop.position) {
    ret += ',' + to_string(*crop.position);
  }
  if (crop.mode) {
    ret += ',';
    switch (*crop.mode) {
    case ImageOptimizerOptions::CropMode::Safe:
      ret += "safe";
      break;
    case ImageOptimizerOptions::CropMode::Smart:
      ret += "smart";
      break;
    }
  }
  return ret;
}
inline std::string to_string(const ImageOptimizerOptions::Crop &crop) {
  return "crop=" + to_string(crop.value);
}
inline std::string to_string(const ImageOptimizerOptions::Dpr &dpr) {
  return "dpr=" + std::to_string(dpr.value);
}
inline std::string to_string(const ImageOptimizerOptions::Frame &frame) {
  return "frame=" + std::to_string(frame.value);
}
inline std::string to_string(const ImageOptimizerOptions::Height &height) {
  return "height=" + to_string(height.value);
}
inline std::string to_string(const ImageOptimizerOptions::Level &level) {
  return "level=" + std::string(std::string_view(level.value));
}
inline std::string to_string(const ImageOptimizerOptions::Sides &sides) {
  return to_string(sides.top) + ',' + to_string(sides.right) + ',' + to_string(sides.bottom) + ',' +
         to_string(sides.left);
}
inline std::string to_string(const ImageOptimizerOptions::Pad &pad) {
  return "pad=" + to_string(pad.value);
}
inline std::string to_string(const ImageOptimizerOptions::Precrop &precrop) {
  return "precrop=" + to_string(precrop.value);
}
inline std::string to_string(const ImageOptimizerOptions::Quality &quality) {
  return "quality=" + std::to_string(quality.value);
}
inline std::string to_string(const ImageOptimizerOptions::Saturation &saturation) {
  return "saturation=" + std::to_string(saturation.value);
}
inline std::string to_string(const ImageOptimizerOptions::Trim &trim) {
  return "trim=" + to_string(trim.value);
}
inline std::string to_string(const ImageOptimizerOptions::TrimColor &trim_color) {
  std::string ret = "trim-color=" + to_string(trim_color.color);
  if (trim_color.threshold) {
    ret += ",t" + std::to_string(*trim_color.threshold);
  }
  return ret;
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