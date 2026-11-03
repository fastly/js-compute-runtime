#include "pending-request.h"

#include "request-response.h"
#include "../../../StarlingMonkey/runtime/encode.h"
#include "host_api.h"
#include <string_view>

using builtins::web::fetch::Headers;

namespace fastly::fetch {

namespace {

enum class HeaderOperation : uint8_t { Set, Append, Remove };

JSObject *operations(JSObject *self) {
  auto value = JS::GetReservedSlot(self, std::to_underlying(PendingRequest::Slots::Operations));
  return value.isObject() ? &value.toObject() : nullptr;
}

bool consumed(JSObject *self) {
  return JS::GetReservedSlot(self, std::to_underlying(PendingRequest::Slots::Consumed)).toBoolean();
}

// TODO(StarlingMonkey): Patch StarlingMonkey's header implementation to support case-insensitive header name comparison.
bool header_name_is(const host_api::HostString &name, std::string_view expected) {
  if (name.len != expected.size()) {
    return false;
  }
  for (size_t index = 0; index < name.len; ++index) {
    unsigned char character = static_cast<unsigned char>(name.ptr[index]);
    if (character >= 'A' && character <= 'Z') {
      character = static_cast<unsigned char>(character + ('a' - 'A'));
    }
    if (character != static_cast<unsigned char>(expected[index])) {
      return false;
    }
  }
  return true;
}

bool header_value_contains_fastly(std::string_view value) {
  while (true) {
    size_t separator = value.find(',');
    std::string_view item = value.substr(0, separator);
    size_t first = item.find_first_not_of(" \t");
    if (first != std::string_view::npos && item.substr(first).starts_with("Fastly")) {
      return true;
    }
    if (separator == std::string_view::npos) {
      return false;
    }
    value.remove_prefix(separator + 1);
  }
}

bool operation_kind(JSContext *cx, JS::HandleObject entry, HeaderOperation *kind) {
  JS::RootedValue value(cx);
  if (!JS_GetElement(cx, entry, 0, &value)) {
    return false;
  }
  JS::RootedString string(cx, JS::ToString(cx, value));
  if (!string) {
    return false;
  }
  auto encoded = core::encode(cx, string);
  if (!encoded.ptr) {
    return false;
  }
  std::string_view name(encoded.ptr.get(), encoded.len);
  if (name == "set") {
    *kind = HeaderOperation::Set;
  } else if (name == "append") {
    *kind = HeaderOperation::Append;
  } else {
    *kind = HeaderOperation::Remove;
  }
  return true;
}

bool operation_values(JSContext *cx, JS::HandleObject entry, JS::MutableHandleString name,
                      JS::MutableHandleString value, host_api::HttpPendingReq::ResponseKind *target) {
  JS::RootedValue name_value(cx);
  JS::RootedValue header_value(cx);
  JS::RootedValue target_value(cx);
  if (!JS_GetElement(cx, entry, 1, &name_value) || !JS_GetElement(cx, entry, 2, &header_value) ||
      !JS_GetElement(cx, entry, 3, &target_value)) {
    return false;
  }
  name.set(name_value.toString());
  value.set(header_value.isString() ? header_value.toString() : JS_GetEmptyString(cx));
  *target = static_cast<host_api::HttpPendingReq::ResponseKind>(target_value.toInt32());
  return true;
}

bool parse_target(JSContext *cx, JS::CallArgs args,
                  host_api::HttpPendingReq::ResponseKind *target) {
  if (args.length() < 3 || args[2].isUndefined()) {
    *target = host_api::HttpPendingReq::ResponseKind::Any;
    return true;
  }
  JS::RootedString string(cx, JS::ToString(cx, args[2]));
  if (!string) {
    return false;
  }
  auto encoded = core::encode(cx, string);
  if (!encoded.ptr) {
    return false;
  }
  std::string_view name(encoded.ptr.get(), encoded.len);
  if (name == "any") {
    *target = host_api::HttpPendingReq::ResponseKind::Any;
  } else if (name == "response") {
    *target = host_api::HttpPendingReq::ResponseKind::Response;
  } else if (name == "error") {
    *target = host_api::HttpPendingReq::ResponseKind::Error;
  } else {
    JS_ReportErrorASCII(cx, "PendingRequest response target must be 'any', 'response', or 'error'");
    return false;
  }
  return true;
}

bool add_header_operation(JSContext *cx, JS::CallArgs args, JS::HandleObject self,
                          HeaderOperation kind) {
  if (consumed(self)) {
    JS_ReportErrorASCII(cx, "PendingRequest has already been sent to the client");
    return false;
  }
  if (!args.requireAtLeast(cx, "PendingRequest header method", kind == HeaderOperation::Remove ? 1 : 2)) {
    return false;
  }

  JS::RootedString name(cx, JS::ToString(cx, args[0]));
  JS::RootedString value(cx);
  if (kind != HeaderOperation::Remove) {
    value.set(JS::ToString(cx, args[1]));
    if (!value) {
      return false;
    }
  }
  if (!name) {
    return false;
  }

  host_api::HttpPendingReq::ResponseKind target;
  if (!parse_target(cx, args, &target)) {
    return false;
  }

  JS::RootedObject entry(cx, JS::NewArrayObject(cx, 4));
  if (!entry) {
    return false;
  }
  const char *kind_name = kind == HeaderOperation::Set ? "set" :
                          kind == HeaderOperation::Append ? "append" : "remove";
  JS::RootedString kind_string(cx, JS_NewStringCopyZ(cx, kind_name));
  if (!kind_string) {
    return false;
  }
  JS::RootedValueArray<4> fields(cx);
  fields[0].setString(kind_string);
  fields[1].setString(name);
  fields[2].set(value ? JS::StringValue(value) : JS::UndefinedValue());
  fields[3].setInt32(static_cast<int32_t>(target));
  for (uint32_t index = 0; index < 4; ++index) {
    if (!JS_SetElement(cx, entry, index, fields[index])) {
      return false;
    }
  }

  JS::RootedObject list(cx, operations(self));
  uint32_t length = 0;
  if (!list || !JS::GetArrayLength(cx, list, &length)) {
    return false;
  }
  return JS_SetElement(cx, list, length, entry);
}

bool apply_operations_to_http_headers(JSContext *cx, JS::HandleObject self,
                                     host_api::HttpHeaders *headers,
                                     host_api::HttpPendingReq::ResponseKind requested_target) {
  JS::RootedObject list(cx, operations(self));
  uint32_t length = 0;
  if (!list || !JS::GetArrayLength(cx, list, &length)) {
    return false;
  }
  for (uint32_t index = 0; index < length; ++index) {
    JS::RootedValue entry_value(cx);
    if (!JS_GetElement(cx, list, index, &entry_value) || !entry_value.isObject()) {
      return false;
    }
    JS::RootedObject entry(cx, &entry_value.toObject());
    HeaderOperation kind;
    if (!operation_kind(cx, entry, &kind)) {
      return false;
    }
    JS::RootedString name(cx);
    JS::RootedString value(cx);
    host_api::HttpPendingReq::ResponseKind target;
    if (!operation_values(cx, entry, &name, &value, &target)) {
      return false;
    }
    if (target != host_api::HttpPendingReq::ResponseKind::Any && target != requested_target) {
      continue;
    }
    auto encoded_name = core::encode(cx, name);
    auto encoded_value = core::encode(cx, value);
    if (!encoded_name.ptr || !encoded_value.ptr) {
      return false;
    }
    std::string_view name_view(encoded_name.ptr.get(), encoded_name.len);
    std::string_view value_view(encoded_value.ptr.get(), encoded_value.len);
    auto result = kind == HeaderOperation::Set ? headers->set(name_view, value_view) :
                  kind == HeaderOperation::Append ? headers->append(name_view, value_view) :
                  headers->remove(name_view);
    if (auto *err = result.to_err()) {
      HANDLE_ERROR(cx, *err);
      return false;
    }
  }
  return true;
}

} // namespace

const JSFunctionSpec PendingRequest::static_methods[] = {JS_FS_END};
const JSPropertySpec PendingRequest::static_properties[] = {JS_PS_END};
const JSFunctionSpec PendingRequest::methods[] = {
    JS_FN("setResponseHeader", setResponseHeader, 2, JSPROP_ENUMERATE),
    JS_FN("appendResponseHeader", appendResponseHeader, 2, JSPROP_ENUMERATE),
    JS_FN("removeResponseHeader", removeResponseHeader, 1, JSPROP_ENUMERATE),
    JS_FS_END,
};
const JSPropertySpec PendingRequest::properties[] = {JS_PS_END};

bool PendingRequest::init_class(JSContext *cx, JS::HandleObject global) {
  return BuiltinNoConstructor<PendingRequest>::init_class(cx, global);
}

JSObject *PendingRequest::create_handle(JSContext *cx, host_api::HttpPendingReq::Handle handle) {
  JS::RootedObject self(cx, JS_NewObjectWithGivenProto(cx, &class_, proto_obj));
  if (!self) {
    return nullptr;
  }
  JS::RootedObject list(cx, JS::NewArrayObject(cx, 0));
  if (!list) {
    return nullptr;
  }
  JS::SetReservedSlot(self, std::to_underlying(Slots::Handle), JS::Int32Value(handle));
  JS::SetReservedSlot(self, std::to_underlying(Slots::Promise), JS::UndefinedValue());
  JS::SetReservedSlot(self, std::to_underlying(Slots::Operations), JS::ObjectValue(*list));
  JS::SetReservedSlot(self, std::to_underlying(Slots::Consumed), JS::FalseValue());
  JS::SetReservedSlot(self, std::to_underlying(Slots::GuestCacheMiss), JS::FalseValue());
  JS::SetReservedSlot(self, std::to_underlying(Slots::PreserveSurrogateHeaders), JS::FalseValue());
  return self;
}

JSObject *PendingRequest::create_promise(JSContext *cx, JS::HandleObject promise) {
  JS::RootedObject self(cx, JS_NewObjectWithGivenProto(cx, &class_, proto_obj));
  if (!self) {
    return nullptr;
  }
  JS::RootedObject list(cx, JS::NewArrayObject(cx, 0));
  if (!list) {
    return nullptr;
  }
  JS::SetReservedSlot(self, std::to_underlying(Slots::Handle), JS::Int32Value(-1));
  JS::SetReservedSlot(self, std::to_underlying(Slots::Promise), JS::ObjectValue(*promise));
  JS::SetReservedSlot(self, std::to_underlying(Slots::Operations), JS::ObjectValue(*list));
  JS::SetReservedSlot(self, std::to_underlying(Slots::Consumed), JS::FalseValue());
  JS::SetReservedSlot(self, std::to_underlying(Slots::GuestCacheMiss), JS::FalseValue());
  JS::SetReservedSlot(self, std::to_underlying(Slots::PreserveSurrogateHeaders), JS::FalseValue());
  return self;
}

bool PendingRequest::is_native(JSObject *self) {
  return JS::GetReservedSlot(self, std::to_underlying(Slots::Handle)).toInt32() >= 0;
}

JSObject *PendingRequest::promise(JSObject *self) {
  return JS::GetReservedSlot(self, std::to_underlying(Slots::Promise)).toObjectOrNull();
}

bool PendingRequest::consume(JSContext *cx, JS::HandleObject self) {
  if (consumed(self)) {
    JS_ReportErrorASCII(cx, "PendingRequest has already been sent to the client");
    return false;
  }
  JS::SetReservedSlot(self, std::to_underlying(Slots::Consumed), JS::TrueValue());
  return true;
}

bool PendingRequest::mark_guest_cache_miss_headers(JSContext *cx, JS::HandleObject self,
                                                   JS::HandleObject request) {
  if (!is_native(self)) {
    return false;
  }
  JS::RootedObject headers(cx, Request::headers(cx, request));
  if (!headers) {
    return false;
  }

  if (!Headers::get_list(cx, headers)) {
    return false;
  }

  // Preserve surrogate metadata for internal Fastly requests; external responses should not
  // expose it downstream.
  bool preserve_surrogate_headers = Headers::lookup(cx, headers, "Fastly-FF").has_value() ||
                                    Headers::lookup(cx, headers, "Fastly-Debug").has_value();
  auto cdn_loop_index = Headers::lookup(cx, headers, "CDN-Loop");
  if (!preserve_surrogate_headers && cdn_loop_index.has_value()) {
    // CDN-Loop may be represented by repeated fields, each containing comma-separated entries.
    auto *entry = Headers::get_index(cx, headers, *cdn_loop_index);
    while (header_name_is(std::get<0>(*entry), "cdn-loop")) {
      const auto &header_value = std::get<1>(*entry);
      std::string_view value(header_value.len == 0 ? "" : header_value.ptr.get(), header_value.len);
      if (header_value_contains_fastly(value)) {
        preserve_surrogate_headers = true;
        break;
      }
      ++*cdn_loop_index;
      if (*cdn_loop_index >= Headers::get_list(cx, headers)->size()) {
        break;
      }
      entry = Headers::get_index(cx, headers, *cdn_loop_index);
    }
  }

  // send_to_client applies MISS/0 headers and conditionally strips surrogate headers from this state.
  JS::SetReservedSlot(self, std::to_underlying(Slots::GuestCacheMiss), JS::TrueValue());
  JS::SetReservedSlot(self, std::to_underlying(Slots::PreserveSurrogateHeaders),
                      JS::BooleanValue(preserve_surrogate_headers));
  return true;
}

bool PendingRequest::send_to_client(JSContext *cx, JS::HandleObject self) {
  if (!is_native(self)) {
    JS_ReportErrorASCII(cx, "PendingRequest does not contain a host pending request");
    return false;
  }
  auto handle = static_cast<uint32_t>(
      JS::GetReservedSlot(self, std::to_underlying(Slots::Handle)).toInt32());
  JS::RootedObject list(cx, operations(self));
  uint32_t length = 0;
  if (!list || !JS::GetArrayLength(cx, list, &length)) {
    return false;
  }
  host_api::HttpPendingReq pending(handle);
  for (uint32_t index = 0; index < length; ++index) {
    JS::RootedValue entry_value(cx);
    if (!JS_GetElement(cx, list, index, &entry_value) || !entry_value.isObject()) {
      return false;
    }
    JS::RootedObject entry(cx, &entry_value.toObject());
    HeaderOperation kind;
    if (!operation_kind(cx, entry, &kind)) {
      return false;
    }
    JS::RootedString name(cx);
    JS::RootedString value(cx);
    host_api::HttpPendingReq::ResponseKind target;
    if (!operation_values(cx, entry, &name, &value, &target)) {
      return false;
    }
    auto encoded_name = core::encode(cx, name);
    auto encoded_value = core::encode(cx, value);
    if (!encoded_name.ptr || !encoded_value.ptr) {
      return false;
    }
    std::string_view name_view(encoded_name.ptr.get(), encoded_name.len);
    std::string_view value_view(encoded_value.ptr.get(), encoded_value.len);

    Result<void> result;
    switch(kind) {
        case HeaderOperation::Set:
            result = pending.set_response_header(name_view, value_view, target);
            break;
        case HeaderOperation::Append:
            result = pending.append_response_header(name_view, value_view, target);
            break;
        case HeaderOperation::Remove:
            result = pending.remove_response_header(name_view, target);
            break;
        default:
            return false;
    }
    if (auto *err = result.to_err()) {
      HANDLE_ERROR(cx, *err);
      return false;
    }
  }
  if (JS::GetReservedSlot(self, std::to_underlying(Slots::GuestCacheMiss)).toBoolean()) {
    auto miss_result = pending.set_response_header(
        "x-cache", "MISS", host_api::HttpPendingReq::ResponseKind::Response);
    auto hits_result = pending.set_response_header(
        "x-cache-hits", "0", host_api::HttpPendingReq::ResponseKind::Response);
    if (auto *err = miss_result.to_err()) {
      HANDLE_ERROR(cx, *err);
      return false;
    }
    if (auto *err = hits_result.to_err()) {
      HANDLE_ERROR(cx, *err);
      return false;
    }
    if (!JS::GetReservedSlot(self, std::to_underlying(Slots::PreserveSurrogateHeaders))
             .toBoolean()) {
      auto key_result = pending.remove_response_header(
          "Surrogate-Key", host_api::HttpPendingReq::ResponseKind::Response);
      auto control_result = pending.remove_response_header(
          "Surrogate-Control", host_api::HttpPendingReq::ResponseKind::Response);
      if (auto *err = key_result.to_err()) {
        HANDLE_ERROR(cx, *err);
        return false;
      }
      if (auto *err = control_result.to_err()) {
        HANDLE_ERROR(cx, *err);
        return false;
      }
    }
  }
  auto result = pending.send_to_client();
  if (auto *err = result.to_err()) {
    HANDLE_ERROR(cx, *err);
    return false;
  }
  return true;
}

// Not the most efficient way to apply response headers, but avoids a heavy patch to StarlingMonkey
// TODO(StarlingMonkey): Factor out common header manipulation logic into C++-accessible parts.
bool PendingRequest::apply_response_headers(JSContext *cx, JS::HandleObject self,
                                            JS::HandleObject response, bool error_response) {
  JS::RootedObject list(cx, operations(self));
  JS::RootedObject headers(cx, Response::headers(cx, response));
  if (!list || !headers) {
    return false;
  }
  uint32_t length = 0;
  if (!JS::GetArrayLength(cx, list, &length)) {
    return false;
  }
  auto requested_target = error_response ? host_api::HttpPendingReq::ResponseKind::Error
                                         : host_api::HttpPendingReq::ResponseKind::Response;
  for (uint32_t index = 0; index < length; ++index) {
    JS::RootedValue entry_value(cx);
    if (!JS_GetElement(cx, list, index, &entry_value) || !entry_value.isObject()) {
      return false;
    }
    JS::RootedObject entry(cx, &entry_value.toObject());
    HeaderOperation kind;
    if (!operation_kind(cx, entry, &kind)) {
      return false;
    }
    JS::RootedString name(cx);
    JS::RootedString value(cx);
    host_api::HttpPendingReq::ResponseKind target;
    if (!operation_values(cx, entry, &name, &value, &target)) {
      return false;
    }
    if (target != host_api::HttpPendingReq::ResponseKind::Any && target != requested_target) {
      continue;
    }
    const char *method_name;
    switch (kind) {
        case HeaderOperation::Set:
            method_name = "set";
            break;
        case HeaderOperation::Append:
            method_name = "append";
            break;
        case HeaderOperation::Remove:
            method_name = "delete";
            break;
        default:
            return false;
    }
    JS::RootedValue method(cx);
    if (!JS_GetProperty(cx, headers, method_name, &method)) {
      return false;
    }
    JS::RootedValueArray<2> method_args(cx);
    method_args[0].setString(name);
    uint32_t argc = 1;
    if (kind != HeaderOperation::Remove) {
      method_args[1].setString(value);
      argc = 2;
    }
    JS::RootedValue result(cx);
    JS::RootedValue this_value(cx, JS::ObjectValue(*headers));
    if (!JS::Call(cx, this_value, method,
            JS::HandleValueArray::subarray(method_args, 0, argc), &result)) {
      return false;
    }
  }
  return true;
}

bool PendingRequest::apply_error_headers(JSContext *cx, JS::HandleObject self,
                                         host_api::HttpResp &response) {
  auto *headers = response.headers_writable();
  if (!headers) {
    return false;
  }
  return apply_operations_to_http_headers(cx, self, headers,
                                          host_api::HttpPendingReq::ResponseKind::Error);
}

bool PendingRequest::setResponseHeader(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(3)
  args.rval().setUndefined();
  return add_header_operation(cx, args, self, HeaderOperation::Set);
}

bool PendingRequest::appendResponseHeader(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(3)
  args.rval().setUndefined();
  return add_header_operation(cx, args, self, HeaderOperation::Append);
}

bool PendingRequest::removeResponseHeader(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(2)
  args.rval().setUndefined();
  return add_header_operation(cx, args, self, HeaderOperation::Remove);
}

} // namespace fastly::fetch