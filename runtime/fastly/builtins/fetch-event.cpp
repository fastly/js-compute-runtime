#include "fetch-event.h"
#include "../../StarlingMonkey/builtins/web/performance.h"
#include "../../StarlingMonkey/builtins/web/url.h"
#include "../../StarlingMonkey/builtins/web/worker-location.h"
#include "../common/ip_octets_to_js_string.h"
#include "../common/normalize_http_method.h"
#include "../host-api/fastly.h"
#include "../host-api/host_api_fastly.h"
#include "./fetch/request-response.h"
#include "encode.h"
#include "fastly.h"
#include "host_api.h"
#include "js/JSON.h"
#include "openssl/evp.h"

#include <iostream>
#include <memory>

using std::chrono::microseconds;
using std::chrono::system_clock;
using namespace std::literals::string_view_literals;
using builtins::web::fetch::Headers;
using builtins::web::url::URL;
using builtins::web::worker_location::WorkerLocation;
using fastly::fastly::Fastly;
using fastly::fetch::RequestOrResponse;
using fastly::fetch::Response;

namespace fastly::fetch_event {

namespace {

JSString *client_request_id(JSObject *obj) {
  JS::Value val =
      JS::GetReservedSlot(obj, static_cast<uint32_t>(ClientInfo::Slots::ClientRequestId));
  return val.isString() ? val.toString() : nullptr;
}

JSString *client_address(JSObject *obj) {
  JS::Value val = JS::GetReservedSlot(obj, static_cast<uint32_t>(ClientInfo::Slots::Address));
  return val.isString() ? val.toString() : nullptr;
}

JSString *geo_info(JSObject *obj) {
  JS::Value val = JS::GetReservedSlot(obj, static_cast<uint32_t>(ClientInfo::Slots::GeoInfo));
  return val.isString() ? val.toString() : nullptr;
}

JSString *cipher(JSObject *obj) {
  JS::Value val = JS::GetReservedSlot(obj, static_cast<uint32_t>(ClientInfo::Slots::Cipher));
  return val.isString() ? val.toString() : nullptr;
}
JSString *ja3(JSObject *obj) {
  JS::Value val = JS::GetReservedSlot(obj, static_cast<uint32_t>(ClientInfo::Slots::JA3));
  return val.isString() ? val.toString() : nullptr;
}
JSString *ja4(JSObject *obj) {
  JS::Value val = JS::GetReservedSlot(obj, static_cast<uint32_t>(ClientInfo::Slots::JA4));
  return val.isString() ? val.toString() : nullptr;
}
JSString *h2Fingerprint(JSObject *obj) {
  JS::Value val = JS::GetReservedSlot(obj, static_cast<uint32_t>(ClientInfo::Slots::H2Fingerprint));
  return val.isString() ? val.toString() : nullptr;
}
JSString *ohFingerprint(JSObject *obj) {
  JS::Value val = JS::GetReservedSlot(obj, static_cast<uint32_t>(ClientInfo::Slots::OHFingerprint));
  return val.isString() ? val.toString() : nullptr;
}
JSObject *clientHello(JSObject *obj) {
  JS::Value val = JS::GetReservedSlot(obj, static_cast<uint32_t>(ClientInfo::Slots::ClientHello));
  return val.isObject() ? val.toObjectOrNull() : nullptr;
}
JSObject *clientCert(JSObject *obj) {
  JS::Value val = JS::GetReservedSlot(obj, static_cast<uint32_t>(ClientInfo::Slots::ClientCert));
  return val.isObject() ? val.toObjectOrNull() : nullptr;
}
JSString *protocol(JSObject *obj) {
  JS::Value val = JS::GetReservedSlot(obj, static_cast<uint32_t>(ClientInfo::Slots::Protocol));
  return val.isString() ? val.toString() : nullptr;
}

} // namespace

JSString *ClientInfo::retrieve_client_address(JSContext *cx, JS::HandleObject self) {
  auto res = request_handle(cx, self).downstream_client_ip_addr();
  if (res.is_err()) {
    return nullptr;
  }

  JS::RootedString address(cx, common::ip_octets_to_js_string(cx, std::move(res.unwrap())));
  if (!address) {
    return nullptr;
  }

  JS::SetReservedSlot(self, static_cast<uint32_t>(ClientInfo::Slots::Address),
                      JS::StringValue(address));
  return address;
}

host_api::HttpReq ClientInfo::request_handle(JSContext *cx, JS::HandleObject self) {
  JS::RootedValue req(cx, JS::GetReservedSlot(self, static_cast<uint32_t>(Slots::Request)));
  return Request::request_handle(&req.toObject());
}

bool ClientInfo::request_id_get(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0);

  JS::RootedString result(cx, client_request_id(self));
  if (!result) {
    auto res = request_handle(cx, self).http_req_downstream_client_request_id();
    if (res.to_err() || !res.unwrap().has_value()) {
      args.rval().setNull();
      return true;
    }

    auto h2fp_str = std::move(res.unwrap().value());
    result.set(JS_NewStringCopyN(cx, h2fp_str.ptr.get(), h2fp_str.len));
    JS::SetReservedSlot(self, static_cast<uint32_t>(ClientInfo::Slots::ClientRequestId),
                        JS::StringValue(result));
  }
  args.rval().setString(result);
  return true;
}

bool ClientInfo::address_get(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0);

  JS::RootedString address_str(cx, client_address(self));
  if (!address_str) {
    address_str = retrieve_client_address(cx, self);
    if (!address_str) {
      args.rval().setNull();
      return true;
    }
  }

  args.rval().setString(address_str);
  return true;
}

