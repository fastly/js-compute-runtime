#include <algorithm>
#include <arpa/inet.h>
#include <cctype>
#include <charconv>
#include <iostream>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

// TODO: remove these once the warnings are fixed
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-offsetof"
#pragma clang diagnostic ignored "-Wdeprecated-enum-enum-conversion"
#include "js/experimental/TypedData.h"
#pragma clang diagnostic pop

#include "./secret-store.h"
#include "builtins/backend.h"
#include "builtins/request-response.h"
#include "core/encode.h"
#include "fastly.h"
#include "js-compute-builtins.h"
#include "js/Conversions.h"

enum class Authentication : uint8_t {
  RSA,
};

enum class KeyExchange : uint8_t {
  EECDH,
  RSA,
};

enum class Encryption : uint8_t {
  AES128,
  AES128GCM,
  AES256,
  AES256GCM,
  CHACHA20POLY1305,
  TRIPLE_DES,
};

enum class EncryptionLevel : uint8_t {
  MEDIUM,
  HIGH,
};

enum class MessageDigest : uint8_t {
  SHA1,
  SHA256,
  SHA384,
  AEAD,
};

enum class Protocol : uint8_t {
  SSLv3,
  TLSv1,
  TLSv1_2,
};

class Cipher {
public:
  std::string_view open_ssl_alias;
  KeyExchange kx;
  Authentication au;
  Encryption enc;
  MessageDigest mac;
  Protocol protocol;
  EncryptionLevel level;
  uint16_t strength_bits;

  constexpr Cipher(std::string_view open_ssl_alias, KeyExchange kx, Authentication au,
                   Encryption enc, MessageDigest mac, Protocol protocol, EncryptionLevel level,
                   int strength_bits)
      : open_ssl_alias(open_ssl_alias), kx(kx), au(au), enc(enc), mac(mac), protocol(protocol),
        level(level), strength_bits(strength_bits) {}

  // Overload the == operator
  const bool operator==(const Cipher &obj) const {
    return open_ssl_alias == obj.open_ssl_alias && kx == obj.kx && au == obj.au && enc == obj.enc &&
           mac == obj.mac && protocol == obj.protocol && level == obj.level &&
           strength_bits == obj.strength_bits;
  }
};

/**
 * Class in charge with parsing openSSL expressions to define a list of ciphers.
 */
class OpenSSLCipherConfigurationParser {
private:
  using AliasMap = std::unordered_map<std::string_view, std::vector<Cipher>>;
  AliasMap aliases;
  // This array should stay aligned with the canonical list located at:
  // https://developer.fastly.com/learning/concepts/routing-traffic-to-fastly/#use-a-tls-configuration
  // The mapping is from OpenSSL cipher names as strings to a the cipher represented as a Cipher
  // object
  static constexpr std::array<std::pair<std::string_view, Cipher>, 11> CIPHER{
      std::pair<std::string_view, Cipher>{
          "DES-CBC3-SHA", Cipher(std::string_view("DES-CBC3-SHA"), KeyExchange::RSA,
                                 Authentication::RSA, Encryption::TRIPLE_DES, MessageDigest::SHA1,
                                 Protocol::SSLv3, EncryptionLevel::MEDIUM, 112)},
      {"AES128-SHA", Cipher(std::string_view("AES128-SHA"), KeyExchange::RSA, Authentication::RSA,
                            Encryption::AES128, MessageDigest::SHA1, Protocol::SSLv3,
                            EncryptionLevel::HIGH, 128)},
      {"AES256-SHA", Cipher(std::string_view("AES256-SHA"), KeyExchange::RSA, Authentication::RSA,
                            Encryption::AES256, MessageDigest::SHA1, Protocol::SSLv3,
                            EncryptionLevel::HIGH, 256)},
      {"AES128-GCM-SHA256", Cipher(std::string_view("AES128-GCM-SHA256"), KeyExchange::RSA,
                                   Authentication::RSA, Encryption::AES128GCM, MessageDigest::AEAD,
                                   Protocol::TLSv1_2, EncryptionLevel::HIGH, 128)},
      {"ECDHE-RSA-AES128-SHA", Cipher(std::string_view("ECDHE-RSA-AES128-SHA"), KeyExchange::EECDH,
                                      Authentication::RSA, Encryption::AES128, MessageDigest::SHA1,
                                      Protocol::TLSv1, EncryptionLevel::HIGH, 128)},
      {"ECDHE-RSA-AES256-SHA", Cipher(std::string_view("ECDHE-RSA-AES256-SHA"), KeyExchange::EECDH,
                                      Authentication::RSA, Encryption::AES256, MessageDigest::SHA1,
                                      Protocol::TLSv1, EncryptionLevel::HIGH, 256)},
      {"ECDHE-RSA-AES128-SHA256",
       Cipher(std::string_view("ECDHE-RSA-AES128-SHA256"), KeyExchange::EECDH, Authentication::RSA,
              Encryption::AES128, MessageDigest::SHA256, Protocol::TLSv1_2, EncryptionLevel::HIGH,
              128)},
      {"ECDHE-RSA-AES256-SHA384",
       Cipher(std::string_view("ECDHE-RSA-AES256-SHA384"), KeyExchange::EECDH, Authentication::RSA,
              Encryption::AES256, MessageDigest::SHA384, Protocol::TLSv1_2, EncryptionLevel::HIGH,
              256)},
      {"ECDHE-RSA-AES128-GCM-SHA256",
       Cipher(std::string_view("ECDHE-RSA-AES128-GCM-SHA256"), KeyExchange::EECDH,
              Authentication::RSA, Encryption::AES128GCM, MessageDigest::AEAD, Protocol::TLSv1_2,
              EncryptionLevel::HIGH, 128)},
      {"ECDHE-RSA-AES256-GCM-SHA384",
       Cipher(std::string_view("ECDHE-RSA-AES256-GCM-SHA384"), KeyExchange::EECDH,
              Authentication::RSA, Encryption::AES256GCM, MessageDigest::AEAD, Protocol::TLSv1_2,
              EncryptionLevel::HIGH, 256)},
      {"ECDHE-RSA-CHACHA20-POLY1305",
       Cipher(std::string_view("ECDHE-RSA-CHACHA20-POLY1305"), KeyExchange::EECDH,
              Authentication::RSA, Encryption::CHACHA20POLY1305, MessageDigest::AEAD,
              Protocol::TLSv1_2, EncryptionLevel::HIGH, 256)},
  };

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
   * Cipher suites using RSA authentication.
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

  void move_to_end(const AliasMap &aliases, std::vector<Cipher> &ciphers,
                   std::string_view cipher) const {
    this->move_to_end(ciphers, aliases.at(cipher));
  }

  void move_to_end(std::vector<Cipher> &ciphers,
                   const std::vector<Cipher> &ciphers_to_move_to_end) const {
    std::stable_partition(ciphers.begin(), ciphers.end(), [&ciphers_to_move_to_end](auto &cipher) {
      return std::find(ciphers_to_move_to_end.begin(), ciphers_to_move_to_end.end(), cipher) ==
             ciphers_to_move_to_end.end();
    });
  }

