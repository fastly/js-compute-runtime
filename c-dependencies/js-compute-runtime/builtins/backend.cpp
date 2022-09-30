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
#include "js/experimental/TypedData.h"
#pragma clang diagnostic pop

#include "backend.h"
#include "host_call.h"
#include "js/Conversions.h"

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

bool isValidIP(std::string_view ip) {
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
bool isValidHost(std::string_view host) {
  if (host.length() < 1) {
    return false;
  }
  auto firstCharacter = host.front();
  // check first character is in the regex [a-zA-Z0-9]
  if (!std::isalnum(firstCharacter)) {
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

  auto lastCharacter = hostname.back();
  // check last character is in the regex [a-zA-Z0-9]
  if (!std::isalnum(lastCharacter)) {
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

namespace builtins {

JS::Result<mozilla::Ok> Backend::register_dynamic_backend(JSContext *cx, JS::HandleObject backend) {
  MOZ_ASSERT(is_instance(backend));

  xqd_world_string_t name_str;
  JS::RootedString name(cx, JS::GetReservedSlot(backend, Backend::Slots::Name).toString());
  JS::UniqueChars nameChars = encode(cx, name, &name_str.len);
  name_str.ptr = nameChars.get();

  xqd_world_string_t target_str;
  JS::RootedString target(cx, JS::GetReservedSlot(backend, Backend::Slots::Target).toString());
  JS::UniqueChars targetChars = encode(cx, target, &target_str.len);
  target_str.ptr = targetChars.get();

  fastly_dynamic_backend_config_t backend_config;
  std::memset(&backend_config, 0, sizeof(backend_config));

  std::string hostOverride;
  auto hostOverrideSlot = JS::GetReservedSlot(backend, Backend::Slots::HostOverride);
  if ((backend_config.host_override.is_some = !hostOverrideSlot.isNullOrUndefined())) {
    JS::RootedString hostOverrideString(cx, hostOverrideSlot.toString());
    size_t hostOverride_len;
    JS::UniqueChars hostOverrideChars = encode(cx, hostOverrideString, &hostOverride_len);
    hostOverride = std::string(hostOverrideChars.get(), hostOverride_len);
    backend_config.host_override.val.ptr = const_cast<char *>(hostOverride.c_str());
    backend_config.host_override.val.len = hostOverride.length();
  }

  auto connectTimeoutSlot = JS::GetReservedSlot(backend, Backend::Slots::ConnectTimeout);
  if ((backend_config.connect_timeout.is_some = !connectTimeoutSlot.isNullOrUndefined())) {
    backend_config.connect_timeout.val = connectTimeoutSlot.toInt32();
  }

  auto firstByteTimeoutSlot = JS::GetReservedSlot(backend, Backend::Slots::FirstByteTimeout);
  if ((backend_config.first_byte_timeout.is_some = !firstByteTimeoutSlot.isNullOrUndefined())) {
    backend_config.first_byte_timeout.val = firstByteTimeoutSlot.toInt32();
  }

  auto betweenBytesTimeoutSlot = JS::GetReservedSlot(backend, Backend::Slots::BetweenBytesTimeout);
  if ((backend_config.between_bytes_timeout.is_some =
           !betweenBytesTimeoutSlot.isNullOrUndefined())) {
    backend_config.between_bytes_timeout.val = betweenBytesTimeoutSlot.toInt32();
  }

  auto useSslSlot = JS::GetReservedSlot(backend, Backend::Slots::UseSsl);
  if ((backend_config.use_ssl.is_some = !useSslSlot.isNullOrUndefined())) {
    backend_config.use_ssl.val = useSslSlot.toBoolean();
  }

  auto tlsMinVersion = JS::GetReservedSlot(backend, Backend::Slots::TlsMinVersion);
  if ((backend_config.ssl_min_version.is_some = !tlsMinVersion.isNullOrUndefined())) {
    backend_config.ssl_min_version.val = (int8_t)tlsMinVersion.toInt32();
  }

  auto tlsMaxVersion = JS::GetReservedSlot(backend, Backend::Slots::TlsMaxVersion);
  if ((backend_config.ssl_max_version.is_some = !tlsMaxVersion.isNullOrUndefined())) {
    backend_config.ssl_max_version.val = (int8_t)tlsMaxVersion.toInt32();
  }

  std::string certificateHostname;
  auto certificateHostnameSlot = JS::GetReservedSlot(backend, Backend::Slots::CertificateHostname);
  if ((backend_config.cert_hostname.is_some = !certificateHostnameSlot.isNullOrUndefined())) {
    JS::RootedString certificateHostnameString(cx, certificateHostnameSlot.toString());
    size_t certificateHostname_len;
    JS::UniqueChars certificateHostnameChars =
        encode(cx, certificateHostnameString, &certificateHostname_len);
    certificateHostname = std::string(certificateHostnameChars.get(), certificateHostname_len);
    backend_config.cert_hostname.val.ptr = const_cast<char *>(certificateHostname.c_str());
    backend_config.cert_hostname.val.len = certificateHostname.length();
  }

  std::string caCertificate;
  auto caCertificateSlot = JS::GetReservedSlot(backend, Backend::Slots::CaCertificate);
  if ((backend_config.ca_cert.is_some = !caCertificateSlot.isNullOrUndefined())) {
    JS::RootedString caCertificateString(cx, caCertificateSlot.toString());
    size_t caCertificate_len;
    JS::UniqueChars caCertificateChars = encode(cx, caCertificateString, &caCertificate_len);
    caCertificate = std::string(caCertificateChars.get(), caCertificate_len);
    backend_config.ca_cert.val.ptr = const_cast<char *>(caCertificate.c_str());
    backend_config.ca_cert.val.len = caCertificate.length();
  }

  std::string ciphers;
  auto ciphersSlot = JS::GetReservedSlot(backend, Backend::Slots::Ciphers);
  if ((backend_config.ciphers.is_some = !ciphersSlot.isNullOrUndefined())) {
    JS::RootedString ciphersString(cx, ciphersSlot.toString());
    size_t ciphers_len;
    JS::UniqueChars ciphersChars = encode(cx, ciphersString, &ciphers_len);
    ciphers = std::string(ciphersChars.get(), ciphers_len);
    backend_config.ciphers.val.ptr = const_cast<char *>(ciphers.c_str());
    backend_config.ciphers.val.len = ciphers.length();
  }

  std::string sniHostname;
  auto sniHostnameSlot = JS::GetReservedSlot(backend, Backend::Slots::SniHostname);
  if ((backend_config.sni_hostname.is_some = !sniHostnameSlot.isNullOrUndefined())) {
    JS::RootedString sniHostnameString(cx, sniHostnameSlot.toString());
    size_t sniHostname_len;
    JS::UniqueChars sniHostnameChars = encode(cx, sniHostnameString, &sniHostname_len);
    sniHostname = std::string(sniHostnameChars.get(), sniHostname_len);
    backend_config.sni_hostname.val.ptr = const_cast<char *>(sniHostname.c_str());
    backend_config.sni_hostname.val.len = sniHostname.length();
  }

  fastly_error_t err;
  auto result =
      xqd_fastly_http_req_register_dynamic_backend(&name_str, &target_str, &backend_config, &err);
  if (!HANDLE_RESULT(cx, result, err)) {
    return JS::Result<mozilla::Ok>(JS::Error());
  } else {
    return mozilla::Ok();
  }
}

JSString *Backend::name(JSContext *cx, JSObject *self) {
  MOZ_ASSERT(is_instance(self));
  return JS::GetReservedSlot(self, Backend::Slots::Name).toString();
}

bool Backend::toString(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0)
  JS::RootedString name(cx, JS::GetReservedSlot(self, Backend::Slots::Name).toString());
  args.rval().setString(name);
  return true;
}

const JSFunctionSpec Backend::methods[] = {JS_FN("toString", toString, 0, JSPROP_ENUMERATE),
                                           JS_FS_END};

const JSPropertySpec Backend::properties[] = {JS_PS_END};

bool Backend::set_name(JSContext *cx, JSObject *backend, JS::HandleValue name_val) {
  if (name_val.isNullOrUndefined()) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_BACKEND_NAME_NOT_SET);
    return false;
  }
  JS::RootedString name(cx, JS::ToString(cx, name_val));
  if (!name) {
    return false;
  }
  auto length = JS::GetStringLength(name);
  if (length > 254) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_BACKEND_NAME_TOO_LONG);
    return false;
  }
  if (length == 0) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_BACKEND_NAME_EMPTY);
    return false;
  }

  JS::SetReservedSlot(backend, Backend::Slots::Name, JS::StringValue(name));
  return true;
}

