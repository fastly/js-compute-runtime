#include "crypto-key.h"
#include "js-compute-builtins.h"

namespace builtins {

CryptoKeyUsages::CryptoKeyUsages(uint8_t mask) { this->mask = mask; };
CryptoKeyUsages::CryptoKeyUsages(bool encrypt, bool decrypt, bool sign, bool verify,
                                 bool derive_key, bool derive_bits, bool wrap_key,
                                 bool unwrap_key) {
  this->mask = 0;
  if (encrypt) {
    this->mask |= encrypt_flag;
  }
  if (decrypt) {
    this->mask |= decrypt_flag;
  }
  if (sign) {
    this->mask |= sign_flag;
  }
  if (verify) {
    this->mask |= verify_flag;
  }
  if (derive_key) {
    this->mask |= derive_key_flag;
  }
  if (derive_bits) {
    this->mask |= derive_bits_flag;
  }
  if (wrap_key) {
    this->mask |= wrap_key_flag;
  }
  if (unwrap_key) {
    this->mask |= unwrap_key_flag;
  }
};

bool CryptoKey::algorithm_get(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0);

  // TODO: Should we move this into the METHOD_HEADER macro?
  // CryptoKey.prototype passes the receiver check in the above macro but is not actually an
  // instance of CryptoKey. We check if `self` is `CryptoKey.prototype` and if it is, we throw a JS
  // Error.
  if (self == proto_obj.get()) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessageBuiltin, nullptr, JSMSG_INCOMPATIBLE_INSTANCE,
                              __func__, CryptoKey::class_.name);
    return false;
  }

  auto algorithm = &JS::GetReservedSlot(self, Slots::Algorithm).toObject();
  JS::RootedObject result(cx, algorithm);
  if (!result) {
    return false;
  }
  args.rval().setObject(*result);

  return true;
}

bool CryptoKey::extractable_get(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0);

  // TODO: Should we move this into the METHOD_HEADER macro?
  // CryptoKey.prototype passes the receiver check in the above macro but is not actually an
  // instance of CryptoKey. We check if `self` is `CryptoKey.prototype` and if it is, we throw a JS
  // Error.
  if (self == proto_obj.get()) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_INVALID_INTERFACE,
                              "extractable get", "CryptoKey");
    return false;
  }

  auto extractable = JS::GetReservedSlot(self, Slots::Extractable).toBoolean();
  args.rval().setBoolean(extractable);

  return true;
}

bool CryptoKey::type_get(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0)

  // TODO: Should we move this into the METHOD_HEADER macro?
  // CryptoKey.prototype passes the receiver check in the above macro but is not actually an
  // instance of CryptoKey. We check if `self` is `CryptoKey.prototype` and if it is, we throw a JS
  // Error.
  if (self == proto_obj.get()) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_INVALID_INTERFACE, "type get",
                              "CryptoKey");
    return false;
  }
  auto type = static_cast<CryptoKeyType>(JS::GetReservedSlot(self, Slots::Type).toInt32());

  // We store the type internally as a CryptoKeyType variant and need to
  // convert it into it's JSString representation.
  switch (type) {
  case CryptoKeyType::Private: {
    auto str = JS_AtomizeString(cx, "private");
    if (!str) {
      return false;
    }
    args.rval().setString(str);
    return true;
  }
  case CryptoKeyType::Public: {
    auto str = JS_AtomizeString(cx, "public");
    if (!str) {
      return false;
    }
    args.rval().setString(str);
    return true;
  }
  case CryptoKeyType::Secret: {
    auto str = JS_AtomizeString(cx, "secret");
    if (!str) {
      return false;
    }
    args.rval().setString(str);
    return true;
  }
  default: {
    MOZ_ASSERT_UNREACHABLE("Unknown `CryptoKeyType` value");
    return false;
  }
  };
}

bool CryptoKey::usages_get(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0);

  // TODO: Should we move this into the METHOD_HEADER macro?
  // CryptoKey.prototype passes the receiver check in the above macro but is not actually an
  // instance of CryptoKey. We check if `self` is `CryptoKey.prototype` and if it is, we throw a JS
  // Error.
  if (self == proto_obj.get()) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_INVALID_INTERFACE, "usages get",
                              "CryptoKey");
    return false;
  }

  // If the JS Array has already been created previously, return it.
  auto cached_usage = JS::GetReservedSlot(self, Slots::UsagesArray);
  if (cached_usage.isObject()) {
    args.rval().setObject(cached_usage.toObject());
    return true;
  }
  // Else, grab the CryptoKeyUsageBitmap value from Slots::Usages and convert
  // it into a JS Array and store the result in Slots::UsagesArray.
  auto usage = CryptoKeyUsages(JS::GetReservedSlot(self, Slots::Usages).toInt32());
  // The result is ordered alphabetically.
  JS::RootedValueVector result(cx);
  JS::RootedString str(cx);
  auto append = [&](const char *name) -> bool {
    if (!(str = JS_AtomizeString(cx, name))) {
      return false;
    }
    if (!result.append(JS::StringValue(str))) {
      js::ReportOutOfMemory(cx);
      return false;
    }
    return true;
  };
  if (usage.canDecrypt()) {
    if (!append("decrypt")) {
      return false;
    }
  }
  if (usage.canDeriveBits()) {
    if (!append("deriveBits")) {
      return false;
    }
  }
  if (usage.canDeriveKey()) {
    if (!append("deriveKey")) {
      return false;
    }
  }
  if (usage.canEncrypt()) {
    if (!append("encrypt")) {
      return false;
    }
  }
  if (usage.canSign()) {
    if (!append("sign")) {
      return false;
    }
  }
  if (usage.canUnwrapKey()) {
    if (!append("unwrapKey")) {
      return false;
    }
  }
  if (usage.canVerify()) {
    if (!append("verify")) {
      return false;
    }
  }
  if (usage.canWrapKey()) {
    if (!append("wrapKey")) {
      return false;
    }
  }

  JS::Rooted<JSObject *> array(cx, JS::NewArrayObject(cx, result));
  if (!array) {
    return false;
  }
  cached_usage.setObject(*array);
  JS::SetReservedSlot(self, Slots::UsagesArray, cached_usage);

  args.rval().setObject(*array);

  return true;
}

const JSFunctionSpec CryptoKey::methods[] = {JS_FS_END};

const JSPropertySpec CryptoKey::properties[] = {
    JS_PSG("type", CryptoKey::type_get, JSPROP_ENUMERATE),
    JS_PSG("extractable", CryptoKey::extractable_get, JSPROP_ENUMERATE),
    JS_PSG("algorithm", CryptoKey::algorithm_get, JSPROP_ENUMERATE),
    JS_PSG("usages", CryptoKey::usages_get, JSPROP_ENUMERATE),
    JS_STRING_SYM_PS(toStringTag, "CryptoKey", JSPROP_READONLY),
    JS_PS_END};

// There is no directly exposed constructor in the CryptoKey interface
// https://w3c.github.io/webcrypto/#cryptokey-interface We throw a JS Error if the application
// attempts to call the CryptoKey constructor directly
bool CryptoKey::constructor(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_ILLEGAL_CTOR);
  return false;
}

bool CryptoKey::init_class(JSContext *cx, JS::HandleObject global) {
  return BuiltinImpl<CryptoKey>::init_class_impl(cx, global);
}

} // namespace builtins