  void add(const AliasMap &aliases, std::vector<Cipher> &ciphers, std::string_view alias) const {
    auto to_add = aliases.at(alias);
    ciphers.insert(ciphers.end(), to_add.begin(), to_add.end());
  }

  void remove(const AliasMap &aliases, std::vector<Cipher> &ciphers, std::string_view alias) const {
    auto &to_remove = aliases.at(alias);
    ciphers.erase(std::remove_if(ciphers.begin(), ciphers.end(),
                                 [&](auto &x) {
                                   return std::find(to_remove.begin(), to_remove.end(), x) !=
                                          to_remove.end();
                                 }),
                  ciphers.end());
  }

  void strength_sort(std::vector<Cipher> &ciphers) const {
    /*
     * This routine sorts the ciphers with descending strength. The sorting
     * must keep the pre-sorted sequence.
     */
    std::stable_sort(ciphers.begin(), ciphers.end(),
                     [](auto &l, auto &r) { return l.strength_bits > r.strength_bits; });
  }

  /*
   * See
   * https://github.com/openssl/openssl/blob/709651c9022e7be7e69cf8a2f6edf2c8722a6a1e/ssl/ssl_ciph.c#L1455
   */
  void default_sort(std::vector<Cipher> &ciphers) const {
    auto by_strength = [](auto &l, auto &r) { return l.strength_bits > r.strength_bits; };
    // order all ciphers by strength first
    std::sort(ciphers.begin(), ciphers.end(), by_strength);

    auto it = std::stable_partition(ciphers.begin(), ciphers.end(),
                                    this->by_key_exchange(KeyExchange::EECDH));

    /* AES is our preferred symmetric cipher */
    auto aes = {Encryption::AES128, Encryption::AES128GCM, Encryption::AES256,
                Encryption::AES256GCM};

    /* Now arrange all ciphers by preference: */
    it = std::stable_partition(it, ciphers.end(), this->by_encryption(aes));

    /* Move ciphers without forward secrecy to the end */;
    std::stable_partition(
        it, ciphers.end(),
        [compare = this->by_key_exchange(KeyExchange::RSA)](auto &c) { return !compare(c); });
  }

  std::function<bool(const Cipher &)> by_protocol(Protocol val) const {
    return [val](auto &c) { return c.protocol == val; };
  }

  std::function<bool(const Cipher &)> by_key_exchange(KeyExchange val) const {
    return [val](auto &c) { return c.kx == val; };
  }

  std::function<bool(const Cipher &)> by_authentication(Authentication val) const {
    return [val](auto &c) { return c.au == val; };
  }

  std::function<bool(const Cipher &)> by_encryption(std::set<Encryption> vals) const {
    return [vals](auto &c) { return vals.find(c.enc) != vals.end(); };
  }

  std::function<bool(const Cipher &)> by_encryption(Encryption val) const {
    return [val](auto &c) { return c.enc == val; };
  }

  std::function<bool(const Cipher &)> by_encryption_level(EncryptionLevel val) const {
    return [val](auto &c) { return c.level == val; };
  }

  std::function<bool(const Cipher &)> by_message_digest(MessageDigest val) const {
    return [val](auto &c) { return c.mac == val; };
  }

  std::vector<std::string_view> split(std::string_view s, std::string_view delimiter) const {
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    std::string token;
    std::vector<std::string_view> res;

    while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
      token = s.substr(pos_start, pos_end - pos_start);
      pos_start = pos_end + delim_len;
      res.push_back(token);
    }

    res.push_back(s.substr(pos_start));
    return res;
  }

  std::pair<std::string_view, std::string_view> split_on(std::string_view str, char c) const {
    auto ix = str.find(c);
    if (ix == str.npos) {
      return {str, ""};
    }

    auto left = str.substr(0, ix);
    ix++;
    if (ix >= str.size()) {
      return {left, ""};
    }

    return {left, str.substr(ix)};
  }

  std::vector<std::string_view> split_cipher_suite_string(std::string_view string) const {
    std::vector<std::string_view> result;

    while (!string.empty()) {
      auto [line, rest] = this->split_on(string, ':');
      string = rest;

      while (!line.empty()) {
        auto [part, rest] = this->split_on(line, ',');
        line = rest;
        result.push_back(part);
      }
    }

    return result;
  }

  std::vector<Cipher> all;

