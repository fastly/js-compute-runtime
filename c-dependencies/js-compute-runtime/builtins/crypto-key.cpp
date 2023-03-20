#include <iostream>
#include <span>
#include <openssl/bn.h>
#include <openssl/err.h>
#include "openssl/evp.h"
#include <openssl/rsa.h>
#include <openssl/rand.h>
#include "openssl/sha.h"

#include "js/ArrayBuffer.h"
// TODO: remove these once the warnings are fixed
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-offsetof"
#include "js/Conversions.h"
#include "js/experimental/TypedData.h"
#include "mozilla/Result.h"
#pragma clang diagnostic pop

#include "crypto-key-rsa-components.h"
#include "crypto-key.h"
#include "js-compute-builtins.h"

#include "js/Array.h"

BIGNUM *convertToBigNumber(std::string bytes) {
  return BN_bin2bn(reinterpret_cast<const unsigned char *>(bytes.data()), bytes.length(), nullptr);
}
BIGNUM *convertToBigNumber(std::span<uint8_t> bytes) {
  return BN_bin2bn(reinterpret_cast<const unsigned char *>(bytes.data()), bytes.size(), nullptr);
}

std::string convertToBytes(const BIGNUM* bignum) {
  size_t length = BN_num_bytes(bignum);
  auto data = reinterpret_cast<char*>(calloc(length, sizeof(char)));
  BN_bn2bin(bignum, reinterpret_cast<uint8_t*>(data));
  std::string bytes(data, length);
  return bytes;
}

int getBigNumberLength(BIGNUM *a) {
  return BN_num_bytes(a) * 8;
}

namespace builtins {

bool CryptoKey::hasKeyUsage(JSObject *self, uint8_t usage) {
  MOZ_ASSERT(is_instance(self));
  auto usages = JS::GetReservedSlot(self, Slots::Usage).toInt32();
  return usage == (usages & usage);
}

CryptoKeyUsageBitmap CryptoKey::toKeyUsageBitmap(std::vector<std::string> key_usages) {
  auto key_usage_mask = 0;
  for (auto usage : key_usages) {
    if (usage == "encrypt") {
      key_usage_mask |= CryptoKeyUsageEncrypt;
    } else if (usage == "decrypt") {
      key_usage_mask |= CryptoKeyUsageDecrypt;
    } else if (usage == "sign") {
      key_usage_mask |= CryptoKeyUsageSign;
    } else if (usage == "verify") {
      key_usage_mask |= CryptoKeyUsageVerify;
    } else if (usage == "deriveKey") {
      key_usage_mask |= CryptoKeyUsageDeriveKey;
    } else if (usage == "deriveBits") {
      key_usage_mask |= CryptoKeyUsageDeriveBits;
    } else if (usage == "wrapKey") {
      key_usage_mask |= CryptoKeyUsageWrapKey;
    } else if (usage == "unwrapKey") {
      key_usage_mask |= CryptoKeyUsageUnwrapKey;
    }
  }
  return key_usage_mask;
}

JS::Result<CryptoKeyUsageBitmap> CryptoKey::toKeyUsageBitmap(JSContext *cx,
                                                             JS::HandleValue key_usages,
                                                             std::string_view error_message) {
  bool key_usages_is_array;
  if (!JS::IsArrayObject(cx, key_usages, &key_usages_is_array)) {
    return JS::Result<CryptoKeyUsageBitmap>(JS::Error());
  }

  if (!key_usages_is_array) {
    // TODO: This should check if the JS::HandleValue is iterable and if so, should convert it into a JS Array
    JS_ReportErrorASCII(cx, "The provided value cannot be converted to a sequence");
    return JS::Result<CryptoKeyUsageBitmap>(JS::Error());
  }

  uint32_t key_usages_length;
  JS::RootedObject array(cx, &key_usages.toObject());
  if (!array) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_SUBTLE_CRYPTO_ERROR,
                              error_message.data());
    return JS::Result<CryptoKeyUsageBitmap>(JS::Error());
  }
  if (!JS::GetArrayLength(cx, array, &key_usages_length)) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_SUBTLE_CRYPTO_ERROR,
                              error_message.data());
    return JS::Result<CryptoKeyUsageBitmap>(JS::Error());
  }
  auto key_usage_mask = 0;
  for (uint32_t index = 0; index < key_usages_length; index++) {
    JS::RootedValue val(cx);
    if (!JS_GetElement(cx, array, index, &val)) {
      JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_SUBTLE_CRYPTO_ERROR,
                                error_message.data());
      return JS::Result<CryptoKeyUsageBitmap>(JS::Error());
    }

    size_t val_len;
    JS::UniqueChars utf8chars = encode(cx, val, &val_len);
    if (!utf8chars) {
      return JS::Result<CryptoKeyUsageBitmap>(JS::Error());
    }

    std::string_view usage(utf8chars.get(), val_len);

    if (usage == "encrypt") {
      key_usage_mask |= CryptoKeyUsageEncrypt;
    } else if (usage == "decrypt") {
      key_usage_mask |= CryptoKeyUsageDecrypt;
    } else if (usage == "sign") {
      key_usage_mask |= CryptoKeyUsageSign;
    } else if (usage == "verify") {
      key_usage_mask |= CryptoKeyUsageVerify;
    } else if (usage == "deriveKey") {
      key_usage_mask |= CryptoKeyUsageDeriveKey;
    } else if (usage == "deriveBits") {
      key_usage_mask |= CryptoKeyUsageDeriveBits;
    } else if (usage == "wrapKey") {
      key_usage_mask |= CryptoKeyUsageWrapKey;
    } else if (usage == "unwrapKey") {
      key_usage_mask |= CryptoKeyUsageUnwrapKey;
    } else {
      JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_SUBTLE_CRYPTO_ERROR,
                                error_message.data());
      return JS::Result<CryptoKeyUsageBitmap>(JS::Error());
    }
  }
  return key_usage_mask;
}

const char *algorithmName(CryptoAlgorithmIdentifier algorithm) {
  const char *result = nullptr;
  switch (algorithm) {
  case CryptoAlgorithmIdentifier::RSAES_PKCS1_v1_5: {
    result = "RSAES-PKCS1-v1_5";
    break;
  }
  case CryptoAlgorithmIdentifier::RSASSA_PKCS1_v1_5: {
    result = "RSASSA-PKCS1-v1_5";
    break;
  }
  case CryptoAlgorithmIdentifier::RSA_PSS: {
    result = "RSA-PSS";
    break;
  }
  case CryptoAlgorithmIdentifier::RSA_OAEP: {
    result = "RSA-OAEP";
    break;
  }
  case CryptoAlgorithmIdentifier::ECDSA: {
    result = "ECDSA";
    break;
  }
  case CryptoAlgorithmIdentifier::ECDH: {
    result = "ECDH";
    break;
  }
  case CryptoAlgorithmIdentifier::AES_CTR: {
    result = "AES-CTR";
    break;
  }
  case CryptoAlgorithmIdentifier::AES_CBC: {
    result = "AES-CBC";
    break;
  }
  case CryptoAlgorithmIdentifier::AES_GCM: {
    result = "AES-GCM";
    break;
  }
  case CryptoAlgorithmIdentifier::AES_CFB: {
    result = "AES-CFB";
    break;
  }
  case CryptoAlgorithmIdentifier::AES_KW: {
    result = "AES-KW";
    break;
  }
  case CryptoAlgorithmIdentifier::HMAC: {
    result = "HMAC";
    break;
  }
  case CryptoAlgorithmIdentifier::SHA_1: {
    result = "SHA-1";
    break;
  }
  case CryptoAlgorithmIdentifier::SHA_224: {
    result = "SHA-224";
    break;
  }
  case CryptoAlgorithmIdentifier::SHA_256: {
    result = "SHA-256";
    break;
  }
  case CryptoAlgorithmIdentifier::SHA_384: {
    result = "SHA-384";
    break;
  }
  case CryptoAlgorithmIdentifier::SHA_512: {
    result = "SHA-512";
    break;
  }
  case CryptoAlgorithmIdentifier::HKDF: {
    result = "HKDF";
    break;
  }
  case CryptoAlgorithmIdentifier::PBKDF2: {
    result = "PBKDF2";
    break;
  }
  default: {
    MOZ_ASSERT_UNREACHABLE("Unknown `CryptoAlgorithmIdentifier` value");
  }
  }
  return result;
}

JS::Result<bool> CryptoKey::is_algorithm(JSContext *cx, JS::HandleObject self, CryptoAlgorithmIdentifier algorithm) {
  MOZ_ASSERT(is_instance(self));
  JS::RootedObject self_algorithm (cx, JS::GetReservedSlot(self, Slots::Algorithm).toObjectOrNull());
  MOZ_ASSERT(self_algorithm != nullptr);
  JS::Rooted<JS::Value> name_val(cx);
  if (!JS_GetProperty(cx, self_algorithm, "name", &name_val)) {
    return JS::Result<bool>(JS::Error());
  }
  JS::Rooted<JSString *> str(cx, JS::ToString(cx, name_val));
  if (!str) {
    return JS::Result<bool>(JS::Error());
  }
  size_t length;
  auto chars = encode(cx, str, &length);
  if (!chars) {
    return JS::Result<bool>(JS::Error());
  }
  bool match;
  if (!JS_StringEqualsAscii(cx, JS::ToString(cx, name_val), algorithmName(algorithm), &match)) {
    return JS::Result<bool>(JS::Error());
  }
  return match;
}

JSObject *CryptoKey::get_algorithm(JS::HandleObject self) {
  MOZ_ASSERT(is_instance(self));
  auto algorithm = JS::GetReservedSlot(self, Slots::Algorithm).toObjectOrNull();
  return algorithm;
}

bool CryptoKey::algorithm_get(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0)

  if (self == proto_obj.get()) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_INVALID_INTERFACE,
                              "algorithm get", "CryptoKey");
    return false;
  }

  auto algorithm = JS::GetReservedSlot(self, Slots::Algorithm).toObjectOrNull();
  MOZ_ASSERT(algorithm != nullptr);
  JS::RootedObject result(cx, algorithm);
  if (!result) {
    return false;
  }
  args.rval().setObject(*result);

  return true;
}

bool CryptoKey::extractable_get(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0)

  if (self == proto_obj.get()) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_INVALID_INTERFACE,
                              "extractable get", "CryptoKey");
    return false;
  }
  auto extractable = JS::GetReservedSlot(self, Slots::Extractable).toBoolean();

  args.rval().setBoolean(extractable);

  return true;
}

EVP_PKEY* CryptoKey::key(JSObject* self) {
  MOZ_ASSERT(is_instance(self));
  return static_cast<EVP_PKEY*>(JS::GetReservedSlot(self, Slots::Key).toPrivate());
}

CryptoKeyType CryptoKey::type(JSObject *self) {
  MOZ_ASSERT(is_instance(self));

  return static_cast<CryptoKeyType>(JS::GetReservedSlot(self, Slots::Type).toInt32());
}

bool CryptoKey::type_get(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0)

  JS::RootedString str(cx);
  if (self == proto_obj.get()) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_INVALID_INTERFACE, "type get",
                              "CryptoKey");
    return false;
  }
  auto type = static_cast<CryptoKeyType>(JS::GetReservedSlot(self, Slots::Type).toInt32());

  switch (type) {
  case CryptoKeyType::Private: {
    if (!(str = JS_NewStringCopyZ(cx, "private"))) {
      return false;
    }
    break;
  }
  case CryptoKeyType::Public: {
    if (!(str = JS_NewStringCopyZ(cx, "public"))) {
      return false;
    }
    break;
  }
  case CryptoKeyType::Secret: {
    if (!(str = JS_NewStringCopyZ(cx, "secret"))) {
      return false;
    }
    break;
  }

  default: {
    MOZ_ASSERT_UNREACHABLE("Unknown `CryptoKeyType` value");
  }
  };
  if (!str) {
    return false;
  }

  args.rval().setString(str);

  return true;
}

bool CryptoKey::usages_get(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0);

  if (self == proto_obj.get()) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_INVALID_INTERFACE, "usages get",
                              "CryptoKey");
    return false;
  }

  auto usage = JS::GetReservedSlot(self, Slots::Usage).toInt32();
  // The result is ordered alphabetically.
  JS::RootedValueVector result(cx);
  JS::RootedString str(cx);
  auto append = [&](const char *name) -> bool {
    if (!(str = JS_NewStringCopyZ(cx, name))) {
      return false;
    }
    if (!result.append(JS::StringValue(str))) {
      js::ReportOutOfMemory(cx);
      return false;
    }
    return true;
  };
  if (usage & CryptoKeyUsageDecrypt) {
    if (!append("decrypt")) {
      return false;
    }
  }
  if (usage & CryptoKeyUsageDeriveBits) {
    if (!append("deriveBits")) {
      return false;
    }
  }
  if (usage & CryptoKeyUsageDeriveKey) {
    if (!append("deriveKey")) {
      return false;
    }
  }
  if (usage & CryptoKeyUsageEncrypt) {
    if (!append("encrypt")) {
      return false;
    }
  }
  if (usage & CryptoKeyUsageSign) {
    if (!append("sign")) {
      return false;
    }
  }
  if (usage & CryptoKeyUsageUnwrapKey) {
    if (!append("unwrapKey")) {
      return false;
    }
  }
  if (usage & CryptoKeyUsageVerify) {
    if (!append("verify")) {
      return false;
    }
  }
  if (usage & CryptoKeyUsageWrapKey) {
    if (!append("wrapKey")) {
      return false;
    }
  }

  JS::Rooted<JSObject *> array(cx, JS::NewArrayObject(cx, result));
  if (!array) {
    return false;
  }

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

JSObject *CryptoKey::create(JSContext *cx, CryptoAlgorithm* algorithm, CryptoKeyType type,
                            bool extractable, CryptoKeyUsageBitmap usage, void *key) {

  JS::RootedObject instance(
      cx, JS_NewObjectWithGivenProto(cx, &CryptoKey::class_, CryptoKey::proto_obj));
  if (!instance) {
    return nullptr;
  }

  JS::RootedObject alg(cx, algorithm->toObject(cx));
  if (!alg) {
    return nullptr;
  }

  JS::SetReservedSlot(instance, Slots::Algorithm, JS::ObjectValue(*alg));
  JS::SetReservedSlot(instance, Slots::Type, JS::Int32Value(static_cast<uint8_t>(type)));
  JS::SetReservedSlot(instance, Slots::Extractable, JS::BooleanValue(extractable));
  JS::SetReservedSlot(instance, Slots::Usage, JS::Int32Value(usage));
  JS::SetReservedSlot(instance, Slots::Key, JS::PrivateValue(key));

  return instance;
}

JSObject *CryptoKey::create(JSContext *cx, CryptoAlgorithmRSA_OAEP_KeyImport* algorithm, CryptoKeyType type,
                            bool extractable, CryptoKeyUsageBitmap usage, EVP_PKEY *key) {
  JS::RootedObject instance(
      cx, JS_NewObjectWithGivenProto(cx, &CryptoKey::class_, CryptoKey::proto_obj));
  if (!instance) {
    return nullptr;
  }


  JS::RootedObject alg(cx, algorithm->toObject(cx));
  if (!alg) {
    return nullptr;
  }

  JS::SetReservedSlot(instance, Slots::Algorithm, JS::ObjectValue(*alg));
  JS::SetReservedSlot(instance, Slots::Type, JS::Int32Value(static_cast<uint8_t>(type)));
  JS::SetReservedSlot(instance, Slots::Extractable, JS::BooleanValue(extractable));
  JS::SetReservedSlot(instance, Slots::Usage, JS::Int32Value(usage));
  JS::SetReservedSlot(instance, Slots::Key, JS::PrivateValue(key));

  return instance;
}

bool CryptoKey::constructor(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_ILLEGAL_CTOR);
  return false;
}

bool CryptoKey::init_class(JSContext *cx, JS::HandleObject global) {
  return BuiltinImpl<CryptoKey>::init_class_impl(cx, global);
}

