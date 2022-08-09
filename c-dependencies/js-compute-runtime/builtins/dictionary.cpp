#include "dictionary.h"
#include "host_call.h"

namespace builtins {

DictionaryHandle Dictionary::dictionary_handle(JSObject *obj) {
  JS::Value val = JS::GetReservedSlot(obj, Dictionary::Slots::Handle);
  return DictionaryHandle{static_cast<uint32_t>(val.toInt32())};
}

bool Dictionary::get(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(1)

  size_t name_len;
  JS::UniqueChars name = encode(cx, args[0], &name_len);

  OwnedHostCallBuffer buffer;
  size_t nwritten = 0;
  int status = xqd_dictionary_get(Dictionary::dictionary_handle(self), name.get(), name_len,
                                  buffer.get(), DICTIONARY_ENTRY_MAX_LEN, &nwritten);
  // Status code 10 indicates the key wasn't found, so we return null.
  if (status == 10) {
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

const JSFunctionSpec Dictionary::methods[] = {JS_FN("get", get, 1, JSPROP_ENUMERATE), JS_FS_END};

const JSPropertySpec Dictionary::properties[] = {JS_PS_END};

bool Dictionary::constructor(JSContext *cx, unsigned argc, JS::Value *vp) {
  REQUEST_HANDLER_ONLY("The Dictionary builtin");
  CTOR_HEADER("Dictionary", 1);

  size_t name_len;
  JS::UniqueChars name = encode(cx, args[0], &name_len);
  JS::RootedObject dictionary(cx, JS_NewObjectForConstructor(cx, &class_, args));
  DictionaryHandle dict_handle = {INVALID_HANDLE};
  if (!HANDLE_RESULT(cx, xqd_dictionary_open(name.get(), name_len, &dict_handle)))
    return false;

  JS::SetReservedSlot(dictionary, Dictionary::Slots::Handle,
                      JS::Int32Value((int)dict_handle.handle));
  if (!dictionary)
    return false;
  args.rval().setObject(*dictionary);
  return true;
}

bool Dictionary::init_class(JSContext *cx, JS::HandleObject global) {
  return BuiltinImpl<Dictionary>::init_class_impl(cx, global);
}

} // namespace builtins
