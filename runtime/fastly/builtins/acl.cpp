#include "acl.h"
#include "../common/validations.h"
#include "../host-api/host_api_fastly.h"
#include "builtin.h"
#include "encode.h"
#include "fastly.h"
#include "js/JSON.h"
#include <arpa/inet.h>

using builtins::BuiltinImpl;
using fastly::FastlyGetErrorMessage;

namespace fastly::acl {

namespace {
host_api::HostString parse_and_validate_name(JSContext *cx, JS::HandleValue name_val) {
  if (!name_val.isString()) {
    JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr, JSMSG_ACL_NAME_NOT_STRING);
    return nullptr;
  }
  JS::RootedString name(cx, name_val.toString());
  if (!name) {
    return nullptr;
  }
  auto length = JS::GetStringLength(name);
  if (length > 254) {
    JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr, JSMSG_ACL_NAME_TOO_LONG);
    return nullptr;
  }
  if (length == 0) {
    JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr, JSMSG_ACL_NAME_EMPTY);
    return nullptr;
  }
  return core::encode(cx, name);
}
} // namespace

bool Acl::open(JSContext *cx, unsigned argc, JS::Value *vp) {
  CallArgs args = CallArgsFromVp(argc, vp);
  if (!args.requireAtLeast(cx, "Acl open", 1))
    return false;

  auto name = parse_and_validate_name(cx, args.get(0));
  if (!name) {
    return false;
  }

  auto open_res = host_api::Acl::open(name);
  if (auto *err = open_res.to_err()) {
    HANDLE_ERROR(cx, *err);
    return false;
  }

  auto found = open_res.unwrap();
  if (!found.has_value()) {
    JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr, JSMSG_ACL_NOT_FOUND,
                              name.begin());
    return false;
  }

  RootedObject acl_instance(cx, JS_NewObjectWithGivenProto(cx, &Acl::class_, Acl::proto_obj));
  if (!acl_instance) {
    return false;
  }

  JS::SetReservedSlot(acl_instance, static_cast<uint32_t>(Slots::HostAcl),
                      JS::Int32Value(found.value().handle));

  args.rval().setObject(*acl_instance);
  return true;
}

bool Acl::lookup(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(1)

  JS::RootedString address_str(cx, JS::ToString(cx, args[0]));
  if (!address_str)
    return false;

  auto address = core::encode(cx, address_str);
  if (!address) {
    return false;
  }

  int format = AF_INET;
  size_t octets_len = 4;
  if (std::find(address.begin(), address.end(), ':') != address.end()) {
    format = AF_INET6;
    octets_len = 16;
  }

  uint8_t octets[sizeof(struct in6_addr)];
  if (inet_pton(format, address.begin(), octets) != 1) {
    JS_ReportErrorLatin1(cx, "Invalid address passed to acl.lookup");
    return false;
  }

  host_api::Acl acl{static_cast<host_api::Acl::Handle>(
      JS::GetReservedSlot(self, static_cast<uint32_t>(Slots::HostAcl)).toInt32())};
  auto lookup_res = acl.lookup(std::span<uint8_t>{octets, octets_len});
  if (auto *err = lookup_res.to_err()) {
    HANDLE_ERROR(cx, *err);
    return false;
  }

  auto [body, error] = lookup_res.unwrap();

  switch (error) {
  case host_api::Acl::LookupError::Ok:
    break;
  case host_api::Acl::LookupError::NoContent:
    JS_ReportErrorLatin1(cx, "Acl.lookup: No content found");
    return false;
  case host_api::Acl::LookupError::TooManyRequests:
    JS_ReportErrorLatin1(cx, "Acl.lookup: Too many requests, please try again later");
    return false;
  case host_api::Acl::LookupError::Uninitialized:
    JS_ReportErrorLatin1(cx, "Acl.lookup: Uninitialized acl passed to lookup");
    return false;
  }

  auto buf_res = body.value().read_all();
  if (auto *err = buf_res.to_err()) {
    HANDLE_ERROR(cx, *err);
    return false;
  }

  JS::RootedString str(
      cx,
      JS_NewStringCopyUTF8N(cx, JS::UTF8Chars(reinterpret_cast<char *>(buf_res.unwrap().ptr.get()),
                                              buf_res.unwrap().len)));
  if (!str) {
    return false;
  }

  JS::RootedValue json(cx);
  if (!JS_ParseJSON(cx, str, &json)) {
    return false;
  }

  args.rval().set(json);
  return true;
}

const JSFunctionSpec Acl::static_methods[] = {
    JS_FN("open", Acl::open, 1, JSPROP_ENUMERATE),
};
const JSPropertySpec Acl::static_properties[] = {JS_PS_END};
const JSFunctionSpec Acl::methods[] = {JS_FN("lookup", Acl::lookup, 1, JSPROP_ENUMERATE),
                                       JS_FS_END};
const JSPropertySpec Acl::properties[] = {JS_PS_END};

bool install(api::Engine *engine) {
  if (!Acl::init_class_impl(engine->cx(), engine->global())) {
    return false;
  }

  RootedObject acl_obj(engine->cx(), JS_GetConstructor(engine->cx(), BuiltinImpl<Acl>::proto_obj));
  RootedValue acl_val(engine->cx(), ObjectValue(*acl_obj));
  RootedObject acl_ns(engine->cx(), JS_NewObject(engine->cx(), nullptr));
  if (!JS_SetProperty(engine->cx(), acl_ns, "Acl", acl_val)) {
    return false;
  }

  RootedValue open_val(engine->cx());
  if (!JS_GetProperty(engine->cx(), acl_obj, "open", &open_val)) {
    return false;
  }
  if (!JS_SetProperty(engine->cx(), acl_ns, "open", open_val)) {
    return false;
  }
  // When migrating to full module mode by default
  // if (!JS_DeleteProperty(engine->cx(), acl_obj, "open")) {
  //   return false;
  // }

  RootedValue acl_ns_val(engine->cx(), JS::ObjectValue(*acl_ns));
  if (!engine->define_builtin_module("fastly:acl", acl_ns_val)) {
    return false;
  }
  return true;
}

} // namespace fastly::acl