JSObject *CryptoKey::createRSA(JSContext *cx, CryptoAlgorithmRSA_OAEP_KeyImport *algorithm,
                                      CryptoKeyRSAComponents keyData, bool extractable,
                                      CryptoKeyUsageBitmap usages) {
  CryptoKeyType keyType;
  switch (keyData.type()) {
  case CryptoKeyRSAComponents::Type::Public:
    keyType = CryptoKeyType::Public;
    break;
  case CryptoKeyRSAComponents::Type::Private:
    keyType = CryptoKeyType::Private;
    break;
  default:
    return nullptr;
  }

  // When creating a private key, we require the p and q prime information.
  if (keyType == CryptoKeyType::Private && !keyData.hasAdditionalPrivateKeyParameters()) {
    return nullptr;
  }

  // But we don't currently support creating keys with any additional prime information.
  if (!keyData.otherPrimeInfos().empty()) {
    return nullptr;
  }

  // For both public and private keys, we need the public modulus and exponent.
  if (keyData.modulus().empty() || keyData.exponent().empty()) {
    return nullptr;
  }

  // For private keys, we require the private exponent, as well as p and q prime information.
  if (keyType == CryptoKeyType::Private) {
    if (keyData.privateExponent().empty() || keyData.firstPrimeInfo()->primeFactor.empty() ||
        keyData.secondPrimeInfo()->primeFactor.empty()) {
          return nullptr;
    }
  }

  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wdeprecated-declarations"
  auto rsa = RSA_new();
  #pragma clang diagnostic pop
  if (!rsa)
    return nullptr;

  auto n = convertToBigNumber(keyData.modulus());
  auto e = convertToBigNumber(keyData.exponent());
  if (!n || !e)
    return nullptr;

  // Calling with d null is fine as long as n and e are not null
  // Ownership of n and e transferred to OpenSSL
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wdeprecated-declarations"
  if (!RSA_set0_key(rsa, n, e, nullptr)){return nullptr;}
  #pragma clang diagnostic pop

  if (keyType == CryptoKeyType::Private) {
    auto d = convertToBigNumber(keyData.privateExponent());
    if (!d) {return nullptr;}

    // Calling with n and e null is fine as long as they were set prior
    // Ownership of d transferred to OpenSSL
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wdeprecated-declarations"
    if (!RSA_set0_key(rsa, nullptr, nullptr, d)) {return nullptr;}
    #pragma clang diagnostic pop

    auto p = convertToBigNumber(keyData.firstPrimeInfo()->primeFactor);
    auto q = convertToBigNumber(keyData.secondPrimeInfo()->primeFactor);
    if (!p || !q) {return nullptr;}

    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wdeprecated-declarations"
    // Ownership of p and q transferred to OpenSSL
    if (!RSA_set0_factors(rsa, p, q)) {return nullptr;}
    #pragma clang diagnostic pop

    // We set dmp1, dmpq1, and iqmp member of the RSA struct if the keyData has corresponding data.

    // dmp1 -- d mod (p - 1)
    auto dmp1 = (!keyData.firstPrimeInfo()->factorCRTExponent.empty())
                    ? convertToBigNumber(keyData.firstPrimeInfo()->factorCRTExponent)
                    : nullptr;
    // dmq1 -- d mod (q - 1)
    auto dmq1 = (!keyData.secondPrimeInfo()->factorCRTExponent.empty())
                    ? convertToBigNumber(keyData.secondPrimeInfo()->factorCRTExponent)
                    : nullptr;
    // iqmp -- q^(-1) mod p
    auto iqmp = (!keyData.secondPrimeInfo()->factorCRTCoefficient.empty())
                    ? convertToBigNumber(keyData.secondPrimeInfo()->factorCRTCoefficient)
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
  JS::RootedValue modulusLength(cx, JS::NumberValue(getBigNumberLength(n)));
  if (!JS_SetProperty(cx, alg, "modulusLength", modulusLength)) {
    return nullptr;
  }

  uint8_t* p = reinterpret_cast<uint8_t*>(calloc(keyData.exponent().size(), sizeof(uint8_t)));
  auto exp = keyData.exponent();
  std::copy(exp.begin(), exp.end(), p);

  JS::RootedObject buffer(cx, JS::NewArrayBufferWithContents(cx, keyData.exponent().size(), p));
  if (!buffer) {
    // We can be here if the array buffer was too large -- if that was the case then a JSMSG_BAD_ARRAY_LENGTH will have been created.
    // No other failure scenarios in this path will create a JS exception and so we need to create one.
    if (!JS_IsExceptionPending(cx)) {
      // TODO Rename error to InternalError
      JS_ReportErrorLatin1(cx, "InternalError");
    }
    JS_free(cx, p);
    return nullptr;
  }

  JS::RootedObject byte_array(cx, JS_NewUint8ArrayWithBuffer(cx, buffer, 0, keyData.exponent().size()));
  JS::RootedValue publicExponent(cx, JS::ObjectValue(*byte_array));
  if (!JS_SetProperty(cx, alg, "publicExponent", publicExponent)) {
    return nullptr;
  }

  JS::SetReservedSlot(instance, Slots::Algorithm, JS::ObjectValue(*alg));
  JS::SetReservedSlot(instance, Slots::Type, JS::Int32Value(static_cast<uint8_t>(keyType)));
  JS::SetReservedSlot(instance, Slots::Extractable, JS::BooleanValue(extractable));
  JS::SetReservedSlot(instance, Slots::Usage, JS::Int32Value(usages));
  JS::SetReservedSlot(instance, Slots::Key, JS::PrivateValue(pkey));
  return instance;
}

JSObject *CryptoKey::createRSA(JSContext *cx, CryptoAlgorithmRSASSA_PKCS1_v1_5_Import *algorithm,
                                      CryptoKeyRSAComponents keyData, bool extractable,
                                      CryptoKeyUsageBitmap usages) {
  MOZ_ASSERT(cx);
  MOZ_ASSERT(algorithm);
  CryptoKeyType keyType;
  switch (keyData.type()) {
  case CryptoKeyRSAComponents::Type::Public: {
    keyType = CryptoKeyType::Public;
    break;
  }
  case CryptoKeyRSAComponents::Type::Private: {
    keyType = CryptoKeyType::Private;
    break;
  }
  default: {
    MOZ_ASSERT_UNREACHABLE("coding error");
    return nullptr;
  }
  }

  // When creating a private key, we require the p and q prime information.
  if (keyType == CryptoKeyType::Private && !keyData.hasAdditionalPrivateKeyParameters()) {
    return nullptr;
  }

  // But we don't currently support creating keys with any additional prime information.
  if (!keyData.otherPrimeInfos().empty()) {
    return nullptr;
  }

  // For both public and private keys, we need the public modulus and exponent.
  if (keyData.modulus().empty() || keyData.exponent().empty()) {
    return nullptr;
  }

  // For private keys, we require the private exponent, as well as p and q prime information.
  if (keyType == CryptoKeyType::Private) {
    if (keyData.privateExponent().empty() || keyData.firstPrimeInfo()->primeFactor.empty() ||
        keyData.secondPrimeInfo()->primeFactor.empty()) {
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

  auto n = convertToBigNumber(keyData.modulus());
  auto e = convertToBigNumber(keyData.exponent());
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
    auto d = convertToBigNumber(keyData.privateExponent());
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

    auto p = convertToBigNumber(keyData.firstPrimeInfo()->primeFactor);
    auto q = convertToBigNumber(keyData.secondPrimeInfo()->primeFactor);
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
    auto dmp1 = (!keyData.firstPrimeInfo()->factorCRTExponent.empty())
                    ? convertToBigNumber(keyData.firstPrimeInfo()->factorCRTExponent)
                    : nullptr;
    // dmq1 -- d mod (q - 1)
    auto dmq1 = (!keyData.secondPrimeInfo()->factorCRTExponent.empty())
                    ? convertToBigNumber(keyData.secondPrimeInfo()->factorCRTExponent)
                    : nullptr;
    // iqmp -- q^(-1) mod p
    auto iqmp = (!keyData.secondPrimeInfo()->factorCRTCoefficient.empty())
                    ? convertToBigNumber(keyData.secondPrimeInfo()->factorCRTCoefficient)
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
  JS::RootedValue modulusLength(cx, JS::NumberValue(getBigNumberLength(n)));
  if (!JS_SetProperty(cx, alg, "modulusLength", modulusLength)) {
    return nullptr;
  }

  uint8_t* p = reinterpret_cast<uint8_t*>(calloc(keyData.exponent().size(), sizeof(uint8_t)));
  auto exp = keyData.exponent();
  std::copy(exp.begin(), exp.end(), p);

  JS::RootedObject buffer(cx, JS::NewArrayBufferWithContents(cx, keyData.exponent().size(), p));
  if (!buffer) {
    // We can be here if the array buffer was too large -- if that was the case then a JSMSG_BAD_ARRAY_LENGTH will have been created.
    // No other failure scenarios in this path will create a JS exception and so we need to create one.
    if (!JS_IsExceptionPending(cx)) {
      // TODO Rename error to InternalError
      JS_ReportErrorLatin1(cx, "InternalError");
    }
    JS_free(cx, p);
    return nullptr;
  }

  JS::RootedObject byte_array(cx, JS_NewUint8ArrayWithBuffer(cx, buffer, 0, keyData.exponent().size()));
  JS::RootedValue publicExponent(cx, JS::ObjectValue(*byte_array));
  if (!JS_SetProperty(cx, alg, "publicExponent", publicExponent)) {
    return nullptr;
  }

  JS::SetReservedSlot(instance, Slots::Algorithm, JS::ObjectValue(*alg));
  JS::SetReservedSlot(instance, Slots::Type, JS::Int32Value(static_cast<uint8_t>(keyType)));
  JS::SetReservedSlot(instance, Slots::Extractable, JS::BooleanValue(extractable));
  JS::SetReservedSlot(instance, Slots::Usage, JS::Int32Value(usages));
  JS::SetReservedSlot(instance, Slots::Key, JS::PrivateValue(pkey));
  return instance;
}

JSObject* CryptoKey::importJwkRsa(JSContext *cx, CryptoAlgorithmRSA_OAEP_KeyImport* algorithm,
                                         JsonWebKey* keyData, bool extractable,
                                         CryptoKeyUsageBitmap usages) {
  MOZ_ASSERT(cx);
  MOZ_ASSERT(algorithm);
  MOZ_ASSERT(keyData);
  if (keyData->kty != "RSA") {
    return nullptr;
  }
  if (keyData->key_ops.size() > 0) {
    auto ops = CryptoKey::toKeyUsageBitmap(keyData->key_ops);
    if (!(ops & usages)) {
      JS_ReportErrorASCII(cx, "The JWK 'key_ops' member was inconsistent with that specified by the Web Crypto call. The JWK usage must be a superset of those requested");
      return nullptr;
    }
  }
  if (keyData->ext && !keyData->ext.value() && extractable) {
    return nullptr;
  }

  if (!keyData->n.has_value() || !keyData->e.has_value()) {
    return nullptr;
  }
  auto modulusResult = GlobalProperties::forgivingBase64Decode(
      keyData->n.value(), GlobalProperties::base64URLDecodeTable);
  if (modulusResult.isErr()) {
    return nullptr;
  }
  auto modulus = modulusResult.unwrap();
  // Per RFC 7518 Section 6.3.1.1: https://tools.ietf.org/html/rfc7518#section-6.3.1.1
  if (modulus.starts_with('0')) {
    modulus = modulus.erase(0, 1);
  }
  auto exponentResult = GlobalProperties::forgivingBase64Decode(
      keyData->e.value(), GlobalProperties::base64URLDecodeTable);
  if (exponentResult.isErr()) {
    return nullptr;
  }
  auto exponent = exponentResult.unwrap();
  if (!keyData->d.has_value()) {
    // import public key
    auto publicKeyComponents = CryptoKeyRSAComponents::createPublic(modulus, exponent);
    return CryptoKey::createRSA(cx, algorithm, publicKeyComponents, extractable, usages);
  }

  // import private key
  auto privateExponentResult = GlobalProperties::forgivingBase64Decode(
      keyData->d.value(), GlobalProperties::base64URLDecodeTable);
  if (privateExponentResult.isErr()) {
    return nullptr;
  }
  auto privateExponent = privateExponentResult.unwrap();
  if (!keyData->p.has_value() && !keyData->q.has_value() && !keyData->dp.has_value() &&
      !keyData->dp.has_value() && !keyData->qi.has_value()) {
    auto privateKeyComponents =
        CryptoKeyRSAComponents::createPrivate(modulus, exponent, privateExponent);
    return CryptoKey::createRSA(cx, algorithm, privateKeyComponents, extractable, usages);
  }

  if (!keyData->p.has_value() || !keyData->q.has_value() || !keyData->dp.has_value() ||
      !keyData->dq.has_value() || !keyData->qi.has_value()) {
    return nullptr;
  }

  auto firstPrimeFactorResult = GlobalProperties::forgivingBase64Decode(
      keyData->p.value(), GlobalProperties::base64URLDecodeTable);
  if (firstPrimeFactorResult.isErr()) {
    return nullptr;
  }
  auto firstPrimeFactor = firstPrimeFactorResult.unwrap();
  auto firstFactorCRTExponentResult = GlobalProperties::forgivingBase64Decode(
      keyData->dp.value(), GlobalProperties::base64URLDecodeTable);
  if (firstFactorCRTExponentResult.isErr()) {
    return nullptr;
  }
  auto firstFactorCRTExponent = firstFactorCRTExponentResult.unwrap();
  auto secondPrimeFactorResult = GlobalProperties::forgivingBase64Decode(
      keyData->q.value(), GlobalProperties::base64URLDecodeTable);
  if (secondPrimeFactorResult.isErr()) {
    return nullptr;
  }
  auto secondPrimeFactor = secondPrimeFactorResult.unwrap();
  auto secondFactorCRTExponentResult = GlobalProperties::forgivingBase64Decode(
      keyData->dq.value(), GlobalProperties::base64URLDecodeTable);
  if (secondFactorCRTExponentResult.isErr()) {
    return nullptr;
  }
  auto secondFactorCRTExponent = secondFactorCRTExponentResult.unwrap();
  auto secondFactorCRTCoefficientResult = GlobalProperties::forgivingBase64Decode(
      keyData->qi.value(), GlobalProperties::base64URLDecodeTable);
  if (secondFactorCRTCoefficientResult.isErr()) {
    return nullptr;
  }
  auto secondFactorCRTCoefficient = secondFactorCRTCoefficientResult.unwrap();

  PrimeInfo firstPrimeInfo { firstPrimeFactor, firstFactorCRTExponent};

  PrimeInfo secondPrimeInfo { secondPrimeFactor, secondFactorCRTExponent, secondFactorCRTCoefficient};

  if (!keyData->oth.size()) {
    auto privateKeyComponents = CryptoKeyRSAComponents::createPrivateWithAdditionalData(
        modulus, exponent, privateExponent, firstPrimeInfo, secondPrimeInfo, {});
    return CryptoKey::createRSA(cx, algorithm, privateKeyComponents, extractable, usages);
  }
  return nullptr;

  std::vector<PrimeInfo> otherPrimeInfos;
  for (const auto &value : keyData->oth) {
    auto primeFactorResult = GlobalProperties::forgivingBase64Decode(
        value.r, GlobalProperties::base64URLDecodeTable);
    if (primeFactorResult.isErr()) {
      return nullptr;
    }
    auto primeFactor = primeFactorResult.unwrap();
    auto factorCRTExponentResult = GlobalProperties::forgivingBase64Decode(
        value.d, GlobalProperties::base64URLDecodeTable);
    if (factorCRTExponentResult.isErr()) {
      return nullptr;
    }
    auto factorCRTExponent = factorCRTExponentResult.unwrap();
    auto factorCRTCoefficientResult = GlobalProperties::forgivingBase64Decode(
        value.t, GlobalProperties::base64URLDecodeTable);
    if (factorCRTCoefficientResult.isErr()) {
      return nullptr;
    }
    auto factorCRTCoefficient = factorCRTCoefficientResult.unwrap();

    otherPrimeInfos.push_back(PrimeInfo(primeFactor,factorCRTExponent,factorCRTCoefficient));
  }

  auto privateKeyComponents = CryptoKeyRSAComponents::createPrivateWithAdditionalData(
      modulus, exponent, privateExponent, firstPrimeInfo, secondPrimeInfo, otherPrimeInfos);
  return CryptoKey::createRSA(cx, algorithm, privateKeyComponents, extractable, usages);
}

JSObject* CryptoKey::importJwkRsa(JSContext *cx, CryptoAlgorithmRSASSA_PKCS1_v1_5_Import* algorithm,
                                         JsonWebKey* keyData, bool extractable,
                                         CryptoKeyUsageBitmap usages) {
  MOZ_ASSERT(cx);
  MOZ_ASSERT(algorithm);
  MOZ_ASSERT(keyData);
  if (keyData->kty != "RSA") {
    return nullptr;
  }
  if (keyData->key_ops.size() > 0) {
    auto ops = CryptoKey::toKeyUsageBitmap(keyData->key_ops);
    if (!(ops & usages)) {
      JS_ReportErrorASCII(cx, "The JWK 'key_ops' member was inconsistent with that specified by the Web Crypto call. The JWK usage must be a superset of those requested");
      return nullptr;
    }
  }
  if (keyData->ext && !keyData->ext.value() && extractable) {
    // TODO: Change to a DataError instance
    JS_ReportErrorLatin1(cx, "Data provided to an operation does not meet requirements");
    return nullptr;
  }

  if (!keyData->n.has_value() || !keyData->e.has_value()) {
// TODO: Change to a DataError instance
    JS_ReportErrorLatin1(cx, "Data provided to an operation does not meet requirements");
        return nullptr;
  }
  auto modulusResult = GlobalProperties::forgivingBase64Decode(
      keyData->n.value(), GlobalProperties::base64URLDecodeTable);
  if (modulusResult.isErr()) {
    // TODO: Change to a DataError instance
    JS_ReportErrorLatin1(cx, "The JWK member 'n' could not be base64url decoded or contained padding");
    return nullptr;
  }
  auto modulus = modulusResult.unwrap();
  // Per RFC 7518 Section 6.3.1.1: https://tools.ietf.org/html/rfc7518#section-6.3.1.1
  if (modulus.starts_with('0')) {
    modulus = modulus.erase(0, 1);
  }
  auto dataResult = ConvertJSValueToByteString(cx, keyData->e.value());
  if (dataResult.isErr()) {
    // TODO: Change to a DataError instance
    JS_ReportErrorLatin1(cx, "Data provided to an operation does not meet requirements");
    return nullptr;
  }
  auto data = dataResult.unwrap();
  auto exponentResult = GlobalProperties::forgivingBase64Decode(
      data, GlobalProperties::base64URLDecodeTable);
  if (exponentResult.isErr()) {
    // TODO: Change to a DataError instance
    JS_ReportErrorLatin1(cx, "The JWK member 'e' could not be base64url decoded or contained padding");
    return nullptr;
  }
  auto exponent = exponentResult.unwrap();
  if (!keyData->d.has_value()) {
    // import public key
    auto publicKeyComponents = CryptoKeyRSAComponents::createPublic(modulus, exponent);
    return CryptoKey::createRSA(cx, algorithm, publicKeyComponents, extractable, usages);
  }

  // import private key
  auto privateExponentResult = GlobalProperties::forgivingBase64Decode(
      keyData->d.value(), GlobalProperties::base64URLDecodeTable);
  if (privateExponentResult.isErr()) {
    // TODO: Change to a DataError instance
    JS_ReportErrorLatin1(cx, "The JWK member 'd' could not be base64url decoded or contained padding");
    return nullptr;
  }
  auto privateExponent = privateExponentResult.unwrap();
  if (!keyData->p.has_value() && !keyData->q.has_value() && !keyData->dp.has_value() &&
      !keyData->dp.has_value() && !keyData->qi.has_value()) {
    auto privateKeyComponents =
        CryptoKeyRSAComponents::createPrivate(modulus, exponent, privateExponent);
    return CryptoKey::createRSA(cx, algorithm, privateKeyComponents, extractable, usages);
  }

  if (!keyData->p.has_value() || !keyData->q.has_value() || !keyData->dp.has_value() ||
      !keyData->dq.has_value() || !keyData->qi.has_value()) {
    // TODO: Change to a DataError instance
    JS_ReportErrorLatin1(cx, "Data provided to an operation does not meet requirements");
    return nullptr;
  }

  auto firstPrimeFactorResult = GlobalProperties::forgivingBase64Decode(
      keyData->p.value(), GlobalProperties::base64URLDecodeTable);
  if (firstPrimeFactorResult.isErr()) {
    // TODO: Change to a DataError instance
    JS_ReportErrorLatin1(cx, "The JWK member 'p' could not be base64url decoded or contained padding");
    return nullptr;
  }
  auto firstPrimeFactor = firstPrimeFactorResult.unwrap();
  auto firstFactorCRTExponentResult = GlobalProperties::forgivingBase64Decode(
      keyData->dp.value(), GlobalProperties::base64URLDecodeTable);
  if (firstFactorCRTExponentResult.isErr()) {
    // TODO: Change to a DataError instance
    JS_ReportErrorLatin1(cx, "The JWK member 'dp' could not be base64url decoded or contained padding");
    return nullptr;
  }
  auto firstFactorCRTExponent = firstFactorCRTExponentResult.unwrap();
  auto secondPrimeFactorResult = GlobalProperties::forgivingBase64Decode(
      keyData->q.value(), GlobalProperties::base64URLDecodeTable);
  if (secondPrimeFactorResult.isErr()) {
    // TODO: Change to a DataError instance
    JS_ReportErrorLatin1(cx, "The JWK member 'q' could not be base64url decoded or contained padding");
    return nullptr;
  }
  auto secondPrimeFactor = secondPrimeFactorResult.unwrap();
  auto secondFactorCRTExponentResult = GlobalProperties::forgivingBase64Decode(
      keyData->dq.value(), GlobalProperties::base64URLDecodeTable);
  if (secondFactorCRTExponentResult.isErr()) {
    // TODO: Change to a DataError instance
    JS_ReportErrorLatin1(cx, "The JWK member 'dq' could not be base64url decoded or contained padding");
    return nullptr;
  }
  auto secondFactorCRTExponent = secondFactorCRTExponentResult.unwrap();
  auto secondFactorCRTCoefficientResult = GlobalProperties::forgivingBase64Decode(
      keyData->qi.value(), GlobalProperties::base64URLDecodeTable);
  if (secondFactorCRTCoefficientResult.isErr()) {
    // TODO: Change to a DataError instance
    JS_ReportErrorLatin1(cx, "The JWK member 'qi' could not be base64url decoded or contained padding");
    return nullptr;
  }
  auto secondFactorCRTCoefficient = secondFactorCRTCoefficientResult.unwrap();

  PrimeInfo firstPrimeInfo {firstPrimeFactor, firstFactorCRTExponent};

  PrimeInfo secondPrimeInfo {secondPrimeFactor, secondFactorCRTExponent, secondFactorCRTCoefficient};

  if (!keyData->oth.size()) {
    auto privateKeyComponents = CryptoKeyRSAComponents::createPrivateWithAdditionalData(
        modulus, exponent, privateExponent, firstPrimeInfo, secondPrimeInfo, {});
    return CryptoKey::createRSA(cx, algorithm, privateKeyComponents, extractable, usages);
  }

  std::vector<PrimeInfo> otherPrimeInfos;
  for (const auto &value : keyData->oth) {
    auto primeFactorResult = GlobalProperties::forgivingBase64Decode(
        value.r, GlobalProperties::base64URLDecodeTable);
    if (primeFactorResult.isErr()) {
      return nullptr;
    }
    auto primeFactor = primeFactorResult.unwrap();
    auto factorCRTExponentResult = GlobalProperties::forgivingBase64Decode(
        value.d, GlobalProperties::base64URLDecodeTable);
    if (factorCRTExponentResult.isErr()) {
      return nullptr;
    }
    auto factorCRTExponent = factorCRTExponentResult.unwrap();
    auto factorCRTCoefficientResult = GlobalProperties::forgivingBase64Decode(
        value.t, GlobalProperties::base64URLDecodeTable);
    if (factorCRTCoefficientResult.isErr()) {
      return nullptr;
    }
    auto factorCRTCoefficient = factorCRTCoefficientResult.unwrap();

    otherPrimeInfos.push_back(PrimeInfo(primeFactor, factorCRTExponent, factorCRTCoefficient));
  }

  auto privateKeyComponents = CryptoKeyRSAComponents::createPrivateWithAdditionalData(
      modulus, exponent, privateExponent, firstPrimeInfo, secondPrimeInfo, otherPrimeInfos);
  return CryptoKey::createRSA(cx, algorithm, privateKeyComponents, extractable, usages);
}

namespace {
  JS::Result<builtins::CryptoAlgorithmIdentifier> toHashIdentifier(JSContext *cx, JS::HandleValue value) {
    auto normalizedHashAlgorithm = CryptoAlgorithmDigest::normalize(cx, value);
    if (!normalizedHashAlgorithm) {
      return JS::Result<builtins::CryptoAlgorithmIdentifier>(JS::Error());
    }
    return normalizedHashAlgorithm->identifier();
  }


JS::Result<CryptoAlgorithmIdentifier> normalizeIdentifier(JSContext *cx, JS::HandleValue value) {
  JS::Rooted<JSString *> algorithmIdentifierString(cx);

  JS::Rooted<JSObject *> params(cx);

  // The value can either be a JS String or a JS Object with a 'name' property
  if (value.isString()) {
    auto obj = JS_NewPlainObject(cx);
    params.set(obj);
    JS_SetProperty(cx, params, "name", value);
    algorithmIdentifierString.set(value.toString());
  } else if (value.isObject()) {
    params.set(&value.toObject());
    JS::Rooted<JS::Value> name_val(cx);
    if (!JS_GetProperty(cx, params, "name", &name_val)) {
      return JS::Result<CryptoAlgorithmIdentifier>(JS::Error());;
    }
    JS::RootedString str(cx, JS::ToString(cx, name_val));
    if (!str) {
      return JS::Result<CryptoAlgorithmIdentifier>(JS::Error());;
    }
    algorithmIdentifierString.set(str);
  } else {
    JS_ReportErrorUTF8(cx, "Algorithm: Unrecognized name");
    return JS::Result<CryptoAlgorithmIdentifier>(JS::Error());;
  }
  size_t algorithmLen;
  JS::UniqueChars algorithmChars = encode(cx, algorithmIdentifierString, &algorithmLen);
  if (!algorithmChars) {
    return JS::Result<CryptoAlgorithmIdentifier>(JS::Error());;
  }

  std::string algorithm(algorithmChars.get(), algorithmLen);
  std::transform(algorithm.begin(), algorithm.end(), algorithm.begin(),
                 [](unsigned char c) { return std::toupper(c); });

  if (algorithm == "RSAES-PKCS1-V1_5") {
    return CryptoAlgorithmIdentifier::RSAES_PKCS1_v1_5;
  } else if (algorithm == "RSASSA-PKCS1-V1_5") {
    return CryptoAlgorithmIdentifier::RSASSA_PKCS1_v1_5;
  } else if (algorithm == "RSA-PSS") {
    return CryptoAlgorithmIdentifier::RSA_PSS;
  } else if (algorithm == "RSA-OAEP") {
    return CryptoAlgorithmIdentifier::RSA_OAEP;
  } else if (algorithm == "ECDSA") {
    return CryptoAlgorithmIdentifier::ECDSA;
  } else if (algorithm == "ECDH") {
    return CryptoAlgorithmIdentifier::ECDH;
  } else if (algorithm == "AES-CTR") {
    return CryptoAlgorithmIdentifier::AES_CTR;
  } else if (algorithm == "AES-CBC") {
    return CryptoAlgorithmIdentifier::AES_CBC;
  } else if (algorithm == "AES-GCM") {
    return CryptoAlgorithmIdentifier::AES_GCM;
  } else if (algorithm == "AES-CFB") {
    return CryptoAlgorithmIdentifier::AES_CFB;
  } else if (algorithm == "AES-KW") {
    return CryptoAlgorithmIdentifier::AES_KW;
  } else if (algorithm == "HMAC") {
    return CryptoAlgorithmIdentifier::HMAC;
  } else if (algorithm == "SHA-1") {
    return CryptoAlgorithmIdentifier::SHA_1;
  } else if (algorithm == "SHA-224") {
    return CryptoAlgorithmIdentifier::SHA_224;
  } else if (algorithm == "SHA-256") {
    return CryptoAlgorithmIdentifier::SHA_256;
  } else if (algorithm == "SHA-384") {
    return CryptoAlgorithmIdentifier::SHA_384;
  } else if (algorithm == "SHA-512") {
    return CryptoAlgorithmIdentifier::SHA_512;
  } else if (algorithm == "HKDF") {
    return CryptoAlgorithmIdentifier::HKDF;
  } else if (algorithm == "PBKDF2") {
    return CryptoAlgorithmIdentifier::PBKDF2;
  } else {
    JS_ReportErrorUTF8(cx, "Algorithm: Unrecognized name");
    return JS::Result<CryptoAlgorithmIdentifier>(JS::Error());
  } 
  }
}

std::unique_ptr<CryptoAlgorithmEncDec> CryptoAlgorithmEncDec::normalize(JSContext *cx, JS::HandleValue value){
  auto identifierResult =  normalizeIdentifier(cx, value);
  if (identifierResult.isErr()) {
    return nullptr;
  }
  auto identifier = identifierResult.unwrap();
  JS::Rooted<JSObject *> params(cx);

  // The value can either be a JS String or a JS Object with a 'name' property
  if (value.isString()) {
    auto obj = JS_NewPlainObject(cx);
    params.set(obj);
    JS_SetProperty(cx, params, "name", value);
  } else if (value.isObject()) {
    params.set(&value.toObject());
  }

  switch (identifier) {
    case CryptoAlgorithmIdentifier::RSAES_PKCS1_v1_5: {
      return std::make_unique<CryptoAlgorithmRSAES_PKCS1_v1_5>(std::nullopt, std::nullopt);
      break;
    }
    case CryptoAlgorithmIdentifier::RSA_OAEP: {
      bool found;
      if (!JS_HasProperty(cx, params, "label", &found)) {
        return nullptr;
      }
      if (!found) {
        return std::make_unique<CryptoAlgorithmRSA_OAEP_EncDec>(std::nullopt);
      } else {
        JS::Rooted<JS::Value> label_val(cx);
        if (!JS_GetProperty(cx, params, "label", &label_val)) {
          return nullptr;
        }
        std::optional<std::span<uint8_t>> label = value_to_buffer(cx, label_val, "normaliseAlgorithm 1");
        if (!label.has_value()) {
          // value_to_buffer would have already created a JS exception so we don't need to create one ourselves.
          return nullptr;
        }
        return std::make_unique<CryptoAlgorithmRSA_OAEP_EncDec>(label);
      }
      break;
    }
    case CryptoAlgorithmIdentifier::AES_CBC:
    case CryptoAlgorithmIdentifier::AES_CFB: {
      JS::Rooted<JS::Value> iv_val(cx);
      if (!JS_GetProperty(cx, params, "iv", &iv_val)) {
        return nullptr;
      }
      std::optional<std::span<uint8_t>>iv = value_to_buffer(cx, iv_val, "normaliseAlgorithm 2");
      if (!iv.has_value()) {
        // value_to_buffer would have already created a JS exception so we don't need to create one ourselves.
        return nullptr;
      }
      if (identifier == CryptoAlgorithmIdentifier::AES_CBC) {
        return std::make_unique<CryptoAlgorithmAES_CBC_EncDec>(iv.value());
      } else {
        return std::make_unique<CryptoAlgorithmAES_CFB_EncDec>(iv.value());
      }
      break;
    }
    case CryptoAlgorithmIdentifier::AES_CTR: {
      JS::Rooted<JS::Value> counter_val(cx);
      if (!JS_GetProperty(cx, params, "counter", &counter_val)) {
        return nullptr;
      }
      size_t length;
      std::optional<std::span<uint8_t>> counter = value_to_buffer(cx, counter_val, "normaliseAlgorithm 3");
      if (!counter.has_value()) {
        // value_to_buffer would have already created a JS exception so we don't need to create one ourselves.
        return nullptr;
      }
      JS::Rooted<JS::Value> length_val(cx);
      if (!JS_GetProperty(cx, params, "length", &length_val)) {
        return nullptr;
      }
      if (!length_val.isNumber()) {
        return nullptr;
      }
      length = length_val.toNumber();
      return std::make_unique<CryptoAlgorithmAES_CTR_EncDec>(counter.value(), length);
      break;
    }
    case CryptoAlgorithmIdentifier::AES_GCM: {
      JS::Rooted<JS::Value> iv_val(cx);
      if (!JS_GetProperty(cx, params, "iv", &iv_val)) {
        return nullptr;
      }
      std::optional<std::span<uint8_t>>iv = value_to_buffer(cx, iv_val, "normaliseAlgorithm 4");
      if (!iv.has_value()) {
        // value_to_buffer would have already created a JS exception so we don't need to create one ourselves.
        return nullptr;
      }
      bool found;
      std::optional<std::span<uint8_t>> additionalData = std::nullopt;
      if (!JS_HasProperty(cx, params, "additionalData", &found)) {
        return nullptr;
      }
      if (found) {
        JS::Rooted<JS::Value> additionalData_val(cx);
        if (!JS_GetProperty(cx, params, "additionalData", &additionalData_val)) {
          return nullptr;
        }
        std::optional<std::span<uint8_t>> additionalDataBuffer = value_to_buffer(cx, additionalData_val, "normaliseAlgorithm 5");
        if (!additionalDataBuffer.has_value()) {
          // value_to_buffer would have already created a JS exception so we don't need to create one ourselves.
          return nullptr;
        }
        additionalData = additionalDataBuffer;
      }
      std::optional<uint8_t> tagLength = std::nullopt;
      if (!JS_HasProperty(cx, params, "tagLength", &found)) {
        return nullptr;
      }
      if (found) {
        JS::Rooted<JS::Value> tagLength_val(cx);
        if (!JS_GetProperty(cx, params, "tagLength", &tagLength_val)) {
          return nullptr;
        }
        if (!tagLength_val.isNumber()) {
          return nullptr;
        }
        tagLength = tagLength_val.toNumber();
      }
      return std::make_unique<CryptoAlgorithmAES_GCM_EncDec>(iv.value(), additionalData, tagLength);
      break;
    }
    default:
      return nullptr;
  }
};
std::unique_ptr<CryptoAlgorithmSignVerify> CryptoAlgorithmSignVerify::normalize(JSContext* cx, JS::HandleValue value){
  auto identifierResult =  normalizeIdentifier(cx, value);
  if (identifierResult.isErr()) {
    return nullptr;
  }
  auto identifier = identifierResult.unwrap();
  JS::Rooted<JSObject *> params(cx);

  // The value can either be a JS String or a JS Object with a 'name' property
  if (value.isString()) {
    auto obj = JS_NewPlainObject(cx);
    params.set(obj);
    if (!JS_SetProperty(cx, params, "name", value)) {
      return nullptr;
    }
  } else if (value.isObject()) {
    params.set(&value.toObject());
  }

  switch (identifier) {
    case CryptoAlgorithmIdentifier::RSASSA_PKCS1_v1_5: {
      return std::make_unique<CryptoAlgorithmRSASSA_PKCS1_v1_5>();
      break;
    }
    case CryptoAlgorithmIdentifier::HMAC: {
      JS::Rooted<JS::Value> hash_val(cx);
      if (!JS_GetProperty(cx, params, "hash", &hash_val)) {
        return nullptr;
      }
      auto hashIdentifier = toHashIdentifier(cx, hash_val);
      if (hashIdentifier.isErr()) {
        return nullptr;
      }
      return std::make_unique<CryptoAlgorithmHMAC>(hashIdentifier.unwrap());
      break;
    }
    case CryptoAlgorithmIdentifier::ECDSA: {
      JS::Rooted<JS::Value> hash_val(cx);
      if (!JS_GetProperty(cx, params, "hash", &hash_val)) {
        return nullptr;
      }
      auto hashIdentifier = toHashIdentifier(cx, hash_val);
      if (hashIdentifier.isErr()) {
        return nullptr;
      }
      return std::make_unique<CryptoAlgorithmECDSASignVerify>(hashIdentifier.unwrap());
      break;
    }
    case CryptoAlgorithmIdentifier::RSA_PSS: {
      size_t saltLength;
      JS::Rooted<JS::Value> saltLength_val(cx);
      if (!JS_GetProperty(cx, params, "saltLength", &saltLength_val)) {
        return nullptr;
      }
      if (!saltLength_val.isNumber()) {
        return nullptr;
      }
      saltLength = saltLength_val.toNumber();
      return std::make_unique<CryptoAlgorithmRSA_PSS_SignVerify>(saltLength);
      break;
    }
    default: {
      return nullptr;
    }
  }

};
std::unique_ptr<CryptoAlgorithmDigest> CryptoAlgorithmDigest::normalize(JSContext* cx, JS::HandleValue value){
  auto identifierResult =  normalizeIdentifier(cx, value);
  if (identifierResult.isErr()) {
    return nullptr;
  }
  auto identifier = identifierResult.unwrap();

  switch (identifier) {
    case CryptoAlgorithmIdentifier::SHA_1:
      return std::make_unique<CryptoAlgorithmSHA1>();
    case CryptoAlgorithmIdentifier::SHA_224:
      return std::make_unique<CryptoAlgorithmSHA224>();
    case CryptoAlgorithmIdentifier::SHA_256:
      return std::make_unique<CryptoAlgorithmSHA256>();
    case CryptoAlgorithmIdentifier::SHA_384:
      return std::make_unique<CryptoAlgorithmSHA384>();
    case CryptoAlgorithmIdentifier::SHA_512: 
      return std::make_unique<CryptoAlgorithmSHA512>();
    default:
      return nullptr;
  }
};
std::unique_ptr<CryptoAlgorithmGenerateKey> CryptoAlgorithmGenerateKey::normalize(JSContext* cx, JS::HandleValue value){
  auto identifierResult =  normalizeIdentifier(cx, value);
  if (identifierResult.isErr()) {
    return nullptr;
  }
  auto identifier = identifierResult.unwrap();
  JS::Rooted<JSObject *> params(cx);

  // The value can either be a JS String or a JS Object with a 'name' property
  if (value.isString()) {
    auto obj = JS_NewPlainObject(cx);
    params.set(obj);
    if (!JS_SetProperty(cx, params, "name", value)) {
      return nullptr;
    }
  } else if (value.isObject()) {
    params.set(&value.toObject());
  }

  switch (identifier) {
    case CryptoAlgorithmIdentifier::RSAES_PKCS1_v1_5: {
      JS::Rooted<JS::Value> publicExponent_val(cx);
      if (!JS_GetProperty(cx, params, "publicExponent", &publicExponent_val)) {
        return nullptr;
      }
      std::optional<std::span<uint8_t>> publicExponent = value_to_buffer(cx, publicExponent_val, "normaliseAlgorithm 6");
      if (!publicExponent.has_value()) {
        // value_to_buffer would have already created a JS exception so we don't need to create one ourselves.
        return nullptr;
      }
      size_t modulusLength;
      JS::Rooted<JS::Value> modulusLength_val(cx);
      if (!JS_GetProperty(cx, params, "modulusLength", &modulusLength_val)) {
        return nullptr;
      }
      if (!modulusLength_val.isNumber()) {
        return nullptr;
      }
      modulusLength = modulusLength_val.toNumber();

      return std::make_unique<CryptoAlgorithmRSAES_PKCS1_v1_5>(modulusLength, publicExponent);
      break;
    }
    case CryptoAlgorithmIdentifier::RSASSA_PKCS1_v1_5:
    case CryptoAlgorithmIdentifier::RSA_PSS:
    case CryptoAlgorithmIdentifier::RSA_OAEP: {
      JS::Rooted<JS::Value> hash_val(cx);
      if (!JS_GetProperty(cx, params, "hash", &hash_val)) {
        return nullptr;
      }
      auto hashIdentifier = toHashIdentifier(cx, hash_val);
      if (hashIdentifier.isErr()) {
        return nullptr;
      }
      JS::Rooted<JS::Value> publicExponent_val(cx);
      if (!JS_GetProperty(cx, params, "publicExponent", &publicExponent_val)) {
        return nullptr;
      }
      std::optional<std::span<uint8_t>>publicExponent = value_to_buffer(cx, publicExponent_val, "normaliseAlgorithm 7");
      if (!publicExponent.has_value()) {
        // value_to_buffer would have already created a JS exception so we don't need to create one ourselves.
        return nullptr;
      }
      size_t modulusLength;
      JS::Rooted<JS::Value> modulusLength_val(cx);
      if (!JS_GetProperty(cx, params, "modulusLength", &modulusLength_val)) {
        return nullptr;
      }
      if (!modulusLength_val.isNumber()) {
        return nullptr;
      }
      modulusLength = modulusLength_val.toNumber();
      if (identifier == CryptoAlgorithmIdentifier::RSASSA_PKCS1_v1_5) {
        return std::make_unique<CryptoAlgorithmRSASSA_PKCS1_v1_5_KeyGen>(modulusLength, publicExponent.value(), hashIdentifier.unwrap());
      } else if (identifier == CryptoAlgorithmIdentifier::RSA_PSS) {
        return std::make_unique<CryptoAlgorithmRSA_PSS_Key>(modulusLength, publicExponent.value(), hashIdentifier.unwrap());
      } else {
        return std::make_unique<CryptoAlgorithmRSA_OAEP_Key>(modulusLength, publicExponent.value(), hashIdentifier.unwrap());
      }
      break;
    }
    case CryptoAlgorithmIdentifier::AES_CTR:
    case CryptoAlgorithmIdentifier::AES_CBC:
    case CryptoAlgorithmIdentifier::AES_GCM:
    case CryptoAlgorithmIdentifier::AES_CFB:
    case CryptoAlgorithmIdentifier::AES_KW: {
      unsigned short length;
      JS::Rooted<JS::Value> length_val(cx);
      if (!JS_GetProperty(cx, params, "length", &length_val)) {
        return nullptr;
      }
      if (!length_val.isNumber()) {
        return nullptr;
      }
      length = length_val.toNumber();
      if (identifier == CryptoAlgorithmIdentifier::AES_CTR){
        return std::make_unique<CryptoAlgorithmAES_CTR_Key>(length);
      } else if (identifier == CryptoAlgorithmIdentifier::AES_CBC){
        return std::make_unique<CryptoAlgorithmAES_CBC_Key>(length);
      } else if (identifier == CryptoAlgorithmIdentifier::AES_GCM){
        return std::make_unique<CryptoAlgorithmAES_GCM_Key>(length);
      } else if (identifier == CryptoAlgorithmIdentifier::AES_CFB){
        return std::make_unique<CryptoAlgorithmAES_CFB_Key>(length);
      } else if (identifier == CryptoAlgorithmIdentifier::AES_KW){
        return std::make_unique<CryptoAlgorithmAES_KW>(length);
      }
      break;
    }
    case CryptoAlgorithmIdentifier::HMAC: {
      JS::Rooted<JS::Value> hash_val(cx);
      if (!JS_GetProperty(cx, params, "hash", &hash_val)) {
        return nullptr;
      }
      auto hashIdentifier = toHashIdentifier(cx, hash_val);
      if (hashIdentifier.isErr()) {
        return nullptr;
      }
      bool found;
      size_t length;
      if (!JS_HasProperty(cx, params, "length", &found)) {
        return nullptr;
      }
      if (found) {
        JS::Rooted<JS::Value> length_val(cx);
        if (!JS_GetProperty(cx, params, "length", &length_val)) {
          return nullptr;
        }
        if (!length_val.isNumber()) {
          return nullptr;
        }
        length = length_val.toNumber();
        return std::make_unique<CryptoAlgorithmHMAC>(hashIdentifier.unwrap(), length);
      } else {
        return std::make_unique<CryptoAlgorithmHMAC>(hashIdentifier.unwrap());
      }
      break;
    }
    case CryptoAlgorithmIdentifier::ECDSA:
    case CryptoAlgorithmIdentifier::ECDH: {
      JS::Rooted<JS::Value> namedCurve_val(cx);
      if (!JS_GetProperty(cx, params, "namedCurve", &namedCurve_val)) {
        return nullptr;
      }
      size_t length;
      auto namedCurveChars = encode(cx, namedCurve_val, &length);
      
      if (!namedCurveChars) {
        return nullptr;
      } 

      if (identifier == CryptoAlgorithmIdentifier::ECDSA) {
        return std::make_unique<CryptoAlgorithmECDSAGenKey>(namedCurveChars.get());
      } else {
        return std::make_unique<CryptoAlgorithmECDHGenKey>(namedCurveChars.get());
      }
      break;
    }
    default:
      return nullptr;
  }
  MOZ_ASSERT_UNREACHABLE("CryptoAlgorithmGenerateKey::normalize unreachable reached.");
  return nullptr;
};
std::unique_ptr<CryptoAlgorithmDeriveBits> CryptoAlgorithmDeriveBits::normalize(JSContext* cx, JS::HandleValue value){
  auto identifierResult =  normalizeIdentifier(cx, value);
  if (identifierResult.isErr()) {
    return nullptr;
  }
  auto identifier = identifierResult.unwrap();
  JS::Rooted<JSObject *> params(cx);

  // The value can either be a JS String or a JS Object with a 'name' property
  if (value.isString()) {
    auto obj = JS_NewPlainObject(cx);
    params.set(obj);
    JS_SetProperty(cx, params, "name", value);
  } else if (value.isObject()) {
    params.set(&value.toObject());
  }

  switch (identifier) {
    case CryptoAlgorithmIdentifier::ECDH: {
      // TODO: finish this
      // return CryptoAlgorithmEcdhKeyDeriveParams(..);
      // break;
    }
    case CryptoAlgorithmIdentifier::HKDF: {
      JS::Rooted<JS::Value> hash_val(cx);
      if (!JS_GetProperty(cx, params, "hash", &hash_val)) {
        return nullptr;
      }
      auto hashIdentifier = toHashIdentifier(cx, hash_val);
      if (hashIdentifier.isErr()) {
        return nullptr;
      }
      JS::Rooted<JS::Value> salt_val(cx);
      if (!JS_GetProperty(cx, params, "salt", &salt_val)) {
        return nullptr;
      }
      std::optional<std::span<uint8_t>>salt = value_to_buffer(cx, salt_val, "normaliseAlgorithm 8");
      if (!salt.has_value()) {
        // value_to_buffer would have already created a JS exception so we don't need to create one ourselves.
        return nullptr;
      }
      JS::Rooted<JS::Value> info_val(cx);
      if (!JS_GetProperty(cx, params, "info", &info_val)) {
        return nullptr;
      }
      std::optional<std::span<uint8_t>>info = value_to_buffer(cx, info_val, "normaliseAlgorithm 9");
      if (!info.has_value()) {
        // value_to_buffer would have already created a JS exception so we don't need to create one ourselves.
        return nullptr;
      }
      return std::make_unique<CryptoAlgorithmHKDFDerive>(hashIdentifier.unwrap(), salt.value(), info.value());
      break;
    }
    case CryptoAlgorithmIdentifier::PBKDF2: {
      JS::Rooted<JS::Value> hash_val(cx);
      if (!JS_GetProperty(cx, params, "hash", &hash_val)) {
        return nullptr;
      }
      auto hashIdentifier = toHashIdentifier(cx, hash_val);
      if (hashIdentifier.isErr()) {
        return nullptr;
      }
      JS::Rooted<JS::Value> salt_val(cx);
      if (!JS_GetProperty(cx, params, "salt", &salt_val)) {
        return nullptr;
      }
      std::optional<std::span<uint8_t>>salt = value_to_buffer(cx, salt_val, "normaliseAlgorithm 10");
      if (!salt.has_value()) {
        // value_to_buffer would have already created a JS exception so we don't need to create one ourselves.
        return nullptr;
      }
      JS::Rooted<JS::Value> iterations_val(cx);
      if (!JS_GetProperty(cx, params, "iterations", &iterations_val)) {
        return nullptr;
      }
      if (!iterations_val.isNumber()) {
        return nullptr;
      }
      return std::make_unique<CryptoAlgorithmPBKDF2Key>(hashIdentifier.unwrap(), salt.value(), iterations_val.toNumber());
      break;
    }
    default:
      return nullptr;
  }

};
std::unique_ptr<CryptoAlgorithmImportKey> CryptoAlgorithmImportKey::normalize(JSContext* cx, JS::HandleValue value){
  auto identifierResult =  normalizeIdentifier(cx, value);
  if (identifierResult.isErr()) {
    return nullptr;
  }
  auto identifier = identifierResult.unwrap();
  JS::Rooted<JSObject *> params(cx);

  // The value can either be a JS String or a JS Object with a 'name' property
  if (value.isString()) {
    auto obj = JS_NewPlainObject(cx);
    params.set(obj);
    JS_SetProperty(cx, params, "name", value);
  } else if (value.isObject()) {
    params.set(&value.toObject());
  }

  switch (identifier) {
    case CryptoAlgorithmIdentifier::RSAES_PKCS1_v1_5:{
      return std::make_unique<CryptoAlgorithmRSAES_PKCS1_v1_5>(std::nullopt, std::nullopt);
      break;
    }
    case CryptoAlgorithmIdentifier::RSASSA_PKCS1_v1_5:
    case CryptoAlgorithmIdentifier::RSA_PSS:
    case CryptoAlgorithmIdentifier::RSA_OAEP: {
      JS::Rooted<JS::Value> hash_val(cx);
      if (!JS_GetProperty(cx, params, "hash", &hash_val)) {
        return nullptr;
      }
      auto hashIdentifier = toHashIdentifier(cx, hash_val);
      if (hashIdentifier.isErr()) {
        return nullptr;
      }  

      if (identifier == CryptoAlgorithmIdentifier::RSASSA_PKCS1_v1_5) {
        return std::make_unique<CryptoAlgorithmRSASSA_PKCS1_v1_5_Import>(hashIdentifier.unwrap());
      } else if (identifier == CryptoAlgorithmIdentifier::RSA_PSS) {
        return std::make_unique<CryptoAlgorithmRSA_PSS_KeyImport>(hashIdentifier.unwrap());
      } else if (identifier == CryptoAlgorithmIdentifier::RSA_OAEP) {
        return std::make_unique<CryptoAlgorithmRSA_OAEP_KeyImport>(hashIdentifier.unwrap());
      }
      break;
    }
    case CryptoAlgorithmIdentifier::AES_CTR:
      return std::make_unique<CryptoAlgorithmAES_CTR_KeyImport>();
    case CryptoAlgorithmIdentifier::AES_CBC:
      return std::make_unique<CryptoAlgorithmAES_CBC_KeyImport>();
    case CryptoAlgorithmIdentifier::AES_GCM:
      return std::make_unique<CryptoAlgorithmAES_GCM_KeyImport>();
    case CryptoAlgorithmIdentifier::AES_CFB:
      return std::make_unique<CryptoAlgorithmAES_CFB_KeyImport>();
    case CryptoAlgorithmIdentifier::AES_KW:
      return std::make_unique<CryptoAlgorithmAES_KWImport>();
    case CryptoAlgorithmIdentifier::HMAC: {
      JS::Rooted<JS::Value> hash_val(cx);
      if (!JS_GetProperty(cx, params, "hash", &hash_val)) {
        return nullptr;
      }
      auto hashIdentifier = toHashIdentifier(cx, hash_val);
      if (hashIdentifier.isErr()) {
        return nullptr;
      }
      bool found;
      unsigned long length;
      if (!JS_HasProperty(cx, params, "length", &found)) {
        return nullptr;
      }
      if (found) {
        JS::Rooted<JS::Value> length_val(cx);
        if (!JS_GetProperty(cx, params, "length", &length_val)) {
          return nullptr;
        }
        if (!length_val.isNumber()) {
          return nullptr;
        }
        length = length_val.toNumber();
        return std::make_unique<CryptoAlgorithmHMAC>(hashIdentifier.unwrap(), length);
      } else {
        return std::make_unique<CryptoAlgorithmHMAC>(hashIdentifier.unwrap());
      }
    }
    case CryptoAlgorithmIdentifier::ECDSA:
    case CryptoAlgorithmIdentifier::ECDH: {
      JS::Rooted<JS::Value> namedCurve_val(cx);
      if (!JS_GetProperty(cx, params, "namedCurve", &namedCurve_val)) {
        return nullptr;
      }
      size_t length;
      auto namedCurveChars = encode(cx, namedCurve_val, &length);
      
      if (!namedCurveChars) {
        return nullptr;
      } 

      // TODO finish -- maybe need to split CryptoAlgorithmECDSAKey and CryptoAlgorithmECDHKey
      if (identifier == CryptoAlgorithmIdentifier::ECDSA) {
        //  TODO: need to parse keydata etc
        // return std::make_unique<CryptoAlgorithmECDSAKey>(namedCurveChars.get());
      } else {
        //  TODO: need to parse keydata etc
        // return std::make_unique<CryptoAlgorithmECDHKey>(namedCurveChars.get());
      }

      break;
    }
    case CryptoAlgorithmIdentifier::HKDF: {
      JS::Rooted<JS::Value> salt_val(cx);
      if (!JS_GetProperty(cx, params, "salt", &salt_val)) {
        return nullptr;
      }
      std::optional<std::span<uint8_t>>salt = value_to_buffer(cx, salt_val, "normaliseAlgorithm 11");
      if (!salt.has_value()) {
        // value_to_buffer would have already created a JS exception so we don't need to create one ourselves.
        return nullptr;
      }
      return std::make_unique<CryptoAlgorithmHKDFImport>(salt.value());
    }
    case CryptoAlgorithmIdentifier::PBKDF2: {
      JS::Rooted<JS::Value> hash_val(cx);
      if (!JS_GetProperty(cx, params, "hash", &hash_val)) {
        return nullptr;
      }
      auto hashIdentifier = toHashIdentifier(cx, hash_val);
      if (hashIdentifier.isErr()) {
        return nullptr;
      }
      JS::Rooted<JS::Value> salt_val(cx);
      if (!JS_GetProperty(cx, params, "salt", &salt_val)) {
        return nullptr;
      }
      std::optional<std::span<uint8_t>>salt = value_to_buffer(cx, salt_val, "normaliseAlgorithm 12");
      if (!salt.has_value()) {
        // value_to_buffer would have already created a JS exception so we don't need to create one ourselves.
        return nullptr;
      }
      JS::Rooted<JS::Value> iterations_val(cx);
      if (!JS_GetProperty(cx, params, "iterations", &iterations_val)) {
        return nullptr;
      }
      if (!iterations_val.isNumber()) {
        return nullptr;
      }
      return std::make_unique<CryptoAlgorithmPBKDF2Key>(hashIdentifier.unwrap(), salt.value(), iterations_val.toNumber());
    }
    default: {
      MOZ_ASSERT_UNREACHABLE("CryptoAlgorithmImportKey::normalize unreachable reached.");
    }
  }
  return nullptr;


};
std::unique_ptr<CryptoAlgorithmWrapUnwrapkey> CryptoAlgorithmWrapUnwrapkey::normalize(JSContext* cx, JS::HandleValue value){
  auto identifierResult =  normalizeIdentifier(cx, value);
  if (identifierResult.isErr()) {
    return nullptr;
  }
  auto identifier = identifierResult.unwrap();
  JS::Rooted<JSObject *> params(cx);

  // The value can either be a JS String or a JS Object with a 'name' property
  if (value.isString()) {
    auto obj = JS_NewPlainObject(cx);
    params.set(obj);
    JS_SetProperty(cx, params, "name", value);
  } else if (value.isObject()) {
    params.set(&value.toObject());
  }

  switch (identifier) {
    case CryptoAlgorithmIdentifier::AES_KW: {
      unsigned short length;
      JS::Rooted<JS::Value> length_val(cx);
      if (!JS_GetProperty(cx, params, "length", &length_val)) {
        return nullptr;
      }
      if (!length_val.isNumber()) {
        return nullptr;
      }
      length = length_val.toNumber();
      return std::make_unique<CryptoAlgorithmAES_KW>(length);
    }
    default:
      return nullptr;
  }
};
std::unique_ptr<CryptoAlgorithmGetKeyLength> CryptoAlgorithmGetKeyLength::normalize(JSContext* cx, JS::HandleValue value){
  auto identifierResult =  normalizeIdentifier(cx, value);
  if (identifierResult.isErr()) {
    return nullptr;
  }
  auto identifier = identifierResult.unwrap();
  JS::Rooted<JSObject *> params(cx);

  // The value can either be a JS String or a JS Object with a 'name' property
  if (value.isString()) {
    auto obj = JS_NewPlainObject(cx);
    params.set(obj);
    JS_SetProperty(cx, params, "name", value);
  } else if (value.isObject()) {
    params.set(&value.toObject());
  }

  switch (identifier) {
    case CryptoAlgorithmIdentifier::AES_CTR:
    case CryptoAlgorithmIdentifier::AES_CBC:
    case CryptoAlgorithmIdentifier::AES_GCM:
    case CryptoAlgorithmIdentifier::AES_CFB:
    case CryptoAlgorithmIdentifier::AES_KW: {
      std::string name;
      unsigned short length;
      JS::Rooted<JS::Value> length_val(cx);
      if (!JS_GetProperty(cx, params, "length", &length_val)) {
        return nullptr;
      }
      if (!length_val.isNumber()) {
        return nullptr;
      }
      length = length_val.toNumber();
      if (identifier == CryptoAlgorithmIdentifier::AES_CTR){
        return std::make_unique<CryptoAlgorithmAES_CTR_Key>(length);
      } else if (identifier == CryptoAlgorithmIdentifier::AES_CBC){
        return std::make_unique<CryptoAlgorithmAES_CBC_Key>(length);
      } else if (identifier == CryptoAlgorithmIdentifier::AES_GCM){
        return std::make_unique<CryptoAlgorithmAES_GCM_Key>(length);
      } else if (identifier == CryptoAlgorithmIdentifier::AES_CFB){
        return std::make_unique<CryptoAlgorithmAES_CFB_Key>(length);
      } else if (identifier == CryptoAlgorithmIdentifier::AES_KW) {
        return std::make_unique<CryptoAlgorithmAES_KW>(length);
      }
      break;
    }
    case CryptoAlgorithmIdentifier::HMAC: {
      JS::Rooted<JS::Value> hash_val(cx);
      if (!JS_GetProperty(cx, params, "hash", &hash_val)) {
        return nullptr;
      }
      auto hashIdentifier = toHashIdentifier(cx, hash_val);
      if (hashIdentifier.isErr()) {
        return nullptr;
      }
      bool found;
      if (!JS_HasProperty(cx, params, "length", &found)) {
        return nullptr;
      }
      if (found) {
        JS::Rooted<JS::Value> length_val(cx);
        if (!JS_GetProperty(cx, params, "length", &length_val)) {
          return nullptr;
        }
        if (!length_val.isNumber()) {
          return nullptr;
        }
        size_t length = length_val.toNumber();
        return std::make_unique<CryptoAlgorithmHMAC>(hashIdentifier.unwrap(), length);
      }
      return std::make_unique<CryptoAlgorithmHMAC>(hashIdentifier.unwrap());
      break;
    }
    case CryptoAlgorithmIdentifier::HKDF:
      return std::make_unique<CryptoAlgorithmHKDF>();
    case CryptoAlgorithmIdentifier::PBKDF2:
      return std::make_unique<CryptoAlgorithmPBKDF2>();
    default:
      MOZ_ASSERT_UNREACHABLE("CryptoAlgorithmGetKeyLength::normalize unreachable reached.");
  }
  return nullptr;

};


JSObject* CryptoAlgorithmRSAES_PKCS1_v1_5::generateKey(JSContext* cx, bool extractable, CryptoKeyUsageBitmap usage) {
  JSObject *key_pair = nullptr;
  // TODO: Return a Object { privateKey: CryptoKey, publicKey: CryptoKey }
  return key_pair;
};

JSObject* CryptoAlgorithmRSA_PSS_Key::generateKey(JSContext* cx, bool extractable, CryptoKeyUsageBitmap usage) {
  JSObject *key_pair = nullptr;
  // TODO: Return a Object { privateKey: CryptoKey, publicKey: CryptoKey }
  return key_pair;
}

JSObject* CryptoAlgorithmRSA_OAEP_Key::generateKey(JSContext* cx, bool extractable, CryptoKeyUsageBitmap usage) {
  JSObject *key_pair = nullptr;
  // TODO: Return a Object { privateKey: CryptoKey, publicKey: CryptoKey }
  return key_pair;
}

JSObject* CryptoAlgorithmRSA_OAEP_Key::toObject(JSContext *cx) {
  JS::RootedObject object (cx, JS_NewPlainObject(cx));
  
  auto alg_name = JS_NewStringCopyZ(cx, this->name());
  if (!alg_name) {
    return nullptr;
  }
  JS::RootedValue name_val(cx, JS::StringValue(alg_name));
  if (!JS_SetProperty(cx, object, "name", name_val)) {
    return nullptr;
  }
  JS::RootedValue modulusLength_val(cx, JS::NumberValue(this->modulusLength));
  if (!JS_SetProperty(cx, object, "modulusLength", modulusLength_val)) {
    return nullptr;
  }
  JS::RootedObject hash(cx, JS_NewObject(cx, nullptr));

  auto hash_name = JS_NewStringCopyZ(cx, algorithmName(this->hashIdentifier));
  if (!hash_name) {
    return nullptr;
  }
  JS::RootedValue hash_name_val(cx, JS::StringValue(hash_name));
  if (!JS_SetProperty(cx, hash, "name", hash_name_val)) {
    return nullptr;
  }
  JS::RootedValue hash_val(cx, JS::ObjectValue(*hash));
  if (!JS_SetProperty(cx, object, "hash", hash_val)) {
    return nullptr;
  }
  return object;
}

JSObject* CryptoAlgorithmECDSAGenKey::generateKey(JSContext* cx, bool extractable, CryptoKeyUsageBitmap usage) {
  JSObject *key_pair = nullptr;
  // TODO: Return a Object { privateKey: CryptoKey, publicKey: CryptoKey }
  return key_pair;
}
JSObject* CryptoAlgorithmECDHGenKey::generateKey(JSContext* cx, bool extractable, CryptoKeyUsageBitmap usage) {
  JSObject *key_pair = nullptr;
  // TODO: Return a Object { privateKey: CryptoKey, publicKey: CryptoKey }
  return key_pair;
}

JSObject* CryptoAlgorithmHMAC::generateKey(JSContext* cx, bool extractable, CryptoKeyUsageBitmap usage) {
  if (usage & (CryptoKeyUsageEncrypt | CryptoKeyUsageDecrypt | CryptoKeyUsageDeriveKey | CryptoKeyUsageDeriveBits | CryptoKeyUsageWrapKey | CryptoKeyUsageUnwrapKey)) {
    JS_ReportErrorLatin1(cx, "Cannot create a HMAC key using the specified key usages - HMAC keys can only have 'sign' and 'verify' as usages");
    return nullptr;
  }

  // Returns a CryptoKey
  auto length = this->length / 8;
  uint8_t* data = reinterpret_cast<uint8_t*>(calloc(length, sizeof(uint8_t)));
  if (RAND_bytes(data, length) != 1) {
    return nullptr;
  }
  return CryptoKey::create(cx, static_cast<CryptoAlgorithmGenerateKey*>(this), CryptoKeyType::Secret, extractable, usage, data);
}
JSObject* CryptoAlgorithmAES_CBC_Key::generateKey(JSContext* cx, bool extractable, CryptoKeyUsageBitmap usage) {
  JSObject* key = nullptr;
  // Returns a CryptoKey
  return key;
}
JSObject* CryptoAlgorithmAES_CTR_Key::generateKey(JSContext* cx, bool extractable, CryptoKeyUsageBitmap usage) {
  JSObject* key = nullptr;
  // Returns a CryptoKey
  return key;
}
JSObject* CryptoAlgorithmAES_GCM_Key::generateKey(JSContext* cx, bool extractable, CryptoKeyUsageBitmap usage) {
  JSObject* key = nullptr;
  // Returns a CryptoKey
  return key;
}
JSObject* CryptoAlgorithmAES_KW::generateKey(JSContext* cx, bool extractable, CryptoKeyUsageBitmap usage) {
  JSObject* key = nullptr;
  // Returns a CryptoKey
  return key;
}

namespace {

std::vector<std::string> usages(JSObject*key) {
  MOZ_ASSERT(CryptoKey::is_instance(key));
  auto usage = JS::GetReservedSlot(key, CryptoKey::Slots::Usage).toInt32();
  // The result is ordered alphabetically.
  std::vector<std::string> usages {};
  if (usage & CryptoKeyUsageDecrypt) {
    usages.push_back("decrypt");
  }
  if (usage & CryptoKeyUsageDeriveBits) {
    usages.push_back("deriveBits");
  }
  if (usage & CryptoKeyUsageDeriveKey) {
    usages.push_back("deriveKey");
  }
  if (usage & CryptoKeyUsageEncrypt) {
    usages.push_back("encrypt");
  }
  if (usage & CryptoKeyUsageSign) {
    usages.push_back("sign");
  }
  if (usage & CryptoKeyUsageUnwrapKey) {
    usages.push_back("unwrapKey");
  }
  if (usage & CryptoKeyUsageVerify) {
    usages.push_back("verify");
  }
  if (usage & CryptoKeyUsageWrapKey) {
    usages.push_back("wrapKey");
  }
  return usages;
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#pragma clang diagnostic ignored "-Wformat-security"
#pragma clang diagnostic ignored "-Wunused-value"
JS::Result<CryptoKeyRSAComponents> exportData(JSObject*key) {
  MOZ_ASSERT(CryptoKey::is_instance(key));
  auto rsa = EVP_PKEY_get0_RSA(CryptoKey::key(key));
  if (!rsa) {
    return JS::Result<CryptoKeyRSAComponents>(JS::Error());
  }

  const BIGNUM* n;
  const BIGNUM* e;
  const BIGNUM* d;
  RSA_get0_key(rsa, &n, &e, &d);
  switch (CryptoKey::type(key)) {
    case CryptoKeyType::Public:
        // We need the public modulus and exponent for the public key.
        if (!n || !e) {
          return JS::Result<CryptoKeyRSAComponents>(JS::Error());
        }
        return CryptoKeyRSAComponents::createPublic(convertToBytes(n), convertToBytes(e));
    case CryptoKeyType::Private: {
        // We need the public modulus, exponent, and private exponent, as well as p and q prime information.
        const BIGNUM* p;
        const BIGNUM* q;
        RSA_get0_factors(rsa, &p, &q);

        if (!n || !e || !d || !p || !q) {
          return JS::Result<CryptoKeyRSAComponents>(JS::Error());
        }

        PrimeInfo firstPrimeInfo {convertToBytes(p)};

        PrimeInfo secondPrimeInfo {convertToBytes(q)};

        auto context = BN_CTX_new();

        const BIGNUM* dmp1;
        const BIGNUM* dmq1;
        const BIGNUM* iqmp;
        RSA_get0_crt_params(rsa, &dmp1, &dmq1, &iqmp);

        // dmp1 -- d mod (p - 1)
        if (dmp1) {
          firstPrimeInfo.factorCRTExponent = convertToBytes(dmp1);
        } else {
            auto dmp1New = BN_new();
            auto pm1 = BN_dup(p);
            if (BN_sub_word(pm1, 1) == 1 && BN_mod(dmp1New, d, pm1, context) == 1)
                firstPrimeInfo.factorCRTExponent = convertToBytes(dmp1New);
        }

        // dmq1 -- d mod (q - 1)
        if (dmq1) {
          secondPrimeInfo.factorCRTExponent = convertToBytes(dmq1);
        }else {
            auto dmq1New = BN_new();
            auto qm1 = BN_dup(q);
            if (BN_sub_word(qm1, 1) == 1 && BN_mod(dmq1New, d, qm1, context) == 1)
                secondPrimeInfo.factorCRTExponent = convertToBytes(dmq1New);
        }

        // iqmp -- q^(-1) mod p
        if (iqmp) {
          secondPrimeInfo.factorCRTCoefficient = convertToBytes(iqmp);
        } else {
            auto iqmpNew = BN_mod_inverse(nullptr, q, p, context);
            if (iqmpNew)
                secondPrimeInfo.factorCRTCoefficient = convertToBytes(iqmpNew);
        }

        return CryptoKeyRSAComponents::createPrivateWithAdditionalData(
            convertToBytes(n), convertToBytes(e), convertToBytes(d),
            firstPrimeInfo, secondPrimeInfo, std::vector<PrimeInfo> {});
    }
    default: {
      MOZ_ASSERT_UNREACHABLE("error");
      return JS::Result<CryptoKeyRSAComponents>(JS::Error());
    }
  }
}
#pragma clang diagnostic pop

std::string base64url_encode(std::string_view data) {
  auto result = GlobalProperties::forgivingBase64Encode(data, GlobalProperties::base64URLEncodeTable);
  if (result.ends_with("==")) {
    result.erase(result.length() - 2);
  } else if (result.ends_with("=")) {
    result.erase(result.length() - 1);
  }
  return result;
}

std::unique_ptr<JsonWebKey> exportJWK(JSContext* cx, JS::HandleObject key) {
  MOZ_ASSERT(cx);
  MOZ_ASSERT(CryptoKey::is_instance(key));
  std::string kty = "RSA";
  auto key_ops = usages(key);
  auto ext = JS::GetReservedSlot(key, CryptoKey::Slots::Extractable).toBoolean();

  auto rsaComponentsResult = exportData(key);

  if (rsaComponentsResult.isErr()) {
    return nullptr;
  }
  auto rsaComponents = rsaComponentsResult.unwrap();

  // public key
  auto n = base64url_encode(rsaComponents.modulus());
  auto e = base64url_encode(rsaComponents.exponent());
  if (rsaComponents.type() == CryptoKeyRSAComponents::Type::Public) {
    return std::make_unique<JsonWebKey>(kty, key_ops, ext, n, e);
  }

  // private key
  auto d = base64url_encode(rsaComponents.privateExponent());
  if (!rsaComponents.hasAdditionalPrivateKeyParameters()) {
    return std::make_unique<JsonWebKey>(kty, key_ops, ext, n, e, d);
  }

  auto p = base64url_encode(rsaComponents.firstPrimeInfo()->primeFactor);
  auto q = base64url_encode(rsaComponents.secondPrimeInfo()->primeFactor);
  auto dp = base64url_encode(rsaComponents.firstPrimeInfo()->factorCRTExponent);
  auto dq = base64url_encode(rsaComponents.secondPrimeInfo()->factorCRTExponent);
  auto qi = base64url_encode(rsaComponents.secondPrimeInfo()->factorCRTCoefficient);
  if (rsaComponents.otherPrimeInfos().size() == 0) {
    return std::make_unique<JsonWebKey>(kty, key_ops, ext, n, e, d, p, q, dp, dq, qi);
  }

  // std::vector<RsaOtherPrimesInfo> oth;
  // for (const auto& info : rsaComponents.otherPrimeInfos()) {
  //     RsaOtherPrimesInfo otherInfo;
  //     otherInfo.r = base64url_encode(info.primeFactor);
  //     otherInfo.d = base64url_encode(info.factorCRTExponent);
  //     otherInfo.t = base64url_encode(info.factorCRTCoefficient);
  //     oth.append(otherInfo);
  // }
  return nullptr;
}

// dictionary JsonWebKey {
//   // The following fields are defined in Section 3.1 of JSON Web Key
//   DOMString kty;
//   DOMString use;
//   sequence<DOMString> key_ops;
//   DOMString alg;

//   // The following fields are defined in JSON Web Key Parameters Registration
//   boolean ext;

//   // The following fields are defined in Section 6 of JSON Web Algorithms
//   DOMString crv;
//   DOMString x;
//   DOMString y;
//   DOMString d;
//   DOMString n;
//   DOMString e;
//   DOMString p;
//   DOMString q;
//   DOMString dp;
//   DOMString dq;
//   DOMString qi;
//   sequence<RsaOtherPrimesInfo> oth;
//   DOMString k;
// }
// dictionary RsaOtherPrimesInfo {
//   // The following fields are defined in Section 6.3.2.7 of JSON Web Algorithms
//   DOMString r;
//   DOMString d;
//   DOMString t;
// }

std::unique_ptr<JsonWebKey> parseJWK(JSContext *cx, JS::HandleValue value, std::string_view required_kty_value) {
  if (!value.isObject()) {
    JS_ReportErrorLatin1(cx, "The provided value is not of type JsonWebKey");
    return nullptr;
  }
  JS::RootedObject object(cx, &value.toObject());

  // TODO: lots of repetition here, let's refactor this?
  //   DOMString kty;
  std::string kty;
  {
    bool has_kty;
    if (!JS_HasProperty(cx, object, "kty", &has_kty)) {
      return nullptr;
    }
    if (!has_kty) {
      JS_ReportErrorLatin1(cx, "The required JWK member 'kty' was missing");
      return nullptr;
    }
    JS::RootedValue kty_val(cx);
    if (!JS_GetProperty(cx, object, "kty", &kty_val)) {
      return nullptr;
    }
    size_t kty_length;
    // Convert into a String following https://tc39.es/ecma262/#sec-tostring
    JS::UniqueChars kty_chars = encode(cx, kty_val, &kty_length);
    if (!kty_chars) {
      return nullptr;
    }
    if (kty_chars.get() != required_kty_value) {
      JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_SUBTLE_CRYPTO_INVALID_JWK_KTY_VALUE,
                              required_kty_value.data());
    }
    kty = std::string(kty_chars.get(), kty_length);
  }

  //   DOMString use;
  std::optional<std::string> use = std::nullopt;
  {
    bool has_use;
    if (!JS_HasProperty(cx, object, "use", &has_use)) {
      return nullptr;
    }
    if (has_use) {
      JS::RootedValue use_val(cx);
      if (!JS_GetProperty(cx, object, "use", &use_val)) {
        return nullptr;
      }
      size_t use_length;
      // Convert into a String following https://tc39.es/ecma262/#sec-tostring
      JS::UniqueChars use_chars = encode(cx, use_val, &use_length);
      if (!use_chars) {
        return nullptr;
      }
      use = std::string(use_chars.get(), use_length);
    }
  }

  //   DOMString alg;
  std::optional<std::string> alg = std::nullopt;
  {
    bool has_alg;
    if (!JS_HasProperty(cx, object, "alg", &has_alg)) {
      return nullptr;
    }
    if (has_alg) {
      JS::RootedValue alg_val(cx);
      if (!JS_GetProperty(cx, object, "alg", &alg_val)) {
        return nullptr;
      }
      size_t alg_length;
      // Convert into a String following https://tc39.es/ecma262/#sec-tostring
      JS::UniqueChars alg_chars = encode(cx, alg_val, &alg_length);
      if (!alg_chars) {
        return nullptr;
      }
      alg = std::string(alg_chars.get(), alg_length);
    }
  }

  //   DOMString crv;
  std::optional<std::string> crv = std::nullopt;
  {
    bool has_crv;
    if (!JS_HasProperty(cx, object, "crv", &has_crv)) {
      return nullptr;
    }
    if (has_crv) {
      JS::RootedValue crv_val(cx);
      if (!JS_GetProperty(cx, object, "crv", &crv_val)) {
        return nullptr;
      }
      size_t crv_length;
      // Convert into a String following https://tc39.es/ecma262/#sec-tostring
      JS::UniqueChars crv_chars = encode(cx, crv_val, &crv_length);
      if (!crv_chars) {
        return nullptr;
      }
      crv = std::string(crv_chars.get(), crv_length);
    }
  }

  //   DOMString x;
  std::optional<std::string> x = std::nullopt;
  {
    bool has_x;
    if (!JS_HasProperty(cx, object, "x", &has_x)) {
      return nullptr;
    }
    if (has_x) {
      JS::RootedValue x_val(cx);
      if (!JS_GetProperty(cx, object, "x", &x_val)) {
        return nullptr;
      }
      size_t x_length;
      // Convert into a String following https://tc39.es/ecma262/#sec-tostring
      JS::UniqueChars x_chars = encode(cx, x_val, &x_length);
      if (!x_chars) {
        return nullptr;
      }
      x = std::string(x_chars.get(), x_length);
    }
  }

  //   DOMString y;
  std::optional<std::string> y = std::nullopt;
  {
    bool has_y;
    if (!JS_HasProperty(cx, object, "y", &has_y)) {
      return nullptr;
    }
    if (has_y) {
      JS::RootedValue y_val(cx);
      if (!JS_GetProperty(cx, object, "y", &y_val)) {
        return nullptr;
      }
      size_t y_length;
      // Convert into a String following https://tc39.es/ecma262/#sec-tostring
      JS::UniqueChars y_chars = encode(cx, y_val, &y_length);
      if (!y_chars) {
        return nullptr;
      }
      y = std::string(y_chars.get(), y_length);
    }
  }

  //   DOMString d;
  std::optional<std::string> d = std::nullopt;
  {
    bool has_d;
    if (!JS_HasProperty(cx, object, "d", &has_d)) {
      return nullptr;
    }
    if (has_d) {
      JS::RootedValue d_val(cx);
      if (!JS_GetProperty(cx, object, "d", &d_val)) {
        return nullptr;
      }
      size_t d_length;
      // Convert into a String following https://tc39.es/ecma262/#sec-tostring
      JS::UniqueChars d_chars = encode(cx, d_val, &d_length);
      if (!d_chars) {
        return nullptr;
      }
      d = std::string(d_chars.get(), d_length);
    }
  }

  //   DOMString n;
  std::optional<std::string> n = std::nullopt;
  {
    bool has_n;
    if (!JS_HasProperty(cx, object, "n", &has_n)) {
      return nullptr;
    }
    if (has_n) {
      JS::RootedValue n_val(cx);
      if (!JS_GetProperty(cx, object, "n", &n_val)) {
        return nullptr;
      }
      size_t n_length;
      // Convert into a String following https://tc39.es/ecma262/#sec-tostring
      JS::UniqueChars n_chars = encode(cx, n_val, &n_length);
      if (!n_chars) {
        return nullptr;
      }
      n = std::string(n_chars.get(), n_length);
    }
  }

  //   DOMString e;
  std::optional<std::string> e = std::nullopt;
  {
    bool has_e;
    if (!JS_HasProperty(cx, object, "e", &has_e)) {
      return nullptr;
    }
    if (has_e) {
      JS::RootedValue e_val(cx);
      if (!JS_GetProperty(cx, object, "e", &e_val)) {
        return nullptr;
      }
      size_t e_length;
      // Convert into a String following https://tc39.es/ecma262/#sec-tostring
      JS::UniqueChars e_chars = encode(cx, e_val, &e_length);
      if (!e_chars) {
        return nullptr;
      }
      e = std::string(e_chars.get(), e_length);
    }
  }

  //   DOMString p;
  std::optional<std::string> p = std::nullopt;
  {
    bool has_p;
    if (!JS_HasProperty(cx, object, "p", &has_p)) {
      return nullptr;
    }
    if (has_p) {
      JS::RootedValue p_val(cx);
      if (!JS_GetProperty(cx, object, "p", &p_val)) {
        return nullptr;
      }
      size_t p_length;
      // Convert into a String following https://tc39.es/ecma262/#sec-tostring
      JS::UniqueChars p_chars = encode(cx, p_val, &p_length);
      if (!p_chars) {
        return nullptr;
      }
      p = std::string(p_chars.get(), p_length);
    }
  }
  
  //   DOMString q;
  std::optional<std::string> q = std::nullopt;
  {
    bool has_q;
    if (!JS_HasProperty(cx, object, "q", &has_q)) {
      return nullptr;
    }
    if (has_q) {
      JS::RootedValue q_val(cx);
      if (!JS_GetProperty(cx, object, "q", &q_val)) {
        return nullptr;
      }
      size_t q_length;
      // Convert into a String following https://tc39.es/ecma262/#sec-tostring
      JS::UniqueChars q_chars = encode(cx, q_val, &q_length);
      if (!q_chars) {
        return nullptr;
      }
      q = std::string(q_chars.get(), q_length);
    }
  }
  
  //   DOMString dp;
  std::optional<std::string> dp = std::nullopt;
  {
    bool has_dp;
    if (!JS_HasProperty(cx, object, "dp", &has_dp)) {
      return nullptr;
    }
    if (has_dp) {
      JS::RootedValue dp_val(cx);
      if (!JS_GetProperty(cx, object, "dp", &dp_val)) {
        return nullptr;
      }
      size_t dp_length;
      // Convert into a String following https://tc39.es/ecma262/#sec-tostring
      JS::UniqueChars dp_chars = encode(cx, dp_val, &dp_length);
      if (!dp_chars) {
        return nullptr;
      }
      dp = std::string(dp_chars.get(), dp_length);
    }
  }

  //   DOMString dq;
  std::optional<std::string> dq = std::nullopt;
  {
    bool has_dq;
    if (!JS_HasProperty(cx, object, "dq", &has_dq)) {
      return nullptr;
    }
    if (has_dq) {
      JS::RootedValue dq_val(cx);
      if (!JS_GetProperty(cx, object, "dq", &dq_val)) {
        return nullptr;
      }
      size_t dq_length;
      // Convert into a String following https://tc39.es/ecma262/#sec-tostring
      JS::UniqueChars dq_chars = encode(cx, dq_val, &dq_length);
      if (!dq_chars) {
        return nullptr;
      }
      dq = std::string(dq_chars.get(), dq_length);
    }
  }

  //   DOMString qi;
  std::optional<std::string> qi = std::nullopt;
  {
    bool has_qi;
    if (!JS_HasProperty(cx, object, "qi", &has_qi)) {
      return nullptr;
    }
    if (has_qi) {
      JS::RootedValue qi_val(cx);
      if (!JS_GetProperty(cx, object, "qi", &qi_val)) {
        return nullptr;
      }
      size_t qi_length;
      // Convert into a String following https://tc39.es/ecma262/#sec-tostring
      JS::UniqueChars qi_chars = encode(cx, qi_val, &qi_length);
      if (!qi_chars) {
        return nullptr;
      }
      qi = std::string(qi_chars.get(), qi_length);
    }
  }
  
  //   DOMString k;
  std::optional<std::string> k = std::nullopt;
  {
    bool has_k;
    if (!JS_HasProperty(cx, object, "k", &has_k)) {
      return nullptr;
    }
    if (has_k) {
      JS::RootedValue k_val(cx);
      if (!JS_GetProperty(cx, object, "k", &k_val)) {
        return nullptr;
      }
      size_t k_length;
      // Convert into a String following https://tc39.es/ecma262/#sec-tostring
      JS::UniqueChars k_chars = encode(cx, k_val, &k_length);
      if (!k_chars) {
        return nullptr;
      }
      k = std::string(k_chars.get(), k_length);
    }
  }

  //   boolean ext;
  std::optional<bool> ext = std::nullopt;
  {
    bool has_ext;
    if (!JS_HasProperty(cx, object, "ext", &has_ext)) {
      return nullptr;
    }
    if (has_ext) {
      JS::RootedValue ext_val(cx);
      if (!JS_GetProperty(cx, object, "ext", &ext_val)) {
        return nullptr;
      }
      ext = JS::ToBoolean(ext_val);
    }
  }
  //   sequence<DOMString> key_ops;
  std::vector<std::string> key_ops;
  {
    bool has_key_ops;
    if (!JS_HasProperty(cx, object, "key_ops", &has_key_ops)) {
      return nullptr;
    }
    if (has_key_ops) {
      JS::RootedValue key_ops_val(cx);
      if (!JS_GetProperty(cx, object, "key_ops", &key_ops_val)) {
        return nullptr;
      }
      bool key_ops_is_array;
      if (!JS::IsArrayObject(cx, key_ops_val, &key_ops_is_array)) {
        return nullptr;
      }
      if (!key_ops_is_array) {
        // TODO: Check if key_ops_val is iterable via Symbol.iterator and if so, convert to a JS Array
        JS_ReportErrorASCII(cx, "Failed to read the 'key_ops' property from 'JsonWebKey': The provided value cannot be converted to a sequence");
        return nullptr;
      }
      uint32_t length;
      JS::RootedObject key_ops_array(cx, &key_ops_val.toObject());
      if (!JS::GetArrayLength(cx, key_ops_array, &length)) {
        return nullptr;
      }
      if (length != 0) {
        // Initialize to the provided array's length.
        // TODO: move to mozilla::Vector so we can catch if this reserve exceeds our memory limits
        key_ops.reserve(length);
        JS::RootedValue op_val(cx);
        for (uint32_t i = 0; i < length; ++i) {
          // We should check for and handle interrupts so we can allow GC to happen whilst we are iterating.
          // TODO: Go through entire codebase and add JS_CheckForInterrupt into code-paths which iterate or are recursive such as the structuredClone function
          // if (!JS_CheckForInterrupt(cx)) {
          //   return nullptr;
          // }

          if (!JS_GetElement(cx, key_ops_array, i, &op_val)) {
            return nullptr;
          }

          size_t op_length;
          auto op_chars = encode(cx, op_val, &op_length);
          if (!op_chars) {
            return nullptr;
          }

          std::string op(op_chars.get(), op_length);

          if (op != "encrypt" && op != "decrypt" && op != "sign" && op != "verify" && op != "deriveKey" && op != "deriveBits" && op != "wrapKey" && op != "unwrapKey") {
            JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_SUBTLE_CRYPTO_INVALID_KEY_USAGES_VALUE);
            return nullptr;
          }

          // No duplicates allowed
          if (std::find(key_ops.begin(), key_ops.end(), op) != key_ops.end()) {
            JS_ReportErrorASCII(cx, "The 'key_ops' member of the JWK dictionary contains duplicate usages");
            return nullptr;
          }

          key_ops.push_back(op);
        }
      }
    }
  }

  // TODO: remaining to implement
  //   sequence<RsaOtherPrimesInfo> oth;
  std::vector<RsaOtherPrimesInfo> oth;
  {
    bool has_oth;
    if (!JS_HasProperty(cx, object, "oth", &has_oth)) {
      return nullptr;
    }
    if (has_oth) {
      JS::RootedValue oth_val(cx);
      if (!JS_GetProperty(cx, object, "oth", &oth_val)) {
        return nullptr;
      }
      bool oth_is_array;
      if (!JS::IsArrayObject(cx, oth_val, &oth_is_array)) {
        return nullptr;
      }
      if (!oth_is_array) {
        // TODO: Check if oth_val is iterable via Symbol.iterator and if so, convert to a JS Array
        JS_ReportErrorASCII(cx, "Failed to read the 'oth' property from 'JsonWebKey': The provided value cannot be converted to a sequence");
        return nullptr;
      }
      uint32_t length;
      JS::RootedObject oth_array(cx, &oth_val.toObject());
      if (!JS::GetArrayLength(cx, oth_array, &length)) {
        return nullptr;
      }
      if (length != 0) {
        // Initialize to the provided array's length.
        // TODO: move to mozilla::Vector so we can catch if this reserve exceeds our memory limits
        oth.reserve(length);
        JS::RootedValue info_val(cx);
        for (uint32_t i = 0; i < length; ++i) {
          // We should check for and handle interrupts so we can allow GC to happen whilst we are iterating.
          // TODO: Go through entire codebase and add JS_CheckForInterrupt into code-paths which iterate or are recursive such as the structuredClone function
          // if (!JS_CheckForInterrupt(cx)) {
          //   return nullptr;
          // }

          if (!JS_GetElement(cx, oth_array, i, &info_val)) {
            return nullptr;
          }

          if (!info_val.isObject()) {
            JS_ReportErrorASCII(cx, "Failed to read the 'oth' property from 'JsonWebKey': The provided value is not of type 'RsaOtherPrimesInfo'");
            return nullptr;
          }
          JS::RootedObject info_obj(cx, &info_val.toObject());
          
          JS::RootedValue r_val(cx);
          if (!JS_GetProperty(cx, info_obj, "r", &r_val)) {
            return nullptr;
          }
          size_t r_length;
          auto r_chars = encode(cx, info_val, &r_length);
          if (!r_chars) {
            JS_ReportErrorASCII(cx, "Failed to read the 'oth' property from 'JsonWebKey': The provided value is not of type 'RsaOtherPrimesInfo'");
            return nullptr;
          }
          std::string r(r_chars.get(), r_length);
          JS::RootedValue d_val(cx);
          if (!JS_GetProperty(cx, info_obj, "d", &d_val)) {
            return nullptr;
          }
          size_t d_length;
          auto d_chars = encode(cx, info_val, &d_length);
          if (!d_chars) {
            JS_ReportErrorASCII(cx, "Failed to read the 'oth' property from 'JsonWebKey': The provided value is not of type 'RsaOtherPrimesInfo'");
            return nullptr;
          }
          std::string d(d_chars.get(), d_length);

          JS::RootedValue t_val(cx);
          if (!JS_GetProperty(cx, info_obj, "t", &t_val)) {
            return nullptr;
          }
          size_t t_length;
          auto t_chars = encode(cx, info_val, &t_length);
          if (!t_chars) {
            JS_ReportErrorASCII(cx, "Failed to read the 'oth' property from 'JsonWebKey': The provided value is not of type 'RsaOtherPrimesInfo'");
            return nullptr;
          }
          std::string t(t_chars.get(), t_length);

          oth.push_back(RsaOtherPrimesInfo(r,d,t));
        }
      }
    }
  }
  return std::make_unique<JsonWebKey>(
    // The following fields are defined in Section 3.1 of JSON Web Key
    kty,
    use,
    key_ops,
    alg,
    ext,
    // The following fields are defined in Section 6 of JSON Web Algorithms
    crv,
    x,
    y,
    n,
    e,
    d,
    p,
    q,
    dp,
    dq,
    qi,
    oth,
    k
  );
}
}

const EVP_MD* createDigestAlgorithm(JSContext* cx, JS::HandleObject key) {
  
  JS::RootedObject alg(cx, CryptoKey::get_algorithm(key));

  JS::RootedValue hash_val(cx);
  JS_GetProperty(cx, alg, "hash", &hash_val);
  JS::RootedObject hash(cx, &hash_val.toObject());
  JS::RootedValue name_val(cx);
  JS_GetProperty(cx, hash, "name", &name_val);
  size_t name_length;
  auto cc = encode(cx, name_val, &name_length);

  std::string_view name(cc.get(), name_length);
  if (name == "SHA-1") {
    return EVP_sha1();
  } else if (name == "SHA-224") {
    return EVP_sha224();
  } else if (name == "SHA-256") {
    return EVP_sha256();
  } else if (name == "SHA-384") {
    return EVP_sha384();
  } else if (name == "SHA-512") {
    return EVP_sha512();
  } else {
    // TODO Rename error to NotSupportedError
    JS_ReportErrorLatin1(cx, "NotSupportedError");
    return nullptr;
  }
}

std::optional<std::span<uint8_t>> createDigest(const EVP_MD* algorithm, std::span<uint8_t> data) {
  size_t digestLength = EVP_MD_size(algorithm);
  if (digestLength <= 0) {
    return std::nullopt;
  }
  uint8_t* digestData = reinterpret_cast<uint8_t*>(calloc(digestLength, sizeof(uint8_t)));
  std::span<uint8_t> digest{digestData, digestLength};

    auto ctx = EVP_MD_CTX_create();
    if (!ctx) {
      return std::nullopt;
    }

    if (EVP_DigestInit_ex(ctx, algorithm, nullptr) != 1) {
      return std::nullopt;
    }

    if (EVP_DigestUpdate(ctx, data.data(), data.size()) != 1) {
      return std::nullopt;
    }

    if (EVP_DigestFinal_ex(ctx, digest.data(), nullptr) != 1) {
      return std::nullopt;
    }

  return digest;
}

JSObject* CryptoAlgorithmRSA_OAEP_KeyImport::importKey(JSContext *cx, CryptoKeyFormat format, KeyData data, bool extractable,
                                  CryptoKeyUsageBitmap usages) {
  JSObject* result = nullptr;
  switch (format) {
  case CryptoKeyFormat::Jwk: {
    auto key = std::get<JsonWebKey*>(data);

    bool isUsagesAllowed = false;
    if (key->d.has_value()) {
      isUsagesAllowed = isUsagesAllowed || !(usages ^ CryptoKeyUsageDecrypt);
      isUsagesAllowed = isUsagesAllowed || !(usages ^ CryptoKeyUsageUnwrapKey);
      isUsagesAllowed =
          isUsagesAllowed || !(usages ^ (CryptoKeyUsageDecrypt | CryptoKeyUsageUnwrapKey));
    } else {
      isUsagesAllowed = isUsagesAllowed || !(usages ^ CryptoKeyUsageEncrypt);
      isUsagesAllowed = isUsagesAllowed || !(usages ^ CryptoKeyUsageWrapKey);
      isUsagesAllowed =
          isUsagesAllowed || !(usages ^ (CryptoKeyUsageEncrypt | CryptoKeyUsageWrapKey));
    }
    isUsagesAllowed = isUsagesAllowed || !usages;
    if (!isUsagesAllowed) {
      // TODO Rename error to SyntaxError
      JS_ReportErrorLatin1(cx, "The JWK 'key_ops' member was inconsistent with that specified by the Web Crypto call. The JWK usage must be a superset of those requested");
      return nullptr;
    }

    if (usages && key->use.has_value() && key->use != "enc") {
      // TODO Rename error to OperationError
      JS_ReportErrorLatin1(cx, "Operation is not permitted");
      return nullptr;
    }

    bool isMatched = false;
    switch (this->hashIdentifier) {
    case CryptoAlgorithmIdentifier::SHA_1:{
      isMatched = !key->alg.has_value() || key->alg == "RSA-OAEP";
      break;
    }
    case CryptoAlgorithmIdentifier::SHA_224: {
      isMatched = !key->alg.has_value() || key->alg == "RSA-OAEP-224";
      break;
    }
    case CryptoAlgorithmIdentifier::SHA_256: {
      isMatched = !key->alg.has_value() || key->alg == "RSA-OAEP-256";
      break;
    }
    case CryptoAlgorithmIdentifier::SHA_384: {
      isMatched = !key->alg.has_value() || key->alg == "RSA-OAEP-384";
      break;
    }
    case CryptoAlgorithmIdentifier::SHA_512: {
      isMatched = !key->alg.has_value() || key->alg == "RSA-OAEP-512";
      break;
    }
    default:
      break;
    }
    if (!isMatched) {
      // TODO Rename error to OperationError
      JS_ReportErrorLatin1(cx, "Specified hashing algorithm is not supported");
      return nullptr;
    }

    result = CryptoKey::importJwkRsa(cx, this, key, extractable, usages);
    
    break;
  }
  case CryptoKeyFormat::Spki:
  case CryptoKeyFormat::Pkcs8: {
    // TODO: need to implement these at some point
  }
  default: {
    // TODO Rename error to DataError
    JS_ReportErrorLatin1(cx, "Supplied format is not supported");
    return nullptr;
  }
  }
  MOZ_ASSERT(result);
  if (!result) {
    // TODO Rename error to OperationError
    JS_ReportErrorLatin1(cx, "Operation Failed");
    return nullptr;
  }

  return result;
}

JSObject* CryptoAlgorithmRSA_OAEP_KeyImport::importKey(JSContext* cx, CryptoKeyFormat key_format, JS::HandleValue key_data, bool extractable, CryptoKeyUsageBitmap usage) {
  if (key_format == CryptoKeyFormat::Raw) {
    // TODO Rename error to OperationError
    JS_ReportErrorLatin1(cx, "Operation not supported");
    return nullptr;
  }

  KeyData data;
  if (key_format == CryptoKeyFormat::Jwk) {
    auto jwk = parseJWK(cx, key_data, "RSA");
    if (!jwk) {
      // TODO Rename error to DataError
      JS_ReportErrorLatin1(cx, "Failed for parse the provided JSON Web Key");
      return nullptr;
    }
    data = jwk.release();
  } else {
    std::optional<std::span<uint8_t>> buffer = value_to_buffer(cx, key_data, "");
    if (!buffer.has_value()) {
      // value_to_buffer would have already created a JS exception so we don't need to create one ourselves.
      return nullptr;
    }
    data = buffer.value();
  }
  return this->importKey(cx, key_format, data, extractable, usage);
}

JSObject* CryptoAlgorithmRSASSA_PKCS1_v1_5_Import::importKey(JSContext* cx, CryptoKeyFormat format, KeyData data, bool extractable, CryptoKeyUsageBitmap usages) {
  MOZ_ASSERT(cx);
  JSObject* result = nullptr;
  switch (format) {
  case CryptoKeyFormat::Jwk: {
    auto key = std::get<JsonWebKey*>(data);

    bool isUsagesAllowed = false;
    // public key
    if (key->d.has_value()) {
      isUsagesAllowed = isUsagesAllowed || !(usages ^ CryptoKeyUsageSign);
    } else {
      //private key
      isUsagesAllowed = isUsagesAllowed || !(usages ^ CryptoKeyUsageVerify);
    }
    isUsagesAllowed = isUsagesAllowed || !usages;
    if (!isUsagesAllowed) {
      // TODO Rename error to SyntaxError
      JS_ReportErrorLatin1(cx, "The JWK 'key_ops' member was inconsistent with that specified by the Web Crypto call. The JWK usage must be a superset of those requested");
      return nullptr;
    }

    if (usages && key->use.has_value() && key->use != "enc") {
      // TODO Rename error to OperationError
      JS_ReportErrorLatin1(cx, "Operation not permitted");
      return nullptr;
    }

    bool isMatched = false;
    switch (this->hashIdentifier) {
    case CryptoAlgorithmIdentifier::SHA_1:{
      isMatched = !key->alg.has_value() || key->alg == "RS1";
      break;
    }
    case CryptoAlgorithmIdentifier::SHA_224: {
      isMatched = !key->alg.has_value() || key->alg == "RS224";
      break;
    }
    case CryptoAlgorithmIdentifier::SHA_256: {
      isMatched = !key->alg.has_value() || key->alg == "RS256";
      break;
    }
    case CryptoAlgorithmIdentifier::SHA_384: {
      isMatched = !key->alg.has_value() || key->alg == "RS384";
      break;
    }
    case CryptoAlgorithmIdentifier::SHA_512: {
      isMatched = !key->alg.has_value() || key->alg == "RS512";
      break;
    }
    default: {
      break;
    }
    }
    if (!isMatched) {
      JS_ReportErrorLatin1(cx, "The JWK 'alg' member was inconsistent with that specified by the Web Crypto call");
      return nullptr;
    }
    result = CryptoKey::importJwkRsa(cx, this, key, extractable, usages);
    if (!result) {
      return nullptr;
    }
    break;
  }
  case CryptoKeyFormat::Spki:
  case CryptoKeyFormat::Pkcs8: {
    // TODO: Add implementations for these
  }
  default: {
    // TODO Rename error to DataError
    JS_ReportErrorLatin1(cx, "Supplied format is not supported");
    return nullptr;
  }
  }
  MOZ_ASSERT(result);
  if (!result) {
    // TODO Rename error to OperationError
    JS_ReportErrorLatin1(cx, "Operation Failed");
    return nullptr;
  }
  MOZ_ASSERT(result);
  return result;
}

JSObject* CryptoAlgorithmRSASSA_PKCS1_v1_5_Import::importKey(JSContext* cx, CryptoKeyFormat key_format, JS::HandleValue key_data, bool extractable, CryptoKeyUsageBitmap usage) {
  MOZ_ASSERT(cx);
  if (key_format == CryptoKeyFormat::Raw) {
    MOZ_ASSERT_UNREACHABLE("coding error");
    return nullptr;
  }

  KeyData data;
  if (key_format == CryptoKeyFormat::Jwk) {
    auto jwk = parseJWK(cx, key_data, "RSA");
    if (!jwk) {
      return nullptr;
    }
    data = jwk.release();
  } else {
    std::optional<std::span<uint8_t>> buffer = value_to_buffer(cx, key_data, "");
    if (!buffer.has_value()) {
      // value_to_buffer would have already created a JS exception so we don't need to create one ourselves.
      return nullptr;
    }
    data = buffer.value();
  }
  return this->importKey(cx, key_format, data, extractable, usage);
}

JSObject* CryptoAlgorithmRSASSA_PKCS1_v1_5::sign(JSContext* cx, JS::HandleObject key, std::span<uint8_t> data) {
  MOZ_ASSERT(CryptoKey::is_instance(key));
  if (CryptoKey::type(key) != CryptoKeyType::Private) {
    // TODO Rename error to InvalidAccessError
    JS_ReportErrorLatin1(cx, "InvalidAccessError");
    return nullptr;
  }

  const EVP_MD* algorithm = createDigestAlgorithm(cx, key);
  if (!algorithm) {
    // TODO Rename error to OperationError
    JS_ReportErrorLatin1(cx, "OperationError");
    return nullptr;
  }

  auto digestOption = createDigest(algorithm, data);
  if (!digestOption.has_value()) {
    // TODO Rename error to OperationError
    JS_ReportErrorLatin1(cx, "OperationError");
    return nullptr;
  }
  auto digest = digestOption.value();

  // sign the digest
  {
    auto ctx = EVP_PKEY_CTX_new(CryptoKey::key(key), nullptr);
    if (!ctx) {
      // TODO Rename error to OperationError
      JS_ReportErrorLatin1(cx, "OperationError");
      return nullptr;
    }

    if (EVP_PKEY_sign_init(ctx) <= 0) {
      // TODO Rename error to OperationError
      JS_ReportErrorLatin1(cx, "OperationError");
      return nullptr;
    }

    if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_PADDING) <= 0) {
      // TODO Rename error to OperationError
      JS_ReportErrorLatin1(cx, "OperationError");
      return nullptr;
    }

    if (EVP_PKEY_CTX_set_signature_md(ctx, algorithm) <= 0) {
      // TODO Rename error to OperationError
      JS_ReportErrorLatin1(cx, "OperationError");
      return nullptr;
    }

    size_t signature_length;
    if (EVP_PKEY_sign(ctx, nullptr, &signature_length, digest.data(), digest.size()) <= 0) {
      // TODO Rename error to OperationError
      JS_ReportErrorLatin1(cx, "OperationError");
      return nullptr;
    }

    uint8_t* signature = reinterpret_cast<uint8_t*>(calloc(signature_length, sizeof(uint8_t)));
    if (EVP_PKEY_sign(ctx, signature, &signature_length, digest.data(), digest.size()) <= 0) {
      // TODO Rename error to OperationError
      JS_ReportErrorLatin1(cx, "OperationError");
      return nullptr;
    }
    
    std::span data{signature, signature_length};

    JS::RootedObject buffer(cx, JS::NewArrayBufferWithContents(cx, signature_length, signature));
    if (!buffer) {
      // We can be here is the array buffer was too large -- if that was the case then a JSMSG_BAD_ARRAY_LENGTH will have been created.
      // No other failure scenarios in this path will create a JS exception and so we need to create one.
      if (!JS_IsExceptionPending(cx)) {
        // TODO Rename error to InternalError
        JS_ReportErrorLatin1(cx, "InternalError");
      }
      JS_free(cx, signature);
      return nullptr;
    }
    return buffer;
  }
}