public:
  OpenSSLCipherConfigurationParser() {
    this->all.reserve(CIPHER.size());
    for (const auto &any : CIPHER) {
      auto &cipher = any.second;
      this->all.push_back(cipher);
      auto cipher_alias = cipher.open_ssl_alias;
      auto alias = aliases.find(cipher_alias);
      if (alias != aliases.end()) {
        alias->second.push_back(cipher);
      } else {
        std::vector<Cipher> list;
        list.push_back(cipher);
        aliases.insert({cipher_alias, list});
      }
      aliases.insert({cipher.open_ssl_alias, std::vector<Cipher>{cipher}});
    }

    // Note: the descriptions of the aliases within the comments are from
    // https://www.openssl.org/docs/manmaster/man1/openssl-ciphers.html

    // All cipher suites except the eNULL ciphers (which must be explicitly enabled if needed).
    // As of OpenSSL 1.0.0, the ALL cipher suites are sensibly ordered by default.
    this->default_sort(this->all);
    aliases.insert({ALL, this->all});
    // "High" encryption cipher suites. This currently means those with key lengths larger than 128
    // bits, and some cipher suites with 128-bit keys.
    std::vector<Cipher> high;
    std::copy_if(this->all.begin(), this->all.end(), std::back_inserter(high),
                 this->by_encryption_level(EncryptionLevel::HIGH));
    aliases.insert({HIGH, high});
    // "Medium" encryption cipher suites, currently some of those using 128 bit encryption.
    std::vector<Cipher> medium;
    std::copy_if(this->all.begin(), this->all.end(), std::back_inserter(medium),
                 this->by_encryption_level(EncryptionLevel::MEDIUM));
    aliases.insert({MEDIUM, medium});

    // Cipher suites using RSA key exchange or authentication. RSA is an alias for kRSA.
    std::vector<Cipher> krsa;
    std::copy_if(this->all.begin(), this->all.end(), std::back_inserter(krsa),
                 this->by_key_exchange(KeyExchange::RSA));
    aliases.insert({kRSA, krsa});
    std::vector<Cipher> arsa;
    std::copy_if(this->all.begin(), this->all.end(), std::back_inserter(arsa),
                 this->by_authentication(Authentication::RSA));
    aliases.insert({aRSA, arsa});
    aliases.insert({RSA, krsa});

    // Cipher suites using ephemeral ECDH key agreement, including anonymous cipher suites.
    std::vector<Cipher> ecdh;
    std::copy_if(this->all.begin(), this->all.end(), std::back_inserter(ecdh),
                 this->by_key_exchange(KeyExchange::EECDH));
    aliases.insert({kEECDH, ecdh});
    aliases.insert({kECDHE, ecdh});
    aliases.insert({ECDH, ecdh});
    // Cipher suites using authenticated ephemeral ECDH key agreement.
    aliases.insert({EECDH, ecdh});
    aliases.insert({ECDHE, ecdh});

    // Lists cipher suites which are only supported in at least TLS v1.2, TLS v1.0 or SSL v3.0
    // respectively. Note: there are no cipher suites specific to TLS v1.1. Since this is only the
    // minimum version, if, for example, TLSv1.0 is negotiated then both TLSv1.0 and SSLv3.0 cipher
    // suites are available. Note: these cipher strings do not change the negotiated version of SSL
    // or TLS, they only affect the list of available cipher suites.
    std::vector<Cipher> tlsv2;
    std::copy_if(this->all.begin(), this->all.end(), std::back_inserter(tlsv2),
                 this->by_protocol(Protocol::TLSv1_2));
    aliases.insert({SSL_PROTO_TLSv1_2, tlsv2});
    std::vector<Cipher> tlsv1;
    std::copy_if(this->all.begin(), this->all.end(), std::back_inserter(tlsv1),
                 this->by_protocol(Protocol::TLSv1));
    aliases.insert({SSL_PROTO_TLSv1_0, tlsv1});
    aliases.insert({SSL_PROTO_TLSv1, tlsv1});
    std::vector<Cipher> sslv3;
    std::copy_if(this->all.begin(), this->all.end(), std::back_inserter(sslv3),
                 this->by_protocol(Protocol::SSLv3));
    aliases.insert({SSL_PROTO_SSLv3, sslv3});

    // cipher suites using 128 bit AES.
    std::vector<Cipher> aes128;
    std::copy_if(this->all.begin(), this->all.end(), std::back_inserter(aes128),
                 this->by_encryption({Encryption::AES128, Encryption::AES128GCM}));
    aliases.insert({AES128, aes128});
    // cipher suites using 256 bit AES.
    std::vector<Cipher> aes256;
    std::copy_if(this->all.begin(), this->all.end(), std::back_inserter(aes256),
                 this->by_encryption({Encryption::AES256, Encryption::AES256GCM}));
    aliases.insert({AES256, aes256});
    // cipher suites using either 128 or 256 bit AES.
    auto aes(aes128);
    aes.insert(aes.end(), aes256.begin(), aes256.end());
    aliases.insert({AES, aes});

    // AES in Galois Counter Mode (GCM).
    std::vector<Cipher> aesgcm;
    std::copy_if(this->all.begin(), this->all.end(), std::back_inserter(aesgcm),
                 this->by_encryption({Encryption::AES128GCM, Encryption::AES256GCM}));
    aliases.insert({AESGCM, aesgcm});

    // Cipher suites using ChaCha20.
    std::vector<Cipher> chacha20;
    std::copy_if(this->all.begin(), this->all.end(), std::back_inserter(chacha20),
                 this->by_encryption(Encryption::CHACHA20POLY1305));
    aliases.insert({CHACHA20, chacha20});

    // Cipher suites using triple DES.
    std::vector<Cipher> triple_des;
    std::copy_if(this->all.begin(), this->all.end(), std::back_inserter(triple_des),
                 this->by_encryption(Encryption::TRIPLE_DES));
    aliases.insert({TRIPLE_DES, triple_des});

    // Cipher suites using SHA1.
    std::vector<Cipher> sha1;
    std::copy_if(this->all.begin(), this->all.end(), std::back_inserter(sha1),
                 this->by_message_digest(MessageDigest::SHA1));
    aliases.insert({SHA1, sha1});
    aliases.insert({SHA, sha1});
    // Cipher suites using SHA256.
    std::vector<Cipher> sha256;
    std::copy_if(this->all.begin(), this->all.end(), std::back_inserter(sha256),
                 this->by_message_digest(MessageDigest::SHA256));
    aliases.insert({SHA256, sha256});
    // Cipher suites using SHA384.
    std::vector<Cipher> sha384;
    std::copy_if(this->all.begin(), this->all.end(), std::back_inserter(sha384),
                 this->by_message_digest(MessageDigest::SHA384));
    aliases.insert({SHA384, sha384});

    // COMPLEMENTOFDEFAULT:
    // The ciphers included in ALL, but not enabled by default. Currently this includes all RC4 and
    // anonymous ciphers. Note that this rule does not cover eNULL, which is not included by ALL
    // (use COMPLEMENTOFALL if necessary). Note that RC4 based cipher suites are not supported by
    // Fastly and the only supported anonymous ciphers are `ecdh` and `triple_des`.
    auto complement_of_default(ecdh);
    complement_of_default.insert(complement_of_default.end(), triple_des.begin(), triple_des.end());
    aliases.insert({COMPLEMENTOFDEFAULT, complement_of_default});

    // The content of the default list is determined at compile time and normally corresponds to
    // ALL:!COMPLEMENTOFDEFAULT:!eNULL.
    aliases.insert({DEFAULT, this->parse("ALL:!COMPLEMENTOFDEFAULT:!eNULL")});
  }

  std::vector<Cipher> parse(std::string_view expression) const {
    /**
     * All ciphers by their openssl alias name.
     */
    auto elements = this->split_cipher_suite_string(expression);
    std::vector<Cipher> ciphers;
    std::vector<Cipher> removed_ciphers;
    for (auto &element : elements) {

      if (element.rfind(DELETE, 0) == 0) {
        auto alias = element.substr(1);
        if (aliases.find(alias) != aliases.end()) {
          this->remove(aliases, ciphers, alias);
        }
      } else if (element.rfind(EXCLUDE, 0) == 0) {
        auto alias = element.substr(1);
        auto found = aliases.find(alias);
        if (found != aliases.end()) {

          auto to_add = found.operator->()->second;
          removed_ciphers.insert(removed_ciphers.end(), to_add.begin(), to_add.end());
        }
      } else if (element.rfind(TO_END, 0) == 0) {
        auto alias = element.substr(1);
        if (aliases.find(alias) != aliases.end()) {
          this->move_to_end(aliases, ciphers, alias);
        }
      } else if ("@STRENGTH" == element) {
        this->strength_sort(ciphers);
        break;
      } else if (aliases.find(element) != aliases.end()) {
        this->add(aliases, ciphers, element);
      } else if (element.find(AND) != std::string::npos) {
        auto intersections = this->split(element, "+\\");
        if (intersections.size() > 0) {
          auto found = aliases.find(intersections[0]);
          if (found != aliases.end()) {
            auto result{found.operator->()->second};
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
    }
    // Remove all ciphers from `ciphers` which are contained in `removed_ciphers`
    ciphers.erase(std::remove_if(ciphers.begin(), ciphers.end(),
                                 [&removed_ciphers](auto &c) {
                                   return std::find(removed_ciphers.begin(), removed_ciphers.end(),
                                                    c) != removed_ciphers.end();
                                 }),
                  ciphers.end());
    return ciphers;
  }
};

std::vector<std::string_view> split(std::string_view string, char delimiter) {
  auto start = 0;
  auto end = string.find(delimiter, start);
  std::vector<std::string_view> result;
  while (end != std::string::npos) {
    result.push_back(string.substr(start, end - start));
    start = end + 1;
    end = string.find(delimiter, start);
  }
  result.push_back(string.substr(start));
  return result;
}

bool is_valid_ip(std::string_view ip) {
  int format = AF_INET;
  if (ip.find(':') != std::string::npos) {
    format = AF_INET6;
  }

  char octets[sizeof(struct in6_addr)];
  if (inet_pton(format, ip.data(), octets) != 1) {
    return false;
  }
  return true;
}

// A "host" is a "hostname" and an optional "port" in the format hostname:port
// A "hostname" is between 1 and 255 octets -- https://www.rfc-editor.org/rfc/rfc1123#page-13
// A "hostname" must start with a letter or digit -- https://www.rfc-editor.org/rfc/rfc1123#page-13
// A "hostname" is made up of "labels" delimited by a dot `.`
// A "label" is between 1 and 63 octets
bool is_valid_host(std::string_view host) {
  if (host.length() < 1) {
    return false;
  }
  auto first_character = host.front();
  // check first character is in the regex [a-zA-Z0-9]
  if (!std::isalnum(first_character)) {
    return false;
  }
  // split the hostname from the port
  int pos = host.find_first_of(':');
  std::string_view hostname = host.substr(0, pos);

  // hostnames can not be longer than 253 characters
  // This is because a hostname is represented as a series of labels, and is terminated by a label
  // of length zero. A label consists of a length octet followed by that number of octets
  // representing the name itself. https://www.rfc-editor.org/rfc/rfc1035#section-3.3
  // https://www.rfc-editor.org/rfc/rfc2181#section-11
  if (hostname.length() > 253) {
    return false;
  }

  auto last_character = hostname.back();
  // check last character is in the regex [a-zA-Z0-9]
  if (!std::isalnum(last_character)) {
    return false;
  }

  auto labels = split(hostname, '.');

  for (auto &label : labels) {
    // Each label in a hostname can not be longer than 63 characters
    // https://www.rfc-editor.org/rfc/rfc2181#section-11
    if (label.length() > 63) {
      return false;
    }

    // Each label can only contain the characters in the regex [a-zA-Z0-9\-]
    auto it = std::find_if_not(label.begin(), label.end(), [](auto character) {
      return std::isalnum(character) || character == '-';
    });
    if (it != label.end()) {

      return false;
    }
  }

  // if there is a port - confirm it is all digits and is between 0 and 65536
  // https://www.iana.org/assignments/service-names-port-numbers/service-names-port-numbers.xhtml
  if (pos != std::string::npos) {
    std::string_view port = host.substr(pos + 1);
    if (!std::all_of(port.begin(), port.end(), [](auto c) { return std::isdigit(c); })) {
      return false;
    }
    int value;
    const std::from_chars_result result =
        std::from_chars(port.data(), port.data() + port.size(), value);
    if (result.ec == std::errc::invalid_argument || result.ec == std::errc::result_out_of_range) {
      return false;
    }
    if (value == 0 || value >= 65536) {
      return false;
    }
  }
  return true;
}

OpenSSLCipherConfigurationParser cipher_parser;

namespace builtins {

bool Backend::is_cipher_suite_supported_by_fastly(std::string_view cipher_spec) {
  auto ciphers = cipher_parser.parse(cipher_spec);
  return ciphers.size() > 0;
}
JS::Result<mozilla::Ok> Backend::register_dynamic_backend(JSContext *cx, JS::HandleObject backend) {
  MOZ_ASSERT(is_instance(backend));

  JS::RootedString name(cx, JS::GetReservedSlot(backend, Backend::Slots::Name).toString());
  auto name_chars = core::encode(cx, name);
  std::string_view name_str = name_chars;

  JS::RootedString target(cx, JS::GetReservedSlot(backend, Backend::Slots::Target).toString());
  auto target_chars = core::encode(cx, target);
  std::string_view target_str = target_chars;

  host_api::BackendConfig backend_config;

  auto host_override_slot = JS::GetReservedSlot(backend, Backend::Slots::HostOverride);
  if (!host_override_slot.isNullOrUndefined()) {
    JS::RootedString host_override(cx, host_override_slot.toString());
    auto host_override_chars = core::encode(cx, host_override);
    backend_config.host_override.emplace(std::move(host_override_chars));
  }

  auto connect_timeout_slot = JS::GetReservedSlot(backend, Backend::Slots::ConnectTimeout);
  if (!connect_timeout_slot.isNullOrUndefined()) {
    backend_config.connect_timeout = connect_timeout_slot.toInt32();
  }

  auto first_byte_timeout_slot = JS::GetReservedSlot(backend, Backend::Slots::FirstByteTimeout);
  if (!first_byte_timeout_slot.isNullOrUndefined()) {
    backend_config.first_byte_timeout = first_byte_timeout_slot.toInt32();
  }

  auto between_bytes_timeout_slot =
      JS::GetReservedSlot(backend, Backend::Slots::BetweenBytesTimeout);
  if (!between_bytes_timeout_slot.isNullOrUndefined()) {
    backend_config.between_bytes_timeout = between_bytes_timeout_slot.toInt32();
  }

  auto use_ssl_slot = JS::GetReservedSlot(backend, Backend::Slots::UseSsl);
  if (!use_ssl_slot.isNullOrUndefined()) {
    backend_config.use_ssl = use_ssl_slot.toBoolean();
  }

  auto dont_pool_slot = JS::GetReservedSlot(backend, Backend::Slots::DontPool);
  if (!dont_pool_slot.isNullOrUndefined()) {
    backend_config.dont_pool = dont_pool_slot.toBoolean();
  }

  auto tls_min_version = JS::GetReservedSlot(backend, Backend::Slots::TlsMinVersion);
  if (!tls_min_version.isNullOrUndefined()) {
    backend_config.ssl_min_version = host_api::TlsVersion(tls_min_version.toInt32());
  }

  auto tls_max_version = JS::GetReservedSlot(backend, Backend::Slots::TlsMaxVersion);
  if (!tls_max_version.isNullOrUndefined()) {
    backend_config.ssl_max_version = host_api::TlsVersion(tls_max_version.toInt32());
  }

  auto certificate_hostname_slot =
      JS::GetReservedSlot(backend, Backend::Slots::CertificateHostname);
  if (!certificate_hostname_slot.isNullOrUndefined()) {
    JS::RootedString certificate_hostname_string(cx, certificate_hostname_slot.toString());
    auto certificate_hostname_chars = core::encode(cx, certificate_hostname_string);
    backend_config.cert_hostname.emplace(std::move(certificate_hostname_chars));
  }

  auto ca_certificate_slot = JS::GetReservedSlot(backend, Backend::Slots::CaCertificate);
  if (!ca_certificate_slot.isNullOrUndefined()) {
    JS::RootedString ca_certificate_string(cx, ca_certificate_slot.toString());
    auto ca_certificate_chars = core::encode(cx, ca_certificate_string);
    backend_config.ca_cert.emplace(std::move(ca_certificate_chars));
  }

  auto ciphers_slot = JS::GetReservedSlot(backend, Backend::Slots::Ciphers);
  if (!ciphers_slot.isNullOrUndefined()) {
    JS::RootedString ciphers_string(cx, ciphers_slot.toString());
    auto ciphers_chars = core::encode(cx, ciphers_string);
    backend_config.ciphers.emplace(std::move(ciphers_chars));
  }

  auto sni_hostname_slot = JS::GetReservedSlot(backend, Backend::Slots::SniHostname);
  if (!sni_hostname_slot.isNullOrUndefined()) {
    JS::RootedString sni_hostname_string(cx, sni_hostname_slot.toString());
    auto sni_hostname_chars = core::encode(cx, sni_hostname_string);
    backend_config.sni_hostname.emplace(std::move(sni_hostname_chars));
  }

  auto client_cert_slot = JS::GetReservedSlot(backend, Backend::Slots::ClientCert);
  if (!client_cert_slot.isNullOrUndefined()) {
    JS::RootedString client_cert_string(cx, client_cert_slot.toString());
    auto client_cert_chars = core::encode(cx, client_cert_string);

    auto client_cert_key_slot = JS::GetReservedSlot(backend, Backend::Slots::ClientCertKey);

    backend_config.client_cert =
        host_api::ClientCert{.cert = std::move(client_cert_chars),
                             .key = host_api::Secret(client_cert_key_slot.toInt32())};
  }

  auto res = host_api::HttpReq::register_dynamic_backend(name_str, target_str, backend_config);
  if (auto *err = res.to_err()) {
    HANDLE_ERROR(cx, *err);
    return JS::Result<mozilla::Ok>(JS::Error());
  }
  return mozilla::Ok();
}

JSString *Backend::name(JSContext *cx, JSObject *self) {
  MOZ_ASSERT(is_instance(self));
  return JS::GetReservedSlot(self, Backend::Slots::Name).toString();
}

bool Backend::to_string(JSContext *cx, unsigned argc, JS::Value *vp) {
  METHOD_HEADER(0);
  JS::RootedString name(cx, JS::GetReservedSlot(self, Backend::Slots::Name).toString());
  args.rval().setString(name);
  return true;
}

namespace {
host_api::HostString parse_and_validate_name(JSContext *cx, JS::HandleValue name_val) {
  if (name_val.isNullOrUndefined()) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_BACKEND_NAME_NOT_SET);
    return nullptr;
  }
  JS::RootedString name(cx, JS::ToString(cx, name_val));
  if (!name) {
    return nullptr;
  }
  auto length = JS::GetStringLength(name);
  if (length > 254) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_BACKEND_NAME_TOO_LONG);
    return nullptr;
  }
  if (length == 0) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_BACKEND_NAME_EMPTY);
    return nullptr;
  }
  return core::encode(cx, name);
}
} // namespace

