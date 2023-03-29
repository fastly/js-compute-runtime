#include "crypto-key.h"
#include "crypto-algorithm.h"
#include "js-compute-builtins.h"
#include "openssl/rsa.h"

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

CryptoKeyUsages CryptoKeyUsages::from(std::vector<std::string> key_usages) {
  uint8_t mask = 0;
  for (auto usage : key_usages) {
    if (usage == "encrypt") {
      mask |= encrypt_flag;
    } else if (usage == "decrypt") {
      mask |= decrypt_flag;
    } else if (usage == "sign") {
      mask |= sign_flag;
    } else if (usage == "verify") {
      mask |= verify_flag;
    } else if (usage == "deriveKey") {
      mask |= derive_key_flag;
    } else if (usage == "deriveBits") {
      mask |= derive_bits_flag;
    } else if (usage == "wrapKey") {
      mask |= wrap_key_flag;
    } else if (usage == "unwrapKey") {
      mask |= unwrap_key_flag;
    }
  }
  return CryptoKeyUsages(mask);
}

JS::Result<CryptoKeyUsages> CryptoKeyUsages::from(JSContext *cx, JS::HandleValue key_usages,
                                                  std::string_view error_message) {
  bool key_usages_is_array;
  if (!JS::IsArrayObject(cx, key_usages, &key_usages_is_array)) {
    return JS::Result<CryptoKeyUsages>(JS::Error());
  }

  if (!key_usages_is_array) {
    // TODO: This should check if the JS::HandleValue is iterable and if so, should convert it into
    // a JS Array
    JS_ReportErrorASCII(cx, "The provided value cannot be converted to a sequence");
    return JS::Result<CryptoKeyUsages>(JS::Error());
  }

  uint32_t key_usages_length;
  JS::RootedObject array(cx, &key_usages.toObject());
  if (!array) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_SUBTLE_CRYPTO_ERROR,
                              error_message.data());
    return JS::Result<CryptoKeyUsages>(JS::Error());
  }
  if (!JS::GetArrayLength(cx, array, &key_usages_length)) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_SUBTLE_CRYPTO_ERROR,
                              error_message.data());
    return JS::Result<CryptoKeyUsages>(JS::Error());
  }
  uint8_t mask = 0;
  for (uint32_t index = 0; index < key_usages_length; index++) {
    JS::RootedValue val(cx);
    if (!JS_GetElement(cx, array, index, &val)) {
      JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_SUBTLE_CRYPTO_ERROR,
                                error_message.data());
      return JS::Result<CryptoKeyUsages>(JS::Error());
    }

    size_t val_len;
    JS::UniqueChars utf8chars = encode(cx, val, &val_len);
    if (!utf8chars) {
      return JS::Result<CryptoKeyUsages>(JS::Error());
    }

    std::string_view usage(utf8chars.get(), val_len);

    if (usage == "encrypt") {
      mask |= encrypt_flag;
    } else if (usage == "decrypt") {
      mask |= decrypt_flag;
    } else if (usage == "sign") {
      mask |= sign_flag;
    } else if (usage == "verify") {
      mask |= verify_flag;
    } else if (usage == "deriveKey") {
      mask |= derive_key_flag;
    } else if (usage == "deriveBits") {
      mask |= derive_bits_flag;
    } else if (usage == "wrapKey") {
      mask |= wrap_key_flag;
    } else if (usage == "unwrapKey") {
      mask |= unwrap_key_flag;
    } else {
      JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_SUBTLE_CRYPTO_ERROR,
                                error_message.data());
      return JS::Result<CryptoKeyUsages>(JS::Error());
    }
  }
  return CryptoKeyUsages(mask);
}

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
  // Else, grab the CryptoKeyUsages value from Slots::Usages and convert
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

namespace {
BIGNUM *convertToBigNumber(std::string_view bytes) {
  return BN_bin2bn(reinterpret_cast<const unsigned char *>(bytes.data()), bytes.length(), nullptr);
}

int getBigNumberLength(BIGNUM *a) { return BN_num_bytes(a) * 8; }
} // namespace

