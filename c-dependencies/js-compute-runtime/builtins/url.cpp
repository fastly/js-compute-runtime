#include "js/Array.h"

#include "builtin.h"
#include "builtins/url.h"
#include "core/sequence.hpp"
#include "rust-url/rust-url.h"

constexpr int ITERTYPE_ENTRIES = 0;
constexpr int ITERTYPE_KEYS = 1;
constexpr int ITERTYPE_VALUES = 2;

namespace builtins {

bool URLSearchParamsIterator::next(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0)
  JS::RootedObject params_obj(cx, &JS::GetReservedSlot(self, Slots::Params).toObject());
  const auto params = URLSearchParams::get_params(params_obj);
  size_t index = JS::GetReservedSlot(self, Slots::Index).toInt32();
  uint8_t type = JS::GetReservedSlot(self, Slots::Type).toInt32();

  JS::RootedObject result(cx, JS_NewPlainObject(cx));
  if (!result)
    return false;

  jsurl::JSSearchParam param{jsurl::SpecSlice(nullptr, 0), jsurl::SpecSlice(nullptr, 0), false};
  jsurl::params_at(params, index, &param);

  if (param.done) {
    JS_DefineProperty(cx, result, "done", true, JSPROP_ENUMERATE);
    JS_DefineProperty(cx, result, "value", JS::UndefinedHandleValue, JSPROP_ENUMERATE);

    args.rval().setObject(*result);
    return true;
  }

  JS_DefineProperty(cx, result, "done", false, JSPROP_ENUMERATE);

  JS::RootedValue key_val(cx);
  JS::RootedValue val_val(cx);

  if (type != ITERTYPE_VALUES) {
    auto chars = JS::UTF8Chars((char *)param.name.data, param.name.len);
    JS::RootedString str(cx, JS_NewStringCopyUTF8N(cx, chars));
    if (!str)
      return false;
    key_val = JS::StringValue(str);
  }

  if (type != ITERTYPE_KEYS) {
    auto chars = JS::UTF8Chars((char *)param.value.data, param.value.len);
    JS::RootedString str(cx, JS_NewStringCopyUTF8N(cx, chars));
    if (!str)
      return false;
    val_val = JS::StringValue(str);
  }

  JS::RootedValue result_val(cx);

  switch (type) {
  case ITERTYPE_ENTRIES: {
    JS::RootedObject pair(cx, JS::NewArrayObject(cx, 2));
    if (!pair)
      return false;
    JS_DefineElement(cx, pair, 0, key_val, JSPROP_ENUMERATE);
    JS_DefineElement(cx, pair, 1, val_val, JSPROP_ENUMERATE);
    result_val = JS::ObjectValue(*pair);
    break;
  }
  case ITERTYPE_KEYS: {
    result_val = key_val;
    break;
  }
  case ITERTYPE_VALUES: {
    result_val = val_val;
    break;
  }
  default:
    MOZ_RELEASE_ASSERT(false, "Invalid iter type");
  }

  JS_DefineProperty(cx, result, "value", result_val, JSPROP_ENUMERATE);

  JS::SetReservedSlot(self, Slots::Index, JS::Int32Value(index + 1));
  args.rval().setObject(*result);
  return true;
}

const JSFunctionSpec URLSearchParamsIterator::methods[] = {
    JS_FN("next", URLSearchParamsIterator::next, 0, JSPROP_ENUMERATE),
    JS_FS_END,
};

const JSPropertySpec URLSearchParamsIterator::properties[] = {
    JS_PS_END,
};

// This constructor will be deleted from the class prototype right after class
// initialization.
bool URLSearchParamsIterator::constructor(JSContext *cx, unsigned argc, JS::Value *vp) {
  MOZ_RELEASE_ASSERT(false, "Should be deleted");
  return false;
}