JS::Result<bool> CryptoAlgorithmRSASSA_PKCS1_v1_5::verify(JSContext* cx, JS::HandleObject key, std::span<uint8_t> signature, std::span<uint8_t> data) {
  MOZ_ASSERT(CryptoKey::is_instance(key));

  if (CryptoKey::type(key) != CryptoKeyType::Public) {
    // TODO Rename error to InvalidAccessError
    JS_ReportErrorLatin1(cx, "InvalidAccessError");
    return JS::Result<bool>(JS::Error());
  }
  const EVP_MD* algorithm = createDigestAlgorithm(cx, key);

  auto digestOption = createDigest(algorithm, data);
  if (!digestOption.has_value()) {
    // TODO Rename error to OperationError
    JS_ReportErrorLatin1(cx, "OperationError");
    return JS::Result<bool>(JS::Error());
  }

  auto digest = digestOption.value();

  auto ctx = EVP_PKEY_CTX_new(CryptoKey::key(key), nullptr);
  if (!ctx) {
    // TODO Rename error to OperationError
    JS_ReportErrorLatin1(cx, "OperationError");
    return JS::Result<bool>(JS::Error());
  }

  if (EVP_PKEY_verify_init(ctx) != 1) {
    // TODO Rename error to OperationError
    JS_ReportErrorLatin1(cx, "OperationError");
    return JS::Result<bool>(JS::Error());
  }

  if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_PADDING) != 1) {
    // TODO Rename error to OperationError
    JS_ReportErrorLatin1(cx, "OperationError");
    return JS::Result<bool>(JS::Error());
  }

    if (EVP_PKEY_CTX_set_signature_md(ctx, algorithm) != 1) {
    // TODO Rename error to OperationError
    JS_ReportErrorLatin1(cx, "OperationError");
    return JS::Result<bool>(JS::Error());
  }

  return EVP_PKEY_verify(ctx, signature.data(), signature.size(), digest.data(), digest.size()) == 1;
}

