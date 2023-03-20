#pragma once
#include <span>

#include "openssl/evp.h"
#include "builtin.h"
#include "crypto-key-rsa-components.h"

namespace builtins {

class RsaOtherPrimesInfo {
public:
  std::string r;
  std::string d;
  std::string t;
  RsaOtherPrimesInfo(
    std::string r,
    std::string d,
    std::string t
  ): r{r}, d{d}, t{t} {}
};

class JsonWebKey {
public:
  // The following fields are defined in Section 3.1 of JSON Web Key
  std::string kty;
  std::optional<std::string> use;
  // TODO: We need to use mozilla::Vector instead of std::vector because std::vector can throw exceptions and we compile without exceptions
  std::vector<std::string> key_ops;
  std::optional<std::string> alg;

  // The following fields are defined in JSON Web Key Parameters Registration
  std::optional<bool> ext;

  // The following fields are defined in Section 6 of JSON Web Algorithms
  std::optional<std::string> crv;
  std::optional<std::string> x;
  std::optional<std::string> y;
  std::optional<std::string> n;
  std::optional<std::string> e;
  std::optional<std::string> d;
  std::optional<std::string> p;
  std::optional<std::string> q;
  std::optional<std::string> dp;
  std::optional<std::string> dq;
  std::optional<std::string> qi;
  // TODO: We need to use mozilla::Vector instead of std::vector because std::vector can throw exceptions and we compile without exceptions
  std::vector<RsaOtherPrimesInfo> oth;
  std::optional<std::string> k;
  JsonWebKey(
    std::string kty,
    std::vector<std::string> key_ops,
    std::optional<bool> ext,
    std::optional<std::string> n,
    std::optional<std::string> e
  ): kty{kty}, key_ops{key_ops}, ext{ext}, n{n}, e{e} {}

  JsonWebKey(
    std::string kty,
    std::vector<std::string> key_ops,
    std::optional<bool> ext,
    std::optional<std::string> n,
    std::optional<std::string> e,
    std::optional<std::string> d
  ): kty{kty}, key_ops{key_ops}, ext{ext}, n{n}, e{e}, d{d} {}
  JsonWebKey(
    std::string kty,
    std::vector<std::string> key_ops,
    std::optional<bool> ext,
    std::optional<std::string> n,
    std::optional<std::string> e,
    std::optional<std::string> d,
    std::optional<std::string> p,
    std::optional<std::string> q, 
    std::optional<std::string> dp, 
    std::optional<std::string> dq, 
    std::optional<std::string> qi
  ): kty{kty}, key_ops{key_ops}, ext{ext}, n{n}, e{e}, d{d}, p{p}, q{q}, dp{dp}, dq{dq}, qi{qi} {}

  JsonWebKey(
    std::string kty,
    std::optional<std::string> use,
    std::vector<std::string> key_ops,
    std::optional<std::string> alg,
    std::optional<bool> ext,
    std::optional<std::string> crv,
    std::optional<std::string> x,
    std::optional<std::string> y,
    std::optional<std::string> n,
    std::optional<std::string> e,
    std::optional<std::string> d,
    std::optional<std::string> p,
    std::optional<std::string> q,
    std::optional<std::string> dp,
    std::optional<std::string> dq,
    std::optional<std::string> qi,
    std::vector<RsaOtherPrimesInfo> oth,
    std::optional<std::string> k
  ): 
  kty{kty},
  use{use},
  key_ops{key_ops},
  alg{alg},
  ext{ext},
  crv{crv},
  x{x},
  y{y},
  n{n},
  e{e},
  d{d},
  p{p},
  q{q},
  dp{dp},
  dq{dq},
  qi{qi},
  oth{oth},
  k{k} {}
};


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

enum class CryptoKeyFormat {
  Raw,
  Spki,
  Pkcs8,
  Jwk
};

enum class CryptoKeyClass {
  AES,
  EC,
  HMAC,
  RSA,
  Raw,
};

enum class CryptoKeyType : uint8_t { Public, Private, Secret };

enum {
  CryptoKeyUsageEncrypt = 1 << 0,
  CryptoKeyUsageDecrypt = 1 << 1,
  CryptoKeyUsageSign = 1 << 2,
  CryptoKeyUsageVerify = 1 << 3,
  CryptoKeyUsageDeriveKey = 1 << 4,
  CryptoKeyUsageDeriveBits = 1 << 5,
  CryptoKeyUsageWrapKey = 1 << 6,
  CryptoKeyUsageUnwrapKey = 1 << 7
};

typedef int CryptoKeyUsageBitmap;

const char *algorithmName(CryptoAlgorithmIdentifier algorithm);

class CryptoAlgorithm {
public:
    virtual ~CryptoAlgorithm() = default;

    virtual const char* name() const noexcept = 0;
    virtual CryptoAlgorithmIdentifier identifier() = 0;
    virtual JSObject* toObject(JSContext* cx) = 0;
};

class CryptoAlgorithmRSA_OAEP_KeyImport;
class CryptoAlgorithmRSASSA_PKCS1_v1_5_Import;

class CryptoKey : public BuiltinImpl<CryptoKey> {
public:
  static const int ctor_length = 0;
  static constexpr const char *class_name = "CryptoKey";
  
  static CryptoKeyType type(JSObject* self);
  static CryptoKeyUsageBitmap toKeyUsageBitmap(std::vector<std::string> key_usages);
  static JS::Result<CryptoKeyUsageBitmap> toKeyUsageBitmap(JSContext* cx, JS::HandleValue key_usages, std::string_view error_message);
  static JS::Result<bool> is_algorithm(JSContext *cx, JS::HandleObject self, CryptoAlgorithmIdentifier algorithm);
  static bool hasKeyUsage(JSObject* self, uint8_t usage);
  static void set_algorithm(JSObject* self, std::unique_ptr<CryptoAlgorithm> algorithm);
  static EVP_PKEY* key(JSObject* self);
  static JSObject *get_algorithm(JS::HandleObject self);
  static JSObject *importJwkRsa(JSContext *cx, CryptoAlgorithmRSA_OAEP_KeyImport* algorithm, JsonWebKey* keyData, bool extractable, CryptoKeyUsageBitmap usages);
  static JSObject *createRSA(JSContext *cx, CryptoAlgorithmRSA_OAEP_KeyImport* algorithm, CryptoKeyRSAComponents keyData, bool extractable, CryptoKeyUsageBitmap usages);

