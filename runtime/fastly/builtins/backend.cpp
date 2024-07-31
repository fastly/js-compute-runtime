#include <algorithm>
#include <arpa/inet.h>
#include <cctype>
#include <charconv>
#include <iostream>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

// TODO: remove these once the warnings are fixed
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-offsetof"
#pragma clang diagnostic ignored "-Wdeprecated-enum-enum-conversion"
#include "js/experimental/TypedData.h"
#pragma clang diagnostic pop

#include "../host-api/host_api_fastly.h"
#include "./fetch/request-response.h"
#include "./secret-store.h"
#include "backend.h"
#include "builtin.h"
#include "encode.h"
#include "fastly.h"

using builtins::BuiltinImpl;
using fastly::fastly::Fastly;
using fastly::fastly::FastlyGetErrorMessage;
using fastly::fetch::RequestOrResponse;
using fastly::secret_store::SecretStoreEntry;

namespace fastly::backend {

std::vector<std::string_view> split(std::string_view string, char delimiter) {
  auto start = 0;
  auto end = string.find(delimiter, start);
  std::vector<std::string_view> result;
  while (end != std::string::npos) {
    result.push_back(string.substr(start, end - start));
    start = end + 1;
    end = string.find(delimiter, start);
  }
  result.push_back(string.substr(start));
  return result;
}

bool is_valid_ip(std::string_view ip) {
  int format = AF_INET;
  if (ip.find(':') != std::string::npos) {
    format = AF_INET6;
  }

  char octets[sizeof(struct in6_addr)];
  if (inet_pton(format, ip.data(), octets) != 1) {
    return false;
  }
  return true;
}

// A "host" is a "hostname" and an optional "port" in the format hostname:port
// A "hostname" is between 1 and 255 octets -- https://www.rfc-editor.org/rfc/rfc1123#page-13
// A "hostname" must start with a letter or digit -- https://www.rfc-editor.org/rfc/rfc1123#page-13
// A "hostname" is made up of "labels" delimited by a dot `.`
// A "label" is between 1 and 63 octets
bool is_valid_host(std::string_view host) {
  if (host.length() < 1) {
    return false;
  }
  auto first_character = host.front();
  // check first character is in the regex [a-zA-Z0-9]
  if (!std::isalnum(first_character)) {
    return false;
  }
  // split the hostname from the port
  int pos = host.find_first_of(':');
  std::string_view hostname = host.substr(0, pos);

  // hostnames can not be longer than 253 characters
  // This is because a hostname is represented as a series of labels, and is terminated by a label
  // of length zero. A label consists of a length octet followed by that number of octets
  // representing the name itself. https://www.rfc-editor.org/rfc/rfc1035#section-3.3
  // https://www.rfc-editor.org/rfc/rfc2181#section-11
  if (hostname.length() > 253) {
    return false;
  }

  auto last_character = hostname.back();
  // check last character is in the regex [a-zA-Z0-9]
  if (!std::isalnum(last_character)) {
    return false;
  }

  auto labels = split(hostname, '.');

  for (auto &label : labels) {
    // Each label in a hostname can not be longer than 63 characters
    // https://www.rfc-editor.org/rfc/rfc2181#section-11
    if (label.length() > 63) {
      return false;
    }

    // Each label can only contain the characters in the regex [a-zA-Z0-9\-]
    auto it = std::find_if_not(label.begin(), label.end(), [](auto character) {
      return std::isalnum(character) || character == '-';
    });
    if (it != label.end()) {

      return false;
    }
  }

  // if there is a port - confirm it is all digits and is between 0 and 65536
  // https://www.iana.org/assignments/service-names-port-numbers/service-names-port-numbers.xhtml
  if (pos != std::string::npos) {
    std::string_view port = host.substr(pos + 1);
    if (!std::all_of(port.begin(), port.end(), [](auto c) { return std::isdigit(c); })) {
      return false;
    }
    int value;
    const std::from_chars_result result =
        std::from_chars(port.data(), port.data() + port.size(), value);
    if (result.ec == std::errc::invalid_argument || result.ec == std::errc::result_out_of_range) {
      return false;
    }
    if (value == 0 || value >= 65536) {
      return false;
    }
  }
  return true;
}

JS::Result<mozilla::Ok> Backend::register_dynamic_backend(JSContext *cx, JS::HandleObject backend) {
  MOZ_ASSERT(is_instance(backend));

  JS::RootedString name(cx, JS::GetReservedSlot(backend, Backend::Slots::Name).toString());
  auto name_chars = core::encode(cx, name);
  std::string_view name_str = name_chars;

  JS::RootedString target(cx, JS::GetReservedSlot(backend, Backend::Slots::Target).toString());
  auto target_chars = core::encode(cx, target);
  std::string_view target_str = target_chars;

  host_api::BackendConfig backend_config;

  auto host_override_slot = JS::GetReservedSlot(backend, Backend::Slots::HostOverride);
  if (!host_override_slot.isNullOrUndefined()) {
    JS::RootedString host_override(cx, host_override_slot.toString());
    auto host_override_chars = core::encode(cx, host_override);
    backend_config.host_override.emplace(std::move(host_override_chars));
  }

  auto connect_timeout_slot = JS::GetReservedSlot(backend, Backend::Slots::ConnectTimeout);
  if (!connect_timeout_slot.isNullOrUndefined()) {
    backend_config.connect_timeout = connect_timeout_slot.toInt32();
  }

  auto first_byte_timeout_slot = JS::GetReservedSlot(backend, Backend::Slots::FirstByteTimeout);
  if (!first_byte_timeout_slot.isNullOrUndefined()) {
    backend_config.first_byte_timeout = first_byte_timeout_slot.toInt32();
  }

  auto between_bytes_timeout_slot =
      JS::GetReservedSlot(backend, Backend::Slots::BetweenBytesTimeout);
  if (!between_bytes_timeout_slot.isNullOrUndefined()) {
    backend_config.between_bytes_timeout = between_bytes_timeout_slot.toInt32();
  }

  auto use_ssl_slot = JS::GetReservedSlot(backend, Backend::Slots::UseSsl);
  if (!use_ssl_slot.isNullOrUndefined()) {
    backend_config.use_ssl = use_ssl_slot.toBoolean();
  }

  auto dont_pool_slot = JS::GetReservedSlot(backend, Backend::Slots::DontPool);
  if (!dont_pool_slot.isNullOrUndefined()) {
    backend_config.dont_pool = dont_pool_slot.toBoolean();
  }

  auto tls_min_version = JS::GetReservedSlot(backend, Backend::Slots::TlsMinVersion);
  if (!tls_min_version.isNullOrUndefined()) {
    backend_config.ssl_min_version = host_api::TlsVersion(tls_min_version.toInt32());
  }

  auto tls_max_version = JS::GetReservedSlot(backend, Backend::Slots::TlsMaxVersion);
  if (!tls_max_version.isNullOrUndefined()) {
    backend_config.ssl_max_version = host_api::TlsVersion(tls_max_version.toInt32());
  }

  auto certificate_hostname_slot =
      JS::GetReservedSlot(backend, Backend::Slots::CertificateHostname);
  if (!certificate_hostname_slot.isNullOrUndefined()) {
    MOZ_ASSERT(certificate_hostname_slot.isString());
    JS::RootedString certificate_hostname_string(cx, certificate_hostname_slot.toString());
    auto certificate_hostname_chars = core::encode(cx, certificate_hostname_string);
    backend_config.cert_hostname.emplace(std::move(certificate_hostname_chars));
  }

  auto ca_certificate_slot = JS::GetReservedSlot(backend, Backend::Slots::CaCertificate);
  if (!ca_certificate_slot.isNullOrUndefined()) {
    MOZ_ASSERT(ca_certificate_slot.isString());
    JS::RootedString ca_certificate_string(cx, ca_certificate_slot.toString());
    auto ca_certificate_chars = core::encode(cx, ca_certificate_string);
    backend_config.ca_cert.emplace(std::move(ca_certificate_chars));
  }

  auto ciphers_slot = JS::GetReservedSlot(backend, Backend::Slots::Ciphers);
  if (!ciphers_slot.isNullOrUndefined()) {
    MOZ_ASSERT(ciphers_slot.isString());
    JS::RootedString ciphers_string(cx, ciphers_slot.toString());
    auto ciphers_chars = core::encode(cx, ciphers_string);
    backend_config.ciphers.emplace(std::move(ciphers_chars));
  }

  auto sni_hostname_slot = JS::GetReservedSlot(backend, Backend::Slots::SniHostname);
  if (!sni_hostname_slot.isNullOrUndefined()) {
    MOZ_ASSERT(sni_hostname_slot.isString());
    JS::RootedString sni_hostname_string(cx, sni_hostname_slot.toString());
    auto sni_hostname_chars = core::encode(cx, sni_hostname_string);
    backend_config.sni_hostname.emplace(std::move(sni_hostname_chars));
  }

  auto client_cert_slot = JS::GetReservedSlot(backend, Backend::Slots::ClientCert);
  if (!client_cert_slot.isNullOrUndefined()) {
    MOZ_ASSERT(client_cert_slot.isString());
    JS::RootedString client_cert_string(cx, client_cert_slot.toString());
    auto client_cert_chars = core::encode(cx, client_cert_string);

    auto client_cert_key_slot = JS::GetReservedSlot(backend, Backend::Slots::ClientCertKey);

    backend_config.client_cert = host_api::ClientCert{
        .cert = std::move(client_cert_chars), .key = (FastlyHandle)client_cert_key_slot.toInt32()};
  }

  auto res = host_api::HttpReq::register_dynamic_backend(name_str, target_str, backend_config);
  if (auto *err = res.to_err()) {
    HANDLE_ERROR(cx, *err);
    return JS::Result<mozilla::Ok>(JS::Error());
  }
  return mozilla::Ok();
}

JSString *Backend::name(JSContext *cx, JSObject *self) {
  MOZ_ASSERT(is_instance(self));
  return JS::GetReservedSlot(self, Backend::Slots::Name).toString();
}

bool Backend::to_string(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0);
  JS::RootedString name(cx, JS::GetReservedSlot(self, Backend::Slots::Name).toString());
  args.rval().setString(name);
  return true;
}