bool URLSearchParamsIterator::init_class(JSContext *cx, JS::HandleObject global) {
  JS::RootedObject iterator_proto(cx, JS::GetRealmIteratorPrototype(cx));
  if (!iterator_proto)
    return false;

  if (!init_class_impl(cx, global, iterator_proto))
    return false;

  // Delete both the `URLSearchParamsIterator` global property and the
  // `constructor` property on `URLSearchParamsIterator.prototype`. The latter
  // because Iterators don't have their own constructor on the prototype.
  return JS_DeleteProperty(cx, global, class_.name) &&
         JS_DeleteProperty(cx, proto_obj, "constructor");
}

JSObject *URLSearchParamsIterator::create(JSContext *cx, JS::HandleObject params, uint8_t type) {
  MOZ_RELEASE_ASSERT(type <= ITERTYPE_VALUES);

  JS::RootedObject self(cx, JS_NewObjectWithGivenProto(cx, &class_, proto_obj));
  if (!self)
    return nullptr;

  JS::SetReservedSlot(self, Slots::Params, JS::ObjectValue(*params));
  JS::SetReservedSlot(self, Slots::Type, JS::Int32Value(type));
  JS::SetReservedSlot(self, Slots::Index, JS::Int32Value(0));

  return self;
}

const JSFunctionSpec URLSearchParams::methods[] = {
    JS_FN("append", URLSearchParams::append, 2, JSPROP_ENUMERATE),
    JS_FN("delete", URLSearchParams::delete_, 1, JSPROP_ENUMERATE),
    JS_FN("has", URLSearchParams::has, 1, JSPROP_ENUMERATE),
    JS_FN("get", URLSearchParams::get, 1, JSPROP_ENUMERATE),
    JS_FN("getAll", URLSearchParams::getAll, 1, JSPROP_ENUMERATE),
    JS_FN("set", URLSearchParams::set, 2, JSPROP_ENUMERATE),
    JS_FN("sort", URLSearchParams::sort, 0, JSPROP_ENUMERATE),
    JS_FN("toString", URLSearchParams::toString, 0, JSPROP_ENUMERATE),
    JS_FN("forEach", URLSearchParams::forEach, 0, JSPROP_ENUMERATE),
    JS_FN("entries", URLSearchParams::entries, 0, 0),
    JS_FN("keys", URLSearchParams::keys, 0, 0),
    JS_FN("values", URLSearchParams::values, 0, 0),
    // [Symbol.iterator] added in init_class.
    JS_FS_END,
};

const JSPropertySpec URLSearchParams::properties[] = {
    JS_PS_END,
};

jsurl::JSUrlSearchParams *URLSearchParams::get_params(JSObject *self) {
  return static_cast<jsurl::JSUrlSearchParams *>(
      JS::GetReservedSlot(self, Slots::Params).toPrivate());
}

namespace {
bool append_impl(JSContext *cx, JS::HandleObject self, JS::HandleValue key, JS::HandleValue val,
                 const char *_) {
  const auto params = URLSearchParams::get_params(self);

  auto name = encode(cx, key);
  if (!name.data)
    return false;

  auto value = encode(cx, val);
  if (!value.data)
    return false;

  jsurl::params_append(params, name, value);
  return true;
}
} // namespace

jsurl::SpecSlice URLSearchParams::serialize(JSContext *cx, JS::HandleObject self) {
  return jsurl::params_to_string(get_params(self));
}

bool URLSearchParams::append(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(2)
  if (!append_impl(cx, self, args[0], args[1], "append"))
    return false;

  args.rval().setUndefined();
  return true;
}

bool URLSearchParams::delete_(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER_WITH_NAME(1, "delete");

  auto *params =
      static_cast<jsurl::JSUrlSearchParams *>(JS::GetReservedSlot(self, Slots::Params).toPrivate());

  auto name = encode(cx, args.get(0));
  if (!name.data)
    return false;

  jsurl::params_delete(params, &name);
  args.rval().setUndefined();
  return true;
}

