#ifndef JS_COMPUTE_RUNTIME_BACKEND_H
#define JS_COMPUTE_RUNTIME_BACKEND_H
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "builtin.h"

enum class Authentication {
  RSA,
};

enum class KeyExchange {
  EECDH,
  RSA,
};

enum class Encryption {
  AES128,
  AES128GCM,
  AES256,
  AES256GCM,
  CHACHA20POLY1305,
  TRIPLE_DES,
};

enum class EncryptionLevel {
  MEDIUM,
  HIGH,
};

enum class MessageDigest {
  SHA1,
  SHA256,
  SHA384,
  AEAD,
};

enum class Protocol {
  SSLv3,
  TLSv1,
  TLSv1_2,
};

class Cipher {
  std::string id;
  std::string openSSLAlias;
  KeyExchange kx;
  Authentication au;
  Encryption enc;
  MessageDigest mac;
  Protocol protocol;
  EncryptionLevel level;
  int strength_bits;

public:
  Cipher(std::string openSSLAlias, KeyExchange kx, Authentication au, Encryption enc,
         MessageDigest mac, Protocol protocol, EncryptionLevel level, int strength_bits)
      : openSSLAlias(openSSLAlias), kx(kx), au(au), enc(enc), mac(mac), protocol(protocol),
        level(level), strength_bits(strength_bits) {}

  std::string getOpenSSLAlias() { return openSSLAlias; }

  KeyExchange getKx() { return kx; }

  Authentication getAu() { return au; }

  Encryption getEnc() { return enc; }

  MessageDigest getMac() { return mac; }

  Protocol getProtocol() { return protocol; }

  EncryptionLevel getLevel() { return level; }

  int getStrength_bits() { return strength_bits; }

  // Overload the == operator
  const bool operator==(const Cipher &obj) const {
    return openSSLAlias == obj.openSSLAlias && kx == obj.kx && au == obj.au && enc == obj.enc &&
           mac == obj.mac && protocol == obj.protocol && level == obj.level &&
           strength_bits == obj.strength_bits;
  }
};

/**
 * Class in charge with parsing openSSL expressions to define a list of ciphers.
 */
class OpenSSLCipherConfigurationParser {
public:
  // inline static std::map<std::string, std::vector<Cipher>> aliases;
  inline static bool initialized = false;
  inline static std::string SSL_PROTO_TLSv1_2 = "TLSv1.2";
  inline static std::string SSL_PROTO_TLSv1_0 = "TLSv1.0";
  inline static std::string SSL_PROTO_SSLv3 = "SSLv3";
  inline static std::string SSL_PROTO_TLSv1 = "TLSv1";
  inline static std::string SSL_PROTO_SSLv2 = "SSLv2";

  inline static std::string SEPARATOR = ":, ";
  /**
   * If ! is used then the ciphers are permanently deleted from the list. The ciphers deleted can
   * never reappear in the list even if they are explicitly stated.
   */
  inline static char EXCLUDE = '!';
  /**
   * If - is used then the ciphers are deleted from the list, but some or all of the ciphers can be
   * added again by later options.
   */
  inline static char DELETE = '-';
  /**
   * If + is used then the ciphers are moved to the end of the list. This option doesn't add any new
   * ciphers it just moves matching existing ones.
   */
  inline static char TO_END = '+';
  /**
   * Lists of cipher suites can be combined in a single cipher string using the + character.
   * This is used as a logical and operation.
   * For example SHA1+DES represents all cipher suites containing the SHA1 and the DES algorithms.
   */
  inline static char AND = '+';

  /**
   * 'high' encryption cipher suites. This currently means those with key lengths larger than 128
   * bits, and some cipher suites with 128-bit keys.
   */
  inline static std::string HIGH = "HIGH";
  /**
   * 'medium' encryption cipher suites, currently some of those using 128 bit encryption::
   */
  inline static std::string MEDIUM = "MEDIUM";
  /**
   * Cipher suites using RSA key exchange.
   */
  inline static std::string kRSA = "kRSA";
  /**
   * Cipher suites using RSA authentication::
   */
  inline static std::string aRSA = "aRSA";
  /**
   * Cipher suites using RSA for key exchange
   * Despite what the docs say, RSA is equivalent to kRSA.
   */
  inline static std::string RSA = "RSA";
  /**
   * Cipher suites using ephemeral ECDH key agreement, including anonymous cipher suites.
   */
  inline static std::string kEECDH = "kEECDH";
  /**
   * Cipher suites using ephemeral ECDH key agreement, excluding anonymous cipher suites.
   * Same as "kEECDH:-AECDH"
   */
  inline static std::string EECDH = "EECDH";
  /**
   * Cipher suitesusing ECDH key exchange, including anonymous, ephemeral and fixed ECDH.
   */
  inline static std::string ECDH = "ECDH";
  /**
   * Cipher suites using ephemeral ECDH key agreement, including anonymous cipher suites.
   */
  inline static std::string kECDHE = "kECDHE";
  /**
   * Cipher suites using authenticated ephemeral ECDH key agreement
   */
  inline static std::string ECDHE = "ECDHE";
  /**
   * Cipher suites using 128 bit AES.
   */
  inline static std::string AES128 = "AES128";
  /**
   * Cipher suites using 256 bit AES.
   */
  inline static std::string AES256 = "AES256";
  /**
   * Cipher suites using either 128 or 256 bit AES.
   */
  inline static std::string AES = "AES";
  /**
   * AES in Galois Counter Mode (GCM): these cipher suites are only supported in TLS v1.2.
   */
  inline static std::string AESGCM = "AESGCM";
  /**
   * Cipher suites using CHACHA20.
   */
  inline static std::string CHACHA20 = "CHACHA20";
  /**
   * Cipher suites using triple DES.
   */
  inline static std::string TRIPLE_DES = "3DES";
  /**
   * Cipher suites using SHA1.
   */
  inline static std::string SHA1 = "SHA1";
  /**
   * Cipher suites using SHA1.
   */
  inline static std::string SHA = "SHA";
  /**
   * Cipher suites using SHA256.
   */
  inline static std::string SHA256 = "SHA256";
  /**
   * Cipher suites using SHA384.
   */
  inline static std::string SHA384 = "SHA384";