bool ClientInfo::geo_get(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0)

  JS::RootedString geo_info_str(cx, geo_info(self));
  if (!geo_info_str) {
    JS::RootedString address_str(cx, client_address(self));
    if (!address_str) {
      address_str = retrieve_client_address(cx, self);
      if (!address_str) {
        args.rval().setNull();
        return true;
      }
    }

    // TODO: skip intermediate encoding, and rely on the fact that we already had the bytes before
    auto address = core::encode(cx, address_str);
    if (!address) {
      return false;
    }

    // TODO: Remove all of this and rely on the host for validation as the hostcall only takes one
    // user-supplied parameter
    int format = AF_INET;
    size_t octets_len = 4;
    if (std::find(address.begin(), address.end(), ':') != address.end()) {
      format = AF_INET6;
      octets_len = 16;
    }

    uint8_t octets[sizeof(struct in6_addr)];
    if (inet_pton(format, address.begin(), octets) != 1) {
      // While get_geo_info can be invoked through FetchEvent#client.geo, too,
      // that path can't result in an invalid address here, so we can be more
      // specific in the error message.
      // TODO: Make a TypeError
      JS_ReportErrorLatin1(cx, "Invalid address passed to fastly.getGeolocationForIpAddress");
      return false;
    }

    auto res = host_api::GeoIp::lookup(std::span<uint8_t>{octets, octets_len});
    if (res.is_err() || !res.unwrap().has_value()) {
      args.rval().setNull();
      return true;
    }

    auto ret = std::move(res.unwrap().value());
    geo_info_str =
        JS::RootedString(cx, JS_NewStringCopyUTF8N(cx, JS::UTF8Chars(ret.ptr.release(), ret.len)));
    if (!geo_info_str)
      return false;
    JS::SetReservedSlot(self, static_cast<uint32_t>(ClientInfo::Slots::GeoInfo),
                        JS::StringValue(geo_info_str));
  }

  return JS_ParseJSON(cx, geo_info_str, args.rval());
}

bool ClientInfo::tls_cipher_openssl_name_get(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0);

  JS::RootedString result(cx, cipher(self));
  if (!result) {
    auto res = request_handle(cx, self).http_req_downstream_tls_cipher_openssl_name();
    if (auto *err = res.to_err()) {
      HANDLE_ERROR(cx, *err);
      return false;
    }

    if (!res.unwrap().has_value()) {
      args.rval().setNull();
      return true;
    }

    auto cipher = std::move(res.unwrap().value());
    result.set(JS_NewStringCopyN(cx, cipher.ptr.get(), cipher.len));
    JS::SetReservedSlot(self, static_cast<uint32_t>(ClientInfo::Slots::Cipher),
                        JS::StringValue(result));
  }

  args.rval().setString(result);
  return true;
}

bool ClientInfo::tls_ja3_md5_get(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0);

  JS::RootedString result(cx, ja3(self));
  if (!result) {
    auto res = request_handle(cx, self).http_req_downstream_tls_ja3_md5();
    if (auto *err = res.to_err()) {
      HANDLE_ERROR(cx, *err);
      return false;
    }

    if (!res.unwrap().has_value()) {
      args.rval().setNull();
      return true;
    }

    auto ja3 = std::move(res.unwrap().value());
    JS::UniqueChars hex{OPENSSL_buf2hexstr(ja3.ptr.get(), ja3.len)};
    std::string ja3hex{hex.get(), std::remove(hex.get(), hex.get() + strlen(hex.get()), ':')};

    result.set(JS_NewStringCopyN(cx, ja3hex.c_str(), ja3hex.length()));
    JS::SetReservedSlot(self, static_cast<uint32_t>(ClientInfo::Slots::JA3),
                        JS::StringValue(result));
  }
  args.rval().setString(result);
  return true;
}

bool ClientInfo::tls_ja4_get(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0);

  JS::RootedString result(cx, ja4(self));
  if (!result) {
    auto res = request_handle(cx, self).http_req_downstream_tls_ja4();
    if (auto *err = res.to_err()) {
      HANDLE_ERROR(cx, *err);
      return false;
    }

    if (!res.unwrap().has_value()) {
      args.rval().setNull();
      return true;
    }

    auto ja4_str = std::move(res.unwrap().value());
    result.set(JS_NewStringCopyN(cx, ja4_str.ptr.get(), ja4_str.len));
    JS::SetReservedSlot(self, static_cast<uint32_t>(ClientInfo::Slots::JA4),
                        JS::StringValue(result));
  }
  args.rval().setString(result);
  return true;
}

bool ClientInfo::h2_fingerprint_get(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0);

  JS::RootedString result(cx, h2Fingerprint(self));
  if (!result) {
    auto res = request_handle(cx, self).http_req_downstream_client_h2_fingerprint();
    if (auto *err = res.to_err()) {
      HANDLE_ERROR(cx, *err);
      return false;
    }

    if (!res.unwrap().has_value()) {
      args.rval().setNull();
      return true;
    }

    auto h2fp_str = std::move(res.unwrap().value());
    result.set(JS_NewStringCopyN(cx, h2fp_str.ptr.get(), h2fp_str.len));
    JS::SetReservedSlot(self, static_cast<uint32_t>(ClientInfo::Slots::H2Fingerprint),
                        JS::StringValue(result));
  }
  args.rval().setString(result);
  return true;
}