CryptoAlgorithmIdentifier getHashForKey(JSContext* cx, JS::HandleObject key) {
  MOZ_ASSERT(CryptoKey::is_instance(key));
  JS::RootedObject alg(cx, CryptoKey::get_algorithm(key));
  JS::RootedValue hash_val(cx);
  JS_GetProperty(cx, alg, "hash", &hash_val);
  JS::RootedObject hash(cx, &hash_val.toObject());
  JS::RootedValue name_val(cx);
  JS_GetProperty(cx, hash, "name", &name_val);
  size_t name_length;
  auto cc = encode(cx, name_val, &name_length);

  std::string_view name(cc.get(), name_length);
  if (name == "SHA-1") {
    return CryptoAlgorithmIdentifier::SHA_1;
  } else if (name == "SHA-224") {
    return CryptoAlgorithmIdentifier::SHA_224;
  } else if (name == "SHA-256") {
    return CryptoAlgorithmIdentifier::SHA_256;
  } else if (name == "SHA-384") {
    return CryptoAlgorithmIdentifier::SHA_384;
  } else if (name == "SHA-512") {
    return CryptoAlgorithmIdentifier::SHA_512;
  } else {
    // TODO Rename error to NotSupportedError
    JS_ReportErrorLatin1(cx, "NotSupportedError");
    MOZ_ASSERT_UNREACHABLE("coding error");
    return CryptoAlgorithmIdentifier::SHA_1;
  }
}

