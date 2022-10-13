#include "config-store.h"
#include "host_call.h"

namespace builtins {

ConfigStoreHandle ConfigStore::config_store_handle(JSObject *obj) {
  JS::Value val = JS::GetReservedSlot(obj, ConfigStore::Slots::Handle);
  return ConfigStoreHandle{static_cast<uint32_t>(val.toInt32())};
}

bool ConfigStore::get(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(1)

  size_t name_len;
  JS::UniqueChars name = encode(cx, args[0], &name_len);

  OwnedHostCallBuffer buffer;
  size_t nwritten = 0;
  auto status = convert_to_fastly_status(
      xqd_config_store_get(ConfigStore::config_store_handle(self), name.get(), name_len,
                           buffer.get(), CONFIG_STORE_ENTRY_MAX_LEN, &nwritten));
  // FastlyStatus::none indicates the key wasn't found, so we return null.
  if (status == FastlyStatus::None) {
    args.rval().setNull();
    return true;
  }

  // Ensure that we throw an exception for all unexpected host errors.
  if (!HANDLE_RESULT(cx, status))
    return false;

  JS::RootedString text(cx, JS_NewStringCopyUTF8N(cx, JS::UTF8Chars(buffer.get(), nwritten)));
  if (!text)
    return false;

  args.rval().setString(text);
  return true;
}

const JSFunctionSpec ConfigStore::methods[] = {JS_FN("get", get, 1, JSPROP_ENUMERATE), JS_FS_END};

const JSPropertySpec ConfigStore::properties[] = {JS_PS_END};

bool ConfigStore::constructor(JSContext *cx, unsigned argc, JS::Value *vp) {
  REQUEST_HANDLER_ONLY("The ConfigStore builtin");
  CTOR_HEADER("ConfigStore", 1);

  size_t name_len;
  JS::UniqueChars name = encode(cx, args[0], &name_len);
  JS::RootedObject config_store(cx, JS_NewObjectForConstructor(cx, &class_, args));
  ConfigStoreHandle dict_handle = {INVALID_HANDLE};
  if (!HANDLE_RESULT(cx, xqd_config_store_open(name.get(), name_len, &dict_handle)))
    return false;

  JS::SetReservedSlot(config_store, ConfigStore::Slots::Handle,
                      JS::Int32Value((int)dict_handle.handle));
  if (!config_store)
    return false;
  args.rval().setObject(*config_store);
  return true;
}

bool ConfigStore::init_class(JSContext *cx, JS::HandleObject global) {
  return BuiltinImpl<ConfigStore>::init_class_impl(cx, global);
}

} // namespace builtins