namespace {
host_api::HostString parse_and_validate_name(JSContext *cx, JS::HandleValue name_val) {
  if (name_val.isNullOrUndefined()) {
    JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr, JSMSG_BACKEND_NAME_NOT_SET);
    return nullptr;
  }
  JS::RootedString name(cx, JS::ToString(cx, name_val));
  if (!name) {
    return nullptr;
  }
  auto length = JS::GetStringLength(name);
  if (length > 254) {
    JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr, JSMSG_BACKEND_NAME_TOO_LONG);
    return nullptr;
  }
  if (length == 0) {
    JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr, JSMSG_BACKEND_NAME_EMPTY);
    return nullptr;
  }
  return core::encode(cx, name);
}
} // namespace

bool Backend::exists(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  if (!args.requireAtLeast(cx, "Backend.exists", 1)) {
    return false;
  }

  auto name = parse_and_validate_name(cx, args.get(0));
  if (!name) {
    return false;
  }
  auto res = host_api::Backend::exists(name);
  if (auto *err = res.to_err()) {
    HANDLE_ERROR(cx, *err);
    return false;
  }
  auto exists = res.unwrap();
  args.rval().setBoolean(exists);
  return true;
}

bool Backend::from_name(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  if (!args.requireAtLeast(cx, "Backend.fromName", 1)) {
    return false;
  }

  auto name = parse_and_validate_name(cx, args.get(0));
  if (!name) {
    return false;
  }
  auto res = host_api::Backend::exists(name);
  if (auto *err = res.to_err()) {
    HANDLE_ERROR(cx, *err);
    return false;
  }
  auto exists = res.unwrap();

  if (!exists) {
    JS_ReportErrorNumberUTF8(cx, FastlyGetErrorMessage, nullptr,
                             JSMSG_BACKEND_FROMNAME_BACKEND_DOES_NOT_EXIST, name.begin());
    return false;
  }

  auto backend_instance = JS_NewObjectWithGivenProto(cx, &Backend::class_, Backend::proto_obj);
  if (!backend_instance) {
    return false;
  }
  JS::RootedValue backend_val(cx, JS::ObjectValue(*backend_instance));
  JS::RootedObject backend(cx, backend_instance);
  if (!backend) {
    return false;
  }

  JS::RootedValue name_val(cx, JS::StringValue(JS_NewStringCopyZ(cx, name.begin())));
  if (!Backend::set_name(cx, backend, name_val)) {
    return false;
  }

  args.rval().setObject(*backend);
  return true;
}