bool Backend::exists(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  JS::RootedObject self(cx, &args.thisv().toObject());
  if (!args.requireAtLeast(cx, "Backend.exists", 1)) {
    return false;
  }

  auto name = parse_and_validate_name(cx, args.get(0));
  if (!name) {
    return false;
  }
  auto res = host_api::Backend::exists(name);
  if (auto *err = res.to_err()) {
    HANDLE_ERROR(cx, *err);
    return false;
  }
  auto exists = res.unwrap();
  args.rval().setBoolean(exists);
  return true;
}

bool Backend::from_name(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  JS::RootedObject self(cx, &args.thisv().toObject());
  if (!args.requireAtLeast(cx, "Backend.fromName", 1)) {
    return false;
  }

  auto name = parse_and_validate_name(cx, args.get(0));
  if (!name) {
    return false;
  }
  auto res = host_api::Backend::exists(name);
  if (auto *err = res.to_err()) {
    HANDLE_ERROR(cx, *err);
    return false;
  }
  auto exists = res.unwrap();

  if (!exists) {
    JS_ReportErrorNumberUTF8(cx, GetErrorMessage, nullptr,
                             JSMSG_BACKEND_FROMNAME_BACKEND_DOES_NOT_EXIST, name.begin());
    return false;
  }

  auto backend_instance = JS_NewObjectWithGivenProto(cx, &Backend::class_, Backend::proto_obj);
  if (!backend_instance) {
    return false;
  }
  JS::RootedValue backend_val(cx, JS::ObjectValue(*backend_instance));
  JS::RootedObject backend(cx, backend_instance);
  if (!backend) {
    return false;
  }

  JS::RootedValue name_val(cx, JS::StringValue(JS_NewStringCopyZ(cx, name.begin())));
  if (!Backend::set_name(cx, backend, name_val)) {
    return false;
  }

  args.rval().setObject(*backend);
  return true;
}