bool URLSearchParams::has(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(1)
  auto *params =
      static_cast<jsurl::JSUrlSearchParams *>(JS::GetReservedSlot(self, Slots::Params).toPrivate());

  auto name = encode(cx, args.get(0));
  if (!name.data)
    return false;

  args.rval().setBoolean(jsurl::params_has(params, &name));
  return true;
}

bool URLSearchParams::get(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(1);
  auto *params = static_cast<const jsurl::JSUrlSearchParams *>(
      JS::GetReservedSlot(self, Slots::Params).toPrivate());

  auto name = encode(cx, args.get(0));
  if (!name.data)
    return false;

  const jsurl::SpecSlice slice = jsurl::params_get(params, &name);
  if (!slice.data) {
    args.rval().setNull();
    return true;
  }

  JS::RootedString str(cx, JS_NewStringCopyUTF8N(cx, JS::UTF8Chars((char *)slice.data, slice.len)));
  if (!str)
    return false;
  args.rval().setString(str);
  return true;
}

bool URLSearchParams::getAll(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(1);

  auto *params = static_cast<const jsurl::JSUrlSearchParams *>(
      JS::GetReservedSlot(self, Slots::Params).toPrivate());

  auto name = encode(cx, args.get(0));
  if (!name.data)
    return false;

  const jsurl::CVec<jsurl::SpecSlice> values = jsurl::params_get_all(params, &name);

  JS::RootedObject result(cx, JS::NewArrayObject(cx, values.len));
  if (!result)
    return false;

  JS::RootedString str(cx);
  JS::RootedValue str_val(cx);
  for (size_t i = 0; i < values.len; i++) {
    const jsurl::SpecSlice value = values.ptr[i];
    str = JS_NewStringCopyUTF8N(cx, JS::UTF8Chars((char *)value.data, value.len));
    if (!str)
      return false;

    str_val.setString(str);
    if (!JS_SetElement(cx, result, i, str_val))
      return false;
  }

  args.rval().setObject(*result);
  return true;
}

bool URLSearchParams::set(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(2);

  auto *params =
      static_cast<jsurl::JSUrlSearchParams *>(JS::GetReservedSlot(self, Slots::Params).toPrivate());

  auto name = encode(cx, args[0]);
  if (!name.data)
    return false;

  auto value = encode(cx, args[1]);
  if (!value.data)
    return false;

  jsurl::params_set(params, name, value);
  return true;

  args.rval().setUndefined();
  return true;
}

bool URLSearchParams::sort(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0)
  auto *params =
      static_cast<jsurl::JSUrlSearchParams *>(JS::GetReservedSlot(self, Slots::Params).toPrivate());
  jsurl::params_sort(params);
  args.rval().setUndefined();
  return true;
}

bool URLSearchParams::toString(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0)
  jsurl::SpecSlice slice = serialize(cx, self);
  JS::RootedString str(
      cx, JS_NewStringCopyUTF8N(
              cx, JS::UTF8Chars(reinterpret_cast<const char *>(slice.data), slice.len)));
  if (!str)
    return false;

  args.rval().setString(str);
  return true;
}

bool URLSearchParams::forEach(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(1)
  const auto params = get_params(self);

  if (!args[0].isObject() || !JS::IsCallable(&args[0].toObject())) {
    JS_ReportErrorASCII(cx, "Failed to execute 'forEach' on 'URLSearchParams': "
                            "parameter 1 is not of type 'Function'");
    return false;
  }

  JS::HandleValue callback = args[0];
  JS::HandleValue thisv = args.get(1);

  JS::RootedValueArray<3> newArgs(cx);
  newArgs[2].setObject(*self);
  JS::RootedValue rval(cx);

  jsurl::JSSearchParam param{jsurl::SpecSlice(nullptr, 0), jsurl::SpecSlice(nullptr, 0), false};
  JS::RootedString name_str(cx);
  JS::RootedString val_str(cx);
  size_t index = 0;
  while (true) {
    jsurl::params_at(params, index, &param);
    if (param.done)
      break;

    name_str = JS_NewStringCopyUTF8N(cx, JS::UTF8Chars((char *)param.name.data, param.name.len));
    if (!name_str)
      return false;
    newArgs[1].setString(name_str);

    val_str = JS_NewStringCopyUTF8N(cx, JS::UTF8Chars((char *)param.value.data, param.value.len));
    if (!val_str)
      return false;
    newArgs[0].setString(val_str);

    if (!JS::Call(cx, thisv, callback, newArgs, &rval))
      return false;

    index++;
  }

  args.rval().setUndefined();
  return true;
}

