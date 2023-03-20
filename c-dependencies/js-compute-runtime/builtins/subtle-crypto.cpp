#include <algorithm>
#include <cctype>
#include <iostream>
#include <iterator>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "js/ArrayBuffer.h"
// TODO: remove these once the warnings are fixed
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-offsetof"
#include "js/Conversions.h"
#include "js/experimental/TypedData.h"
#pragma clang diagnostic pop
#include <mozilla/Unused.h>

#include "crypto-key.h"
#include "host_interface/c-at-e.h"
#include "js-compute-builtins.h" // for encode
#include "subtle-crypto.h"

namespace builtins {
namespace {
std::vector<CryptoAlgorithmIdentifier> supportedEncryptAlgorithms{
    CryptoAlgorithmIdentifier::RSAES_PKCS1_v1_5, CryptoAlgorithmIdentifier::RSA_OAEP,
    CryptoAlgorithmIdentifier::AES_CBC,          CryptoAlgorithmIdentifier::AES_CFB,
    CryptoAlgorithmIdentifier::AES_CTR,          CryptoAlgorithmIdentifier::AES_GCM};
std::vector<CryptoAlgorithmIdentifier> supportedDecryptAlgorithms = supportedEncryptAlgorithms;
std::vector<CryptoAlgorithmIdentifier> supportedSignAlgorithms{
    CryptoAlgorithmIdentifier::RSASSA_PKCS1_v1_5, CryptoAlgorithmIdentifier::HMAC,
    CryptoAlgorithmIdentifier::ECDSA, CryptoAlgorithmIdentifier::RSA_PSS};
std::vector<CryptoAlgorithmIdentifier> supportedVerifyAlgorithms = supportedSignAlgorithms;
std::vector<CryptoAlgorithmIdentifier> supportedDigestAlgorithms{
    CryptoAlgorithmIdentifier::SHA_1, CryptoAlgorithmIdentifier::SHA_224,
    CryptoAlgorithmIdentifier::SHA_256, CryptoAlgorithmIdentifier::SHA_384,
    CryptoAlgorithmIdentifier::SHA_512};
std::vector<CryptoAlgorithmIdentifier> supportedGenerateKeyAlgorithms{
    CryptoAlgorithmIdentifier::RSAES_PKCS1_v1_5,
    CryptoAlgorithmIdentifier::RSASSA_PKCS1_v1_5,
    CryptoAlgorithmIdentifier::RSA_PSS,
    CryptoAlgorithmIdentifier::RSA_OAEP,
    CryptoAlgorithmIdentifier::AES_CTR,
    CryptoAlgorithmIdentifier::AES_CBC,
    CryptoAlgorithmIdentifier::AES_GCM,
    CryptoAlgorithmIdentifier::AES_CFB,
    CryptoAlgorithmIdentifier::AES_KW,
    CryptoAlgorithmIdentifier::HMAC,
    CryptoAlgorithmIdentifier::ECDSA,
    CryptoAlgorithmIdentifier::ECDH};
std::vector<CryptoAlgorithmIdentifier> supportedDeriveBitsAlgorithms{
    CryptoAlgorithmIdentifier::ECDH, CryptoAlgorithmIdentifier::HKDF,
    CryptoAlgorithmIdentifier::PBKDF2};
std::vector<CryptoAlgorithmIdentifier> supportedImportKeyAlgorithms{
    CryptoAlgorithmIdentifier::RSAES_PKCS1_v1_5,
    CryptoAlgorithmIdentifier::RSASSA_PKCS1_v1_5,
    CryptoAlgorithmIdentifier::RSA_PSS,
    CryptoAlgorithmIdentifier::RSA_OAEP,
    CryptoAlgorithmIdentifier::AES_CTR,
    CryptoAlgorithmIdentifier::AES_CBC,
    CryptoAlgorithmIdentifier::AES_GCM,
    CryptoAlgorithmIdentifier::AES_CFB,
    CryptoAlgorithmIdentifier::AES_KW,
    CryptoAlgorithmIdentifier::HMAC,
    CryptoAlgorithmIdentifier::ECDSA,
    CryptoAlgorithmIdentifier::ECDH,
    CryptoAlgorithmIdentifier::HKDF,
    CryptoAlgorithmIdentifier::PBKDF2};
std::vector<CryptoAlgorithmIdentifier> supportedWrapKeyAlgorithms{
    CryptoAlgorithmIdentifier::AES_KW};
std::vector<CryptoAlgorithmIdentifier> supportedUnwrapKeyAlgorithms = supportedWrapKeyAlgorithms;
std::vector<CryptoAlgorithmIdentifier> supportedGetKeyLengthAlgorithms{
    CryptoAlgorithmIdentifier::AES_CTR, CryptoAlgorithmIdentifier::AES_CBC,
    CryptoAlgorithmIdentifier::AES_GCM, CryptoAlgorithmIdentifier::AES_CFB,
    CryptoAlgorithmIdentifier::AES_KW,  CryptoAlgorithmIdentifier::HMAC,
    CryptoAlgorithmIdentifier::HKDF,    CryptoAlgorithmIdentifier::PBKDF2};
} // namespace

bool SubtleCrypto::exportKey(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS::CallArgs args = CallArgsFromVp(argc, vp);
  // 4. Let promise be a new Promise.
  JS::RootedObject promise(cx, JS::NewPromiseObject(cx, nullptr));
  if (!promise) {
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }
  // 5. Return promise and perform the remaining steps in parallel.
  args.rval().setObject(*promise);
  if (!args.requireAtLeast(cx, "SubtleCrypto.exportKey", 2)) {
    return RejectPromiseWithPendingError(cx, promise);
  }
  if (!check_receiver(cx, args.thisv(), "SubtleCrypto.exportKey")) {
    return RejectPromiseWithPendingError(cx, promise);
  }

  CryptoKeyFormat format;
  {
    auto format_arg = args.get(0);
    size_t format_length;
    // Convert into a String following https://tc39.es/ecma262/#sec-tostring
    JS::UniqueChars format_chars = encode(cx, format_arg, &format_length);
    if (!format_chars || format_length == 0) {
      return RejectPromiseWithPendingError(cx, promise);
    }
    std::string_view format_string(format_chars.get(), format_length);
    if (format_string == "spki") {
      format = CryptoKeyFormat::Spki;
    } else if (format_string == "pkcs8") {
      format = CryptoKeyFormat::Pkcs8;
    } else if (format_string == "jwk"){
      format = CryptoKeyFormat::Jwk;
    } else if (format_string == "raw"){
      format = CryptoKeyFormat::Raw;
    } else {
      // TODO: Change to an OperationError instance
      JS_ReportErrorLatin1(cx, "Provided format parameter is not supported. Supported formats are: 'spki', 'pkcs8', 'jwk', and 'raw'");
      return RejectPromiseWithPendingError(cx, promise);
    }
  }
  auto key_data = args.get(1);

  if (!key_data.isObject()) {
    // TODO: Change to TypeError
    JS_ReportErrorLatin1(cx, "parameter 2 is not of type 'CryptoKey'");
    return RejectPromiseWithPendingError(cx, promise);
  }

  JS::RootedObject key(cx, &key_data.toObject());

  JSObject* result = CryptoAlgorithmRSASSA_PKCS1_v1_5::exportKey(cx, format, key);
  if (!result) {
    return RejectPromiseWithPendingError(cx, promise);
  }
  // 9. If the [[type]] internal slot of result is "secret" or "private" and
  // usages is empty, then throw a SyntaxError.

  // 10. Set the [[extractable]] internal slot of result to extractable.

  // 11. Set the [[usages]] internal slot of result to the normalized value of
  // usages.

  // TODO: finish the above.

  // 12. Resolve promise with result.
  JS::RootedValue result_val(cx);
  result_val.setObject(*result);
  JS::ResolvePromise(cx, promise, result_val);

  return true;
}

// Promise<any> generateKey(AlgorithmIdentifier algorithm,
//                         boolean extractable,
//                         sequence<KeyUsage> keyUsages );
// https://w3c.github.io/webcrypto/#SubtleCrypto-method-generateKey
bool SubtleCrypto::generateKey(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS::CallArgs args = CallArgsFromVp(argc, vp);
  // 4. Let promise be a new Promise.
  JS::RootedObject promise(cx, JS::NewPromiseObject(cx, nullptr));
  if (!promise) {
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }
  // 5. Return promise and perform the remaining steps in parallel.
  args.rval().setObject(*promise);
  if (!check_receiver(cx, args.thisv(), "SubtleCrypto.generateKey")) {
    return RejectPromiseWithPendingError(cx, promise);
  }
  if (!args.requireAtLeast(cx, "SubtleCrypto.generateKey", 3)) {
    return RejectPromiseWithPendingError(cx, promise);
  }
  // 1. Let algorithm, extractable and usages be the algorithm, extractable and keyUsages parameters
  // passed to the generateKey() method, respectively.
  auto algorithm = args.get(0);
  auto extractable = args.get(1).toBoolean();
  auto keyUsages = args.get(2);

  std::string_view error_message("SubtleCrypto.generateKey: Operation is not supported");
  auto keyUsageMaskResult = builtins::CryptoKey::toKeyUsageBitmap(cx, keyUsages, error_message);
  if (keyUsageMaskResult.isErr()) {
    // TODO Rename error to NotSupportedError
    return RejectPromiseWithPendingError(cx, promise);
  }
  auto keyUsageMask = keyUsageMaskResult.unwrap();

  // 2. Let normalizedAlgorithm be the result of normalizing an algorithm, with alg set to algorithm
  // and op set to "generateKey".
  // 3. If an error occurred, return a Promise rejected with normalizedAlgorithm.
  auto normalizedAlgorithm = CryptoAlgorithmGenerateKey::normalize(cx, algorithm);
  if (!normalizedAlgorithm) {
    // TODO Rename error to NotSupportedError
    return RejectPromiseWithPendingError(cx, promise);
  }

  // 6. If the following steps or referenced procedures say to throw an error, reject promise with
  // the returned error and then terminate the algorithm
  // 7. Let result be the result of performing the generate key operation specified by
  // normalizedAlgorithm using algorithm, extractable and usages.


  JS::RootedValue result(cx);
  JSObject *cryptoKey = normalizedAlgorithm->generateKey(cx, extractable, keyUsageMask);
  if (!cryptoKey) {
    if (!JS_IsExceptionPending(cx)) {
      JS_ReportErrorUTF8(cx, "SubtleCrypto.generateKey: Operation failed");
    }
    // TODO Rename error to NotSupportedError
    return RejectPromiseWithPendingError(cx, promise);
  }
  result.setObject(*cryptoKey);
  
  // 8. If result is a CryptoKey object:
  //      If the [[type]] internal slot of result is "secret" or "private" and usages is empty, then
  //      throw a SyntaxError.
  if (CryptoKey::is_instance(cryptoKey)) {
    CryptoKeyType type = CryptoKey::type(cryptoKey);
    if (type == CryptoKeyType::Private || type == CryptoKeyType::Secret) {
      if (keyUsageMask == 0) {
        JS_ReportErrorUTF8(cx, "SubtleCrypto.generateKey: Operation is not supported");
        // TODO Rename error to NotSupportedError
        return RejectPromiseWithPendingError(cx, promise);
      }
    }
  }
  //    If result is a CryptoKeyPair object:
  //      If the [[usages]] internal slot of the privateKey attribute of result is the empty
  //      sequence, then throw a SyntaxError.
  // TODO: the above.

  // 9. Resolve promise with result.
  JS::ResolvePromise(cx, promise, result);

  return true;
}

//  Promise<CryptoKey> importKey(KeyFormat format,
//                         (BufferSource or JsonWebKey) keyData,
//                         AlgorithmIdentifier algorithm,
//                         boolean extractable,
//                         sequence<KeyUsage> keyUsages );
// https://w3c.github.io/webcrypto/#dfn-SubtleCrypto-method-importKey
bool SubtleCrypto::importKey(JSContext *cx, unsigned argc, JS::Value *vp) {
  MOZ_ASSERT(cx);
  JS::CallArgs args = CallArgsFromVp(argc, vp);
  // 5. Let promise be a new Promise.
  JS::RootedObject promise(cx, JS::NewPromiseObject(cx, nullptr));
  if (!promise) {
    JS_ReportErrorASCII(cx, "InternalError");
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }

  // 6. Return promise and perform the remaining steps in parallel.
  args.rval().setObject(*promise);

  if (!args.requireAtLeast(cx, "SubtleCrypto.importKey", 5)) {
    return RejectPromiseWithPendingError(cx, promise);
  }
  if (!check_receiver(cx, args.thisv(), "SubtleCrypto.importKey")) {
    return RejectPromiseWithPendingError(cx, promise);
  }

  // 1. Let format, algorithm, extractable and usages, be the format, algorithm,
  // extractable and keyUsages parameters passed to the importKey() method, respectively.
  CryptoKeyFormat format;
  {
    auto format_arg = args.get(0);
    size_t format_length;
    // Convert into a String following https://tc39.es/ecma262/#sec-tostring
    JS::UniqueChars format_chars = encode(cx, format_arg, &format_length);
    if (!format_chars || format_length == 0) {
      return RejectPromiseWithPendingError(cx, promise);
    }
    std::string_view format_string(format_chars.get(), format_length);
    if (format_string == "spki") {
      format = CryptoKeyFormat::Spki;
    } else if (format_string == "pkcs8") {
      format = CryptoKeyFormat::Pkcs8;
    } else if (format_string == "jwk"){
      format = CryptoKeyFormat::Jwk;
    } else if (format_string == "raw"){
      format = CryptoKeyFormat::Raw;
    } else {
      // TODO: Change to a SyntaxError instance
      JS_ReportErrorLatin1(cx, "Provided format parameter is not supported. Supported formats are: 'spki', 'pkcs8', 'jwk', and 'raw'");
      return RejectPromiseWithPendingError(cx, promise);
    }
  }
  auto key_data = args.get(1);
  auto algorithm = args.get(2);
  bool extractable = JS::ToBoolean(args.get(3));
  CryptoKeyUsageBitmap usages;
  {
    auto usages_arg = args.get(4);

    std::string_view error_message("SubtleCrypto.importKey: Invalid keyUsages argument");
    auto keyUsageMaskResult = builtins::CryptoKey::toKeyUsageBitmap(cx, usages_arg, error_message);
    if (keyUsageMaskResult.isErr()) {
      return RejectPromiseWithPendingError(cx, promise);
    }
    usages = keyUsageMaskResult.unwrap();
  }


  // 3. Let normalizedAlgorithm be the result of normalizing an algorithm, with alg set to algorithm
  // and op set to "importKey".
  // 4. If an error occurred, return a Promise rejected with normalizedAlgorithm.
  auto normalizedAlgorithm = CryptoAlgorithmImportKey::normalize(cx, algorithm);
  if (!normalizedAlgorithm) {
    return RejectPromiseWithPendingError(cx, promise);
  }

  // 7. If the following steps or referenced procedures say to throw an error,
  // reject promise with the returned error and then terminate the algorithm.
  // 8. Let result be the CryptoKey object that results from performing the
  // import key operation specified by normalizedAlgorithm using keyData,
  // algorithm, format, extractable and usages.
  JS::RootedObject result(cx);
  JSObject* key = normalizedAlgorithm->importKey(cx, format, key_data, extractable, usages);
  if (!key) {
    return RejectPromiseWithPendingError(cx, promise);
  }
  // 9. If the [[type]] internal slot of result is "secret" or "private" and
  // usages is empty, then throw a SyntaxError.

  // 10. Set the [[extractable]] internal slot of result to extractable.

  // 11. Set the [[usages]] internal slot of result to the normalized value of
  // usages.

  // TODO: finish the above.

  // 12. Resolve promise with result.
  JS::RootedValue result_val(cx);
  result_val.setObject(*key);
  JS::ResolvePromise(cx, promise, result_val);

  return true;
}

// Promise<any> verify(AlgorithmIdentifier algorithm,
//                     CryptoKey key,
//                     BufferSource signature,
//                     BufferSource data);
// https://w3c.github.io/webcrypto/#dfn-SubtleCrypto-method-verify
bool SubtleCrypto::verify(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS::CallArgs args = CallArgsFromVp(argc, vp);
  // 6. Let promise be a new Promise.
  JS::RootedObject promise(cx, JS::NewPromiseObject(cx, nullptr));
  if (!promise) {
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }

  // 7. Return promise and perform the remaining steps in parallel.
  args.rval().setObject(*promise);

  if (!check_receiver(cx, args.thisv(), "SubtleCrypto.verify")) {
    return RejectPromiseWithPendingError(cx, promise);
  }
  if (!args.requireAtLeast(cx, "SubtleCrypto.verify", 4)) {
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }

  // 1. Let algorithm and key be the algorithm and key parameters passed to the verify() method,
  // respectively.
  auto algorithm = args.get(0);
  auto key_arg = args.get(1);
  if (!key_arg.isObject()) {
    JS_ReportErrorLatin1(cx, "parameter 2 is not of type 'CryptoKey'");
    return RejectPromiseWithPendingError(cx, promise);
  }
  JS::RootedObject key(cx, &key_arg.toObject());
  if (!key) {
    // todo: error message
    return RejectPromiseWithPendingError(cx, promise);
  }

  if (!CryptoKey::is_instance(key)) {
    JS_ReportErrorASCII(
        cx, "SubtleCrypto.verify: key (argument 2)  does not implement interface CryptoKey");
    return RejectPromiseWithPendingError(cx, promise);
  }

  // 2. Let signature be the result of getting a copy of the bytes held by the signature
  // parameter passed to the verify() method.
  std::optional<std::span<uint8_t>> signature = value_to_buffer(
      cx, args.get(2), "SubtleCrypto.verify: signature (argument 3)");
  if (!signature) {
    return RejectPromiseWithPendingError(cx, promise);
  }

  // 3. Let data be the result of getting a copy of the bytes held by the data parameter passed
  // to the verify() method.
  std::optional<std::span<uint8_t>> data =
      value_to_buffer(cx, args.get(3), "SubtleCrypto.verify: data (argument 4)");
  if (!data) {
    return RejectPromiseWithPendingError(cx, promise);
  }

  // 4. Let normalizedAlgorithm be the result of normalizing an algorithm, with alg set to
  // algorithm and op set to "verify".
  // 5. If an error occurred, return a Promise rejected with normalizedAlgorithm.
  auto normalizedAlgorithm = CryptoAlgorithmSignVerify::normalize(cx, algorithm);
  if (!normalizedAlgorithm) {
    // TODO Rename error to NotSupportedError
    return RejectPromiseWithPendingError(cx, promise);
  }

  // 8. If the following steps or referenced procedures say to throw an error, reject promise
  // with the returned error and then terminate the algorithm.
  // 9. If the name member of normalizedAlgorithm is not equal to the name attribute of the
  // [[algorithm]] internal slot of key then throw an InvalidAccessError.
  auto identifier = normalizedAlgorithm->identifier();
  auto match_result = CryptoKey::is_algorithm(cx, key, identifier);
  if (match_result.isErr() || match_result.unwrap() == false) {
    // TODO: Change to an InvalidAccessError instance
    JS_ReportErrorUTF8(cx, "CryptoKey doesn't match AlgorithmIdentifier");
    return RejectPromiseWithPendingError(cx, promise);
  }
  // 10. If the [[usages]] internal slot of key does not contain an entry that is "verify", then
  // throw an InvalidAccessError.
  if (!CryptoKey::hasKeyUsage(key, static_cast<uint8_t>(builtins::CryptoKeyUsageVerify))) {
    // TODO: Change to an InvalidAccessError instance
    JS_ReportErrorUTF8(cx, "CryptoKey doesn't support verification");
    return RejectPromiseWithPendingError(cx, promise);
  }
  // 11. Let result be the result of performing the verify operation specified by
  // normalizedAlgorithm using key, algorithm and signature and with data as message.

  auto matchResult = normalizedAlgorithm->verify(cx, key, signature.value(), data.value()); 

  if (matchResult.isErr()) {
    JS_ReportErrorUTF8(cx, "Crypto verification failed");
    return RejectPromiseWithPendingError(cx, promise);
  }
  // 12. Resolve promise with result.
  JS::RootedValue result(cx);
  result.setBoolean(matchResult.unwrap());
  JS::ResolvePromise(cx, promise, result);

  return true;
}

// digest(algorithm, data)
// https://w3c.github.io/webcrypto/#SubtleCrypto-method-digest
bool SubtleCrypto::digest(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS::CallArgs args = CallArgsFromVp(argc, vp);
  // 5. Let promise be a new Promise.
  JS::RootedObject promise(cx, JS::NewPromiseObject(cx, nullptr));
  if (!promise) {
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }

  // 6. Return promise and perform the remaining steps in parallel.
  args.rval().setObject(*promise);
  if (!check_receiver(cx, args.thisv(), "SubtleCrypto.digest")) {
    return RejectPromiseWithPendingError(cx, promise);
  }
  if (!args.requireAtLeast(cx, "SubtleCrypto.digest", 2)) {
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }
  // 1. Let algorithm be the algorithm parameter passed to the digest() method.
  auto algorithm = args.get(0);

  // 2 . Let data be the result of getting a copy of the bytes held by the data parameter
  // passed to the digest() method.
  auto data = value_to_buffer(cx, args.get(1), "SubtleCrypto.digest: data");
  if (!data.has_value()) {
    // value_to_buffer would have already created a JS exception so we don't need to create one ourselves.
    return RejectPromiseWithPendingError(cx, promise);
  }

  // 3. Let normalizedAlgorithm be the result of normalizing an algorithm, with alg set to
  // algorithm and op set to "digest".
  // 4. If an error occurred, return a Promise rejected with normalizedAlgorithm.
  auto normalizedAlgorithm = CryptoAlgorithmDigest::normalize(cx, algorithm);
  if (!normalizedAlgorithm) {
    // TODO Rename error to NotSupportedError
    return RejectPromiseWithPendingError(cx, promise);
  }

  // 7. If the following steps or referenced procedures say to throw an error, reject promise with
  // the returned error and then terminate the algorithm.
  // 8. Let result be the result of performing the digest operation specified by normalizedAlgorithm
  // using algorithm, with data as message.

  auto array_buffer = normalizedAlgorithm->digest(cx, data.value());
  if (!array_buffer) {
    return RejectPromiseWithPendingError(cx, promise);
  }
  
  // 9. Resolve promise with result.
  JS::RootedValue result(cx);
  result.setObject(*array_buffer);
  JS::ResolvePromise(cx, promise, result);

  return true;
}

// https://w3c.github.io/webcrypto/#SubtleCrypto-method-sign
bool SubtleCrypto::sign(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS::CallArgs args = CallArgsFromVp(argc, vp);
  // 5. Let promise be a new Promise.
  JS::RootedObject promise(cx, JS::NewPromiseObject(cx, nullptr));
  if (!promise) {
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }
  // 6. Return promise and perform the remaining steps in parallel.
  args.rval().setObject(*promise);

  if (!check_receiver(cx, args.thisv(), "SubtleCrypto.sign")) {
    return RejectPromiseWithPendingError(cx, promise);
  }
  if (!args.requireAtLeast(cx, "SubtleCrypto.sign", 3)) {
    return RejectPromiseWithPendingError(cx, promise);
  }

  // 1. Let algorithm and key be the algorithm and key parameters passed to the sign() method,
  // respectively.
  auto algorithm = args.get(0);
  auto key_arg = args.get(1);
  if (!key_arg.isObject()) {
    JS_ReportErrorLatin1(cx, "parameter 2 is not of type 'CryptoKey'");
    return RejectPromiseWithPendingError(cx, promise);
  }
  JS::RootedObject key(cx, &key_arg.toObject());
  if (!key) {
    // todo: error message
    return RejectPromiseWithPendingError(cx, promise);
  }
  if (!CryptoKey::is_instance(key)) {
    JS_ReportErrorLatin1(cx, "parameter 2 is not of type 'CryptoKey'");
    return RejectPromiseWithPendingError(cx, promise);
  }

  // 2. Let data be the result of getting a copy of the bytes held by the data parameter passed to
  // the sign() method.
  std::optional<std::span<uint8_t>> data = value_to_buffer(cx, args.get(2), "SubtleCrypto.sign: data");
  if (!data.has_value()) {
    // value_to_buffer would have already created a JS exception so we don't need to create one ourselves.
    return RejectPromiseWithPendingError(cx, promise);
  }

  // 3. Let normalizedAlgorithm be the result of normalizing an algorithm, with alg set to algorithm
  // and op set to "sign".
  auto normalizedAlgorithm = CryptoAlgorithmSignVerify::normalize(cx, algorithm);
  // 4. If an error occurred, return a Promise rejected with normalizedAlgorithm.
  if (!normalizedAlgorithm) {
    // TODO Rename error to NotSupportedError
    return RejectPromiseWithPendingError(cx, promise);
  }

  // 7. If the following steps or referenced procedures say to throw an error, reject promise with
  // the returned error and then terminate the algorithm.
  // 8. If the name member of normalizedAlgorithm is not equal to the name attribute of the
  // [[algorithm]] internal slot of key then throw an InvalidAccessError.
  auto identifier = normalizedAlgorithm->identifier();
  auto match_result = CryptoKey::is_algorithm(cx, key, identifier);
  if (match_result.isErr()) {
    JS_ReportErrorUTF8(cx, "CryptoKey doesn't match AlgorithmIdentifier");
    return RejectPromiseWithPendingError(cx, promise);
  }

  if (match_result.unwrap() == false) {
    // TODO: Change to an InvalidAccessError instance
    JS_ReportErrorUTF8(cx, "CryptoKey doesn't match AlgorithmIdentifier");
    return RejectPromiseWithPendingError(cx, promise);
  }

  // 9. If the [[usages]] internal slot of key does not contain an entry that is "sign", then throw
  // an InvalidAccessError.
  if (!CryptoKey::hasKeyUsage(key, static_cast<uint8_t>(builtins::CryptoKeyUsageSign))) {
    // TODO: Change to an InvalidAccessError instance
    JS_ReportErrorLatin1(cx, "CryptoKey doesn't support signing");
    return RejectPromiseWithPendingError(cx, promise);
  }

  JS::RootedValue result(cx);
  // 10. Let result be the result of performing the sign operation specified by normalizedAlgorithm
  // using key and algorithm and with data as message.

  {
    // 1. If the [[type]] internal slot of key is not "private", then throw an InvalidAccessError.
    if (CryptoKey::type(key) != CryptoKeyType::Private) {
      // TODO: Change to an InvalidAccessError instance
      JS_ReportErrorLatin1(cx, "InvalidAccessError");
      return RejectPromiseWithPendingError(cx, promise);
    }

    // 2. Perform the signature generation operation defined in Section 8.2 of [RFC3447] with the
    // key represented by the [[handle]] internal slot of key as the signer's private key and the
    // contents of message as M and using the hash function specified in the hash attribute of the
    // [[algorithm]] internal slot of key as the Hash option for the EMSA-PKCS1-v1_5 encoding
    // method.
    auto signature = normalizedAlgorithm->sign(cx, key, data.value());

    // 3. If performing the operation results in an error, then throw an OperationError.
    if (!signature) {
      // TODO: Change to an OperationError instance
      JS_ReportErrorLatin1(cx, "OperationError");
      return RejectPromiseWithPendingError(cx, promise);
    }
    // 4. Let signature be the value S that results from performing the operation.

    // 5. Return a new ArrayBuffer associated with the relevant global object of this [HTML], and
    // containing the bytes of signature.
    result.set(JS::ObjectValue(*signature));
  }

  // 11. Resolve promise with result.
  JS::ResolvePromise(cx, promise, result);
  return true;
}
const JSFunctionSpec SubtleCrypto::methods[] = {
    JS_FN("digest", digest, 2, JSPROP_ENUMERATE),
    JS_FN("exportKey", exportKey, 2, JSPROP_ENUMERATE),
    JS_FN("generateKey", generateKey, 3, JSPROP_ENUMERATE),
    JS_FN("importKey", importKey, 5, JSPROP_ENUMERATE),
    JS_FN("sign", sign, 3, JSPROP_ENUMERATE),
    JS_FN("verify", verify, 4, JSPROP_ENUMERATE),
    JS_FS_END};

const JSPropertySpec SubtleCrypto::properties[] = {
    JS_STRING_SYM_PS(toStringTag, "SubtleCrypto", JSPROP_READONLY), JS_PS_END};

bool SubtleCrypto::constructor(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS_ReportErrorNumberASCII(cx, GetErrorMessage, nullptr, JSMSG_ILLEGAL_CTOR);
  return false;
}

bool SubtleCrypto::init_class(JSContext *cx, JS::HandleObject global) {
  return BuiltinImpl<SubtleCrypto>::init_class_impl(cx, global);
}
} // namespace builtins