  // The content of the default list is determined at compile time and normally corresponds to
  // ALL:!COMPLEMENTOFDEFAULT:!eNULL.
  inline static std::string DEFAULT = "DEFAULT";
  // The ciphers included in ALL, but not enabled by default.
  inline static std::string COMPLEMENTOFDEFAULT = "COMPLEMENTOFDEFAULT";

  inline static std::string ALL = "ALL";

  static void init(std::map<std::string, std::vector<Cipher>> *aliases);

  static void moveToEnd(std::map<std::string, std::vector<Cipher>> *aliases,
                        std::vector<Cipher> *ciphers, std::string cipher);

  static void moveToEnd(std::vector<Cipher> *ciphers, std::vector<Cipher> *toBeMovedCiphers);

  static void moveToStart(std::vector<Cipher> *ciphers, std::vector<Cipher> *toBeMovedCiphers);
  static void add(std::map<std::string, std::vector<Cipher>> *aliases, std::vector<Cipher> *ciphers,
                  std::string alias);

  static void remove(std::map<std::string, std::vector<Cipher>> *aliases,
                     std::vector<Cipher> *ciphers, std::string alias);

  static void strengthSort(std::vector<Cipher> *ciphers);

  /*
   * See
   * https://github.com/openssl/openssl/blob/7c96dbcdab959fef74c4caae63cdebaa354ab252/ssl/ssl_ciph.c#L1371
   */
  static std::vector<Cipher> defaultSort(std::vector<Cipher> *ciphers);

  static std::vector<Cipher> filterByStrengthBits(std::vector<Cipher> *ciphers, int strength_bits);

  static std::vector<Cipher> filterByProtocol(std::vector<Cipher> *ciphers,
                                              std::set<Protocol> protocol);

  static std::vector<Cipher> filterByKeyExchange(std::vector<Cipher> *ciphers,
                                                 std::set<KeyExchange> kx);

  static std::vector<Cipher> filterByAuthentication(std::vector<Cipher> *ciphers,
                                                    std::set<Authentication> au);

  static std::vector<Cipher> filterByEncryption(std::vector<Cipher> *ciphers,
                                                std::set<Encryption> enc);

  static std::vector<Cipher> filterByEncryptionLevel(std::vector<Cipher> *ciphers,
                                                     std::set<EncryptionLevel> level);

  static std::vector<Cipher> filterByMessageDigest(std::vector<Cipher> *ciphers,
                                                   std::set<MessageDigest> mac);

  static std::vector<Cipher>
  filter(std::vector<Cipher> *ciphers, std::optional<std::set<Protocol>> protocols,
         std::optional<std::set<KeyExchange>> kx, std::optional<std::set<Authentication>> au,
         std::optional<std::set<Encryption>> enc, std::optional<std::set<EncryptionLevel>> level,
         std::optional<std::set<MessageDigest>> mac);

  static std::vector<std::string> split(std::string s, std::string_view delimiter);

  static std::vector<std::string> splitCipherSuiteString(std::string string);

  static std::vector<Cipher> parse(std::string_view expression);
};

namespace builtins {

class Backend : public BuiltinImpl<Backend> {
private:
public:
  static constexpr const char *class_name = "Backend";
  static const int ctor_length = 1;
  enum Slots {
    Name,
    Target,
    HostOverride,
    ConnectTimeout,
    FirstByteTimeout,
    BetweenBytesTimeout,
    UseSsl,
    TlsMinVersion,
    TlsMaxVersion,
    CertificateHostname,
    CaCertificate,
    Ciphers,
    SniHostname,
    Count
  };

  static const JSFunctionSpec methods[];
  static const JSPropertySpec properties[];

  inline static JS::PersistentRootedObject backends;

  static JSString *name(JSContext *cx, JSObject *self);
  static JS::Result<mozilla::Ok> register_dynamic_backend(JSContext *cx, JS::HandleObject request);
  static JSObject *create(JSContext *cx, JS::HandleObject request);
  static bool set_target(JSContext *cx, JSObject *backend, JS::HandleValue target_val);
  static bool set_timeout_slot(JSContext *cx, JSObject *backend, JS::HandleValue value,
                               Backend::Slots slot, std::string property_name);
  static bool set_host_override(JSContext *cx, JSObject *backend, JS::HandleValue hostOverride_val);
  static bool set_name(JSContext *cx, JSObject *backend, JS::HandleValue name_val);
  static bool toString(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool constructor(JSContext *cx, unsigned argc, JS::Value *vp);

  static bool init_class(JSContext *cx, JS::HandleObject global);
};

} // namespace builtins

#endif