JSObject* CryptoAlgorithmRSASSA_PKCS1_v1_5::exportKey(JSContext* cx, CryptoKeyFormat format, JS::HandleObject key){
  MOZ_ASSERT(CryptoKey::is_instance(key));

  switch (format) {
    case CryptoKeyFormat::Jwk: {
      auto jwk = exportJWK(cx, key);
      if (!jwk) {
        return nullptr;
      }
      auto hashIdentifier = getHashForKey(cx, key);
      switch (hashIdentifier) {
        case CryptoAlgorithmIdentifier::SHA_1:
          jwk->alg = "RS1";
          break;
        case CryptoAlgorithmIdentifier::SHA_224:
          jwk->alg = "RS224";
          break;
        case CryptoAlgorithmIdentifier::SHA_256:
          jwk->alg = "RS256";
          break;
        case CryptoAlgorithmIdentifier::SHA_384:
          jwk->alg = "RS384";
          break;
        case CryptoAlgorithmIdentifier::SHA_512:
          jwk->alg = "RS512";
          break;
        default:{
          MOZ_ASSERT_UNREACHABLE("coding error");
        }
      }
      
      JS::RootedObject result(cx, JS_NewPlainObject(cx));
      // JsonWebKey to plain JS object
      {
        JS::RootedValue value(cx);
        // The following fields are defined in Section 3.1 of JSON Web Key
        value.setString(JS_NewStringCopyN(cx, jwk->kty.c_str(), jwk->kty.length()));
        if (!JS_SetProperty(cx, result, "kty", value)) { return nullptr;}
        if (jwk->alg.has_value()) {
          auto alg = jwk->alg.value();
          value.setString(JS_NewStringCopyN(cx, alg.c_str(), alg.length()));
          if (!JS_SetProperty(cx, result, "alg", value)) { return nullptr;}
        }

        if (jwk->crv.has_value()) {
          auto crv = jwk->crv.value();
          value.setString(JS_NewStringCopyN(cx, crv.c_str(), crv.length()));
          if (!JS_SetProperty(cx, result, "crv", value)) { return nullptr;}
        }
        
        if (jwk->d.has_value()) {
          auto d = jwk->d.value();
          value.setString(JS_NewStringCopyN(cx, d.c_str(), d.length()));
          if (!JS_SetProperty(cx, result, "d", value)) { return nullptr;}
        }

        if (jwk->dp.has_value()) {
          auto dp = jwk->dp.value();
          value.setString(JS_NewStringCopyN(cx, dp.c_str(), dp.length()));
          if (!JS_SetProperty(cx, result, "dp", value)) { return nullptr;}
        }

        if (jwk->dq.has_value()) {
          auto dq = jwk->dq.value();
          value.setString(JS_NewStringCopyN(cx, dq.c_str(), dq.length()));
          if (!JS_SetProperty(cx, result, "dq", value)) { return nullptr;}
        }

        if (jwk->e.has_value()) {
          auto e = jwk->e.value();
          value.setString(JS_NewStringCopyN(cx, e.c_str(), e.length()));
          if (!JS_SetProperty(cx, result, "e", value)) { return nullptr;}
        }

        if (jwk->ext.has_value()) {
          auto ext = jwk->ext.value();
          value.setBoolean(ext);
          if (!JS_SetProperty(cx, result, "ext", value)) { return nullptr;}
        }

        if (jwk->k.has_value()) {
          auto k = jwk->k.value();
          value.setString(JS_NewStringCopyN(cx, k.c_str(), k.length()));
          if (!JS_SetProperty(cx, result, "k", value)) { return nullptr;}
        }

        if (jwk->key_ops.size() > 0) {
          JS::RootedValueVector ops(cx);
          JS::RootedString str(cx);
          for (auto op: jwk->key_ops) {
            if (!(str = JS_NewStringCopyN(cx, op.c_str(), op.length()))) {
              return nullptr;
            }
            if (!ops.append(JS::StringValue(str))) {
              js::ReportOutOfMemory(cx);
              return nullptr;
            }
          }

          auto array = JS::NewArrayObject(cx, ops);
          value.setObject(*array);
          if (!JS_SetProperty(cx, result, "key_ops", value)) { return nullptr;}
        }

        if (jwk->n.has_value()) {
          auto n = jwk->n.value();
          value.setString(JS_NewStringCopyN(cx, n.c_str(), n.length()));
          if (!JS_SetProperty(cx, result, "n", value)) { return nullptr;}
        }
        
        // TODO: Add to the exported key this field: std::optional<std::vector<RsaOtherPrimesInfo>> oth;
        
        if (jwk->p.has_value()) {
          auto p = jwk->p.value();
          value.setString(JS_NewStringCopyN(cx, p.c_str(), p.length()));
          if (!JS_SetProperty(cx, result, "p", value)) { return nullptr;}
        }

        if (jwk->q.has_value()) {
          auto q = jwk->q.value();
          value.setString(JS_NewStringCopyN(cx, q.c_str(), q.length()));
          if (!JS_SetProperty(cx, result, "q", value)) { return nullptr;}
        }
        
        if (jwk->qi.has_value()) {
          auto qi = jwk->qi.value();
          value.setString(JS_NewStringCopyN(cx, qi.c_str(), qi.length()));
          if (!JS_SetProperty(cx, result, "qi", value)) { return nullptr;}
        }

        if (jwk->use.has_value()) {
          auto use = jwk->use.value();
          value.setString(JS_NewStringCopyN(cx, use.c_str(), use.length()));
          if (!JS_SetProperty(cx, result, "use", value)) { return nullptr;}
        }

        if (jwk->x.has_value()) {
          auto x = jwk->x.value();
          value.setString(JS_NewStringCopyN(cx, x.c_str(), x.length()));
          if (!JS_SetProperty(cx, result, "x", value)) { return nullptr;}
        }

        if (jwk->y.has_value()) {
          auto y = jwk->y.value();
          value.setString(JS_NewStringCopyN(cx, y.c_str(), y.length()));
          if (!JS_SetProperty(cx, result, "y", value)) { return nullptr;}
        }
      }

      return result;
      break;
    }
    case CryptoKeyFormat::Spki: {
      JS_ReportErrorLatin1(cx, "Exporting in SubjectPublicKeyInfo format is not yet supported.");
    }
    case CryptoKeyFormat::Pkcs8: {
      JS_ReportErrorLatin1(cx, "Exporting in PKCS #8 format is not yet supported.");
      break;
    }
    case CryptoKeyFormat::Raw: {
      JS_ReportErrorLatin1(cx, "Exporting in Raw format is not yet supported.");
      break;
    }
    default:{
      MOZ_ASSERT_UNREACHABLE("coding error");
      return nullptr;
    }
  }
  return nullptr;
}