#define ITERATOR_METHOD(name, type)                                                                \
  bool URLSearchParams::name(JSContext *cx, unsigned argc, JS::Value *vp) {                        \
    METHOD_HEADER(0)                                                                               \
                                                                                                   \
    JS::RootedObject iter(cx, builtins::URLSearchParamsIterator::create(cx, self, type));          \
    if (!iter)                                                                                     \
      return false;                                                                                \
    args.rval().setObject(*iter);                                                                  \
    return true;                                                                                   \
  }

ITERATOR_METHOD(entries, ITERTYPE_ENTRIES);
ITERATOR_METHOD(keys, ITERTYPE_KEYS);
ITERATOR_METHOD(values, ITERTYPE_VALUES);

#undef ITERTYPE_VALUES

bool URLSearchParams::constructor(JSContext *cx, unsigned argc, JS::Value *vp) {
  CTOR_HEADER("URLSearchParams", 0);

  JS::RootedObject urlSearchParamsInstance(cx, JS_NewObjectForConstructor(cx, &class_, args));
  JS::RootedObject self(cx, create(cx, urlSearchParamsInstance, args.get(0)));
  if (!self)
    return false;

  args.rval().setObject(*self);
  return true;
}

bool URLSearchParams::init_class(JSContext *cx, JS::HandleObject global) {
  if (!init_class_impl(cx, global))
    return false;

  JS::RootedValue entries(cx);
  if (!JS_GetProperty(cx, proto_obj, "entries", &entries))
    return false;

  JS::SymbolCode code = JS::SymbolCode::iterator;
  JS::RootedId iteratorId(cx, JS::GetWellKnownSymbolKey(cx, code));
  return JS_DefinePropertyById(cx, proto_obj, iteratorId, entries, 0);
}

JSObject *URLSearchParams::create(JSContext *cx, JS::HandleObject self,
                                  JS::HandleValue params_val) {
  auto params = jsurl::new_params();
  JS::SetReservedSlot(self, Slots::Params, JS::PrivateValue(params));

  bool consumed = false;
  const char *alt_text = ", or a value that can be stringified";
  if (!maybe_consume_sequence_or_record<append_impl>(cx, params_val, self, &consumed,
                                                     "URLSearchParams", alt_text)) {
    return nullptr;
  }

  if (!consumed) {
    auto init = encode(cx, params_val);
    if (!init.data)
      return nullptr;

    jsurl::params_init(params, &init);
  }

  return self;
}

JSObject *URLSearchParams::create(JSContext *cx, JS::HandleObject self, jsurl::JSUrl *url) {

  jsurl::JSUrlSearchParams *params = jsurl::url_search_params(url);
  if (!params)
    return nullptr;

  JS::SetReservedSlot(self, Slots::Params, JS::PrivateValue(params));
  JS::SetReservedSlot(self, Slots::Url, JS::PrivateValue(url));

  return self;
}

