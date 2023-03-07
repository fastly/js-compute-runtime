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
#include "openssl/evp.h"
#include "openssl/sha.h"
// TODO: remove these once the warnings are fixed
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-offsetof"
#include "js/Conversions.h"
#include "js/experimental/TypedData.h"
#pragma clang diagnostic pop
#include <mozilla/Unused.h>

#include "host_interface/xqd.h"
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

bool renameExceptionNameToNotSupportedError(JSContext *cx, JS::HandleObject promise) {
  // We need to change the name of the error from Error to NotSupportedError
  if (JS_IsExceptionPending(cx)) {
    JS::ExceptionStack exception(cx);
    if (JS::GetPendingExceptionStack(cx, &exception)) {
      JS::HandleValue exp = exception.exception();
      JS::Rooted<JSObject *> object(cx, &exp.toObject());
      JS::Rooted<JS::Value> name_val(cx);
      if (!JS_GetProperty(cx, object, "name", &name_val)) {
        return RejectPromiseWithPendingError(cx, promise);
      }
      JS::Rooted<JSString *> str(cx, JS::ToString(cx, name_val));
      if (!str) {
        return RejectPromiseWithPendingError(cx, promise);
      }
      name_val.set(JS::StringValue(JS_NewStringCopyN(cx, "NotSupportedError", 17)));
      if (!JS_SetProperty(cx, object, "name", name_val)) {
        return RejectPromiseWithPendingError(cx, promise);
      }
    }
  }
  return true;
}
// https://w3c.github.io/webcrypto/#algorithm-normalization-normalize-an-algorithm
JS::Result<CryptoAlgorithmIdentifier>
SubtleCrypto::normalizeAlgorithm(JSContext *cx, JS::HandleValue algorithmIdentifier,
                                 Operations operation) {
  JS::Rooted<JSString *> algorithmIdentifierString(cx);

  // The algorithmIdentifier can either be a JS String or a JS Object with a 'name' property
  if (algorithmIdentifier.isString()) {
    algorithmIdentifierString.set(algorithmIdentifier.toString());
  } else if (algorithmIdentifier.isObject()) {
    JS::Rooted<JSObject *> object(cx, &algorithmIdentifier.toObject());
    JS::Rooted<JS::Value> name_val(cx);
    if (!JS_GetProperty(cx, object, "name", &name_val)) {
      return JS::Result<CryptoAlgorithmIdentifier>(JS::Error());
    }
    JS::Rooted<JSString *> str(cx, JS::ToString(cx, name_val));
    if (!str) {
      JS_ReportErrorUTF8(cx, "SubtleCrypto.digest: failed to normalize algorithm");
      return JS::Result<CryptoAlgorithmIdentifier>(JS::Error());
    }
    algorithmIdentifierString.set(str);
  } else {
    JS_ReportErrorUTF8(cx, "SubtleCrypto.digest: Operation is not supported");
    return JS::Result<CryptoAlgorithmIdentifier>(JS::Error());
  }
  size_t algorithmLen;
  JS::UniqueChars algorithmChars = encode(cx, algorithmIdentifierString, &algorithmLen);
  if (!algorithmChars) {
    return JS::Result<CryptoAlgorithmIdentifier>(JS::Error());
  }

  std::string algorithm(algorithmChars.get(), algorithmLen);
  std::transform(algorithm.begin(), algorithm.end(), algorithm.begin(),
                 [](unsigned char c) { return std::toupper(c); });

  using enum CryptoAlgorithmIdentifier;

  // 1. Return the result of running the normalize an algorithm algorithm, with the alg set to a
  // new Algorithm dictionary whose name attribute is alg, and with the op set to op.
  if (algorithm == "RSAES-PKCS1-v1_5") {
    return normalizeAlgorithm(cx, RSAES_PKCS1_v1_5, operation);
  }
  if (algorithm == "RSASSA-PKCS1-v1_5") {
    return normalizeAlgorithm(cx, RSASSA_PKCS1_v1_5, operation);
  }
  if (algorithm == "RSA-PSS") {
    return normalizeAlgorithm(cx, RSA_PSS, operation);
  }
  if (algorithm == "RSA-OAEP") {
    return normalizeAlgorithm(cx, RSA_OAEP, operation);
  }
  if (algorithm == "ECDSA") {
    return normalizeAlgorithm(cx, ECDSA, operation);
  }
  if (algorithm == "ECDH") {
    return normalizeAlgorithm(cx, ECDH, operation);
  }
  if (algorithm == "AES-CTR") {
    return normalizeAlgorithm(cx, AES_CTR, operation);
  }
  if (algorithm == "AES-CBC") {
    return normalizeAlgorithm(cx, AES_CBC, operation);
  }
  if (algorithm == "AES-GCM") {
    return normalizeAlgorithm(cx, AES_GCM, operation);
  }
  if (algorithm == "AES-CFB") {
    return normalizeAlgorithm(cx, AES_CFB, operation);
  }
  if (algorithm == "AES-KW") {
    return normalizeAlgorithm(cx, AES_KW, operation);
  }
  if (algorithm == "HMAC") {
    return normalizeAlgorithm(cx, HMAC, operation);
  }
  if (algorithm == "SHA-1") {
    return normalizeAlgorithm(cx, SHA_1, operation);
  }
  if (algorithm == "SHA-224") {
    return normalizeAlgorithm(cx, SHA_224, operation);
  }
  if (algorithm == "SHA-256") {
    return normalizeAlgorithm(cx, SHA_256, operation);
  }
  if (algorithm == "SHA-384") {
    return normalizeAlgorithm(cx, SHA_384, operation);
  }
  if (algorithm == "SHA-512") {
    return normalizeAlgorithm(cx, SHA_512, operation);
  }
  if (algorithm == "HKDF") {
    return normalizeAlgorithm(cx, HKDF, operation);
  }
  if (algorithm == "PBKDF2") {
    return normalizeAlgorithm(cx, PBKDF2, operation);
  }

  JS_ReportErrorUTF8(cx, "SubtleCrypto.digest: Operation is not supported");
  return JS::Result<CryptoAlgorithmIdentifier>(JS::Error());
}

