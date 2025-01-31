#include <algorithm>
#include <cstring>
#include <iostream>
#include <optional>
#include <string>

#include "../../../StarlingMonkey/builtins/web/fetch/fetch-errors.h"
#include "../../../StarlingMonkey/builtins/web/streams/native-stream-source.h"
#include "../../../StarlingMonkey/builtins/web/url.h"
#include "../../../StarlingMonkey/runtime/encode.h"
#include "../host-api/host_api_fastly.h"
#include "./fetch/request-response.h"
#include "body.h"
#include "fastly.h"
#include "js/Stream.h"

using builtins::web::streams::NativeStreamSource;
using fastly::FastlyGetErrorMessage;
using fastly::fastly::convertBodyInit;
using fastly::fetch::RequestOrResponse;

namespace fastly::body {

host_api::HttpBody host_body(JSObject *obj) {
  JS::Value val = JS::GetReservedSlot(obj, static_cast<uint32_t>(FastlyBody::Slots::Body));
  return host_api::HttpBody(static_cast<uint32_t>(val.toInt32()));
}

// concat(dest: FastlyBody): void;
bool FastlyBody::concat(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(1);
  auto destination_val = args.get(0);
  if (!FastlyBody::is_instance(destination_val)) {
    JS_ReportErrorUTF8(
        cx, "FastlyBody.concat: The `destination` argument is not an instance of FastlyBody.");
    return false;
  }
  JS::RootedObject destination(cx, &destination_val.toObject());

  auto body = host_body(self);
  auto result = body.append(host_body(destination));
  if (auto *err = result.to_err()) {
    HANDLE_ERROR(cx, *err);
    return false;
  }

  args.rval().setUndefined();
  return true;
}

// read(chunkSize: number): ArrayBuffer;
bool FastlyBody::read(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(1);
  double chunkSize_val;
  if (!JS::ToNumber(cx, args.get(0), &chunkSize_val)) {
    return false;
  }
  if (chunkSize_val < 0) {
    JS_ReportErrorUTF8(cx,
                       "FastlyBody.read: The `chunkSize` argument has to be a positive integer.");
    return false;
  }
  if (chunkSize_val < 0 || std::isnan(chunkSize_val) || std::isinf(chunkSize_val)) {
    JS_ReportErrorASCII(cx, "chunkSize parameter is an invalid value, only positive numbers can be "
                            "used for chunkSize.");
    return false;
  }
  uint32_t chunkSize = std::round(chunkSize_val);

  auto body = host_body(self);
  auto result = body.read(chunkSize);
  if (auto *err = result.to_err()) {
    if (chunkSize < 0) {
      JS_ReportErrorUTF8(cx,
                         "FastlyBody.read: The `chunkSize` argument has to be a positive integer.");
      return false;
    }
    HANDLE_ERROR(cx, *err);
    return false;
  }

  auto &chunk = result.unwrap();
  JS::UniqueChars buffer = std::move(chunk.ptr);
  JS::RootedObject array_buffer(cx);
  array_buffer.set(JS::NewArrayBufferWithContents(
      cx, chunk.len, buffer.get(), JS::NewArrayBufferOutOfMemory::CallerMustFreeMemory));
  if (!array_buffer) {
    JS_ReportOutOfMemory(cx);
    return false;
  }

  // `array_buffer` now owns `buf`
  static_cast<void>(buffer.release());

  args.rval().setObject(*array_buffer);
  return true;
}

// append(data: BodyInit): void;
bool FastlyBody::append(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(1);

  auto body = host_body(self);
  if (!body.valid()) {
    return false;
  }

  auto data_val = args.get(0);
  host_api::HttpBody source_body;
  JS::UniqueChars data;
  JS::RootedObject data_obj(cx, data_val.isObject() ? &data_val.toObject() : nullptr);
  // If the data parameter is a Host-backed ReadableStream we optimise our implementation
  // by using the ReadableStream's handle directly.
  if (data_obj && JS::IsReadableStream(data_obj)) {
    if (RequestOrResponse::body_unusable(cx, data_obj)) {
      api::throw_error(cx, FetchErrors::BodyStreamUnusable);
    }

    // If the stream is backed by a C@E body handle, we can use that handle directly.
    if (NativeStreamSource::stream_is_body(cx, data_obj)) {
      JS::RootedObject stream_source(cx, NativeStreamSource::get_stream_source(cx, data_obj));
      JS::RootedObject source_owner(cx, NativeStreamSource::owner(stream_source));

      source_body = RequestOrResponse::body_handle(source_owner);
      if (!source_body.valid()) {
        return false;
      }
      auto res = body.append(source_body);
      if (auto *err = res.to_err()) {
        HANDLE_ERROR(cx, *err);
        return false;
      }
      args.rval().setUndefined();
      return true;
    } else {
      JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr,
                                JSMSG_BODY_APPEND_CONTENT_STREAM);
      return false;
    }
  } else {
    auto result = convertBodyInit(cx, data_val);
    if (result.isErr()) {
      return false;
    }
    size_t length;
    std::tie(data, length) = result.unwrap();
    auto write_res = body.write_all_back(reinterpret_cast<uint8_t *>(data.get()), length);
    if (auto *err = write_res.to_err()) {
      HANDLE_ERROR(cx, *err);
      return false;
    }
    args.rval().setUndefined();
    return true;
  }
}