bool ClientInfo::oh_fingerprint_get(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0);

  JS::RootedString result(cx, ohFingerprint(self));
  if (!result) {
    auto res = request_handle(cx, self).http_req_downstream_client_oh_fingerprint();
    if (auto *err = res.to_err()) {
      HANDLE_ERROR(cx, *err);
      return false;
    }

    if (!res.unwrap().has_value()) {
      args.rval().setNull();
      return true;
    }

    auto ohfp_str = std::move(res.unwrap().value());
    result.set(JS_NewStringCopyN(cx, ohfp_str.ptr.get(), ohfp_str.len));
    JS::SetReservedSlot(self, static_cast<uint32_t>(ClientInfo::Slots::OHFingerprint),
                        JS::StringValue(result));
  }
  args.rval().setString(result);
  return true;
}

bool ClientInfo::tls_client_hello_get(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0);

  JS::RootedObject buffer(cx, clientHello(self));
  if (!buffer) {
    auto res = request_handle(cx, self).http_req_downstream_tls_client_hello();
    if (auto *err = res.to_err()) {
      HANDLE_ERROR(cx, *err);
      return false;
    }

    if (!res.unwrap().has_value()) {
      args.rval().setNull();
      return true;
    }

    auto hello = std::move(res.unwrap().value());
    buffer.set(JS::NewArrayBufferWithContents(cx, hello.len, hello.ptr.get(),
                                              JS::NewArrayBufferOutOfMemory::CallerMustFreeMemory));
    if (!buffer) {
      // We can be here if the array buffer was too large -- if that was the case then a
      // JSMSG_BAD_ARRAY_LENGTH will have been created.
      return false;
    }

    // `hello` is now owned by `buffer`
    static_cast<void>(hello.ptr.release());
    JS::SetReservedSlot(self, static_cast<uint32_t>(ClientInfo::Slots::ClientHello),
                        JS::ObjectValue(*buffer));
  }

  args.rval().setObject(*buffer);
  return true;
}

bool ClientInfo::tls_client_certificate_get(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0);

  JS::RootedObject buffer(cx, clientCert(self));
  if (!buffer) {
    auto res = request_handle(cx, self).http_req_downstream_tls_raw_client_certificate();
    if (auto *err = res.to_err()) {
      HANDLE_ERROR(cx, *err);
      return false;
    }

    if (!res.unwrap().has_value()) {
      args.rval().setNull();
      return true;
    }

    auto cert = std::move(res.unwrap().value());

    buffer.set(JS::NewArrayBufferWithContents(cx, cert.len, cert.ptr.get(),
                                              JS::NewArrayBufferOutOfMemory::CallerMustFreeMemory));
    if (!buffer) {
      // We can be here if the array buffer was too large -- if that was the case then a
      // JSMSG_BAD_ARRAY_LENGTH will have been created.
      return false;
    }

    // `cert` is now owned by `buffer`
    static_cast<void>(cert.ptr.release());
    JS::SetReservedSlot(self, static_cast<uint32_t>(ClientInfo::Slots::ClientCert),
                        JS::ObjectValue(*buffer));
  }

  args.rval().setObject(*buffer);
  return true;
}

bool ClientInfo::tls_protocol_get(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0);

  JS::RootedString result(cx, protocol(self));
  if (!result) {
    auto res = request_handle(cx, self).http_req_downstream_tls_protocol();
    if (auto *err = res.to_err()) {
      HANDLE_ERROR(cx, *err);
      return false;
    }

    if (!res.unwrap().has_value()) {
      args.rval().setNull();
      return true;
    }

    auto protocol = std::move(res.unwrap().value());
    result.set(JS_NewStringCopyN(cx, protocol.ptr.get(), protocol.len));
    JS::SetReservedSlot(self, static_cast<uint32_t>(ClientInfo::Slots::Protocol),
                        JS::StringValue(result));
  }

  args.rval().setString(result);
  return true;
}

const JSFunctionSpec ClientInfo::static_methods[] = {
    JS_FS_END,
};

const JSPropertySpec ClientInfo::static_properties[] = {
    JS_PS_END,
};

const JSFunctionSpec ClientInfo::methods[] = {
    JS_FS_END,
};

const JSPropertySpec ClientInfo::properties[] = {
    JS_PSG("requestId", request_id_get, JSPROP_ENUMERATE),
    JS_PSG("address", address_get, JSPROP_ENUMERATE),
    JS_PSG("geo", geo_get, JSPROP_ENUMERATE),
    JS_PSG("tlsCipherOpensslName", tls_cipher_openssl_name_get, JSPROP_ENUMERATE),
    JS_PSG("tlsProtocol", tls_protocol_get, JSPROP_ENUMERATE),
    JS_PSG("tlsJA3MD5", tls_ja3_md5_get, JSPROP_ENUMERATE),
    JS_PSG("tlsJA4", tls_ja4_get, JSPROP_ENUMERATE),
    JS_PSG("h2Fingerprint", h2_fingerprint_get, JSPROP_ENUMERATE),
    JS_PSG("ohFingerprint", oh_fingerprint_get, JSPROP_ENUMERATE),
    JS_PSG("tlsClientCertificate", tls_client_certificate_get, JSPROP_ENUMERATE),
    JS_PSG("tlsClientHello", tls_client_hello_get, JSPROP_ENUMERATE),
    JS_PS_END,
};

JSObject *ClientInfo::create(JSContext *cx, JS::HandleValue req) {
  JS::RootedObject obj(cx, JS_NewObjectWithGivenProto(cx, &class_, proto_obj));
  if (!obj) {
    return nullptr;
  }
  JS::SetReservedSlot(obj, static_cast<uint32_t>(Slots::Request), req);
  return obj;
}

