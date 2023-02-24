
#include "builtin.h"
#include "rust-url/rust-url.h"

namespace builtins {

class URLSearchParamsIterator : public BuiltinNoConstructor<URLSearchParamsIterator> {
public:
  static constexpr const char *class_name = "URLSearchParamsIterator";

  enum Slots { Params, Type, Index, Count };

  static bool next(JSContext *cx, unsigned argc, JS::Value *vp);

  static const JSFunctionSpec methods[];
  static const JSPropertySpec properties[];

  static const unsigned ctor_length = 0;

  static JSObject *create(JSContext *cx, JS::HandleObject params, uint8_t type);
  static bool init_class(JSContext *cx, JS::HandleObject global);
  static bool constructor(JSContext *cx, unsigned argc, JS::Value *vp);
};

} // namespace builtins

namespace URLSearchParams {
// TODO: these shouldn't be exposed
extern const JSClass class_;
extern JS::PersistentRooted<JSObject *> proto_obj;

bool is_instance(JSObject *obj);
bool is_instance(JS::Value val);

jsurl::SpecSlice serialize(JSContext *cx, JS::HandleObject self);
jsurl::JSUrlSearchParams *get_params(JSObject *self);

bool init_class(JSContext *cx, JS::HandleObject global);

JSObject *create(JSContext *cx, JS::HandleObject self, jsurl::JSUrl *url);
JSObject *create(JSContext *cx, JS::HandleObject self, JS::HandleValue params_val);
JSObject *create(JSContext *cx, JS::HandleObject self,
                 JS::HandleValue params_val = JS::UndefinedHandleValue);
} // namespace URLSearchParams

namespace URL {
// TODO: these shouldn't be exposed
extern const JSClass class_;
extern JS::PersistentRooted<JSObject *> proto_obj;

bool is_instance(JSObject *obj);
bool is_instance(JS::Value val);

bool hash(JSContext *cx, JS::HandleObject self, JS::MutableHandleValue rval);
bool host(JSContext *cx, JS::HandleObject self, JS::MutableHandleValue rval);
bool hostname(JSContext *cx, JS::HandleObject self, JS::MutableHandleValue rval);
bool href(JSContext *cx, JS::HandleObject self, JS::MutableHandleValue rval);
bool password(JSContext *cx, JS::HandleObject self, JS::MutableHandleValue rval);
bool pathname(JSContext *cx, JS::HandleObject self, JS::MutableHandleValue rval);
bool port(JSContext *cx, JS::HandleObject self, JS::MutableHandleValue rval);
bool protocol(JSContext *cx, JS::HandleObject self, JS::MutableHandleValue rval);
bool search(JSContext *cx, JS::HandleObject self, JS::MutableHandleValue rval);
bool username(JSContext *cx, JS::HandleObject self, JS::MutableHandleValue rval);

jsurl::SpecString origin(JSContext *cx, JS::HandleObject self);
bool origin(JSContext *cx, JS::HandleObject self, JS::MutableHandleValue rval);

JSObject *create(JSContext *cx, JS::HandleObject self, jsurl::SpecString url_str,
                 const jsurl::JSUrl *base = nullptr);

JSObject *create(JSContext *cx, JS::HandleObject self, JS::HandleValue url_val,
                 const jsurl::JSUrl *base = nullptr);

JSObject *create(JSContext *cx, JS::HandleObject self, JS::HandleValue url_val,
                 JS::HandleObject base_obj);

JSObject *create(JSContext *cx, JS::HandleObject self, JS::HandleValue url_val,
                 JS::HandleValue base_val);

bool init_class(JSContext *cx, JS::HandleObject global);
} // namespace URL