bool Backend::health(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  JS::RootedObject self(cx, &args.thisv().toObject());
  if (!args.requireAtLeast(cx, "Backend.health", 1)) {
    return false;
  }

  auto name = parse_and_validate_name(cx, args.get(0));
  if (!name) {
    return false;
  }
  auto exists = host_api::Backend::exists(name);
  if (auto *err = exists.to_err()) {
    HANDLE_ERROR(cx, *err);
    return false;
  }

  if (!exists.unwrap()) {
    JS_ReportErrorNumberUTF8(cx, GetErrorMessage, nullptr,
                             JSMSG_BACKEND_IS_HEALTHY_BACKEND_DOES_NOT_EXIST, name.begin());
    return false;
  }

  auto res = host_api::Backend::health(name);
  if (auto *err = res.to_err()) {
    HANDLE_ERROR(cx, *err);
    return false;
  }

  auto health = res.unwrap();
  if (health.is_healthy()) {
    args.rval().setString(JS_NewStringCopyZ(cx, "healthy"));
  } else if (health.is_unhealthy()) {
    args.rval().setString(JS_NewStringCopyZ(cx, "unhealthy"));
  } else {
    args.rval().setString(JS_NewStringCopyZ(cx, "unknown"));
  }

  return true;
}

const JSFunctionSpec Backend::static_methods[] = {
    JS_FN("exists", exists, 1, JSPROP_ENUMERATE), JS_FN("fromName", from_name, 1, JSPROP_ENUMERATE),
    JS_FN("health", health, 1, JSPROP_ENUMERATE), JS_FS_END};
const JSPropertySpec Backend::static_properties[] = {JS_PS_END};
const JSFunctionSpec Backend::methods[] = {JS_FN("toString", to_string, 0, JSPROP_ENUMERATE),
                                           JS_FN("toName", to_string, 0, JSPROP_ENUMERATE),
                                           JS_FS_END};
const JSPropertySpec Backend::properties[] = {JS_PS_END};