bool Backend::set_host_override(JSContext *cx, JSObject *backend,
                                JS::HandleValue hostOverride_val) {
  auto hostOverride = JS::ToString(cx, hostOverride_val);
  if (!hostOverride) {
    return false;
  }

  if (JS_GetStringLength(hostOverride) == 0) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_BACKEND_HOST_OVERRIDE_EMPTY);
    return false;
  }
  JS::SetReservedSlot(backend, Backend::Slots::HostOverride, JS::StringValue(hostOverride));
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
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_BACKEND_TIMEOUT_NEGATIVE,
                              property_name.c_str());
    return false;
  }
  if (timeout >= std::pow(2, 32)) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_BACKEND_TIMEOUT_TOO_BIG,
                              property_name.c_str());
    return false;
  }
  JS::SetReservedSlot(backend, slot, JS::Int32Value(timeout));
  return true;
}

bool Backend::set_target(JSContext *cx, JSObject *backend, JS::HandleValue target_val) {
  if (target_val.isNullOrUndefined()) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_BACKEND_TARGET_NOT_SET);
    return false;
  }

  auto targetStringSlice = encode(cx, target_val);
  if (!targetStringSlice.data) {
    return false;
  }

  std::string_view targetString(reinterpret_cast<char *>(targetStringSlice.data),
                                targetStringSlice.len);
  auto length = targetString.length();
  if (length == 0) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_BACKEND_TARGET_EMPTY);
    return false;
  }

  if (targetString == "::") {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_BACKEND_TARGET_INVALID);
    return false;
  }
  if (!isValidHost(targetString) && !isValidIP(targetString)) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_BACKEND_TARGET_INVALID);
    return false;
  }

  auto targetStr = JS_NewStringCopyN(cx, targetString.data(), targetString.length());
  if (!targetStr) {
    return false;
  }
  JS::RootedValue target(cx, JS::StringValue(targetStr));
  JS::SetReservedSlot(backend, Backend::Slots::Target, target);
  return true;
}