  static JSObject *importJwkRsa(JSContext *cx, CryptoAlgorithmRSASSA_PKCS1_v1_5_Import* algorithm, JsonWebKey* keyData, bool extractable, CryptoKeyUsageBitmap usages);
  static JSObject *createRSA(JSContext *cx, CryptoAlgorithmRSASSA_PKCS1_v1_5_Import* algorithm, CryptoKeyRSAComponents keyData, bool extractable, CryptoKeyUsageBitmap usages);

  static bool algorithm_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool extractable_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool usages_get(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool type_get(JSContext *cx, unsigned argc, JS::Value *vp);

  enum Slots { Algorithm, Type, Extractable, Usage, Key, Count };
  static const JSFunctionSpec methods[];
  static const JSPropertySpec properties[];
  static JSObject *create(JSContext *cx, CryptoAlgorithm* algorithm, CryptoKeyType type,
                            bool extractable, CryptoKeyUsageBitmap usage, void *key);
  static JSObject *create(JSContext *cx, CryptoAlgorithmRSA_OAEP_KeyImport* algorithm, CryptoKeyType type,
                            bool extractable, CryptoKeyUsageBitmap usage, EVP_PKEY *key);
  static bool constructor(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool init_class(JSContext *cx, JS::HandleObject global);
};

using KeyData = std::variant<std::span<uint8_t>, JsonWebKey*>;

class CryptoAlgorithmEncDec: public CryptoAlgorithm {
public:
  virtual JSObject* encrypt(JSContext *cx, CryptoKey, std::vector<uint8_t>) = 0;
  virtual JSObject* decrypt(JSContext *cx, CryptoKey, std::vector<uint8_t>) = 0;
  static std::unique_ptr<CryptoAlgorithmEncDec> normalize(JSContext *cx, JS::HandleValue value);
};

class CryptoAlgorithmSignVerify: public CryptoAlgorithm {
public:
  virtual JSObject* sign(JSContext* cx, JS::HandleObject key, std::span<uint8_t> data) = 0;
  virtual JS::Result<bool> verify(JSContext* cx, JS::HandleObject key, std::span<uint8_t> signature, std::span<uint8_t> data) = 0;
  static std::unique_ptr<CryptoAlgorithmSignVerify> normalize(JSContext* cx, JS::HandleValue value);
};

class CryptoAlgorithmDigest: public CryptoAlgorithm {
public:
  virtual JSObject* digest(JSContext *cx, std::span<uint8_t>) = 0;
  static std::unique_ptr<CryptoAlgorithmDigest> normalize(JSContext* cx, JS::HandleValue value);
};

class CryptoAlgorithmGenerateKey: public CryptoAlgorithm {
public:
  virtual JSObject* generateKey(JSContext *cx, bool extractable, CryptoKeyUsageBitmap) = 0;
  static std::unique_ptr<CryptoAlgorithmGenerateKey> normalize(JSContext* cx, JS::HandleValue value);
};

class CryptoAlgorithmDeriveBits: public CryptoAlgorithm {
public:
  virtual JSObject* deriveBits(JSContext *cx, CryptoKey, size_t length) = 0;
  static std::unique_ptr<CryptoAlgorithmDeriveBits> normalize(JSContext* cx, JS::HandleValue value);
};

class CryptoAlgorithmImportKey: public CryptoAlgorithm {
public:
  virtual JSObject* importKey(JSContext* cx, CryptoKeyFormat, JS::HandleValue, bool extractable, CryptoKeyUsageBitmap) = 0;
  virtual JSObject* importKey(JSContext *cx, CryptoKeyFormat, KeyData, bool extractable, CryptoKeyUsageBitmap) = 0;
  static std::unique_ptr<CryptoAlgorithmImportKey> normalize(JSContext* cx, JS::HandleValue value);
};

class CryptoAlgorithmWrapUnwrapkey: public CryptoAlgorithm {
public:
  virtual JSObject* wrapKey(JSContext *cx, CryptoKey, std::vector<uint8_t>) = 0;
    virtual JSObject* unwrapKey(JSContext *cx, CryptoKey, std::vector<uint8_t>) = 0;
  static std::unique_ptr<CryptoAlgorithmWrapUnwrapkey> normalize(JSContext* cx, JS::HandleValue value);
};

class CryptoAlgorithmGetKeyLength: public CryptoAlgorithm {
public:
  virtual size_t getKeyLength(JSContext *cx);
  static std::unique_ptr<CryptoAlgorithmGetKeyLength> normalize(JSContext* cx, JS::HandleValue value);
};

class CryptoAlgorithmECDHKey final : public CryptoAlgorithmImportKey, public CryptoAlgorithmDeriveBits {
public:
    // A CryptoKey object representing the public key of the other entity.
    CryptoKey* publicKey;
    // The named curve that the key uses
    const char* namedCurve;
    const char* name() const noexcept override {
      return "ECDH";
    };
    CryptoAlgorithmECDHKey(CryptoKey* publicKey, const char* namedCurve): publicKey{publicKey}, namedCurve{namedCurve} {};
    CryptoAlgorithmIdentifier identifier() override {
      return CryptoAlgorithmIdentifier::ECDH;
    };

    JSObject* deriveBits(JSContext *cx, CryptoKey, size_t length) override;
    JSObject* importKey(JSContext* cx, CryptoKeyFormat, JS::HandleValue, bool extractable, CryptoKeyUsageBitmap) override;
    JSObject* importKey(JSContext *cx, CryptoKeyFormat, KeyData, bool extractable, CryptoKeyUsageBitmap) override;
    JSObject* exportKey(JSContext* cx, CryptoKeyFormat format, JS::HandleObject key);
    JSObject* toObject(JSContext* cx) override;
};

class CryptoAlgorithmECDHGenKey final : public CryptoAlgorithmGenerateKey {
public:
    // The named curve that the key uses
    const char* namedCurve;
    const char* name() const noexcept override {
      return "ECDH";
    };
    CryptoAlgorithmECDHGenKey(const char* namedCurve): namedCurve{namedCurve} {};
    CryptoAlgorithmIdentifier identifier() override {
      return CryptoAlgorithmIdentifier::ECDH;
    };

    JSObject* generateKey(JSContext *cx, bool extractable, CryptoKeyUsageBitmap) override;
    JSObject* exportKey(JSContext* cx, CryptoKeyFormat format, JS::HandleObject key);
    JSObject* toObject(JSContext* cx) override;
};

class CryptoAlgorithmECDSASignVerify final : public CryptoAlgorithmSignVerify {
public:
    CryptoAlgorithmIdentifier hashIdentifier;
    const char* name() const noexcept override {
      return "ECDSA";
    };
    CryptoAlgorithmECDSASignVerify(CryptoAlgorithmIdentifier hashIdentifier): hashIdentifier{hashIdentifier} {};
    CryptoAlgorithmIdentifier identifier() override {
      return CryptoAlgorithmIdentifier::ECDSA;
    };

    JSObject* sign(JSContext* cx, JS::HandleObject key, std::span<uint8_t> data) override;
    JS::Result<bool> verify(JSContext* cx, JS::HandleObject key, std::span<uint8_t> signature, std::span<uint8_t> data) override;
    JSObject* toObject(JSContext* cx) override;
};
class CryptoAlgorithmECDSAKey final : public CryptoAlgorithmImportKey {
public:
    CryptoAlgorithmIdentifier hashIdentifier;
    // The named curve that the key uses
    const char* namedCurve;

    const char* name() const noexcept override {
      return "ECDSA";
    };
    CryptoAlgorithmECDSAKey(CryptoAlgorithmIdentifier hashIdentifier, const char* namedCurve): hashIdentifier{hashIdentifier}, namedCurve{namedCurve} {};
    CryptoAlgorithmIdentifier identifier() override {
      return CryptoAlgorithmIdentifier::ECDSA;
    };

    JSObject* importKey(JSContext* cx, CryptoKeyFormat, JS::HandleValue, bool extractable, CryptoKeyUsageBitmap) override;
    JSObject* importKey(JSContext *cx, CryptoKeyFormat, KeyData, bool extractable, CryptoKeyUsageBitmap) override;
    JSObject* exportKey(JSContext* cx, CryptoKeyFormat format, JS::HandleObject key);
    JSObject* toObject(JSContext* cx) override;
};

class CryptoAlgorithmECDSAGenKey final : public CryptoAlgorithmGenerateKey {
public:
    // The named curve that the key uses
    const char* namedCurve;

    const char* name() const noexcept override {
      return "ECDSA";
    };
    CryptoAlgorithmECDSAGenKey(const char* namedCurve): namedCurve{namedCurve} {};
    CryptoAlgorithmIdentifier identifier() override {
      return CryptoAlgorithmIdentifier::ECDSA;
    };

    JSObject* generateKey(JSContext *cx, bool extractable, CryptoKeyUsageBitmap) override;
    JSObject* exportKey(JSContext* cx, CryptoKeyFormat format, JS::HandleObject key);
    JSObject* toObject(JSContext* cx) override;
};

class CryptoAlgorithmHKDF final : public CryptoAlgorithmGetKeyLength {
public:

    const char* name() const noexcept override {
      return "HKDF";
    };
    CryptoAlgorithmHKDF() {};
    CryptoAlgorithmIdentifier identifier() override {
      return CryptoAlgorithmIdentifier::HKDF;
    };

    size_t getKeyLength(JSContext *cx) override {
      return 0;
    };
    JSObject* toObject(JSContext* cx) override;
};

class CryptoAlgorithmHKDFDerive final : public CryptoAlgorithmDeriveBits {
public:
    CryptoAlgorithmIdentifier hashIdentifier;
    std::span<uint8_t>salt;
    std::span<uint8_t>info;

    const char* name() const noexcept override {
      return "HKDF";
    };
    CryptoAlgorithmHKDFDerive(CryptoAlgorithmIdentifier hashIdentifier, std::span<uint8_t>salt, std::span<uint8_t>info): hashIdentifier{hashIdentifier}, salt{salt}, info{info} {};
    CryptoAlgorithmIdentifier identifier() override {
      return CryptoAlgorithmIdentifier::HKDF;
    };

    JSObject* deriveBits(JSContext* cx, CryptoKey, size_t length) override;
    JSObject* toObject(JSContext* cx) override;
};

class CryptoAlgorithmHKDFImport final : public CryptoAlgorithmImportKey {
public:
    std::span<uint8_t>salt;

    const char* name() const noexcept override {
      return "HKDF";
    };
    CryptoAlgorithmHKDFImport(std::span<uint8_t>salt): salt{salt} {};
    CryptoAlgorithmIdentifier identifier() override {
      return CryptoAlgorithmIdentifier::HKDF;
    };

    JSObject* importKey(JSContext* cx, CryptoKeyFormat, JS::HandleValue, bool extractable, CryptoKeyUsageBitmap) override;
    JSObject* importKey(JSContext* cx, CryptoKeyFormat, KeyData, bool extractable, CryptoKeyUsageBitmap) override;
    JSObject* toObject(JSContext* cx) override;
};


class CryptoAlgorithmHMAC final : public CryptoAlgorithmSignVerify, public CryptoAlgorithmGenerateKey, public CryptoAlgorithmImportKey, public CryptoAlgorithmGetKeyLength {
public:
    // The digest function to use. This can take a value of SHA-256, SHA-384, or SHA-512.
    CryptoAlgorithmIdentifier hashIdentifier;
    // A Number representing the length in bits of the key.
    // If this is omitted the length of the key is equal to the length of the digest generated
    // by the digest function you have chosen.
    size_t length;

    const char* name() const noexcept override {
      return "HMAC";
    };
    CryptoAlgorithmHMAC(CryptoAlgorithmIdentifier hashIdentifier): hashIdentifier{hashIdentifier} {
      switch (hashIdentifier) {
        case CryptoAlgorithmIdentifier::SHA_1:
        case CryptoAlgorithmIdentifier::SHA_224:
        case CryptoAlgorithmIdentifier::SHA_256:
            length = 512;
            break;
        case CryptoAlgorithmIdentifier::SHA_384:
        case CryptoAlgorithmIdentifier::SHA_512:
            length = 1024;
            break;
        default:
            MOZ_ASSERT_UNREACHABLE();
      }
    };
    
    CryptoAlgorithmHMAC(CryptoAlgorithmIdentifier hashIdentifier, size_t length): hashIdentifier{hashIdentifier}, length{length} {};
    
    CryptoAlgorithmIdentifier identifier() override {
      return CryptoAlgorithmIdentifier::HMAC;
    };

    JSObject* sign(JSContext* cx, JS::HandleObject key, std::span<uint8_t> data) override;
    JS::Result<bool> verify(JSContext* cx, JS::HandleObject key, std::span<uint8_t> signature, std::span<uint8_t> data) override;
    JSObject* generateKey(JSContext* cx, bool extractable, CryptoKeyUsageBitmap) override;
    JSObject* importKey(JSContext* cx, CryptoKeyFormat, JS::HandleValue, bool extractable, CryptoKeyUsageBitmap) override;
    JSObject* importKey(JSContext* cx, CryptoKeyFormat, KeyData, bool extractable, CryptoKeyUsageBitmap) override;
    JSObject* exportKey(JSContext* cx, CryptoKeyFormat format, JS::HandleObject key);
    size_t getKeyLength(JSContext* cx) override;
    JSObject* toObject(JSContext* cx) override;
};

class CryptoAlgorithmPBKDF2 final : public CryptoAlgorithmGetKeyLength {
public:
    const char* name() const noexcept override {
      return "PBKDF2";
    };
    CryptoAlgorithmPBKDF2() {};
    CryptoAlgorithmIdentifier identifier() final {
      return CryptoAlgorithmIdentifier::PBKDF2;
    };

    size_t getKeyLength(JSContext* cx) override {
      return 0;
    };
    JSObject* toObject(JSContext* cx) override;
};

class CryptoAlgorithmPBKDF2Key final : public CryptoAlgorithmImportKey, public CryptoAlgorithmDeriveBits {
public:
    CryptoAlgorithmIdentifier hashIdentifier;
    std::span<uint8_t>salt;
    unsigned long iterations;

    const char* name() const noexcept override {
      return "PBKDF2";
    };
    CryptoAlgorithmPBKDF2Key(CryptoAlgorithmIdentifier hashIdentifier, std::span<uint8_t>salt, unsigned long iterations): hashIdentifier{hashIdentifier}, salt{salt}, iterations{iterations} {};
    CryptoAlgorithmIdentifier identifier() final {
      return CryptoAlgorithmIdentifier::PBKDF2;
    };

    JSObject* deriveBits(JSContext *cx, CryptoKey, size_t length) override;
    JSObject* importKey(JSContext* cx, CryptoKeyFormat, JS::HandleValue, bool extractable, CryptoKeyUsageBitmap) override;
    JSObject* importKey(JSContext *cx, CryptoKeyFormat, KeyData, bool extractable, CryptoKeyUsageBitmap) override;
    JSObject* toObject(JSContext* cx) override;
};

class CryptoAlgorithmRSAES_PKCS1_v1_5 final : public CryptoAlgorithmEncDec, public CryptoAlgorithmGenerateKey, public CryptoAlgorithmImportKey {
public:
    std::optional<size_t> modulusLength;
    std::optional<std::span<uint8_t>> publicExponent;

    const char* name() const noexcept override {
      return "RSAES-PKCS1-v1_5";
    };
    CryptoAlgorithmRSAES_PKCS1_v1_5(std::optional<size_t> modulusLength, std::optional<std::span<uint8_t>> publicExponent): modulusLength{modulusLength}, publicExponent{publicExponent} {};
    CryptoAlgorithmIdentifier identifier() final {
      return CryptoAlgorithmIdentifier::RSAES_PKCS1_v1_5;
    };

    JSObject* encrypt(JSContext* cx, CryptoKey, std::vector<uint8_t>) override;
    JSObject* decrypt(JSContext* cx, CryptoKey, std::vector<uint8_t>) override;
    JSObject* generateKey(JSContext* cx, bool extractable, CryptoKeyUsageBitmap) override;
    JSObject* importKey(JSContext* cx, CryptoKeyFormat, JS::HandleValue, bool extractable, CryptoKeyUsageBitmap) override;
    JSObject* importKey(JSContext* cx, CryptoKeyFormat, KeyData, bool extractable, CryptoKeyUsageBitmap) override;
    JSObject* exportKey(JSContext* cx, CryptoKeyFormat format, JS::HandleObject key);
    JSObject* toObject(JSContext* cx) override;
};

class CryptoAlgorithmRSASSA_PKCS1_v1_5 final : public CryptoAlgorithmSignVerify {
public:

    const char* name() const noexcept override {
      return "RSASSA-PKCS1-v1_5";
    };
    CryptoAlgorithmRSASSA_PKCS1_v1_5() {};
    CryptoAlgorithmIdentifier identifier() final {
      return CryptoAlgorithmIdentifier::RSASSA_PKCS1_v1_5;
    };
    
    JSObject* sign(JSContext* cx, JS::HandleObject key, std::span<uint8_t> data) override;
    JS::Result<bool> verify(JSContext* cx, JS::HandleObject key, std::span<uint8_t> signature, std::span<uint8_t> data) override;
    static JSObject* exportKey(JSContext* cx, CryptoKeyFormat format, JS::HandleObject key);
    JSObject* toObject(JSContext* cx) override;
};

class CryptoAlgorithmRSASSA_PKCS1_v1_5_KeyGen final : public CryptoAlgorithmGenerateKey {
public:
    size_t modulusLength;
    std::span<uint8_t>publicExponent;
    CryptoAlgorithmIdentifier hashIdentifier;

    const char* name() const noexcept override {
      return "RSASSA-PKCS1-v1_5";
    };
    CryptoAlgorithmRSASSA_PKCS1_v1_5_KeyGen(size_t modulusLength,std::span<uint8_t>publicExponent,CryptoAlgorithmIdentifier hashIdentifier): modulusLength{modulusLength}, publicExponent{publicExponent}, hashIdentifier{hashIdentifier} {};
    CryptoAlgorithmIdentifier identifier() final {
      return CryptoAlgorithmIdentifier::RSASSA_PKCS1_v1_5;
    };

    JSObject* generateKey(JSContext* cx, bool extractable, CryptoKeyUsageBitmap) override;
    JSObject* exportKey(JSContext* cx, CryptoKeyFormat format, JS::HandleObject key);
    JSObject* toObject(JSContext* cx) override;
};

class CryptoAlgorithmRSASSA_PKCS1_v1_5_Import final : public CryptoAlgorithmImportKey {
public:
    CryptoAlgorithmIdentifier hashIdentifier;

    const char* name() const noexcept override {
      return "RSASSA-PKCS1-v1_5";
    };
    CryptoAlgorithmRSASSA_PKCS1_v1_5_Import(CryptoAlgorithmIdentifier hashIdentifier): hashIdentifier{hashIdentifier} {};
    CryptoAlgorithmIdentifier identifier() final {
      return CryptoAlgorithmIdentifier::RSASSA_PKCS1_v1_5;
    };

    JSObject* importKey(JSContext* cx, CryptoKeyFormat, JS::HandleValue, bool extractable, CryptoKeyUsageBitmap) override;
    JSObject* importKey(JSContext* cx, CryptoKeyFormat, KeyData, bool extractable, CryptoKeyUsageBitmap) override;
    JSObject* exportKey(JSContext* cx, CryptoKeyFormat format, JS::HandleObject key);
    JSObject* toObject(JSContext* cx) override;
};

class CryptoAlgorithmRSA_OAEP_Key final : public CryptoAlgorithmGenerateKey {
public:
    size_t modulusLength;
    std::span<uint8_t> publicExponent;
    CryptoAlgorithmIdentifier hashIdentifier;
    CryptoAlgorithmRSA_OAEP_Key(size_t modulusLength,std::span<uint8_t> publicExponent,CryptoAlgorithmIdentifier hashIdentifier):modulusLength{modulusLength}, publicExponent{publicExponent}, hashIdentifier{hashIdentifier} {};

    const char* name() const noexcept override {
      return "RSA-OAEP";
    };

    CryptoAlgorithmIdentifier identifier() override {
      return CryptoAlgorithmIdentifier::RSA_OAEP;
    };

    JSObject* generateKey(JSContext* cx, bool extractable, CryptoKeyUsageBitmap) override;
    JSObject* exportKey(JSContext* cx, CryptoKeyFormat format, JS::HandleObject key);

    JSObject* toObject(JSContext* cx) override;
};

class CryptoAlgorithmRSA_OAEP_KeyImport final : public CryptoAlgorithmImportKey {
public:
    CryptoAlgorithmIdentifier hashIdentifier;
    CryptoAlgorithmRSA_OAEP_KeyImport(CryptoAlgorithmIdentifier hashIdentifier): hashIdentifier{hashIdentifier} {};

    const char* name() const noexcept override {
      return "RSA-OAEP";
    };

    CryptoAlgorithmIdentifier identifier() override {
      return CryptoAlgorithmIdentifier::RSA_OAEP;
    };

    JSObject* importKey(JSContext* cx, CryptoKeyFormat, JS::HandleValue, bool extractable, CryptoKeyUsageBitmap) override;
    JSObject* importKey(JSContext* cx, CryptoKeyFormat, KeyData, bool extractable, CryptoKeyUsageBitmap) override;
    JSObject* exportKey(JSContext* cx, CryptoKeyFormat format, JS::HandleObject key);
    JSObject* toObject(JSContext* cx) override;
};

class CryptoAlgorithmRSA_OAEP_EncDec final : public CryptoAlgorithmEncDec {
public:
    // An ArrayBuffer, a TypedArray, or a DataView — an array of bytes that does not itself need to be encrypted but which should be bound to the ciphertext. A digest of the label is part of the input to the encryption operation.
    // Unless your application calls for a label, you can just omit this argument and it will not affect the security of the encryption operation.
    std::optional<std::span<uint8_t>> label;
    const char* name() const noexcept override {
      return "RSA-OAEP";
    };

    CryptoAlgorithmRSA_OAEP_EncDec(std::optional<std::span<uint8_t>> label): label{label} {};
    CryptoAlgorithmIdentifier identifier() override {
      return CryptoAlgorithmIdentifier::RSA_OAEP;
    };
    JSObject* encrypt(JSContext* cx, CryptoKey, std::vector<uint8_t>) override;
    JSObject* decrypt(JSContext* cx, CryptoKey, std::vector<uint8_t>) override;
    JSObject* toObject(JSContext* cx) override;
};

class CryptoAlgorithmRSA_PSS_Key final : public CryptoAlgorithmGenerateKey {
public:
    size_t modulusLength;
    std::span<uint8_t>publicExponent;
    CryptoAlgorithmIdentifier hashIdentifier;

    const char* name() const noexcept override {
      return "RSA-PSS";
    };
    CryptoAlgorithmRSA_PSS_Key(size_t modulusLength,std::span<uint8_t>publicExponent, CryptoAlgorithmIdentifier hashIdentifier): modulusLength{modulusLength}, publicExponent{publicExponent}, hashIdentifier{hashIdentifier} {}
    CryptoAlgorithmIdentifier identifier() override {
      return CryptoAlgorithmIdentifier::RSA_PSS;
    };


    JSObject* generateKey(JSContext* cx, bool extractable, CryptoKeyUsageBitmap) override;
    JSObject* exportKey(JSContext* cx, CryptoKeyFormat format, JS::HandleObject key);
    JSObject* toObject(JSContext* cx) override;
};

class CryptoAlgorithmRSA_PSS_KeyImport final : public CryptoAlgorithmImportKey {
public:
    CryptoAlgorithmIdentifier hashIdentifier;

    const char* name() const noexcept override {
      return "RSA-PSS";
    };
    CryptoAlgorithmRSA_PSS_KeyImport(CryptoAlgorithmIdentifier hashIdentifier): hashIdentifier{hashIdentifier} {}
    CryptoAlgorithmIdentifier identifier() override {
      return CryptoAlgorithmIdentifier::RSA_PSS;
    };


    JSObject* importKey(JSContext* cx, CryptoKeyFormat, JS::HandleValue, bool extractable, CryptoKeyUsageBitmap) override;
    JSObject* importKey(JSContext* cx, CryptoKeyFormat, KeyData, bool extractable, CryptoKeyUsageBitmap) override;
    JSObject* exportKey(JSContext* cx, CryptoKeyFormat format, JS::HandleObject key);
    JSObject* toObject(JSContext* cx) override;
};

class CryptoAlgorithmRSA_PSS_SignVerify final: public CryptoAlgorithmSignVerify {
public:
    // A long integer representing the length of the random salt to use, in bytes.
    // RFC 3447 says that "Typical salt lengths" are either 0 or the length of the output of the digest algorithm that was selected when this key was generated. For example, if you use SHA-256 as the digest algorithm, this could be 32.
    // The maximum size of saltLength is given by:
    // Math.ceil((keySizeInBits - 1)/8) - digestSizeInBytes - 2
    // So for a key length of 2048 bits and a digest output size of 32 bytes, the maximum size would be 222.
    size_t saltLength;
    const char* name() const noexcept override {
      return "RSA-PSS";
    };
    CryptoAlgorithmRSA_PSS_SignVerify(size_t saltLength): saltLength{saltLength} {}
    CryptoAlgorithmIdentifier identifier() override {
      return CryptoAlgorithmIdentifier::RSA_PSS;
    };

    JSObject* sign(JSContext* cx, JS::HandleObject key, std::span<uint8_t> data) override;
    JS::Result<bool> verify(JSContext* cx, JS::HandleObject key, std::span<uint8_t> signature, std::span<uint8_t> data) override;
    JSObject* toObject(JSContext* cx) override;
};

class CryptoAlgorithmSHA1 final : public CryptoAlgorithmDigest {
public:
    const char* name() const noexcept override {
      return "SHA-1";
    };
    CryptoAlgorithmIdentifier identifier() override {return CryptoAlgorithmIdentifier::SHA_1;};
    JSObject* digest(JSContext *cx, std::span<uint8_t>) override;
    JSObject* toObject(JSContext* cx) override;
};

class CryptoAlgorithmSHA224 final : public CryptoAlgorithmDigest {
public:
    const char* name() const noexcept override {
      return "SHA-224";
    };
    CryptoAlgorithmIdentifier identifier() override {
      return CryptoAlgorithmIdentifier::SHA_224;
    };
    JSObject* digest(JSContext *cx, std::span<uint8_t>) override;
    JSObject* toObject(JSContext* cx) override;
};

class CryptoAlgorithmSHA256 final : public CryptoAlgorithmDigest {
public:
    const char* name() const noexcept override {
      return "SHA-256";
    };
    CryptoAlgorithmIdentifier identifier() override {
      return CryptoAlgorithmIdentifier::SHA_256;
    };
    JSObject* digest(JSContext *cx, std::span<uint8_t>) override;
    JSObject* toObject(JSContext* cx) override;
};

class CryptoAlgorithmSHA384 final : public CryptoAlgorithmDigest {
public:
    const char* name() const noexcept override {
      return "SHA-384";
    };
    CryptoAlgorithmIdentifier identifier() override {
      return CryptoAlgorithmIdentifier::SHA_384;
    };
    JSObject* digest(JSContext *cx, std::span<uint8_t>) override;
    JSObject* toObject(JSContext* cx) override;
};

class CryptoAlgorithmSHA512 final : public CryptoAlgorithmDigest {
public:
    const char* name() const noexcept override {
      return "SHA-512";
    };
    CryptoAlgorithmIdentifier identifier() override {
      return CryptoAlgorithmIdentifier::SHA_512;
    };
    JSObject* digest(JSContext *cx, std::span<uint8_t>) override;
    JSObject* toObject(JSContext* cx) override;
};

class CryptoAlgorithmAES_CBC_EncDec final : public CryptoAlgorithmEncDec {
public:
    // An ArrayBuffer, a TypedArray, or a DataView. The initialization vector. 
    // Must be 16 bytes, unpredictable, and preferably cryptographically random. 
    // However, it need not be secret (for example, it may be transmitted unencrypted along with the ciphertext).
    std::span<uint8_t> iv;

    enum class Padding : uint8_t {
        Yes,
        No
    };

    const char* name() const noexcept override {
      return "AES-CBC";
    };
    CryptoAlgorithmAES_CBC_EncDec(std::span<uint8_t>iv): iv{iv} {}
    CryptoAlgorithmIdentifier identifier() override {
      return CryptoAlgorithmIdentifier::AES_CBC;
    };

    JSObject* encrypt(JSContext* cx, CryptoKey, std::vector<uint8_t>) override;
    JSObject* decrypt(JSContext* cx, CryptoKey, std::vector<uint8_t>) override;
    JSObject* toObject(JSContext* cx) override;
};

class CryptoAlgorithmAES_CBC_Key final : public CryptoAlgorithmGenerateKey, public CryptoAlgorithmGetKeyLength {
public:
    // A Number — the length in bits of the key to generate. This must be one of: 128, 192, or 256.
    unsigned short length;

    enum class Padding : uint8_t {
        Yes,
        No
    };

    const char* name() const noexcept override {
      return "AES-CBC";
    };
    CryptoAlgorithmAES_CBC_Key(unsigned short length): length{length} {
      MOZ_ASSERT(length == 128 || length == 192 || length == 256);
    }
    CryptoAlgorithmIdentifier identifier() override {
      return CryptoAlgorithmIdentifier::AES_CBC;
    };

    JSObject* generateKey(JSContext* cx, bool extractable, CryptoKeyUsageBitmap) override;
    JSObject* exportKey(JSContext* cx, CryptoKeyFormat format, JS::HandleObject key);
    size_t getKeyLength(JSContext* cx) override;
    JSObject* toObject(JSContext* cx) override;
};

class CryptoAlgorithmAES_CBC_KeyImport final : public CryptoAlgorithmImportKey{
public:
    enum class Padding : uint8_t {
        Yes,
        No
    };

    const char* name() const noexcept override {
    return "AES-CBC";
  };
    CryptoAlgorithmAES_CBC_KeyImport() {}
    CryptoAlgorithmIdentifier identifier() override {
      return CryptoAlgorithmIdentifier::AES_CBC;
    };

    JSObject* importKey(JSContext* cx, CryptoKeyFormat, JS::HandleValue, bool extractable, CryptoKeyUsageBitmap) override;
    JSObject* importKey(JSContext* cx, CryptoKeyFormat, KeyData, bool extractable, CryptoKeyUsageBitmap) override;
    JSObject* exportKey(JSContext* cx, CryptoKeyFormat format, JS::HandleObject key);
    JSObject* toObject(JSContext* cx) override;
};


class CryptoAlgorithmAES_CFB_EncDec final : public CryptoAlgorithmEncDec {
public:
  // An ArrayBuffer, a TypedArray, or a DataView. The initialization vector. 
  // Must be 16 bytes, unpredictable, and preferably cryptographically random. 
  // However, it need not be secret (for example, it may be transmitted unencrypted along with the ciphertext).
  std::span<uint8_t> iv;

  const char* name() const noexcept override {
    return "AES-CFB-8";
  };
  CryptoAlgorithmAES_CFB_EncDec(std::span<uint8_t>iv): iv{iv}{}
  CryptoAlgorithmIdentifier identifier() override {
    return CryptoAlgorithmIdentifier::AES_CFB;
  };

  JSObject* encrypt(JSContext* cx, CryptoKey, std::vector<uint8_t>) override;
  JSObject* decrypt(JSContext* cx, CryptoKey, std::vector<uint8_t>) override;
JSObject* toObject(JSContext* cx) override;
};

class CryptoAlgorithmAES_CFB_Key final : public CryptoAlgorithmGenerateKey, public CryptoAlgorithmGetKeyLength {
public:
  // A Number — the length in bits of the key to generate. This must be one of: 128, 192, or 256.
  unsigned short length;

  const char* name() const noexcept override {
    return "AES-CFB-8";
  };
  
  CryptoAlgorithmAES_CFB_Key(unsigned short length): length{length} {
    MOZ_ASSERT(length == 128 || length == 192 || length == 256);
  }
  CryptoAlgorithmIdentifier identifier() override {
    return CryptoAlgorithmIdentifier::AES_CFB;
  };

  JSObject* generateKey(JSContext* cx, bool extractable, CryptoKeyUsageBitmap) override;
  JSObject* exportKey(JSContext* cx, CryptoKeyFormat format, JS::HandleObject key);
  size_t getKeyLength(JSContext* cx) override;
  JSObject* toObject(JSContext* cx) override;
};

class CryptoAlgorithmAES_CFB_KeyImport final : public CryptoAlgorithmImportKey{
public:
    const char* name() const noexcept override {
    return "AES-CFB-8";
  };
  
    CryptoAlgorithmAES_CFB_KeyImport() {}
    CryptoAlgorithmIdentifier identifier() override {
      return CryptoAlgorithmIdentifier::AES_CFB;
    };

    JSObject* importKey(JSContext* cx, CryptoKeyFormat, JS::HandleValue, bool extractable, CryptoKeyUsageBitmap) override;
    JSObject* importKey(JSContext* cx, CryptoKeyFormat, KeyData, bool extractable, CryptoKeyUsageBitmap) override;
    JSObject* exportKey(JSContext* cx, CryptoKeyFormat format, JS::HandleObject key);
  JSObject* toObject(JSContext* cx) override;
};

class CryptoAlgorithmAES_CTR_EncDec final : public CryptoAlgorithmEncDec {
public:
    // An ArrayBuffer, a TypedArray, or a DataView — the initial value of the counter block. This must be 16 bytes long (the AES block size). The rightmost length bits of this block are used for the counter, and the rest is used for the nonce. For example, if length is set to 64, then the first half of counter is the nonce and the second half is used for the counter.
    std::span<uint8_t> counter;
    // A Number — the number of bits in the counter block that are used for the actual counter. The counter must be big enough that it doesn't wrap: if the message is n blocks and the counter is m bits long, then the following must be true: n <= 2^m. The NIST SP800-38A standard, which defines CTR, suggests that the counter should occupy half of the counter block (see Appendix B.2), so for AES it would be 64.
    size_t length;

    const char* name() const noexcept override {
    return "AES-CTR";
  };
    CryptoAlgorithmAES_CTR_EncDec(std::span<uint8_t> counter, size_t length): counter{counter}, length{length} {}

    CryptoAlgorithmIdentifier identifier() override {
      return CryptoAlgorithmIdentifier::AES_CTR;
    };

    JSObject* encrypt(JSContext* cx, CryptoKey, std::vector<uint8_t>) override;
    JSObject* decrypt(JSContext* cx, CryptoKey, std::vector<uint8_t>) override;
    JSObject* toObject(JSContext* cx) override;
};

class CryptoAlgorithmAES_CTR_Key final : public CryptoAlgorithmGenerateKey, public CryptoAlgorithmImportKey, public CryptoAlgorithmGetKeyLength {
public:
    // A Number — the length in bits of the key to generate. This must be one of: 128, 192, or 256.
    unsigned short length;

    const char* name() const noexcept override {
    return "AES-CTR";
  };
    CryptoAlgorithmAES_CTR_Key(unsigned short length): length{length} {
      MOZ_ASSERT(length == 128 || length == 192 || length == 256);
    }

    CryptoAlgorithmIdentifier identifier() override {
      return CryptoAlgorithmIdentifier::AES_CTR;
    };

    JSObject* generateKey(JSContext* cx, bool extractable, CryptoKeyUsageBitmap) override;
    JSObject* importKey(JSContext* cx, CryptoKeyFormat, JS::HandleValue, bool extractable, CryptoKeyUsageBitmap) override;
    JSObject* importKey(JSContext* cx, CryptoKeyFormat, KeyData, bool extractable, CryptoKeyUsageBitmap) override;
    JSObject* exportKey(JSContext* cx, CryptoKeyFormat format, JS::HandleObject key);
    size_t getKeyLength(JSContext* cx) override;
  JSObject* toObject(JSContext* cx) override;
};

class CryptoAlgorithmAES_CTR_KeyImport final : public CryptoAlgorithmImportKey {
public:
    const char* name() const noexcept override {
    return "AES-CTR";
  };
    CryptoAlgorithmAES_CTR_KeyImport() {}

    CryptoAlgorithmIdentifier identifier() override {
      return CryptoAlgorithmIdentifier::AES_CTR;
    };

    JSObject* importKey(JSContext* cx, CryptoKeyFormat, JS::HandleValue, bool extractable, CryptoKeyUsageBitmap) override;
    JSObject* importKey(JSContext* cx, CryptoKeyFormat, KeyData, bool extractable, CryptoKeyUsageBitmap) override;
    JSObject* exportKey(JSContext* cx, CryptoKeyFormat format, JS::HandleObject key);
  JSObject* toObject(JSContext* cx) override;
};

class CryptoAlgorithmAES_GCM_EncDec final : public CryptoAlgorithmEncDec {
public:
    // An ArrayBuffer, a TypedArray, or a DataView with the initialization vector. This must be unique for every encryption operation carried out with a given key. Put another way: never reuse an IV with the same key. The AES-GCM specification recommends that the IV should be 96 bits long, and typically contains bits from a random number generator. Section 8.2 of the specification outlines methods for constructing IVs. Note that the IV does not have to be secret, just unique: so it is OK, for example, to transmit it in the clear alongside the encrypted message.
    std::span<uint8_t> iv;
    
    // An ArrayBuffer, a TypedArray, or a DataView. This contains additional data that will not be encrypted but will be authenticated along with the encrypted data. If additionalData is given here then the same data must be given in the corresponding call to decrypt(): if the data given to the decrypt() call does not match the original data, the decryption will throw an exception. This gives you a way to authenticate associated data without having to encrypt it.
    // The bit length of additionalData must be smaller than 2^64 - 1.
    // The additionalData property is optional and may be omitted without compromising the security of the encryption operation.
    std::optional<std::span<uint8_t>> additionalData;


    // A Number. This determines the size in bits of the authentication tag generated in the encryption operation and used for authentication in the corresponding decryption.
    // According to the Web Crypto specification this must have one of the following values: 32, 64, 96, 104, 112, 120, or 128. The AES-GCM specification recommends that it should be 96, 104, 112, 120 or 128, although 32 or 64 bits may be acceptable in some applications: Appendix C of the specification provides additional guidance here.
    // tagLength is optional and defaults to 128 if it is not specified.
    std::optional<uint8_t> tagLength;

    const char* name() const noexcept override {
    return "AES-GCM";
  };
    CryptoAlgorithmAES_GCM_EncDec(std::span<uint8_t> iv, std::optional<std::span<uint8_t>> additionalData, std::optional<uint8_t> tagLength): iv{iv}, additionalData{additionalData}, tagLength{tagLength} {};
    CryptoAlgorithmIdentifier identifier() final {
      return CryptoAlgorithmIdentifier::AES_GCM;
    };

    JSObject* encrypt(JSContext* cx, CryptoKey, std::vector<uint8_t>) final override;
    JSObject* decrypt(JSContext* cx, CryptoKey, std::vector<uint8_t>) final override;
    JSObject* toObject(JSContext* cx) override;
};

class CryptoAlgorithmAES_GCM_Key final : public CryptoAlgorithmGenerateKey, public CryptoAlgorithmGetKeyLength {
public:
    // A Number — the length in bits of the key to generate. This must be one of: 128, 192, or 256.
    unsigned short length;

    const char* name() const noexcept override {
    return "AES-GCM";
  };
    CryptoAlgorithmAES_GCM_Key(unsigned short length): length{length} {};
    CryptoAlgorithmIdentifier identifier() override {
      return CryptoAlgorithmIdentifier::AES_GCM;
    };

    JSObject* generateKey(JSContext* cx, bool extractable, CryptoKeyUsageBitmap) override;
    JSObject* exportKey(JSContext* cx, CryptoKeyFormat format, JS::HandleObject key);
    size_t getKeyLength(JSContext* cx) override;
  JSObject* toObject(JSContext* cx) override;
};

class CryptoAlgorithmAES_GCM_KeyImport final : public CryptoAlgorithmImportKey {
public:

    const char* name() const noexcept override {
    return "AES-GCM";
  };
    CryptoAlgorithmAES_GCM_KeyImport() {};
    CryptoAlgorithmIdentifier identifier() override {
      return CryptoAlgorithmIdentifier::AES_GCM;
    };

    JSObject* importKey(JSContext* cx, CryptoKeyFormat, JS::HandleValue, bool extractable, CryptoKeyUsageBitmap) override;
    JSObject* importKey(JSContext* cx, CryptoKeyFormat, KeyData, bool extractable, CryptoKeyUsageBitmap) override;
    JSObject* exportKey(JSContext* cx, CryptoKeyFormat format, JS::HandleObject key);
  JSObject* toObject(JSContext* cx) override;
};

class CryptoAlgorithmAES_KW final : public CryptoAlgorithmWrapUnwrapkey, public CryptoAlgorithmGenerateKey, public CryptoAlgorithmGetKeyLength {
public:
    // A Number — the length in bits of the key to generate. This must be one of: 128, 192, or 256.
    unsigned short length;

    const char* name() const noexcept override {
    return "AES-KW";
  };
    CryptoAlgorithmAES_KW(unsigned short length): length{length} {};
    CryptoAlgorithmIdentifier identifier() override {
      return CryptoAlgorithmIdentifier::AES_KW;
    };

    JSObject* generateKey(JSContext* cx, bool extractable, CryptoKeyUsageBitmap) override;
    JSObject* exportKey(JSContext* cx, CryptoKeyFormat format, JS::HandleObject key);
    JSObject* wrapKey(JSContext* cx, CryptoKey, std::vector<uint8_t>) override;
    JSObject* unwrapKey(JSContext* cx, CryptoKey, std::vector<uint8_t>) override;
    size_t getKeyLength(JSContext* cx) override;
    JSObject* toObject(JSContext* cx) override;
};

class CryptoAlgorithmAES_KWImport final : public CryptoAlgorithmImportKey {
public:
    const char* name() const noexcept override {
    return "AES-KW";
  };
    CryptoAlgorithmAES_KWImport() {};
    CryptoAlgorithmIdentifier identifier() override {
      return CryptoAlgorithmIdentifier::AES_KW;
    };

    JSObject* importKey(JSContext* cx, CryptoKeyFormat, JS::HandleValue, bool extractable, CryptoKeyUsageBitmap) override;
    JSObject* importKey(JSContext* cx, CryptoKeyFormat, KeyData, bool extractable, CryptoKeyUsageBitmap) override;
    JSObject* exportKey(JSContext* cx, CryptoKeyFormat format, JS::HandleObject key);
    JSObject* toObject(JSContext* cx) override;
};

} // namespace builtins