
#include "subtle-crypto.h"
#include "js-compute-builtins.h"

namespace builtins {
// digest(algorithm, data)
// https://w3c.github.io/webcrypto/#SubtleCrypto-method-digest
bool SubtleCrypto::digest(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS::CallArgs args = CallArgsFromVp(argc, vp);
  if (!check_receiver(cx, args.thisv(), "SubtleCrypto.digest")) {
    return ReturnPromiseRejectedWithPendingError(cx, args);
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
    // value_to_buffer would have already created a JS exception so we don't need to create one
    // ourselves.
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }

  // 3. Let normalizedAlgorithm be the result of normalizing an algorithm, with alg set to
  // algorithm and op set to "digest".
  // 4. If an error occurred, return a Promise rejected with normalizedAlgorithm.
  auto normalizedAlgorithm = CryptoAlgorithmDigest::normalize(cx, algorithm);
  if (!normalizedAlgorithm) {
    // TODO Rename error to NotSupportedError
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }

  // 5. Let promise be a new Promise.
  JS::RootedObject promise(cx, JS::NewPromiseObject(cx, nullptr));
  if (!promise) {
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }

  // 6. Return promise and perform the remaining steps in parallel.
  args.rval().setObject(*promise);

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

//  Promise<CryptoKey> importKey(KeyFormat format,
//                         (BufferSource or JsonWebKey) keyData,
//                         AlgorithmIdentifier algorithm,
//                         boolean extractable,
//                         sequence<KeyUsage> keyUsages );
// https://w3c.github.io/webcrypto/#dfn-SubtleCrypto-method-importKey
bool SubtleCrypto::importKey(JSContext *cx, unsigned argc, JS::Value *vp) {
  MOZ_ASSERT(cx);
  JS::CallArgs args = CallArgsFromVp(argc, vp);

  if (!args.requireAtLeast(cx, "SubtleCrypto.importKey", 5)) {
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }
  if (!check_receiver(cx, args.thisv(), "SubtleCrypto.importKey")) {
    return ReturnPromiseRejectedWithPendingError(cx, args);
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
      return ReturnPromiseRejectedWithPendingError(cx, args);
    }
    std::string_view format_string(format_chars.get(), format_length);
    if (format_string == "spki") {
      format = CryptoKeyFormat::Spki;
    } else if (format_string == "pkcs8") {
      format = CryptoKeyFormat::Pkcs8;
    } else if (format_string == "jwk") {
      format = CryptoKeyFormat::Jwk;
    } else if (format_string == "raw") {
      format = CryptoKeyFormat::Raw;
    } else {
      // TODO: Change to a SyntaxError instance
      JS_ReportErrorLatin1(cx, "Provided format parameter is not supported. Supported formats are: "
                               "'spki', 'pkcs8', 'jwk', and 'raw'");
      return ReturnPromiseRejectedWithPendingError(cx, args);
    }
  }
  auto key_data = args.get(1);
  auto algorithm = args.get(2);
  bool extractable = JS::ToBoolean(args.get(3));
  CryptoKeyUsages usages;
  {
    auto usages_arg = args.get(4);

    std::string_view error_message("SubtleCrypto.importKey: Invalid keyUsages argument");
    auto keyUsageMaskResult = builtins::CryptoKeyUsages::from(cx, usages_arg, error_message);
    if (keyUsageMaskResult.isErr()) {
      return ReturnPromiseRejectedWithPendingError(cx, args);
    }
    usages = keyUsageMaskResult.unwrap();
  }

  // 3. Let normalizedAlgorithm be the result of normalizing an algorithm, with alg set to algorithm
  // and op set to "importKey".
  // 4. If an error occurred, return a Promise rejected with normalizedAlgorithm.
  auto normalizedAlgorithm = CryptoAlgorithmImportKey::normalize(cx, algorithm);
  if (!normalizedAlgorithm) {
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }

  // 5. Let promise be a new Promise.
  JS::RootedObject promise(cx, JS::NewPromiseObject(cx, nullptr));
  if (!promise) {
    JS_ReportErrorASCII(cx, "InternalError");
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }

  // 6. Return promise and perform the remaining steps in parallel.
  args.rval().setObject(*promise);

  // 7. If the following steps or referenced procedures say to throw an error,
  // reject promise with the returned error and then terminate the algorithm.

  // Steps 8 - 11 are all done in the `importKey` call
  // 8. Let result be the CryptoKey object that results from performing the
  // import key operation specified by normalizedAlgorithm using keyData,
  // algorithm, format, extractable and usages.
  // 9. If the [[type]] internal slot of result is "secret" or "private" and
  // usages is empty, then throw a SyntaxError.
  // 10. Set the [[extractable]] internal slot of result to extractable.
  // 11. Set the [[usages]] internal slot of result to the normalized value of
  // usages.
  JS::RootedObject result(cx);
  JSObject *key = normalizedAlgorithm->importKey(cx, format, key_data, extractable, usages);
  if (!key) {
    return RejectPromiseWithPendingError(cx, promise);
  }

  // 12. Resolve promise with result.
  JS::RootedValue result_val(cx);
  result_val.setObject(*key);
  JS::ResolvePromise(cx, promise, result_val);

  return true;
}

// https://w3c.github.io/webcrypto/#SubtleCrypto-method-sign
bool SubtleCrypto::sign(JSContext *cx, unsigned argc, JS::Value *vp) {
  MOZ_ASSERT(cx);
  MOZ_ASSERT(vp);
  JS::CallArgs args = CallArgsFromVp(argc, vp);

  if (!args.requireAtLeast(cx, "SubtleCrypto.sign", 3)) {
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }
  if (!check_receiver(cx, args.thisv(), "SubtleCrypto.sign")) {
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }

  // 1. Let algorithm and key be the algorithm and key parameters passed to the sign() method,
  // respectively.
  auto algorithm = args.get(0);
  JS::RootedObject key(cx);
  {
    auto key_arg = args.get(1);
    if (!key_arg.isObject()) {
      JS_ReportErrorLatin1(cx, "parameter 2 is not of type 'CryptoKey'");
      return ReturnPromiseRejectedWithPendingError(cx, args);
    }
    key.set(&key_arg.toObject());
    if (!CryptoKey::is_instance(key)) {
      JS_ReportErrorLatin1(cx, "parameter 2 is not of type 'CryptoKey'");
      return ReturnPromiseRejectedWithPendingError(cx, args);
    }
  }

  // 2. Let data be the result of getting a copy of the bytes held by the data parameter passed to
  // the sign() method.
  std::span<uint8_t> data;
  {
    std::optional<std::span<uint8_t>> dataOptional =
        value_to_buffer(cx, args.get(2), "SubtleCrypto.sign: data");
    if (!dataOptional.has_value()) {
      // value_to_buffer would have already created a JS exception so we don't need to create one
      // ourselves.
      return ReturnPromiseRejectedWithPendingError(cx, args);
    }
    data = dataOptional.value();
  }

  // 3. Let normalizedAlgorithm be the result of normalizing an algorithm, with alg set to algorithm
  // and op set to "sign".
  auto normalizedAlgorithm = CryptoAlgorithmSignVerify::normalize(cx, algorithm);
  // 4. If an error occurred, return a Promise rejected with normalizedAlgorithm.
  if (!normalizedAlgorithm) {
    // TODO Rename error to NotSupportedError
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }

  // 5. Let promise be a new Promise.
  JS::RootedObject promise(cx, JS::NewPromiseObject(cx, nullptr));
  if (!promise) {
    return ReturnPromiseRejectedWithPendingError(cx, args);
  }
  // 6. Return promise and perform the remaining steps in parallel.
  args.rval().setObject(*promise);

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
  if (!CryptoKey::canSign(key)) {
    // TODO: Change to an InvalidAccessError instance
    JS_ReportErrorLatin1(cx, "CryptoKey doesn't support signing");
    return RejectPromiseWithPendingError(cx, promise);
  }

  // 10. Let result be the result of performing the sign operation specified by normalizedAlgorithm
  // using key and algorithm and with data as message.
  auto signature = normalizedAlgorithm->sign(cx, key, data);
  if (!signature) {
    return RejectPromiseWithPendingError(cx, promise);
  }
  JS::RootedValue result(cx, JS::ObjectValue(*signature));

  // 11. Resolve promise with result.
  JS::ResolvePromise(cx, promise, result);
  return true;
}

const JSFunctionSpec SubtleCrypto::methods[] = {JS_FN("digest", digest, 2, JSPROP_ENUMERATE),
                                                JS_FN("importKey", importKey, 5, JSPROP_ENUMERATE),
                                                JS_FN("sign", sign, 3, JSPROP_ENUMERATE),
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
