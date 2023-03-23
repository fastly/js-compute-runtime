
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