JS::Result<CryptoAlgorithmIdentifier>
SubtleCrypto::normalizeAlgorithm(JSContext *cx, CryptoAlgorithmIdentifier algorithmIdentifier,
                                 Operations operation) {
  // 1. Let registeredAlgorithms be the associative container stored at the op key of
  // supportedAlgorithms.
  std::vector<CryptoAlgorithmIdentifier> registeredAlgorithms;
  switch (operation) {
    using enum Operations;
  case Encrypt: {
    registeredAlgorithms = supportedEncryptAlgorithms;
    break;
  }
  case Decrypt: {
    registeredAlgorithms = supportedDecryptAlgorithms;
    break;
  }
  case Sign: {
    registeredAlgorithms = supportedSignAlgorithms;
    break;
  }
  case Verify: {
    registeredAlgorithms = supportedVerifyAlgorithms;
    break;
  }
  case Digest: {
    registeredAlgorithms = supportedDigestAlgorithms;
    break;
  }
  case GenerateKey: {
    registeredAlgorithms = supportedGenerateKeyAlgorithms;
    break;
  }
  case DeriveBits: {
    registeredAlgorithms = supportedDeriveBitsAlgorithms;
    break;
  }
  case ImportKey: {
    registeredAlgorithms = supportedImportKeyAlgorithms;
    break;
  }
  case WrapKey: {
    registeredAlgorithms = supportedWrapKeyAlgorithms;
    break;
  }
  case UnwrapKey: {
    registeredAlgorithms = supportedUnwrapKeyAlgorithms;
    break;
  }
  case GetKeyLength: {
    registeredAlgorithms = supportedGetKeyLengthAlgorithms;
    break;
  }
  default: {
    MOZ_ASSERT_UNREACHABLE("Unknown `Operations` value");
  }
  }

  // 2. Let initialAlg be the result of converting the ECMAScript object represented by alg to the
  // IDL dictionary type Algorithm, as defined by [WebIDL].
  auto initialAlg = algorithmIdentifier;
  // 3. If an error occurred, return the error and terminate this algorithm.
  // 4. Let algName be the value of the name attribute of initialAlg.
  // auto algName = algorithmName(initialAlg);
  // 5. If registeredAlgorithms contains a key that is a case-insensitive string match for algName:
  // 1. Set algName to the value of the matching key.
  // 2. Let desiredType be the IDL dictionary type stored at algName in registeredAlgorithms.
  // Otherwise:
  // Return a new NotSupportedError and terminate this algorithm.
  auto result = std::find(registeredAlgorithms.begin(), registeredAlgorithms.end(), initialAlg);
  if (result == registeredAlgorithms.end()) {
    JS_ReportErrorUTF8(cx, "SubtleCrypto.digest: Operation is not supported");
    return JS::Result<CryptoAlgorithmIdentifier>(JS::Error());
  }
  return initialAlg;
}

