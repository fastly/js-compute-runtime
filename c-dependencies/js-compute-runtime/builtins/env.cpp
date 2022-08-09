
#include "env.h"
namespace builtins {

bool Env::env_get(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS::CallArgs args = CallArgsFromVp(argc, vp);
  if (!args.requireAtLeast(cx, "fastly.env.get", 1))
    return false;

  size_t var_name_len;
  JS::UniqueChars var_name_chars = encode(cx, args[0], &var_name_len);
  if (!var_name_chars) {
    return false;
  }
  JS::RootedString env_var(cx, JS_NewStringCopyZ(cx, getenv(var_name_chars.get())));
  if (!env_var)
    return false;

  args.rval().setString(env_var);
  return true;
}

const JSFunctionSpec Env::methods[] = {JS_FN("get", env_get, 1, JSPROP_ENUMERATE), JS_FS_END};

const JSPropertySpec Env::properties[] = {JS_PS_END};

JSObject *Env::create(JSContext *cx) {
  JS::RootedObject env(cx, JS_NewPlainObject(cx));
  if (!env || !JS_DefineFunctions(cx, env, methods))
    return nullptr;
  return env;
}

} // namespace builtins