bool Backend::health(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  if (!args.requireAtLeast(cx, "Backend.health", 1)) {
    return false;
  }

  auto name = parse_and_validate_name(cx, args.get(0));
  if (!name) {
    return false;
  }
  auto exists = host_api::Backend::exists(name);
  if (auto *err = exists.to_err()) {
    HANDLE_ERROR(cx, *err);
    return false;
  }

  if (!exists.unwrap()) {
    JS_ReportErrorNumberUTF8(cx, FastlyGetErrorMessage, nullptr,
                             JSMSG_BACKEND_IS_HEALTHY_BACKEND_DOES_NOT_EXIST, name.begin());
    return false;
  }

  auto res = host_api::Backend::health(name);
  if (auto *err = res.to_err()) {
    HANDLE_ERROR(cx, *err);
    return false;
  }

  auto health = res.unwrap();
  if (health.is_healthy()) {
    args.rval().setString(JS_NewStringCopyZ(cx, "healthy"));
  } else if (health.is_unhealthy()) {
    args.rval().setString(JS_NewStringCopyZ(cx, "unhealthy"));
  } else {
    args.rval().setString(JS_NewStringCopyZ(cx, "unknown"));
  }

  return true;
}

const JSFunctionSpec Backend::static_methods[] = {
    JS_FN("exists", exists, 1, JSPROP_ENUMERATE), JS_FN("fromName", from_name, 1, JSPROP_ENUMERATE),
    JS_FN("health", health, 1, JSPROP_ENUMERATE), JS_FS_END};