namespace {

JSString *server_address(JSObject *obj) {
  JS::Value val = JS::GetReservedSlot(obj, static_cast<uint32_t>(ServerInfo::Slots::Address));
  return val.isString() ? val.toString() : nullptr;
}

} // namespace

JSString *ServerInfo::retrieve_server_address(JSContext *cx, JS::HandleObject self) {
  auto res = request_handle(cx, self).downstream_server_ip_addr();
  if (auto *err = res.to_err()) {
    HANDLE_ERROR(cx, *err);
    return nullptr;
  }

  JS::RootedString address(cx, common::ip_octets_to_js_string(cx, std::move(res.unwrap())));
  if (!address) {
    return nullptr;
  }

  JS::SetReservedSlot(self, static_cast<uint32_t>(ServerInfo::Slots::Address),
                      JS::StringValue(address));
  return address;
}

host_api::HttpReq ServerInfo::request_handle(JSContext *cx, JS::HandleObject self) {
  JS::RootedValue req(cx, JS::GetReservedSlot(self, static_cast<uint32_t>(Slots::Request)));
  return Request::request_handle(&req.toObject());
}

bool ServerInfo::address_get(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0);

  JS::RootedString address_str(cx, server_address(self));
  if (!address_str) {
    address_str = retrieve_server_address(cx, self);
    if (!address_str)
      return false;
  }

  args.rval().setString(address_str);
  return true;
}

const JSFunctionSpec ServerInfo::static_methods[] = {
    JS_FS_END,
};

const JSPropertySpec ServerInfo::static_properties[] = {
    JS_PS_END,
};

const JSFunctionSpec ServerInfo::methods[] = {
    JS_FS_END,
};

const JSPropertySpec ServerInfo::properties[] = {
    JS_PSG("address", address_get, JSPROP_ENUMERATE),
    JS_PS_END,
};

JSObject *ServerInfo::create(JSContext *cx, JS::HandleValue req) {
  JS::RootedObject obj(cx, JS_NewObjectWithGivenProto(cx, &class_, proto_obj));
  if (!obj) {
    return nullptr;
  }
  JS::SetReservedSlot(obj, static_cast<uint32_t>(Slots::Request), req);
  return obj;
}

namespace {

api::Engine *ENGINE;

PersistentRooted<JSObject *> INSTANCE;
JS::PersistentRootedObjectVector *FETCH_HANDLERS;

void inc_pending_promise_count(JSObject *self) {
  MOZ_ASSERT(FetchEvent::is_instance(self));
  auto count =
      JS::GetReservedSlot(self, static_cast<uint32_t>(FetchEvent::Slots::PendingPromiseCount))
          .toInt32();
  if (count == 0) {
    ENGINE->incr_event_loop_interest();
  }
  count++;
  MOZ_ASSERT(count > 0);
  JS::SetReservedSlot(self, static_cast<uint32_t>(FetchEvent::Slots::PendingPromiseCount),
                      JS::Int32Value(count));
}

void dec_pending_promise_count(JSObject *self) {
  MOZ_ASSERT(FetchEvent::is_instance(self));
  auto count =
      JS::GetReservedSlot(self, static_cast<uint32_t>(FetchEvent::Slots::PendingPromiseCount))
          .toInt32();
  MOZ_ASSERT(count > 0);
  count--;
  if (count == 0) {
    ENGINE->decr_event_loop_interest();
  }
  JS::SetReservedSlot(self, static_cast<uint32_t>(FetchEvent::Slots::PendingPromiseCount),
                      JS::Int32Value(count));
}

bool add_pending_promise(JSContext *cx, JS::HandleObject self, JS::HandleObject promise) {
  MOZ_ASSERT(FetchEvent::is_instance(self));
  MOZ_ASSERT(JS::IsPromiseObject(promise));

  JS::RootedObject handler(cx);
  handler = &JS::GetReservedSlot(
                 self, static_cast<uint32_t>(FetchEvent::Slots::DecPendingPromiseCountFunc))
                 .toObject();
  if (!JS::AddPromiseReactions(cx, promise, handler, handler))
    return false;

  inc_pending_promise_count(self);
  return true;
}

} // namespace

bool FetchEvent::client_get(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0)

  JS::RootedValue clientInfo(cx,
                             JS::GetReservedSlot(self, static_cast<uint32_t>(Slots::ClientInfo)));

  if (clientInfo.isUndefined()) {
    JS::RootedValue req(cx, JS::GetReservedSlot(self, static_cast<uint32_t>(Slots::Request)));
    JS::RootedObject obj(cx, ClientInfo::create(cx, req));
    if (!obj)
      return false;
    clientInfo.setObject(*obj);
    JS::SetReservedSlot(self, static_cast<uint32_t>(Slots::ClientInfo), clientInfo);
  }

  args.rval().set(clientInfo);
  return true;
}

bool FetchEvent::server_get(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0)

  JS::RootedValue serverInfo(cx,
                             JS::GetReservedSlot(self, static_cast<uint32_t>(Slots::ServerInfo)));

  if (serverInfo.isUndefined()) {
    JS::RootedValue req(cx, JS::GetReservedSlot(self, static_cast<uint32_t>(Slots::Request)));
    JS::RootedObject obj(cx, ServerInfo::create(cx, req));
    if (!obj) {
      return false;
    }
    serverInfo.setObject(*obj);
    JS::SetReservedSlot(self, static_cast<uint32_t>(Slots::ServerInfo), serverInfo);
  }

  args.rval().set(serverInfo);
  return true;
}

