#include "shielding.h"
#include "../../../StarlingMonkey/runtime/encode.h"
#include "../common/validations.h"
#include "../host-api/host_api_fastly.h"
#include "backend.h"
#include "fastly.h"

namespace fastly::shielding {
const JSFunctionSpec Shield::static_methods[] = {
    JS_FS_END,
};

const JSPropertySpec Shield::static_properties[] = {
    JS_PS_END,
};

const JSFunctionSpec Shield::methods[] = {
    JS_FN("runningOn", runningOn, 0, JSPROP_ENUMERATE),
    JS_FN("unencryptedBackend", unencryptedBackend, 0, JSPROP_ENUMERATE),
    JS_FN("encryptedBackend", encryptedBackend, 0, JSPROP_ENUMERATE), JS_FS_END};

const JSPropertySpec Shield::properties[] = {JS_PS_END};

bool Shield::runningOn(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0);
  bool is_me = JS::GetReservedSlot(self, static_cast<uint32_t>(Slots::IsMe)).toBoolean();
  args.rval().setBoolean(is_me);
  return true;
}
bool Shield::unencryptedBackend(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0)
  JS::RootedString target(
      cx, JS::GetReservedSlot(self, static_cast<uint32_t>(Slots::PlainTarget)).toString());
  return backend_for_shield(cx, target, args.rval());
}
bool Shield::encryptedBackend(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0)
  JS::RootedString target(
      cx, JS::GetReservedSlot(self, static_cast<uint32_t>(Slots::SSLTarget)).toString());
  return backend_for_shield(cx, target, args.rval());
}
bool Shield::backend_for_shield(JSContext *cx, JS::HandleString target,
                                JS::MutableHandleValue rval) {
  auto name = core::encode(cx, target);
  fastly_shielding_shield_backend_config config{nullptr, 0, 0};
  auto options_mask = 0;
  std::uint32_t backend_name_size_out = 0;
  constexpr std::size_t max_backend_name_size = 1024;
  std::string backend_name_out(max_backend_name_size, 0);
  auto status = fastly_shielding_backend_for_shield(name.ptr.get(), name.len, options_mask, &config,
                                                    backend_name_out.data(), max_backend_name_size,
                                                    &backend_name_size_out);
  if (status != 0) {
    HANDLE_ERROR(cx, status);
    return false;
  }
  backend_name_out.resize(backend_name_size_out);
  host_api::HostString backend_name(backend_name_out);
  return backend::Backend::get_from_valid_name(cx, std::move(backend_name), rval);
}

bool Shield::constructor(JSContext *cx, unsigned argc, JS::Value *vp) {
  REQUEST_HANDLER_ONLY("The Shield builtin");
  CTOR_HEADER("Shield", 1);

  JS::HandleValue name_arg = args.get(0);
  auto name = core::encode(cx, name_arg);

  // Keep calling fastly_shielding_shield_info with a increasingly large buffer until it returns OK
  std::uint32_t buf_size = 1024;
  std::vector<char> out_buf(buf_size);
  while (true) {
    std::uint32_t used_amount = 0;
    auto status = fastly_shielding_shield_info(name.ptr.get(), name.len, out_buf.data(), buf_size,
                                               &used_amount);
    if (status == 0) {
      out_buf.resize(used_amount);
      break;
    } else if (status == FASTLY_HOST_ERROR_BUFFER_LEN) {
      buf_size *= 2;
      out_buf = std::vector<char>(1024);
    } else {
      HANDLE_ERROR(cx, status);
      return false;
    }
  }

  if (out_buf.size() < 3) {
    return false;
  }

  JS::RootedObject self(cx, JS_NewObjectWithGivenProto(cx, &class_, proto_obj));

  bool is_me = out_buf[0] != 0;
  JS_SetReservedSlot(self, static_cast<uint32_t>(Slots::IsMe), JS::BooleanValue(is_me));
  JS_SetReservedSlot(self, static_cast<uint32_t>(Slots::PlainTarget),
                     JS::StringValue(JS_NewStringCopyZ(cx, out_buf.data() + 1)));
  auto plain_bytes_end = std::find(begin(out_buf) + 1, end(out_buf), 0);
  JS_SetReservedSlot(self, static_cast<uint32_t>(Slots::SSLTarget),
                     JS::StringValue(JS_NewStringCopyZ(cx, &*plain_bytes_end + 1)));

  args.rval().setObject(*self);
  return true;
}

bool install(api::Engine *engine) {
  RootedObject shielding_ns(engine->cx(), JS_NewObject(engine->cx(), nullptr));
  if (!Shield::init_class_impl(engine->cx(), shielding_ns)) {
    return false;
  }
  RootedObject shield_obj(engine->cx(), JS_GetConstructor(engine->cx(), Shield::proto_obj));
  RootedValue shield_val(engine->cx(), ObjectValue(*shield_obj));
  if (!JS_SetProperty(engine->cx(), shielding_ns, "Shield", shield_val)) {
    return false;
  }

  RootedValue shielding_ns_val(engine->cx(), JS::ObjectValue(*shielding_ns));
  if (!engine->define_builtin_module("fastly:shielding", shielding_ns_val)) {
    return false;
  }

  RootedObject fastly(engine->cx());
  if (!fastly::get_fastly_object(engine, &fastly)) {
    return false;
  }
  if (!JS_SetProperty(engine->cx(), fastly, "shielding", shielding_ns_val)) {
    return false;
  }

  return true;
}

} // namespace fastly::shielding
