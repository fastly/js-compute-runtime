#include "openssl/sha.h"
#include <iostream>
#include <span>

#include "crypto-algorithm.h"

namespace builtins {

namespace {

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