JSObject *Backend::create(JSContext *cx, JS::HandleObject request) {
  JS::RootedValue request_url(cx, RequestOrResponse::url(request));
  auto url_string = encode(cx, request_url);
  if (!url_string.data) {
    return nullptr;
  }

  auto url = jsurl::new_jsurl(&url_string);
  if (!url) {
    JS_ReportErrorUTF8(cx, "URL constructor: %s is not a valid URL.", (char *)url_string.data);
    return nullptr;
  }
  const jsurl::SpecSlice slice = jsurl::host(url);
  auto nameStr = JS_NewStringCopyN(cx, (char *)slice.data, slice.len);
  if (!nameStr) {
    return nullptr;
  }

  // Check if we already constructed an implicit dynamic backend for this host.
  bool found;
  JS::RootedValue alreadyBuiltBackend(cx);
  if (!JS_HasProperty(cx, Backend::backends, reinterpret_cast<const char *>(slice.data), &found)) {
    return nullptr;
  }
  if (found) {
    if (!JS_GetProperty(cx, Backend::backends, reinterpret_cast<const char *>(slice.data),
                        &alreadyBuiltBackend)) {
      return nullptr;
    }
    JS::RootedObject backend(cx, &alreadyBuiltBackend.toObject());
    return backend;
  }

  auto backendInstance = JS_NewObjectWithGivenProto(cx, &Backend::class_, Backend::proto_obj);
  if (!backendInstance) {
    return nullptr;
  }
  JS::RootedValue backendVal(cx, JS::ObjectValue(*backendInstance));
  JS::RootedObject backend(cx, backendInstance);
  if (!backend) {
    return nullptr;
  }

  JS::RootedValue name(cx, JS::StringValue(nameStr));
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

  auto result = Backend::register_dynamic_backend(cx, backend);
  if (result.isErr()) {
    return nullptr;
  } else {
    if (!JS_SetProperty(cx, Backend::backends, reinterpret_cast<const char *>(slice.data),
                        backendVal)) {
      return nullptr;
    }
    return backend;
  }
}

