#include "html-rewriter.h"
#include "../../../StarlingMonkey/builtins/web/streams/transform-stream-default-controller.h"
#include "../../../StarlingMonkey/builtins/web/streams/transform-stream.h"
#include "../../../StarlingMonkey/runtime/encode.h"
#include "../host-api/host_api_fastly.h"
#include <lol_html.h>

using builtins::web::streams::TransformStream;
using builtins::web::streams::TransformStreamDefaultController;

namespace fastly::html_rewriter {
const JSFunctionSpec Element::static_methods[] = {JS_FS_END};
const JSPropertySpec Element::static_properties[] = {JS_PS_END};
const JSFunctionSpec Element::methods[] = {
    JS_FN("before", before, 1, JSPROP_ENUMERATE),
    JS_FN("prepend", prepend, 1, JSPROP_ENUMERATE),
    JS_FN("append", append, 1, JSPROP_ENUMERATE),
    JS_FN("after", after, 1, JSPROP_ENUMERATE),
    JS_FN("getAttribute", getAttribute, 1, JSPROP_ENUMERATE),
    JS_FN("setAttribute", setAttribute, 2, JSPROP_ENUMERATE),
    JS_FN("removeAttribute", removeAttribute, 1, JSPROP_ENUMERATE),
    JS_FN("replaceChildren", replaceChildren, 1, JSPROP_ENUMERATE),
    JS_FN("replaceWith", replaceWith, 1, JSPROP_ENUMERATE),
    JS_FS_END};
const JSPropertySpec Element::properties[] = {
    JS_PSG("selector", Element::selector_get, JSPROP_ENUMERATE),
    JS_PSG("tag", Element::tag_get, JSPROP_ENUMERATE), JS_PS_END};

lol_html_element_t *raw_element(JSObject *self) {
  MOZ_ASSERT(Element::is_instance(self));
  return static_cast<lol_html_element_t *>(
      JS::GetReservedSlot(self, static_cast<uint32_t>(Element::Slots::Raw)).toPrivate());
}

bool Element::selector_get(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0)
  auto selector = JS::GetReservedSlot(self, static_cast<size_t>(Slots::Selector)).toString();
  args.rval().setString(selector);
  return true;
}

bool Element::tag_get(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0)
  auto element = raw_element(self);
  MOZ_ASSERT(element);
  auto str = lol_html_element_tag_name_get(element);
  args.rval().setString(JS_NewStringCopyN(cx, str.data, str.len));
  return true;
}

// We should escape only if the user supplies { escapeHTML: true } in the options object
static bool should_escape_html(JSContext *cx, JS::HandleValue options_arg) {
  if (!options_arg.isObject()) {
    return false;
  }

  JS::RootedObject options_obj(cx, &options_arg.toObject());
  JS::RootedValue escape_html_val(cx);
  if (!JS_GetProperty(cx, options_obj, "escapeHTML", &escape_html_val)) {
    return false;
  }
  return escape_html_val.isBoolean() && escape_html_val.toBoolean();
}

bool Element::before(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(1)

  JS::HandleValue content_arg = args.get(0);
  auto content = core::encode(cx, content_arg);
  if (!content) {
    return false;
  }

  auto element = raw_element(self);
  MOZ_ASSERT(element);
  return lol_html_element_before(element, content.begin(), content.size(),
                                 !should_escape_html(cx, args.get(1))) == 0;
}

bool Element::prepend(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(1)

  JS::HandleValue content_arg = args.get(0);
  auto content = core::encode(cx, content_arg);
  if (!content) {
    return false;
  }

  auto element = raw_element(self);
  MOZ_ASSERT(element);
  return lol_html_element_prepend(element, content.begin(), content.size(),
                                  !should_escape_html(cx, args.get(1))) == 0;
}

bool Element::append(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(1)

  JS::HandleValue content_arg = args.get(0);
  auto content = core::encode(cx, content_arg);
  if (!content) {
    return false;
  }

  auto element = raw_element(self);
  MOZ_ASSERT(element);
  return lol_html_element_append(element, content.begin(), content.size(),
                                 !should_escape_html(cx, args.get(1))) == 0;
}

bool Element::after(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(1)

  JS::HandleValue content_arg = args.get(0);
  auto content = core::encode(cx, content_arg);
  if (!content) {
    return false;
  }
  auto element = raw_element(self);
  MOZ_ASSERT(element);
  return lol_html_element_after(element, content.begin(), content.size(),
                                !should_escape_html(cx, args.get(1))) == 0;
}

