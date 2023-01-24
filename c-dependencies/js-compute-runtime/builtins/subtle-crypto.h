#pragma once

#include "builtin.h"

namespace builtins {
enum class CryptoAlgorithmIdentifier {
  RSAES_PKCS1_v1_5,
  RSASSA_PKCS1_v1_5,
  RSA_PSS,
  RSA_OAEP,
  ECDSA,
  ECDH,
  AES_CTR,
  AES_CBC,
  AES_GCM,
  AES_CFB,
  AES_KW,
  HMAC,
  SHA_1,
  SHA_224,
  SHA_256,
  SHA_384,
  SHA_512,
  HKDF,
  PBKDF2
};

enum class Operations {
  Encrypt,
  Decrypt,
  Sign,
  Verify,
  Digest,
  GenerateKey,
  DeriveBits,
  ImportKey,
  WrapKey,
  UnwrapKey,
  GetKeyLength
};

class AlgorithmParameters {
public:
  enum class Class {
    None,
    AesCbcCfbParams,
    AesCtrParams,
    AesGcmParams,
    AesKeyParams,
    EcKeyParams,
    EcdhKeyDeriveParams,
    EcdsaParams,
    HkdfParams,
    HmacKeyParams,
    Pbkdf2Params,
    RsaHashedKeyGenParams,
    RsaHashedImportParams,
    RsaKeyGenParams,
    RsaOaepParams,
    RsaPssParams,
  };

  std::string name;
  CryptoAlgorithmIdentifier identifier;

  virtual Class parametersClass() const { return Class::None; }
};

class SubtleCrypto : public BuiltinImpl<SubtleCrypto> {
private:
public:
  static constexpr const char *class_name = "SubtleCrypto";
  static const int ctor_length = 0;

  enum Slots { Count };
  static const JSFunctionSpec methods[];
  static const JSPropertySpec properties[];
  static bool digest(JSContext *cx, unsigned argc, JS::Value *vp);
  static JS::Result<CryptoAlgorithmIdentifier>
  normalizeAlgorithm(JSContext *cx, JS::HandleValue algorithmIdentifier, Operations operation);
  static JS::Result<CryptoAlgorithmIdentifier>
  normalizeAlgorithm(JSContext *cx, CryptoAlgorithmIdentifier algorithmIdentifier,
                     Operations operation);

  static bool constructor(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool init_class(JSContext *cx, JS::HandleObject global);
};

} // namespace builtins