bool Backend::set_name(JSContext *cx, JSObject *backend, JS::HandleValue name_val) {
  MOZ_ASSERT(is_instance(backend));
  auto name = parse_and_validate_name(cx, name_val);
  if (!name) {
    return false;
  }

  JS::SetReservedSlot(backend, Backend::Slots::Name,
                      JS::StringValue(JS_NewStringCopyZ(cx, name.begin())));
  return true;
}

bool Backend::set_host_override(JSContext *cx, JSObject *backend,
                                JS::HandleValue host_override_val) {
  MOZ_ASSERT(is_instance(backend));
  auto host_override = JS::ToString(cx, host_override_val);
  if (!host_override) {
    return false;
  }

  if (JS_GetStringLength(host_override) == 0) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_BACKEND_HOST_OVERRIDE_EMPTY);
    return false;
  }
  JS::SetReservedSlot(backend, Backend::Slots::HostOverride, JS::StringValue(host_override));
  return true;
}

bool Backend::set_sni_hostname(JSContext *cx, JSObject *backend, JS::HandleValue sni_hostname_val) {
  auto sni_hostname = JS::ToString(cx, sni_hostname_val);
  if (!sni_hostname) {
    return false;
  }

  if (JS_GetStringLength(sni_hostname) == 0) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_BACKEND_SNI_HOSTNAME_EMPTY);
    return false;
  }
  JS::SetReservedSlot(backend, Backend::Slots::SniHostname, JS::StringValue(sni_hostname));
  return true;
}

bool Backend::set_client_cert(JSContext *cx, JSObject *backend, JS::HandleValue client_cert_val) {
  auto client_cert = JS::ToString(cx, client_cert_val);
  if (!client_cert) {
    return false;
  }

  if (JS_GetStringLength(client_cert) == 0) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr,
                              JSMSG_BACKEND_CLIENT_CERTIFICATE_CERTIFICATE_EMPTY);
    return false;
  }
  JS::SetReservedSlot(backend, Backend::Slots::ClientCert, JS::StringValue(client_cert));
  return true;
}

bool Backend::set_client_cert_key(JSContext *cx, JSObject *backend,
                                  JS::HandleValue client_cert_key_val) {
  if (!SecretStoreEntry::is_instance(client_cert_key_val)) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr,
                              JSMSG_BACKEND_CLIENT_CERTIFICATE_KEY_INVALID);
    return false;
  }
  JS::RootedObject client_cert_key_obj(cx, &client_cert_key_val.toObject());
  JS::SetReservedSlot(backend, Backend::Slots::ClientCertKey,
                      JS::Int32Value(SecretStoreEntry::secret_handle(client_cert_key_obj).handle));
  return true;
}

/// Timeouts for backends must be less than 2^32 milliseconds, or
/// about a month and a half.
bool Backend::set_timeout_slot(JSContext *cx, JSObject *backend, JS::HandleValue value,
                               Backend::Slots slot, std::string property_name) {
  double native_value;
  if (!JS::ToNumber(cx, value, &native_value)) {
    return false;
  }
  int64_t timeout = std::round(native_value);
  if (timeout < 0) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_BACKEND_TIMEOUT_NEGATIVE,
                              property_name.c_str());
    return false;
  }
  if (timeout >= std::pow(2, 32)) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_BACKEND_TIMEOUT_TOO_BIG,
                              property_name.c_str());
    return false;
  }
  JS::SetReservedSlot(backend, slot, JS::Int32Value(timeout));
  return true;
}

bool Backend::set_target(JSContext *cx, JSObject *backend, JS::HandleValue target_val) {
  if (target_val.isNullOrUndefined()) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_BACKEND_TARGET_NOT_SET);
    return false;
  }

  auto target_string_slice = core::encode_spec_string(cx, target_val);
  if (!target_string_slice.data) {
    return false;
  }

  std::string_view target_string(reinterpret_cast<char *>(target_string_slice.data),
                                 target_string_slice.len);
  auto length = target_string.length();
  if (length == 0) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_BACKEND_TARGET_EMPTY);
    return false;
  }

  if (target_string == "::") {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_BACKEND_TARGET_INVALID);
    return false;
  }
  if (!is_valid_host(target_string) && !is_valid_ip(target_string)) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_BACKEND_TARGET_INVALID);
    return false;
  }

  auto target_str = JS_NewStringCopyN(cx, target_string.data(), target_string.length());
  if (!target_str) {
    return false;
  }
  JS::RootedValue target(cx, JS::StringValue(target_str));
  JS::SetReservedSlot(backend, Backend::Slots::Target, target);
  return true;
}

JSObject *Backend::create(JSContext *cx, JS::HandleObject request) {
  JS::RootedValue request_url(cx, RequestOrResponse::url(request));
  auto url_string = core::encode_spec_string(cx, request_url);
  if (!url_string.data) {
    return nullptr;
  }

  auto url = jsurl::new_jsurl(&url_string);
  if (!url) {
    JS_ReportErrorUTF8(cx, "URL constructor: %s is not a valid URL.", (char *)url_string.data);
    return nullptr;
  }
  const jsurl::SpecSlice slice = jsurl::host(url);
  auto name_js_str = JS_NewStringCopyN(cx, (char *)slice.data, slice.len);
  if (!name_js_str) {
    return nullptr;
  }
  std::string name_str((char *)slice.data, slice.len);

  // Check if we already constructed an implicit dynamic backend for this host.
  bool found;
  JS::RootedValue already_built_backend(cx);
  if (!JS_HasProperty(cx, Backend::backends, name_str.c_str(), &found)) {
    return nullptr;
  }
  if (found) {
    if (!JS_GetProperty(cx, Backend::backends, name_str.c_str(), &already_built_backend)) {
      return nullptr;
    }
    JS::RootedObject backend(cx, &already_built_backend.toObject());
    return backend;
  }

  auto backend_instance = JS_NewObjectWithGivenProto(cx, &Backend::class_, Backend::proto_obj);
  if (!backend_instance) {
    return nullptr;
  }
  JS::RootedValue backend_val(cx, JS::ObjectValue(*backend_instance));
  JS::RootedObject backend(cx, backend_instance);
  if (!backend) {
    return nullptr;
  }

  JS::RootedValue name(cx, JS::StringValue(name_js_str));
  if (!Backend::set_name(cx, backend, name)) {
    return nullptr;
  }
  if (!Backend::set_host_override(cx, backend, name)) {
    return nullptr;
  }
  if (!Backend::set_target(cx, backend, name)) {
    return nullptr;
  }
  const jsurl::SpecString origin_specstring = jsurl::origin(url);
  std::string_view origin((char *)origin_specstring.data, origin_specstring.len);

  auto use_ssl = origin.rfind("https://", 0) == 0;
  JS::SetReservedSlot(backend, Backend::Slots::UseSsl, JS::BooleanValue(use_ssl));
  if (use_ssl) {
    if (!Backend::set_sni_hostname(cx, backend, name)) {
      return nullptr;
    }
  }

  JS::SetReservedSlot(backend, Backend::Slots::DontPool, JS::BooleanValue(false));

  if (Fastly::defaultDynamicBackendConfig.connect_timeout.has_value()) {
    JS::RootedValue connect_timeout_val(
        cx, JS::NumberValue(Fastly::defaultDynamicBackendConfig.connect_timeout.value()));
    if (!Backend::set_timeout_slot(cx, backend, connect_timeout_val, Backend::Slots::ConnectTimeout,
                                   "connectTimeout")) {
      return nullptr;
    }
  }
  if (Fastly::defaultDynamicBackendConfig.between_bytes_timeout.has_value()) {
    JS::RootedValue between_bytes_timeout_val(
        cx, JS::NumberValue(Fastly::defaultDynamicBackendConfig.between_bytes_timeout.value()));
    if (!Backend::set_timeout_slot(cx, backend, between_bytes_timeout_val,
                                   Backend::Slots::BetweenBytesTimeout, "betweenBytesTimeout")) {
      return nullptr;
    }
  }
  if (Fastly::defaultDynamicBackendConfig.first_byte_timeout.has_value()) {
    JS::RootedValue first_byte_timeout_val(
        cx, JS::NumberValue(Fastly::defaultDynamicBackendConfig.first_byte_timeout.value()));
    if (!Backend::set_timeout_slot(cx, backend, first_byte_timeout_val,
                                   Backend::Slots::FirstByteTimeout, "firstByteTimeout")) {
      return nullptr;
    }
  }

  auto result = Backend::register_dynamic_backend(cx, backend);
  if (result.isErr()) {
    return nullptr;
  } else {
    if (!JS_SetProperty(cx, Backend::backends, name_str.c_str(), backend_val)) {
      return nullptr;
    }
    return backend;
  }
}