// digest(algorithm, data)
// https://w3c.github.io/webcrypto/#SubtleCrypto-method-digest
bool SubtleCrypto::digest(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS::CallArgs args = CallArgsFromVp(argc, vp);
  if (!args.requireAtLeast(cx, "SubtleCrypto.digest", 2)) {
    return false;
  }
  // 1. Let algorithm be the algorithm parameter passed to the digest() method.
  auto algorithm = args.get(0);

  // 2 . Let data be the result of getting a copy of the bytes held by the data parameter
  // passed to the digest() method.
  auto data = value_to_buffer(cx, args.get(1), "SubtleCrypto#digest: data");
  if (!data.has_value()) {
    return false;
  }

  // 5. Let promise be a new Promise.
  JS::RootedObject promise(cx, JS::NewPromiseObject(cx, nullptr));
  if (!promise) {
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }

  // 6. Return promise and perform the remaining steps in parallel.
  args.rval().setObject(*promise);

  // 3. Let normalizedAlgorithm be the result of normalizing an algorithm, with alg set to
  // algorithm and op set to "digest".
  // 4. If an error occurred, return a Promise rejected with normalizedAlgorithm.
  auto normalizedAlgorithmResult = normalizeAlgorithm(cx, algorithm, Operations::Digest);
  if (normalizedAlgorithmResult.isErr()) {
    if (!renameExceptionNameToNotSupportedError(cx, promise)) {
      return RejectPromiseWithPendingError(cx, promise);
    }
    return RejectPromiseWithPendingError(cx, promise);
  }
  auto normalizedAlgorithm = normalizedAlgorithmResult.unwrap();

  // 7. If the following steps or referenced procedures say to throw an error, reject promise with
  // the returned error and then terminate the algorithm.
  // 8. Let result be the result of performing the digest operation specified by normalizedAlgorithm
  // using algorithm, with data as message.

  size_t digest_length = 0;
  const EVP_MD *alg;
  switch (normalizedAlgorithm) {
    using enum CryptoAlgorithmIdentifier;
  case SHA_1: {
    alg = EVP_sha1();
    digest_length = SHA_DIGEST_LENGTH;
    break;
  }
  case SHA_224: {
    alg = EVP_sha224();
    digest_length = SHA224_DIGEST_LENGTH;
    break;
  }
  case SHA_256: {
    alg = EVP_sha256();
    digest_length = SHA256_DIGEST_LENGTH;
    break;
  }
  case SHA_384: {
    alg = EVP_sha384();
    digest_length = SHA384_DIGEST_LENGTH;
    break;
  }
  case SHA_512: {
    alg = EVP_sha512();
    digest_length = SHA512_DIGEST_LENGTH;
    break;
  }
  default: {
    JS_ReportErrorUTF8(cx, "SubtleCrypto.digest: Operation is not supported");
    if (!renameExceptionNameToNotSupportedError(cx, promise)) {
      return RejectPromiseWithPendingError(cx, promise);
    }
    return RejectPromiseWithPendingError(cx, promise);
  }
  }

  unsigned int size;
  auto buf = static_cast<unsigned char *>(JS_malloc(cx, digest_length));
  if (!buf) {
    JS_ReportOutOfMemory(cx);
    return false;
  }
  if (!EVP_Digest(data->data(), data->size(), buf, &size, alg, NULL)) {
    JS_ReportErrorUTF8(cx, "SubtleCrypto.digest: failed to create digest");
    return RejectPromiseWithPendingError(cx, promise);
  }
  JS::RootedObject array_buffer(cx);
  array_buffer.set(JS::NewArrayBufferWithContents(cx, size, buf));
  if (!array_buffer) {
    return RejectPromiseWithPendingError(cx, promise);
  }

  // 9. Resolve promise with result.
  JS::RootedValue result(cx);
  result.setObject(*array_buffer);
  JS::ResolvePromise(cx, promise, result);

  return true;
}

const JSFunctionSpec SubtleCrypto::methods[] = {JS_FN("digest", digest, 2, JSPROP_ENUMERATE),
                                                JS_FS_END};

const JSPropertySpec SubtleCrypto::properties[] = {
    JS_STRING_SYM_PS(toStringTag, "SubtleCrypto", JSPROP_READONLY), JS_PS_END};

bool SubtleCrypto::constructor(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS_ReportErrorLatin1(cx, "Illegal constructor SubtleCrypto");
  return false;
}

bool SubtleCrypto::init_class(JSContext *cx, JS::HandleObject global) {
  return BuiltinImpl<SubtleCrypto>::init_class_impl(cx, global);
}
} // namespace builtins
