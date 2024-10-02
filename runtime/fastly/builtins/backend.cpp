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

#include "../host-api/host_api_fastly.h"
#include "./fetch/request-response.h"
#include "./secret-store.h"
#include "backend.h"
#include "builtin.h"
#include "encode.h"
#include "fastly.h"

using builtins::BuiltinImpl;
using fastly::fastly::Fastly;
using fastly::fastly::FastlyGetErrorMessage;
using fastly::fetch::RequestOrResponse;
using fastly::secret_store::SecretStoreEntry;

namespace fastly::backend {

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

bool is_cipher_suite_supported_by_fastly(std::string_view cipher_spec) {
  auto ciphers = cipher_parser.parse(cipher_spec);
  return ciphers.size() > 0;
}

JSString *Backend::name(JSContext *cx, JSObject *self) {
  MOZ_ASSERT(is_instance(self));
  return JS::GetReservedSlot(self, Backend::Slots::Name).toString();
  return nullptr;
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
    JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr, JSMSG_BACKEND_NAME_NOT_SET);
    return nullptr;
  }
  JS::RootedString name(cx, JS::ToString(cx, name_val));
  if (!name) {
    return nullptr;
  }
  auto length = JS::GetStringLength(name);
  if (length > 254) {
    JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr, JSMSG_BACKEND_NAME_TOO_LONG);
    return nullptr;
  }
  if (length == 0) {
    JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr, JSMSG_BACKEND_NAME_EMPTY);
    return nullptr;
  }
  return core::encode(cx, name);
}

bool set_host_override(JSContext *cx, host_api::BackendConfig &backend_config,
                       JS::HandleValue host_override_val) {
  auto host_override = JS::RootedString(cx, JS::ToString(cx, host_override_val));
  if (!host_override) {
    return false;
  }

  if (JS_GetStringLength(host_override) == 0) {
    JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr,
                              JSMSG_BACKEND_HOST_OVERRIDE_EMPTY);
    return false;
  }
  backend_config.host_override = core::encode(cx, host_override);
  return true;
}

bool set_sni_hostname(JSContext *cx, host_api::BackendConfig &backend,
                      JS::HandleValue sni_hostname_val) {
  auto sni_hostname = RootedString(cx, JS::ToString(cx, sni_hostname_val));
  if (!sni_hostname) {
    return false;
  }

  if (JS_GetStringLength(sni_hostname) == 0) {
    JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr, JSMSG_BACKEND_SNI_HOSTNAME_EMPTY);
    return false;
  }
  backend.sni_hostname = core::encode(cx, sni_hostname);
  return true;
}

// Timeouts for backends must be less than 2^32 milliseconds, or
// about a month and a half.
std::optional<uint32_t> parse_and_validate_timeout(JSContext *cx, JS::HandleValue value,
                                                   std::string property_name) {
  double native_value;
  if (!JS::ToNumber(cx, value, &native_value)) {
    return std::nullopt;
  }
  if (std::isnan(native_value)) {
    JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr, JSMSG_BACKEND_TIMEOUT_NAN,
                              property_name.c_str());
    return std::nullopt;
  }
  if (native_value < 0) {
    JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr, JSMSG_BACKEND_TIMEOUT_NEGATIVE,
                              property_name.c_str());
    return std::nullopt;
  }
  if (native_value >= 0x100000000) {
    JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr, JSMSG_BACKEND_TIMEOUT_TOO_BIG,
                              property_name.c_str());
    return std::nullopt;
  }
  return std::round(native_value);
}

bool validate_target(JSContext *cx, std::string_view target_string) {
  auto length = target_string.length();
  if (length == 0) {
    JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr, JSMSG_BACKEND_TARGET_EMPTY);
    return false;
  }

  if (target_string == "::") {
    JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr, JSMSG_BACKEND_TARGET_INVALID);
    return false;
  }
  if (!is_valid_host(target_string) && !is_valid_ip(target_string)) {
    JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr, JSMSG_BACKEND_TARGET_INVALID);
    return false;
  }

  return true;
}

host_api::BackendConfig default_backend_config{};

