#include "openssl/sha.h"
#include <iostream>
#include <span>

#include "crypto-algorithm.h"
#include "crypto-key-rsa-components.h"
#include "js-compute-builtins.h"

namespace builtins {

namespace {

// https://datatracker.ietf.org/doc/html/rfc7518#section-6.3.1
// 6.3.1.  Parameters for RSA Public Keys
std::unique_ptr<CryptoKeyRSAComponents> createRSAPublicKeyFromJWK(JSContext *cx, JsonWebKey *jwk) {
  if (!jwk->n.has_value() || !jwk->e.has_value()) {
    // TODO: Change to a DataError instance
    JS_ReportErrorLatin1(cx, "Data provided to an operation does not meet requirements");
    return nullptr;
  }
  auto modulusResult = GlobalProperties::forgivingBase64Decode(
      jwk->n.value(), GlobalProperties::base64URLDecodeTable);
  if (modulusResult.isErr()) {
    // TODO: Change to a DataError instance
    JS_ReportErrorLatin1(cx,
                         "The JWK member 'n' could not be base64url decoded or contained padding");
    return nullptr;
  }
  auto modulus = modulusResult.unwrap();

  // Per RFC 7518 Section 6.3.1.1: https://tools.ietf.org/html/rfc7518#section-6.3.1.1
  if (modulus.starts_with('0')) {
    modulus = modulus.erase(0, 1);
  }
  auto dataResult = GlobalProperties::ConvertJSValueToByteString(cx, jwk->e.value());
  if (dataResult.isErr()) {
    // TODO: Change to a DataError instance
    JS_ReportErrorLatin1(cx, "Data provided to an operation does not meet requirements");
    return nullptr;
  }
  auto data = dataResult.unwrap();
  auto exponentResult =
      GlobalProperties::forgivingBase64Decode(data, GlobalProperties::base64URLDecodeTable);
  if (exponentResult.isErr()) {
    // TODO: Change to a DataError instance
    JS_ReportErrorLatin1(cx,
                         "The JWK member 'e' could not be base64url decoded or contained padding");
    return nullptr;
  }
  auto exponent = exponentResult.unwrap();

  // import public key
  auto publicKeyComponents = CryptoKeyRSAComponents::createPublic(modulus, exponent);
  return publicKeyComponents;
}

// https://datatracker.ietf.org/doc/html/rfc7518#section-6.3.2
// 6.3.2.  Parameters for RSA Private Keys
std::unique_ptr<CryptoKeyRSAComponents> createRSAPrivateKeyFromJWK(JSContext *cx, JsonWebKey *jwk) {
  // 2.10.1 If jwk does not meet the requirements of Section 6.3.2 of JSON Web Algorithms [JWA],
  // then throw a DataError. 2.10.2 Let privateKey represents the RSA private key identified by
  // interpreting jwk according to Section 6.3.2 of JSON Web Algorithms [JWA]. 2.10.3 If privateKey
  // is not a valid RSA private key according to [RFC3447], then throw a DataError.
  auto modulusResult = GlobalProperties::forgivingBase64Decode(
      jwk->n.value(), GlobalProperties::base64URLDecodeTable);
  if (modulusResult.isErr()) {
    // TODO: Change to a DataError instance
    JS_ReportErrorLatin1(cx,
                         "The JWK member 'n' could not be base64url decoded or contained padding");
    return nullptr;
  }
  auto modulus = modulusResult.unwrap();
  // Per RFC 7518 Section 6.3.1.1: https://tools.ietf.org/html/rfc7518#section-6.3.1.1
  if (modulus.starts_with('0')) {
    modulus = modulus.erase(0, 1);
  }
  auto dataResult = GlobalProperties::ConvertJSValueToByteString(cx, jwk->e.value());
  if (dataResult.isErr()) {
    // TODO: Change to a DataError instance
    JS_ReportErrorLatin1(cx, "Data provided to an operation does not meet requirements");
    return nullptr;
  }
  auto data = dataResult.unwrap();
  auto exponentResult =
      GlobalProperties::forgivingBase64Decode(data, GlobalProperties::base64URLDecodeTable);
  if (exponentResult.isErr()) {
    // TODO: Change to a DataError instance
    JS_ReportErrorLatin1(cx,
                         "The JWK member 'e' could not be base64url decoded or contained padding");
    return nullptr;
  }
  auto exponent = exponentResult.unwrap();
  auto privateExponentResult = GlobalProperties::forgivingBase64Decode(
      jwk->d.value(), GlobalProperties::base64URLDecodeTable);
  if (privateExponentResult.isErr()) {
    // TODO: Change to a DataError instance
    JS_ReportErrorLatin1(cx,
                         "The JWK member 'd' could not be base64url decoded or contained padding");
    return nullptr;
  }
  auto privateExponent = privateExponentResult.unwrap();
  if (!jwk->p.has_value() && !jwk->q.has_value() && !jwk->dp.has_value() && !jwk->dp.has_value() &&
      !jwk->qi.has_value()) {
    auto privateKeyComponents =
        CryptoKeyRSAComponents::createPrivate(modulus, exponent, privateExponent);
    return privateKeyComponents;
  }

  if (!jwk->p.has_value() || !jwk->q.has_value() || !jwk->dp.has_value() || !jwk->dq.has_value() ||
      !jwk->qi.has_value()) {
    // TODO: Change to a DataError instance
    JS_ReportErrorLatin1(cx, "Data provided to an operation does not meet requirements");
    return nullptr;
  }

  auto firstPrimeFactorResult = GlobalProperties::forgivingBase64Decode(
      jwk->p.value(), GlobalProperties::base64URLDecodeTable);
  if (firstPrimeFactorResult.isErr()) {
    // TODO: Change to a DataError instance
    JS_ReportErrorLatin1(cx,
                         "The JWK member 'p' could not be base64url decoded or contained padding");
    return nullptr;
  }
  auto firstPrimeFactor = firstPrimeFactorResult.unwrap();
  auto firstFactorCRTExponentResult = GlobalProperties::forgivingBase64Decode(
      jwk->dp.value(), GlobalProperties::base64URLDecodeTable);
  if (firstFactorCRTExponentResult.isErr()) {
    // TODO: Change to a DataError instance
    JS_ReportErrorLatin1(cx,
                         "The JWK member 'dp' could not be base64url decoded or contained padding");
    return nullptr;
  }
  auto firstFactorCRTExponent = firstFactorCRTExponentResult.unwrap();
  auto secondPrimeFactorResult = GlobalProperties::forgivingBase64Decode(
      jwk->q.value(), GlobalProperties::base64URLDecodeTable);
  if (secondPrimeFactorResult.isErr()) {
    // TODO: Change to a DataError instance
    JS_ReportErrorLatin1(cx,
                         "The JWK member 'q' could not be base64url decoded or contained padding");
    return nullptr;
  }
  auto secondPrimeFactor = secondPrimeFactorResult.unwrap();
  auto secondFactorCRTExponentResult = GlobalProperties::forgivingBase64Decode(
      jwk->dq.value(), GlobalProperties::base64URLDecodeTable);
  if (secondFactorCRTExponentResult.isErr()) {
    // TODO: Change to a DataError instance
    JS_ReportErrorLatin1(cx,
                         "The JWK member 'dq' could not be base64url decoded or contained padding");
    return nullptr;
  }
  auto secondFactorCRTExponent = secondFactorCRTExponentResult.unwrap();
  auto secondFactorCRTCoefficientResult = GlobalProperties::forgivingBase64Decode(
      jwk->qi.value(), GlobalProperties::base64URLDecodeTable);
  if (secondFactorCRTCoefficientResult.isErr()) {
    // TODO: Change to a DataError instance
    JS_ReportErrorLatin1(cx,
                         "The JWK member 'qi' could not be base64url decoded or contained padding");
    return nullptr;
  }
  auto secondFactorCRTCoefficient = secondFactorCRTCoefficientResult.unwrap();

  CryptoKeyRSAComponents::PrimeInfo firstPrimeInfo{firstPrimeFactor, firstFactorCRTExponent};

  CryptoKeyRSAComponents::PrimeInfo secondPrimeInfo{secondPrimeFactor, secondFactorCRTExponent,
                                                    secondFactorCRTCoefficient};

  if (!jwk->oth.size()) {
    auto privateKeyComponents = CryptoKeyRSAComponents::createPrivateWithAdditionalData(
        modulus, exponent, privateExponent, firstPrimeInfo, secondPrimeInfo, {});
    return privateKeyComponents;
  }

  std::vector<CryptoKeyRSAComponents::PrimeInfo> otherPrimeInfos;
  for (const auto &value : jwk->oth) {
    auto primeFactorResult =
        GlobalProperties::forgivingBase64Decode(value.r, GlobalProperties::base64URLDecodeTable);
    if (primeFactorResult.isErr()) {
      return nullptr;
    }
    auto primeFactor = primeFactorResult.unwrap();
    auto factorCRTExponentResult =
        GlobalProperties::forgivingBase64Decode(value.d, GlobalProperties::base64URLDecodeTable);
    if (factorCRTExponentResult.isErr()) {
      return nullptr;
    }
    auto factorCRTExponent = factorCRTExponentResult.unwrap();
    auto factorCRTCoefficientResult =
        GlobalProperties::forgivingBase64Decode(value.t, GlobalProperties::base64URLDecodeTable);
    if (factorCRTCoefficientResult.isErr()) {
      return nullptr;
    }
    auto factorCRTCoefficient = factorCRTCoefficientResult.unwrap();

    otherPrimeInfos.push_back(
        CryptoKeyRSAComponents::PrimeInfo(primeFactor, factorCRTExponent, factorCRTCoefficient));
  }

  auto privateKeyComponents = CryptoKeyRSAComponents::createPrivateWithAdditionalData(
      modulus, exponent, privateExponent, firstPrimeInfo, secondPrimeInfo, otherPrimeInfos);
  return privateKeyComponents;
}

const char *algorithmName(CryptoAlgorithmIdentifier algorithm) {
  switch (algorithm) {
  case CryptoAlgorithmIdentifier::RSASSA_PKCS1_v1_5: {
    return "RSASSA-PKCS1-v1_5";
  }
  case CryptoAlgorithmIdentifier::RSA_PSS: {
    return "RSA-PSS";
  }
  case CryptoAlgorithmIdentifier::RSA_OAEP: {
    return "RSA-OAEP";
  }
  case CryptoAlgorithmIdentifier::ECDSA: {
    return "ECDSA";
  }
  case CryptoAlgorithmIdentifier::ECDH: {
    return "ECDH";
  }
  case CryptoAlgorithmIdentifier::AES_CTR: {
    return "AES-CTR";
  }
  case CryptoAlgorithmIdentifier::AES_CBC: {
    return "AES-CBC";
  }
  case CryptoAlgorithmIdentifier::AES_GCM: {
    return "AES-GCM";
  }
  case CryptoAlgorithmIdentifier::AES_KW: {
    return "AES-KW";
  }
  case CryptoAlgorithmIdentifier::HMAC: {
    return "HMAC";
  }
  case CryptoAlgorithmIdentifier::SHA_1: {
    return "SHA-1";
  }
  case CryptoAlgorithmIdentifier::SHA_256: {
    return "SHA-256";
  }
  case CryptoAlgorithmIdentifier::SHA_384: {
    return "SHA-384";
  }
  case CryptoAlgorithmIdentifier::SHA_512: {
    return "SHA-512";
  }
  case CryptoAlgorithmIdentifier::HKDF: {
    return "HKDF";
  }
  case CryptoAlgorithmIdentifier::PBKDF2: {
    return "PBKDF2";
  }
  default: {
    MOZ_ASSERT_UNREACHABLE("Unknown `CryptoAlgorithmIdentifier` value");
  }
  }
}

// Web Crypto API uses DOMExceptions to indicate errors
// We are adding the fields which are tested for in Web Platform Tests
// TODO: Implement DOMExceptions class and use that instead of duck-typing on an Error instance
void convertErrorToNotSupported(JSContext *cx) {
  MOZ_ASSERT(JS_IsExceptionPending(cx));
  JS::RootedValue exn(cx);
  if (!JS_GetPendingException(cx, &exn)) {
    return;
  }
  MOZ_ASSERT(exn.isObject());
  JS::RootedObject error(cx, &exn.toObject());
  JS::RootedValue name(cx, JS::StringValue(JS_NewStringCopyZ(cx, "NotSupportedError")));
  JS_SetProperty(cx, error, "name", name);
  JS::RootedValue code(cx, JS::NumberValue(9));
  JS_SetProperty(cx, error, "code", code);
}

JS::Result<builtins::CryptoAlgorithmIdentifier> toHashIdentifier(JSContext *cx,
                                                                 JS::HandleValue value) {
  auto normalizedHashAlgorithm = CryptoAlgorithmDigest::normalize(cx, value);
  if (!normalizedHashAlgorithm) {
    return JS::Result<builtins::CryptoAlgorithmIdentifier>(JS::Error());
  }
  return normalizedHashAlgorithm->identifier();
}

// This implements the first section of
// https://w3c.github.io/webcrypto/#algorithm-normalization-normalize-an-algorithm which is shared
// across all the diffent algorithms, but importantly does not implement the parts to do with the
// chosen `op` (operation) the `op` parts are handled in the specialized `normalize` functions on
// the concrete classes which derive from CryptoAlgorithm such as CryptoAlgorithmDigest.
JS::Result<CryptoAlgorithmIdentifier> normalizeIdentifier(JSContext *cx, JS::HandleValue value) {

  // The specification states:
  // --------
  //    If alg is an instance of a DOMString:
  //    Return the result of running the normalize an algorithm algorithm,
  //    with the alg set to a new Algorithm dictionary whose name attribute
  //    is alg, and with the op set to op.
  // --------
  // Instead of doing that, we operate on the string and not the dictionary.
  // If we see a dictionary (JSObject), we pull the name attribute out
  // and coerce it's value to a String.
  // The reason we chose this direct is because we only need this one field
  // from the provided dictionary, so we store the field on it's own and not
  // in a JSObject which would take up more memory.

  // 1. Let registeredAlgorithms be the associative container stored at the op key of
  // supportedAlgorithms.
  // 2. Let initialAlg be the result of converting the ECMAScript object represented by alg to the
  // IDL dictionary type Algorithm, as defined by [WebIDL].
  // 3. If an error occurred, return the error and terminate this algorithm.
  // 4. Let algName be the value of the name attribute of initialAlg.
  JS::Rooted<JSString *> algName(cx);
  if (value.isObject()) {
    JS::Rooted<JSObject *> params(cx, &value.toObject());
    JS::Rooted<JS::Value> name_val(cx);
    if (!JS_GetProperty(cx, params, "name", &name_val)) {
      return JS::Result<CryptoAlgorithmIdentifier>(JS::Error());
    }
    algName.set(JS::ToString(cx, name_val));
  } else {
    algName.set(JS::ToString(cx, value));
  }
  // If `algName` is falsey, it means the call to JS::ToString failed.
  // In that scenario, we should already have an exception, which is why we are not creating our own
  // one.
  if (!algName) {
    return JS::Result<CryptoAlgorithmIdentifier>(JS::Error());
  }

  // TODO: We convert from JSString to std::string quite a lot in the codebase, should we pull this
  // logic out into a new function?
  size_t algorithmLen;
  JS::UniqueChars algorithmChars = encode(cx, algName, &algorithmLen);
  if (!algorithmChars) {
    return JS::Result<CryptoAlgorithmIdentifier>(JS::Error());
  }
  std::string algorithm(algorithmChars.get(), algorithmLen);

  // 5. If registeredAlgorithms contains a key that is a case-insensitive string match for algName:
  // 5.1 Set algName to the value of the matching key.
  // 5.2 Let desiredType be the IDL dictionary type stored at algName in registeredAlgorithms.
  // Note: We do not implement 5.2 here, it is instead implemented in the specialized `normalize`
  // functions.
  std::transform(algorithm.begin(), algorithm.end(), algorithm.begin(),
                 [](unsigned char c) { return std::toupper(c); });
  if (algorithm == "RSASSA-PKCS1-V1_5") {
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
  } else if (algorithm == "AES-KW") {
    return CryptoAlgorithmIdentifier::AES_KW;
  } else if (algorithm == "HMAC") {
    return CryptoAlgorithmIdentifier::HMAC;
  } else if (algorithm == "SHA-1") {
    return CryptoAlgorithmIdentifier::SHA_1;
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
    // Otherwise: Return a new NotSupportedError and terminate this algorithm.
    JS_ReportErrorUTF8(cx, "Algorithm: Unrecognized name");
    return JS::Result<CryptoAlgorithmIdentifier>(JS::Error());
  }
}
} // namespace

// clang-format off
/// This table is from https://w3c.github.io/webcrypto/#h-note-15
// | Algorithm					|	encrypt	|	decrypt	|	sign	|	verify	|	digest	|	generateKey	|	deriveKey	|	deriveBits	|	importKey	|	exportKey	|	wrapKey	|	unwrapKey	|
// |	RSASSA-PKCS1-v1_5	|					|					|		✔		|		✔			|					|			✔				| 					|							|			✔			|			✔			|					|						|
// |	RSA-PSS         	|					|					|		✔		|		✔			|					|			✔				|						|							|			✔			|			✔			|					|						|
// |	RSA-OAEP					|		✔			|		✔			|				|					|					|			✔				|						|							|			✔			|			✔			|		✔			|		✔				|
// |	ECDSA						 	|					|					|		✔		|		✔			|					|			✔				| 					|							|			✔			|			✔			|					|						|
// |	ECDH							|					|					|				|					|					|			✔				|			✔			|			✔				|			✔			|			✔			|					|						| 		
// |	AES-CTR						|		✔			|		✔			|				|					|					|			✔				|						|							|			✔			|			✔			|		✔			|		✔				|
// |	AES-CBC						|		✔			|		✔			|				|					|					|			✔				|						|							|			✔			|			✔			|		✔			|		✔				|
// |	AES-GCM						|		✔			|		✔			|				|					|					|			✔				|						|							|			✔			|			✔			|		✔			|		✔				|
// |	AES-KW						|					|					|				|					|					|			✔				|						|							|			✔			|			✔			|		✔			|		✔				|
// |	HMAC							|					|					|		✔		|		✔			|					|			✔				| 					|							|			✔			|			✔			|					|						|
// |	SHA-1							|					|					|				|					|		✔			|							|						|							|						|						|					|						|
// |	SHA-256						|					|					|				|					|		✔			|							|						|							|						|						|					|						|
// |	SHA-384						|					|					|				|					|		✔			|							|						|							|						|						|					|						|
// |	SHA-512						|					|					|				|					|		✔			|							|						|							|						|						|					|						|
// |	HKDF							|					|					|				|					|					|							|			✔			|			✔				|			✔			|						|					|						|
// |	PBKDF2						|					|					|				|					|					|							|			✔			|			✔				|			✔			|						|					|						|
//clang-format on

std::unique_ptr<CryptoAlgorithmDigest> CryptoAlgorithmDigest::normalize(JSContext *cx,
                                                                        JS::HandleValue value) {
  // Do steps 1 through 5.1 of https://w3c.github.io/webcrypto/#algorithm-normalization-normalize-an-algorithm
  auto identifierResult = normalizeIdentifier(cx, value);
  if (identifierResult.isErr()) {
    // If we are here, this means either the identifier could not be coerced to a String or was not recognized
    // In both those scenarios an exception will have already been created, which is why we are not creating one here.
    return nullptr;
  }
  auto identifier = identifierResult.unwrap();

  // The table listed at https://w3c.github.io/webcrypto/#h-note-15 is what defines which algorithms support which operations
  // SHA-1, SHA-256, SHA-384, and SHA-512 are the only algorithms which support the digest operation

  // Note: The specification states that none of the SHA algorithms take any parameters -- https://w3c.github.io/webcrypto/#sha-registration
  switch (identifier) {
  case CryptoAlgorithmIdentifier::SHA_1: {
    return std::make_unique<CryptoAlgorithmSHA1>();
  }
  case CryptoAlgorithmIdentifier::SHA_256: {
    return std::make_unique<CryptoAlgorithmSHA256>();
  }
  case CryptoAlgorithmIdentifier::SHA_384: {
    return std::make_unique<CryptoAlgorithmSHA384>();
  }
  case CryptoAlgorithmIdentifier::SHA_512: {
    return std::make_unique<CryptoAlgorithmSHA512>();
  }
  default: {
    JS_ReportErrorASCII(cx, "Supplied algorithm does not support the digest operation");
    convertErrorToNotSupported(cx);
    return nullptr;
  }
  }
};


std::unique_ptr<CryptoAlgorithmImportKey>
CryptoAlgorithmImportKey::normalize(JSContext *cx, JS::HandleValue value) {
  // Do steps 1 through 5.1 of https://w3c.github.io/webcrypto/#algorithm-normalization-normalize-an-algorithm
  auto identifierResult = normalizeIdentifier(cx, value);
  if (identifierResult.isErr()) {
    // If we are here, this means either the identifier could not be coerced to a String or was not recognized
    // In both those scenarios an exception will have already been created, which is why we are not creating one here.
    return nullptr;
  }
  auto identifier = identifierResult.unwrap();
  JS::RootedObject params(cx);

  // The value can either be a JS String or a JS Object with a 'name' property which is the algorithm identifier.
  // Other properties within the object will be the parameters for the algorithm to use.
  if (value.isString()) {
    auto obj = JS_NewPlainObject(cx);
    params.set(obj);
    JS_SetProperty(cx, params, "name", value);
  } else if (value.isObject()) {
    params.set(&value.toObject());
  }

  // The table listed at https://w3c.github.io/webcrypto/#h-note-15 is what defines which algorithms support which operations
  // RSASSA-PKCS1-v1_5, RSA-PSS, RSA-OAEP, ECDSA, ECDH, AES-CTR, AES-CBC, AES-GCM, AES-KW, HMAC, HKDF, PBKDF2 are the algorithms 
  // which support the importKey operation
  switch (identifier) {
  case CryptoAlgorithmIdentifier::RSASSA_PKCS1_v1_5: {
    return CryptoAlgorithmRSASSA_PKCS1_v1_5_Import::fromParameters(cx, params);
  }
  case CryptoAlgorithmIdentifier::RSA_PSS:
  case CryptoAlgorithmIdentifier::RSA_OAEP:
  case CryptoAlgorithmIdentifier::AES_CTR:
  case CryptoAlgorithmIdentifier::AES_CBC:
  case CryptoAlgorithmIdentifier::AES_GCM:
  case CryptoAlgorithmIdentifier::AES_KW:
  case CryptoAlgorithmIdentifier::HMAC:
  case CryptoAlgorithmIdentifier::ECDSA:
  case CryptoAlgorithmIdentifier::ECDH:
  case CryptoAlgorithmIdentifier::HKDF:
  case CryptoAlgorithmIdentifier::PBKDF2: {
    MOZ_ASSERT(false);
    JS_ReportErrorASCII(cx, "Supplied algorithm is not yet supported");
    convertErrorToNotSupported(cx);
    return nullptr;
  }
  default: {
    JS_ReportErrorASCII(cx, "Supplied algorithm does not support the importKey operation");
    convertErrorToNotSupported(cx);
    return nullptr;
  }
  }
};

std::unique_ptr<CryptoAlgorithmRSASSA_PKCS1_v1_5_Import> CryptoAlgorithmRSASSA_PKCS1_v1_5_Import::fromParameters(JSContext *cx, JS::HandleObject parameters) {
  JS::Rooted<JS::Value> hash_val(cx);
  if (!JS_GetProperty(cx, parameters, "hash", &hash_val)) {
    return nullptr;
  }
  auto hashIdentifier = toHashIdentifier(cx, hash_val);
  if (hashIdentifier.isErr()) {
    return nullptr;
  }
  return std::make_unique<CryptoAlgorithmRSASSA_PKCS1_v1_5_Import>(hashIdentifier.unwrap());
}

// https://w3c.github.io/webcrypto/#rsassa-pkcs1-operations
JSObject *CryptoAlgorithmRSASSA_PKCS1_v1_5_Import::importKey(JSContext *cx, CryptoKeyFormat format,
                                                             KeyData keyData, bool extractable,
                                                             CryptoKeyUsages usages) {
  MOZ_ASSERT(cx);
  JS::RootedObject result(cx);
  switch (format) {
  // 2. If format is "jwk":
  case CryptoKeyFormat::Jwk: {
    // 2.1. If keyData is a JsonWebKey dictionary:
      // Let jwk equal keyData.
    // Otherwise:
      // Throw a DataError.
    auto jwk = std::get<JsonWebKey *>(keyData);
    if (!jwk) {
      // TODO Rename error to DataError
      JS_ReportErrorLatin1(cx, "Supplied format is not a JSONWebKey");
      return nullptr;
    }


    // 2.2 If the d field of jwk is present and usages contains an entry which 
    // is not "sign", or, if the d field of jwk is not present and usages 
    // contains an entry which is not "verify" then throw a SyntaxError.
    bool isUsagesAllowed = false;
    // public key
    if (jwk->d.has_value()) {
      isUsagesAllowed = usages.canSign();
    } else {
      // private key
      isUsagesAllowed = usages.canVerify();
    }
    if (!isUsagesAllowed) {
      // TODO Rename error to SyntaxError
      JS_ReportErrorLatin1(cx,
                           "The JWK 'key_ops' member was inconsistent with that specified by the "
                           "Web Crypto call. The JWK usage must be a superset of those requested");
      return nullptr;
    }

    // 2.3 If the kty field of jwk is not a case-sensitive string match to "RSA", then throw a DataError.
    
    // Step 2.3 has already been done in the other implementation of 
    // CryptoAlgorithmRSASSA_PKCS1_v1_5_Import::importKey which is called before this one.

    // 2.4. If usages is non-empty and the use field of jwk is present and is
    // not a case-sensitive string match to "sig", then throw a DataError.
    if (!usages.isEmpty() && jwk->use.has_value() && jwk->use != "sig") {
      // TODO Rename error to DataError
      JS_ReportErrorLatin1(cx, "Operation not permitted");
      return nullptr;
    }

    // 2.5. If the key_ops field of jwk is present, and is invalid according to 
    // the requirements of JSON Web Key [JWK] or does not contain all of the 
    // specified usages values, then throw a DataError.
    if (jwk->key_ops.size() > 0) {
      auto ops = CryptoKeyUsages::from(jwk->key_ops);
      if (!ops.isSuperSetOf(usages)) {
        // TODO Rename error to DataError
        JS_ReportErrorASCII(cx,
                            "The JWK 'key_ops' member was inconsistent with that specified by the "
                            "Web Crypto call. The JWK usage must be a superset of those requested");
        return nullptr;
      }
    }

    // 2.6 If the ext field of jwk is present and has the value false and 
    // extractable is true, then throw a DataError.
    if (jwk->ext && !jwk->ext.value() && extractable) {
      // TODO: Change to a DataError instance
      JS_ReportErrorLatin1(cx, "Data provided to an operation does not meet requirements");
      return nullptr;
    }

    // 2.7 Let hash be a be a string whose initial value is undefined.
    // 2.8 If the alg field of jwk is not present:
        // Let hash be undefined.
      // If the alg field is equal to the string "RS1":
        // Let hash be the string "SHA-1".
      // If the alg field is equal to the string "RS256":
        // Let hash be the string "SHA-256".
      // If the alg field is equal to the string "RS384":
        // Let hash be the string "SHA-384".
      // If the alg field is equal to the string "RS512":
        // Let hash be the string "SHA-512".
      // Otherwise:
        // Perform any key import steps defined by other applicable specifications, passing format, jwk and obtaining hash.
        // If an error occurred or there are no applicable specifications, throw a DataError.
    // 2.9 If hash is not undefined:
      // Let normalizedHash be the result of normalize an algorithm with alg set to hash and op set to digest.
      // If normalizedHash is not equal to the hash member of normalizedAlgorithm, throw a DataError.
    bool isMatched = false;
    switch (this->hashIdentifier) {
    case CryptoAlgorithmIdentifier::SHA_1: {
      isMatched = !jwk->alg.has_value() || jwk->alg == "RS1";
      break;
    }
    case CryptoAlgorithmIdentifier::SHA_256: {
      isMatched = !jwk->alg.has_value() || jwk->alg == "RS256";
      break;
    }
    case CryptoAlgorithmIdentifier::SHA_384: {
      isMatched = !jwk->alg.has_value() || jwk->alg == "RS384";
      break;
    }
    case CryptoAlgorithmIdentifier::SHA_512: {
      isMatched = !jwk->alg.has_value() || jwk->alg == "RS512";
      break;
    }
    default: {
      break;
    }
    }
    if (!isMatched) {
      JS_ReportErrorLatin1(
          cx, "The JWK 'alg' member was inconsistent with that specified by the Web Crypto call");
      return nullptr;
    }

    std::unique_ptr<CryptoKeyRSAComponents> key = nullptr;
    // 2.10 If the d field of jwk is present:
    if (jwk->d.has_value()) {
      // 2.10.1 If jwk does not meet the requirements of Section 6.3.2 of JSON Web Algorithms [JWA], then throw a DataError.
      // 2.10.2 Let privateKey represents the RSA private key identified by interpreting jwk according to Section 6.3.2 of JSON Web Algorithms [JWA].
      // 2.10.3 If privateKey is not a valid RSA private key according to [RFC3447], then throw a DataError.
      key = createRSAPrivateKeyFromJWK(cx, jwk);
      if (!key) {
        return nullptr;
      }
    // 2.10 Otherwise:
    } else {
      // 2.10.1 If jwk does not meet the requirements of Section 6.3.1 of JSON Web Algorithms [JWA], then throw a DataError.
      // 2.10.2 Let publicKey represent the RSA public key identified by interpreting jwk according to Section 6.3.1 of JSON Web Algorithms [JWA].
      // 2.10.3 If publicKey can be determined to not be a valid RSA public key according to [RFC3447], then throw a DataError.
      key = createRSAPublicKeyFromJWK(cx, jwk);
      if (!key) {
        return nullptr;
      }
    }
    // 2.10.4 Let key be a new CryptoKey object that represents privateKey.
    // 2.10.5 Set the [[type]] internal slot of key to "private"
    // 3. Let algorithm be a new RsaHashedKeyAlgorithm dictionary.
    // 4. Set the name attribute of algorithm to "RSASSA-PKCS1-v1_5"
    // 5. Set the modulusLength attribute of algorithm to the length, in bits, of the RSA public modulus.
    // 6. Set the publicExponent attribute of algorithm to the BigInteger representation of the RSA public exponent.
    // 7. Set the hash attribute of algorithm to the hash member of normalizedAlgorithm.
    // 8. Set the [[algorithm]] internal slot of key to algorithm.
    // 9. Return key.
    return CryptoKey::createRSA(cx, this, std::move(key), extractable, usages);
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
}

JSObject *CryptoAlgorithmRSASSA_PKCS1_v1_5_Import::importKey(JSContext *cx,
                                                             CryptoKeyFormat format,
                                                             JS::HandleValue key_data,
                                                             bool extractable,
                                                             CryptoKeyUsages usages) {
  MOZ_ASSERT(cx);
  // The only support formats for RSASSA PKCS1 v1-5 are spki, pkcs8, and jwk.
  if (format == CryptoKeyFormat::Raw) {
    MOZ_ASSERT_UNREACHABLE("coding error");
    return nullptr;
  }

  KeyData data;
  if (format == CryptoKeyFormat::Jwk) {
    // This handles set 2.3: If the kty field of jwk is not a case-sensitive string match to "RSA", then throw a DataError.
    auto jwk = JsonWebKey::parse(cx, key_data, "RSA");
    if (!jwk) {
      return nullptr;
    }
    data = jwk.release();
  } else {
    std::optional<std::span<uint8_t>> buffer = value_to_buffer(cx, key_data, "");
    if (!buffer.has_value()) {
      // value_to_buffer would have already created a JS exception so we don't need to create one
      // ourselves.
      return nullptr;
    }
    data = buffer.value();
  }
  return this->importKey(cx, format, data, extractable, usages);
}

JSObject *CryptoAlgorithmRSASSA_PKCS1_v1_5_Import::toObject(JSContext *cx) {
  // Let algorithm be a new RsaHashedKeyAlgorithm dictionary.
  JS::RootedObject algorithm(cx, JS_NewPlainObject(cx));

  // Set the name attribute of algorithm to "RSASSA-PKCS1-v1_5"
  auto alg_name = JS_NewStringCopyZ(cx, this->name());
  if (!alg_name) {
    return nullptr;
  }
  JS::RootedValue name_val(cx, JS::StringValue(alg_name));
  if (!JS_SetProperty(cx, algorithm, "name", name_val)) {
    return nullptr;
  }

  // Set the hash attribute of algorithm to the hash member of normalizedAlgorithm.
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
  if (!JS_SetProperty(cx, algorithm, "hash", hash_val)) {
    return nullptr;
  }
  return algorithm;
}

namespace {
  // This implements https://w3c.github.io/webcrypto/#sha-operations for all
  // the SHA algorithms that we support.
  JSObject *digest(JSContext *cx, std::span<uint8_t> data, const EVP_MD * algorithm, size_t buffer_size) {
    unsigned int size;
    auto buf = static_cast<unsigned char *>(JS_malloc(cx, buffer_size));
    if (!buf) {
      JS_ReportOutOfMemory(cx);
      return nullptr;
    }
    if (!EVP_Digest(data.data(), data.size(), buf, &size, algorithm, NULL)) {
      // 2. If performing the operation results in an error, then throw an OperationError.
      // TODO: Change to an OperationError DOMException
      JS_ReportErrorUTF8(cx, "SubtleCrypto.digest: failed to create digest");
      return nullptr;
    }
    // 3. Return a new ArrayBuffer containing result.
    JS::RootedObject array_buffer(cx);
    array_buffer.set(JS::NewArrayBufferWithContents(cx, size, buf));
    if (!array_buffer) {
      JS_ReportOutOfMemory(cx);
      return nullptr;
    }
    return array_buffer;
  };
}

JSObject *CryptoAlgorithmSHA1::digest(JSContext *cx, std::span<uint8_t> data) {
  return ::builtins::digest(cx, data, EVP_sha1(), SHA_DIGEST_LENGTH);
}
JSObject *CryptoAlgorithmSHA256::digest(JSContext *cx, std::span<uint8_t> data) {
  return ::builtins::digest(cx, data, EVP_sha256(), SHA256_DIGEST_LENGTH);
}
JSObject *CryptoAlgorithmSHA384::digest(JSContext *cx, std::span<uint8_t> data) {
  return ::builtins::digest(cx, data, EVP_sha384(), SHA384_DIGEST_LENGTH);
}
JSObject *CryptoAlgorithmSHA512::digest(JSContext *cx, std::span<uint8_t> data) {
  return ::builtins::digest(cx, data, EVP_sha512(), SHA512_DIGEST_LENGTH);
}

} // namespace builtins