// prepend(data: BodyInit): void;
bool FastlyBody::prepend(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(1);

  auto body = host_body(self);
  if (!body.valid()) {
    return false;
  }

  auto data_val = args.get(0);
  host_api::HttpBody source_body;
  JS::RootedObject data_obj(cx, data_val.isObject() ? &data_val.toObject() : nullptr);
  // If the data parameter is a Host-backed ReadableStream we optimise our implementation
  // by using the ReadableStream's handle directly.
  if (data_obj && JS::IsReadableStream(data_obj)) {
    if (RequestOrResponse::body_unusable(cx, data_obj)) {
      return api::throw_error(cx, FetchErrors::BodyStreamUnusable);
    }

    // If the stream is backed by a C@E body handle, we can use that handle directly.
    if (NativeStreamSource::stream_is_body(cx, data_obj)) {
      JS::RootedObject stream_source(cx, NativeStreamSource::get_stream_source(cx, data_obj));
      JS::RootedObject source_owner(cx, NativeStreamSource::owner(stream_source));

      source_body = RequestOrResponse::body_handle(source_owner);
      if (!source_body.valid()) {
        return false;
      }

      auto make_res = host_api::HttpBody::make();
      if (auto *err = make_res.to_err()) {
        HANDLE_ERROR(cx, *err);
        return false;
      }

      auto new_body = make_res.unwrap();
      if (!new_body.valid()) {
        return false;
      }

      auto res = new_body.append(source_body);
      if (auto *err = res.to_err()) {
        HANDLE_ERROR(cx, *err);
        return false;
      }
      res = new_body.append(body);
      if (auto *err = res.to_err()) {
        HANDLE_ERROR(cx, *err);
        return false;
      }

      JS::SetReservedSlot(self, static_cast<uint32_t>(Slots::Body),
                          JS::Int32Value(new_body.handle));

      args.rval().setUndefined();
      return true;
    } else {
      JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr,
                                JSMSG_BODY_PREPEND_CONTENT_STREAM);
      return false;
    }
  } else {
    auto result = convertBodyInit(cx, data_val);
    if (result.isErr()) {
      return false;
    }
    size_t length;
    JS::UniqueChars data;
    std::tie(data, length) = result.unwrap();
    auto write_res = body.write_all_front(reinterpret_cast<uint8_t *>(data.get()), length);
    if (auto *err = write_res.to_err()) {
      HANDLE_ERROR(cx, *err);
      return false;
    }
    args.rval().setUndefined();
    return true;
  }
}

// close(): void;
bool FastlyBody::close(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0);
  auto body = host_body(self);
  auto result = body.close();

  if (auto *err = result.to_err()) {
    HANDLE_ERROR(cx, *err);
    return false;
  }
  args.rval().setUndefined();
  return true;
}

bool FastlyBody::abandon(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0);
  auto body = host_body(self);
  auto result = body.abandon();

  if (auto *err = result.to_err()) {
    HANDLE_ERROR(cx, *err);
    return false;
  }
  args.rval().setUndefined();
  return true;
}

const JSFunctionSpec FastlyBody::static_methods[] = {
    JS_FS_END,
};

const JSPropertySpec FastlyBody::static_properties[] = {
    JS_PS_END,
};

const JSFunctionSpec FastlyBody::methods[] = {
    JS_FN("concat", concat, 1, JSPROP_ENUMERATE),
    JS_FN("read", read, 1, JSPROP_ENUMERATE),
    JS_FN("append", append, 1, JSPROP_ENUMERATE),
    JS_FN("prepend", prepend, 1, JSPROP_ENUMERATE),
    JS_FN("close", close, 0, JSPROP_ENUMERATE),
    JS_FN("abandon", abandon, 0, JSPROP_ENUMERATE),
    JS_FS_END,
};

const JSPropertySpec FastlyBody::properties[] = {
    JS_STRING_SYM_PS(toStringTag, "FastlyBody", JSPROP_READONLY),
    JS_PS_END,
};

bool FastlyBody::constructor(JSContext *cx, unsigned argc, JS::Value *vp) {
  REQUEST_HANDLER_ONLY("The FastlyBody builtin");
  CTOR_HEADER("FastlyBody", 0);

  auto make_res = host_api::HttpBody::make();
  if (auto *err = make_res.to_err()) {
    HANDLE_ERROR(cx, *err);
    return false;
  }

  auto body = make_res.unwrap();
  if (!body.valid()) {
    return false;
  }

  JS::RootedObject instance(cx, JS_NewObjectForConstructor(cx, &class_, args));
  if (!instance) {
    return false;
  }
  JS::SetReservedSlot(instance, static_cast<uint32_t>(Slots::Body), JS::Int32Value(body.handle));

  args.rval().setObject(*instance);
  return true;
}

JSObject *FastlyBody::create(JSContext *cx, uint32_t handle) {
  host_api::HttpBody body{handle};

  JS::RootedObject instance(
      cx, JS_NewObjectWithGivenProto(cx, &FastlyBody::class_, FastlyBody::proto_obj));
  if (!instance) {
    return nullptr;
  }
  JS::SetReservedSlot(instance, static_cast<uint32_t>(Slots::Body), JS::Int32Value(handle));
  return instance;
}

bool install(api::Engine *engine) {
  if (!FastlyBody::init_class_impl(engine->cx(), engine->global())) {
    return false;
  }
  RootedObject body_obj(engine->cx(), JS_GetConstructor(engine->cx(), FastlyBody::proto_obj));
  RootedValue body_val(engine->cx(), ObjectValue(*body_obj));
  RootedObject body_ns(engine->cx(), JS_NewObject(engine->cx(), nullptr));
  if (!JS_SetProperty(engine->cx(), body_ns, "FastlyBody", body_val)) {
    return false;
  }
  RootedValue body_ns_val(engine->cx(), JS::ObjectValue(*body_ns));
  if (!engine->define_builtin_module("fastly:body", body_ns_val)) {
    return false;
  }
  return true;
}

} // namespace fastly::body