bool apply_backend_config(JSContext *cx, host_api::BackendConfig &backend,
                          HandleObject configuration) {
  bool found;
  JS::RootedValue host_override_val(cx);
  if (!JS_HasProperty(cx, configuration, "hostOverride", &found)) {
    return false;
  }
  if (found) {
    if (!JS_GetProperty(cx, configuration, "hostOverride", &host_override_val)) {
      return false;
    }
    if (!set_host_override(cx, backend, host_override_val)) {
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
    auto parsed = parse_and_validate_timeout(cx, connect_timeout_val, "connectTimeout");
    if (!parsed) {
      return false;
    }
    backend.connect_timeout = parsed;
  }

  // Timeouts for backends must be less than 2^32 milliseconds, or
  // about a month and a half.
  JS::RootedValue first_byte_timeout_val(cx);
  if (!JS_HasProperty(cx, configuration, "firstByteTimeout", &found)) {
    return false;
  }
  if (found) {
    if (!JS_GetProperty(cx, configuration, "firstByteTimeout", &first_byte_timeout_val)) {
      return false;
    }
    auto parsed = parse_and_validate_timeout(cx, first_byte_timeout_val, "firstByteTimeout");
    if (!parsed) {
      return false;
    }
    backend.first_byte_timeout = parsed;
  }

  // Timeouts for backends must be less than 2^32 milliseconds, or
  // about a month and a half.
  JS::RootedValue between_bytes_timeout_val(cx);
  if (!JS_HasProperty(cx, configuration, "betweenBytesTimeout", &found)) {
    return false;
  }
  if (found) {
    if (!JS_GetProperty(cx, configuration, "betweenBytesTimeout", &between_bytes_timeout_val)) {
      return false;
    }
    auto parsed = parse_and_validate_timeout(cx, between_bytes_timeout_val, "betweenBytesTimeout");
    if (!parsed) {
      return false;
    }
    backend.between_bytes_timeout = parsed;
  }

  // Has to be either: 1; 1.1; 1.2; 1.3;
  JS::RootedValue tls_min_version_val(cx);
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
      JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr, JSMSG_BACKEND_TLS_MIN_INVALID);
      return false;
    }

    if (version == 1.3) {
      backend.ssl_min_version = host_api::TlsVersion::version_1_3();
    } else if (version == 1.2) {
      backend.ssl_min_version = host_api::TlsVersion::version_1_2();
    } else if (version == 1.1) {
      backend.ssl_min_version = host_api::TlsVersion::version_1_1();
    } else if (version == 1) {
      backend.ssl_min_version = host_api::TlsVersion::version_1();
    } else {
      JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr, JSMSG_BACKEND_TLS_MIN_INVALID);
      return false;
    }
  }

  // Has to be either: 1; 1.1; 1.2; 1.3;
  JS::RootedValue tls_max_version_val(cx);
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
      JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr, JSMSG_BACKEND_TLS_MAX_INVALID);
      return false;
    }

    if (version == 1.3) {
      backend.ssl_max_version = host_api::TlsVersion::version_1_3();
    } else if (version == 1.2) {
      backend.ssl_max_version = host_api::TlsVersion::version_1_2();
    } else if (version == 1.1) {
      backend.ssl_max_version = host_api::TlsVersion::version_1_1();
    } else if (version == 1) {
      backend.ssl_max_version = host_api::TlsVersion::version_1();
    } else {
      JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr, JSMSG_BACKEND_TLS_MAX_INVALID);
      return false;
    }
  }

  if (backend.ssl_min_version.has_value() && backend.ssl_max_version.has_value()) {
    if (backend.ssl_min_version->value > backend.ssl_max_version->value) {
      JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr,
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
    auto certificate_hostname = JS::RootedString(cx, JS::ToString(cx, certificate_hostname_val));
    if (!certificate_hostname) {
      return false;
    }

    if (JS_GetStringLength(certificate_hostname) == 0) {
      JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr,
                                JSMSG_BACKEND_CERTIFICATE_HOSTNAME_EMPTY);
      return false;
    }
    backend.cert_hostname = core::encode(cx, certificate_hostname);
  }

  JS::RootedValue use_ssl_val(cx);
  if (!JS_HasProperty(cx, configuration, "useSSL", &found)) {
    return false;
  }
  if (found) {
    if (!JS_GetProperty(cx, configuration, "useSSL", &use_ssl_val)) {
      return false;
    }
    backend.use_ssl = JS::ToBoolean(use_ssl_val);
  }

  JS::RootedValue dont_pool_val(cx);
  if (!JS_HasProperty(cx, configuration, "dontPool", &found)) {
    return false;
  }
  if (found) {
    if (!JS_GetProperty(cx, configuration, "dontPool", &dont_pool_val)) {
      return false;
    }
    backend.dont_pool = JS::ToBoolean(dont_pool_val);
  }

  JS::RootedValue ca_certificate_val(cx);
  if (!JS_HasProperty(cx, configuration, "caCertificate", &found)) {
    return false;
  }
  if (found) {
    if (!JS_GetProperty(cx, configuration, "caCertificate", &ca_certificate_val)) {
      return false;
    }
    auto ca_certificate = JS::RootedString(cx, JS::ToString(cx, ca_certificate_val));
    if (!ca_certificate) {
      return false;
    }
    if (JS_GetStringLength(ca_certificate) == 0) {
      JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr,
                                JSMSG_BACKEND_CA_CERTIFICATE_EMPTY);
      return false;
    }
    backend.ca_cert = core::encode(cx, ca_certificate);
  }

  // Cipher list consisting of one or more cipher strings separated by colons.
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
      JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr, JSMSG_BACKEND_CIPHERS_EMPTY);
      return false;
    }
    std::string cipher_spec(ciphers_chars.begin(), ciphers_chars.len);
    if (!is_cipher_suite_supported_by_fastly(cipher_spec)) {
      JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr,
                                JSMSG_BACKEND_CIPHERS_NOT_AVALIABLE);
      return false;
    }
    backend.ciphers.emplace(std::move(ciphers_chars));
  }

  JS::RootedValue sni_hostname_val(cx);
  if (!JS_HasProperty(cx, configuration, "sniHostname", &found)) {
    return false;
  }
  if (found) {
    if (!JS_GetProperty(cx, configuration, "sniHostname", &sni_hostname_val)) {
      return false;
    }
    if (!set_sni_hostname(cx, backend, sni_hostname_val)) {
      return false;
    }
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
      JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr,
                                JSMSG_BACKEND_CLIENT_CERTIFICATE_NOT_OBJECT);
      return false;
    }
    JS::RootedObject client_cert_obj(cx, &client_cert_val.toObject());

    JS::RootedValue client_cert_cert_val(cx);
    if (!JS_HasProperty(cx, client_cert_obj, "certificate", &found)) {
      return false;
    }
    if (!found) {
      JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr,
                                JSMSG_BACKEND_CLIENT_CERTIFICATE_NO_CERTIFICATE);
      return false;
    }
    if (!JS_GetProperty(cx, client_cert_obj, "certificate", &client_cert_cert_val)) {
      return false;
    }
    RootedString client_cert(cx, JS::ToString(cx, client_cert_cert_val));
    if (!client_cert) {
      return false;
    }

    if (JS_GetStringLength(client_cert) == 0) {
      JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr,
                                JSMSG_BACKEND_CLIENT_CERTIFICATE_CERTIFICATE_EMPTY);
      return false;
    }
    JS::RootedValue client_cert_key_val(cx);
    if (!JS_HasProperty(cx, client_cert_obj, "key", &found)) {
      return false;
    }
    if (!found) {
      JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr,
                                JSMSG_BACKEND_CLIENT_CERTIFICATE_KEY_INVALID);
      return false;
    }
    if (!JS_GetProperty(cx, client_cert_obj, "key", &client_cert_key_val)) {
      return false;
    }

    if (!SecretStoreEntry::is_instance(client_cert_key_val)) {
      JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr,
                                JSMSG_BACKEND_CLIENT_CERTIFICATE_KEY_INVALID);
      return false;
    }
    JS::RootedObject client_cert_key_obj(cx, &client_cert_key_val.toObject());

    backend.client_cert =
        host_api::ClientCert{.cert = core::encode(cx, client_cert),
                             .key = SecretStoreEntry::secret_handle(client_cert_key_obj).handle};
  }

  JS::RootedValue grpc_val(cx);
  if (!JS_HasProperty(cx, configuration, "grpc", &found)) {
    return false;
  }
  if (found) {
    if (!JS_GetProperty(cx, configuration, "grpc", &grpc_val)) {
      return false;
    }
    backend.grpc = JS::ToBoolean(grpc_val);
  }

  JS::RootedValue http_keepalive_time_ms_val(cx);
  if (!JS_HasProperty(cx, configuration, "httpKeepalive", &found)) {
    return false;
  }
  if (found) {
    if (!JS_GetProperty(cx, configuration, "httpKeepalive", &http_keepalive_time_ms_val)) {
      return false;
    }
    auto parsed = parse_and_validate_timeout(cx, http_keepalive_time_ms_val, "httpKeepalive");
    if (!parsed) {
      return false;
    }
    backend.http_keepalive_time_ms = parsed;
  }

  JS::RootedValue tcp_keepalive_val(cx);
  if (!JS_HasProperty(cx, configuration, "tcpKeepalive", &found)) {
    return false;
  }
  if (found) {
    if (!JS_GetProperty(cx, configuration, "tcpKeepalive", &tcp_keepalive_val)) {
      return false;
    }
    if (tcp_keepalive_val.isBoolean()) {
      if (tcp_keepalive_val.toBoolean()) {
        backend.tcp_keepalive = host_api::TcpKeepalive{};
      }
    } else if (!tcp_keepalive_val.isObject()) {
      JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr,
                                JSMSG_BACKEND_TCP_KEEPALIVE_NOT_OBJECT_OR_BOOL);
      return false;
    }
    if (tcp_keepalive_val.isObject()) {
      host_api::TcpKeepalive tcp_keepalive{};
      JS::RootedObject tcp_keepalive_obj(cx, &tcp_keepalive_val.toObject());

      JS::RootedValue tcp_keepalive_time_secs_val(cx);
      if (!JS_HasProperty(cx, tcp_keepalive_obj, "timeSecs", &found)) {
        return false;
      }
      if (found) {
        if (!JS_GetProperty(cx, tcp_keepalive_obj, "timeSecs", &tcp_keepalive_time_secs_val)) {
          return false;
        }
        auto parsed = parse_and_validate_timeout(cx, tcp_keepalive_time_secs_val, "timeSecs");
        if (!parsed) {
          return false;
        }
        tcp_keepalive.time_secs = parsed;
      }

      JS::RootedValue tcp_keepalive_interval_secs_val(cx);
      if (!JS_HasProperty(cx, tcp_keepalive_obj, "intervalSecs", &found)) {
        return false;
      }
      if (found) {
        if (!JS_GetProperty(cx, tcp_keepalive_obj, "intervalSecs",
                            &tcp_keepalive_interval_secs_val)) {
          return false;
        }
        auto parsed =
            parse_and_validate_timeout(cx, tcp_keepalive_interval_secs_val, "intervalSecs");
        if (!parsed) {
          return false;
        }
        tcp_keepalive.interval_secs = parsed;
      }

      JS::RootedValue tcp_keepalive_probes_val(cx);
      if (!JS_HasProperty(cx, tcp_keepalive_obj, "probes", &found)) {
        return false;
      }
      if (found) {
        if (!JS_GetProperty(cx, tcp_keepalive_obj, "probes", &tcp_keepalive_probes_val)) {
          return false;
        }
        double native_value;
        if (!JS::ToNumber(cx, tcp_keepalive_probes_val, &native_value)) {
          return false;
        }
        if (std::isnan(native_value) || native_value <= 0 || native_value >= 0x100000000) {
          JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr,
                                    JSMSG_BACKEND_TCP_KEEPALIVE_INVALID_PROBES);
          return false;
        }
        tcp_keepalive.probes = std::round(native_value);
      }
      backend.tcp_keepalive = tcp_keepalive;
    }
  }
  return true;
}

} // namespace