const JSPropertySpec Backend::static_properties[] = {JS_PS_END};
const JSFunctionSpec Backend::methods[] = {JS_FN("toString", to_string, 0, JSPROP_ENUMERATE),
                                           JS_FN("toName", to_string, 0, JSPROP_ENUMERATE),
                                           JS_FS_END};
const JSPropertySpec Backend::properties[] = {JS_PS_END};

bool Backend::set_name(JSContext *cx, JSObject *backend, JS::HandleValue name_val) {
  MOZ_ASSERT(is_instance(backend));
  auto name = parse_and_validate_name(cx, name_val);
  if (!name) {
    return false;
  }

  JS::SetReservedSlot(backend, Backend::Slots::Name,
                      JS::StringValue(JS_NewStringCopyZ(cx, name.begin())));
  return true;
}

bool Backend::set_host_override(JSContext *cx, JSObject *backend,
                                JS::HandleValue host_override_val) {
  MOZ_ASSERT(is_instance(backend));
  auto host_override = JS::ToString(cx, host_override_val);
  if (!host_override) {
    return false;
  }

  if (JS_GetStringLength(host_override) == 0) {
    JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr,
                              JSMSG_BACKEND_HOST_OVERRIDE_EMPTY);
    return false;
  }
  JS::SetReservedSlot(backend, Backend::Slots::HostOverride, JS::StringValue(host_override));
  return true;
}

bool Backend::set_sni_hostname(JSContext *cx, JSObject *backend, JS::HandleValue sni_hostname_val) {
  auto sni_hostname = JS::ToString(cx, sni_hostname_val);
  if (!sni_hostname) {
    return false;
  }

  if (JS_GetStringLength(sni_hostname) == 0) {
    JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr, JSMSG_BACKEND_SNI_HOSTNAME_EMPTY);
    return false;
  }
  JS::SetReservedSlot(backend, Backend::Slots::SniHostname, JS::StringValue(sni_hostname));
  return true;
}

bool Backend::set_client_cert(JSContext *cx, JSObject *backend, JS::HandleValue client_cert_val) {
  auto client_cert = JS::ToString(cx, client_cert_val);
  if (!client_cert) {
    return false;
  }

  if (JS_GetStringLength(client_cert) == 0) {
    JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr,
                              JSMSG_BACKEND_CLIENT_CERTIFICATE_CERTIFICATE_EMPTY);
    return false;
  }
  JS::SetReservedSlot(backend, Backend::Slots::ClientCert, JS::StringValue(client_cert));
  return true;
}

bool Backend::set_client_cert_key(JSContext *cx, JSObject *backend,
                                  JS::HandleValue client_cert_key_val) {
  if (!SecretStoreEntry::is_instance(client_cert_key_val)) {
    JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr,
                              JSMSG_BACKEND_CLIENT_CERTIFICATE_KEY_INVALID);
    return false;
  }
  JS::RootedObject client_cert_key_obj(cx, &client_cert_key_val.toObject());
  JS::SetReservedSlot(backend, Backend::Slots::ClientCertKey,
                      JS::Int32Value(SecretStoreEntry::secret_handle(client_cert_key_obj).handle));
  return true;
}

/// Timeouts for backends must be less than 2^32 milliseconds, or
/// about a month and a half.
bool Backend::set_timeout_slot(JSContext *cx, JSObject *backend, JS::HandleValue value,
                               Backend::Slots slot, std::string property_name) {
  double native_value;
  if (!JS::ToNumber(cx, value, &native_value)) {
    return false;
  }
  int64_t timeout = std::round(native_value);
  if (timeout < 0) {
    JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr, JSMSG_BACKEND_TIMEOUT_NEGATIVE,
                              property_name.c_str());
    return false;
  }
  if (timeout >= std::pow(2, 32)) {
    JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr, JSMSG_BACKEND_TIMEOUT_TOO_BIG,
                              property_name.c_str());
    return false;
  }
  JS::SetReservedSlot(backend, slot, JS::Int32Value(timeout));
  return true;
}