bool Backend::constructor(JSContext *cx, unsigned argc, JS::Value *vp) {
  REQUEST_HANDLER_ONLY("The Backend builtin");
  CTOR_HEADER("Backend", 1);

  auto configuration_parameter = args.get(0);

  if (!configuration_parameter.isObject()) {
    JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_BACKEND_PARAMETER_NOT_OBJECT);
    return false;
  }

  JS::RootedObject backend(cx, JS_NewObjectForConstructor(cx, &class_, args));
  if (!backend) {
    return false;
  }

  JS::RootedObject configuration(cx, &configuration_parameter.toObject());

  JS::RootedValue name_val(cx);
  if (!JS_GetProperty(cx, configuration, "name", &name_val)) {
    return false;
  }
  if (!Backend::set_name(cx, backend, name_val)) {
    return false;
  }

  JS::RootedValue target_val(cx);
  if (!JS_GetProperty(cx, configuration, "target", &target_val)) {
    return false;
  }
  if (!Backend::set_target(cx, backend, target_val)) {
    return false;
  }

  bool found;
  JS::RootedValue host_override_val(cx);
  if (!JS_HasProperty(cx, configuration, "hostOverride", &found)) {
    return false;
  }
  if (found) {
    if (!JS_GetProperty(cx, configuration, "hostOverride", &host_override_val)) {
      return false;
    }
    if (!Backend::set_host_override(cx, backend, host_override_val)) {
      return false;
    }
  }

  JS::RootedValue connect_timeout_val(cx);
  if (!JS_HasProperty(cx, configuration, "connectTimeout", &found)) {
    return false;
  }
  if (found) {
    if (!JS_GetProperty(cx, configuration, "connectTimeout", &connect_timeout_val)) {
      return false;
    }
    if (!Backend::set_timeout_slot(cx, backend, connect_timeout_val, Backend::Slots::ConnectTimeout,
                                   "connectTimeout")) {
      return false;
    }
  }

  /// Timeouts for backends must be less than 2^32 milliseconds, or
  /// about a month and a half.
  JS::RootedValue first_byte_timeout_val(cx);
  if (!JS_HasProperty(cx, configuration, "firstByteTimeout", &found)) {
    return false;
  }
  if (found) {
    if (!JS_GetProperty(cx, configuration, "firstByteTimeout", &first_byte_timeout_val)) {
      return false;
    }
    if (!Backend::set_timeout_slot(cx, backend, first_byte_timeout_val,
                                   Backend::Slots::FirstByteTimeout, "firstByteTimeout")) {
      return false;
    }
  }

  /// Timeouts for backends must be less than 2^32 milliseconds, or
  /// about a month and a half.
  JS::RootedValue between_bytes_timeout_val(cx);
  if (!JS_HasProperty(cx, configuration, "betweenBytesTimeout", &found)) {
    return false;
  }
  if (found) {
    if (!JS_GetProperty(cx, configuration, "betweenBytesTimeout", &between_bytes_timeout_val)) {
      return false;
    }
    if (!Backend::set_timeout_slot(cx, backend, between_bytes_timeout_val,
                                   Backend::Slots::BetweenBytesTimeout, "betweenBytesTimeout")) {
      return false;
    }
  }

  /// Has to be either: 1; 1.1; 1.2; 1.3;
  JS::RootedValue tls_min_version_val(cx);
  std::optional<host_api::TlsVersion> tls_min_version;
  if (!JS_HasProperty(cx, configuration, "tlsMinVersion", &found)) {
    return false;
  }
  if (found) {
    if (!JS_GetProperty(cx, configuration, "tlsMinVersion", &tls_min_version_val)) {
      return false;
    }
    double version;
    if (!JS::ToNumber(cx, tls_min_version_val, &version)) {
      return false;
    }

    if (std::isnan(version)) {
      JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_BACKEND_TLS_MIN_INVALID);
      return false;
    }

    if (version == 1.3) {
      tls_min_version = host_api::TlsVersion::version_1_3();
    } else if (version == 1.2) {
      tls_min_version = host_api::TlsVersion::version_1_2();
    } else if (version == 1.1) {
      tls_min_version = host_api::TlsVersion::version_1_1();
    } else if (version == 1) {
      tls_min_version = host_api::TlsVersion::version_1();
    } else {
      JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_BACKEND_TLS_MIN_INVALID);
      return false;
    }
    JS::SetReservedSlot(backend, Backend::Slots::TlsMinVersion,
                        JS::Int32Value(tls_min_version->value));
  }

  /// Has to be either: 1; 1.1; 1.2; 1.3;
  JS::RootedValue tls_max_version_val(cx);
  std::optional<host_api::TlsVersion> tls_max_version;
  if (!JS_HasProperty(cx, configuration, "tlsMaxVersion", &found)) {
    return false;
  }
  if (found) {
    if (!JS_GetProperty(cx, configuration, "tlsMaxVersion", &tls_max_version_val)) {
      return false;
    }
    double version;
    if (!JS::ToNumber(cx, tls_max_version_val, &version)) {
      return false;
    }

    if (std::isnan(version)) {
      JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_BACKEND_TLS_MAX_INVALID);
      return false;
    }

    if (version == 1.3) {
      tls_max_version = host_api::TlsVersion::version_1_3();
    } else if (version == 1.2) {
      tls_max_version = host_api::TlsVersion::version_1_2();
    } else if (version == 1.1) {
      tls_max_version = host_api::TlsVersion::version_1_1();
    } else if (version == 1) {
      tls_max_version = host_api::TlsVersion::version_1();
    } else {
      JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_BACKEND_TLS_MAX_INVALID);
      return false;
    }
    JS::SetReservedSlot(backend, Backend::Slots::TlsMaxVersion,
                        JS::Int32Value(tls_max_version->value));
  }

  if (tls_min_version.has_value() && tls_max_version.has_value()) {
    if (tls_min_version->value > tls_max_version->value) {
      JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr,
                                JSMSG_BACKEND_TLS_MIN_GREATER_THAN_TLS_MAX);
      return false;
    }
  }

  JS::RootedValue certificate_hostname_val(cx);
  if (!JS_HasProperty(cx, configuration, "certificateHostname", &found)) {
    return false;
  }
  if (found) {
    if (!JS_GetProperty(cx, configuration, "certificateHostname", &certificate_hostname_val)) {
      return false;
    }
    auto certificate_hostname = JS::ToString(cx, certificate_hostname_val);
    if (!certificate_hostname) {
      return false;
    }

    if (JS_GetStringLength(certificate_hostname) == 0) {
      JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr,
                                JSMSG_BACKEND_CERTIFICATE_HOSTNAME_EMPTY);
      return false;
    }
    JS::SetReservedSlot(backend, Backend::Slots::CertificateHostname,
                        JS::StringValue(certificate_hostname));
  }

  JS::RootedValue use_ssl_val(cx);
  if (!JS_HasProperty(cx, configuration, "useSSL", &found)) {
    return false;
  }
  if (found) {
    if (!JS_GetProperty(cx, configuration, "useSSL", &use_ssl_val)) {
      return false;
    }
    auto value = JS::ToBoolean(use_ssl_val);
    JS::SetReservedSlot(backend, Backend::Slots::UseSsl, JS::BooleanValue(value));
  }

  JS::RootedValue dont_pool_val(cx);
  if (!JS_HasProperty(cx, configuration, "dontPool", &found)) {
    return false;
  }
  if (found) {
    if (!JS_GetProperty(cx, configuration, "dontPool", &dont_pool_val)) {
      return false;
    }
    auto value = JS::ToBoolean(dont_pool_val);
    JS::SetReservedSlot(backend, Backend::Slots::DontPool, JS::BooleanValue(value));
  }

  JS::RootedValue ca_certificate_val(cx);
  if (!JS_HasProperty(cx, configuration, "caCertificate", &found)) {
    return false;
  }
  if (found) {
    if (!JS_GetProperty(cx, configuration, "caCertificate", &ca_certificate_val)) {
      return false;
    }
    auto ca_certificate = JS::ToString(cx, ca_certificate_val);
    if (!ca_certificate) {
      return false;
    }
    if (JS_GetStringLength(ca_certificate) == 0) {
      JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_BACKEND_CA_CERTIFICATE_EMPTY);
      return false;
    }
    JS::SetReservedSlot(backend, Backend::Slots::CaCertificate, JS::StringValue(ca_certificate));
  }

  /// Cipher list consisting of one or more cipher strings separated by colons.
  // Commas or spaces are also acceptable separators but colons are normally used.
  JS::RootedValue ciphers_val(cx);
  if (!JS_HasProperty(cx, configuration, "ciphers", &found)) {
    return false;
  }
  if (found) {
    if (!JS_GetProperty(cx, configuration, "ciphers", &ciphers_val)) {
      return false;
    }
    auto ciphers_chars = core::encode(cx, ciphers_val);
    if (!ciphers_chars) {
      return false;
    }
    if (ciphers_chars.size() == 0) {
      JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_BACKEND_CIPHERS_EMPTY);
      return false;
    }
    std::string cipher_spec(ciphers_chars.begin(), ciphers_chars.len);
    if (!is_cipher_suite_supported_by_fastly(cipher_spec)) {
      JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_BACKEND_CIPHERS_NOT_AVALIABLE);
      return false;
    }
    JS::SetReservedSlot(
        backend, Backend::Slots::Ciphers,
        JS::StringValue(JS_NewStringCopyN(cx, ciphers_chars.begin(), ciphers_chars.len)));
  }

  JS::RootedValue sni_hostname_val(cx);
  if (!JS_HasProperty(cx, configuration, "sniHostname", &found)) {
    return false;
  }
  if (found) {
    if (!JS_GetProperty(cx, configuration, "sniHostname", &sni_hostname_val)) {
      return false;
    }
    auto sni_hostname = JS::ToString(cx, sni_hostname_val);
    if (!sni_hostname) {
      return false;
    }
    if (JS_GetStringLength(sni_hostname) == 0) {
      JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_BACKEND_SNI_HOSTNAME_EMPTY);
      return false;
    }
    JS::SetReservedSlot(backend, Backend::Slots::SniHostname, JS::StringValue(sni_hostname));
  }

  JS::RootedValue client_cert_val(cx);
  if (!JS_HasProperty(cx, configuration, "clientCertificate", &found)) {
    return false;
  }
  if (found) {
    if (!JS_GetProperty(cx, configuration, "clientCertificate", &client_cert_val)) {
      return false;
    }
    if (!client_cert_val.isObject()) {
      JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr,
                                JSMSG_BACKEND_CLIENT_CERTIFICATE_NOT_OBJECT);
      return false;
    }
    JS::RootedObject client_cert_obj(cx, &client_cert_val.toObject());

    JS::RootedValue client_cert_cert_val(cx);
    if (!JS_HasProperty(cx, client_cert_obj, "certificate", &found)) {
      return false;
    }
    if (!found) {
      JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr,
                                JSMSG_BACKEND_CLIENT_CERTIFICATE_NO_CERTIFICATE);
      return false;
    }
    if (!JS_GetProperty(cx, client_cert_obj, "certificate", &client_cert_cert_val)) {
      return false;
    }
    if (!Backend::set_client_cert(cx, backend, client_cert_cert_val)) {
      return false;
    }

    JS::RootedValue client_cert_key_val(cx);
    if (!JS_HasProperty(cx, client_cert_obj, "key", &found)) {
      return false;
    }
    if (!found) {
      JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr,
                                JSMSG_BACKEND_CLIENT_CERTIFICATE_KEY_INVALID);
      return false;
    }
    if (!JS_GetProperty(cx, client_cert_obj, "key", &client_cert_key_val)) {
      return false;
    }
    if (!Backend::set_client_cert_key(cx, backend, client_cert_key_val)) {
      return false;
    }
  }

  auto result = Backend::register_dynamic_backend(cx, backend);
  if (result.isErr()) {
    return false;
  }
  args.rval().setObject(*backend);
  return true;
}

bool Backend::init_class(JSContext *cx, JS::HandleObject global) {
  JS::RootedObject backends(cx, JS_NewPlainObject(cx));
  if (!backends) {
    return false;
  }
  Backend::backends.init(cx, backends);
  return BuiltinImpl<Backend>::init_class_impl(cx, global);
}

} // namespace builtins