#define ACCESSOR_GET(field)                                                                        \
  bool URL::field(JSContext *cx, JS::HandleObject self, JS::MutableHandleValue rval) {             \
    const jsurl::JSUrl *url =                                                                      \
        static_cast<jsurl::JSUrl *>(JS::GetReservedSlot(self, URL::Slots::Url).toPrivate());       \
    const jsurl::SpecSlice slice = jsurl::field(url);                                              \
    JS::RootedString str(cx,                                                                       \
                         JS_NewStringCopyUTF8N(cx, JS::UTF8Chars((char *)slice.data, slice.len))); \
    if (!str) {                                                                                    \
      return false;                                                                                \
    }                                                                                              \
                                                                                                   \
    rval.setString(str);                                                                           \
    return true;                                                                                   \
  }                                                                                                \
                                                                                                   \
  bool URL::field##_get(JSContext *cx, unsigned argc, JS::Value *vp) {                             \
    METHOD_HEADER(0)                                                                               \
    return field(cx, self, args.rval());                                                           \
  }

#define ACCESSOR_SET(field)                                                                        \
  bool URL::field##_set(JSContext *cx, unsigned argc, JS::Value *vp) {                             \
    METHOD_HEADER(1)                                                                               \
    jsurl::JSUrl *url =                                                                            \
        static_cast<jsurl::JSUrl *>(JS::GetReservedSlot(self, URL::Slots::Url).toPrivate());       \
                                                                                                   \
    jsurl::SpecString str = encode(cx, args.get(0));                                               \
    if (!str.data) {                                                                               \
      return false;                                                                                \
    }                                                                                              \
                                                                                                   \
    jsurl::set_##field(url, &str);                                                                 \
                                                                                                   \
    args.rval().set(args.get(0));                                                                  \
    return true;                                                                                   \
  }

#define ACCESSOR(field)                                                                            \
  ACCESSOR_GET(field)                                                                              \
  ACCESSOR_SET(field)

ACCESSOR(hash)
ACCESSOR(host)
ACCESSOR(hostname)
ACCESSOR(href)
ACCESSOR(password)
ACCESSOR(pathname)
ACCESSOR(port)
ACCESSOR(protocol)
ACCESSOR(search)
ACCESSOR(username)

#undef ACCESSOR_GET
#undef ACCESSOR_SET
#undef ACCESSOR

const JSFunctionSpec URL::methods[] = {
    JS_FN("toString", toString, 0, JSPROP_ENUMERATE),
    JS_FN("toJSON", toJSON, 0, JSPROP_ENUMERATE),
    JS_FS_END,
};

const JSPropertySpec URL::properties[] = {
    JS_PSGS("hash", URL::hash_get, URL::hash_set, JSPROP_ENUMERATE),
    JS_PSGS("host", URL::host_get, URL::host_set, JSPROP_ENUMERATE),
    JS_PSGS("hostname", URL::hostname_get, URL::hostname_set, JSPROP_ENUMERATE),
    JS_PSGS("href", URL::href_get, URL::href_set, JSPROP_ENUMERATE),
    JS_PSG("origin", URL::origin_get, JSPROP_ENUMERATE),
    JS_PSGS("password", URL::password_get, URL::password_set, JSPROP_ENUMERATE),
    JS_PSGS("pathname", URL::pathname_get, URL::pathname_set, JSPROP_ENUMERATE),
    JS_PSGS("port", URL::port_get, URL::port_set, JSPROP_ENUMERATE),
    JS_PSGS("protocol", URL::protocol_get, URL::protocol_set, JSPROP_ENUMERATE),
    JS_PSGS("search", URL::search_get, URL::search_set, JSPROP_ENUMERATE),
    JS_PSG("searchParams", URL::searchParams_get, JSPROP_ENUMERATE),
    JS_PSGS("username", URL::username_get, URL::username_set, JSPROP_ENUMERATE),
    JS_PS_END,
};

jsurl::SpecString URL::origin(JSContext *cx, JS::HandleObject self) {
  auto *url = static_cast<const jsurl::JSUrl *>(JS::GetReservedSlot(self, Slots::Url).toPrivate());
  return jsurl::origin(url);
}

bool URL::origin(JSContext *cx, JS::HandleObject self, JS::MutableHandleValue rval) {
  jsurl::SpecString slice = origin(cx, self);
  JS::RootedString str(cx, JS_NewStringCopyUTF8N(cx, JS::UTF8Chars((char *)slice.data, slice.len)));
  if (!str)
    return false;
  rval.setString(str);
  return true;
}