bool Element::getAttribute(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(1)

  JS::HandleValue name_arg = args.get(0);
  auto name = core::encode(cx, name_arg);
  if (!name) {
    return false;
  }

  auto element = raw_element(self);
  MOZ_ASSERT(element);
  auto attr = lol_html_element_get_attribute(element, name.begin(), name.size());
  if (!attr.data) {
    args.rval().setNull();
  } else {
    args.rval().setString(JS_NewStringCopyN(cx, attr.data, attr.len));
  }

  return true;
}

bool Element::setAttribute(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(2)

  JS::HandleValue name_arg = args.get(0);
  auto name = core::encode(cx, name_arg);
  if (!name) {
    return false;
  }

  JS::HandleValue value_arg = args.get(1);
  auto value = core::encode(cx, value_arg);
  if (!value) {
    return false;
  }

  auto element = raw_element(self);
  MOZ_ASSERT(element);
  return lol_html_element_set_attribute(element, name.begin(), name.size(), value.begin(),
                                        value.size()) == 0;
}

bool Element::removeAttribute(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(1)

  JS::HandleValue name_arg = args.get(0);
  auto name = core::encode(cx, name_arg);
  if (!name) {
    return false;
  }

  auto element = raw_element(self);
  MOZ_ASSERT(element);
  lol_html_element_remove_attribute(element, name.begin(), name.size());

  return true;
}

bool Element::replaceChildren(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(1)

  JS::HandleValue content_arg = args.get(0);
  auto content = core::encode(cx, content_arg);
  if (!content) {
    return false;
  }

  auto element = raw_element(self);
  MOZ_ASSERT(element);
  return lol_html_element_set_inner_content(element, content.begin(), content.size(),
                                            !should_escape_html(cx, args.get(1))) == 0;
}

bool Element::replaceWith(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(1)

  JS::HandleValue content_arg = args.get(0);
  auto content = core::encode(cx, content_arg);
  if (!content) {
    return false;
  }

  auto element = raw_element(self);
  MOZ_ASSERT(element);
  return lol_html_element_replace(element, content.begin(), content.size(),
                                  !should_escape_html(cx, args.get(1))) == 0;
}

static JSObject *create_element(JSContext *cx, lol_html_element_t *element,
                                JS::HandleString selector) {
  JS::RootedObject obj(cx, JS_NewObjectWithGivenProto(cx, &Element::class_, Element::proto_obj));
  if (!obj) {
    return nullptr;
  }
  JS::SetReservedSlot(obj, static_cast<uint32_t>(Element::Slots::Raw), JS::PrivateValue(element));
  JS::SetReservedSlot(obj, static_cast<uint32_t>(Element::Slots::Selector),
                      JS::StringValue(selector));
  return obj;
}

const JSFunctionSpec HTMLRewritingStream::static_methods[] = {JS_FS_END};
const JSPropertySpec HTMLRewritingStream::static_properties[] = {JS_PS_END};
const JSFunctionSpec HTMLRewritingStream::methods[] = {
    JS_FN("onElement", onElement, 2, JSPROP_ENUMERATE), JS_FS_END};
const JSPropertySpec HTMLRewritingStream::properties[] = {
    JS_PSG("readable", HTMLRewritingStream::readable_get, JSPROP_ENUMERATE),
    JS_PSG("writable", HTMLRewritingStream::writable_get, JSPROP_ENUMERATE), JS_PS_END};

static lol_html_rewriter_builder_t *builder(JSObject *self) {
  MOZ_ASSERT(HTMLRewritingStream::is_instance(self));
  return static_cast<lol_html_rewriter_builder_t *>(
      JS::GetReservedSlot(self, static_cast<uint32_t>(HTMLRewritingStream::Slots::RawBuilder))
          .toPrivate());
}

static void set_builder(JSObject *self, lol_html_rewriter_builder_t *builder) {
  MOZ_ASSERT(HTMLRewritingStream::is_instance(self));
  JS::SetReservedSlot(self, static_cast<uint32_t>(HTMLRewritingStream::Slots::RawBuilder),
                      JS::PrivateValue(builder));
}

static JSObject *transform(JSObject *self) {
  MOZ_ASSERT(HTMLRewritingStream::is_instance(self));
  return &JS::GetReservedSlot(self, HTMLRewritingStream::Slots::Transform).toObject();
}