bool Backend::set_target(JSContext *cx, JSObject *backend, JS::HandleValue target_val) {
  if (target_val.isNullOrUndefined()) {
    JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr, JSMSG_BACKEND_TARGET_NOT_SET);
    return false;
  }

  auto target_string_slice = core::encode_spec_string(cx, target_val);
  if (!target_string_slice.data) {
    return false;
  }

  std::string_view target_string(reinterpret_cast<char *>(target_string_slice.data),
                                 target_string_slice.len);
  auto length = target_string.length();
  if (length == 0) {
    JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr, JSMSG_BACKEND_TARGET_EMPTY);
    return false;
  }

  if (target_string == "::") {
    JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr, JSMSG_BACKEND_TARGET_INVALID);
    return false;
  }
  if (!is_valid_host(target_string) && !is_valid_ip(target_string)) {
    JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr, JSMSG_BACKEND_TARGET_INVALID);
    return false;
  }

  auto target_str = JS_NewStringCopyN(cx, target_string.data(), target_string.length());
  if (!target_str) {
    return false;
  }
  JS::RootedValue target(cx, JS::StringValue(target_str));
  JS::SetReservedSlot(backend, Backend::Slots::Target, target);
  return true;
}

JSObject *Backend::create(JSContext *cx, JS::HandleObject request) {
  JS::RootedValue request_url(cx, RequestOrResponse::url(request));
  auto url_string = core::encode_spec_string(cx, request_url);
  if (!url_string.data) {
    return nullptr;
  }

  auto url = jsurl::new_jsurl(&url_string);
  if (!url) {
    JS_ReportErrorUTF8(cx, "URL constructor: %s is not a valid URL.", (char *)url_string.data);
    return nullptr;
  }
  const jsurl::SpecSlice slice = jsurl::host(url);
  auto name_js_str = JS_NewStringCopyN(cx, (char *)slice.data, slice.len);
  if (!name_js_str) {
    return nullptr;
  }
  std::string name_str((char *)slice.data, slice.len);

  // Check if we already constructed an implicit dynamic backend for this host.
  bool found;
  JS::RootedValue already_built_backend(cx);
  if (!JS_HasProperty(cx, Backend::backends, name_str.c_str(), &found)) {
    return nullptr;
  }
  if (found) {
    if (!JS_GetProperty(cx, Backend::backends, name_str.c_str(), &already_built_backend)) {
      return nullptr;
    }
    JS::RootedObject backend(cx, &already_built_backend.toObject());
    return backend;
  }

  auto backend_instance = JS_NewObjectWithGivenProto(cx, &Backend::class_, Backend::proto_obj);
  if (!backend_instance) {
    return nullptr;
  }
  JS::RootedValue backend_val(cx, JS::ObjectValue(*backend_instance));
  JS::RootedObject backend(cx, backend_instance);
  if (!backend) {
    return nullptr;
  }

  JS::RootedValue name(cx, JS::StringValue(name_js_str));
  if (!Backend::set_name(cx, backend, name)) {
    return nullptr;
  }
  if (!Backend::set_host_override(cx, backend, name)) {
    return nullptr;
  }
  if (!Backend::set_target(cx, backend, name)) {
    return nullptr;
  }
  const jsurl::SpecString origin_specstring = jsurl::origin(url);
  std::string_view origin((char *)origin_specstring.data, origin_specstring.len);

  auto use_ssl = origin.rfind("https://", 0) == 0;
  JS::SetReservedSlot(backend, Backend::Slots::UseSsl, JS::BooleanValue(use_ssl));
  if (use_ssl) {
    if (!Backend::set_sni_hostname(cx, backend, name)) {
      return nullptr;
    }
  }

  JS::SetReservedSlot(backend, Backend::Slots::DontPool, JS::BooleanValue(false));

  if (Fastly::defaultDynamicBackendConfig.connect_timeout.has_value()) {
    JS::RootedValue connect_timeout_val(
        cx, JS::NumberValue(Fastly::defaultDynamicBackendConfig.connect_timeout.value()));
    if (!Backend::set_timeout_slot(cx, backend, connect_timeout_val, Backend::Slots::ConnectTimeout,
                                   "connectTimeout")) {
      return nullptr;
    }
  }
  if (Fastly::defaultDynamicBackendConfig.between_bytes_timeout.has_value()) {
    JS::RootedValue between_bytes_timeout_val(
        cx, JS::NumberValue(Fastly::defaultDynamicBackendConfig.between_bytes_timeout.value()));
    if (!Backend::set_timeout_slot(cx, backend, between_bytes_timeout_val,
                                   Backend::Slots::BetweenBytesTimeout, "betweenBytesTimeout")) {
      return nullptr;
    }
  }
  if (Fastly::defaultDynamicBackendConfig.first_byte_timeout.has_value()) {
    JS::RootedValue first_byte_timeout_val(
        cx, JS::NumberValue(Fastly::defaultDynamicBackendConfig.first_byte_timeout.value()));
    if (!Backend::set_timeout_slot(cx, backend, first_byte_timeout_val,
                                   Backend::Slots::FirstByteTimeout, "firstByteTimeout")) {
      return nullptr;
    }
  }

  auto result = Backend::register_dynamic_backend(cx, backend);
  if (result.isErr()) {
    return nullptr;
  } else {
    if (!JS_SetProperty(cx, Backend::backends, name_str.c_str(), backend_val)) {
      return nullptr;
    }
    return backend;
  }
}

