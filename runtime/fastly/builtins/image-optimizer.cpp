#include "image-optimizer.h"
#include <iostream>
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

#define FASTLY_BEGIN_IMAGE_OPTIMIZER_OPTION_TYPE(type, lowercase)                                  \
  const JSPropertySpec type::static_properties[] = {
#define FASTLY_DEFINE_IMAGE_OPTIMIZER_OPTION(name, str, value)                                     \
  JS_STRING_PS(#name, str, JSPROP_READONLY),
#define FASTLY_END_IMAGE_OPTIMIZER_OPTION_TYPE(type)                                               \
  JS_PS_END,                                                                                       \
  }                                                                                                \
  ;
#include "image-optimizer-options.inc"

bool install(api::Engine *engine) {
  RootedObject image_optimizer_ns(engine->cx(), JS_NewObject(engine->cx(), nullptr));

#define FASTLY_BEGIN_IMAGE_OPTIMIZER_OPTION_TYPE(type, lowercase)                                  \
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
#define FASTLY_DEFINE_IMAGE_OPTIMIZER_OPTION(name, str, value)
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
  // Extract properties. `region` is the only mandatory property
  JS::RootedValue region_val(cx);
  JS::RootedValue preserve_query_string_on_origin_request_val(cx);
  JS::RootedValue auto_val(cx);
  JS::RootedValue bg_color_val(cx);
  JS::RootedValue blur_val(cx);
  JS::RootedValue brightness_val(cx);
  JS::RootedValue bw_val(cx);
  JS::RootedValue canvas_val(cx);
  JS::RootedValue contrast_val(cx);
  JS::RootedValue crop_val(cx);
  JS::RootedValue dpr_val(cx);
  JS::RootedValue enable_val(cx);
  JS::RootedValue fit_val(cx);
  JS::RootedValue format_val(cx);
  JS::RootedValue frame_val(cx);
  JS::RootedValue height_val(cx);
  JS::RootedValue level_val(cx);
  JS::RootedValue metadata_val(cx);
  JS::RootedValue optimize_val(cx);
  JS::RootedValue orient_val(cx);
  JS::RootedValue pad_val(cx);
  JS::RootedValue precrop_val(cx);
  JS::RootedValue profile_val(cx);
  JS::RootedValue quality_val(cx);
  JS::RootedValue resize_filter_val(cx);
  JS::RootedValue saturation_val(cx);
  JS::RootedValue sharpen_val(cx);
  JS::RootedValue trim_val(cx);
  JS::RootedValue trim_color_val(cx);
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
      !JS_GetProperty(cx, opts, "crop", &crop_val) || !JS_GetProperty(cx, opts, "dpr", &dpr_val) ||
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
      !JS_GetProperty(cx, opts, "width", &width_val)) {
    return nullptr;
  }

  if (region_val.isUndefined()) {
    api::throw_error(cx, api::Errors::TypeError, "Request", "imageOptimizerOptions",
                     "contain region");
    return nullptr;
  }
  auto region = to_region(cx, region_val);
  if (!region) {
    return nullptr;
  }

  auto auto_ = to_auto(cx, auto_val);
  auto format = to_format(cx, format_val);

  return std::unique_ptr<ImageOptimizerOptions>{
      new ImageOptimizerOptions(region.value(), auto_, format)};
}

std::string ImageOptimizerOptions::to_string() const {
  using image_optimizer::to_string;
  std::string ret = to_string(region_);
  auto append = [&ret](auto &&v) {
    if (v)
      ret += "&" + to_string(*v);
  };
  append(auto_);
  append(format_);
  return ret;
}

fastly_image_optimizer_transform_config ImageOptimizerOptions::to_config() {
  fastly_image_optimizer_transform_config config;
  auto str = this->to_string();
  host_api::HostString host_str(str);
  config.sdk_claims_opts = host_str.ptr.release();
  config.sdk_claims_opts_len = host_str.len;
  return config;
}
} // namespace fastly::image_optimizer