static lol_html_rewriter_t *raw_rewriter(JSObject *self) {
  MOZ_ASSERT(HTMLRewritingStream::is_instance(self));
  return static_cast<lol_html_rewriter_t *>(
      JS::GetReservedSlot(self, static_cast<uint32_t>(HTMLRewritingStream::Slots::RawRewriter))
          .toPrivate());
}
static void set_raw_rewriter(JSObject *self, lol_html_rewriter_t *rewriter) {
  MOZ_ASSERT(HTMLRewritingStream::is_instance(self));
  JS::SetReservedSlot(self, HTMLRewritingStream::Slots::RawRewriter, JS::PrivateValue(rewriter));
}

// Data needed to call an element handler from lol_html
// This also manages the lifetime of the lol-html selector
// There should be exactly one of these per element handler registered on the rewriter
// They should all be deleted when the HTMLRewritingStream is finalized
class ElementHandlerData {
public:
  ElementHandlerData(JSContext *cx, JSObject *handler, JSString *js_selector,
                     lol_html_selector_t *raw_selector)
      : cx_(cx), handler_(handler), js_selector_(js_selector), raw_selector_(raw_selector) {}

  ~ElementHandlerData() { lol_html_selector_free(raw_selector_); }

  JSContext *cx() const { return cx_; }
  JSObject *handler() const { return handler_; }
  JSString *selector() const { return js_selector_; }

private:
  JSContext *cx_;
  JS::Heap<JSObject *> handler_;
  JS::Heap<JSString *> js_selector_;
  lol_html_selector_t *raw_selector_;
};

// Called by lol_html when an element matching a registered selector is found
static lol_html_rewriter_directive_t handle_element(lol_html_element_t *element, void *user_data) {
  auto *data = static_cast<ElementHandlerData *>(user_data);
  JS::RootedString selector(data->cx(), data->selector());
  JS::RootedObject jsElement(data->cx(), create_element(data->cx(), element, selector));
  if (!jsElement) {
    return LOL_HTML_STOP;
  }
  JS::RootedValue jsElementVal(data->cx(), JS::ObjectValue(*jsElement));
  JS::RootedValue handlerVal(data->cx(), JS::ObjectValue(*data->handler()));
  JS::HandleValueArray arg(jsElementVal);
  JS::RootedValue rval(data->cx());
  if (!JS_CallFunctionValue(data->cx(), nullptr, handlerVal, arg, &rval)) {
    return LOL_HTML_STOP;
  }
  return LOL_HTML_CONTINUE;
}

bool HTMLRewritingStream::onElement(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(2)

  if (!builder(self)) {
    JS_ReportErrorASCII(cx, "HTMLRewriter: cannot add handlers after the rewriter has been used");
    return false;
  }

  JS::HandleValue selector_arg = args.get(0);
  auto selector_str = core::encode(cx, selector_arg);
  if (!selector_str) {
    return false;
  }

  JS::HandleValue handler = args.get(1);
  if (!handler.isObject() || !JS_ObjectIsFunction(&handler.toObject())) {
    JS_ReportErrorASCII(cx, "HTMLRewriter: element handler must be a function");
    return false;
  }

  auto raw_selector = lol_html_selector_parse(selector_str.begin(), selector_str.size());
  if (!raw_selector) {
    auto error = lol_html_take_last_error();
    if (error.data) {
      // Error may not be null-terminated
      std::string msg(error.data, error.len);
      JS_ReportErrorASCII(cx, "HTMLRewriter: invalid selector - %s", msg.c_str());
    } else {
      JS_ReportErrorASCII(cx, "HTMLRewriter: invalid selector");
    }
    return false;
  }
  // Create a unique_ptr so we don't leak if we error out below
  auto handler_data = std::make_unique<ElementHandlerData>(cx, &handler.toObject(),
                                                           selector_arg.toString(), raw_selector);

  if (lol_html_rewriter_builder_add_element_content_handlers(
          builder(self), raw_selector, handle_element, handler_data.get(), nullptr, nullptr,
          nullptr, nullptr) != 0) {
    return false;
  }

  // This slot holds all element handlers so we can free them when the stream is finalized.
  // This will also free the lol-html selectors.
  auto element_handlers = static_cast<std::vector<ElementHandlerData *> *>(
      JS::GetReservedSlot(self, static_cast<uint32_t>(Slots::ElementHandlers)).toPrivate());
  element_handlers->push_back(handler_data.release());

  args.rval().setObject(*self);
  return true;
}

