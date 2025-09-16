// An HTML rewriter implementation based on TransformStreams.

#ifndef FASTLY_HTML_REWRITER_H
#define FASTLY_HTML_REWRITER_H

#include "../host-api/host_api_fastly.h"
#include "builtin.h"
#include "extension-api.h"

namespace fastly::html_rewriter {
class Element : public builtins::BuiltinNoConstructor<Element> {
private:
  static bool selector_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool tag_get(JSContext *cx, unsigned argc, JS::Value *vp);

public:
  static constexpr const char *class_name = "Element";
  static const int ctor_length = 0;
  enum Slots { Raw, Selector, Count };
  static const JSFunctionSpec static_methods[];
  static const JSPropertySpec static_properties[];
  static const JSFunctionSpec methods[];
  static const JSPropertySpec properties[];

  static bool before(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool prepend(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool append(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool after(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool getAttribute(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool setAttribute(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool removeAttribute(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool replaceChildren(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool replaceWith(JSContext *cx, unsigned argc, JS::Value *vp);
};

class HTMLRewritingStream : public builtins::BuiltinImpl<HTMLRewritingStream> {
private:
  static bool transformAlgorithm(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool flushAlgorithm(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool readable_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool writable_get(JSContext *cx, unsigned argc, JS::Value *vp);

public:
  static constexpr const char *class_name = "HTMLRewritingStream";
  static const int ctor_length = 0;
  enum Slots { RawBuilder, RawRewriter, Buffer, OutputContext, ElementHandlers, Transform, Count };
  static const JSFunctionSpec static_methods[];
  static const JSPropertySpec static_properties[];
  static const JSFunctionSpec methods[];
  static const JSPropertySpec properties[];

  static bool init_class(JSContext *cx, JS::HandleObject global);
  static bool constructor(JSContext *cx, unsigned argc, JS::Value *vp);

  static bool onElement(JSContext *cx, unsigned argc, JS::Value *vp);

  static bool finish_building(JSContext *cx, JS::HandleObject stream);
  static void finalize(JS::GCContext *gcx, JSObject *self);
};
} // namespace fastly::html_rewriter

#endif