bool URL::origin_get(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0)
  return origin(cx, self, args.rval());
}

bool URL::searchParams_get(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0)
  JS::Value params_val = JS::GetReservedSlot(self, Slots::Params);
  JS::RootedObject params(cx);
  if (params_val.isNullOrUndefined()) {
    auto *url = static_cast<jsurl::JSUrl *>(JS::GetReservedSlot(self, Slots::Url).toPrivate());
    JS::RootedObject url_search_params_instance(
        cx, JS_NewObjectWithGivenProto(cx, &builtins::URLSearchParams::class_,
                                       builtins::URLSearchParams::proto_obj));
    if (!url_search_params_instance)
      return false;
    params = builtins::URLSearchParams::create(cx, url_search_params_instance, url);
    if (!params)
      return false;
    JS::SetReservedSlot(self, Slots::Params, JS::ObjectValue(*params));
  } else {
    params = &params_val.toObject();
  }

  args.rval().setObject(*params);
  return true;
}

bool URL::toString(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0)
  return href_get(cx, argc, vp);
}

bool URL::toJSON(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0)
  return href_get(cx, argc, vp);
}

bool URL::constructor(JSContext *cx, unsigned argc, JS::Value *vp) {
  CTOR_HEADER("URL", 1);

  JS::RootedObject urlInstance(cx, JS_NewObjectForConstructor(cx, &class_, args));
  if (!urlInstance)
    return false;
  JS::RootedObject self(cx, create(cx, urlInstance, args.get(0), args.get(1)));
  if (!self)
    return false;

  args.rval().setObject(*self);
  return true;
}

JSObject *URL::create(JSContext *cx, JS::HandleObject self, jsurl::SpecString url_str,
                      const jsurl::JSUrl *base) {
  jsurl::JSUrl *url;
  if (base) {
    url = jsurl::new_jsurl_with_base(&url_str, base);
  } else {
    url = jsurl::new_jsurl(&url_str);
  }

  if (!url) {
    JS_ReportErrorUTF8(cx, "URL constructor: %s is not a valid URL.", (char *)url_str.data);
    return nullptr;
  }

  JS::SetReservedSlot(self, Slots::Url, JS::PrivateValue(url));

  return self;
}

JSObject *URL::create(JSContext *cx, JS::HandleObject self, JS::HandleValue url_val,
                      const jsurl::JSUrl *base) {
  auto str = encode(cx, url_val);
  if (!str.data)
    return nullptr;

  return create(cx, self, str, base);
}

JSObject *URL::create(JSContext *cx, JS::HandleObject self, JS::HandleValue url_val,
                      JS::HandleObject base_obj) {
  MOZ_RELEASE_ASSERT(is_instance(base_obj));
  const auto *base =
      static_cast<jsurl::JSUrl *>(JS::GetReservedSlot(base_obj, Slots::Url).toPrivate());

  return create(cx, self, url_val, base);
}

JSObject *URL::create(JSContext *cx, JS::HandleObject self, JS::HandleValue url_val,
                      JS::HandleValue base_val) {
  if (is_instance(base_val)) {
    JS::RootedObject base_obj(cx, &base_val.toObject());
    return create(cx, self, url_val, base_obj);
  }

  jsurl::JSUrl *base = nullptr;

  if (!base_val.isUndefined()) {
    auto str = encode(cx, base_val);
    if (!str.data)
      return nullptr;

    base = jsurl::new_jsurl(&str);
    if (!base) {
      JS_ReportErrorUTF8(cx, "URL constructor: %s is not a valid URL.", (char *)str.data);
      return nullptr;
    }
  }

  return create(cx, self, url_val, base);
}

bool URL::init_class(JSContext *cx, JS::HandleObject global) {
  return URL::init_class_impl(cx, global);
}

} // namespace builtins