struct OutputContextData {
  JSContext *cx;
  JSObject *self;
};

void HTMLRewritingStream::finalize(JS::GCContext *gcx, JSObject *self) {
  MOZ_ASSERT(is_instance(self));
  auto build = builder(self);
  if (build) {
    lol_html_rewriter_builder_free(static_cast<lol_html_rewriter_builder_t *>(build));
  }

  auto element_handlers = static_cast<std::vector<ElementHandlerData *> *>(
      JS::GetReservedSlot(self, static_cast<uint32_t>(Slots::ElementHandlers)).toPrivate());
  if (element_handlers) {
    for (auto handler : *element_handlers) {
      delete handler;
    }
    delete element_handlers;
  }

  auto output_context = static_cast<OutputContextData *>(
      JS::GetReservedSlot(self, HTMLRewritingStream::Slots::OutputContext).toPrivate());
  if (output_context) {
    delete output_context;
  }

  auto rewriter = raw_rewriter(self);
  if (rewriter) {
    lol_html_rewriter_free(static_cast<lol_html_rewriter_t *>(rewriter));
  }
}

static void output_callback(const char *chunk, size_t chunk_len, void *user_data) {
  auto *ctx = static_cast<OutputContextData *>(user_data);
  JSContext *cx = ctx->cx;
  JSObject *self = ctx->self;

  JS::RootedObject out_obj(cx, JS_NewUint8Array(cx, chunk_len));
  if (!out_obj) {
    return;
  }

  {
    bool is_shared;
    JS::AutoCheckCannotGC nogc(cx);
    uint8_t *out_buffer = JS_GetUint8ArrayData(out_obj, &is_shared, nogc);
    memcpy(out_buffer, chunk, chunk_len);
  }

  JS::RootedObject controller(cx, TransformStream::controller(transform(self)));
  JS::RootedValue out_chunk(cx, JS::ObjectValue(*out_obj));

  if (!TransformStreamDefaultController::Enqueue(cx, controller, out_chunk)) {
    return;
  }
}

// lol-html doesn't support registering handlers after any input has been processed,
// so we finalize the builder and create the rewriter on the first chunk submitted to transform
bool HTMLRewritingStream::finish_building(JSContext *cx, JS::HandleObject stream) {
  MOZ_ASSERT(is_instance(stream));

  // The output callback needs the JSContext and the stream object so it can enqueue into output
  // stream. We use a unique_ptr to ensure we don't leak if something fails.
  auto output_context = std::make_unique<OutputContextData>(cx, stream);
  // Same defaults as Rust
  lol_html_memory_settings_t memory_settings = {1024, std::numeric_limits<size_t>::max()};
  auto encoding_string_length = 5; // "utf-8"
  auto rewriter =
      lol_html_rewriter_build(builder(stream), "utf-8", encoding_string_length, memory_settings,
                              output_callback, output_context.get(), true);
  if (!rewriter) {
    return false;
  }

  JS_SetReservedSlot(stream, HTMLRewritingStream::Slots::OutputContext,
                     JS::PrivateValue(output_context.release()));
  set_raw_rewriter(stream, rewriter);

  // The builder is no longer needed after building the rewriter and can be safely freed
  lol_html_rewriter_builder_free(builder(stream));
  set_builder(stream, nullptr); // Ensure we don't try to free it again in finalize

  return true;
}

bool HTMLRewritingStream::transformAlgorithm(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER_WITH_NAME(1, "HTML rewriter transform algorithm")

  if (!raw_rewriter(self)) {
    HTMLRewritingStream::finish_building(cx, self);
  }

  auto rewriter = raw_rewriter(self);
  MOZ_ASSERT(rewriter);

  auto chunk = args.get(0);
  auto data = value_to_buffer(cx, chunk, "HTMLRewritingStream transform: chunks");
  if (!data.has_value()) {
    return false;
  }

  if (data->size() == 0) {
    return true;
  }

  lol_html_take_last_error(); // Clear any previous error
  // lol-html will call output_callback with the processed data
  lol_html_rewriter_write(rewriter, reinterpret_cast<const char *>(data->data()), data->size());
  auto err = lol_html_take_last_error();
  if (err.data) {
    // Error may not be null-terminated
    JS_ReportErrorASCII(cx, "Error processing HTML: %s", std::string(err.data, err.len).c_str());
    return false;
  }

  args.rval().setUndefined();
  return true;
}