JSObject* CryptoAlgorithmHMAC::sign(JSContext* cx, JS::HandleObject key, std::span<uint8_t> data){return nullptr;}
JS::Result<bool> CryptoAlgorithmHMAC::verify(JSContext* cx, JS::HandleObject key, std::span<uint8_t> signature, std::span<uint8_t> data){return JS::Result<bool>(JS::Error());}
JSObject* CryptoAlgorithmHMAC::importKey(JSContext* cx, CryptoKeyFormat, JS::HandleValue, bool extractable, CryptoKeyUsageBitmap){return nullptr;}
JSObject* CryptoAlgorithmHMAC::importKey(JSContext* cx, CryptoKeyFormat, KeyData, bool extractable, CryptoKeyUsageBitmap){return nullptr;}
JSObject* CryptoAlgorithmHMAC::exportKey(JSContext* cx, CryptoKeyFormat format, JS::HandleObject key){return nullptr;}
size_t CryptoAlgorithmHMAC::getKeyLength(JSContext* cx) {
  return this->length;
}

JSObject* CryptoAlgorithmECDSASignVerify::sign(JSContext* cx, JS::HandleObject key, std::span<uint8_t> data){return nullptr;}
JS::Result<bool> CryptoAlgorithmECDSASignVerify::verify(JSContext* cx, JS::HandleObject key, std::span<uint8_t> signature, std::span<uint8_t> data){return JS::Result<bool>(JS::Error());}