bool Backend::exists(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
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
    JS_ReportErrorNumberUTF8(cx, FastlyGetErrorMessage, nullptr,
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
    JS_ReportErrorNumberUTF8(cx, FastlyGetErrorMessage, nullptr,
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

std::optional<host_api::HostString> Backend::set_name(JSContext *cx, JSObject *backend,
                                                      JS::HandleValue name_val) {
  MOZ_ASSERT(is_instance(backend));
  auto name = parse_and_validate_name(cx, name_val);
  if (!name) {
    return std::nullopt;
  }

  JS::SetReservedSlot(backend, Backend::Slots::Name,
                      JS::StringValue(JS_NewStringCopyZ(cx, name.begin())));
  return name;
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

  host_api::BackendConfig backend_config = default_backend_config.clone();

  JS::RootedValue name(cx, JS::StringValue(name_js_str));
  if (!Backend::set_name(cx, backend, name)) {
    return nullptr;
  }
  if (!set_host_override(cx, backend_config, name)) {
    return nullptr;
  }
  auto target_string_slice = core::encode_spec_string(cx, name);
  if (!target_string_slice.data) {
    return nullptr;
  }
  std::string_view target_string(reinterpret_cast<char *>(target_string_slice.data),
                                 target_string_slice.len);
  if (!validate_target(cx, target_string)) {
    return nullptr;
  }
  const jsurl::SpecString origin_specstring = jsurl::origin(url);
  std::string_view origin((char *)origin_specstring.data, origin_specstring.len);

  auto use_ssl = origin.rfind("https://", 0) == 0;
  if (use_ssl) {
    if (!set_sni_hostname(cx, backend_config, name)) {
      return nullptr;
    }
  }

  auto res = host_api::HttpReq::register_dynamic_backend(name_str, target_string, backend_config);
  if (auto *err = res.to_err()) {
    HANDLE_ERROR(cx, *err);
    return nullptr;
  }
  if (!JS_SetProperty(cx, Backend::backends, name_str.c_str(), backend_val)) {
    return nullptr;
  }
  return backend;
}

bool Backend::constructor(JSContext *cx, unsigned argc, JS::Value *vp) {
  REQUEST_HANDLER_ONLY("The Backend builtin");
  CTOR_HEADER("Backend", 1);

  auto configuration_parameter = args.get(0);

  if (!configuration_parameter.isObject()) {
    JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr,
                              JSMSG_BACKEND_PARAMETER_NOT_OBJECT);
    return false;
  }

  auto configuration = RootedObject(cx, &configuration_parameter.toObject());

  JS::RootedObject backend(cx, JS_NewObjectForConstructor(cx, &class_, args));
  if (!backend) {
    return false;
  }

  JS::RootedValue name_val(cx);
  if (!JS_GetProperty(cx, configuration, "name", &name_val)) {
    return false;
  }
  auto backend_name = Backend::set_name(cx, backend, name_val);
  if (!backend_name) {
    return false;
  }

  JS::RootedValue target_val(cx);
  if (!JS_GetProperty(cx, configuration, "target", &target_val)) {
    return false;
  }
  if (target_val.isNullOrUndefined()) {
    JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr, JSMSG_BACKEND_TARGET_NOT_SET);
    return false;
  }
  auto target_string_slice = core::encode_spec_string(cx, target_val);
  if (!target_string_slice.data) {
    return false;
  }
  std::string_view target_string(reinterpret_cast<char *>(target_string_slice.data),
                                 target_string_slice.len);
  if (!validate_target(cx, target_string)) {
    return false;
  }

  host_api::BackendConfig backend_config = default_backend_config.clone();
  if (!apply_backend_config(cx, backend_config, configuration)) {
    return false;
  }

  auto res = host_api::HttpReq::register_dynamic_backend(backend_name.value(), target_string,
                                                         backend_config);
  if (auto *err = res.to_err()) {
    HANDLE_ERROR(cx, *err);
    return false;
  }
  args.rval().setObject(*backend);
  return true;
}

bool set_default_backend_config(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  if (!args.requireAtLeast(cx, "setDefaultDynamicBackendConfiguration", 1)) {
    return false;
  }
  auto backend_config_val = args.get(0);
  if (!backend_config_val.isObject()) {
    JS_ReportErrorNumberASCII(cx, FastlyGetErrorMessage, nullptr,
                              JSMSG_BACKEND_PARAMETER_NOT_OBJECT);
    return false;
  }
  RootedObject backend_config_obj(cx, &backend_config_val.toObject());
  if (!apply_backend_config(cx, default_backend_config, backend_config_obj)) {
    return false;
  }
  return true;
}

bool install(api::Engine *engine) {
  JS::RootedObject backends(engine->cx(), JS_NewPlainObject(engine->cx()));
  if (!backends) {
    return false;
  }
  Backend::backends.init(engine->cx(), backends);
  if (!Backend::init_class_impl(engine->cx(), engine->global())) {
    return false;
  }

  RootedObject backend_obj(engine->cx(),
                           JS_GetConstructor(engine->cx(), BuiltinImpl<Backend>::proto_obj));
  RootedValue backend_val(engine->cx(), ObjectValue(*backend_obj));
  RootedObject backend_ns(engine->cx(), JS_NewObject(engine->cx(), nullptr));
  if (!JS_SetProperty(engine->cx(), backend_ns, "Backend", backend_val)) {
    return false;
  }
  auto set_default_backend_config_fn = JS_NewFunction(engine->cx(), &set_default_backend_config, 1,
                                                      0, "setDefaultDynamicBackendConfig");
  RootedObject set_default_backend_config_obj(engine->cx(),
                                              JS_GetFunctionObject(set_default_backend_config_fn));
  RootedValue set_default_backend_config_val(engine->cx(),
                                             JS::ObjectValue(*set_default_backend_config_obj));
  if (!JS_SetProperty(engine->cx(), backend_ns, "setDefaultDynamicBackendConfig",
                      set_default_backend_config_val)) {
    return false;
  }
  RootedValue backend_ns_val(engine->cx(), JS::ObjectValue(*backend_ns));
  if (!engine->define_builtin_module("fastly:backend", backend_ns_val)) {
    return false;
  }
  return true;
}

} // namespace fastly::backend