void dispatch_fetch_event(HandleObject event) {
  MOZ_ASSERT(FetchEvent::is_instance(event));
  RootedValue result(ENGINE->cx());
  RootedValue event_val(ENGINE->cx(), JS::ObjectValue(*event));
  HandleValueArray argsv = HandleValueArray(event_val);
  RootedValue handler(ENGINE->cx());
  RootedValue rval(ENGINE->cx());

  FetchEvent::start_dispatching(event);

  for (size_t i = 0; i < FETCH_HANDLERS->length(); i++) {
    handler.setObject(*(*FETCH_HANDLERS)[i]);
    if (!JS_CallFunctionValue(ENGINE->cx(), ENGINE->global(), handler, argsv, &rval)) {
      ENGINE->dump_pending_exception("dispatching FetchEvent\n");
      break;
    }
    if (FetchEvent::state(event) != FetchEvent::State::unhandled) {
      break;
    }
  }

  FetchEvent::stop_dispatching(event);
}

void dispatch_fetch_event(HandleObject event, double *total_compute) {
  auto pre_handler = system_clock::now();
  dispatch_fetch_event(event);
  double diff = duration_cast<microseconds>(system_clock::now() - pre_handler).count();
  *total_compute += diff;
  if (ENGINE->debug_logging_enabled()) {
    printf("Request handler took %fms\n", diff / 1000);
  }
}

JSObject *FetchEvent::prepare_downstream_request(JSContext *cx) {
  JS::RootedObject requestInstance(
      cx, JS_NewObjectWithGivenProto(cx, &Request::class_, Request::proto_obj));
  if (!requestInstance)
    return nullptr;
  return Request::create(cx, requestInstance, host_api::HttpReq{}, host_api::HttpBody{}, true);
}

bool FetchEvent::init_request(JSContext *cx, JS::HandleObject self, host_api::HttpReq req,
                              host_api::HttpBody body) {

  builtins::web::performance::Performance::timeOrigin.emplace(
      std::chrono::high_resolution_clock::now());

  JS::RootedObject request(
      cx, &JS::GetReservedSlot(self, static_cast<uint32_t>(Slots::Request)).toObject());

  MOZ_ASSERT(!Request::request_handle(request).is_valid());

  JS::SetReservedSlot(request, static_cast<uint32_t>(Request::Slots::Request),
                      JS::Int32Value(req.handle));
  JS::SetReservedSlot(request, static_cast<uint32_t>(Request::Slots::Body),
                      JS::Int32Value(body.handle));

  // Set the method.
  auto res = req.get_method();
  if (auto *err = res.to_err()) {
    HANDLE_ERROR(cx, *err);
    return false;
  }

  auto method_str = std::move(res.unwrap());
  bool is_get = method_str == "GET"sv;
  bool is_head = method_str == "HEAD"sv;

  if (!is_get) {
    std::ignore = common::normalize_http_method(method_str.begin(), method_str.size());
    JS::RootedString method(cx, JS_NewStringCopyN(cx, method_str.begin(), method_str.len));
    if (!method) {
      return false;
    }

    JS::SetReservedSlot(request, static_cast<uint32_t>(Request::Slots::Method),
                        JS::StringValue(method));
  }

  // Set whether we have a body depending on the method.
  // TODO: verify if that's right. I.e. whether we should treat all requests
  // that are not GET or HEAD as having a body, which might just be 0-length.
  // It's not entirely clear what else we even could do here though.
  if (!is_get && !is_head) {
    JS::SetReservedSlot(request, static_cast<uint32_t>(Request::Slots::HasBody), JS::TrueValue());
  }

  auto uri_res = req.get_uri();
  if (auto *err = uri_res.to_err()) {
    HANDLE_ERROR(cx, *err);
    return false;
  }

  auto uri_str = std::move(uri_res.unwrap());
  JS::RootedString url(cx, JS_NewStringCopyN(cx, uri_str.ptr.get(), uri_str.len));
  if (!url) {
    return false;
  }
  JS::SetReservedSlot(request, static_cast<uint32_t>(Request::Slots::URL), JS::StringValue(url));

  // Set the URL for `globalThis.location` to the client request's URL.
  JS::RootedObject url_instance(cx, JS_NewObjectWithGivenProto(cx, &URL::class_, URL::proto_obj));
  if (!url_instance) {
    return false;
  }

  jsurl::SpecString spec(reinterpret_cast<uint8_t *>(uri_str.ptr.get()), uri_str.len, uri_str.len);
  WorkerLocation::url = URL::create(cx, url_instance, spec);
  if (!WorkerLocation::url) {
    return false;
  }

  // Set `fastly.baseURL` to the origin of the client request's URL.
  // Note that this only happens if baseURL hasn't already been set to another
  // value explicitly.
  if (!Fastly::baseURL.get()) {
    JS::RootedObject url_instance(cx, JS_NewObjectWithGivenProto(cx, &URL::class_, URL::proto_obj));
    if (!url_instance)
      return false;

    Fastly::baseURL = URL::create(cx, url_instance, URL::origin(cx, WorkerLocation::url));
    if (!Fastly::baseURL)
      return false;
  }

  return true;
}

bool FetchEvent::request_get(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0)

  args.rval().set(JS::GetReservedSlot(self, static_cast<uint32_t>(Slots::Request)));
  return true;
}

