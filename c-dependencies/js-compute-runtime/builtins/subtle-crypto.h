#pragma once

#include "builtin.h"
#include "js-compute-builtins.h"
#include "crypto-key.h"

namespace builtins {

enum class Operations {
  Encrypt,
  Decrypt,
  Sign,
  Verify,
  Digest,
  GenerateKey,
  DeriveBits,
  ImportKey,
  WrapKey,
  UnwrapKey,
  GetKeyLength
};

class SubtleCrypto : public BuiltinImpl<SubtleCrypto> {
private:
public:
  static constexpr const char *class_name = "SubtleCrypto";
  static const int ctor_length = 0;

  enum Slots { Count };
  static const JSFunctionSpec methods[];
  static const JSPropertySpec properties[];
  static bool digest(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool exportKey(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool generateKey(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool importKey(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool sign(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool verify(JSContext *cx, unsigned argc, JS::Value *vp);

  static std::unique_ptr<CryptoAlgorithm> normalizeAlgorithm(JSContext *cx, JS::HandleValue value, Operations operation);
  static JS::Result<builtins::CryptoAlgorithmIdentifier> toHashIdentifier(JSContext *cx, JS::HandleValue value);

  static bool constructor(JSContext *cx, unsigned argc, JS::Value *vp);
  static bool init_class(JSContext *cx, JS::HandleObject global);
};

} // namespace builtins
