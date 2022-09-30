#include <set>
#include <string>
#include <unordered_map>
#include <vector>

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

  const std::string &getOpenSSLAlias() { return openSSLAlias; }

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
  // This unordered_map should stay aligned with the canonical list located at:
  // https://developer.fastly.com/learning/concepts/routing-traffic-to-fastly/#use-a-tls-configuration
  // The mapping is from OpenSSL cipher names as strings to a the cipher represented as a Cipher
  // object
  inline static std::unordered_map<std::string, Cipher> CIPHER{
      {"DES-CBC3-SHA", Cipher(std::string("DES-CBC3-SHA"), KeyExchange::RSA, Authentication::RSA,
                              Encryption::TRIPLE_DES, MessageDigest::SHA1, Protocol::SSLv3,
                              EncryptionLevel::MEDIUM, 112)},
      {"AES128-SHA",
       Cipher(std::string("AES128-SHA"), KeyExchange::RSA, Authentication::RSA, Encryption::AES128,
              MessageDigest::SHA1, Protocol::SSLv3, EncryptionLevel::HIGH, 128)},
      {"AES256-SHA",
       Cipher(std::string("AES256-SHA"), KeyExchange::RSA, Authentication::RSA, Encryption::AES256,
              MessageDigest::SHA1, Protocol::SSLv3, EncryptionLevel::HIGH, 256)},
      {"AES128-GCM-SHA256", Cipher(std::string("AES128-GCM-SHA256"), KeyExchange::RSA,
                                   Authentication::RSA, Encryption::AES128GCM, MessageDigest::AEAD,
                                   Protocol::TLSv1_2, EncryptionLevel::HIGH, 128)},
      {"ECDHE-RSA-AES128-SHA", Cipher(std::string("ECDHE-RSA-AES128-SHA"), KeyExchange::EECDH,
                                      Authentication::RSA, Encryption::AES128, MessageDigest::SHA1,
                                      Protocol::TLSv1, EncryptionLevel::HIGH, 128)},
      {"ECDHE-RSA-AES256-SHA", Cipher(std::string("ECDHE-RSA-AES256-SHA"), KeyExchange::EECDH,
                                      Authentication::RSA, Encryption::AES256, MessageDigest::SHA1,
                                      Protocol::TLSv1, EncryptionLevel::HIGH, 256)},
      {"ECDHE-RSA-AES128-SHA256",
       Cipher(std::string("ECDHE-RSA-AES128-SHA256"), KeyExchange::EECDH, Authentication::RSA,
              Encryption::AES128, MessageDigest::SHA256, Protocol::TLSv1_2, EncryptionLevel::HIGH,
              128)},
      {"ECDHE-RSA-AES256-SHA384",
       Cipher(std::string("ECDHE-RSA-AES256-SHA384"), KeyExchange::EECDH, Authentication::RSA,
              Encryption::AES256, MessageDigest::SHA384, Protocol::TLSv1_2, EncryptionLevel::HIGH,
              256)},
      {"ECDHE-RSA-AES128-GCM-SHA256",
       Cipher(std::string("ECDHE-RSA-AES128-GCM-SHA256"), KeyExchange::EECDH, Authentication::RSA,
              Encryption::AES128GCM, MessageDigest::AEAD, Protocol::TLSv1_2, EncryptionLevel::HIGH,
              128)},
      {"ECDHE-RSA-AES256-GCM-SHA384",
       Cipher(std::string("ECDHE-RSA-AES256-GCM-SHA384"), KeyExchange::EECDH, Authentication::RSA,
              Encryption::AES256GCM, MessageDigest::AEAD, Protocol::TLSv1_2, EncryptionLevel::HIGH,
              256)},
      {"ECDHE-RSA-CHACHA20-POLY1305",
       Cipher(std::string("ECDHE-RSA-CHACHA20-POLY1305"), KeyExchange::EECDH, Authentication::RSA,
              Encryption::CHACHA20POLY1305, MessageDigest::AEAD, Protocol::TLSv1_2,
              EncryptionLevel::HIGH, 256)},
  };
  std::unordered_map<std::string, std::vector<Cipher>> aliases;
  static constexpr auto SSL_PROTO_TLSv1_2 = "TLSv1.2";
  static constexpr auto SSL_PROTO_TLSv1_0 = "TLSv1.0";
  static constexpr auto SSL_PROTO_SSLv3 = "SSLv3";
  static constexpr auto SSL_PROTO_TLSv1 = "TLSv1";
  static constexpr auto SSL_PROTO_SSLv2 = "SSLv2";

  static constexpr auto SEPARATOR = ":, ";
  /**
   * If ! is used then the ciphers are permanently deleted from the list. The ciphers deleted can
   * never reappear in the list even if they are explicitly stated.
   */
  static constexpr char EXCLUDE = '!';
  /**
   * If - is used then the ciphers are deleted from the list, but some or all of the ciphers can be
   * added again by later options.
   */
  static constexpr char DELETE = '-';
  /**
   * If + is used then the ciphers are moved to the end of the list. This option doesn't add any new
   * ciphers it just moves matching existing ones.
   */
  static constexpr char TO_END = '+';
  /**
   * Lists of cipher suites can be combined in a single cipher string using the + character.
   * This is used as a logical and operation.
   * For example SHA1+DES represents all cipher suites containing the SHA1 and the DES algorithms.
   */
  static constexpr char AND = '+';

  /**
   * 'high' encryption cipher suites. This currently means those with key lengths larger than 128
   * bits, and some cipher suites with 128-bit keys.
   */
  static constexpr auto HIGH = "HIGH";
  /**
   * 'medium' encryption cipher suites, currently some of those using 128 bit encryption::
   */
  static constexpr auto MEDIUM = "MEDIUM";
  /**
   * Cipher suites using RSA key exchange.
   */
  static constexpr auto kRSA = "kRSA";
  /**
   * Cipher suites using RSA authentication::
   */
  static constexpr auto aRSA = "aRSA";
  /**
   * Cipher suites using RSA for key exchange
   * Despite what the docs say, RSA is equivalent to kRSA.
   */
  static constexpr auto RSA = "RSA";
  /**
   * Cipher suites using ephemeral ECDH key agreement, including anonymous cipher suites.
   */
  static constexpr auto kEECDH = "kEECDH";
  /**
   * Cipher suites using ephemeral ECDH key agreement, excluding anonymous cipher suites.
   * Same as "kEECDH:-AECDH"
   */
  static constexpr auto EECDH = "EECDH";
  /**
   * Cipher suitesusing ECDH key exchange, including anonymous, ephemeral and fixed ECDH.
   */
  static constexpr auto ECDH = "ECDH";
  /**
   * Cipher suites using ephemeral ECDH key agreement, including anonymous cipher suites.
   */
  static constexpr auto kECDHE = "kECDHE";
  /**
   * Cipher suites using authenticated ephemeral ECDH key agreement
   */
  static constexpr auto ECDHE = "ECDHE";
  /**
   * Cipher suites using 128 bit AES.
   */
  static constexpr auto AES128 = "AES128";
  /**
   * Cipher suites using 256 bit AES.
   */
  static constexpr auto AES256 = "AES256";
  /**
   * Cipher suites using either 128 or 256 bit AES.
   */
  static constexpr auto AES = "AES";
  /**
   * AES in Galois Counter Mode (GCM): these cipher suites are only supported in TLS v1.2.
   */
  static constexpr auto AESGCM = "AESGCM";
  /**
   * Cipher suites using CHACHA20.
   */
  static constexpr auto CHACHA20 = "CHACHA20";
  /**
   * Cipher suites using triple DES.
   */
  static constexpr auto TRIPLE_DES = "3DES";
  /**
   * Cipher suites using SHA1.
   */
  static constexpr auto SHA1 = "SHA1";
  /**
   * Cipher suites using SHA1.
   */
  static constexpr auto SHA = "SHA";
  /**
   * Cipher suites using SHA256.
   */
  static constexpr auto SHA256 = "SHA256";
  /**
   * Cipher suites using SHA384.
   */
  static constexpr auto SHA384 = "SHA384";

  // The content of the default list is determined at compile time and normally corresponds to
  // ALL:!COMPLEMENTOFDEFAULT:!eNULL.
  static constexpr auto DEFAULT = "DEFAULT";
  // The ciphers included in ALL, but not enabled by default.
  static constexpr auto COMPLEMENTOFDEFAULT = "COMPLEMENTOFDEFAULT";

  static constexpr auto ALL = "ALL";

  // This function is used to setup all the OpenSSL aliases that we (Fastly) support.
  // Note: A number of aliases are not implemented as they would be empty, such as `LOW`
  static void init(std::unordered_map<std::string, std::vector<Cipher>> *aliases, bool *initialized) {
    *initialized = true;
    std::vector<Cipher> all;
    all.reserve(CIPHER.size());
    for (const auto &any : CIPHER) {
      auto cipher = any.second;
      all.push_back(cipher);
      auto cipherAlias = cipher.getOpenSSLAlias();
      auto alias = aliases->find(cipherAlias);
      if (alias != aliases->end()) {
        alias->second.push_back(cipher);
      } else {
        std::vector<Cipher> list;
        list.push_back(cipher);
        aliases->insert({cipherAlias, list});
      }
      aliases->insert({cipher.getOpenSSLAlias(), std::vector<Cipher>{cipher}});
    }

    // Note: the descriptions of the aliases within the comments are from
    // https://www.openssl.org/docs/manmaster/man1/openssl-ciphers.html

    // All cipher suites except the eNULL ciphers (which must be explicitly enabled if needed).
    // As of OpenSSL 1.0.0, the ALL cipher suites are sensibly ordered by default.
    all = defaultSort(&all);
    aliases->insert({ALL, all});
    // "High" encryption cipher suites. This currently means those with key lengths larger than 128
    // bits, and some cipher suites with 128-bit keys.
    aliases->insert({HIGH, filterByEncryptionLevel(&all, std::set{EncryptionLevel::HIGH})});
    // "Medium" encryption cipher suites, currently some of those using 128 bit encryption.
    aliases->insert({MEDIUM, filterByEncryptionLevel(&all, std::set{EncryptionLevel::MEDIUM})});

    // Cipher suites using RSA key exchange or authentication. RSA is an alias for kRSA.
    auto krsa = filterByKeyExchange(&all, std::set{KeyExchange::RSA});
    aliases->insert({kRSA, krsa});
    auto arsa = filterByAuthentication(&all, std::set{Authentication::RSA});
    aliases->insert({aRSA, arsa});
    aliases->insert({RSA, krsa});

    // Cipher suites using ephemeral ECDH key agreement, including anonymous cipher suites.
    auto ecdh = filterByKeyExchange(&all, std::set{KeyExchange::EECDH});
    aliases->insert({kEECDH, ecdh});
    aliases->insert({kECDHE, ecdh});
    aliases->insert({ECDH, ecdh});
    // Cipher suites using authenticated ephemeral ECDH key agreement.
    aliases->insert({EECDH, ecdh});
    aliases->insert({ECDHE, ecdh});

    // Lists cipher suites which are only supported in at least TLS v1.2, TLS v1.0 or SSL v3.0
    // respectively. Note: there are no cipher suites specific to TLS v1.1. Since this is only the
    // minimum version, if, for example, TLSv1.0 is negotiated then both TLSv1.0 and SSLv3.0 cipher
    // suites are available. Note: these cipher strings do not change the negotiated version of SSL
    // or TLS, they only affect the list of available cipher suites.
    aliases->insert({SSL_PROTO_TLSv1_2, filterByProtocol(&all, std::set{Protocol::TLSv1_2})});
    auto tlsv1 = filterByProtocol(&all, std::set{Protocol::TLSv1});
    aliases->insert({SSL_PROTO_TLSv1_0, tlsv1});
    aliases->insert({SSL_PROTO_TLSv1, tlsv1});
    aliases->insert({SSL_PROTO_SSLv3, filterByProtocol(&all, std::set{Protocol::SSLv3})});

    // cipher suites using 128 bit AES.
    auto aes128 = filterByEncryption(&all, std::set{Encryption::AES128, Encryption::AES128GCM});
    aliases->insert({AES128, aes128});
    // cipher suites using 256 bit AES.
    auto aes256 = filterByEncryption(&all, std::set{Encryption::AES256, Encryption::AES256GCM});
    aliases->insert({AES256, aes256});
    // cipher suites using either 128 or 256 bit AES.
    auto aes(aes128);
    aes.insert(aes.end(), aes256.begin(), aes256.end());
    aliases->insert({AES, aes});

    // AES in Galois Counter Mode (GCM).
    aliases->insert(
        {AESGCM, filterByEncryption(&all, std::set{Encryption::AES128GCM, Encryption::AES256GCM})});

    // Cipher suites using ChaCha20.
    aliases->insert({CHACHA20, filterByEncryption(&all, std::set{Encryption::CHACHA20POLY1305})});

    // Cipher suites using triple DES.
    auto triple_des = filterByEncryption(&all, std::set{Encryption::TRIPLE_DES});
    aliases->insert({TRIPLE_DES, triple_des});

    // Cipher suites using SHA1.
    auto sha1 = filterByMessageDigest(&all, std::set{MessageDigest::SHA1});
    aliases->insert({SHA1, sha1});
    aliases->insert({SHA, sha1});
    // Cipher suites using SHA256.
    aliases->insert({SHA256, filterByMessageDigest(&all, std::set{MessageDigest::SHA256})});
    // Cipher suites using SHA384.
    aliases->insert({SHA384, filterByMessageDigest(&all, std::set{MessageDigest::SHA384})});

    // COMPLEMENTOFDEFAULT:
    // The ciphers included in ALL, but not enabled by default. Currently this includes all RC4 and
    // anonymous ciphers. Note that this rule does not cover eNULL, which is not included by ALL
    // (use COMPLEMENTOFALL if necessary). Note that RC4 based cipher suites are not supported by
    // Fastly and the only supported anonymous ciphers are `ecdh` and `triple_des`.
    auto complementOfDefault(ecdh);
    complementOfDefault.insert(complementOfDefault.end(), triple_des.begin(), triple_des.end());
    aliases->insert({COMPLEMENTOFDEFAULT, complementOfDefault});

    // The content of the default list is determined at compile time and normally corresponds to
    // ALL:!COMPLEMENTOFDEFAULT:!eNULL.
    aliases->insert({DEFAULT, parse("ALL:!COMPLEMENTOFDEFAULT:!eNULL")});
  }

  static void moveToEnd(std::unordered_map<std::string, std::vector<Cipher>> *aliases,
                        std::vector<Cipher> *ciphers, std::string cipher) {
    moveToEnd(ciphers, &aliases->at(cipher));
  }

  static void moveToEnd(std::vector<Cipher> *ciphers, std::vector<Cipher> *ciphersToMoveToEnd) {
    std::stable_partition(ciphers->begin(), ciphers->end(), [ciphersToMoveToEnd](auto cipher) {
      return std::find(ciphersToMoveToEnd->begin(), ciphersToMoveToEnd->end(), cipher) ==
             ciphersToMoveToEnd->end();
    });
  }

  static void moveToStart(std::vector<Cipher> *ciphers, std::vector<Cipher> *ciphersToMoveToStart) {
    std::stable_partition(ciphers->begin(), ciphers->end(), [ciphersToMoveToStart](auto cipher) {
      return std::find(ciphersToMoveToStart->begin(), ciphersToMoveToStart->end(), cipher) !=
             ciphersToMoveToStart->end();
    });
  }

  static void add(std::unordered_map<std::string, std::vector<Cipher>> *aliases,
                  std::vector<Cipher> *ciphers, std::string alias) {
    auto toAdd = aliases->at(alias);
    ciphers->insert(ciphers->end(), toAdd.begin(), toAdd.end());
  }

  static void remove(std::unordered_map<std::string, std::vector<Cipher>> *aliases,
                     std::vector<Cipher> *ciphers, std::string alias) {
    auto toRemove = aliases->at(alias);
    ciphers->erase(std::remove_if(ciphers->begin(), ciphers->end(),
                                  [&](auto x) {
                                    return find(toRemove.begin(), toRemove.end(), x) !=
                                           toRemove.end();
                                  }),
                   ciphers->end());
  }

  static void strengthSort(std::vector<Cipher> *ciphers) {
    /*
     * This routine sorts the ciphers with descending strength. The sorting
     * must keep the pre-sorted sequence, so we apply the normal sorting
     * routine as '+' movement to the end of the list.
     */
    std::vector<int> strength_bits;
    strength_bits.reserve(ciphers->size());
    for (auto cipher : *ciphers) {
      strength_bits.push_back(cipher.getStrength_bits());
    }
    // sort strength_bits in descending order.
    // using reverse iterators with sort was the fastest implementation, tested here:
    // https://quick-bench.com/q/bnRrrc1MvGpidLXcWF3u24hyfqA
    std::sort(strength_bits.rbegin(), strength_bits.rend());
    for (auto strength : strength_bits) {
      auto ciphersToMoveToEnd = filterByStrengthBits(ciphers, strength);
      moveToEnd(ciphers, &ciphersToMoveToEnd);
    }
  }

  /*
   * See
   * https://github.com/openssl/openssl/blob/709651c9022e7be7e69cf8a2f6edf2c8722a6a1e/ssl/ssl_ciph.c#L1455
   */
  static std::vector<Cipher> defaultSort(std::vector<Cipher> *ciphers) {
    std::vector<Cipher> result;
    result.reserve(ciphers->size());
    std::vector<Cipher> ecdh;

    /* Everything else being equal, prefer ephemeral ECDH over other key exchange mechanisms */
    auto eecdh = filterByKeyExchange(ciphers, std::set{KeyExchange::EECDH});
    ecdh.insert(ecdh.end(), eecdh.begin(), eecdh.end());

    /* AES is our preferred symmetric cipher */
    auto aes = {Encryption::AES128, Encryption::AES128GCM, Encryption::AES256,
                Encryption::AES256GCM};

    /* Now arrange all ciphers by preference: */
    auto ecdhaes = filterByEncryption(&ecdh, aes);
    result.insert(result.end(), ecdhaes.begin(), ecdhaes.end());
    auto ciphersaes = filterByEncryption(ciphers, aes);
    result.insert(result.end(), ciphersaes.begin(), ciphersaes.end());

    /* Add everything else */
    result.insert(result.end(), ecdh.begin(), ecdh.end());
    result.insert(result.end(), ciphers->begin(), ciphers->end());

    /* Move ciphers without forward secrecy to the end */
    auto ciphersWithoutForwardSecrecy = filterByKeyExchange(&result, std::set{KeyExchange::RSA});
    moveToEnd(&result, &ciphersWithoutForwardSecrecy);

    strengthSort(&result);
    return result;
  }

  static std::vector<Cipher> filterByStrengthBits(std::vector<Cipher> *ciphers, int strength_bits) {
    std::vector<Cipher> result;
    for (auto cipher : *ciphers) {
      if (cipher.getStrength_bits() == strength_bits) {
        result.push_back(cipher);
      }
    }
    return result;
  }

  static std::vector<Cipher> filterByProtocol(std::vector<Cipher> *ciphers,
                                              std::set<Protocol> protocol) {
    return filter(ciphers, protocol, std::nullopt, std::nullopt, std::nullopt, std::nullopt,
                  std::nullopt);
  }

  std::vector<Cipher> static filterByKeyExchange(std::vector<Cipher> *ciphers,
                                                 std::set<KeyExchange> kx) {
    return filter(ciphers, std::nullopt, kx, std::nullopt, std::nullopt, std::nullopt,
                  std::nullopt);
  }

  std::vector<Cipher> static filterByAuthentication(std::vector<Cipher> *ciphers,
                                                    std::set<Authentication> au) {
    return filter(ciphers, std::nullopt, std::nullopt, au, std::nullopt, std::nullopt,
                  std::nullopt);
  }

  std::vector<Cipher> static filterByEncryption(std::vector<Cipher> *ciphers,
                                                std::set<Encryption> enc) {
    return filter(ciphers, std::nullopt, std::nullopt, std::nullopt, enc, std::nullopt,
                  std::nullopt);
  }

  std::vector<Cipher> static filterByEncryptionLevel(std::vector<Cipher> *ciphers,
                                                     std::set<EncryptionLevel> level) {
    return filter(ciphers, std::nullopt, std::nullopt, std::nullopt, std::nullopt, level,
                  std::nullopt);
  }

  std::vector<Cipher> static filterByMessageDigest(std::vector<Cipher> *ciphers,
                                                   std::set<MessageDigest> mac) {
    return filter(ciphers, std::nullopt, std::nullopt, std::nullopt, std::nullopt, std::nullopt,
                  mac);
  }

  static std::vector<Cipher>
  filter(std::vector<Cipher> *ciphers, std::optional<std::set<Protocol>> protocols,
         std::optional<std::set<KeyExchange>> kx, std::optional<std::set<Authentication>> au,
         std::optional<std::set<Encryption>> enc, std::optional<std::set<EncryptionLevel>> level,
         std::optional<std::set<MessageDigest>> mac) {
    std::vector<Cipher> result;
    for (auto cipher : *ciphers) {
      if (protocols.has_value() && protocols->find(cipher.getProtocol()) != protocols->end()) {
        result.push_back(cipher);
      }
      if (kx.has_value() && kx->find(cipher.getKx()) != kx->end()) {
        result.push_back(cipher);
      }
      if (au.has_value() && au->find(cipher.getAu()) != au->end()) {
        result.push_back(cipher);
      }
      if (enc.has_value() && enc->find(cipher.getEnc()) != enc->end()) {
        result.push_back(cipher);
      }
      if (level.has_value() && level->find(cipher.getLevel()) != level->end()) {
        result.push_back(cipher);
      }
      if (mac.has_value() && mac->find(cipher.getMac()) != mac->end()) {
        result.push_back(cipher);
      }
    }
    return result;
  }

  static std::vector<std::string> split(std::string s, std::string_view delimiter) {
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    std::string token;
    std::vector<std::string> res;

    while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
      token = s.substr(pos_start, pos_end - pos_start);
      pos_start = pos_end + delim_len;
      res.push_back(token);
    }

    res.push_back(s.substr(pos_start));
    return res;
  }

  static std::vector<Cipher> parse(std::string_view expression) {
    /**
     * All ciphers by their openssl alias name.
     */
    static std::unordered_map<std::string, std::vector<Cipher>> aliases;
    static bool *initialized;
    if (!*initialized) {
      init(&aliases, initialized);
    }
    auto elements = splitCipherSuiteString(std::string(expression));
    std::vector<Cipher> ciphers;
    std::vector<Cipher> removedCiphers;
    for (auto element : elements) {

      if (element.rfind(DELETE, 0) == 0) {
        auto alias = element.substr(1);
        if (aliases.find(alias) != aliases.end()) {
          remove(&aliases, &ciphers, alias);
        }
      } else if (element.rfind(EXCLUDE, 0) == 0) {
        auto alias = element.substr(1);
        if (aliases.find(alias) != aliases.end()) {
          auto toAdd = aliases[alias];
          removedCiphers.insert(removedCiphers.end(), toAdd.begin(), toAdd.end());
        }
      } else if (element.rfind(TO_END, 0) == 0) {
        auto alias = element.substr(1);
        if (aliases.find(alias) != aliases.end()) {
          moveToEnd(&aliases, &ciphers, alias);
        }
      } else if ("@STRENGTH" == element) {
        strengthSort(&ciphers);
        break;
      } else if (aliases.find(element) != aliases.end()) {
        add(&aliases, &ciphers, element);
      } else if (element.find(AND) != std::string::npos) {
        auto intersections = split(element, std::string(1, AND).append("\\"));
        if (intersections.size() > 0 && aliases.find(intersections[0]) != aliases.end()) {
          auto result = aliases[intersections[0]];
          for (int i = 1; i < intersections.size(); i++) {
            auto alias = aliases.find(intersections[i]);
            if (alias != aliases.end()) {
              // make `result` only contain the aliases from `alias`
              result.erase(std::remove_if(result.begin(), result.end(),
                                          [&](auto x) {
                                            return std::find(alias->second.begin(),
                                                             alias->second.end(),
                                                             x) != alias->second.end();
                                          }),
                           result.end());
            }
          }
          // Add all of `result` onto `ciphers`
          ciphers.insert(ciphers.end(), result.begin(), result.end());
        }
      }
    }
    // Remove all ciphers from `ciphers` which are contained in `removedCiphers`
    ciphers.erase(std::remove_if(ciphers.begin(), ciphers.end(),
                                 [removedCiphers](Cipher c) {
                                   return std::find(removedCiphers.begin(), removedCiphers.end(),
                                                    c) != removedCiphers.end();
                                 }),
                  ciphers.end());
    return ciphers;
  }

  static std::vector<std::string> splitCipherSuiteString(std::string string) {
    std::vector<std::string> result;
    std::stringstream stringStream(string);
    std::string line;
    while (std::getline(stringStream, line, ':')) {
      std::size_t prev = 0, pos;
      while ((pos = line.find_first_of(" ,", prev)) != std::string::npos) {
        if (pos > prev) {
          result.push_back(line.substr(prev, pos - prev));
        }
        prev = pos + 1;
      }
      if (prev < line.length()) {
        result.push_back(line.substr(prev, std::string::npos));
      }
    }
    return result;
  }
};