JSObject *CryptoKey::createRSA(JSContext *cx, CryptoAlgorithmRSASSA_PKCS1_v1_5_Import *algorithm,
                               std::unique_ptr<CryptoKeyRSAComponents> keyData, bool extractable,
                               CryptoKeyUsages usages) {
  MOZ_ASSERT(cx);
  MOZ_ASSERT(algorithm);
  CryptoKeyType keyType;
  switch (keyData->type) {
  case CryptoKeyRSAComponents::Type::Public: {
    keyType = CryptoKeyType::Public;
    break;
  }
  case CryptoKeyRSAComponents::Type::Private: {
    keyType = CryptoKeyType::Private;
    break;
  }
  default: {
    MOZ_ASSERT_UNREACHABLE("Unknown `CryptoKeyRSAComponents::Type` value");
    return nullptr;
  }
  }

  // When creating a private key, we require the p and q prime information.
  if (keyType == CryptoKeyType::Private && !keyData->hasAdditionalPrivateKeyParameters) {
    return nullptr;
  }

  // But we don't currently support creating keys with any additional prime information.
  if (!keyData->otherPrimeInfos.empty()) {
    return nullptr;
  }

  // For both public and private keys, we need the public modulus and exponent.
  if (keyData->modulus.empty() || keyData->exponent.empty()) {
    return nullptr;
  }

  // For private keys, we require the private exponent, as well as p and q prime information.
  if (keyType == CryptoKeyType::Private) {
    if (keyData->privateExponent.empty() || keyData->firstPrimeInfo->primeFactor.empty() ||
        keyData->secondPrimeInfo->primeFactor.empty()) {
      return nullptr;
    }
  }

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
  auto rsa = RSA_new();
#pragma clang diagnostic pop
  if (!rsa) {
    return nullptr;
  }

  auto n = convertToBigNumber(keyData->modulus);
  auto e = convertToBigNumber(keyData->exponent);
  if (!n || !e) {
    return nullptr;
  }

// Calling with d null is fine as long as n and e are not null
// Ownership of n and e transferred to OpenSSL
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
  if (!RSA_set0_key(rsa, n, e, nullptr)) {
    return nullptr;
  }
#pragma clang diagnostic pop

  if (keyType == CryptoKeyType::Private) {
    auto d = convertToBigNumber(keyData->privateExponent);
    if (!d) {
      return nullptr;
    }

// Calling with n and e null is fine as long as they were set prior
// Ownership of d transferred to OpenSSL
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    if (!RSA_set0_key(rsa, nullptr, nullptr, d)) {
      return nullptr;
    }
#pragma clang diagnostic pop

    auto p = convertToBigNumber(keyData->firstPrimeInfo->primeFactor);
    auto q = convertToBigNumber(keyData->secondPrimeInfo->primeFactor);
    if (!p || !q) {
      return nullptr;
    }

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    // Ownership of p and q transferred to OpenSSL
    if (!RSA_set0_factors(rsa, p, q)) {
      return nullptr;
    }
#pragma clang diagnostic pop

    // We set dmp1, dmpq1, and iqmp member of the RSA struct if the keyData has corresponding data.

    // dmp1 -- d mod (p - 1)
    auto dmp1 = (!keyData->firstPrimeInfo->factorCRTExponent.empty())
                    ? convertToBigNumber(keyData->firstPrimeInfo->factorCRTExponent)
                    : nullptr;
    // dmq1 -- d mod (q - 1)
    auto dmq1 = (!keyData->secondPrimeInfo->factorCRTExponent.empty())
                    ? convertToBigNumber(keyData->secondPrimeInfo->factorCRTExponent)
                    : nullptr;
    // iqmp -- q^(-1) mod p
    auto iqmp = (!keyData->secondPrimeInfo->factorCRTCoefficient.empty())
                    ? convertToBigNumber(keyData->secondPrimeInfo->factorCRTCoefficient)
                    : nullptr;

// Ownership of dmp1, dmq1 and iqmp transferred to OpenSSL
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    if (!RSA_set0_crt_params(rsa, dmp1, dmq1, iqmp)) {
#pragma clang diagnostic pop
      return nullptr;
    }
  }

  auto pkey = EVP_PKEY_new();
  if (!pkey) {
    return nullptr;
  }

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
  if (EVP_PKEY_set1_RSA(pkey, rsa) != 1) {
#pragma clang diagnostic pop
    return nullptr;
  }

  JS::RootedObject instance(
      cx, JS_NewObjectWithGivenProto(cx, &CryptoKey::class_, CryptoKey::proto_obj));
  if (!instance) {
    return nullptr;
  }

  JS::RootedObject alg(cx, algorithm->toObject(cx));
  if (!alg) {
    return nullptr;
  }
  // Set the modulusLength attribute of algorithm to the length, in bits, of the RSA public modulus.
  JS::RootedValue modulusLength(cx, JS::NumberValue(getBigNumberLength(n)));
  if (!JS_SetProperty(cx, alg, "modulusLength", modulusLength)) {
    return nullptr;
  }

  uint8_t *p = reinterpret_cast<uint8_t *>(calloc(keyData->exponent.size(), sizeof(uint8_t)));
  auto exp = keyData->exponent;
  std::copy(exp.begin(), exp.end(), p);

  JS::RootedObject buffer(cx, JS::NewArrayBufferWithContents(cx, keyData->exponent.size(), p));
  if (!buffer) {
    // We can be here if the array buffer was too large -- if that was the case then a
    // JSMSG_BAD_ARRAY_LENGTH will have been created. No other failure scenarios in this path will
    // create a JS exception and so we need to create one.
    if (!JS_IsExceptionPending(cx)) {
      // TODO Rename error to InternalError
      JS_ReportErrorLatin1(cx, "InternalError");
    }
    JS_free(cx, p);
    return nullptr;
  }

  // Set the publicExponent attribute of algorithm to the BigInteger representation of the RSA
  // public exponent.
  JS::RootedObject byte_array(cx,
                              JS_NewUint8ArrayWithBuffer(cx, buffer, 0, keyData->exponent.size()));
  JS::RootedValue publicExponent(cx, JS::ObjectValue(*byte_array));
  if (!JS_SetProperty(cx, alg, "publicExponent", publicExponent)) {
    return nullptr;
  }

  JS::SetReservedSlot(instance, Slots::Algorithm, JS::ObjectValue(*alg));
  JS::SetReservedSlot(instance, Slots::Type, JS::Int32Value(static_cast<uint8_t>(keyType)));
  JS::SetReservedSlot(instance, Slots::Extractable, JS::BooleanValue(extractable));
  JS::SetReservedSlot(instance, Slots::Usages, JS::Int32Value(usages.toInt()));
  JS::SetReservedSlot(instance, Slots::Key, JS::PrivateValue(pkey));
  return instance;
}

} // namespace builtins