bool HTMLRewritingStream::flushAlgorithm(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER_WITH_NAME(0, "HTML rewriter flush algorithm")

  // Just in case the stream is flushed immediately
  if (!raw_rewriter(self)) {
    HTMLRewritingStream::finish_building(cx, self);
  }

  auto rewriter = raw_rewriter(self);
  MOZ_ASSERT(rewriter);

  lol_html_take_last_error(); // Clear any previous error
  // lol-html will call output_callback with any remaining data
  lol_html_rewriter_end(rewriter);
  auto err = lol_html_take_last_error();
  if (err.data) {
    // Error may not be null-terminated
    JS_ReportErrorASCII(cx, "Error processing HTML: %s", std::string(err.data, err.len).c_str());
    return false;
  }

  args.rval().setUndefined();
  return true;
}

bool HTMLRewritingStream::readable_get(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER_WITH_NAME(0, "get readable")
  args.rval().setObject(*TransformStream::readable(transform(self)));
  return true;
}

bool HTMLRewritingStream::writable_get(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER_WITH_NAME(0, "get writable")
  args.rval().setObject(*TransformStream::writable(transform(self)));
  return true;
}

JS::PersistentRooted<JSObject *> transformAlgo;
JS::PersistentRooted<JSObject *> flushAlgo;

bool HTMLRewritingStream::constructor(JSContext *cx, unsigned argc, JS::Value *vp) {
  CTOR_HEADER("HTMLRewritingStream", 0)

  JS::RootedObject instance(cx, JS_NewObjectForConstructor(cx, &class_, args));
  if (!instance) {
    return false;
  }
  auto builder = lol_html_rewriter_builder_new();
  if (!builder) {
    return false;
  }
  set_builder(instance, builder);
  // We have no rewriter initially; it will be created on the first chunk processed
  set_raw_rewriter(instance, nullptr);
  JS::RootedValue stream_val(cx, JS::ObjectValue(*instance));
  JS::RootedObject transform(cx, TransformStream::create(cx, 1, nullptr, 0, nullptr, stream_val,
                                                         nullptr, transformAlgo, flushAlgo));
  if (!transform) {
    return false;
  }

  TransformStream::set_used_as_mixin(transform);
  JS::SetReservedSlot(instance, HTMLRewritingStream::Slots::Transform, JS::ObjectValue(*transform));

  JS::SetReservedSlot(instance, static_cast<uint32_t>(Slots::ElementHandlers),
                      JS::PrivateValue(new std::vector<ElementHandlerData *>()));

  args.rval().setObject(*instance);
  return true;
}

bool HTMLRewritingStream::init_class(JSContext *cx, JS::HandleObject global) {
  if (!init_class_impl(cx, global)) {
    return false;
  }

  JSFunction *transformFun =
      JS_NewFunction(cx, transformAlgorithm, 1, 0, "HTML Rewriter Transform");
  if (!transformFun)
    return false;
  transformAlgo.init(cx, JS_GetFunctionObject(transformFun));

  JSFunction *flushFun = JS_NewFunction(cx, flushAlgorithm, 1, 0, "HTML Rewriter Flush");
  if (!flushFun)
    return false;
  flushAlgo.init(cx, JS_GetFunctionObject(flushFun));

  return true;
}

bool install(api::Engine *engine) {
  if (!HTMLRewritingStream::init_class(engine->cx(), engine->global())) {
    return false;
  }

  if (!Element::init_class(engine->cx(), engine->global())) {
    return false;
  }

  RootedObject html_rewriter_obj(
      engine->cx(),
      JS_GetConstructor(engine->cx(), builtins::BuiltinImpl<HTMLRewritingStream>::proto_obj));
  RootedValue html_rewriter_val(engine->cx(), ObjectValue(*html_rewriter_obj));
  RootedObject html_rewriter_ns(engine->cx(), JS_NewObject(engine->cx(), nullptr));
  if (!JS_SetProperty(engine->cx(), html_rewriter_ns, "HTMLRewritingStream", html_rewriter_val)) {
    return false;
  }
  RootedValue html_rewriter_ns_val(engine->cx(), JS::ObjectValue(*html_rewriter_ns));
  if (!engine->define_builtin_module("fastly:html-rewriter", html_rewriter_ns_val)) {
    return false;
  }

  return true;
}
} // namespace fastly::html_rewriter