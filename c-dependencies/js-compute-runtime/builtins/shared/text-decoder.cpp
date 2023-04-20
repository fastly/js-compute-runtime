#include "builtins/shared/text-decoder.h"
#include "builtin.h"
#include "js-compute-builtins.h"

namespace builtins {

bool TextDecoder::decode(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(1)

  // Default to empty string if no input is given.
  if (args[0].isUndefined()) {
    args.rval().set(JS_GetEmptyStringValue(cx));
    return true;
  }

  auto data = value_to_buffer(cx, args[0], "TextDecoder#decode: input");
  if (!data.has_value()) {
    return false;
  }

  auto fatal =
      JS::GetReservedSlot(self, static_cast<uint32_t>(TextDecoder::Slots::Fatal)).toBoolean();
  auto ignoreBOM =
      JS::GetReservedSlot(self, static_cast<uint32_t>(TextDecoder::Slots::IgnoreBOM)).toBoolean();

  JS::RootedString str(cx);
  if (ignoreBOM) {
    auto chars = JS::UTF8Chars((char *)data->data(), data->size());
    size_t length;
    auto lossyUTF8Chars = JS::LossyUTF8CharsToNewTwoByteCharsZ(cx, chars, &length, js::MallocArena);
    JS::UniqueTwoByteChars lossyChars(static_cast<char16_t *>(JS_malloc(cx, length + 1)));
    if (!lossyChars) {
      return false;
    }
    memcpy(lossyChars.get(), lossyUTF8Chars.get(), length);

    str.set(JS_NewUCStringDontDeflate(cx, std::move(lossyChars), length));

  } else {
    str.set(JS_NewStringCopyUTF8N(cx, JS::UTF8Chars((char *)data->data(), data->size())));
  }
  if (!str) {
    if (fatal) {
      JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_TEXT_DECODER_DECODING_FAILED);
    }
    return false;
  }

  args.rval().setString(str);
  return true;
}

bool TextDecoder::encoding_get(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0)

  JS::RootedString str(cx, JS_NewStringCopyN(cx, "utf-8", 5));
  if (!str)
    return false;

  args.rval().setString(str);
  return true;
}

const JSFunctionSpec TextDecoder::static_methods[] = {
    JS_FS_END,
};

const JSPropertySpec TextDecoder::static_properties[] = {
    JS_PS_END,
};

const JSFunctionSpec TextDecoder::methods[] = {
    JS_FN("decode", decode, 1, JSPROP_ENUMERATE),
    JS_FS_END,
};

const JSPropertySpec TextDecoder::properties[] = {
    JS_PSG("encoding", encoding_get, JSPROP_ENUMERATE),
    JS_STRING_SYM_PS(toStringTag, "TextDecoder", JSPROP_READONLY),
    JS_PS_END,
};

// constructor(optional DOMString label = "utf-8", optional TextDecoderOptions options = {});
bool TextDecoder::constructor(JSContext *cx, unsigned argc, JS::Value *vp) {
  CTOR_HEADER("TextDecoder", 0);

  // 1. Let encoding be the result of getting an encoding from label.
  auto label_value = args.get(0);
  JS::RootedValue encoding(cx);
  if (!label_value.isUndefined()) {
    // https://encoding.spec.whatwg.org/#concept-encoding-get
    // To get an encoding from a string label, run these steps:
    size_t length;
    auto label_chars = encode(cx, label_value, &length);
    if (!label_chars) {
      return false;
    }
    std::string label(label_chars.get(), length);
    // 1. Remove any leading and trailing ASCII whitespace from label.
    auto ascii_whitespace = "\t\n\f\r ";
    label.erase(0, label.find_first_not_of(ascii_whitespace)); // left trim
    label.erase(label.find_last_not_of(ascii_whitespace) + 1); // right trim
    // 2. If label is an ASCII case-insensitive match for any of the labels listed in the table
    // below, then return the corresponding encoding; otherwise return failure. JS-Compute-Runtime:
    // We only support utf-8 right now so we only allow that label through.
    std::transform(label.begin(), label.end(), label.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    if (label == "unicode-1-1-utf-8" || label == "unicode11utf8" || label == "unicode20utf8" ||
        label == "utf-8" || label == "utf8" || label == "x-unicode20utf8") {
      encoding.setString(JS_NewStringCopyZ(cx, "UTF-8"));
    } else {
      JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_TEXT_DECODER_INVALID_ENCODING);
      return false;
    }
  } else {
    encoding.setString(JS_NewStringCopyZ(cx, "UTF-8"));
  }
  auto options_val = args.get(1);
  bool fatal = false;
  bool ignoreBOM = false;
  if (options_val.isObject()) {
    JS::RootedObject options(cx, &options_val.toObject());
    JS::RootedValue fatal_value(cx);
    if (!JS_GetProperty(cx, options, "fatal", &fatal_value)) {
      return false;
    }
    fatal = fatal_value.toBoolean();
    JS::RootedValue ignoreBOM_value(cx);
    if (!JS_GetProperty(cx, options, "ignoreBOM", &ignoreBOM_value)) {
      return false;
    }
    ignoreBOM = ignoreBOM_value.toBoolean();
  } else if (!options_val.isNullOrUndefined()) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr,
                              JSMSG_TEXT_DECODER_OPTIONS_NOT_DICTIONARY);
    return false;
  }
  JS::RootedObject self(cx, JS_NewObjectForConstructor(cx, &class_, args));
  JS::SetReservedSlot(self, static_cast<uint32_t>(TextDecoder::Slots::Encoding), encoding);
  JS::SetReservedSlot(self, static_cast<uint32_t>(TextDecoder::Slots::Fatal),
                      JS::BooleanValue(fatal));
  JS::SetReservedSlot(self, static_cast<uint32_t>(TextDecoder::Slots::IgnoreBOM),
                      JS::BooleanValue(ignoreBOM));

  args.rval().setObject(*self);
  return true;
}

bool TextDecoder::init_class(JSContext *cx, JS::HandleObject global) {
  return init_class_impl(cx, global);
}

} // namespace builtins
