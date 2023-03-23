#ifndef JS_COMPUTE_RUNTIME_CRYPTO_ALGORITHM_H
#define JS_COMPUTE_RUNTIME_CRYPTO_ALGORITHM_H
#include <span>

#include "builtin.h"
#include "openssl/evp.h"

namespace builtins {

// We are defining all the algorithms from
// https://w3c.github.io/webcrypto/#issue-container-generatedID-15
enum class CryptoAlgorithmIdentifier {
  RSASSA_PKCS1_v1_5,
  RSA_PSS,
  RSA_OAEP,
  ECDSA,
  ECDH,
  AES_CTR,
  AES_CBC,
  AES_GCM,
  AES_KW,
  HMAC,
  SHA_1,
  SHA_256,
  SHA_384,
  SHA_512,
  HKDF,
  PBKDF2
};

/// The base class that all algorithm implementations should derive from.
class CryptoAlgorithm {
public:
  virtual ~CryptoAlgorithm() = default;

  virtual const char *name() const noexcept = 0;
  virtual CryptoAlgorithmIdentifier identifier() = 0;
};

class CryptoAlgorithmDigest : public CryptoAlgorithm {
public:
  virtual JSObject *digest(JSContext *cx, std::span<uint8_t>) = 0;
  static std::unique_ptr<CryptoAlgorithmDigest> normalize(JSContext *cx, JS::HandleValue value);
};

class CryptoAlgorithmSHA1 final : public CryptoAlgorithmDigest {
public:
  const char *name() const noexcept override { return "SHA-1"; };
  CryptoAlgorithmIdentifier identifier() override { return CryptoAlgorithmIdentifier::SHA_1; };
  JSObject *digest(JSContext *cx, std::span<uint8_t>) override;
};

class CryptoAlgorithmSHA256 final : public CryptoAlgorithmDigest {
public:
  const char *name() const noexcept override { return "SHA-256"; };
  CryptoAlgorithmIdentifier identifier() override { return CryptoAlgorithmIdentifier::SHA_256; };
  JSObject *digest(JSContext *cx, std::span<uint8_t>) override;
};

class CryptoAlgorithmSHA384 final : public CryptoAlgorithmDigest {
public:
  const char *name() const noexcept override { return "SHA-384"; };
  CryptoAlgorithmIdentifier identifier() override { return CryptoAlgorithmIdentifier::SHA_384; };
  JSObject *digest(JSContext *cx, std::span<uint8_t>) override;
};

class CryptoAlgorithmSHA512 final : public CryptoAlgorithmDigest {
public:
  const char *name() const noexcept override { return "SHA-512"; };
  CryptoAlgorithmIdentifier identifier() override { return CryptoAlgorithmIdentifier::SHA_512; };
  JSObject *digest(JSContext *cx, std::span<uint8_t>) override;
};

} // namespace builtins
#endif