namespace {

bool start_response(JSContext *cx, JS::HandleObject response_obj, bool streaming) {
  auto response = Response::response_handle(response_obj);
  auto body = RequestOrResponse::body_handle(response_obj);

  // write all the headers
  if (!RequestOrResponse::commit_headers(cx, response_obj))
    return false;

  auto res = response.send_downstream(body, streaming);
  if (auto *err = res.to_err()) {
    HANDLE_ERROR(cx, *err);
    return false;
  }
  return true;
}

// Steps in this function refer to the spec at
// https://w3c.github.io/ServiceWorker/#fetch-event-respondwith
bool response_promise_then_handler(JSContext *cx, JS::HandleObject event, JS::HandleValue extra,
                                   JS::CallArgs args) {
  // Step 10.1
  // Note: the `then` handler is only invoked after all Promise resolution has
  // happened. (Even if there were multiple Promises to unwrap first.) That
  // means that at this point we're guaranteed to have the final value instead
  // of a Promise wrapping it, so either the value is a Response, or we have to
  // bail.
  if (!Response::is_instance(args.get(0))) {
    JS_ReportErrorUTF8(cx, "FetchEvent#respondWith must be called with a Response "
                           "object or a Promise resolving to a Response object as "
                           "the first argument");
    JS::RootedObject rejection(cx, PromiseRejectedWithPendingError(cx));
    if (!rejection)
      return false;
    args.rval().setObject(*rejection);
    return FetchEvent::respondWithError(cx, event);
  }

  // Step 10.2 (very roughly: the way we handle responses and their bodies is
  // very different.)
  JS::RootedObject response_obj(cx, &args[0].toObject());

  if (Response::is_upstream(response_obj)) {
    JS::RootedObject headers(cx, Response::headers(cx, response_obj));
    // Calling get_list() transitions to Mode::ContentOnly or Mode::CachedInContent.
    if (!Headers::get_list(cx, headers))
      return false;
  }

  if (auto grip_upgrade_request = Response::grip_upgrade_request(response_obj)) {
    auto backend = Response::backend_str(cx, response_obj);

    auto res = grip_upgrade_request->redirect_to_grip_proxy(backend);
    if (auto *err = res.to_err()) {
      HANDLE_ERROR(cx, *err);
      return false;
    }
    return true;
  }

  if (auto websocket_upgrade_request = Response::websocket_upgrade_request(response_obj)) {
    auto backend = Response::backend_str(cx, response_obj);

    auto res = websocket_upgrade_request->redirect_to_websocket_proxy(backend);
    if (auto *err = res.to_err()) {
      HANDLE_ERROR(cx, *err);
      return false;
    }
    return true;
  }

  bool streaming = false;
  if (!RequestOrResponse::maybe_stream_body(cx, response_obj, &streaming)) {
    return false;
  }

  if (streaming) {
    ENGINE->incr_event_loop_interest();
  }
  FetchEvent::mark_done(event, streaming, Response::status(response_obj));
  return start_response(cx, response_obj, streaming);
}

// Steps in this function refer to the spec at
// https://w3c.github.io/ServiceWorker/#fetch-event-respondwith
bool response_promise_catch_handler(JSContext *cx, JS::HandleObject event,
                                    JS::HandleValue promise_val, JS::CallArgs args) {
  JS::RootedObject promise(cx, &promise_val.toObject());

  fprintf(stderr, "Error while running request handler: ");
  ENGINE->dump_promise_rejection(args.get(0), promise, stderr);

  // TODO: verify that this is the right behavior.
  // Steps 9.1-2
  return FetchEvent::respondWithError(cx, event);
}

} // namespace

// Steps in this function refer to the spec at
// https://w3c.github.io/ServiceWorker/#fetch-event-respondwith
bool FetchEvent::respondWith(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(1)

  // Coercion of argument `r` to a Promise<Response>
  JS::RootedObject response_promise(cx, JS::CallOriginalPromiseResolve(cx, args.get(0)));
  if (!response_promise)
    return false;

  // Step 2
  if (!is_dispatching(self)) {
    JS_ReportErrorUTF8(cx, "FetchEvent#respondWith must be called synchronously from "
                           "within a FetchEvent handler");
    return false;
  }

  // Step 3
  if (state(self) != State::unhandled && state(self) != State::waitToRespond) {
    JS_ReportErrorUTF8(cx, "FetchEvent#respondWith can't be called twice on the same event");
    return false;
  }

  // Step 4
  add_pending_promise(cx, self, response_promise);

  // Steps 5-7 (very roughly)
  set_state(self, State::waitToRespond);

  // Step 9 (continued in `response_promise_catch_handler` above)
  JS::RootedObject catch_handler(cx);
  JS::RootedValue extra(cx, JS::ObjectValue(*response_promise));
  catch_handler = create_internal_method<response_promise_catch_handler>(cx, self, extra);
  if (!catch_handler)
    return false;

  // Step 10 (continued in `response_promise_then_handler` above)
  JS::RootedObject then_handler(cx);
  then_handler = create_internal_method<response_promise_then_handler>(cx, self);
  if (!then_handler)
    return false;

  if (!JS::AddPromiseReactions(cx, response_promise, then_handler, catch_handler))
    return false;

  args.rval().setUndefined();
  return true;
}

