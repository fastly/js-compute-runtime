#include <algorithm>
#include <cstring>
#include <iostream>
#include <optional>
#include <string>

#include "builtin.h"
#include "builtins/body.h"
#include "builtins/native-stream-source.h"
#include "builtins/shared/url.h"
#include "core/encode.h"
#include "host_interface/host_api.h"
#include "js-compute-builtins.h"
#include "js/Stream.h"

namespace builtins {
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
  uint32_t chunkSize = std::round(chunkSize_val);
  if (chunkSize < 0) {
    JS_ReportErrorUTF8(cx,
                       "FastlyBody.read: The `chunkSize` argument has to be a positive integer.");
    return false;
  }

  auto body = host_body(self);
  auto result = body.read(chunkSize);
  if (auto *err = result.to_err()) {
    HANDLE_ERROR(cx, *err);
    return false;
  }

  auto &chunk = result.unwrap();
  JS::UniqueChars buffer = std::move(chunk.ptr);
  JS::RootedObject array_buffer(cx);
  array_buffer.set(JS::NewArrayBufferWithContents(cx, chunk.len, buffer.get()));
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
      JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr,
                                JSMSG_READABLE_STREAM_LOCKED_OR_DISTRUBED);
      return false;
    }

    // If the stream is backed by a C@E body handle, we can use that handle directly.
    if (builtins::NativeStreamSource::stream_is_body(cx, data_obj)) {
      JS::RootedObject stream_source(cx,
                                     builtins::NativeStreamSource::get_stream_source(cx, data_obj));
      JS::RootedObject source_owner(cx, builtins::NativeStreamSource::owner(stream_source));

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
      JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr,
                                JSMSG_SIMPLE_CACHE_SET_CONTENT_STREAM);
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
      JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr,
                                JSMSG_READABLE_STREAM_LOCKED_OR_DISTRUBED);
      return false;
    }

    // If the stream is backed by a C@E body handle, we can use that handle directly.
    if (builtins::NativeStreamSource::stream_is_body(cx, data_obj)) {
      JS::RootedObject stream_source(cx,
                                     builtins::NativeStreamSource::get_stream_source(cx, data_obj));
      JS::RootedObject source_owner(cx, builtins::NativeStreamSource::owner(stream_source));

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
      JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr,
                                JSMSG_SIMPLE_CACHE_SET_CONTENT_STREAM);
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

const JSFunctionSpec FastlyBody::static_methods[] = {
    JS_FS_END,
};

const JSPropertySpec FastlyBody::static_properties[] = {
    JS_PS_END,
};

const JSFunctionSpec FastlyBody::methods[] = {
    JS_FN("concat", concat, 1, JSPROP_ENUMERATE), JS_FN("read", read, 1, JSPROP_ENUMERATE),
    JS_FN("append", append, 1, JSPROP_ENUMERATE), JS_FN("prepend", prepend, 1, JSPROP_ENUMERATE),
    JS_FN("close", close, 0, JSPROP_ENUMERATE),   JS_FS_END,
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

  JS::RootedObject instance(cx, JS_NewObjectWithGivenProto(cx, &builtins::FastlyBody::class_,
                                                           builtins::FastlyBody::proto_obj));
  if (!instance) {
    return nullptr;
  }
  JS::SetReservedSlot(instance, static_cast<uint32_t>(Slots::Body), JS::Int32Value(handle));
  return instance;
}

bool FastlyBody::init_class(JSContext *cx, JS::HandleObject global) {
  return init_class_impl(cx, global);
}

} // namespace builtins
