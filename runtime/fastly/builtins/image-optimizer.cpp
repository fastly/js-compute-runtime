#include "image-optimizer.h"
#include <iostream>

namespace {
std::optional<double> from_percentage(std::string_view sv) {
  if (!sv.ends_with('%')) {
    return std::nullopt;
  }
  char *end;
  auto percentage = std::strtod(sv.data(), &end);
  if (end != sv.data() + sv.size() - 1 || percentage == HUGE_VAL) {
    return std::nullopt;
  }
  return percentage;
}

std::optional<double> to_number_between_inclusive(JS::HandleValue val, double from, double to) {
  if (!val.isNumber()) {
    return std::nullopt;
  }
  auto num = val.toNumber();
  if (num < from || num > to) {
    return std::nullopt;
  }
  return num;
}
} // namespace

namespace fastly::image_optimizer {
const JSFunctionSpec EnumOption::static_methods[] = {
    JS_FS_END,
};
const JSFunctionSpec EnumOption::methods[] = {
    JS_FS_END,
};

const JSPropertySpec EnumOption::properties[] = {
    JS_PS_END,
};

#define FASTLY_BEGIN_IMAGE_OPTIMIZER_OPTION_TYPE(type, lowercase, str)                             \
  const JSPropertySpec type::static_properties[] = {
#define FASTLY_DEFINE_IMAGE_OPTIMIZER_OPTION(name, str) JS_STRING_PS(#name, str, JSPROP_READONLY),
#define FASTLY_END_IMAGE_OPTIMIZER_OPTION_TYPE(type)                                               \
  JS_PS_END,                                                                                       \
  }                                                                                                \
  ;
#include "image-optimizer-options.inc"

bool install(api::Engine *engine) {
  RootedObject image_optimizer_ns(engine->cx(), JS_NewObject(engine->cx(), nullptr));

#define FASTLY_BEGIN_IMAGE_OPTIMIZER_OPTION_TYPE(type, lowercase, str)                             \
  if (!type::init_class_impl(engine->cx(), engine->global())) {                                    \
    return false;                                                                                  \
  }                                                                                                \
  RootedObject lowercase##_obj(                                                                    \
      engine->cx(),                                                                                \
      JS_GetConstructor(                                                                           \
          engine->cx(),                                                                            \
          builtins::BuiltinNoConstructor<fastly::image_optimizer::type>::proto_obj));              \
  RootedValue lowercase##_val(engine->cx(), ObjectValue(*lowercase##_obj));                        \
  if (!JS_SetProperty(engine->cx(), image_optimizer_ns, #type, lowercase##_val)) {                 \
    return false;                                                                                  \
  }
#define FASTLY_DEFINE_IMAGE_OPTIMIZER_OPTION(name, str)
#define FASTLY_END_IMAGE_OPTIMIZER_OPTION_TYPE(type)
#include "image-optimizer-options.inc"

  RootedValue image_optimizer_ns_val(engine->cx(), JS::ObjectValue(*image_optimizer_ns));
  if (!engine->define_builtin_module("fastly:image-optimizer", image_optimizer_ns_val)) {
    return false;
  }
  return true;
} // namespace fastly::image_optimizer

std::unique_ptr<ImageOptimizerOptions> ImageOptimizerOptions::create(JSContext *cx,
                                                                     JS::HandleValue opts_val) {
  // Extract properties.
#define FASTLY_BEGIN_IMAGE_OPTIMIZER_OPTION_TYPE(type, lowercase, str)                             \
  JS::RootedValue lowercase##_val(cx);
#include "image-optimizer-options.inc"
  JS::RootedValue preserve_query_string_on_origin_request_val(cx);
  JS::RootedValue bg_color_val(cx);
  JS::RootedValue blur_val(cx);
  JS::RootedValue brightness_val(cx);
  JS::RootedValue canvas_val(cx);
  JS::RootedValue contrast_val(cx);
  JS::RootedValue crop_val(cx);
  JS::RootedValue dpr_val(cx);
  JS::RootedValue frame_val(cx);
  JS::RootedValue height_val(cx);
  JS::RootedValue level_val(cx);
  JS::RootedValue pad_val(cx);
  JS::RootedValue precrop_val(cx);
  JS::RootedValue quality_val(cx);
  JS::RootedValue saturation_val(cx);
  JS::RootedValue sharpen_val(cx);
  JS::RootedValue trim_val(cx);
  JS::RootedValue trim_color_val(cx);
  JS::RootedValue viewbox_val(cx);
  JS::RootedValue width_val(cx);
  JS::RootedObject opts(cx, opts_val.toObjectOrNull());
  if (!JS_GetProperty(cx, opts, "region", &region_val) ||
      !JS_GetProperty(cx, opts, "preserve_query_string_on_origin_request",
                      &preserve_query_string_on_origin_request_val) ||
      !JS_GetProperty(cx, opts, "auto", &auto_val) ||
      !JS_GetProperty(cx, opts, "bgColor", &bg_color_val) ||
      !JS_GetProperty(cx, opts, "blur", &blur_val) ||
      !JS_GetProperty(cx, opts, "brightness", &brightness_val) ||
      !JS_GetProperty(cx, opts, "bw", &bw_val) ||
      !JS_GetProperty(cx, opts, "canvas", &canvas_val) ||
      !JS_GetProperty(cx, opts, "contrast", &contrast_val) ||
      !JS_GetProperty(cx, opts, "crop", &crop_val) ||
      !JS_GetProperty(cx, opts, "disable", &disable_val) ||
      !JS_GetProperty(cx, opts, "dpr", &dpr_val) ||
      !JS_GetProperty(cx, opts, "enable", &enable_val) ||
      !JS_GetProperty(cx, opts, "fit", &fit_val) ||
      !JS_GetProperty(cx, opts, "format", &format_val) ||
      !JS_GetProperty(cx, opts, "frame", &frame_val) ||
      !JS_GetProperty(cx, opts, "height", &height_val) ||
      !JS_GetProperty(cx, opts, "level", &level_val) ||
      !JS_GetProperty(cx, opts, "metadata", &metadata_val) ||
      !JS_GetProperty(cx, opts, "optimize", &optimize_val) ||
      !JS_GetProperty(cx, opts, "orient", &orient_val) ||
      !JS_GetProperty(cx, opts, "pad", &pad_val) ||
      !JS_GetProperty(cx, opts, "precrop", &precrop_val) ||
      !JS_GetProperty(cx, opts, "profile", &profile_val) ||
      !JS_GetProperty(cx, opts, "quality", &quality_val) ||
      !JS_GetProperty(cx, opts, "resizeFilter", &resize_filter_val) ||
      !JS_GetProperty(cx, opts, "saturation", &saturation_val) ||
      !JS_GetProperty(cx, opts, "sharpen", &sharpen_val) ||
      !JS_GetProperty(cx, opts, "trim", &trim_val) ||
      !JS_GetProperty(cx, opts, "trimColor", &trim_color_val) ||
      !JS_GetProperty(cx, opts, "viewbox", &viewbox_val) ||
      !JS_GetProperty(cx, opts, "width", &width_val)) {
    return nullptr;
  }

  if (region_val.isUndefined()) {
    api::throw_error(cx, api::Errors::TypeError, "Request", "imageOptimizerOptions",
                     "contain region");
    return nullptr;
  }

#define TRY_CONVERT(name)                                                                          \
  decltype(to_##name(cx, name##_val)) name##_opt;                                                  \
  if (!name##_val.isUndefined()) {                                                                 \
    name##_opt = to_##name(cx, name##_val);                                                        \
    if (!name##_opt) {                                                                             \
      return nullptr;                                                                              \
    }                                                                                              \
  }

#define FASTLY_BEGIN_IMAGE_OPTIMIZER_OPTION_TYPE(type, lowercase, str) TRY_CONVERT(lowercase);
#include "image-optimizer-options.inc"

  TRY_CONVERT(bg_color);
  TRY_CONVERT(blur);
  TRY_CONVERT(brightness);
  TRY_CONVERT(contrast);
  TRY_CONVERT(dpr);
  TRY_CONVERT(frame);
  TRY_CONVERT(height);
  TRY_CONVERT(level);
  TRY_CONVERT(saturation);
  TRY_CONVERT(sharpen);
  TRY_CONVERT(viewbox);
  TRY_CONVERT(width);

  return std::unique_ptr<ImageOptimizerOptions>{new ImageOptimizerOptions(
      *region_opt, auto_opt, std::move(bg_color_opt), blur_opt, brightness_opt, bw_opt,
      contrast_opt, disable_opt, dpr_opt, enable_opt, fit_opt, format_opt, frame_opt, height_opt,
      std::move(level_opt), metadata_opt, optimize_opt, orient_opt, profile_opt, resize_filter_opt,
      saturation_opt, sharpen_opt, viewbox_opt, width_opt)};
}

std::string ImageOptimizerOptions::to_string() const {
  using image_optimizer::to_string;
  std::string ret = to_string(region_);
  auto append = [&ret](auto &&v) {
    if (v)
      ret += "&" + to_string(*v);
  };
  append(auto_);
  append(bg_color_);
  append(blur_);
  append(brightness_);
  append(bw_);
  append(contrast_);
  append(disable_);
  append(dpr_);
  append(enable_);
  append(fit_);
  append(format_);
  append(frame_);
  append(height_);
  append(level_);
  append(metadata_);
  append(optimize_);
  append(orient_);
  append(profile_);
  append(resizeFilter_);
  append(saturation_);
  append(sharpen_);
  append(viewbox_);
  append(width_);
  return ret;
}

// TODO
// canvas
// crop
// pad
// precrop
// quality
// trim
// trimcolor

fastly_image_optimizer_transform_config ImageOptimizerOptions::to_config() {
  fastly_image_optimizer_transform_config config;
  auto str = this->to_string();
  host_api::HostString host_str(str);
  config.sdk_claims_opts = host_str.ptr.release();
  config.sdk_claims_opts_len = host_str.len;
  return config;
}

std::optional<ImageOptimizerOptions::Brightness>
ImageOptimizerOptions::to_brightness(JSContext *cx, JS::HandleValue val) {
  auto num = to_number_between_inclusive(val, -100, 100);
  if (!num) {
    api::throw_error(cx, api::Errors::TypeError, "imageOptimizerOptions", "brightness",
                     "must be a number between -100 and 100");
    return std::nullopt;
  }
  return Brightness{*num};
}
std::optional<ImageOptimizerOptions::Contrast>
ImageOptimizerOptions::to_contrast(JSContext *cx, JS::HandleValue val) {
  auto num = to_number_between_inclusive(val, -100, 100);
  if (!num) {
    api::throw_error(cx, api::Errors::TypeError, "imageOptimizerOptions", "contrast",
                     "must be a number between -100 and 100");
    return std::nullopt;
  }
  return Contrast{*num};
}
std::optional<ImageOptimizerOptions::Saturation>
ImageOptimizerOptions::to_saturation(JSContext *cx, JS::HandleValue val) {
  auto num = to_number_between_inclusive(val, -100, 100);
  if (!num) {
    api::throw_error(cx, api::Errors::TypeError, "imageOptimizerOptions", "saturation",
                     "must be a number between -100 and 100");
    return std::nullopt;
  }
  return Saturation{*num};
}
std::optional<ImageOptimizerOptions::Dpr> ImageOptimizerOptions::to_dpr(JSContext *cx,
                                                                        JS::HandleValue val) {
  auto num = to_number_between_inclusive(val, 1, 10);
  if (!num) {
    api::throw_error(cx, api::Errors::TypeError, "imageOptimizerOptions", "dpr",
                     "must be a number between 1 and 10");
    return std::nullopt;
  }
  return Dpr{*num};
}
std::optional<ImageOptimizerOptions::Level> ImageOptimizerOptions::to_level(JSContext *cx,
                                                                            JS::HandleValue val) {
  auto throw_error = [&]() {
    api::throw_error(
        cx, api::Errors::TypeError, "imageOptimizerOptions", "level",
        "must be a string containing 1.0, 1.1, 1.2, 1.3, 2.0, 2.1, 2.2, 3.0, 3.1, 3.2, "
        "4.0, 4.1, 4.2, 5.0, 5.1, 5.2, 6.0, 6.1, or 6.2");
  };
  if (!val.isString()) {
    throw_error();
    return std::nullopt;
  }
  JS::RootedString js_str(cx, val.toString());
  auto str = core::encode(cx, js_str);
  std::string_view sv = str;
  if (sv != "1.0" && sv != "1.1" && sv != "1.2" && sv != "1.3" && sv != "2.0" && sv != "2.1" &&
      sv != "2.2" && sv != "3.0" && sv != "3.1" && sv != "3.2" && sv != "4.0" && sv != "4.1" &&
      sv != "4.2" && sv != "5.0" && sv != "5.1" && sv != "5.2" && sv != "6.0" && sv != "6.1" &&
      sv != "6.2") {
    throw_error();
    return std::nullopt;
  }
  return Level{std::move(str)};
}
std::optional<ImageOptimizerOptions::Frame> ImageOptimizerOptions::to_frame(JSContext *cx,
                                                                            JS::HandleValue val) {
  if (!val.isInt32() || val.toInt32() != 1) {
    api::throw_error(cx, api::Errors::TypeError, "imageOptimizerOptions", "frame", "must be 1");
    return std::nullopt;
  }
  return Frame{1};
}
std::optional<ImageOptimizerOptions::Viewbox>
ImageOptimizerOptions::to_viewbox(JSContext *cx, JS::HandleValue val) {
  if (!val.isInt32() || val.toInt32() != 1) {
    api::throw_error(cx, api::Errors::TypeError, "imageOptimizerOptions", "viewbox", "must be 1");
    return std::nullopt;
  }
  return Viewbox{1};
}
std::optional<ImageOptimizerOptions::Blur> ImageOptimizerOptions::to_blur(JSContext *cx,
                                                                          JS::HandleValue val) {
  auto throw_error = [&]() {
    api::throw_error(cx, api::Errors::TypeError, "imageOptimizerOptions", "blur",
                     "must be a number between 0.5 and 1000 or a string percentage value");
  };
  if (val.isNumber()) {
    auto num = to_number_between_inclusive(val, 0.5, 1000);
    if (!num) {
      throw_error();
      return std::nullopt;
    }
    return Blur{.value = *num, .is_percentage = false};
  }
  if (!val.isString()) {
    throw_error();
    return std::nullopt;
  }
  JS::RootedString js_str(cx, val.toString());
  auto str = core::encode(cx, js_str);
  auto percentage = from_percentage(str);
  if (!percentage) {
    throw_error();
    return std::nullopt;
  }
  return Blur{.value = *percentage, .is_percentage = true};
}
std::optional<ImageOptimizerOptions::Sharpen>
ImageOptimizerOptions::to_sharpen(JSContext *cx, JS::HandleValue val) {
  auto throw_error = [&]() {
    api::throw_error(cx, api::Errors::TypeError, "imageOptimizerOptions", "sharpen",
                     "must be an object with keys `amount` (0.0-10.0), `radius` (0.5-1000.0), and "
                     "`threshold` (0-255)");
  };
  if (!val.isObject()) {
    throw_error();
    return std::nullopt;
  }
  JS::RootedValue amount_val(cx);
  JS::RootedValue radius_val(cx);
  JS::RootedValue threshold_val(cx);
  JS::RootedObject opts(cx, &val.toObject());
  if (!JS_GetProperty(cx, opts, "amount", &amount_val) ||
      !JS_GetProperty(cx, opts, "radius", &radius_val) ||
      !JS_GetProperty(cx, opts, "threshold", &threshold_val)) {
    return std::nullopt;
  }
  auto amount = to_number_between_inclusive(amount_val, 0, 10);
  auto radius = to_number_between_inclusive(radius_val, 0.5, 1000);
  if (!amount || !radius || !threshold_val.isInt32() || threshold_val.toInt32() < 0 ||
      threshold_val.toInt32() > 255) {
    throw_error();
    return std::nullopt;
  }
  return Sharpen{*amount, *radius, threshold_val.toInt32()};
}
std::optional<ImageOptimizerOptions::PixelsOrPercentage>
ImageOptimizerOptions::to_pixels_or_percentage(JSContext *cx, JS::HandleValue val,
                                               const std::string &field_name) {
  auto throw_error = [&]() {
    api::throw_error(cx, api::Errors::TypeError, "imageOptimizerOptions", field_name.data(),
                     "must be an integer pixel value or a string percentage value");
  };
  if (val.isInt32()) {
    return PixelsOrPercentage{.pixels = val.toInt32(), .is_percentage = false};
  }
  if (!val.isString()) {
    throw_error();
    return std::nullopt;
  }
  JS::RootedString js_str(cx, val.toString());
  auto str = core::encode(cx, js_str);
  auto percentage = from_percentage(str);
  if (!percentage) {
    throw_error();
    return std::nullopt;
  }
  return PixelsOrPercentage{.percentage = *percentage, .is_percentage = true};
}

std::optional<ImageOptimizerOptions::Color>
ImageOptimizerOptions::to_color(JSContext *cx, JS::HandleValue val, const std::string &field_name) {
  // Hex strings of size 3 or 6 are allowed
  if (val.isString()) {
    JS::RootedString str_val(cx, val.toString());
    auto str = core::encode(cx, str_val);
    if ((str.len == 3 || str.len == 6) &&
        (std::all_of(str.begin(), str.end(), [](char c) { return std::isxdigit(c); }))) {
      return Color{std::move(str)};
    }
  }

  auto throw_error = [&]() {
    api::throw_error(cx, api::Errors::TypeError, "imageOptimizerOptions", field_name.data(),
                     "must be a 3/6 character hex string or RGB(A) object");
  };

  // Otherwise, it should be an rgb(a) object
  if (!val.isObject()) {
    throw_error();
    return std::nullopt;
  }

  JS::RootedValue r(cx);
  JS::RootedValue g(cx);
  JS::RootedValue b(cx);
  JS::RootedValue a(cx);
  JS::RootedObject obj(cx, &val.toObject());
  if (!JS_GetProperty(cx, obj, "r", &r) || !JS_GetProperty(cx, obj, "g", &g) ||
      !JS_GetProperty(cx, obj, "b", &b) || !JS_GetProperty(cx, obj, "a", &a)) {
    return std::nullopt;
  }

  auto is_valid_color_component = [](const auto &c) {
    return c.isInt32() && c.toInt32() >= 0 && c.toInt32() < 256;
  };
  auto alpha_valid =
      a.isUndefined() || (a.isNumber() && a.toNumber() >= 0.0 && a.toNumber() <= 1.0);
  if (!is_valid_color_component(r) || !is_valid_color_component(g) ||
      !is_valid_color_component(b) || !alpha_valid) {
    throw_error();
    return std::nullopt;
  }

  std::string rep;
  rep += std::to_string(r.toInt32()) + ',' + std::to_string(g.toInt32()) + ',' +
         std::to_string(b.toInt32());
  if (!a.isUndefined()) {
    rep += ',' + std::to_string(a.toNumber());
  }
  return Color{host_api::HostString(rep)};
}

#define FASTLY_BEGIN_IMAGE_OPTIMIZER_OPTION_TYPE(type, lowercase, str)                             \
  std::optional<ImageOptimizerOptions::type> ImageOptimizerOptions::to_##lowercase(                \
      JSContext *cx, JS::HandleValue val) {                                                        \
    if (!val.isString()) {                                                                         \
      api::throw_error(cx, api::Errors::TypeError, "imageOptimizerOptions", #type,                 \
                       "must be a string");                                                        \
      return std::nullopt;                                                                         \
    }                                                                                              \
    JS::RootedString str_val(cx, val.toString());                                                  \
    using enum type;
#define FASTLY_DEFINE_IMAGE_OPTIMIZER_OPTION(name, str)                                            \
  if (core::encode(cx, str_val) == std::string_view(str)) {                                        \
    return name;                                                                                   \
  }
#define FASTLY_END_IMAGE_OPTIMIZER_OPTION_TYPE(type)                                               \
  JS_ReportErrorUTF8(cx, #type " out of range");                                                   \
  return std::nullopt;                                                                             \
  }
#include "image-optimizer-options.inc"
} // namespace fastly::image_optimizer