bool FetchEvent::sendEarlyHints(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(1)
  MOZ_RELEASE_ASSERT(state(self) == State::unhandled || state(self) == State::waitToRespond);

  if (!is_dispatching(self)) {
    JS_ReportErrorUTF8(cx, "FetchEvent#sendEarlyHints must be called synchronously from "
                           "within a FetchEvent handler");
    return false;
  }
  if (state(self) != State::unhandled && state(self) != State::waitToRespond) {
    JS_ReportErrorUTF8(
        cx, "FetchEvent#sendEarlyHints can't be called after the main response has been sent");
    return false;
  }

  JS::RootedObject headers(cx, Headers::create(cx, args.get(0), Headers::HeadersGuard::None));
  // Calling get_list() transitions to Mode::ContentOnly or Mode::CachedInContent.
  if (!Headers::get_list(cx, headers)) {
    return false;
  }

  auto response_handle = host_api::HttpResp::make();
  if (auto *err = response_handle.to_err()) {
    HANDLE_ERROR(cx, *err);
    return false;
  }
  // 103: Early Hint
  auto set_res = response_handle.unwrap().set_status(103);
  if (auto *err = set_res.to_err()) {
    HANDLE_ERROR(cx, *err);
    return false;
  }

  auto body_handle = host_api::HttpBody::make();
  if (auto *err = body_handle.to_err()) {
    HANDLE_ERROR(cx, *err);
    return false;
  }

  JS::RootedObject response_instance(
      cx, JS_NewObjectWithGivenProto(cx, &Response::class_, Response::proto_obj));
  if (!response_instance) {
    return false;
  }
  RootedObject response_obj(cx, Response::create(cx, response_instance, response_handle.unwrap(),
                                                 body_handle.unwrap(), false, nullptr, nullptr,
                                                 nullptr));
  RootedValue headers_val(cx, JS::ObjectValue(*headers));
  JS::SetReservedSlot(response_obj, static_cast<uint32_t>(Response::Slots::Headers), headers_val);
  JS::SetReservedSlot(response_obj, static_cast<uint32_t>(Response::Slots::Status),
                      JS::Int32Value(103));
  RequestOrResponse::set_url(response_obj, JS_GetEmptyStringValue(cx));

  args.rval().setUndefined();
  return start_response(cx, response_obj, false);
}

bool FetchEvent::respondWithError(JSContext *cx, JS::HandleObject self) {
  MOZ_RELEASE_ASSERT(state(self) == State::unhandled || state(self) == State::waitToRespond);
  set_state(self, State::responsedWithError);

  auto response_res = host_api::HttpResp::make();
  if (auto *err = response_res.to_err()) {
    HANDLE_ERROR(cx, *err);
    return false;
  }

  auto make_res = host_api::HttpBody::make();
  if (auto *err = make_res.to_err()) {
    HANDLE_ERROR(cx, *err);
    return false;
  }

  auto response = response_res.unwrap();
  auto status_res = response.set_status(500);
  if (auto *err = status_res.to_err()) {
    HANDLE_ERROR(cx, *err);
    return false;
  }

  auto send_res = response.send_downstream(make_res.unwrap(), false);
  if (auto *err = send_res.to_err()) {
    HANDLE_ERROR(cx, *err);
    return false;
  }

  return true;
}

namespace {

// Step 5 of https://w3c.github.io/ServiceWorker/#wait-until-method
bool dec_pending_promise_count(JSContext *cx, JS::HandleObject event, JS::HandleValue extra,
                               JS::CallArgs args) {
  // Step 5.1
  dec_pending_promise_count(event);

  // Note: step 5.2 not relevant to our implementation.
  return true;
}

} // namespace

// Steps in this function refer to the spec at
// https://w3c.github.io/ServiceWorker/#wait-until-method
bool FetchEvent::waitUntil(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(1)

  JS::RootedObject promise(cx, JS::CallOriginalPromiseResolve(cx, args.get(0)));
  if (!promise)
    return false;

  // Step 2
  if (!is_active(self)) {
    JS_ReportErrorUTF8(cx, "FetchEvent#waitUntil called on inactive event");
    return false;
  }

  // Steps 3-4
  add_pending_promise(cx, self, promise);

  // Note: step 5 implemented in dec_pending_promise_count

  args.rval().setUndefined();
  return true;
}

const JSFunctionSpec FetchEvent::static_methods[] = {
    JS_FS_END,
};

const JSPropertySpec FetchEvent::static_properties[] = {
    JS_PS_END,
};

const JSFunctionSpec FetchEvent::methods[] = {
    JS_FN("respondWith", respondWith, 1, JSPROP_ENUMERATE),
    JS_FN("waitUntil", waitUntil, 1, JSPROP_ENUMERATE),
    JS_FN("sendEarlyHints", sendEarlyHints, 1, JSPROP_ENUMERATE),
    JS_FS_END,
};

const JSPropertySpec FetchEvent::properties[] = {
    JS_PSG("client", client_get, JSPROP_ENUMERATE),
    JS_PSG("request", request_get, JSPROP_ENUMERATE),
    JS_PSG("server", server_get, JSPROP_ENUMERATE),
    JS_PS_END,
};

JSObject *FetchEvent::create(JSContext *cx) {
  JS::RootedObject self(cx, JS_NewObjectWithGivenProto(cx, &class_, proto_obj));
  if (!self)
    return nullptr;

  if (!reset(cx, self)) {
    return nullptr;
  }

  INSTANCE.init(cx, self);
  return self;
}

