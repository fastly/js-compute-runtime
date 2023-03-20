#include "crypto-key-rsa-components.h"

CryptoKeyRSAComponents::CryptoKeyRSAComponents(std::string modulus, std::string exponent)
    : _type(Type::Public), _modulus(modulus), _exponent(exponent) {}

CryptoKeyRSAComponents::CryptoKeyRSAComponents(std::string modulus, std::string exponent,
                                               std::string privateExponent)
    : _type(Type::Private), _modulus(modulus), _exponent(exponent),
      _privateExponent(privateExponent), _hasAdditionalPrivateKeyParameters(false) {}

CryptoKeyRSAComponents::CryptoKeyRSAComponents(std::string modulus, std::string exponent,
                                               std::string privateExponent,
                                               std::optional<PrimeInfo> firstPrimeInfo,
                                               std::optional<PrimeInfo> secondPrimeInfo,
                                               std::vector<PrimeInfo> otherPrimeInfos)
    : _type(Type::Private), _modulus(modulus), _exponent(exponent),
      _privateExponent(privateExponent), _hasAdditionalPrivateKeyParameters(true),
      _firstPrimeInfo(firstPrimeInfo), _secondPrimeInfo(secondPrimeInfo),
      _otherPrimeInfos(otherPrimeInfos) {}

CryptoKeyRSAComponents::~CryptoKeyRSAComponents() = default;

CryptoKeyRSAComponents CryptoKeyRSAComponents::createPrivateWithAdditionalData(
    std::string modulus, std::string exponent, std::string privateExponent,
    std::optional<PrimeInfo> firstPrimeInfo, std::optional<PrimeInfo> secondPrimeInfo,
    std::vector<PrimeInfo> otherPrimeInfos) {
  return CryptoKeyRSAComponents(modulus, exponent, privateExponent, firstPrimeInfo, secondPrimeInfo,
                                otherPrimeInfos);
}