bool Backend::constructor(JSContext *cx, unsigned argc, JS::Value *vp) {
  REQUEST_HANDLER_ONLY("The Backend builtin");
  CTOR_HEADER("Backend", 1);

  auto configurationParameter = args.get(0);

  if (!configurationParameter.isObject()) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_BACKEND_PARAMETER_NOT_OBJECT);
    return false;
  }

  JS::RootedObject backend(cx, JS_NewObjectForConstructor(cx, &class_, args));
  if (!backend) {
    return false;
  }

  JS::RootedObject configuration(cx, &configurationParameter.toObject());

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
  JS::RootedValue hostOverride_val(cx);
  if (!JS_HasProperty(cx, configuration, "hostOverride", &found)) {
    return false;
  }
  if (found) {
    if (!JS_GetProperty(cx, configuration, "hostOverride", &hostOverride_val)) {
      return false;
    }
    if (!Backend::set_host_override(cx, backend, hostOverride_val)) {
      return false;
    }
  }

  JS::RootedValue connectTimeout_val(cx);
  if (!JS_HasProperty(cx, configuration, "connectTimeout", &found)) {
    return false;
  }
  if (found) {
    if (!JS_GetProperty(cx, configuration, "connectTimeout", &connectTimeout_val)) {
      return false;
    }
    if (!Backend::set_timeout_slot(cx, backend, connectTimeout_val, Backend::Slots::ConnectTimeout,
                                   "connectTimeout")) {
      return false;
    }
  }

  /// Timeouts for backends must be less than 2^32 milliseconds, or
  /// about a month and a half.
  JS::RootedValue firstByteTimeout_val(cx);
  if (!JS_HasProperty(cx, configuration, "firstByteTimeout", &found)) {
    return false;
  }
  if (found) {
    if (!JS_GetProperty(cx, configuration, "firstByteTimeout", &firstByteTimeout_val)) {
      return false;
    }
    if (!Backend::set_timeout_slot(cx, backend, firstByteTimeout_val,
                                   Backend::Slots::FirstByteTimeout, "firstByteTimeout")) {
      return false;
    }
  }

  /// Timeouts for backends must be less than 2^32 milliseconds, or
  /// about a month and a half.
  JS::RootedValue betweenBytesTimeout_val(cx);
  if (!JS_HasProperty(cx, configuration, "betweenBytesTimeout", &found)) {
    return false;
  }
  if (found) {
    if (!JS_GetProperty(cx, configuration, "betweenBytesTimeout", &betweenBytesTimeout_val)) {
      return false;
    }
    if (!Backend::set_timeout_slot(cx, backend, betweenBytesTimeout_val,
                                   Backend::Slots::BetweenBytesTimeout, "betweenBytesTimeout")) {
      return false;
    }
  }

  /// Has to be either: 1; 1.1; 1.2; 1.3;
  JS::RootedValue tlsMinVersion_val(cx);
  std::optional<int> tlsMinVersion;
  if (!JS_HasProperty(cx, configuration, "tlsMinVersion", &found)) {
    return false;
  }
  if (found) {
    if (!JS_GetProperty(cx, configuration, "tlsMinVersion", &tlsMinVersion_val)) {
      return false;
    }
    double version;
    if (!JS::ToNumber(cx, tlsMinVersion_val, &version)) {
      return false;
    }

    if (isnan(version)) {
      JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_BACKEND_TLS_MIN_INVALID);
      return false;
    }

    if (version == 1.3) {
      tlsMinVersion = TLS::VERSION_1_3;
    } else if (version == 1.2) {
      tlsMinVersion = TLS::VERSION_1_2;
    } else if (version == 1.1) {
      tlsMinVersion = TLS::VERSION_1_1;
    } else if (version == 1) {
      tlsMinVersion = TLS::VERSION_1;
    } else {
      JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_BACKEND_TLS_MIN_INVALID);
      return false;
    }
    JS::SetReservedSlot(backend, Backend::Slots::TlsMinVersion,
                        JS::Int32Value(tlsMinVersion.value()));
  }

  /// Has to be either: 1; 1.1; 1.2; 1.3;
  JS::RootedValue tlsMaxVersion_val(cx);
  std::optional<int> tlsMaxVersion;
  if (!JS_HasProperty(cx, configuration, "tlsMaxVersion", &found)) {
    return false;
  }
  if (found) {
    if (!JS_GetProperty(cx, configuration, "tlsMaxVersion", &tlsMaxVersion_val)) {
      return false;
    }
    double version;
    if (!JS::ToNumber(cx, tlsMaxVersion_val, &version)) {
      return false;
    }

    if (isnan(version)) {
      JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_BACKEND_TLS_MAX_INVALID);
      return false;
    }

    if (version == 1.3) {
      tlsMaxVersion = TLS::VERSION_1_3;
    } else if (version == 1.2) {
      tlsMaxVersion = TLS::VERSION_1_2;
    } else if (version == 1.1) {
      tlsMaxVersion = TLS::VERSION_1_1;
    } else if (version == 1) {
      tlsMaxVersion = TLS::VERSION_1;
    } else {
      JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_BACKEND_TLS_MAX_INVALID);
      return false;
    }
    JS::SetReservedSlot(backend, Backend::Slots::TlsMaxVersion,
                        JS::Int32Value(tlsMaxVersion.value()));
  }

  if (tlsMinVersion.has_value() && tlsMaxVersion.has_value()) {
    if (tlsMinVersion.value() > tlsMaxVersion.value()) {
      JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr,
                                JSMSG_BACKEND_TLS_MIN_GREATER_THAN_TLS_MAX);
      return false;
    }
  }

  JS::RootedValue certificateHostname_val(cx);
  if (!JS_HasProperty(cx, configuration, "certificateHostname", &found)) {
    return false;
  }
  if (found) {
    if (!JS_GetProperty(cx, configuration, "certificateHostname", &certificateHostname_val)) {
      return false;
    }
    auto certificateHostname = JS::ToString(cx, certificateHostname_val);
    if (!certificateHostname) {
      return false;
    }

    if (JS_GetStringLength(certificateHostname) == 0) {
      JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr,
                                JSMSG_BACKEND_CERTIFICATE_HOSTNAME_EMPTY);
      return false;
    }
    JS::SetReservedSlot(backend, Backend::Slots::CertificateHostname,
                        JS::StringValue(certificateHostname));
  }

  JS::RootedValue useSsl_val(cx);
  if (!JS_HasProperty(cx, configuration, "useSSL", &found)) {
    return false;
  }
  if (found) {
    if (!JS_GetProperty(cx, configuration, "useSSL", &useSsl_val)) {
      return false;
    }
    auto value = JS::ToBoolean(useSsl_val);
    JS::SetReservedSlot(backend, Backend::Slots::UseSsl, JS::BooleanValue(value));
  }

  JS::RootedValue caCertificate_val(cx);
  if (!JS_HasProperty(cx, configuration, "caCertificate", &found)) {
    return false;
  }
  if (found) {
    if (!JS_GetProperty(cx, configuration, "caCertificate", &caCertificate_val)) {
      return false;
    }
    auto caCertificate = JS::ToString(cx, caCertificate_val);
    if (!caCertificate) {
      return false;
    }
    if (JS_GetStringLength(caCertificate) == 0) {
      JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_BACKEND_CA_CERTIFICATE_EMPTY);
      return false;
    }
    JS::SetReservedSlot(backend, Backend::Slots::CaCertificate, JS::StringValue(caCertificate));
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
    size_t length;
    auto ciphers_chars = encode(cx, ciphers_val, &length);
    if (!ciphers_chars) {
      return false;
    }
    if (length == 0) {
      JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_BACKEND_CIPHERS_EMPTY);
      return false;
    }
    std::string cipherSpec(ciphers_chars.get(), length);
    if (!isCipherSuiteSupportedByFastly(cipherSpec)) {
      JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_BACKEND_CIPHERS_NOT_AVALIABLE);
      return false;
    }
    JS::SetReservedSlot(backend, Backend::Slots::Ciphers,
                        JS::StringValue(JS_NewStringCopyN(cx, ciphers_chars.get(), length)));
    auto ciphersSlot = JS::GetReservedSlot(backend, Backend::Slots::Ciphers);
    if (!ciphersSlot.isNullOrUndefined()) {
      JS::RootedString ciphers(cx, ciphersSlot.toString());
      size_t ciphers_len;
      JS::UniqueChars ciphersChars = encode(cx, ciphers, &ciphers_len);
    }
  }

  JS::RootedValue sniHostname_val(cx);
  if (!JS_HasProperty(cx, configuration, "sniHostname", &found)) {
    return false;
  }
  if (found) {
    if (!JS_GetProperty(cx, configuration, "sniHostname", &sniHostname_val)) {
      return false;
    }
    auto sniHostname = JS::ToString(cx, sniHostname_val);
    if (!sniHostname) {
      return false;
    }
    if (JS_GetStringLength(sniHostname) == 0) {
      JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_BACKEND_SNI_HOSTNAME_EMPTY);
      return false;
    }
    JS::SetReservedSlot(backend, Backend::Slots::SniHostname, JS::StringValue(sniHostname));
  }

  auto result = Backend::register_dynamic_backend(cx, backend);
  if (result.isErr()) {
    return false;
  }
  args.rval().setObject(*backend);
  return true;
}

bool Backend::init_class(JSContext *cx, JS::HandleObject global) {
  JS::RootedObject backends(cx, JS_NewPlainObject(cx));
  if (!backends) {
    return false;
  }
  Backend::backends.init(cx, backends);
  return BuiltinImpl<Backend>::init_class_impl(cx, global);
}

} // namespace builtins