bool FetchEvent::reset(JSContext *cx, JS::HandleObject self) {
  JS::RootedObject request(cx, prepare_downstream_request(cx));
  if (!request)
    return false;

  JS::RootedObject dec_count_handler(cx,
                                     create_internal_method<dec_pending_promise_count>(cx, self));
  if (!dec_count_handler)
    return false;

  JS::SetReservedSlot(self, static_cast<uint32_t>(Slots::Request), JS::ObjectValue(*request));
  JS::SetReservedSlot(self, static_cast<uint32_t>(Slots::Dispatch), JS::FalseValue());
  JS::SetReservedSlot(self, static_cast<uint32_t>(Slots::State),
                      JS::Int32Value((int)State::unhandled));
  JS::SetReservedSlot(self, static_cast<uint32_t>(Slots::PendingPromiseCount), JS::Int32Value(0));
  JS::SetReservedSlot(self, static_cast<uint32_t>(Slots::DecPendingPromiseCountFunc),
                      JS::ObjectValue(*dec_count_handler));
  JS::SetReservedSlot(self, static_cast<uint32_t>(Slots::ClientInfo), JS::UndefinedValue());
  JS::SetReservedSlot(self, static_cast<uint32_t>(Slots::ServerInfo), JS::UndefinedValue());
  return true;
}

JS::HandleObject FetchEvent::instance() {
  MOZ_ASSERT(INSTANCE);
  MOZ_ASSERT(is_instance(INSTANCE));
  return INSTANCE;
}

bool FetchEvent::is_active(JSObject *self) {
  MOZ_ASSERT(is_instance(self));
  // Note: we also treat the FetchEvent as active if it's in `responseStreaming`
  // state because that requires us to extend the service's lifetime as well. In
  // the spec this is achieved using individual promise counts for the body read
  // operations.
  return JS::GetReservedSlot(self, static_cast<uint32_t>(Slots::Dispatch)).toBoolean() ||
         state(self) == State::responseStreaming ||
         JS::GetReservedSlot(self, static_cast<uint32_t>(Slots::PendingPromiseCount)).toInt32() > 0;
}

bool FetchEvent::is_dispatching(JSObject *self) {
  MOZ_ASSERT(is_instance(self));
  return JS::GetReservedSlot(self, static_cast<uint32_t>(Slots::Dispatch)).toBoolean();
}

void FetchEvent::start_dispatching(JSObject *self) {
  MOZ_ASSERT(!is_dispatching(self));
  JS::SetReservedSlot(self, static_cast<uint32_t>(Slots::Dispatch), JS::TrueValue());
}

void FetchEvent::stop_dispatching(JSObject *self) {
  MOZ_ASSERT(is_dispatching(self));
  JS::SetReservedSlot(self, static_cast<uint32_t>(Slots::Dispatch), JS::FalseValue());
}

FetchEvent::State FetchEvent::state(JSObject *self) {
  MOZ_ASSERT(is_instance(self));
  return static_cast<State>(
      JS::GetReservedSlot(self, static_cast<uint32_t>(Slots::State)).toInt32());
}

void FetchEvent::mark_done(JSObject *self, bool streaming, uint16_t status_code) {
  MOZ_ASSERT(is_instance(self));
  auto new_state = [&] {
    // 103: Early Hint
    if (status_code == 103) {
      return State::unhandled;
    }
    if (streaming) {
      return State::responseStreaming;
    }
    return State::responseDone;
  }();
  JS::SetReservedSlot(self, static_cast<uint32_t>(Slots::State),
                      JS::Int32Value(static_cast<int32_t>(new_state)));
}

void FetchEvent::set_state(JSObject *self, State new_state) {
  MOZ_ASSERT(is_instance(self));
  JS::SetReservedSlot(self, static_cast<uint32_t>(Slots::State),
                      JS::Int32Value(static_cast<int32_t>(new_state)));
}

bool FetchEvent::response_started(JSObject *self) {
  auto current_state = state(self);
  return current_state != State::unhandled && current_state != State::waitToRespond;
}

static bool addEventListener(JSContext *cx, unsigned argc, Value *vp) {
  JS::CallArgs args = CallArgsFromVp(argc, vp);
  if (!args.requireAtLeast(cx, "addEventListener", 2)) {
    return false;
  }

  auto event_chars = core::encode(cx, args[0]);
  if (!event_chars) {
    return false;
  }

  if (strncmp(event_chars.begin(), "fetch", event_chars.len)) {
    fprintf(stderr,
            "Error: addEventListener only supports the event 'fetch' right now, "
            "but got event '%s'\n",
            event_chars.begin());
    exit(1);
  }

  RootedValue val(cx, args[1]);
  if (!val.isObject() || !JS_ObjectIsFunction(&val.toObject())) {
    fprintf(stderr, "Error: addEventListener: Argument 2 is not a function.\n");
    exit(1);
  }

  return FETCH_HANDLERS->append(&val.toObject());
}

bool install(api::Engine *engine) {
  ENGINE = engine;
  FETCH_HANDLERS = new JS::PersistentRootedObjectVector(engine->cx());

  if (!JS_DefineFunction(engine->cx(), engine->global(), "addEventListener", addEventListener, 2,
                         0)) {
    MOZ_RELEASE_ASSERT(false);
  }

  if (!FetchEvent::init_class(engine->cx(), engine->global()))
    return false;

  if (!FetchEvent::create(engine->cx())) {
    MOZ_RELEASE_ASSERT(false);
  }

  if (!ClientInfo::init_class(engine->cx(), engine->global())) {
    return false;
  }

  if (!ServerInfo::init_class(engine->cx(), engine->global())) {
    return false;
  }

  return true;
}

} // namespace fastly::fetch_event