JSObject* CryptoAlgorithmRSA_PSS_SignVerify::sign(JSContext* cx, JS::HandleObject key, std::span<uint8_t> data) {return nullptr;};
JS::Result<bool> CryptoAlgorithmRSA_PSS_SignVerify::verify(JSContext* cx, JS::HandleObject key, std::span<uint8_t> signature, std::span<uint8_t> data) {return JS::Result<bool>(JS::Error());}

JSObject* CryptoAlgorithmSHA1::digest(JSContext *cx, std::span<uint8_t> data) {
  auto alg = EVP_sha1();
  unsigned int size;
  auto buf = static_cast<unsigned char *>(JS_malloc(cx, SHA_DIGEST_LENGTH));
  if (!buf) {
    JS_ReportOutOfMemory(cx);
    return nullptr;
  }
  if (!EVP_Digest(data.data(), data.size(), buf, &size, alg, NULL)) {
    JS_ReportErrorUTF8(cx, "SubtleCrypto.digest: failed to create digest");
    return nullptr;
  }
  JS::RootedObject array_buffer(cx);
  array_buffer.set(JS::NewArrayBufferWithContents(cx, size, buf));
  if (!array_buffer) {
    JS_ReportOutOfMemory(cx);
    return nullptr;
  }
  return array_buffer;
}
JSObject* CryptoAlgorithmSHA224::digest(JSContext *cx, std::span<uint8_t> data) {
  auto alg = EVP_sha224();
  unsigned int size;
  auto buf = static_cast<unsigned char *>(JS_malloc(cx, SHA224_DIGEST_LENGTH));
  if (!buf) {
    JS_ReportOutOfMemory(cx);
    return nullptr;
  }
  if (!EVP_Digest(data.data(), data.size(), buf, &size, alg, NULL)) {
    JS_ReportErrorUTF8(cx, "SubtleCrypto.digest: failed to create digest");
    return nullptr;
  }
  JS::RootedObject array_buffer(cx);
  array_buffer.set(JS::NewArrayBufferWithContents(cx, size, buf));
  if (!array_buffer) {
    JS_ReportOutOfMemory(cx);
    return nullptr;
  }
  return array_buffer;
}
JSObject* CryptoAlgorithmSHA256::digest(JSContext *cx, std::span<uint8_t> data) {
  auto alg = EVP_sha256();
  unsigned int size;
  auto buf = static_cast<unsigned char *>(JS_malloc(cx, SHA256_DIGEST_LENGTH));
  if (!buf) {
    JS_ReportOutOfMemory(cx);
    return nullptr;
  }
  if (!EVP_Digest(data.data(), data.size(), buf, &size, alg, NULL)) {
    JS_ReportErrorUTF8(cx, "SubtleCrypto.digest: failed to create digest");
    return nullptr;
  }
  JS::RootedObject array_buffer(cx);
  array_buffer.set(JS::NewArrayBufferWithContents(cx, size, buf));
  if (!array_buffer) {
    JS_ReportOutOfMemory(cx);
    return nullptr;
  }
  return array_buffer;
}
JSObject* CryptoAlgorithmSHA384::digest(JSContext *cx, std::span<uint8_t> data) {
  auto alg = EVP_sha384();
  unsigned int size;
  auto buf = static_cast<unsigned char *>(JS_malloc(cx, SHA384_DIGEST_LENGTH));
  if (!buf) {
    JS_ReportOutOfMemory(cx);
    return nullptr;
  }
  if (!EVP_Digest(data.data(), data.size(), buf, &size, alg, NULL)) {
    JS_ReportErrorUTF8(cx, "SubtleCrypto.digest: failed to create digest");
    return nullptr;
  }
  JS::RootedObject array_buffer(cx);
  array_buffer.set(JS::NewArrayBufferWithContents(cx, size, buf));
  if (!array_buffer) {
    JS_ReportOutOfMemory(cx);
    return nullptr;
  }
  return array_buffer;
}
JSObject* CryptoAlgorithmSHA512::digest(JSContext *cx, std::span<uint8_t> data) {
  auto alg = EVP_sha512();
  unsigned int size;
  auto buf = static_cast<unsigned char *>(JS_malloc(cx, SHA512_DIGEST_LENGTH));
  if (!buf) {
    JS_ReportOutOfMemory(cx);
    return nullptr;
  }
  if (!EVP_Digest(data.data(), data.size(), buf, &size, alg, NULL)) {
    JS_ReportErrorUTF8(cx, "SubtleCrypto.digest: failed to create digest");
    return nullptr;
  }
  JS::RootedObject array_buffer(cx);
  array_buffer.set(JS::NewArrayBufferWithContents(cx, size, buf));
  if (!array_buffer) {
    JS_ReportOutOfMemory(cx);
    return nullptr;
  }
  return array_buffer;
}

JSObject* CryptoAlgorithmRSAES_PKCS1_v1_5::encrypt(JSContext* cx, CryptoKey, std::vector<uint8_t>) {return nullptr;}
JSObject* CryptoAlgorithmRSAES_PKCS1_v1_5::decrypt(JSContext* cx, CryptoKey, std::vector<uint8_t>) {return nullptr;}
JSObject* CryptoAlgorithmRSAES_PKCS1_v1_5::importKey(JSContext* cx, CryptoKeyFormat, JS::HandleValue, bool extractable, CryptoKeyUsageBitmap) {return nullptr;}
JSObject* CryptoAlgorithmRSAES_PKCS1_v1_5::importKey(JSContext* cx, CryptoKeyFormat, KeyData, bool extractable, CryptoKeyUsageBitmap) {return nullptr;}
JSObject* CryptoAlgorithmRSAES_PKCS1_v1_5::exportKey(JSContext* cx, CryptoKeyFormat format, JS::HandleObject key) {return nullptr;}

// Convert the exponent vector to a 32-bit value, if possible.
std::optional<uint32_t> exponentVectorToUInt32(const std::span<uint8_t>& exponent)
{
    if (exponent.size() > 4) {
        if (std::any_of(exponent.begin(), exponent.end() - 4, [](uint8_t element) { return !!element; }))
            return std::nullopt;
    }

    uint32_t result = 0;
    for (size_t size = exponent.size(), i = std::min<size_t>(4, size); i > 0; --i) {
        result <<= 8;
        result += exponent[size - i];
    }

    return result;
}
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#pragma clang diagnostic ignored "-Wformat-security"
#pragma clang diagnostic ignored "-Wunused-value"
JSObject* generatePair(JSContext* cx, CryptoAlgorithmRSASSA_PKCS1_v1_5_KeyGen* algorithm, bool extractable, CryptoKeyUsageBitmap usages) {
  // OpenSSL doesn't report an error if the exponent is smaller than three or even.
  auto e = exponentVectorToUInt32(algorithm->publicExponent);
  if (!e || *e < 3 || !(*e & 0x1)) {
    // TODO Rename error to OperationError
    JS_ReportErrorLatin1(cx, "OperationError");
    return nullptr;
  }

  auto exponent = convertToBigNumber(algorithm->publicExponent);
  if (!exponent) {
    // TODO Rename error to OperationError
    JS_ReportErrorLatin1(cx, "OperationError");
    return nullptr;
  }
  auto privateRSA = RSA_new();
  if (!privateRSA) {
    // TODO Rename error to OperationError
    JS_ReportErrorLatin1(cx, "OperationError");
    return nullptr;
  }
  // RSA_generate_key_ex() returns 1 on success or 0 on error.
  auto result = RSA_generate_key_ex(privateRSA, algorithm->modulusLength, exponent, nullptr);
  if (result != 1) {
    // TODO Rename error to OperationError
    JS_ReportErrorLatin1(cx, "OperationError");
    return nullptr;
  }

  auto publicRSA = RSAPublicKey_dup(privateRSA);
  if (!publicRSA) {
    // TODO Rename error to OperationError
    JS_ReportErrorLatin1(cx, "OperationError");
    return nullptr;
  }

  auto privatePKey = EVP_PKEY_new();
  if (EVP_PKEY_set1_RSA(privatePKey, privateRSA) <= 0) {
    // TODO Rename error to OperationError
    JS_ReportErrorLatin1(cx, "OperationError");
    return nullptr;
  }

  auto publicPKey = EVP_PKEY_new();
  if (EVP_PKEY_set1_RSA(publicPKey, publicRSA) <= 0) {
    // TODO Rename error to OperationError
    JS_ReportErrorLatin1(cx, "OperationError");
    return nullptr;
  }

  JS::RootedObject publicKey(
      cx, JS_NewObjectWithGivenProto(cx, &CryptoKey::class_, CryptoKey::proto_obj));
  if (!publicKey) {
    // TODO Rename error to OperationError
    JS_ReportErrorLatin1(cx, "OperationError");
    return nullptr;
  }

  {
    JS::RootedObject alg(cx, algorithm->toObject(cx));
    if (!alg) {
      // TODO Rename error to OperationError
    JS_ReportErrorLatin1(cx, "OperationError");
      return nullptr;
    }
    JS::RootedValue modulusLength(cx, JS::NumberValue(algorithm->modulusLength));
    if (!JS_SetProperty(cx, alg, "modulusLength", modulusLength)) {
      return nullptr;
    }

    uint8_t* p = reinterpret_cast<uint8_t*>(calloc(algorithm->publicExponent.size(), sizeof(uint8_t)));
    auto exp = algorithm->publicExponent;
    std::copy(exp.begin(), exp.end(), p);

    JS::RootedObject buffer(cx, JS::NewArrayBufferWithContents(cx, algorithm->publicExponent.size(), p));
    if (!buffer) {
      // We can be here is the array buffer was too large -- if that was the case then a JSMSG_BAD_ARRAY_LENGTH will have been created.
      // No other failure scenarios in this path will create a JS exception and so we need to create one.
      if (!JS_IsExceptionPending(cx)) {
        // TODO Rename error to InternalError
        JS_ReportErrorLatin1(cx, "InternalError");
      }
      JS_free(cx, p);
      return nullptr;
    }

    JS::RootedObject byte_array(cx, JS_NewUint8ArrayWithBuffer(cx, buffer, 0, algorithm->publicExponent.size()));
    JS::RootedValue publicExponent(cx, JS::ObjectValue(*byte_array));
    if (!JS_SetProperty(cx, alg, "publicExponent", publicExponent)) {
      return nullptr;
    }

    JS::SetReservedSlot(publicKey, CryptoKey::Slots::Algorithm, JS::ObjectValue(*alg));
    JS::SetReservedSlot(publicKey, CryptoKey::Slots::Type, JS::Int32Value(static_cast<uint8_t>(CryptoKeyType::Public)));
    JS::SetReservedSlot(publicKey, CryptoKey::Slots::Extractable, JS::BooleanValue(true));
    JS::SetReservedSlot(publicKey, CryptoKey::Slots::Usage, JS::Int32Value(usages));
    JS::SetReservedSlot(publicKey, CryptoKey::Slots::Key, JS::PrivateValue(publicPKey));
  }

  JS::RootedObject privateKey(
      cx, JS_NewObjectWithGivenProto(cx, &CryptoKey::class_, CryptoKey::proto_obj));
  if (!privateKey) {
    return nullptr;
  }

  {
    JS::RootedObject alg(cx, algorithm->toObject(cx));
    if (!alg) {
      return nullptr;
    }
    JS::RootedValue modulusLength(cx, JS::NumberValue(algorithm->modulusLength));
    if (!JS_SetProperty(cx, alg, "modulusLength", modulusLength)) {
      return nullptr;
    }

    uint8_t* p = reinterpret_cast<uint8_t*>(calloc(algorithm->publicExponent.size(), sizeof(uint8_t)));
    auto exp = algorithm->publicExponent;
    std::copy(exp.begin(), exp.end(), p);

    JS::RootedObject buffer(cx, JS::NewArrayBufferWithContents(cx, algorithm->publicExponent.size(), p));
    if (!buffer) {
      // We can be here is the array buffer was too large -- if that was the case then a JSMSG_BAD_ARRAY_LENGTH will have been created.
      // No other failure scenarios in this path will create a JS exception and so we need to create one.
      if (!JS_IsExceptionPending(cx)) {
        // TODO Rename error to InternalError
        JS_ReportErrorLatin1(cx, "InternalError");
      }
      JS_free(cx, p);
      return nullptr;
    }

    JS::RootedObject byte_array(cx, JS_NewUint8ArrayWithBuffer(cx, buffer, 0, algorithm->publicExponent.size()));
    JS::RootedValue publicExponent(cx, JS::ObjectValue(*byte_array));
    if (!JS_SetProperty(cx, alg, "publicExponent", publicExponent)) {
      return nullptr;
    }

    JS::SetReservedSlot(privateKey, CryptoKey::Slots::Algorithm, JS::ObjectValue(*alg));
    JS::SetReservedSlot(privateKey, CryptoKey::Slots::Type, JS::Int32Value(static_cast<uint8_t>(CryptoKeyType::Private)));
    JS::SetReservedSlot(privateKey, CryptoKey::Slots::Extractable, JS::BooleanValue(true));
    JS::SetReservedSlot(privateKey, CryptoKey::Slots::Usage, JS::Int32Value(usages));
    JS::SetReservedSlot(privateKey, CryptoKey::Slots::Key, JS::PrivateValue(privatePKey));
  }
  JS::RootedObject cryptoKeyPair(cx, JS_NewPlainObject(cx));
  JS::RootedValue pubKey (cx, JS::ObjectValue(*publicKey));
  if (!JS_SetProperty(cx, cryptoKeyPair, "publicKey", pubKey)) {
    return nullptr;
  }
  JS::RootedValue privKey (cx, JS::ObjectValue(*privateKey));
  if (!JS_SetProperty(cx, cryptoKeyPair, "privateKey", privKey)) {
    return nullptr;
  }
  return cryptoKeyPair;
}
#pragma clang diagnostic pop

