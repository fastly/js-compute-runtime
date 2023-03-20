#pragma once
#include <iostream>
#include <list>
#include <span>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <vector>

class PrimeInfo {
public:
  std::string primeFactor;
  std::string factorCRTExponent;
  std::string factorCRTCoefficient;
  PrimeInfo(
    std::string primeFactor,
    std::string factorCRTExponent,
    std::string factorCRTCoefficient): 
    primeFactor{primeFactor}, 
    factorCRTExponent{factorCRTExponent}, 
    factorCRTCoefficient{factorCRTCoefficient} 
  {};
  PrimeInfo(
    std::string primeFactor,
    std::string factorCRTExponent
    ): 
    primeFactor{primeFactor}, 
    factorCRTExponent{factorCRTExponent} {};
  PrimeInfo(
    std::string primeFactor
    ): 
    primeFactor{primeFactor} {};
};

class CryptoKeyRSAComponents {
public:
  enum class Type { Public, Private };

  static CryptoKeyRSAComponents createPublic(std::string modulus, std::string exponent) {
    return CryptoKeyRSAComponents(modulus, exponent);
  }

  static CryptoKeyRSAComponents createPrivate(std::string modulus, std::string exponent,
                                              std::string privateExponent) {
    return CryptoKeyRSAComponents(modulus, exponent, privateExponent);
  }


  static CryptoKeyRSAComponents createPrivateWithAdditionalData(
      std::string modulus, std::string exponent, std::string privateExponent,
      std::optional<PrimeInfo> firstPrimeInfo, std::optional<PrimeInfo> secondPrimeInfo, std::vector<PrimeInfo> otherPrimeInfos);

  virtual ~CryptoKeyRSAComponents();

  Type type() { return _type; }

  // Private and public keys.
  std::string modulus() { return _modulus; }
  std::string exponent() { return _exponent; }

  // Only private keys.
  std::string privateExponent() { return _privateExponent; }
  bool hasAdditionalPrivateKeyParameters() { return _hasAdditionalPrivateKeyParameters; }
  std::optional<PrimeInfo> firstPrimeInfo() { return _firstPrimeInfo; }
  std::optional<PrimeInfo> secondPrimeInfo() { return _secondPrimeInfo; }
  std::vector<PrimeInfo> otherPrimeInfos() { return _otherPrimeInfos; }
  CryptoKeyRSAComponents(std::string modulus, std::string exponent);

  CryptoKeyRSAComponents(std::string modulus, std::string exponent, std::string privateExponent);

  CryptoKeyRSAComponents(std::string modulus, std::string exponent, std::string privateExponent,
                         std::optional<PrimeInfo> firstPrimeInfo, std::optional<PrimeInfo> secondPrimeInfo,
                         std::vector<PrimeInfo> otherPrimeInfos);

  Type _type;

  // Private and public keys.
  std::string _modulus;
  std::string _exponent;

  // Only private keys.
  std::string _privateExponent;
  bool _hasAdditionalPrivateKeyParameters;
  std::optional<PrimeInfo> _firstPrimeInfo;
  std::optional<PrimeInfo> _secondPrimeInfo;
  std::vector<PrimeInfo>
      _otherPrimeInfos; // When three or more primes have been used, the number of array elements
                         // is be the number of primes used minus two.
};