bool Backend::constructor(JSContext *cx, unsigned argc, JS::Value *vp) {
  REQUEST_HANDLER_ONLY("The Backend builtin");
  CTOR_HEADER("Backend", 1);

  auto configuration_parameter = args.get(0);

  if (!configuration_parameter.isObject()) {
    JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr,
                              JSMSG_BACKEND_PARAMETER_NOT_OBJECT);
    return false;
  }

  JS::RootedObject backend(cx, JS_NewObjectForConstructor(cx, &class_, args));
  if (!backend) {
    return false;
  }

  JS::RootedObject configuration(cx, &configuration_parameter.toObject());

  JS::RootedValue name_val(cx);
  if (!JS_GetProperty(cx, configuration, "name", &name_val)) {
    return false;
  }
  if (!Backend::set_name(cx, backend, name_val)) {
    return false;
  }

  JS::RootedValue target_val(cx);
  if (!JS_GetProperty(cx, configuration, "target", &target_val)) {
    return false;
  }
  if (!Backend::set_target(cx, backend, target_val)) {
    return false;
  }

  bool found;
  JS::RootedValue host_override_val(cx);
  if (!JS_HasProperty(cx, configuration, "hostOverride", &found)) {
    return false;
  }
  if (found) {
    if (!JS_GetProperty(cx, configuration, "hostOverride", &host_override_val)) {
      return false;
    }
    if (!Backend::set_host_override(cx, backend, host_override_val)) {
      return false;
    }
  }

  JS::RootedValue connect_timeout_val(cx);
  if (!JS_HasProperty(cx, configuration, "connectTimeout", &found)) {
    return false;
  }
  if (found) {
    if (!JS_GetProperty(cx, configuration, "connectTimeout", &connect_timeout_val)) {
      return false;
    }
    if (!Backend::set_timeout_slot(cx, backend, connect_timeout_val, Backend::Slots::ConnectTimeout,
                                   "connectTimeout")) {
      return false;
    }
  }

  /// Timeouts for backends must be less than 2^32 milliseconds, or
  /// about a month and a half.
  JS::RootedValue first_byte_timeout_val(cx);
  if (!JS_HasProperty(cx, configuration, "firstByteTimeout", &found)) {
    return false;
  }
  if (found) {
    if (!JS_GetProperty(cx, configuration, "firstByteTimeout", &first_byte_timeout_val)) {
      return false;
    }
    if (!Backend::set_timeout_slot(cx, backend, first_byte_timeout_val,
                                   Backend::Slots::FirstByteTimeout, "firstByteTimeout")) {
      return false;
    }
  }

  /// Timeouts for backends must be less than 2^32 milliseconds, or
  /// about a month and a half.
  JS::RootedValue between_bytes_timeout_val(cx);
  if (!JS_HasProperty(cx, configuration, "betweenBytesTimeout", &found)) {
    return false;
  }
  if (found) {
    if (!JS_GetProperty(cx, configuration, "betweenBytesTimeout", &between_bytes_timeout_val)) {
      return false;
    }
    if (!Backend::set_timeout_slot(cx, backend, between_bytes_timeout_val,
                                   Backend::Slots::BetweenBytesTimeout, "betweenBytesTimeout")) {
      return false;
    }
  }

  /// Has to be either: 1; 1.1; 1.2; 1.3;
  JS::RootedValue tls_min_version_val(cx);
  std::optional<host_api::TlsVersion> tls_min_version;
  if (!JS_HasProperty(cx, configuration, "tlsMinVersion", &found)) {
    return false;
  }
  if (found) {
    if (!JS_GetProperty(cx, configuration, "tlsMinVersion", &tls_min_version_val)) {
      return false;
    }
    double version;
    if (!JS::ToNumber(cx, tls_min_version_val, &version)) {
      return false;
    }

    if (std::isnan(version)) {
      JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr, JSMSG_BACKEND_TLS_MIN_INVALID);
      return false;
    }

    if (version == 1.3) {
      tls_min_version = host_api::TlsVersion::version_1_3();
    } else if (version == 1.2) {
      tls_min_version = host_api::TlsVersion::version_1_2();
    } else if (version == 1.1) {
      tls_min_version = host_api::TlsVersion::version_1_1();
    } else if (version == 1) {
      tls_min_version = host_api::TlsVersion::version_1();
    } else {
      JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr, JSMSG_BACKEND_TLS_MIN_INVALID);
      return false;
    }
    JS::SetReservedSlot(backend, Backend::Slots::TlsMinVersion,
                        JS::Int32Value(tls_min_version->value));
  }

  /// Has to be either: 1; 1.1; 1.2; 1.3;
  JS::RootedValue tls_max_version_val(cx);
  std::optional<host_api::TlsVersion> tls_max_version;
  if (!JS_HasProperty(cx, configuration, "tlsMaxVersion", &found)) {
    return false;
  }
  if (found) {
    if (!JS_GetProperty(cx, configuration, "tlsMaxVersion", &tls_max_version_val)) {
      return false;
    }
    double version;
    if (!JS::ToNumber(cx, tls_max_version_val, &version)) {
      return false;
    }

    if (std::isnan(version)) {
      JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr, JSMSG_BACKEND_TLS_MAX_INVALID);
      return false;
    }

    if (version == 1.3) {
      tls_max_version = host_api::TlsVersion::version_1_3();
    } else if (version == 1.2) {
      tls_max_version = host_api::TlsVersion::version_1_2();
    } else if (version == 1.1) {
      tls_max_version = host_api::TlsVersion::version_1_1();
    } else if (version == 1) {
      tls_max_version = host_api::TlsVersion::version_1();
    } else {
      JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr, JSMSG_BACKEND_TLS_MAX_INVALID);
      return false;
    }
    JS::SetReservedSlot(backend, Backend::Slots::TlsMaxVersion,
                        JS::Int32Value(tls_max_version->value));
  }

  if (tls_min_version.has_value() && tls_max_version.has_value()) {
    if (tls_min_version->value > tls_max_version->value) {
      JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr,
                                JSMSG_BACKEND_TLS_MIN_GREATER_THAN_TLS_MAX);
      return false;
    }
  }

  JS::RootedValue certificate_hostname_val(cx);
  if (!JS_HasProperty(cx, configuration, "certificateHostname", &found)) {
    return false;
  }
  if (found) {
    if (!JS_GetProperty(cx, configuration, "certificateHostname", &certificate_hostname_val)) {
      return false;
    }
    auto certificate_hostname = JS::ToString(cx, certificate_hostname_val);
    if (!certificate_hostname) {
      return false;
    }

    if (JS_GetStringLength(certificate_hostname) == 0) {
      JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr,
                                JSMSG_BACKEND_CERTIFICATE_HOSTNAME_EMPTY);
      return false;
    }
    JS::SetReservedSlot(backend, Backend::Slots::CertificateHostname,
                        JS::StringValue(certificate_hostname));
  }

  JS::RootedValue use_ssl_val(cx);
  if (!JS_HasProperty(cx, configuration, "useSSL", &found)) {
    return false;
  }
  if (found) {
    if (!JS_GetProperty(cx, configuration, "useSSL", &use_ssl_val)) {
      return false;
    }
    auto value = JS::ToBoolean(use_ssl_val);
    JS::SetReservedSlot(backend, Backend::Slots::UseSsl, JS::BooleanValue(value));
  }

  JS::RootedValue dont_pool_val(cx);
  if (!JS_HasProperty(cx, configuration, "dontPool", &found)) {
    return false;
  }
  if (found) {
    if (!JS_GetProperty(cx, configuration, "dontPool", &dont_pool_val)) {
      return false;
    }
    auto value = JS::ToBoolean(dont_pool_val);
    JS::SetReservedSlot(backend, Backend::Slots::DontPool, JS::BooleanValue(value));
  }

  JS::RootedValue ca_certificate_val(cx);
  if (!JS_HasProperty(cx, configuration, "caCertificate", &found)) {
    return false;
  }
  if (found) {
    if (!JS_GetProperty(cx, configuration, "caCertificate", &ca_certificate_val)) {
      return false;
    }
    auto ca_certificate = JS::ToString(cx, ca_certificate_val);
    if (!ca_certificate) {
      return false;
    }
    if (JS_GetStringLength(ca_certificate) == 0) {
      JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr,
                                JSMSG_BACKEND_CA_CERTIFICATE_EMPTY);
      return false;
    }
    JS::SetReservedSlot(backend, Backend::Slots::CaCertificate, JS::StringValue(ca_certificate));
  }

  /// Cipher list consisting of one or more cipher strings separated by colons.
  // Commas or spaces are also acceptable separators but colons are normally used.
  JS::RootedValue ciphers_val(cx);
  if (!JS_HasProperty(cx, configuration, "ciphers", &found)) {
    return false;
  }
  if (found) {
    if (!JS_GetProperty(cx, configuration, "ciphers", &ciphers_val)) {
      return false;
    }
    auto ciphers_chars = core::encode(cx, ciphers_val);
    if (!ciphers_chars) {
      return false;
    }
    if (ciphers_chars.size() == 0) {
      JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr, JSMSG_BACKEND_CIPHERS_EMPTY);
      return false;
    }
    std::string cipher_spec(ciphers_chars.begin(), ciphers_chars.len);
    JS::SetReservedSlot(
        backend, Backend::Slots::Ciphers,
        JS::StringValue(JS_NewStringCopyN(cx, ciphers_chars.begin(), ciphers_chars.len)));
  }

  JS::RootedValue sni_hostname_val(cx);
  if (!JS_HasProperty(cx, configuration, "sniHostname", &found)) {
    return false;
  }
  if (found) {
    if (!JS_GetProperty(cx, configuration, "sniHostname", &sni_hostname_val)) {
      return false;
    }
    if (!Backend::set_sni_hostname(cx, backend, sni_hostname_val)) {
      return false;
    }
  }

  JS::RootedValue client_cert_val(cx);
  if (!JS_HasProperty(cx, configuration, "clientCertificate", &found)) {
    return false;
  }
  if (found) {
    if (!JS_GetProperty(cx, configuration, "clientCertificate", &client_cert_val)) {
      return false;
    }
    if (!client_cert_val.isObject()) {
      JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr,
                                JSMSG_BACKEND_CLIENT_CERTIFICATE_NOT_OBJECT);
      return false;
    }
    JS::RootedObject client_cert_obj(cx, &client_cert_val.toObject());

    JS::RootedValue client_cert_cert_val(cx);
    if (!JS_HasProperty(cx, client_cert_obj, "certificate", &found)) {
      return false;
    }
    if (!found) {
      JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr,
                                JSMSG_BACKEND_CLIENT_CERTIFICATE_NO_CERTIFICATE);
      return false;
    }
    if (!JS_GetProperty(cx, client_cert_obj, "certificate", &client_cert_cert_val)) {
      return false;
    }
    if (!Backend::set_client_cert(cx, backend, client_cert_cert_val)) {
      return false;
    }

    JS::RootedValue client_cert_key_val(cx);
    if (!JS_HasProperty(cx, client_cert_obj, "key", &found)) {
      return false;
    }
    if (!found) {
      JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr,
                                JSMSG_BACKEND_CLIENT_CERTIFICATE_KEY_INVALID);
      return false;
    }
    if (!JS_GetProperty(cx, client_cert_obj, "key", &client_cert_key_val)) {
      return false;
    }
    if (!Backend::set_client_cert_key(cx, backend, client_cert_key_val)) {
      return false;
    }
  }

  auto result = Backend::register_dynamic_backend(cx, backend);
  if (result.isErr()) {
    return false;
  }
  args.rval().setObject(*backend);
  return true;
}

bool install(api::Engine *engine) {
  JS::RootedObject backends(engine->cx(), JS_NewPlainObject(engine->cx()));
  if (!backends) {
    return false;
  }
  Backend::backends.init(engine->cx(), backends);
  if (!Backend::init_class_impl(engine->cx(), engine->global())) {
    return false;
  }

  RootedObject backend_obj(engine->cx(),
                           JS_GetConstructor(engine->cx(), BuiltinImpl<Backend>::proto_obj));
  RootedValue backend_val(engine->cx(), ObjectValue(*backend_obj));
  RootedObject backend_ns(engine->cx(), JS_NewObject(engine->cx(), nullptr));
  if (!JS_SetProperty(engine->cx(), backend_ns, "Backend", backend_val)) {
    return false;
  }
  RootedValue backend_ns_val(engine->cx(), JS::ObjectValue(*backend_ns));
  if (!engine->define_builtin_module("fastly:backend", backend_ns_val)) {
    return false;
  }
  return true;
}

} // namespace fastly::backend