JSObject* CryptoAlgorithmRSASSA_PKCS1_v1_5_KeyGen::generateKey(JSContext* cx, bool extractable, CryptoKeyUsageBitmap usages){

  JSObject* result = nullptr;
    
  if (usages & (CryptoKeyUsageDecrypt | CryptoKeyUsageEncrypt | CryptoKeyUsageDeriveKey | CryptoKeyUsageDeriveBits | CryptoKeyUsageWrapKey | CryptoKeyUsageUnwrapKey)) {
    // TODO Rename error to SyntaxError
    JS_ReportErrorLatin1(cx, "SyntaxError");
    return nullptr;
  }
  result = generatePair(cx, this, extractable, usages);
  return result;

}
JSObject* CryptoAlgorithmRSASSA_PKCS1_v1_5_KeyGen::exportKey(JSContext* cx, CryptoKeyFormat format, JS::HandleObject key){return nullptr;}


JSObject* CryptoAlgorithmAES_CFB_Key::generateKey(JSContext* cx, bool extractable, CryptoKeyUsageBitmap) {
  return nullptr;
}
JSObject* CryptoAlgorithmAES_CFB_Key::exportKey(JSContext* cx, CryptoKeyFormat format, JS::HandleObject key){return nullptr;}
size_t CryptoAlgorithmAES_CFB_Key::getKeyLength(JSContext* cx) {
  return this->length;
}

JSObject* CryptoAlgorithmRSASSA_PKCS1_v1_5_Import::exportKey(JSContext* cx, CryptoKeyFormat format, JS::HandleObject key){return nullptr;}

JSObject* CryptoAlgorithmRSA_PSS_KeyImport::importKey(JSContext* cx, CryptoKeyFormat, JS::HandleValue, bool extractable, CryptoKeyUsageBitmap){return nullptr;}
JSObject* CryptoAlgorithmRSA_PSS_KeyImport::importKey(JSContext* cx, CryptoKeyFormat, KeyData, bool extractable, CryptoKeyUsageBitmap){return nullptr;}
JSObject* CryptoAlgorithmRSA_PSS_KeyImport::exportKey(JSContext* cx, CryptoKeyFormat format, JS::HandleObject key){return nullptr;}


JSObject* CryptoAlgorithmAES_CTR_KeyImport::importKey(JSContext* cx, CryptoKeyFormat, JS::HandleValue, bool extractable, CryptoKeyUsageBitmap){return nullptr;}
JSObject* CryptoAlgorithmAES_CTR_KeyImport::importKey(JSContext* cx, CryptoKeyFormat, KeyData, bool extractable, CryptoKeyUsageBitmap){return nullptr;}
JSObject* CryptoAlgorithmAES_CTR_KeyImport::exportKey(JSContext* cx, CryptoKeyFormat format, JS::HandleObject key){return nullptr;}

JSObject* CryptoAlgorithmAES_CBC_KeyImport::importKey(JSContext* cx, CryptoKeyFormat, JS::HandleValue, bool extractable, CryptoKeyUsageBitmap){return nullptr;}
JSObject* CryptoAlgorithmAES_CBC_KeyImport::importKey(JSContext* cx, CryptoKeyFormat, KeyData, bool extractable, CryptoKeyUsageBitmap){return nullptr;}
JSObject* CryptoAlgorithmAES_CBC_KeyImport::exportKey(JSContext* cx, CryptoKeyFormat format, JS::HandleObject key){return nullptr;}

JSObject* CryptoAlgorithmAES_GCM_KeyImport::importKey(JSContext* cx, CryptoKeyFormat, JS::HandleValue, bool extractable, CryptoKeyUsageBitmap){return nullptr;}
JSObject* CryptoAlgorithmAES_GCM_KeyImport::importKey(JSContext* cx, CryptoKeyFormat, KeyData, bool extractable, CryptoKeyUsageBitmap){return nullptr;}
JSObject* CryptoAlgorithmAES_GCM_KeyImport::exportKey(JSContext* cx, CryptoKeyFormat format, JS::HandleObject key){return nullptr;}

JSObject* CryptoAlgorithmAES_CFB_KeyImport::importKey(JSContext* cx, CryptoKeyFormat, JS::HandleValue, bool extractable, CryptoKeyUsageBitmap){return nullptr;}
JSObject* CryptoAlgorithmAES_CFB_KeyImport::importKey(JSContext* cx, CryptoKeyFormat, KeyData, bool extractable, CryptoKeyUsageBitmap){return nullptr;}
JSObject* CryptoAlgorithmAES_CFB_KeyImport::exportKey(JSContext* cx, CryptoKeyFormat format, JS::HandleObject key){return nullptr;}

JSObject* CryptoAlgorithmAES_KWImport::importKey(JSContext* cx, CryptoKeyFormat, JS::HandleValue, bool extractable, CryptoKeyUsageBitmap){return nullptr;}
JSObject* CryptoAlgorithmAES_KWImport::importKey(JSContext* cx, CryptoKeyFormat, KeyData, bool extractable, CryptoKeyUsageBitmap){return nullptr;}
JSObject* CryptoAlgorithmAES_KWImport::exportKey(JSContext* cx, CryptoKeyFormat format, JS::HandleObject key){return nullptr;}

JSObject* CryptoAlgorithmHKDFImport::importKey(JSContext* cx, CryptoKeyFormat, JS::HandleValue, bool extractable, CryptoKeyUsageBitmap){return nullptr;}
JSObject* CryptoAlgorithmHKDFImport::importKey(JSContext* cx, CryptoKeyFormat, KeyData, bool extractable, CryptoKeyUsageBitmap){return nullptr;}

JSObject* CryptoAlgorithmPBKDF2Key::deriveBits(JSContext *cx, CryptoKey, size_t length){return nullptr;}
JSObject* CryptoAlgorithmPBKDF2Key::importKey(JSContext* cx, CryptoKeyFormat, JS::HandleValue, bool extractable, CryptoKeyUsageBitmap){return nullptr;}
JSObject* CryptoAlgorithmPBKDF2Key::importKey(JSContext *cx, CryptoKeyFormat, KeyData, bool extractable, CryptoKeyUsageBitmap){return nullptr;}

size_t CryptoAlgorithmAES_CBC_Key::getKeyLength(JSContext*) {
  return this->length;
};

JSObject* CryptoAlgorithmAES_CTR_Key::importKey(JSContext* cx, CryptoKeyFormat, JS::HandleValue, bool extractable, CryptoKeyUsageBitmap){return nullptr;}
JSObject* CryptoAlgorithmAES_CTR_Key::importKey(JSContext* cx, CryptoKeyFormat, KeyData, bool extractable, CryptoKeyUsageBitmap){return nullptr;}
JSObject* CryptoAlgorithmAES_CTR_Key::exportKey(JSContext* cx, CryptoKeyFormat format, JS::HandleObject key){return nullptr;}
size_t CryptoAlgorithmAES_CTR_Key::getKeyLength(JSContext* cx) {
  return this->length;
}

JSObject* CryptoAlgorithmAES_GCM_Key::exportKey(JSContext* cx, CryptoKeyFormat format, JS::HandleObject key){return nullptr;}
size_t CryptoAlgorithmAES_GCM_Key::getKeyLength(JSContext* cx) {
  return this->length;
}

JSObject* CryptoAlgorithmAES_KW::exportKey(JSContext* cx, CryptoKeyFormat format, JS::HandleObject key){return nullptr;}
JSObject* CryptoAlgorithmAES_KW::wrapKey(JSContext* cx, CryptoKey, std::vector<uint8_t>){return nullptr;}
JSObject* CryptoAlgorithmAES_KW::unwrapKey(JSContext* cx, CryptoKey, std::vector<uint8_t>){return nullptr;}
size_t CryptoAlgorithmAES_KW::getKeyLength(JSContext* cx) {
  return this->length;
}

JSObject* CryptoAlgorithmAES_CBC_Key::toObject(JSContext* cx) {return nullptr;}
JSObject* CryptoAlgorithmAES_CBC_KeyImport::toObject(JSContext* cx) {return nullptr;}
JSObject* CryptoAlgorithmAES_CFB_Key::toObject(JSContext* cx) {return nullptr;}
JSObject* CryptoAlgorithmAES_CFB_KeyImport::toObject(JSContext* cx) {return nullptr;}
JSObject* CryptoAlgorithmAES_CTR_Key::toObject(JSContext* cx) {return nullptr;}
JSObject* CryptoAlgorithmAES_CTR_KeyImport::toObject(JSContext* cx) {return nullptr;}
JSObject* CryptoAlgorithmAES_GCM_Key::toObject(JSContext* cx) {return nullptr;}
JSObject* CryptoAlgorithmAES_GCM_KeyImport::toObject(JSContext* cx) {return nullptr;}
JSObject* CryptoAlgorithmAES_KW::toObject(JSContext* cx) {return nullptr;}
JSObject* CryptoAlgorithmAES_KWImport::toObject(JSContext* cx) {return nullptr;}
JSObject* CryptoAlgorithmECDHGenKey::toObject(JSContext *cx){return nullptr;}
JSObject* CryptoAlgorithmECDHKey::toObject(JSContext *cx){return nullptr;}
JSObject* CryptoAlgorithmECDSAGenKey::toObject(JSContext *cx){return nullptr;}
JSObject* CryptoAlgorithmECDSAKey::toObject(JSContext *cx){return nullptr;}
JSObject* CryptoAlgorithmECDSASignVerify::toObject(JSContext *cx){return nullptr;}
JSObject* CryptoAlgorithmHKDF::toObject(JSContext *cx){return nullptr;}
JSObject* CryptoAlgorithmHKDFDerive::toObject(JSContext *cx){return nullptr;}
JSObject* CryptoAlgorithmHKDFImport::toObject(JSContext *cx){return nullptr;}
JSObject* CryptoAlgorithmHMAC::toObject(JSContext *cx){
  JS::RootedObject object (cx, JS_NewPlainObject(cx));
  
  auto alg_name = JS_NewStringCopyZ(cx, this->name());
  if (!alg_name) {
    return nullptr;
  }
  JS::RootedValue name_val(cx, JS::StringValue(alg_name));
  if (!JS_SetProperty(cx, object, "name", name_val)) {
    return nullptr;
  }
  JS::RootedObject hash(cx, JS_NewObject(cx, nullptr));

  auto hash_name = JS_NewStringCopyZ(cx, algorithmName(this->hashIdentifier));
  if (!hash_name) {
    return nullptr;
  }
  JS::RootedValue hash_name_val(cx, JS::StringValue(hash_name));
  if (!JS_SetProperty(cx, hash, "name", hash_name_val)) {
    return nullptr;
  }
  JS::RootedValue hash_val(cx, JS::ObjectValue(*hash));
  if (!JS_SetProperty(cx, object, "hash", hash_val)) {
    return nullptr;
  }
  JS::RootedValue length_val(cx, JS::NumberValue(this->length));
  if (!JS_SetProperty(cx, object, "length", length_val)) {
    return nullptr;
  }


  return object;
}
JSObject* CryptoAlgorithmPBKDF2::toObject(JSContext *cx){return nullptr;}
JSObject* CryptoAlgorithmPBKDF2Key::toObject(JSContext *cx){return nullptr;}
JSObject* CryptoAlgorithmRSAES_PKCS1_v1_5::toObject(JSContext *cx){return nullptr;}
JSObject* CryptoAlgorithmRSASSA_PKCS1_v1_5::toObject(JSContext *cx){return nullptr;}
JSObject* CryptoAlgorithmRSASSA_PKCS1_v1_5_Import::toObject(JSContext *cx) {
  JS::RootedObject object (cx, JS_NewPlainObject(cx));
  
  auto alg_name = JS_NewStringCopyZ(cx, this->name());
  if (!alg_name) {
    return nullptr;
  }
  JS::RootedValue name_val(cx, JS::StringValue(alg_name));
  if (!JS_SetProperty(cx, object, "name", name_val)) {
    return nullptr;
  }
  JS::RootedObject hash(cx, JS_NewObject(cx, nullptr));

  auto hash_name = JS_NewStringCopyZ(cx, algorithmName(this->hashIdentifier));
  if (!hash_name) {
    return nullptr;
  }
  JS::RootedValue hash_name_val(cx, JS::StringValue(hash_name));
  if (!JS_SetProperty(cx, hash, "name", hash_name_val)) {
    return nullptr;
  }
  JS::RootedValue hash_val(cx, JS::ObjectValue(*hash));
  if (!JS_SetProperty(cx, object, "hash", hash_val)) {
    return nullptr;
  }
  return object;
}
JSObject* CryptoAlgorithmRSASSA_PKCS1_v1_5_KeyGen::toObject(JSContext *cx){
  JS::RootedObject object (cx, JS_NewPlainObject(cx));
  
  auto alg_name = JS_NewStringCopyZ(cx, this->name());
  if (!alg_name) {
    return nullptr;
  }
  JS::RootedValue name_val(cx, JS::StringValue(alg_name));
  if (!JS_SetProperty(cx, object, "name", name_val)) {
    return nullptr;
  }
  JS::RootedObject hash(cx, JS_NewObject(cx, nullptr));

  auto hash_name = JS_NewStringCopyZ(cx, algorithmName(this->hashIdentifier));
  if (!hash_name) {
    return nullptr;
  }
  JS::RootedValue hash_name_val(cx, JS::StringValue(hash_name));
  if (!JS_SetProperty(cx, hash, "name", hash_name_val)) {
    return nullptr;
  }
  JS::RootedValue hash_val(cx, JS::ObjectValue(*hash));
  if (!JS_SetProperty(cx, object, "hash", hash_val)) {
    return nullptr;
  }

  JS::RootedValue modulusLength(cx, JS::NumberValue(this->modulusLength));
  if (!JS_SetProperty(cx, object, "modulusLength", modulusLength)) {
    return nullptr;
  }

  uint8_t* p = reinterpret_cast<uint8_t*>(calloc(this->publicExponent.size(), sizeof(uint8_t)));
  auto exp = this->publicExponent;
  std::copy(exp.begin(), exp.end(), p);

  JS::RootedObject buffer(cx, JS::NewArrayBufferWithContents(cx, this->publicExponent.size(), p));
  if (!buffer) {
    // We can be here is the array buffer was too large -- if that was the case then a JSMSG_BAD_ARRAY_LENGTH will have been created.
      // No other failure scenarios in this path will create a JS exception and so we need to create one.
      if (!JS_IsExceptionPending(cx)) {
        // TODO Rename error to InternalError
        JS_ReportErrorLatin1(cx, "InternalError");
      }
    JS_free(cx, p);
    return nullptr;
  }

  JS::RootedObject byte_array(cx, JS_NewUint8ArrayWithBuffer(cx, buffer, 0, this->publicExponent.size()));
  JS::RootedValue publicExponent(cx, JS::ObjectValue(*byte_array));
  if (!JS_SetProperty(cx, object, "publicExponent", publicExponent)) {
    return nullptr;
  }

  return object;
}
JSObject* CryptoAlgorithmRSA_OAEP_EncDec::toObject(JSContext *cx){return nullptr;}
JSObject* CryptoAlgorithmRSA_OAEP_KeyImport::toObject(JSContext *cx){
  JS::RootedObject object (cx, JS_NewPlainObject(cx));
  
  auto alg_name = JS_NewStringCopyZ(cx, this->name());
  if (!alg_name) {
    return nullptr;
  }
  JS::RootedValue name_val(cx, JS::StringValue(alg_name));
  if (!JS_SetProperty(cx, object, "name", name_val)) {
    return nullptr;
  }
  JS::RootedObject hash(cx, JS_NewObject(cx, nullptr));

  auto hash_name = JS_NewStringCopyZ(cx, algorithmName(this->hashIdentifier));
  if (!hash_name) {
    return nullptr;
  }
  JS::RootedValue hash_name_val(cx, JS::StringValue(hash_name));
  if (!JS_SetProperty(cx, hash, "name", hash_name_val)) {
    return nullptr;
  }
  JS::RootedValue hash_val(cx, JS::ObjectValue(*hash));
  if (!JS_SetProperty(cx, object, "hash", hash_val)) {
    return nullptr;
  }
  return object;
}
JSObject* CryptoAlgorithmRSA_PSS_Key::toObject(JSContext *cx){return nullptr;}
JSObject* CryptoAlgorithmRSA_PSS_KeyImport::toObject(JSContext* cx) {return nullptr;}
JSObject* CryptoAlgorithmRSA_PSS_SignVerify::toObject(JSContext* cx) {return nullptr;}
JSObject* CryptoAlgorithmSHA1::toObject(JSContext* cx) {return nullptr;}
JSObject* CryptoAlgorithmSHA224::toObject(JSContext* cx) {return nullptr;}
JSObject* CryptoAlgorithmSHA256::toObject(JSContext* cx) {return nullptr;}
JSObject* CryptoAlgorithmSHA384::toObject(JSContext* cx) {return nullptr;}
JSObject* CryptoAlgorithmSHA512::toObject(JSContext* cx) {return nullptr;}

} // namespace builtins