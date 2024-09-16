var __defProp = Object.defineProperty;
var __export = (target, all) => {
  for (var name in all)
    __defProp(target, name, { get: all[name], enumerable: true });
};

// ../../../node_modules/fast-check/lib/esm/fast-check-default.js
var fast_check_default_exports = {};
__export(fast_check_default_exports, {
  Arbitrary: () => Arbitrary,
  ExecutionStatus: () => ExecutionStatus,
  PreconditionFailure: () => PreconditionFailure,
  Random: () => Random,
  Stream: () => Stream,
  Value: () => Value,
  VerbosityLevel: () => VerbosityLevel,
  __commitHash: () => __commitHash2,
  __type: () => __type2,
  __version: () => __version2,
  anything: () => anything,
  array: () => array,
  ascii: () => ascii,
  asciiString: () => asciiString,
  assert: () => assert,
  asyncDefaultReportMessage: () => asyncDefaultReportMessage,
  asyncModelRun: () => asyncModelRun,
  asyncProperty: () => asyncProperty,
  asyncStringify: () => asyncStringify,
  asyncToStringMethod: () => asyncToStringMethod,
  base64: () => base64,
  base64String: () => base64String,
  bigInt: () => bigInt,
  bigInt64Array: () => bigInt64Array,
  bigIntN: () => bigIntN,
  bigUint: () => bigUint,
  bigUint64Array: () => bigUint64Array,
  bigUintN: () => bigUintN,
  boolean: () => boolean,
  char: () => char,
  char16bits: () => char16bits,
  check: () => check,
  clone: () => clone,
  cloneIfNeeded: () => cloneIfNeeded,
  cloneMethod: () => cloneMethod,
  commands: () => commands,
  compareBooleanFunc: () => compareBooleanFunc,
  compareFunc: () => compareFunc,
  configureGlobal: () => configureGlobal,
  constant: () => constant,
  constantFrom: () => constantFrom,
  context: () => context,
  createDepthIdentifier: () => createDepthIdentifier,
  date: () => date,
  defaultReportMessage: () => defaultReportMessage,
  dictionary: () => dictionary,
  domain: () => domain,
  double: () => double,
  emailAddress: () => emailAddress,
  falsy: () => falsy,
  float: () => float,
  float32Array: () => float32Array,
  float64Array: () => float64Array,
  fullUnicode: () => fullUnicode,
  fullUnicodeString: () => fullUnicodeString,
  func: () => func,
  gen: () => gen,
  getDepthContextFor: () => getDepthContextFor,
  hasAsyncToStringMethod: () => hasAsyncToStringMethod,
  hasCloneMethod: () => hasCloneMethod,
  hasToStringMethod: () => hasToStringMethod,
  hash: () => hash,
  hexa: () => hexa,
  hexaString: () => hexaString,
  infiniteStream: () => infiniteStream,
  int16Array: () => int16Array,
  int32Array: () => int32Array,
  int8Array: () => int8Array,
  integer: () => integer,
  ipV4: () => ipV4,
  ipV4Extended: () => ipV4Extended,
  ipV6: () => ipV6,
  json: () => json,
  jsonValue: () => jsonValue,
  letrec: () => letrec,
  lorem: () => lorem,
  mapToConstant: () => mapToConstant,
  maxSafeInteger: () => maxSafeInteger,
  maxSafeNat: () => maxSafeNat,
  memo: () => memo,
  mixedCase: () => mixedCase,
  modelRun: () => modelRun,
  nat: () => nat,
  object: () => object,
  oneof: () => oneof,
  option: () => option,
  pre: () => pre,
  property: () => property,
  readConfigureGlobal: () => readConfigureGlobal,
  record: () => record,
  resetConfigureGlobal: () => resetConfigureGlobal,
  sample: () => sample,
  scheduledModelRun: () => scheduledModelRun,
  scheduler: () => scheduler,
  schedulerFor: () => schedulerFor,
  shuffledSubarray: () => shuffledSubarray,
  sparseArray: () => sparseArray,
  statistics: () => statistics,
  stream: () => stream,
  string: () => string,
  string16bits: () => string16bits,
  stringMatching: () => stringMatching,
  stringOf: () => stringOf,
  stringify: () => stringify,
  subarray: () => subarray,
  toStringMethod: () => toStringMethod,
  tuple: () => tuple,
  uint16Array: () => uint16Array,
  uint32Array: () => uint32Array,
  uint8Array: () => uint8Array,
  uint8ClampedArray: () => uint8ClampedArray,
  ulid: () => ulid,
  unicode: () => unicode,
  unicodeJson: () => unicodeJson,
  unicodeJsonValue: () => unicodeJsonValue,
  unicodeString: () => unicodeString,
  uniqueArray: () => uniqueArray,
  uuid: () => uuid,
  uuidV: () => uuidV,
  webAuthority: () => webAuthority,
  webFragments: () => webFragments,
  webPath: () => webPath,
  webQueryParameters: () => webQueryParameters,
  webSegment: () => webSegment,
  webUrl: () => webUrl,
});

// ../../../node_modules/fast-check/lib/esm/check/precondition/PreconditionFailure.js
var PreconditionFailure = class _PreconditionFailure extends Error {
  constructor(interruptExecution = false) {
    super();
    this.interruptExecution = interruptExecution;
    this.footprint = _PreconditionFailure.SharedFootPrint;
  }
  static isFailure(err) {
    return (
      err != null && err.footprint === _PreconditionFailure.SharedFootPrint
    );
  }
};
PreconditionFailure.SharedFootPrint = Symbol('fast-check/PreconditionFailure');

// ../../../node_modules/fast-check/lib/esm/check/precondition/Pre.js
function pre(expectTruthy) {
  if (!expectTruthy) {
    throw new PreconditionFailure();
  }
}

// ../../../node_modules/fast-check/lib/esm/stream/StreamHelpers.js
var Nil = class {
  [Symbol.iterator]() {
    return this;
  }
  next(value) {
    return { value, done: true };
  }
};
Nil.nil = new Nil();
function nilHelper() {
  return Nil.nil;
}
function* mapHelper(g, f) {
  for (const v of g) {
    yield f(v);
  }
}
function* flatMapHelper(g, f) {
  for (const v of g) {
    yield* f(v);
  }
}
function* filterHelper(g, f) {
  for (const v of g) {
    if (f(v)) {
      yield v;
    }
  }
}
function* takeNHelper(g, n) {
  for (let i = 0; i < n; ++i) {
    const cur = g.next();
    if (cur.done) {
      break;
    }
    yield cur.value;
  }
}
function* takeWhileHelper(g, f) {
  let cur = g.next();
  while (!cur.done && f(cur.value)) {
    yield cur.value;
    cur = g.next();
  }
}
function* joinHelper(g, others) {
  for (let cur = g.next(); !cur.done; cur = g.next()) {
    yield cur.value;
  }
  for (const s of others) {
    for (let cur = s.next(); !cur.done; cur = s.next()) {
      yield cur.value;
    }
  }
}

// ../../../node_modules/fast-check/lib/esm/stream/Stream.js
var safeSymbolIterator = Symbol.iterator;
var Stream = class _Stream {
  static nil() {
    return new _Stream(nilHelper());
  }
  static of(...elements) {
    return new _Stream(elements[safeSymbolIterator]());
  }
  constructor(g) {
    this.g = g;
  }
  next() {
    return this.g.next();
  }
  [safeSymbolIterator]() {
    return this.g;
  }
  map(f) {
    return new _Stream(mapHelper(this.g, f));
  }
  flatMap(f) {
    return new _Stream(flatMapHelper(this.g, f));
  }
  dropWhile(f) {
    let foundEligible = false;
    function* helper(v) {
      if (foundEligible || !f(v)) {
        foundEligible = true;
        yield v;
      }
    }
    return this.flatMap(helper);
  }
  drop(n) {
    if (n <= 0) {
      return this;
    }
    let idx = 0;
    function helper() {
      return idx++ < n;
    }
    return this.dropWhile(helper);
  }
  takeWhile(f) {
    return new _Stream(takeWhileHelper(this.g, f));
  }
  take(n) {
    return new _Stream(takeNHelper(this.g, n));
  }
  filter(f) {
    return new _Stream(filterHelper(this.g, f));
  }
  every(f) {
    for (const v of this.g) {
      if (!f(v)) {
        return false;
      }
    }
    return true;
  }
  has(f) {
    for (const v of this.g) {
      if (f(v)) {
        return [true, v];
      }
    }
    return [false, null];
  }
  join(...others) {
    return new _Stream(joinHelper(this.g, others));
  }
  getNthOrLast(nth) {
    let remaining = nth;
    let last = null;
    for (const v of this.g) {
      if (remaining-- === 0) return v;
      last = v;
    }
    return last;
  }
};
function stream(g) {
  return new Stream(g);
}

// ../../../node_modules/fast-check/lib/esm/check/symbols.js
var cloneMethod = Symbol('fast-check/cloneMethod');
function hasCloneMethod(instance) {
  return (
    instance !== null &&
    (typeof instance === 'object' || typeof instance === 'function') &&
    cloneMethod in instance &&
    typeof instance[cloneMethod] === 'function'
  );
}
function cloneIfNeeded(instance) {
  return hasCloneMethod(instance) ? instance[cloneMethod]() : instance;
}

// ../../../node_modules/fast-check/lib/esm/check/arbitrary/definition/Value.js
var safeObjectDefineProperty = Object.defineProperty;
var Value = class {
  constructor(value_, context2, customGetValue = void 0) {
    this.value_ = value_;
    this.context = context2;
    this.hasToBeCloned = customGetValue !== void 0 || hasCloneMethod(value_);
    this.readOnce = false;
    if (this.hasToBeCloned) {
      safeObjectDefineProperty(this, 'value', {
        get: customGetValue !== void 0 ? customGetValue : this.getValue,
      });
    } else {
      this.value = value_;
    }
  }
  getValue() {
    if (this.hasToBeCloned) {
      if (!this.readOnce) {
        this.readOnce = true;
        return this.value_;
      }
      return this.value_[cloneMethod]();
    }
    return this.value_;
  }
};

// ../../../node_modules/fast-check/lib/esm/check/arbitrary/definition/Arbitrary.js
var safeObjectAssign = Object.assign;
var Arbitrary = class {
  filter(refinement) {
    return new FilterArbitrary(this, refinement);
  }
  map(mapper, unmapper) {
    return new MapArbitrary(this, mapper, unmapper);
  }
  chain(chainer) {
    return new ChainArbitrary(this, chainer);
  }
  noShrink() {
    return new NoShrinkArbitrary(this);
  }
  noBias() {
    return new NoBiasArbitrary(this);
  }
};
var ChainArbitrary = class extends Arbitrary {
  constructor(arb, chainer) {
    super();
    this.arb = arb;
    this.chainer = chainer;
  }
  generate(mrng, biasFactor) {
    const clonedMrng = mrng.clone();
    const src = this.arb.generate(mrng, biasFactor);
    return this.valueChainer(src, mrng, clonedMrng, biasFactor);
  }
  canShrinkWithoutContext(value) {
    return false;
  }
  shrink(value, context2) {
    if (this.isSafeContext(context2)) {
      return (
        !context2.stoppedForOriginal
          ? this.arb
              .shrink(context2.originalValue, context2.originalContext)
              .map((v) =>
                this.valueChainer(
                  v,
                  context2.clonedMrng.clone(),
                  context2.clonedMrng,
                  context2.originalBias,
                ),
              )
          : Stream.nil()
      ).join(
        context2.chainedArbitrary
          .shrink(value, context2.chainedContext)
          .map((dst) => {
            const newContext = safeObjectAssign(
              safeObjectAssign({}, context2),
              {
                chainedContext: dst.context,
                stoppedForOriginal: true,
              },
            );
            return new Value(dst.value_, newContext);
          }),
      );
    }
    return Stream.nil();
  }
  valueChainer(v, generateMrng, clonedMrng, biasFactor) {
    const chainedArbitrary = this.chainer(v.value_);
    const dst = chainedArbitrary.generate(generateMrng, biasFactor);
    const context2 = {
      originalBias: biasFactor,
      originalValue: v.value_,
      originalContext: v.context,
      stoppedForOriginal: false,
      chainedArbitrary,
      chainedContext: dst.context,
      clonedMrng,
    };
    return new Value(dst.value_, context2);
  }
  isSafeContext(context2) {
    return (
      context2 != null &&
      typeof context2 === 'object' &&
      'originalBias' in context2 &&
      'originalValue' in context2 &&
      'originalContext' in context2 &&
      'stoppedForOriginal' in context2 &&
      'chainedArbitrary' in context2 &&
      'chainedContext' in context2 &&
      'clonedMrng' in context2
    );
  }
};
var MapArbitrary = class extends Arbitrary {
  constructor(arb, mapper, unmapper) {
    super();
    this.arb = arb;
    this.mapper = mapper;
    this.unmapper = unmapper;
    this.bindValueMapper = (v) => this.valueMapper(v);
  }
  generate(mrng, biasFactor) {
    const g = this.arb.generate(mrng, biasFactor);
    return this.valueMapper(g);
  }
  canShrinkWithoutContext(value) {
    if (this.unmapper !== void 0) {
      try {
        const unmapped = this.unmapper(value);
        return this.arb.canShrinkWithoutContext(unmapped);
      } catch (_err) {
        return false;
      }
    }
    return false;
  }
  shrink(value, context2) {
    if (this.isSafeContext(context2)) {
      return this.arb
        .shrink(context2.originalValue, context2.originalContext)
        .map(this.bindValueMapper);
    }
    if (this.unmapper !== void 0) {
      const unmapped = this.unmapper(value);
      return this.arb.shrink(unmapped, void 0).map(this.bindValueMapper);
    }
    return Stream.nil();
  }
  mapperWithCloneIfNeeded(v) {
    const sourceValue = v.value;
    const mappedValue = this.mapper(sourceValue);
    if (
      v.hasToBeCloned &&
      ((typeof mappedValue === 'object' && mappedValue !== null) ||
        typeof mappedValue === 'function') &&
      Object.isExtensible(mappedValue) &&
      !hasCloneMethod(mappedValue)
    ) {
      Object.defineProperty(mappedValue, cloneMethod, {
        get: () => () => this.mapperWithCloneIfNeeded(v)[0],
      });
    }
    return [mappedValue, sourceValue];
  }
  valueMapper(v) {
    const [mappedValue, sourceValue] = this.mapperWithCloneIfNeeded(v);
    const context2 = { originalValue: sourceValue, originalContext: v.context };
    return new Value(mappedValue, context2);
  }
  isSafeContext(context2) {
    return (
      context2 != null &&
      typeof context2 === 'object' &&
      'originalValue' in context2 &&
      'originalContext' in context2
    );
  }
};
var FilterArbitrary = class extends Arbitrary {
  constructor(arb, refinement) {
    super();
    this.arb = arb;
    this.refinement = refinement;
    this.bindRefinementOnValue = (v) => this.refinementOnValue(v);
  }
  generate(mrng, biasFactor) {
    while (true) {
      const g = this.arb.generate(mrng, biasFactor);
      if (this.refinementOnValue(g)) {
        return g;
      }
    }
  }
  canShrinkWithoutContext(value) {
    return this.arb.canShrinkWithoutContext(value) && this.refinement(value);
  }
  shrink(value, context2) {
    return this.arb.shrink(value, context2).filter(this.bindRefinementOnValue);
  }
  refinementOnValue(v) {
    return this.refinement(v.value);
  }
};
var NoShrinkArbitrary = class extends Arbitrary {
  constructor(arb) {
    super();
    this.arb = arb;
  }
  generate(mrng, biasFactor) {
    return this.arb.generate(mrng, biasFactor);
  }
  canShrinkWithoutContext(value) {
    return this.arb.canShrinkWithoutContext(value);
  }
  shrink(_value, _context) {
    return Stream.nil();
  }
  noShrink() {
    return this;
  }
};
var NoBiasArbitrary = class extends Arbitrary {
  constructor(arb) {
    super();
    this.arb = arb;
  }
  generate(mrng, _biasFactor) {
    return this.arb.generate(mrng, void 0);
  }
  canShrinkWithoutContext(value) {
    return this.arb.canShrinkWithoutContext(value);
  }
  shrink(value, context2) {
    return this.arb.shrink(value, context2);
  }
  noBias() {
    return this;
  }
};
function isArbitrary(instance) {
  return (
    typeof instance === 'object' &&
    instance !== null &&
    'generate' in instance &&
    'shrink' in instance &&
    'canShrinkWithoutContext' in instance
  );
}
function assertIsArbitrary(instance) {
  if (!isArbitrary(instance)) {
    throw new Error('Unexpected value received: not an instance of Arbitrary');
  }
}

// ../../../node_modules/fast-check/lib/esm/utils/apply.js
var untouchedApply = Function.prototype.apply;
var ApplySymbol = Symbol('apply');
function safeExtractApply(f) {
  try {
    return f.apply;
  } catch (err) {
    return void 0;
  }
}
function safeApplyHacky(f, instance, args) {
  const ff = f;
  ff[ApplySymbol] = untouchedApply;
  const out = ff[ApplySymbol](instance, args);
  delete ff[ApplySymbol];
  return out;
}
function safeApply(f, instance, args) {
  if (safeExtractApply(f) === untouchedApply) {
    return f.apply(instance, args);
  }
  return safeApplyHacky(f, instance, args);
}

// ../../../node_modules/fast-check/lib/esm/utils/globals.js
var SArray = typeof Array !== 'undefined' ? Array : void 0;
var SBigInt = typeof BigInt !== 'undefined' ? BigInt : void 0;
var SBigInt64Array =
  typeof BigInt64Array !== 'undefined' ? BigInt64Array : void 0;
var SBigUint64Array =
  typeof BigUint64Array !== 'undefined' ? BigUint64Array : void 0;
var SBoolean = typeof Boolean !== 'undefined' ? Boolean : void 0;
var SDate = typeof Date !== 'undefined' ? Date : void 0;
var SError = typeof Error !== 'undefined' ? Error : void 0;
var SFloat32Array = typeof Float32Array !== 'undefined' ? Float32Array : void 0;
var SFloat64Array = typeof Float64Array !== 'undefined' ? Float64Array : void 0;
var SInt8Array = typeof Int8Array !== 'undefined' ? Int8Array : void 0;
var SInt16Array = typeof Int16Array !== 'undefined' ? Int16Array : void 0;
var SInt32Array = typeof Int32Array !== 'undefined' ? Int32Array : void 0;
var SNumber = typeof Number !== 'undefined' ? Number : void 0;
var SString = typeof String !== 'undefined' ? String : void 0;
var SSet = typeof Set !== 'undefined' ? Set : void 0;
var SUint8Array = typeof Uint8Array !== 'undefined' ? Uint8Array : void 0;
var SUint8ClampedArray =
  typeof Uint8ClampedArray !== 'undefined' ? Uint8ClampedArray : void 0;
var SUint16Array = typeof Uint16Array !== 'undefined' ? Uint16Array : void 0;
var SUint32Array = typeof Uint32Array !== 'undefined' ? Uint32Array : void 0;
var SencodeURIComponent =
  typeof encodeURIComponent !== 'undefined' ? encodeURIComponent : void 0;
var untouchedForEach = Array.prototype.forEach;
var untouchedIndexOf = Array.prototype.indexOf;
var untouchedJoin = Array.prototype.join;
var untouchedMap = Array.prototype.map;
var untouchedFilter = Array.prototype.filter;
var untouchedPush = Array.prototype.push;
var untouchedPop = Array.prototype.pop;
var untouchedSplice = Array.prototype.splice;
var untouchedSlice = Array.prototype.slice;
var untouchedSort = Array.prototype.sort;
var untouchedEvery = Array.prototype.every;
function extractForEach(instance) {
  try {
    return instance.forEach;
  } catch (err) {
    return void 0;
  }
}
function extractIndexOf(instance) {
  try {
    return instance.indexOf;
  } catch (err) {
    return void 0;
  }
}
function extractJoin(instance) {
  try {
    return instance.join;
  } catch (err) {
    return void 0;
  }
}
function extractMap(instance) {
  try {
    return instance.map;
  } catch (err) {
    return void 0;
  }
}
function extractFilter(instance) {
  try {
    return instance.filter;
  } catch (err) {
    return void 0;
  }
}
function extractPush(instance) {
  try {
    return instance.push;
  } catch (err) {
    return void 0;
  }
}
function extractPop(instance) {
  try {
    return instance.pop;
  } catch (err) {
    return void 0;
  }
}
function extractSplice(instance) {
  try {
    return instance.splice;
  } catch (err) {
    return void 0;
  }
}
function extractSlice(instance) {
  try {
    return instance.slice;
  } catch (err) {
    return void 0;
  }
}
function extractSort(instance) {
  try {
    return instance.sort;
  } catch (err) {
    return void 0;
  }
}
function extractEvery(instance) {
  try {
    return instance.every;
  } catch (err) {
    return void 0;
  }
}
function safeForEach(instance, fn) {
  if (extractForEach(instance) === untouchedForEach) {
    return instance.forEach(fn);
  }
  return safeApply(untouchedForEach, instance, [fn]);
}
function safeIndexOf(instance, ...args) {
  if (extractIndexOf(instance) === untouchedIndexOf) {
    return instance.indexOf(...args);
  }
  return safeApply(untouchedIndexOf, instance, args);
}
function safeJoin(instance, ...args) {
  if (extractJoin(instance) === untouchedJoin) {
    return instance.join(...args);
  }
  return safeApply(untouchedJoin, instance, args);
}
function safeMap(instance, fn) {
  if (extractMap(instance) === untouchedMap) {
    return instance.map(fn);
  }
  return safeApply(untouchedMap, instance, [fn]);
}
function safeFilter(instance, predicate) {
  if (extractFilter(instance) === untouchedFilter) {
    return instance.filter(predicate);
  }
  return safeApply(untouchedFilter, instance, [predicate]);
}
function safePush(instance, ...args) {
  if (extractPush(instance) === untouchedPush) {
    return instance.push(...args);
  }
  return safeApply(untouchedPush, instance, args);
}
function safePop(instance) {
  if (extractPop(instance) === untouchedPop) {
    return instance.pop();
  }
  return safeApply(untouchedPop, instance, []);
}
function safeSplice(instance, ...args) {
  if (extractSplice(instance) === untouchedSplice) {
    return instance.splice(...args);
  }
  return safeApply(untouchedSplice, instance, args);
}
function safeSlice(instance, ...args) {
  if (extractSlice(instance) === untouchedSlice) {
    return instance.slice(...args);
  }
  return safeApply(untouchedSlice, instance, args);
}
function safeSort(instance, ...args) {
  if (extractSort(instance) === untouchedSort) {
    return instance.sort(...args);
  }
  return safeApply(untouchedSort, instance, args);
}
function safeEvery(instance, ...args) {
  if (extractEvery(instance) === untouchedEvery) {
    return instance.every(...args);
  }
  return safeApply(untouchedEvery, instance, args);
}
var untouchedGetTime = Date.prototype.getTime;
var untouchedToISOString = Date.prototype.toISOString;
function extractGetTime(instance) {
  try {
    return instance.getTime;
  } catch (err) {
    return void 0;
  }
}
function extractToISOString(instance) {
  try {
    return instance.toISOString;
  } catch (err) {
    return void 0;
  }
}
function safeGetTime(instance) {
  if (extractGetTime(instance) === untouchedGetTime) {
    return instance.getTime();
  }
  return safeApply(untouchedGetTime, instance, []);
}
function safeToISOString(instance) {
  if (extractToISOString(instance) === untouchedToISOString) {
    return instance.toISOString();
  }
  return safeApply(untouchedToISOString, instance, []);
}
var untouchedAdd = Set.prototype.add;
function extractAdd(instance) {
  try {
    return instance.add;
  } catch (err) {
    return void 0;
  }
}
function safeAdd(instance, value) {
  if (extractAdd(instance) === untouchedAdd) {
    return instance.add(value);
  }
  return safeApply(untouchedAdd, instance, [value]);
}
var untouchedSplit = String.prototype.split;
var untouchedStartsWith = String.prototype.startsWith;
var untouchedEndsWith = String.prototype.endsWith;
var untouchedSubstring = String.prototype.substring;
var untouchedToLowerCase = String.prototype.toLowerCase;
var untouchedToUpperCase = String.prototype.toUpperCase;
var untouchedPadStart = String.prototype.padStart;
var untouchedCharCodeAt = String.prototype.charCodeAt;
var untouchedReplace = String.prototype.replace;
function extractSplit(instance) {
  try {
    return instance.split;
  } catch (err) {
    return void 0;
  }
}
function extractStartsWith(instance) {
  try {
    return instance.startsWith;
  } catch (err) {
    return void 0;
  }
}
function extractEndsWith(instance) {
  try {
    return instance.endsWith;
  } catch (err) {
    return void 0;
  }
}
function extractSubstring(instance) {
  try {
    return instance.substring;
  } catch (err) {
    return void 0;
  }
}
function extractToLowerCase(instance) {
  try {
    return instance.toLowerCase;
  } catch (err) {
    return void 0;
  }
}
function extractToUpperCase(instance) {
  try {
    return instance.toUpperCase;
  } catch (err) {
    return void 0;
  }
}
function extractPadStart(instance) {
  try {
    return instance.padStart;
  } catch (err) {
    return void 0;
  }
}
function extractCharCodeAt(instance) {
  try {
    return instance.charCodeAt;
  } catch (err) {
    return void 0;
  }
}
function extractReplace(instance) {
  try {
    return instance.replace;
  } catch (err) {
    return void 0;
  }
}
function safeSplit(instance, ...args) {
  if (extractSplit(instance) === untouchedSplit) {
    return instance.split(...args);
  }
  return safeApply(untouchedSplit, instance, args);
}
function safeStartsWith(instance, ...args) {
  if (extractStartsWith(instance) === untouchedStartsWith) {
    return instance.startsWith(...args);
  }
  return safeApply(untouchedStartsWith, instance, args);
}
function safeEndsWith(instance, ...args) {
  if (extractEndsWith(instance) === untouchedEndsWith) {
    return instance.endsWith(...args);
  }
  return safeApply(untouchedEndsWith, instance, args);
}
function safeSubstring(instance, ...args) {
  if (extractSubstring(instance) === untouchedSubstring) {
    return instance.substring(...args);
  }
  return safeApply(untouchedSubstring, instance, args);
}
function safeToLowerCase(instance) {
  if (extractToLowerCase(instance) === untouchedToLowerCase) {
    return instance.toLowerCase();
  }
  return safeApply(untouchedToLowerCase, instance, []);
}
function safeToUpperCase(instance) {
  if (extractToUpperCase(instance) === untouchedToUpperCase) {
    return instance.toUpperCase();
  }
  return safeApply(untouchedToUpperCase, instance, []);
}
function safePadStart(instance, ...args) {
  if (extractPadStart(instance) === untouchedPadStart) {
    return instance.padStart(...args);
  }
  return safeApply(untouchedPadStart, instance, args);
}
function safeCharCodeAt(instance, index) {
  if (extractCharCodeAt(instance) === untouchedCharCodeAt) {
    return instance.charCodeAt(index);
  }
  return safeApply(untouchedCharCodeAt, instance, [index]);
}
function safeReplace(instance, pattern, replacement) {
  if (extractReplace(instance) === untouchedReplace) {
    return instance.replace(pattern, replacement);
  }
  return safeApply(untouchedReplace, instance, [pattern, replacement]);
}
var untouchedNumberToString = Number.prototype.toString;
function extractNumberToString(instance) {
  try {
    return instance.toString;
  } catch (err) {
    return void 0;
  }
}
function safeNumberToString(instance, ...args) {
  if (extractNumberToString(instance) === untouchedNumberToString) {
    return instance.toString(...args);
  }
  return safeApply(untouchedNumberToString, instance, args);
}
var untouchedHasOwnProperty = Object.prototype.hasOwnProperty;
var untouchedToString = Object.prototype.toString;
function safeHasOwnProperty(instance, v) {
  return safeApply(untouchedHasOwnProperty, instance, [v]);
}
function safeToString(instance) {
  return safeApply(untouchedToString, instance, []);
}

// ../../../node_modules/fast-check/lib/esm/stream/LazyIterableIterator.js
var LazyIterableIterator = class {
  constructor(producer) {
    this.producer = producer;
  }
  [Symbol.iterator]() {
    if (this.it === void 0) {
      this.it = this.producer();
    }
    return this.it;
  }
  next() {
    if (this.it === void 0) {
      this.it = this.producer();
    }
    return this.it.next();
  }
};
function makeLazy(producer) {
  return new LazyIterableIterator(producer);
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/_internals/TupleArbitrary.js
var safeArrayIsArray = Array.isArray;
var safeObjectDefineProperty2 = Object.defineProperty;
function tupleMakeItCloneable(vs, values) {
  return safeObjectDefineProperty2(vs, cloneMethod, {
    value: () => {
      const cloned = [];
      for (let idx = 0; idx !== values.length; ++idx) {
        safePush(cloned, values[idx].value);
      }
      tupleMakeItCloneable(cloned, values);
      return cloned;
    },
  });
}
function tupleWrapper(values) {
  let cloneable = false;
  const vs = [];
  const ctxs = [];
  for (let idx = 0; idx !== values.length; ++idx) {
    const v = values[idx];
    cloneable = cloneable || v.hasToBeCloned;
    safePush(vs, v.value);
    safePush(ctxs, v.context);
  }
  if (cloneable) {
    tupleMakeItCloneable(vs, values);
  }
  return new Value(vs, ctxs);
}
function tupleShrink(arbs, value, context2) {
  const shrinks = [];
  const safeContext = safeArrayIsArray(context2) ? context2 : [];
  for (let idx = 0; idx !== arbs.length; ++idx) {
    safePush(
      shrinks,
      makeLazy(() =>
        arbs[idx]
          .shrink(value[idx], safeContext[idx])
          .map((v) => {
            const nextValues = safeMap(
              value,
              (v2, idx2) => new Value(cloneIfNeeded(v2), safeContext[idx2]),
            );
            return [
              ...safeSlice(nextValues, 0, idx),
              v,
              ...safeSlice(nextValues, idx + 1),
            ];
          })
          .map(tupleWrapper),
      ),
    );
  }
  return Stream.nil().join(...shrinks);
}
var TupleArbitrary = class extends Arbitrary {
  constructor(arbs) {
    super();
    this.arbs = arbs;
    for (let idx = 0; idx !== arbs.length; ++idx) {
      const arb = arbs[idx];
      if (arb == null || arb.generate == null)
        throw new Error(
          `Invalid parameter encountered at index ${idx}: expecting an Arbitrary`,
        );
    }
  }
  generate(mrng, biasFactor) {
    const mapped = [];
    for (let idx = 0; idx !== this.arbs.length; ++idx) {
      safePush(mapped, this.arbs[idx].generate(mrng, biasFactor));
    }
    return tupleWrapper(mapped);
  }
  canShrinkWithoutContext(value) {
    if (!safeArrayIsArray(value) || value.length !== this.arbs.length) {
      return false;
    }
    for (let index = 0; index !== this.arbs.length; ++index) {
      if (!this.arbs[index].canShrinkWithoutContext(value[index])) {
        return false;
      }
    }
    return true;
  }
  shrink(value, context2) {
    return tupleShrink(this.arbs, value, context2);
  }
};

// ../../../node_modules/fast-check/lib/esm/arbitrary/tuple.js
function tuple(...arbs) {
  return new TupleArbitrary(arbs);
}

// ../../../node_modules/fast-check/lib/esm/check/property/IRawProperty.js
var safeMathLog = Math.log;
function runIdToFrequency(runId) {
  return 2 + ~~(safeMathLog(runId + 1) * 0.4342944819032518);
}

// ../../../node_modules/fast-check/lib/esm/check/runner/configuration/GlobalParameters.js
var globalParameters = {};
function configureGlobal(parameters) {
  globalParameters = parameters;
}
function readConfigureGlobal() {
  return globalParameters;
}
function resetConfigureGlobal() {
  globalParameters = {};
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/_internals/helpers/NoUndefinedAsContext.js
var UndefinedContextPlaceholder = Symbol('UndefinedContextPlaceholder');
function noUndefinedAsContext(value) {
  if (value.context !== void 0) {
    return value;
  }
  if (value.hasToBeCloned) {
    return new Value(
      value.value_,
      UndefinedContextPlaceholder,
      () => value.value,
    );
  }
  return new Value(value.value_, UndefinedContextPlaceholder);
}

// ../../../node_modules/fast-check/lib/esm/check/property/AsyncProperty.generic.js
var AsyncProperty = class _AsyncProperty {
  constructor(arb, predicate) {
    this.arb = arb;
    this.predicate = predicate;
    const { asyncBeforeEach, asyncAfterEach, beforeEach, afterEach } =
      readConfigureGlobal() || {};
    if (asyncBeforeEach !== void 0 && beforeEach !== void 0) {
      throw SError(
        `Global "asyncBeforeEach" and "beforeEach" parameters can't be set at the same time when running async properties`,
      );
    }
    if (asyncAfterEach !== void 0 && afterEach !== void 0) {
      throw SError(
        `Global "asyncAfterEach" and "afterEach" parameters can't be set at the same time when running async properties`,
      );
    }
    this.beforeEachHook =
      asyncBeforeEach || beforeEach || _AsyncProperty.dummyHook;
    this.afterEachHook =
      asyncAfterEach || afterEach || _AsyncProperty.dummyHook;
  }
  isAsync() {
    return true;
  }
  generate(mrng, runId) {
    const value = this.arb.generate(
      mrng,
      runId != null ? runIdToFrequency(runId) : void 0,
    );
    return noUndefinedAsContext(value);
  }
  shrink(value) {
    if (
      value.context === void 0 &&
      !this.arb.canShrinkWithoutContext(value.value_)
    ) {
      return Stream.nil();
    }
    const safeContext =
      value.context !== UndefinedContextPlaceholder ? value.context : void 0;
    return this.arb.shrink(value.value_, safeContext).map(noUndefinedAsContext);
  }
  async runBeforeEach() {
    await this.beforeEachHook();
  }
  async runAfterEach() {
    await this.afterEachHook();
  }
  async run(v, dontRunHook) {
    if (!dontRunHook) {
      await this.beforeEachHook();
    }
    try {
      const output = await this.predicate(v);
      return output == null || output === true
        ? null
        : {
            error: new SError('Property failed by returning false'),
            errorMessage: 'Error: Property failed by returning false',
          };
    } catch (err) {
      if (PreconditionFailure.isFailure(err)) return err;
      if (err instanceof SError && err.stack) {
        return { error: err, errorMessage: err.stack };
      }
      return { error: err, errorMessage: SString(err) };
    } finally {
      if (!dontRunHook) {
        await this.afterEachHook();
      }
    }
  }
  beforeEach(hookFunction) {
    const previousBeforeEachHook = this.beforeEachHook;
    this.beforeEachHook = () => hookFunction(previousBeforeEachHook);
    return this;
  }
  afterEach(hookFunction) {
    const previousAfterEachHook = this.afterEachHook;
    this.afterEachHook = () => hookFunction(previousAfterEachHook);
    return this;
  }
};
AsyncProperty.dummyHook = () => {};

// ../../../node_modules/fast-check/lib/esm/arbitrary/_internals/AlwaysShrinkableArbitrary.js
var AlwaysShrinkableArbitrary = class extends Arbitrary {
  constructor(arb) {
    super();
    this.arb = arb;
  }
  generate(mrng, biasFactor) {
    const value = this.arb.generate(mrng, biasFactor);
    return noUndefinedAsContext(value);
  }
  canShrinkWithoutContext(value) {
    return true;
  }
  shrink(value, context2) {
    if (context2 === void 0 && !this.arb.canShrinkWithoutContext(value)) {
      return Stream.nil();
    }
    const safeContext =
      context2 !== UndefinedContextPlaceholder ? context2 : void 0;
    return this.arb.shrink(value, safeContext).map(noUndefinedAsContext);
  }
};

// ../../../node_modules/fast-check/lib/esm/check/property/AsyncProperty.js
function asyncProperty(...args) {
  if (args.length < 2) {
    throw new Error('asyncProperty expects at least two parameters');
  }
  const arbs = safeSlice(args, 0, args.length - 1);
  const p = args[args.length - 1];
  safeForEach(arbs, assertIsArbitrary);
  const mappedArbs = safeMap(arbs, (arb) => new AlwaysShrinkableArbitrary(arb));
  return new AsyncProperty(tuple(...mappedArbs), (t) => p(...t));
}

// ../../../node_modules/fast-check/lib/esm/check/property/Property.generic.js
var Property = class _Property {
  constructor(arb, predicate) {
    this.arb = arb;
    this.predicate = predicate;
    const {
      beforeEach = _Property.dummyHook,
      afterEach = _Property.dummyHook,
      asyncBeforeEach,
      asyncAfterEach,
    } = readConfigureGlobal() || {};
    if (asyncBeforeEach !== void 0) {
      throw SError(
        `"asyncBeforeEach" can't be set when running synchronous properties`,
      );
    }
    if (asyncAfterEach !== void 0) {
      throw SError(
        `"asyncAfterEach" can't be set when running synchronous properties`,
      );
    }
    this.beforeEachHook = beforeEach;
    this.afterEachHook = afterEach;
  }
  isAsync() {
    return false;
  }
  generate(mrng, runId) {
    const value = this.arb.generate(
      mrng,
      runId != null ? runIdToFrequency(runId) : void 0,
    );
    return noUndefinedAsContext(value);
  }
  shrink(value) {
    if (
      value.context === void 0 &&
      !this.arb.canShrinkWithoutContext(value.value_)
    ) {
      return Stream.nil();
    }
    const safeContext =
      value.context !== UndefinedContextPlaceholder ? value.context : void 0;
    return this.arb.shrink(value.value_, safeContext).map(noUndefinedAsContext);
  }
  runBeforeEach() {
    this.beforeEachHook();
  }
  runAfterEach() {
    this.afterEachHook();
  }
  run(v, dontRunHook) {
    if (!dontRunHook) {
      this.beforeEachHook();
    }
    try {
      const output = this.predicate(v);
      return output == null || output === true
        ? null
        : {
            error: new SError('Property failed by returning false'),
            errorMessage: 'Error: Property failed by returning false',
          };
    } catch (err) {
      if (PreconditionFailure.isFailure(err)) return err;
      if (err instanceof SError && err.stack) {
        return { error: err, errorMessage: err.stack };
      }
      return { error: err, errorMessage: SString(err) };
    } finally {
      if (!dontRunHook) {
        this.afterEachHook();
      }
    }
  }
  beforeEach(hookFunction) {
    const previousBeforeEachHook = this.beforeEachHook;
    this.beforeEachHook = () => hookFunction(previousBeforeEachHook);
    return this;
  }
  afterEach(hookFunction) {
    const previousAfterEachHook = this.afterEachHook;
    this.afterEachHook = () => hookFunction(previousAfterEachHook);
    return this;
  }
};
Property.dummyHook = () => {};

// ../../../node_modules/fast-check/lib/esm/check/property/Property.js
function property(...args) {
  if (args.length < 2) {
    throw new Error('property expects at least two parameters');
  }
  const arbs = safeSlice(args, 0, args.length - 1);
  const p = args[args.length - 1];
  safeForEach(arbs, assertIsArbitrary);
  const mappedArbs = safeMap(arbs, (arb) => new AlwaysShrinkableArbitrary(arb));
  return new Property(tuple(...mappedArbs), (t) => p(...t));
}

// ../../../node_modules/pure-rand/lib/esm/pure-rand-default.js
var pure_rand_default_exports = {};
__export(pure_rand_default_exports, {
  __commitHash: () => __commitHash,
  __type: () => __type,
  __version: () => __version,
  congruential32: () => congruential32,
  generateN: () => generateN,
  mersenne: () => MersenneTwister_default,
  skipN: () => skipN,
  uniformArrayIntDistribution: () => uniformArrayIntDistribution,
  uniformBigIntDistribution: () => uniformBigIntDistribution,
  uniformIntDistribution: () => uniformIntDistribution,
  unsafeGenerateN: () => unsafeGenerateN,
  unsafeSkipN: () => unsafeSkipN,
  unsafeUniformArrayIntDistribution: () => unsafeUniformArrayIntDistribution,
  unsafeUniformBigIntDistribution: () => unsafeUniformBigIntDistribution,
  unsafeUniformIntDistribution: () => unsafeUniformIntDistribution,
  xoroshiro128plus: () => xoroshiro128plus,
  xorshift128plus: () => xorshift128plus,
});

// ../../../node_modules/pure-rand/lib/esm/generator/RandomGenerator.js
function unsafeGenerateN(rng, num) {
  var out = [];
  for (var idx = 0; idx != num; ++idx) {
    out.push(rng.unsafeNext());
  }
  return out;
}
function generateN(rng, num) {
  var nextRng = rng.clone();
  var out = unsafeGenerateN(nextRng, num);
  return [out, nextRng];
}
function unsafeSkipN(rng, num) {
  for (var idx = 0; idx != num; ++idx) {
    rng.unsafeNext();
  }
}
function skipN(rng, num) {
  var nextRng = rng.clone();
  unsafeSkipN(nextRng, num);
  return nextRng;
}

// ../../../node_modules/pure-rand/lib/esm/generator/LinearCongruential.js
var MULTIPLIER = 214013;
var INCREMENT = 2531011;
var MASK = 4294967295;
var MASK_2 = (1 << 31) - 1;
var computeNextSeed = function (seed) {
  return (seed * MULTIPLIER + INCREMENT) & MASK;
};
var computeValueFromNextSeed = function (nextseed) {
  return (nextseed & MASK_2) >> 16;
};
var LinearCongruential32 = (function () {
  function LinearCongruential322(seed) {
    this.seed = seed;
  }
  LinearCongruential322.prototype.clone = function () {
    return new LinearCongruential322(this.seed);
  };
  LinearCongruential322.prototype.next = function () {
    var nextRng = new LinearCongruential322(this.seed);
    var out = nextRng.unsafeNext();
    return [out, nextRng];
  };
  LinearCongruential322.prototype.unsafeNext = function () {
    var s1 = computeNextSeed(this.seed);
    var v1 = computeValueFromNextSeed(s1);
    var s2 = computeNextSeed(s1);
    var v2 = computeValueFromNextSeed(s2);
    this.seed = computeNextSeed(s2);
    var v3 = computeValueFromNextSeed(this.seed);
    var vnext = v3 + ((v2 + (v1 << 15)) << 15);
    return vnext | 0;
  };
  LinearCongruential322.prototype.getState = function () {
    return [this.seed];
  };
  return LinearCongruential322;
})();
function fromState(state) {
  var valid = state.length === 1;
  if (!valid) {
    throw new Error(
      'The state must have been produced by a congruential32 RandomGenerator',
    );
  }
  return new LinearCongruential32(state[0]);
}
var congruential32 = Object.assign(
  function (seed) {
    return new LinearCongruential32(seed);
  },
  { fromState },
);

// ../../../node_modules/pure-rand/lib/esm/generator/MersenneTwister.js
var __read = function (o, n) {
  var m = typeof Symbol === 'function' && o[Symbol.iterator];
  if (!m) return o;
  var i = m.call(o),
    r,
    ar = [],
    e;
  try {
    while ((n === void 0 || n-- > 0) && !(r = i.next()).done) ar.push(r.value);
  } catch (error) {
    e = { error };
  } finally {
    try {
      if (r && !r.done && (m = i['return'])) m.call(i);
    } finally {
      if (e) throw e.error;
    }
  }
  return ar;
};
var __spreadArray = function (to, from, pack) {
  if (pack || arguments.length === 2)
    for (var i = 0, l = from.length, ar; i < l; i++) {
      if (ar || !(i in from)) {
        if (!ar) ar = Array.prototype.slice.call(from, 0, i);
        ar[i] = from[i];
      }
    }
  return to.concat(ar || Array.prototype.slice.call(from));
};
var MersenneTwister = (function () {
  function MersenneTwister2(states, index) {
    this.states = states;
    this.index = index;
  }
  MersenneTwister2.twist = function (prev) {
    var mt = prev.slice();
    for (var idx = 0; idx !== MersenneTwister2.N - MersenneTwister2.M; ++idx) {
      var y_1 =
        (mt[idx] & MersenneTwister2.MASK_UPPER) +
        (mt[idx + 1] & MersenneTwister2.MASK_LOWER);
      mt[idx] =
        mt[idx + MersenneTwister2.M] ^
        (y_1 >>> 1) ^
        (-(y_1 & 1) & MersenneTwister2.A);
    }
    for (
      var idx = MersenneTwister2.N - MersenneTwister2.M;
      idx !== MersenneTwister2.N - 1;
      ++idx
    ) {
      var y_2 =
        (mt[idx] & MersenneTwister2.MASK_UPPER) +
        (mt[idx + 1] & MersenneTwister2.MASK_LOWER);
      mt[idx] =
        mt[idx + MersenneTwister2.M - MersenneTwister2.N] ^
        (y_2 >>> 1) ^
        (-(y_2 & 1) & MersenneTwister2.A);
    }
    var y =
      (mt[MersenneTwister2.N - 1] & MersenneTwister2.MASK_UPPER) +
      (mt[0] & MersenneTwister2.MASK_LOWER);
    mt[MersenneTwister2.N - 1] =
      mt[MersenneTwister2.M - 1] ^ (y >>> 1) ^ (-(y & 1) & MersenneTwister2.A);
    return mt;
  };
  MersenneTwister2.seeded = function (seed) {
    var out = Array(MersenneTwister2.N);
    out[0] = seed;
    for (var idx = 1; idx !== MersenneTwister2.N; ++idx) {
      var xored = out[idx - 1] ^ (out[idx - 1] >>> 30);
      out[idx] = (Math.imul(MersenneTwister2.F, xored) + idx) | 0;
    }
    return out;
  };
  MersenneTwister2.from = function (seed) {
    return new MersenneTwister2(
      MersenneTwister2.twist(MersenneTwister2.seeded(seed)),
      0,
    );
  };
  MersenneTwister2.prototype.clone = function () {
    return new MersenneTwister2(this.states, this.index);
  };
  MersenneTwister2.prototype.next = function () {
    var nextRng = new MersenneTwister2(this.states, this.index);
    var out = nextRng.unsafeNext();
    return [out, nextRng];
  };
  MersenneTwister2.prototype.unsafeNext = function () {
    var y = this.states[this.index];
    y ^= this.states[this.index] >>> MersenneTwister2.U;
    y ^= (y << MersenneTwister2.S) & MersenneTwister2.B;
    y ^= (y << MersenneTwister2.T) & MersenneTwister2.C;
    y ^= y >>> MersenneTwister2.L;
    if (++this.index >= MersenneTwister2.N) {
      this.states = MersenneTwister2.twist(this.states);
      this.index = 0;
    }
    return y;
  };
  MersenneTwister2.prototype.getState = function () {
    return __spreadArray([this.index], __read(this.states), false);
  };
  MersenneTwister2.fromState = function (state) {
    var valid =
      state.length === MersenneTwister2.N + 1 &&
      state[0] >= 0 &&
      state[0] < MersenneTwister2.N;
    if (!valid) {
      throw new Error(
        'The state must have been produced by a mersenne RandomGenerator',
      );
    }
    return new MersenneTwister2(state.slice(1), state[0]);
  };
  MersenneTwister2.N = 624;
  MersenneTwister2.M = 397;
  MersenneTwister2.R = 31;
  MersenneTwister2.A = 2567483615;
  MersenneTwister2.F = 1812433253;
  MersenneTwister2.U = 11;
  MersenneTwister2.S = 7;
  MersenneTwister2.B = 2636928640;
  MersenneTwister2.T = 15;
  MersenneTwister2.C = 4022730752;
  MersenneTwister2.L = 18;
  MersenneTwister2.MASK_LOWER = Math.pow(2, MersenneTwister2.R) - 1;
  MersenneTwister2.MASK_UPPER = Math.pow(2, MersenneTwister2.R);
  return MersenneTwister2;
})();
function fromState2(state) {
  return MersenneTwister.fromState(state);
}
var MersenneTwister_default = Object.assign(
  function (seed) {
    return MersenneTwister.from(seed);
  },
  { fromState: fromState2 },
);

// ../../../node_modules/pure-rand/lib/esm/generator/XorShift.js
var XorShift128Plus = (function () {
  function XorShift128Plus2(s01, s00, s11, s10) {
    this.s01 = s01;
    this.s00 = s00;
    this.s11 = s11;
    this.s10 = s10;
  }
  XorShift128Plus2.prototype.clone = function () {
    return new XorShift128Plus2(this.s01, this.s00, this.s11, this.s10);
  };
  XorShift128Plus2.prototype.next = function () {
    var nextRng = new XorShift128Plus2(this.s01, this.s00, this.s11, this.s10);
    var out = nextRng.unsafeNext();
    return [out, nextRng];
  };
  XorShift128Plus2.prototype.unsafeNext = function () {
    var a0 = this.s00 ^ (this.s00 << 23);
    var a1 = this.s01 ^ ((this.s01 << 23) | (this.s00 >>> 9));
    var b0 =
      a0 ^
      this.s10 ^
      ((a0 >>> 18) | (a1 << 14)) ^
      ((this.s10 >>> 5) | (this.s11 << 27));
    var b1 = a1 ^ this.s11 ^ (a1 >>> 18) ^ (this.s11 >>> 5);
    var out = (this.s00 + this.s10) | 0;
    this.s01 = this.s11;
    this.s00 = this.s10;
    this.s11 = b1;
    this.s10 = b0;
    return out;
  };
  XorShift128Plus2.prototype.jump = function () {
    var nextRng = new XorShift128Plus2(this.s01, this.s00, this.s11, this.s10);
    nextRng.unsafeJump();
    return nextRng;
  };
  XorShift128Plus2.prototype.unsafeJump = function () {
    var ns01 = 0;
    var ns00 = 0;
    var ns11 = 0;
    var ns10 = 0;
    var jump = [1667051007, 2321340297, 1548169110, 304075285];
    for (var i = 0; i !== 4; ++i) {
      for (var mask = 1; mask; mask <<= 1) {
        if (jump[i] & mask) {
          ns01 ^= this.s01;
          ns00 ^= this.s00;
          ns11 ^= this.s11;
          ns10 ^= this.s10;
        }
        this.unsafeNext();
      }
    }
    this.s01 = ns01;
    this.s00 = ns00;
    this.s11 = ns11;
    this.s10 = ns10;
  };
  XorShift128Plus2.prototype.getState = function () {
    return [this.s01, this.s00, this.s11, this.s10];
  };
  return XorShift128Plus2;
})();
function fromState3(state) {
  var valid = state.length === 4;
  if (!valid) {
    throw new Error(
      'The state must have been produced by a xorshift128plus RandomGenerator',
    );
  }
  return new XorShift128Plus(state[0], state[1], state[2], state[3]);
}
var xorshift128plus = Object.assign(
  function (seed) {
    return new XorShift128Plus(-1, ~seed, seed | 0, 0);
  },
  { fromState: fromState3 },
);

// ../../../node_modules/pure-rand/lib/esm/generator/XoroShiro.js
var XoroShiro128Plus = (function () {
  function XoroShiro128Plus2(s01, s00, s11, s10) {
    this.s01 = s01;
    this.s00 = s00;
    this.s11 = s11;
    this.s10 = s10;
  }
  XoroShiro128Plus2.prototype.clone = function () {
    return new XoroShiro128Plus2(this.s01, this.s00, this.s11, this.s10);
  };
  XoroShiro128Plus2.prototype.next = function () {
    var nextRng = new XoroShiro128Plus2(this.s01, this.s00, this.s11, this.s10);
    var out = nextRng.unsafeNext();
    return [out, nextRng];
  };
  XoroShiro128Plus2.prototype.unsafeNext = function () {
    var out = (this.s00 + this.s10) | 0;
    var a0 = this.s10 ^ this.s00;
    var a1 = this.s11 ^ this.s01;
    var s00 = this.s00;
    var s01 = this.s01;
    this.s00 = (s00 << 24) ^ (s01 >>> 8) ^ a0 ^ (a0 << 16);
    this.s01 = (s01 << 24) ^ (s00 >>> 8) ^ a1 ^ ((a1 << 16) | (a0 >>> 16));
    this.s10 = (a1 << 5) ^ (a0 >>> 27);
    this.s11 = (a0 << 5) ^ (a1 >>> 27);
    return out;
  };
  XoroShiro128Plus2.prototype.jump = function () {
    var nextRng = new XoroShiro128Plus2(this.s01, this.s00, this.s11, this.s10);
    nextRng.unsafeJump();
    return nextRng;
  };
  XoroShiro128Plus2.prototype.unsafeJump = function () {
    var ns01 = 0;
    var ns00 = 0;
    var ns11 = 0;
    var ns10 = 0;
    var jump = [3639956645, 3750757012, 1261568508, 386426335];
    for (var i = 0; i !== 4; ++i) {
      for (var mask = 1; mask; mask <<= 1) {
        if (jump[i] & mask) {
          ns01 ^= this.s01;
          ns00 ^= this.s00;
          ns11 ^= this.s11;
          ns10 ^= this.s10;
        }
        this.unsafeNext();
      }
    }
    this.s01 = ns01;
    this.s00 = ns00;
    this.s11 = ns11;
    this.s10 = ns10;
  };
  XoroShiro128Plus2.prototype.getState = function () {
    return [this.s01, this.s00, this.s11, this.s10];
  };
  return XoroShiro128Plus2;
})();
function fromState4(state) {
  var valid = state.length === 4;
  if (!valid) {
    throw new Error(
      'The state must have been produced by a xoroshiro128plus RandomGenerator',
    );
  }
  return new XoroShiro128Plus(state[0], state[1], state[2], state[3]);
}
var xoroshiro128plus = Object.assign(
  function (seed) {
    return new XoroShiro128Plus(-1, ~seed, seed | 0, 0);
  },
  { fromState: fromState4 },
);

// ../../../node_modules/pure-rand/lib/esm/distribution/internals/ArrayInt.js
function addArrayIntToNew(arrayIntA, arrayIntB) {
  if (arrayIntA.sign !== arrayIntB.sign) {
    return substractArrayIntToNew(arrayIntA, {
      sign: -arrayIntB.sign,
      data: arrayIntB.data,
    });
  }
  var data = [];
  var reminder = 0;
  var dataA = arrayIntA.data;
  var dataB = arrayIntB.data;
  for (
    var indexA = dataA.length - 1, indexB = dataB.length - 1;
    indexA >= 0 || indexB >= 0;
    --indexA, --indexB
  ) {
    var vA = indexA >= 0 ? dataA[indexA] : 0;
    var vB = indexB >= 0 ? dataB[indexB] : 0;
    var current = vA + vB + reminder;
    data.push(current >>> 0);
    reminder = ~~(current / 4294967296);
  }
  if (reminder !== 0) {
    data.push(reminder);
  }
  return { sign: arrayIntA.sign, data: data.reverse() };
}
function addOneToPositiveArrayInt(arrayInt) {
  arrayInt.sign = 1;
  var data = arrayInt.data;
  for (var index = data.length - 1; index >= 0; --index) {
    if (data[index] === 4294967295) {
      data[index] = 0;
    } else {
      data[index] += 1;
      return arrayInt;
    }
  }
  data.unshift(1);
  return arrayInt;
}
function isStrictlySmaller(dataA, dataB) {
  var maxLength = Math.max(dataA.length, dataB.length);
  for (var index = 0; index < maxLength; ++index) {
    var indexA = index + dataA.length - maxLength;
    var indexB = index + dataB.length - maxLength;
    var vA = indexA >= 0 ? dataA[indexA] : 0;
    var vB = indexB >= 0 ? dataB[indexB] : 0;
    if (vA < vB) return true;
    if (vA > vB) return false;
  }
  return false;
}
function substractArrayIntToNew(arrayIntA, arrayIntB) {
  if (arrayIntA.sign !== arrayIntB.sign) {
    return addArrayIntToNew(arrayIntA, {
      sign: -arrayIntB.sign,
      data: arrayIntB.data,
    });
  }
  var dataA = arrayIntA.data;
  var dataB = arrayIntB.data;
  if (isStrictlySmaller(dataA, dataB)) {
    var out = substractArrayIntToNew(arrayIntB, arrayIntA);
    out.sign = -out.sign;
    return out;
  }
  var data = [];
  var reminder = 0;
  for (
    var indexA = dataA.length - 1, indexB = dataB.length - 1;
    indexA >= 0 || indexB >= 0;
    --indexA, --indexB
  ) {
    var vA = indexA >= 0 ? dataA[indexA] : 0;
    var vB = indexB >= 0 ? dataB[indexB] : 0;
    var current = vA - vB - reminder;
    data.push(current >>> 0);
    reminder = current < 0 ? 1 : 0;
  }
  return { sign: arrayIntA.sign, data: data.reverse() };
}
function trimArrayIntInplace(arrayInt) {
  var data = arrayInt.data;
  var firstNonZero = 0;
  for (
    ;
    firstNonZero !== data.length && data[firstNonZero] === 0;
    ++firstNonZero
  ) {}
  if (firstNonZero === data.length) {
    arrayInt.sign = 1;
    arrayInt.data = [0];
    return arrayInt;
  }
  data.splice(0, firstNonZero);
  return arrayInt;
}
function fromNumberToArrayInt64(out, n) {
  if (n < 0) {
    var posN = -n;
    out.sign = -1;
    out.data[0] = ~~(posN / 4294967296);
    out.data[1] = posN >>> 0;
  } else {
    out.sign = 1;
    out.data[0] = ~~(n / 4294967296);
    out.data[1] = n >>> 0;
  }
  return out;
}
function substractArrayInt64(out, arrayIntA, arrayIntB) {
  var lowA = arrayIntA.data[1];
  var highA = arrayIntA.data[0];
  var signA = arrayIntA.sign;
  var lowB = arrayIntB.data[1];
  var highB = arrayIntB.data[0];
  var signB = arrayIntB.sign;
  out.sign = 1;
  if (signA === 1 && signB === -1) {
    var low_1 = lowA + lowB;
    var high = highA + highB + (low_1 > 4294967295 ? 1 : 0);
    out.data[0] = high >>> 0;
    out.data[1] = low_1 >>> 0;
    return out;
  }
  var lowFirst = lowA;
  var highFirst = highA;
  var lowSecond = lowB;
  var highSecond = highB;
  if (signA === -1) {
    lowFirst = lowB;
    highFirst = highB;
    lowSecond = lowA;
    highSecond = highA;
  }
  var reminderLow = 0;
  var low = lowFirst - lowSecond;
  if (low < 0) {
    reminderLow = 1;
    low = low >>> 0;
  }
  out.data[0] = highFirst - highSecond - reminderLow;
  out.data[1] = low;
  return out;
}

// ../../../node_modules/pure-rand/lib/esm/distribution/internals/UnsafeUniformIntDistributionInternal.js
function unsafeUniformIntDistributionInternal(rangeSize, rng) {
  var MaxAllowed =
    rangeSize > 2 ? ~~(4294967296 / rangeSize) * rangeSize : 4294967296;
  var deltaV = rng.unsafeNext() + 2147483648;
  while (deltaV >= MaxAllowed) {
    deltaV = rng.unsafeNext() + 2147483648;
  }
  return deltaV % rangeSize;
}

// ../../../node_modules/pure-rand/lib/esm/distribution/internals/UnsafeUniformArrayIntDistributionInternal.js
function unsafeUniformArrayIntDistributionInternal(out, rangeSize, rng) {
  var rangeLength = rangeSize.length;
  while (true) {
    for (var index = 0; index !== rangeLength; ++index) {
      var indexRangeSize = index === 0 ? rangeSize[0] + 1 : 4294967296;
      var g = unsafeUniformIntDistributionInternal(indexRangeSize, rng);
      out[index] = g;
    }
    for (var index = 0; index !== rangeLength; ++index) {
      var current = out[index];
      var currentInRange = rangeSize[index];
      if (current < currentInRange) {
        return out;
      } else if (current > currentInRange) {
        break;
      }
    }
  }
}

// ../../../node_modules/pure-rand/lib/esm/distribution/UnsafeUniformArrayIntDistribution.js
function unsafeUniformArrayIntDistribution(from, to, rng) {
  var rangeSize = trimArrayIntInplace(
    addOneToPositiveArrayInt(substractArrayIntToNew(to, from)),
  );
  var emptyArrayIntData = rangeSize.data.slice(0);
  var g = unsafeUniformArrayIntDistributionInternal(
    emptyArrayIntData,
    rangeSize.data,
    rng,
  );
  return trimArrayIntInplace(addArrayIntToNew({ sign: 1, data: g }, from));
}

// ../../../node_modules/pure-rand/lib/esm/distribution/UniformArrayIntDistribution.js
function uniformArrayIntDistribution(from, to, rng) {
  if (rng != null) {
    var nextRng = rng.clone();
    return [unsafeUniformArrayIntDistribution(from, to, nextRng), nextRng];
  }
  return function (rng2) {
    var nextRng2 = rng2.clone();
    return [unsafeUniformArrayIntDistribution(from, to, nextRng2), nextRng2];
  };
}

// ../../../node_modules/pure-rand/lib/esm/distribution/UnsafeUniformBigIntDistribution.js
var SBigInt2 = typeof BigInt !== 'undefined' ? BigInt : void 0;
function unsafeUniformBigIntDistribution(from, to, rng) {
  var diff = to - from + SBigInt2(1);
  var MinRng = SBigInt2(-2147483648);
  var NumValues = SBigInt2(4294967296);
  var FinalNumValues = NumValues;
  var NumIterations = 1;
  while (FinalNumValues < diff) {
    FinalNumValues *= NumValues;
    ++NumIterations;
  }
  var MaxAcceptedRandom = FinalNumValues - (FinalNumValues % diff);
  while (true) {
    var value = SBigInt2(0);
    for (var num = 0; num !== NumIterations; ++num) {
      var out = rng.unsafeNext();
      value = NumValues * value + (SBigInt2(out) - MinRng);
    }
    if (value < MaxAcceptedRandom) {
      var inDiff = value % diff;
      return inDiff + from;
    }
  }
}

// ../../../node_modules/pure-rand/lib/esm/distribution/UniformBigIntDistribution.js
function uniformBigIntDistribution(from, to, rng) {
  if (rng != null) {
    var nextRng = rng.clone();
    return [unsafeUniformBigIntDistribution(from, to, nextRng), nextRng];
  }
  return function (rng2) {
    var nextRng2 = rng2.clone();
    return [unsafeUniformBigIntDistribution(from, to, nextRng2), nextRng2];
  };
}

// ../../../node_modules/pure-rand/lib/esm/distribution/UnsafeUniformIntDistribution.js
var safeNumberMaxSafeInteger = Number.MAX_SAFE_INTEGER;
var sharedA = { sign: 1, data: [0, 0] };
var sharedB = { sign: 1, data: [0, 0] };
var sharedC = { sign: 1, data: [0, 0] };
var sharedData = [0, 0];
function uniformLargeIntInternal(from, to, rangeSize, rng) {
  var rangeSizeArrayIntValue =
    rangeSize <= safeNumberMaxSafeInteger
      ? fromNumberToArrayInt64(sharedC, rangeSize)
      : substractArrayInt64(
          sharedC,
          fromNumberToArrayInt64(sharedA, to),
          fromNumberToArrayInt64(sharedB, from),
        );
  if (rangeSizeArrayIntValue.data[1] === 4294967295) {
    rangeSizeArrayIntValue.data[0] += 1;
    rangeSizeArrayIntValue.data[1] = 0;
  } else {
    rangeSizeArrayIntValue.data[1] += 1;
  }
  unsafeUniformArrayIntDistributionInternal(
    sharedData,
    rangeSizeArrayIntValue.data,
    rng,
  );
  return sharedData[0] * 4294967296 + sharedData[1] + from;
}
function unsafeUniformIntDistribution(from, to, rng) {
  var rangeSize = to - from;
  if (rangeSize <= 4294967295) {
    var g = unsafeUniformIntDistributionInternal(rangeSize + 1, rng);
    return g + from;
  }
  return uniformLargeIntInternal(from, to, rangeSize, rng);
}

// ../../../node_modules/pure-rand/lib/esm/distribution/UniformIntDistribution.js
function uniformIntDistribution(from, to, rng) {
  if (rng != null) {
    var nextRng = rng.clone();
    return [unsafeUniformIntDistribution(from, to, nextRng), nextRng];
  }
  return function (rng2) {
    var nextRng2 = rng2.clone();
    return [unsafeUniformIntDistribution(from, to, nextRng2), nextRng2];
  };
}

// ../../../node_modules/pure-rand/lib/esm/pure-rand-default.js
var __type = 'module';
var __version = '6.1.0';
var __commitHash = 'a413dd2b721516be2ef29adffb515c5ae67bfbad';

// ../../../node_modules/pure-rand/lib/esm/pure-rand.js
var pure_rand_default = pure_rand_default_exports;

// ../../../node_modules/fast-check/lib/esm/check/runner/configuration/VerbosityLevel.js
var VerbosityLevel;
(function (VerbosityLevel2) {
  VerbosityLevel2[(VerbosityLevel2['None'] = 0)] = 'None';
  VerbosityLevel2[(VerbosityLevel2['Verbose'] = 1)] = 'Verbose';
  VerbosityLevel2[(VerbosityLevel2['VeryVerbose'] = 2)] = 'VeryVerbose';
})(VerbosityLevel || (VerbosityLevel = {}));

// ../../../node_modules/fast-check/lib/esm/check/runner/configuration/QualifiedParameters.js
var safeDateNow = Date.now;
var safeMathMin = Math.min;
var safeMathRandom = Math.random;
var QualifiedParameters = class _QualifiedParameters {
  constructor(op) {
    const p = op || {};
    this.seed = _QualifiedParameters.readSeed(p);
    this.randomType = _QualifiedParameters.readRandomType(p);
    this.numRuns = _QualifiedParameters.readNumRuns(p);
    this.verbose = _QualifiedParameters.readVerbose(p);
    this.maxSkipsPerRun = _QualifiedParameters.readOrDefault(
      p,
      'maxSkipsPerRun',
      100,
    );
    this.timeout = _QualifiedParameters.safeTimeout(
      _QualifiedParameters.readOrDefault(p, 'timeout', null),
    );
    this.skipAllAfterTimeLimit = _QualifiedParameters.safeTimeout(
      _QualifiedParameters.readOrDefault(p, 'skipAllAfterTimeLimit', null),
    );
    this.interruptAfterTimeLimit = _QualifiedParameters.safeTimeout(
      _QualifiedParameters.readOrDefault(p, 'interruptAfterTimeLimit', null),
    );
    this.markInterruptAsFailure = _QualifiedParameters.readBoolean(
      p,
      'markInterruptAsFailure',
    );
    this.skipEqualValues = _QualifiedParameters.readBoolean(
      p,
      'skipEqualValues',
    );
    this.ignoreEqualValues = _QualifiedParameters.readBoolean(
      p,
      'ignoreEqualValues',
    );
    this.logger = _QualifiedParameters.readOrDefault(p, 'logger', (v) => {
      console.log(v);
    });
    this.path = _QualifiedParameters.readOrDefault(p, 'path', '');
    this.unbiased = _QualifiedParameters.readBoolean(p, 'unbiased');
    this.examples = _QualifiedParameters.readOrDefault(p, 'examples', []);
    this.endOnFailure = _QualifiedParameters.readBoolean(p, 'endOnFailure');
    this.reporter = _QualifiedParameters.readOrDefault(p, 'reporter', null);
    this.asyncReporter = _QualifiedParameters.readOrDefault(
      p,
      'asyncReporter',
      null,
    );
    this.errorWithCause = _QualifiedParameters.readBoolean(p, 'errorWithCause');
  }
  toParameters() {
    const orUndefined = (value) => (value !== null ? value : void 0);
    const parameters = {
      seed: this.seed,
      randomType: this.randomType,
      numRuns: this.numRuns,
      maxSkipsPerRun: this.maxSkipsPerRun,
      timeout: orUndefined(this.timeout),
      skipAllAfterTimeLimit: orUndefined(this.skipAllAfterTimeLimit),
      interruptAfterTimeLimit: orUndefined(this.interruptAfterTimeLimit),
      markInterruptAsFailure: this.markInterruptAsFailure,
      skipEqualValues: this.skipEqualValues,
      ignoreEqualValues: this.ignoreEqualValues,
      path: this.path,
      logger: this.logger,
      unbiased: this.unbiased,
      verbose: this.verbose,
      examples: this.examples,
      endOnFailure: this.endOnFailure,
      reporter: orUndefined(this.reporter),
      asyncReporter: orUndefined(this.asyncReporter),
      errorWithCause: this.errorWithCause,
    };
    return parameters;
  }
  static read(op) {
    return new _QualifiedParameters(op);
  }
};
QualifiedParameters.createQualifiedRandomGenerator = (random) => {
  return (seed) => {
    const rng = random(seed);
    if (rng.unsafeJump === void 0) {
      rng.unsafeJump = () => unsafeSkipN(rng, 42);
    }
    return rng;
  };
};
QualifiedParameters.readSeed = (p) => {
  if (p.seed == null) return safeDateNow() ^ (safeMathRandom() * 4294967296);
  const seed32 = p.seed | 0;
  if (p.seed === seed32) return seed32;
  const gap = p.seed - seed32;
  return seed32 ^ (gap * 4294967296);
};
QualifiedParameters.readRandomType = (p) => {
  if (p.randomType == null) return pure_rand_default.xorshift128plus;
  if (typeof p.randomType === 'string') {
    switch (p.randomType) {
      case 'mersenne':
        return QualifiedParameters.createQualifiedRandomGenerator(
          pure_rand_default.mersenne,
        );
      case 'congruential':
      case 'congruential32':
        return QualifiedParameters.createQualifiedRandomGenerator(
          pure_rand_default.congruential32,
        );
      case 'xorshift128plus':
        return pure_rand_default.xorshift128plus;
      case 'xoroshiro128plus':
        return pure_rand_default.xoroshiro128plus;
      default:
        throw new Error(`Invalid random specified: '${p.randomType}'`);
    }
  }
  const mrng = p.randomType(0);
  if ('min' in mrng && mrng.min !== -2147483648) {
    throw new Error(
      `Invalid random number generator: min must equal -0x80000000, got ${String(mrng.min)}`,
    );
  }
  if ('max' in mrng && mrng.max !== 2147483647) {
    throw new Error(
      `Invalid random number generator: max must equal 0x7fffffff, got ${String(mrng.max)}`,
    );
  }
  if ('unsafeJump' in mrng) {
    return p.randomType;
  }
  return QualifiedParameters.createQualifiedRandomGenerator(p.randomType);
};
QualifiedParameters.readNumRuns = (p) => {
  const defaultValue = 100;
  if (p.numRuns != null) return p.numRuns;
  if (p.num_runs != null) return p.num_runs;
  return defaultValue;
};
QualifiedParameters.readVerbose = (p) => {
  if (p.verbose == null) return VerbosityLevel.None;
  if (typeof p.verbose === 'boolean') {
    return p.verbose === true ? VerbosityLevel.Verbose : VerbosityLevel.None;
  }
  if (p.verbose <= VerbosityLevel.None) {
    return VerbosityLevel.None;
  }
  if (p.verbose >= VerbosityLevel.VeryVerbose) {
    return VerbosityLevel.VeryVerbose;
  }
  return p.verbose | 0;
};
QualifiedParameters.readBoolean = (p, key) => p[key] === true;
QualifiedParameters.readOrDefault = (p, key, defaultValue) => {
  const value = p[key];
  return value != null ? value : defaultValue;
};
QualifiedParameters.safeTimeout = (value) => {
  if (value === null) {
    return null;
  }
  return safeMathMin(value, 2147483647);
};

// ../../../node_modules/fast-check/lib/esm/check/property/SkipAfterProperty.js
function interruptAfter(timeMs, setTimeoutSafe, clearTimeoutSafe) {
  let timeoutHandle = null;
  const promise = new Promise((resolve) => {
    timeoutHandle = setTimeoutSafe(() => {
      const preconditionFailure = new PreconditionFailure(true);
      resolve(preconditionFailure);
    }, timeMs);
  });
  return {
    clear: () => clearTimeoutSafe(timeoutHandle),
    promise,
  };
}
var SkipAfterProperty = class {
  constructor(
    property2,
    getTime,
    timeLimit,
    interruptExecution,
    setTimeoutSafe,
    clearTimeoutSafe,
  ) {
    this.property = property2;
    this.getTime = getTime;
    this.interruptExecution = interruptExecution;
    this.setTimeoutSafe = setTimeoutSafe;
    this.clearTimeoutSafe = clearTimeoutSafe;
    this.skipAfterTime = this.getTime() + timeLimit;
    if (
      this.property.runBeforeEach !== void 0 &&
      this.property.runAfterEach !== void 0
    ) {
      this.runBeforeEach = () => this.property.runBeforeEach();
      this.runAfterEach = () => this.property.runAfterEach();
    }
  }
  isAsync() {
    return this.property.isAsync();
  }
  generate(mrng, runId) {
    return this.property.generate(mrng, runId);
  }
  shrink(value) {
    return this.property.shrink(value);
  }
  run(v, dontRunHook) {
    const remainingTime = this.skipAfterTime - this.getTime();
    if (remainingTime <= 0) {
      const preconditionFailure = new PreconditionFailure(
        this.interruptExecution,
      );
      if (this.isAsync()) {
        return Promise.resolve(preconditionFailure);
      } else {
        return preconditionFailure;
      }
    }
    if (this.interruptExecution && this.isAsync()) {
      const t = interruptAfter(
        remainingTime,
        this.setTimeoutSafe,
        this.clearTimeoutSafe,
      );
      const propRun = Promise.race([
        this.property.run(v, dontRunHook),
        t.promise,
      ]);
      propRun.then(t.clear, t.clear);
      return propRun;
    }
    return this.property.run(v, dontRunHook);
  }
};

// ../../../node_modules/fast-check/lib/esm/check/property/TimeoutProperty.js
var timeoutAfter = (timeMs, setTimeoutSafe, clearTimeoutSafe) => {
  let timeoutHandle = null;
  const promise = new Promise((resolve) => {
    timeoutHandle = setTimeoutSafe(() => {
      resolve({
        error: new SError(
          `Property timeout: exceeded limit of ${timeMs} milliseconds`,
        ),
        errorMessage: `Property timeout: exceeded limit of ${timeMs} milliseconds`,
      });
    }, timeMs);
  });
  return {
    clear: () => clearTimeoutSafe(timeoutHandle),
    promise,
  };
};
var TimeoutProperty = class {
  constructor(property2, timeMs, setTimeoutSafe, clearTimeoutSafe) {
    this.property = property2;
    this.timeMs = timeMs;
    this.setTimeoutSafe = setTimeoutSafe;
    this.clearTimeoutSafe = clearTimeoutSafe;
    if (
      this.property.runBeforeEach !== void 0 &&
      this.property.runAfterEach !== void 0
    ) {
      this.runBeforeEach = () => Promise.resolve(this.property.runBeforeEach());
      this.runAfterEach = () => Promise.resolve(this.property.runAfterEach());
    }
  }
  isAsync() {
    return true;
  }
  generate(mrng, runId) {
    return this.property.generate(mrng, runId);
  }
  shrink(value) {
    return this.property.shrink(value);
  }
  async run(v, dontRunHook) {
    const t = timeoutAfter(
      this.timeMs,
      this.setTimeoutSafe,
      this.clearTimeoutSafe,
    );
    const propRun = Promise.race([
      this.property.run(v, dontRunHook),
      t.promise,
    ]);
    propRun.then(t.clear, t.clear);
    return propRun;
  }
};

// ../../../node_modules/fast-check/lib/esm/check/property/UnbiasedProperty.js
var UnbiasedProperty = class {
  constructor(property2) {
    this.property = property2;
    if (
      this.property.runBeforeEach !== void 0 &&
      this.property.runAfterEach !== void 0
    ) {
      this.runBeforeEach = () => this.property.runBeforeEach();
      this.runAfterEach = () => this.property.runAfterEach();
    }
  }
  isAsync() {
    return this.property.isAsync();
  }
  generate(mrng, _runId) {
    return this.property.generate(mrng, void 0);
  }
  shrink(value) {
    return this.property.shrink(value);
  }
  run(v, dontRunHook) {
    return this.property.run(v, dontRunHook);
  }
};

// ../../../node_modules/fast-check/lib/esm/utils/stringify.js
var safeArrayFrom = Array.from;
var safeBufferIsBuffer =
  typeof Buffer !== 'undefined' ? Buffer.isBuffer : void 0;
var safeJsonStringify = JSON.stringify;
var safeNumberIsNaN = Number.isNaN;
var safeObjectKeys = Object.keys;
var safeObjectGetOwnPropertySymbols = Object.getOwnPropertySymbols;
var safeObjectGetOwnPropertyDescriptor = Object.getOwnPropertyDescriptor;
var safeObjectGetPrototypeOf = Object.getPrototypeOf;
var safeNegativeInfinity = Number.NEGATIVE_INFINITY;
var safePositiveInfinity = Number.POSITIVE_INFINITY;
var toStringMethod = Symbol('fast-check/toStringMethod');
function hasToStringMethod(instance) {
  return (
    instance !== null &&
    (typeof instance === 'object' || typeof instance === 'function') &&
    toStringMethod in instance &&
    typeof instance[toStringMethod] === 'function'
  );
}
var asyncToStringMethod = Symbol('fast-check/asyncToStringMethod');
function hasAsyncToStringMethod(instance) {
  return (
    instance !== null &&
    (typeof instance === 'object' || typeof instance === 'function') &&
    asyncToStringMethod in instance &&
    typeof instance[asyncToStringMethod] === 'function'
  );
}
var findSymbolNameRegex = /^Symbol\((.*)\)$/;
function getSymbolDescription(s) {
  if (s.description !== void 0) return s.description;
  const m = findSymbolNameRegex.exec(SString(s));
  return m && m[1].length ? m[1] : null;
}
function stringifyNumber(numValue) {
  switch (numValue) {
    case 0:
      return 1 / numValue === safeNegativeInfinity ? '-0' : '0';
    case safeNegativeInfinity:
      return 'Number.NEGATIVE_INFINITY';
    case safePositiveInfinity:
      return 'Number.POSITIVE_INFINITY';
    default:
      return numValue === numValue ? SString(numValue) : 'Number.NaN';
  }
}
function isSparseArray(arr) {
  let previousNumberedIndex = -1;
  for (const index in arr) {
    const numberedIndex = Number(index);
    if (numberedIndex !== previousNumberedIndex + 1) return true;
    previousNumberedIndex = numberedIndex;
  }
  return previousNumberedIndex + 1 !== arr.length;
}
function stringifyInternal(value, previousValues, getAsyncContent) {
  const currentValues = [...previousValues, value];
  if (typeof value === 'object') {
    if (safeIndexOf(previousValues, value) !== -1) {
      return '[cyclic]';
    }
  }
  if (hasAsyncToStringMethod(value)) {
    const content = getAsyncContent(value);
    if (content.state === 'fulfilled') {
      return content.value;
    }
  }
  if (hasToStringMethod(value)) {
    try {
      return value[toStringMethod]();
    } catch (err) {}
  }
  switch (safeToString(value)) {
    case '[object Array]': {
      const arr = value;
      if (arr.length >= 50 && isSparseArray(arr)) {
        const assignments = [];
        for (const index in arr) {
          if (!safeNumberIsNaN(Number(index)))
            safePush(
              assignments,
              `${index}:${stringifyInternal(arr[index], currentValues, getAsyncContent)}`,
            );
        }
        return assignments.length !== 0
          ? `Object.assign(Array(${arr.length}),{${safeJoin(assignments, ',')}})`
          : `Array(${arr.length})`;
      }
      const stringifiedArray = safeJoin(
        safeMap(arr, (v) =>
          stringifyInternal(v, currentValues, getAsyncContent),
        ),
        ',',
      );
      return arr.length === 0 || arr.length - 1 in arr
        ? `[${stringifiedArray}]`
        : `[${stringifiedArray},]`;
    }
    case '[object BigInt]':
      return `${value}n`;
    case '[object Boolean]': {
      const unboxedToString = value == true ? 'true' : 'false';
      return typeof value === 'boolean'
        ? unboxedToString
        : `new Boolean(${unboxedToString})`;
    }
    case '[object Date]': {
      const d = value;
      return safeNumberIsNaN(safeGetTime(d))
        ? `new Date(NaN)`
        : `new Date(${safeJsonStringify(safeToISOString(d))})`;
    }
    case '[object Map]':
      return `new Map(${stringifyInternal(Array.from(value), currentValues, getAsyncContent)})`;
    case '[object Null]':
      return `null`;
    case '[object Number]':
      return typeof value === 'number'
        ? stringifyNumber(value)
        : `new Number(${stringifyNumber(Number(value))})`;
    case '[object Object]': {
      try {
        const toStringAccessor = value.toString;
        if (
          typeof toStringAccessor === 'function' &&
          toStringAccessor !== Object.prototype.toString
        ) {
          return value.toString();
        }
      } catch (err) {
        return '[object Object]';
      }
      const mapper = (k) =>
        `${k === '__proto__' ? '["__proto__"]' : typeof k === 'symbol' ? `[${stringifyInternal(k, currentValues, getAsyncContent)}]` : safeJsonStringify(k)}:${stringifyInternal(value[k], currentValues, getAsyncContent)}`;
      const stringifiedProperties = [
        ...safeMap(safeObjectKeys(value), mapper),
        ...safeMap(
          safeFilter(safeObjectGetOwnPropertySymbols(value), (s) => {
            const descriptor = safeObjectGetOwnPropertyDescriptor(value, s);
            return descriptor && descriptor.enumerable;
          }),
          mapper,
        ),
      ];
      const rawRepr = '{' + safeJoin(stringifiedProperties, ',') + '}';
      if (safeObjectGetPrototypeOf(value) === null) {
        return rawRepr === '{}'
          ? 'Object.create(null)'
          : `Object.assign(Object.create(null),${rawRepr})`;
      }
      return rawRepr;
    }
    case '[object Set]':
      return `new Set(${stringifyInternal(Array.from(value), currentValues, getAsyncContent)})`;
    case '[object String]':
      return typeof value === 'string'
        ? safeJsonStringify(value)
        : `new String(${safeJsonStringify(value)})`;
    case '[object Symbol]': {
      const s = value;
      if (Symbol.keyFor(s) !== void 0) {
        return `Symbol.for(${safeJsonStringify(Symbol.keyFor(s))})`;
      }
      const desc = getSymbolDescription(s);
      if (desc === null) {
        return 'Symbol()';
      }
      const knownSymbol =
        desc.startsWith('Symbol.') && Symbol[desc.substring(7)];
      return s === knownSymbol ? desc : `Symbol(${safeJsonStringify(desc)})`;
    }
    case '[object Promise]': {
      const promiseContent = getAsyncContent(value);
      switch (promiseContent.state) {
        case 'fulfilled':
          return `Promise.resolve(${stringifyInternal(promiseContent.value, currentValues, getAsyncContent)})`;
        case 'rejected':
          return `Promise.reject(${stringifyInternal(promiseContent.value, currentValues, getAsyncContent)})`;
        case 'pending':
          return `new Promise(() => {/*pending*/})`;
        case 'unknown':
        default:
          return `new Promise(() => {/*unknown*/})`;
      }
    }
    case '[object Error]':
      if (value instanceof Error) {
        return `new Error(${stringifyInternal(value.message, currentValues, getAsyncContent)})`;
      }
      break;
    case '[object Undefined]':
      return `undefined`;
    case '[object Int8Array]':
    case '[object Uint8Array]':
    case '[object Uint8ClampedArray]':
    case '[object Int16Array]':
    case '[object Uint16Array]':
    case '[object Int32Array]':
    case '[object Uint32Array]':
    case '[object Float32Array]':
    case '[object Float64Array]':
    case '[object BigInt64Array]':
    case '[object BigUint64Array]': {
      if (
        typeof safeBufferIsBuffer === 'function' &&
        safeBufferIsBuffer(value)
      ) {
        return `Buffer.from(${stringifyInternal(safeArrayFrom(value.values()), currentValues, getAsyncContent)})`;
      }
      const valuePrototype = safeObjectGetPrototypeOf(value);
      const className =
        valuePrototype &&
        valuePrototype.constructor &&
        valuePrototype.constructor.name;
      if (typeof className === 'string') {
        const typedArray2 = value;
        const valuesFromTypedArr = typedArray2.values();
        return `${className}.from(${stringifyInternal(safeArrayFrom(valuesFromTypedArr), currentValues, getAsyncContent)})`;
      }
      break;
    }
  }
  try {
    return value.toString();
  } catch (_a) {
    return safeToString(value);
  }
}
function stringify(value) {
  return stringifyInternal(value, [], () => ({
    state: 'unknown',
    value: void 0,
  }));
}
function possiblyAsyncStringify(value) {
  const stillPendingMarker = Symbol();
  const pendingPromisesForCache = [];
  const cache = /* @__PURE__ */ new Map();
  function createDelay0() {
    let handleId = null;
    const cancel = () => {
      if (handleId !== null) {
        clearTimeout(handleId);
      }
    };
    const delay = new Promise((resolve) => {
      handleId = setTimeout(() => {
        handleId = null;
        resolve(stillPendingMarker);
      }, 0);
    });
    return { delay, cancel };
  }
  const unknownState = { state: 'unknown', value: void 0 };
  const getAsyncContent = function getAsyncContent2(data) {
    const cacheKey = data;
    if (cache.has(cacheKey)) {
      return cache.get(cacheKey);
    }
    const delay0 = createDelay0();
    const p =
      asyncToStringMethod in data
        ? Promise.resolve().then(() => data[asyncToStringMethod]())
        : data;
    p.catch(() => {});
    pendingPromisesForCache.push(
      Promise.race([p, delay0.delay]).then(
        (successValue) => {
          if (successValue === stillPendingMarker)
            cache.set(cacheKey, { state: 'pending', value: void 0 });
          else cache.set(cacheKey, { state: 'fulfilled', value: successValue });
          delay0.cancel();
        },
        (errorValue) => {
          cache.set(cacheKey, { state: 'rejected', value: errorValue });
          delay0.cancel();
        },
      ),
    );
    cache.set(cacheKey, unknownState);
    return unknownState;
  };
  function loop() {
    const stringifiedValue = stringifyInternal(value, [], getAsyncContent);
    if (pendingPromisesForCache.length === 0) {
      return stringifiedValue;
    }
    return Promise.all(pendingPromisesForCache.splice(0)).then(loop);
  }
  return loop();
}
async function asyncStringify(value) {
  return Promise.resolve(possiblyAsyncStringify(value));
}

// ../../../node_modules/fast-check/lib/esm/check/property/IgnoreEqualValuesProperty.js
function fromSyncCached(cachedValue) {
  return cachedValue === null ? new PreconditionFailure() : cachedValue;
}
function fromCached(...data) {
  if (data[1]) return data[0].then(fromSyncCached);
  return fromSyncCached(data[0]);
}
function fromCachedUnsafe(cachedValue, isAsync) {
  return fromCached(cachedValue, isAsync);
}
var IgnoreEqualValuesProperty = class {
  constructor(property2, skipRuns) {
    this.property = property2;
    this.skipRuns = skipRuns;
    this.coveredCases = /* @__PURE__ */ new Map();
    if (
      this.property.runBeforeEach !== void 0 &&
      this.property.runAfterEach !== void 0
    ) {
      this.runBeforeEach = () => this.property.runBeforeEach();
      this.runAfterEach = () => this.property.runAfterEach();
    }
  }
  isAsync() {
    return this.property.isAsync();
  }
  generate(mrng, runId) {
    return this.property.generate(mrng, runId);
  }
  shrink(value) {
    return this.property.shrink(value);
  }
  run(v, dontRunHook) {
    const stringifiedValue = stringify(v);
    if (this.coveredCases.has(stringifiedValue)) {
      const lastOutput = this.coveredCases.get(stringifiedValue);
      if (!this.skipRuns) {
        return lastOutput;
      }
      return fromCachedUnsafe(lastOutput, this.property.isAsync());
    }
    const out = this.property.run(v, dontRunHook);
    this.coveredCases.set(stringifiedValue, out);
    return out;
  }
};

// ../../../node_modules/fast-check/lib/esm/check/runner/DecorateProperty.js
var safeDateNow2 = Date.now;
var safeSetTimeout = setTimeout;
var safeClearTimeout = clearTimeout;
function decorateProperty(rawProperty, qParams) {
  let prop = rawProperty;
  if (rawProperty.isAsync() && qParams.timeout != null) {
    prop = new TimeoutProperty(
      prop,
      qParams.timeout,
      safeSetTimeout,
      safeClearTimeout,
    );
  }
  if (qParams.unbiased) {
    prop = new UnbiasedProperty(prop);
  }
  if (qParams.skipAllAfterTimeLimit != null) {
    prop = new SkipAfterProperty(
      prop,
      safeDateNow2,
      qParams.skipAllAfterTimeLimit,
      false,
      safeSetTimeout,
      safeClearTimeout,
    );
  }
  if (qParams.interruptAfterTimeLimit != null) {
    prop = new SkipAfterProperty(
      prop,
      safeDateNow2,
      qParams.interruptAfterTimeLimit,
      true,
      safeSetTimeout,
      safeClearTimeout,
    );
  }
  if (qParams.skipEqualValues) {
    prop = new IgnoreEqualValuesProperty(prop, true);
  }
  if (qParams.ignoreEqualValues) {
    prop = new IgnoreEqualValuesProperty(prop, false);
  }
  return prop;
}

// ../../../node_modules/fast-check/lib/esm/check/runner/reporter/ExecutionStatus.js
var ExecutionStatus;
(function (ExecutionStatus2) {
  ExecutionStatus2[(ExecutionStatus2['Success'] = 0)] = 'Success';
  ExecutionStatus2[(ExecutionStatus2['Skipped'] = -1)] = 'Skipped';
  ExecutionStatus2[(ExecutionStatus2['Failure'] = 1)] = 'Failure';
})(ExecutionStatus || (ExecutionStatus = {}));

// ../../../node_modules/fast-check/lib/esm/check/runner/reporter/RunExecution.js
var RunExecution = class _RunExecution {
  constructor(verbosity, interruptedAsFailure) {
    this.verbosity = verbosity;
    this.interruptedAsFailure = interruptedAsFailure;
    this.isSuccess = () => this.pathToFailure == null;
    this.firstFailure = () =>
      this.pathToFailure ? +safeSplit(this.pathToFailure, ':')[0] : -1;
    this.numShrinks = () =>
      this.pathToFailure ? safeSplit(this.pathToFailure, ':').length - 1 : 0;
    this.rootExecutionTrees = [];
    this.currentLevelExecutionTrees = this.rootExecutionTrees;
    this.failure = null;
    this.numSkips = 0;
    this.numSuccesses = 0;
    this.interrupted = false;
  }
  appendExecutionTree(status, value) {
    const currentTree = { status, value, children: [] };
    this.currentLevelExecutionTrees.push(currentTree);
    return currentTree;
  }
  fail(value, id, failure) {
    if (this.verbosity >= VerbosityLevel.Verbose) {
      const currentTree = this.appendExecutionTree(
        ExecutionStatus.Failure,
        value,
      );
      this.currentLevelExecutionTrees = currentTree.children;
    }
    if (this.pathToFailure == null) this.pathToFailure = `${id}`;
    else this.pathToFailure += `:${id}`;
    this.value = value;
    this.failure = failure;
  }
  skip(value) {
    if (this.verbosity >= VerbosityLevel.VeryVerbose) {
      this.appendExecutionTree(ExecutionStatus.Skipped, value);
    }
    if (this.pathToFailure == null) {
      ++this.numSkips;
    }
  }
  success(value) {
    if (this.verbosity >= VerbosityLevel.VeryVerbose) {
      this.appendExecutionTree(ExecutionStatus.Success, value);
    }
    if (this.pathToFailure == null) {
      ++this.numSuccesses;
    }
  }
  interrupt() {
    this.interrupted = true;
  }
  extractFailures() {
    if (this.isSuccess()) {
      return [];
    }
    const failures = [];
    let cursor = this.rootExecutionTrees;
    while (
      cursor.length > 0 &&
      cursor[cursor.length - 1].status === ExecutionStatus.Failure
    ) {
      const failureTree = cursor[cursor.length - 1];
      failures.push(failureTree.value);
      cursor = failureTree.children;
    }
    return failures;
  }
  toRunDetails(seed, basePath, maxSkips, qParams) {
    if (!this.isSuccess()) {
      return {
        failed: true,
        interrupted: this.interrupted,
        numRuns: this.firstFailure() + 1 - this.numSkips,
        numSkips: this.numSkips,
        numShrinks: this.numShrinks(),
        seed,
        counterexample: this.value,
        counterexamplePath: _RunExecution.mergePaths(
          basePath,
          this.pathToFailure,
        ),
        error: this.failure.errorMessage,
        errorInstance: this.failure.error,
        failures: this.extractFailures(),
        executionSummary: this.rootExecutionTrees,
        verbose: this.verbosity,
        runConfiguration: qParams.toParameters(),
      };
    }
    const considerInterruptedAsFailure =
      this.interruptedAsFailure || this.numSuccesses === 0;
    const failed =
      this.numSkips > maxSkips ||
      (this.interrupted && considerInterruptedAsFailure);
    const out = {
      failed,
      interrupted: this.interrupted,
      numRuns: this.numSuccesses,
      numSkips: this.numSkips,
      numShrinks: 0,
      seed,
      counterexample: null,
      counterexamplePath: null,
      error: null,
      errorInstance: null,
      failures: [],
      executionSummary: this.rootExecutionTrees,
      verbose: this.verbosity,
      runConfiguration: qParams.toParameters(),
    };
    return out;
  }
};
RunExecution.mergePaths = (offsetPath, path) => {
  if (offsetPath.length === 0) return path;
  const offsetItems = offsetPath.split(':');
  const remainingItems = path.split(':');
  const middle = +offsetItems[offsetItems.length - 1] + +remainingItems[0];
  return [
    ...offsetItems.slice(0, offsetItems.length - 1),
    `${middle}`,
    ...remainingItems.slice(1),
  ].join(':');
};

// ../../../node_modules/fast-check/lib/esm/check/runner/RunnerIterator.js
var RunnerIterator = class {
  constructor(sourceValues, shrink, verbose, interruptedAsFailure) {
    this.sourceValues = sourceValues;
    this.shrink = shrink;
    this.runExecution = new RunExecution(verbose, interruptedAsFailure);
    this.currentIdx = -1;
    this.nextValues = sourceValues;
  }
  [Symbol.iterator]() {
    return this;
  }
  next() {
    const nextValue = this.nextValues.next();
    if (nextValue.done || this.runExecution.interrupted) {
      return { done: true, value: void 0 };
    }
    this.currentValue = nextValue.value;
    ++this.currentIdx;
    return { done: false, value: nextValue.value.value_ };
  }
  handleResult(result) {
    if (
      result != null &&
      typeof result === 'object' &&
      !PreconditionFailure.isFailure(result)
    ) {
      this.runExecution.fail(this.currentValue.value_, this.currentIdx, result);
      this.currentIdx = -1;
      this.nextValues = this.shrink(this.currentValue);
    } else if (result != null) {
      if (!result.interruptExecution) {
        this.runExecution.skip(this.currentValue.value_);
        this.sourceValues.skippedOne();
      } else {
        this.runExecution.interrupt();
      }
    } else {
      this.runExecution.success(this.currentValue.value_);
    }
  }
};

// ../../../node_modules/fast-check/lib/esm/check/runner/SourceValuesIterator.js
var SourceValuesIterator = class {
  constructor(initialValues, maxInitialIterations, remainingSkips) {
    this.initialValues = initialValues;
    this.maxInitialIterations = maxInitialIterations;
    this.remainingSkips = remainingSkips;
  }
  [Symbol.iterator]() {
    return this;
  }
  next() {
    if (--this.maxInitialIterations !== -1 && this.remainingSkips >= 0) {
      const n = this.initialValues.next();
      if (!n.done) return { value: n.value, done: false };
    }
    return { value: void 0, done: true };
  }
  skippedOne() {
    --this.remainingSkips;
    ++this.maxInitialIterations;
  }
};

// ../../../node_modules/fast-check/lib/esm/random/generator/Random.js
var Random = class _Random {
  constructor(sourceRng) {
    this.internalRng = sourceRng.clone();
  }
  clone() {
    return new _Random(this.internalRng);
  }
  next(bits) {
    return unsafeUniformIntDistribution(0, (1 << bits) - 1, this.internalRng);
  }
  nextBoolean() {
    return unsafeUniformIntDistribution(0, 1, this.internalRng) == 1;
  }
  nextInt(min, max) {
    return unsafeUniformIntDistribution(
      min == null ? _Random.MIN_INT : min,
      max == null ? _Random.MAX_INT : max,
      this.internalRng,
    );
  }
  nextBigInt(min, max) {
    return unsafeUniformBigIntDistribution(min, max, this.internalRng);
  }
  nextArrayInt(min, max) {
    return unsafeUniformArrayIntDistribution(min, max, this.internalRng);
  }
  nextDouble() {
    const a = this.next(26);
    const b = this.next(27);
    return (a * _Random.DBL_FACTOR + b) * _Random.DBL_DIVISOR;
  }
  getState() {
    if (
      'getState' in this.internalRng &&
      typeof this.internalRng.getState === 'function'
    ) {
      return this.internalRng.getState();
    }
    return void 0;
  }
};
Random.MIN_INT = 2147483648 | 0;
Random.MAX_INT = 2147483647 | 0;
Random.DBL_FACTOR = Math.pow(2, 27);
Random.DBL_DIVISOR = Math.pow(2, -53);

// ../../../node_modules/fast-check/lib/esm/check/runner/Tosser.js
function tossNext(generator, rng, index) {
  rng.unsafeJump();
  return generator.generate(new Random(rng), index);
}
function* toss(generator, seed, random, examples) {
  for (let idx = 0; idx !== examples.length; ++idx) {
    yield new Value(examples[idx], void 0);
  }
  for (let idx = 0, rng = random(seed); ; ++idx) {
    yield tossNext(generator, rng, idx);
  }
}
function lazyGenerate(generator, rng, idx) {
  return () => generator.generate(new Random(rng), idx);
}
function* lazyToss(generator, seed, random, examples) {
  yield* safeMap(examples, (e) => () => new Value(e, void 0));
  let idx = 0;
  let rng = random(seed);
  for (;;) {
    rng = rng.jump ? rng.jump() : skipN(rng, 42);
    yield lazyGenerate(generator, rng, idx++);
  }
}

// ../../../node_modules/fast-check/lib/esm/check/runner/utils/PathWalker.js
function produce(producer) {
  return producer();
}
function pathWalk(path, initialProducers, shrink) {
  const producers = initialProducers;
  const segments = path.split(':').map((text) => +text);
  if (segments.length === 0) {
    return producers.map(produce);
  }
  if (!segments.every((v) => !Number.isNaN(v))) {
    throw new Error(`Unable to replay, got invalid path=${path}`);
  }
  let values = producers.drop(segments[0]).map(produce);
  for (const s of segments.slice(1)) {
    const valueToShrink = values.getNthOrLast(0);
    if (valueToShrink === null) {
      throw new Error(`Unable to replay, got wrong path=${path}`);
    }
    values = shrink(valueToShrink).drop(s);
  }
  return values;
}

// ../../../node_modules/fast-check/lib/esm/check/runner/utils/RunDetailsFormatter.js
var safeObjectAssign2 = Object.assign;
function formatHints(hints) {
  if (hints.length === 1) {
    return `Hint: ${hints[0]}`;
  }
  return hints.map((h2, idx) => `Hint (${idx + 1}): ${h2}`).join('\n');
}
function formatFailures(failures, stringifyOne) {
  return `Encountered failures were:
- ${failures.map(stringifyOne).join('\n- ')}`;
}
function formatExecutionSummary(executionTrees, stringifyOne) {
  const summaryLines = [];
  const remainingTreesAndDepth = [];
  for (const tree of executionTrees.slice().reverse()) {
    remainingTreesAndDepth.push({ depth: 1, tree });
  }
  while (remainingTreesAndDepth.length !== 0) {
    const currentTreeAndDepth = remainingTreesAndDepth.pop();
    const currentTree = currentTreeAndDepth.tree;
    const currentDepth = currentTreeAndDepth.depth;
    const statusIcon =
      currentTree.status === ExecutionStatus.Success
        ? '\x1B[32m\u221A\x1B[0m'
        : currentTree.status === ExecutionStatus.Failure
          ? '\x1B[31m\xD7\x1B[0m'
          : '\x1B[33m!\x1B[0m';
    const leftPadding = Array(currentDepth).join('. ');
    summaryLines.push(
      `${leftPadding}${statusIcon} ${stringifyOne(currentTree.value)}`,
    );
    for (const tree of currentTree.children.slice().reverse()) {
      remainingTreesAndDepth.push({ depth: currentDepth + 1, tree });
    }
  }
  return `Execution summary:
${summaryLines.join('\n')}`;
}
function preFormatTooManySkipped(out, stringifyOne) {
  const message = `Failed to run property, too many pre-condition failures encountered
{ seed: ${out.seed} }

Ran ${out.numRuns} time(s)
Skipped ${out.numSkips} time(s)`;
  let details = null;
  const hints = [
    'Try to reduce the number of rejected values by combining map, flatMap and built-in arbitraries',
    'Increase failure tolerance by setting maxSkipsPerRun to an higher value',
  ];
  if (out.verbose >= VerbosityLevel.VeryVerbose) {
    details = formatExecutionSummary(out.executionSummary, stringifyOne);
  } else {
    safePush(
      hints,
      'Enable verbose mode at level VeryVerbose in order to check all generated values and their associated status',
    );
  }
  return { message, details, hints };
}
function preFormatFailure(out, stringifyOne) {
  const noErrorInMessage = out.runConfiguration.errorWithCause;
  const messageErrorPart = noErrorInMessage
    ? ''
    : `
Got ${safeReplace(out.error, /^Error: /, 'error: ')}`;
  const message = `Property failed after ${out.numRuns} tests
{ seed: ${out.seed}, path: "${out.counterexamplePath}", endOnFailure: true }
Counterexample: ${stringifyOne(out.counterexample)}
Shrunk ${out.numShrinks} time(s)${messageErrorPart}`;
  let details = null;
  const hints = [];
  if (out.verbose >= VerbosityLevel.VeryVerbose) {
    details = formatExecutionSummary(out.executionSummary, stringifyOne);
  } else if (out.verbose === VerbosityLevel.Verbose) {
    details = formatFailures(out.failures, stringifyOne);
  } else {
    safePush(
      hints,
      'Enable verbose mode in order to have the list of all failing values encountered during the run',
    );
  }
  return { message, details, hints };
}
function preFormatEarlyInterrupted(out, stringifyOne) {
  const message = `Property interrupted after ${out.numRuns} tests
{ seed: ${out.seed} }`;
  let details = null;
  const hints = [];
  if (out.verbose >= VerbosityLevel.VeryVerbose) {
    details = formatExecutionSummary(out.executionSummary, stringifyOne);
  } else {
    safePush(
      hints,
      'Enable verbose mode at level VeryVerbose in order to check all generated values and their associated status',
    );
  }
  return { message, details, hints };
}
function defaultReportMessageInternal(out, stringifyOne) {
  if (!out.failed) return;
  const { message, details, hints } =
    out.counterexamplePath === null
      ? out.interrupted
        ? preFormatEarlyInterrupted(out, stringifyOne)
        : preFormatTooManySkipped(out, stringifyOne)
      : preFormatFailure(out, stringifyOne);
  let errorMessage = message;
  if (details != null)
    errorMessage += `

${details}`;
  if (hints.length > 0)
    errorMessage += `

${formatHints(hints)}`;
  return errorMessage;
}
function defaultReportMessage(out) {
  return defaultReportMessageInternal(out, stringify);
}
async function asyncDefaultReportMessage(out) {
  const pendingStringifieds = [];
  function stringifyOne(value) {
    const stringified = possiblyAsyncStringify(value);
    if (typeof stringified === 'string') {
      return stringified;
    }
    pendingStringifieds.push(Promise.all([value, stringified]));
    return '\u2026';
  }
  const firstTryMessage = defaultReportMessageInternal(out, stringifyOne);
  if (pendingStringifieds.length === 0) {
    return firstTryMessage;
  }
  const registeredValues = new Map(await Promise.all(pendingStringifieds));
  function stringifySecond(value) {
    const asyncStringifiedIfRegistered = registeredValues.get(value);
    if (asyncStringifiedIfRegistered !== void 0) {
      return asyncStringifiedIfRegistered;
    }
    return stringify(value);
  }
  return defaultReportMessageInternal(out, stringifySecond);
}
function buildError(errorMessage, out) {
  if (!out.runConfiguration.errorWithCause) {
    throw new SError(errorMessage);
  }
  const ErrorWithCause = SError;
  const error = new ErrorWithCause(errorMessage, { cause: out.errorInstance });
  if (!('cause' in error)) {
    safeObjectAssign2(error, { cause: out.errorInstance });
  }
  return error;
}
function throwIfFailed(out) {
  if (!out.failed) return;
  throw buildError(defaultReportMessage(out), out);
}
async function asyncThrowIfFailed(out) {
  if (!out.failed) return;
  throw buildError(await asyncDefaultReportMessage(out), out);
}
function reportRunDetails(out) {
  if (out.runConfiguration.asyncReporter)
    return out.runConfiguration.asyncReporter(out);
  else if (out.runConfiguration.reporter)
    return out.runConfiguration.reporter(out);
  else return throwIfFailed(out);
}
async function asyncReportRunDetails(out) {
  if (out.runConfiguration.asyncReporter)
    return out.runConfiguration.asyncReporter(out);
  else if (out.runConfiguration.reporter)
    return out.runConfiguration.reporter(out);
  else return asyncThrowIfFailed(out);
}

// ../../../node_modules/fast-check/lib/esm/check/runner/Runner.js
var safeObjectAssign3 = Object.assign;
function runIt(property2, shrink, sourceValues, verbose, interruptedAsFailure) {
  const isModernProperty =
    property2.runBeforeEach !== void 0 && property2.runAfterEach !== void 0;
  const runner = new RunnerIterator(
    sourceValues,
    shrink,
    verbose,
    interruptedAsFailure,
  );
  for (const v of runner) {
    if (isModernProperty) {
      property2.runBeforeEach();
    }
    const out = property2.run(v, isModernProperty);
    if (isModernProperty) {
      property2.runAfterEach();
    }
    runner.handleResult(out);
  }
  return runner.runExecution;
}
async function asyncRunIt(
  property2,
  shrink,
  sourceValues,
  verbose,
  interruptedAsFailure,
) {
  const isModernProperty =
    property2.runBeforeEach !== void 0 && property2.runAfterEach !== void 0;
  const runner = new RunnerIterator(
    sourceValues,
    shrink,
    verbose,
    interruptedAsFailure,
  );
  for (const v of runner) {
    if (isModernProperty) {
      await property2.runBeforeEach();
    }
    const out = await property2.run(v, isModernProperty);
    if (isModernProperty) {
      await property2.runAfterEach();
    }
    runner.handleResult(out);
  }
  return runner.runExecution;
}
function check(rawProperty, params) {
  if (rawProperty == null || rawProperty.generate == null)
    throw new Error(
      'Invalid property encountered, please use a valid property',
    );
  if (rawProperty.run == null)
    throw new Error(
      'Invalid property encountered, please use a valid property not an arbitrary',
    );
  const qParams = QualifiedParameters.read(
    safeObjectAssign3(safeObjectAssign3({}, readConfigureGlobal()), params),
  );
  if (qParams.reporter !== null && qParams.asyncReporter !== null)
    throw new Error(
      'Invalid parameters encountered, reporter and asyncReporter cannot be specified together',
    );
  if (qParams.asyncReporter !== null && !rawProperty.isAsync())
    throw new Error(
      'Invalid parameters encountered, only asyncProperty can be used when asyncReporter specified',
    );
  const property2 = decorateProperty(rawProperty, qParams);
  const maxInitialIterations =
    qParams.path.length === 0 || qParams.path.indexOf(':') === -1
      ? qParams.numRuns
      : -1;
  const maxSkips = qParams.numRuns * qParams.maxSkipsPerRun;
  const shrink = (...args) => property2.shrink(...args);
  const initialValues =
    qParams.path.length === 0
      ? toss(property2, qParams.seed, qParams.randomType, qParams.examples)
      : pathWalk(
          qParams.path,
          stream(
            lazyToss(
              property2,
              qParams.seed,
              qParams.randomType,
              qParams.examples,
            ),
          ),
          shrink,
        );
  const sourceValues = new SourceValuesIterator(
    initialValues,
    maxInitialIterations,
    maxSkips,
  );
  const finalShrink = !qParams.endOnFailure ? shrink : Stream.nil;
  return property2.isAsync()
    ? asyncRunIt(
        property2,
        finalShrink,
        sourceValues,
        qParams.verbose,
        qParams.markInterruptAsFailure,
      ).then((e) =>
        e.toRunDetails(qParams.seed, qParams.path, maxSkips, qParams),
      )
    : runIt(
        property2,
        finalShrink,
        sourceValues,
        qParams.verbose,
        qParams.markInterruptAsFailure,
      ).toRunDetails(qParams.seed, qParams.path, maxSkips, qParams);
}
function assert(property2, params) {
  const out = check(property2, params);
  if (property2.isAsync()) return out.then(asyncReportRunDetails);
  else reportRunDetails(out);
}

// ../../../node_modules/fast-check/lib/esm/check/runner/Sampler.js
function toProperty(generator, qParams) {
  const prop = !Object.prototype.hasOwnProperty.call(generator, 'isAsync')
    ? new Property(generator, () => true)
    : generator;
  return qParams.unbiased === true ? new UnbiasedProperty(prop) : prop;
}
function streamSample(generator, params) {
  const extendedParams =
    typeof params === 'number'
      ? Object.assign(Object.assign({}, readConfigureGlobal()), {
          numRuns: params,
        })
      : Object.assign(Object.assign({}, readConfigureGlobal()), params);
  const qParams = QualifiedParameters.read(extendedParams);
  const nextProperty = toProperty(generator, qParams);
  const shrink = nextProperty.shrink.bind(nextProperty);
  const tossedValues =
    qParams.path.length === 0
      ? stream(
          toss(
            nextProperty,
            qParams.seed,
            qParams.randomType,
            qParams.examples,
          ),
        )
      : pathWalk(
          qParams.path,
          stream(
            lazyToss(
              nextProperty,
              qParams.seed,
              qParams.randomType,
              qParams.examples,
            ),
          ),
          shrink,
        );
  return tossedValues.take(qParams.numRuns).map((s) => s.value_);
}
function sample(generator, params) {
  return [...streamSample(generator, params)];
}
function round2(n) {
  return (Math.round(n * 100) / 100).toFixed(2);
}
function statistics(generator, classify, params) {
  const extendedParams =
    typeof params === 'number'
      ? Object.assign(Object.assign({}, readConfigureGlobal()), {
          numRuns: params,
        })
      : Object.assign(Object.assign({}, readConfigureGlobal()), params);
  const qParams = QualifiedParameters.read(extendedParams);
  const recorded = {};
  for (const g of streamSample(generator, params)) {
    const out = classify(g);
    const categories = Array.isArray(out) ? out : [out];
    for (const c of categories) {
      recorded[c] = (recorded[c] || 0) + 1;
    }
  }
  const data = Object.entries(recorded)
    .sort((a, b) => b[1] - a[1])
    .map((i) => [i[0], `${round2((i[1] * 100) / qParams.numRuns)}%`]);
  const longestName = data
    .map((i) => i[0].length)
    .reduce((p, c) => Math.max(p, c), 0);
  const longestPercent = data
    .map((i) => i[1].length)
    .reduce((p, c) => Math.max(p, c), 0);
  for (const item of data) {
    qParams.logger(
      `${item[0].padEnd(longestName, '.')}..${item[1].padStart(longestPercent, '.')}`,
    );
  }
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/_internals/builders/GeneratorValueBuilder.js
function buildGeneratorValue(
  mrng,
  biasFactor,
  computePreBuiltValues,
  arbitraryCache,
) {
  const preBuiltValues = computePreBuiltValues();
  let localMrng = mrng.clone();
  const context2 = { mrng: mrng.clone(), biasFactor, history: [] };
  const valueFunction = (arb) => {
    const preBuiltValue = preBuiltValues[context2.history.length];
    if (preBuiltValue !== void 0 && preBuiltValue.arb === arb) {
      const value2 = preBuiltValue.value;
      context2.history.push({
        arb,
        value: value2,
        context: preBuiltValue.context,
        mrng: preBuiltValue.mrng,
      });
      localMrng = preBuiltValue.mrng.clone();
      return value2;
    }
    const g = arb.generate(localMrng, biasFactor);
    context2.history.push({
      arb,
      value: g.value_,
      context: g.context,
      mrng: localMrng.clone(),
    });
    return g.value;
  };
  const memoedValueFunction = (arb, ...args) => {
    return valueFunction(arbitraryCache(arb, args));
  };
  const valueMethods = {
    values() {
      return context2.history.map((c) => c.value);
    },
    [cloneMethod]() {
      return buildGeneratorValue(
        mrng,
        biasFactor,
        computePreBuiltValues,
        arbitraryCache,
      ).value;
    },
    [toStringMethod]() {
      return stringify(context2.history.map((c) => c.value));
    },
  };
  const value = Object.assign(memoedValueFunction, valueMethods);
  return new Value(value, context2);
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/_internals/builders/StableArbitraryGeneratorCache.js
function buildStableArbitraryGeneratorCache(isEqual) {
  const previousCallsPerBuilder = /* @__PURE__ */ new Map();
  return function stableArbitraryGeneratorCache(builder, args) {
    const entriesForBuilder = previousCallsPerBuilder.get(builder);
    if (entriesForBuilder === void 0) {
      const newValue2 = builder(...args);
      previousCallsPerBuilder.set(builder, [{ args, value: newValue2 }]);
      return newValue2;
    }
    const safeEntriesForBuilder = entriesForBuilder;
    for (const entry of safeEntriesForBuilder) {
      if (isEqual(args, entry.args)) {
        return entry.value;
      }
    }
    const newValue = builder(...args);
    safeEntriesForBuilder.push({ args, value: newValue });
    return newValue;
  };
}
function naiveIsEqual(v1, v2) {
  if (
    v1 !== null &&
    typeof v1 === 'object' &&
    v2 !== null &&
    typeof v2 === 'object'
  ) {
    if (Array.isArray(v1)) {
      if (!Array.isArray(v2)) return false;
      if (v1.length !== v2.length) return false;
    } else if (Array.isArray(v2)) {
      return false;
    }
    if (Object.keys(v1).length !== Object.keys(v2).length) {
      return false;
    }
    for (const index in v1) {
      if (!(index in v2)) {
        return false;
      }
      if (!naiveIsEqual(v1[index], v2[index])) {
        return false;
      }
    }
    return true;
  } else {
    return Object.is(v1, v2);
  }
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/_internals/GeneratorArbitrary.js
var GeneratorArbitrary = class extends Arbitrary {
  constructor() {
    super(...arguments);
    this.arbitraryCache = buildStableArbitraryGeneratorCache(naiveIsEqual);
  }
  generate(mrng, biasFactor) {
    return buildGeneratorValue(mrng, biasFactor, () => [], this.arbitraryCache);
  }
  canShrinkWithoutContext(value) {
    return false;
  }
  shrink(_value, context2) {
    if (context2 === void 0) {
      return Stream.nil();
    }
    const safeContext = context2;
    const mrng = safeContext.mrng;
    const biasFactor = safeContext.biasFactor;
    const history = safeContext.history;
    return tupleShrink(
      history.map((c) => c.arb),
      history.map((c) => c.value),
      history.map((c) => c.context),
    ).map((shrink) => {
      function computePreBuiltValues() {
        const subValues = shrink.value;
        const subContexts = shrink.context;
        return history.map((entry, index) => ({
          arb: entry.arb,
          value: subValues[index],
          context: subContexts[index],
          mrng: entry.mrng,
        }));
      }
      return buildGeneratorValue(
        mrng,
        biasFactor,
        computePreBuiltValues,
        this.arbitraryCache,
      );
    });
  }
};

// ../../../node_modules/fast-check/lib/esm/arbitrary/gen.js
function gen() {
  return new GeneratorArbitrary();
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/_internals/helpers/BiasNumericRange.js
var safeMathFloor = Math.floor;
var safeMathLog2 = Math.log;
function integerLogLike(v) {
  return safeMathFloor(safeMathLog2(v) / safeMathLog2(2));
}
function bigIntLogLike(v) {
  if (v === SBigInt(0)) return SBigInt(0);
  return SBigInt(SString(v).length);
}
function biasNumericRange(min, max, logLike) {
  if (min === max) {
    return [{ min, max }];
  }
  if (min < 0 && max > 0) {
    const logMin = logLike(-min);
    const logMax = logLike(max);
    return [
      { min: -logMin, max: logMax },
      { min: max - logMax, max },
      { min, max: min + logMin },
    ];
  }
  const logGap = logLike(max - min);
  const arbCloseToMin = { min, max: min + logGap };
  const arbCloseToMax = { min: max - logGap, max };
  return min < 0
    ? [arbCloseToMax, arbCloseToMin]
    : [arbCloseToMin, arbCloseToMax];
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/_internals/helpers/ShrinkInteger.js
var safeMathCeil = Math.ceil;
var safeMathFloor2 = Math.floor;
function halvePosInteger(n) {
  return safeMathFloor2(n / 2);
}
function halveNegInteger(n) {
  return safeMathCeil(n / 2);
}
function shrinkInteger(current, target, tryTargetAsap) {
  const realGap = current - target;
  function* shrinkDecr() {
    let previous = tryTargetAsap ? void 0 : target;
    const gap = tryTargetAsap ? realGap : halvePosInteger(realGap);
    for (
      let toremove = gap;
      toremove > 0;
      toremove = halvePosInteger(toremove)
    ) {
      const next = toremove === realGap ? target : current - toremove;
      yield new Value(next, previous);
      previous = next;
    }
  }
  function* shrinkIncr() {
    let previous = tryTargetAsap ? void 0 : target;
    const gap = tryTargetAsap ? realGap : halveNegInteger(realGap);
    for (
      let toremove = gap;
      toremove < 0;
      toremove = halveNegInteger(toremove)
    ) {
      const next = toremove === realGap ? target : current - toremove;
      yield new Value(next, previous);
      previous = next;
    }
  }
  return realGap > 0 ? stream(shrinkDecr()) : stream(shrinkIncr());
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/_internals/IntegerArbitrary.js
var safeMathSign = Math.sign;
var safeNumberIsInteger = Number.isInteger;
var safeObjectIs = Object.is;
var IntegerArbitrary = class _IntegerArbitrary extends Arbitrary {
  constructor(min, max) {
    super();
    this.min = min;
    this.max = max;
  }
  generate(mrng, biasFactor) {
    const range = this.computeGenerateRange(mrng, biasFactor);
    return new Value(mrng.nextInt(range.min, range.max), void 0);
  }
  canShrinkWithoutContext(value) {
    return (
      typeof value === 'number' &&
      safeNumberIsInteger(value) &&
      !safeObjectIs(value, -0) &&
      this.min <= value &&
      value <= this.max
    );
  }
  shrink(current, context2) {
    if (!_IntegerArbitrary.isValidContext(current, context2)) {
      const target = this.defaultTarget();
      return shrinkInteger(current, target, true);
    }
    if (this.isLastChanceTry(current, context2)) {
      return Stream.of(new Value(context2, void 0));
    }
    return shrinkInteger(current, context2, false);
  }
  defaultTarget() {
    if (this.min <= 0 && this.max >= 0) {
      return 0;
    }
    return this.min < 0 ? this.max : this.min;
  }
  computeGenerateRange(mrng, biasFactor) {
    if (biasFactor === void 0 || mrng.nextInt(1, biasFactor) !== 1) {
      return { min: this.min, max: this.max };
    }
    const ranges = biasNumericRange(this.min, this.max, integerLogLike);
    if (ranges.length === 1) {
      return ranges[0];
    }
    const id = mrng.nextInt(-2 * (ranges.length - 1), ranges.length - 2);
    return id < 0 ? ranges[0] : ranges[id + 1];
  }
  isLastChanceTry(current, context2) {
    if (current > 0) return current === context2 + 1 && current > this.min;
    if (current < 0) return current === context2 - 1 && current < this.max;
    return false;
  }
  static isValidContext(current, context2) {
    if (context2 === void 0) {
      return false;
    }
    if (typeof context2 !== 'number') {
      throw new Error(`Invalid context type passed to IntegerArbitrary (#1)`);
    }
    if (context2 !== 0 && safeMathSign(current) !== safeMathSign(context2)) {
      throw new Error(`Invalid context value passed to IntegerArbitrary (#2)`);
    }
    return true;
  }
};

// ../../../node_modules/fast-check/lib/esm/arbitrary/integer.js
var safeNumberIsInteger2 = Number.isInteger;
function buildCompleteIntegerConstraints(constraints) {
  const min = constraints.min !== void 0 ? constraints.min : -2147483648;
  const max = constraints.max !== void 0 ? constraints.max : 2147483647;
  return { min, max };
}
function integer(constraints = {}) {
  const fullConstraints = buildCompleteIntegerConstraints(constraints);
  if (fullConstraints.min > fullConstraints.max) {
    throw new Error(
      'fc.integer maximum value should be equal or greater than the minimum one',
    );
  }
  if (!safeNumberIsInteger2(fullConstraints.min)) {
    throw new Error('fc.integer minimum value should be an integer');
  }
  if (!safeNumberIsInteger2(fullConstraints.max)) {
    throw new Error('fc.integer maximum value should be an integer');
  }
  return new IntegerArbitrary(fullConstraints.min, fullConstraints.max);
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/_internals/helpers/DepthContext.js
var depthContextCache = /* @__PURE__ */ new Map();
function getDepthContextFor(contextMeta) {
  if (contextMeta === void 0) {
    return { depth: 0 };
  }
  if (typeof contextMeta !== 'string') {
    return contextMeta;
  }
  const cachedContext = depthContextCache.get(contextMeta);
  if (cachedContext !== void 0) {
    return cachedContext;
  }
  const context2 = { depth: 0 };
  depthContextCache.set(contextMeta, context2);
  return context2;
}
function createDepthIdentifier() {
  const identifier = { depth: 0 };
  return identifier;
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/_internals/implementations/NoopSlicedGenerator.js
var NoopSlicedGenerator = class {
  constructor(arb, mrng, biasFactor) {
    this.arb = arb;
    this.mrng = mrng;
    this.biasFactor = biasFactor;
  }
  attemptExact() {
    return;
  }
  next() {
    return this.arb.generate(this.mrng, this.biasFactor);
  }
};

// ../../../node_modules/fast-check/lib/esm/arbitrary/_internals/implementations/SlicedBasedGenerator.js
var safeMathMin2 = Math.min;
var safeMathMax = Math.max;
var SlicedBasedGenerator = class {
  constructor(arb, mrng, slices, biasFactor) {
    this.arb = arb;
    this.mrng = mrng;
    this.slices = slices;
    this.biasFactor = biasFactor;
    this.activeSliceIndex = 0;
    this.nextIndexInSlice = 0;
    this.lastIndexInSlice = -1;
  }
  attemptExact(targetLength) {
    if (targetLength !== 0 && this.mrng.nextInt(1, this.biasFactor) === 1) {
      const eligibleIndices = [];
      for (let index = 0; index !== this.slices.length; ++index) {
        const slice = this.slices[index];
        if (slice.length === targetLength) {
          safePush(eligibleIndices, index);
        }
      }
      if (eligibleIndices.length === 0) {
        return;
      }
      this.activeSliceIndex =
        eligibleIndices[this.mrng.nextInt(0, eligibleIndices.length - 1)];
      this.nextIndexInSlice = 0;
      this.lastIndexInSlice = targetLength - 1;
    }
  }
  next() {
    if (this.nextIndexInSlice <= this.lastIndexInSlice) {
      return new Value(
        this.slices[this.activeSliceIndex][this.nextIndexInSlice++],
        void 0,
      );
    }
    if (this.mrng.nextInt(1, this.biasFactor) !== 1) {
      return this.arb.generate(this.mrng, this.biasFactor);
    }
    this.activeSliceIndex = this.mrng.nextInt(0, this.slices.length - 1);
    const slice = this.slices[this.activeSliceIndex];
    if (this.mrng.nextInt(1, this.biasFactor) !== 1) {
      this.nextIndexInSlice = 1;
      this.lastIndexInSlice = slice.length - 1;
      return new Value(slice[0], void 0);
    }
    const rangeBoundaryA = this.mrng.nextInt(0, slice.length - 1);
    const rangeBoundaryB = this.mrng.nextInt(0, slice.length - 1);
    this.nextIndexInSlice = safeMathMin2(rangeBoundaryA, rangeBoundaryB);
    this.lastIndexInSlice = safeMathMax(rangeBoundaryA, rangeBoundaryB);
    return new Value(slice[this.nextIndexInSlice++], void 0);
  }
};

// ../../../node_modules/fast-check/lib/esm/arbitrary/_internals/helpers/BuildSlicedGenerator.js
function buildSlicedGenerator(arb, mrng, slices, biasFactor) {
  if (
    biasFactor === void 0 ||
    slices.length === 0 ||
    mrng.nextInt(1, biasFactor) !== 1
  ) {
    return new NoopSlicedGenerator(arb, mrng, biasFactor);
  }
  return new SlicedBasedGenerator(arb, mrng, slices, biasFactor);
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/_internals/ArrayArbitrary.js
var safeMathFloor3 = Math.floor;
var safeMathLog3 = Math.log;
var safeMathMax2 = Math.max;
var safeArrayIsArray2 = Array.isArray;
function biasedMaxLength(minLength, maxLength) {
  if (minLength === maxLength) {
    return minLength;
  }
  return (
    minLength +
    safeMathFloor3(safeMathLog3(maxLength - minLength) / safeMathLog3(2))
  );
}
var ArrayArbitrary = class _ArrayArbitrary extends Arbitrary {
  constructor(
    arb,
    minLength,
    maxGeneratedLength,
    maxLength,
    depthIdentifier,
    setBuilder,
    customSlices,
  ) {
    super();
    this.arb = arb;
    this.minLength = minLength;
    this.maxGeneratedLength = maxGeneratedLength;
    this.maxLength = maxLength;
    this.setBuilder = setBuilder;
    this.customSlices = customSlices;
    this.lengthArb = integer({ min: minLength, max: maxGeneratedLength });
    this.depthContext = getDepthContextFor(depthIdentifier);
  }
  preFilter(tab) {
    if (this.setBuilder === void 0) {
      return tab;
    }
    const s = this.setBuilder();
    for (let index = 0; index !== tab.length; ++index) {
      s.tryAdd(tab[index]);
    }
    return s.getData();
  }
  static makeItCloneable(vs, shrinkables) {
    vs[cloneMethod] = () => {
      const cloned = [];
      for (let idx = 0; idx !== shrinkables.length; ++idx) {
        safePush(cloned, shrinkables[idx].value);
      }
      this.makeItCloneable(cloned, shrinkables);
      return cloned;
    };
    return vs;
  }
  generateNItemsNoDuplicates(setBuilder, N, mrng, biasFactorItems) {
    let numSkippedInRow = 0;
    const s = setBuilder();
    const slicedGenerator = buildSlicedGenerator(
      this.arb,
      mrng,
      this.customSlices,
      biasFactorItems,
    );
    while (s.size() < N && numSkippedInRow < this.maxGeneratedLength) {
      const current = slicedGenerator.next();
      if (s.tryAdd(current)) {
        numSkippedInRow = 0;
      } else {
        numSkippedInRow += 1;
      }
    }
    return s.getData();
  }
  safeGenerateNItemsNoDuplicates(setBuilder, N, mrng, biasFactorItems) {
    const depthImpact = safeMathMax2(
      0,
      N - biasedMaxLength(this.minLength, this.maxGeneratedLength),
    );
    this.depthContext.depth += depthImpact;
    try {
      return this.generateNItemsNoDuplicates(
        setBuilder,
        N,
        mrng,
        biasFactorItems,
      );
    } finally {
      this.depthContext.depth -= depthImpact;
    }
  }
  generateNItems(N, mrng, biasFactorItems) {
    const items = [];
    const slicedGenerator = buildSlicedGenerator(
      this.arb,
      mrng,
      this.customSlices,
      biasFactorItems,
    );
    slicedGenerator.attemptExact(N);
    for (let index = 0; index !== N; ++index) {
      const current = slicedGenerator.next();
      safePush(items, current);
    }
    return items;
  }
  safeGenerateNItems(N, mrng, biasFactorItems) {
    const depthImpact = safeMathMax2(
      0,
      N - biasedMaxLength(this.minLength, this.maxGeneratedLength),
    );
    this.depthContext.depth += depthImpact;
    try {
      return this.generateNItems(N, mrng, biasFactorItems);
    } finally {
      this.depthContext.depth -= depthImpact;
    }
  }
  wrapper(itemsRaw, shrunkOnce, itemsRawLengthContext, startIndex) {
    const items = shrunkOnce ? this.preFilter(itemsRaw) : itemsRaw;
    let cloneable = false;
    const vs = [];
    const itemsContexts = [];
    for (let idx = 0; idx !== items.length; ++idx) {
      const s = items[idx];
      cloneable = cloneable || s.hasToBeCloned;
      safePush(vs, s.value);
      safePush(itemsContexts, s.context);
    }
    if (cloneable) {
      _ArrayArbitrary.makeItCloneable(vs, items);
    }
    const context2 = {
      shrunkOnce,
      lengthContext:
        itemsRaw.length === items.length && itemsRawLengthContext !== void 0
          ? itemsRawLengthContext
          : void 0,
      itemsContexts,
      startIndex,
    };
    return new Value(vs, context2);
  }
  generate(mrng, biasFactor) {
    const biasMeta = this.applyBias(mrng, biasFactor);
    const targetSize = biasMeta.size;
    const items =
      this.setBuilder !== void 0
        ? this.safeGenerateNItemsNoDuplicates(
            this.setBuilder,
            targetSize,
            mrng,
            biasMeta.biasFactorItems,
          )
        : this.safeGenerateNItems(targetSize, mrng, biasMeta.biasFactorItems);
    return this.wrapper(items, false, void 0, 0);
  }
  applyBias(mrng, biasFactor) {
    if (biasFactor === void 0) {
      return { size: this.lengthArb.generate(mrng, void 0).value };
    }
    if (this.minLength === this.maxGeneratedLength) {
      return {
        size: this.lengthArb.generate(mrng, void 0).value,
        biasFactorItems: biasFactor,
      };
    }
    if (mrng.nextInt(1, biasFactor) !== 1) {
      return { size: this.lengthArb.generate(mrng, void 0).value };
    }
    if (
      mrng.nextInt(1, biasFactor) !== 1 ||
      this.minLength === this.maxGeneratedLength
    ) {
      return {
        size: this.lengthArb.generate(mrng, void 0).value,
        biasFactorItems: biasFactor,
      };
    }
    const maxBiasedLength = biasedMaxLength(
      this.minLength,
      this.maxGeneratedLength,
    );
    const targetSizeValue = integer({
      min: this.minLength,
      max: maxBiasedLength,
    }).generate(mrng, void 0);
    return { size: targetSizeValue.value, biasFactorItems: biasFactor };
  }
  canShrinkWithoutContext(value) {
    if (
      !safeArrayIsArray2(value) ||
      this.minLength > value.length ||
      value.length > this.maxLength
    ) {
      return false;
    }
    for (let index = 0; index !== value.length; ++index) {
      if (!(index in value)) {
        return false;
      }
      if (!this.arb.canShrinkWithoutContext(value[index])) {
        return false;
      }
    }
    const filtered = this.preFilter(
      safeMap(value, (item) => new Value(item, void 0)),
    );
    return filtered.length === value.length;
  }
  shrinkItemByItem(value, safeContext, endIndex) {
    const shrinks = [];
    for (let index = safeContext.startIndex; index < endIndex; ++index) {
      safePush(
        shrinks,
        makeLazy(() =>
          this.arb
            .shrink(value[index], safeContext.itemsContexts[index])
            .map((v) => {
              const beforeCurrent = safeMap(
                safeSlice(value, 0, index),
                (v2, i) =>
                  new Value(cloneIfNeeded(v2), safeContext.itemsContexts[i]),
              );
              const afterCurrent = safeMap(
                safeSlice(value, index + 1),
                (v2, i) =>
                  new Value(
                    cloneIfNeeded(v2),
                    safeContext.itemsContexts[i + index + 1],
                  ),
              );
              return [[...beforeCurrent, v, ...afterCurrent], void 0, index];
            }),
        ),
      );
    }
    return Stream.nil().join(...shrinks);
  }
  shrinkImpl(value, context2) {
    if (value.length === 0) {
      return Stream.nil();
    }
    const safeContext =
      context2 !== void 0
        ? context2
        : {
            shrunkOnce: false,
            lengthContext: void 0,
            itemsContexts: [],
            startIndex: 0,
          };
    return this.lengthArb
      .shrink(value.length, safeContext.lengthContext)
      .drop(
        safeContext.shrunkOnce &&
          safeContext.lengthContext === void 0 &&
          value.length > this.minLength + 1
          ? 1
          : 0,
      )
      .map((lengthValue) => {
        const sliceStart = value.length - lengthValue.value;
        return [
          safeMap(
            safeSlice(value, sliceStart),
            (v, index) =>
              new Value(
                cloneIfNeeded(v),
                safeContext.itemsContexts[index + sliceStart],
              ),
          ),
          lengthValue.context,
          0,
        ];
      })
      .join(
        makeLazy(() =>
          value.length > this.minLength
            ? this.shrinkItemByItem(value, safeContext, 1)
            : this.shrinkItemByItem(value, safeContext, value.length),
        ),
      )
      .join(
        value.length > this.minLength
          ? makeLazy(() => {
              const subContext = {
                shrunkOnce: false,
                lengthContext: void 0,
                itemsContexts: safeSlice(safeContext.itemsContexts, 1),
                startIndex: 0,
              };
              return this.shrinkImpl(safeSlice(value, 1), subContext)
                .filter((v) => this.minLength <= v[0].length + 1)
                .map((v) => {
                  return [
                    [
                      new Value(
                        cloneIfNeeded(value[0]),
                        safeContext.itemsContexts[0],
                      ),
                      ...v[0],
                    ],
                    void 0,
                    0,
                  ];
                });
            })
          : Stream.nil(),
      );
  }
  shrink(value, context2) {
    return this.shrinkImpl(value, context2).map((contextualValue) =>
      this.wrapper(
        contextualValue[0],
        true,
        contextualValue[1],
        contextualValue[2],
      ),
    );
  }
};

// ../../../node_modules/fast-check/lib/esm/arbitrary/_internals/helpers/MaxLengthFromMinLength.js
var safeMathFloor4 = Math.floor;
var safeMathMin3 = Math.min;
var MaxLengthUpperBound = 2147483647;
var orderedSize = ['xsmall', 'small', 'medium', 'large', 'xlarge'];
var orderedRelativeSize = ['-4', '-3', '-2', '-1', '=', '+1', '+2', '+3', '+4'];
var DefaultSize = 'small';
function maxLengthFromMinLength(minLength, size) {
  switch (size) {
    case 'xsmall':
      return safeMathFloor4(1.1 * minLength) + 1;
    case 'small':
      return 2 * minLength + 10;
    case 'medium':
      return 11 * minLength + 100;
    case 'large':
      return 101 * minLength + 1e3;
    case 'xlarge':
      return 1001 * minLength + 1e4;
    default:
      throw new Error(
        `Unable to compute lengths based on received size: ${size}`,
      );
  }
}
function relativeSizeToSize(size, defaultSize) {
  const sizeInRelative = safeIndexOf(orderedRelativeSize, size);
  if (sizeInRelative === -1) {
    return size;
  }
  const defaultSizeInSize = safeIndexOf(orderedSize, defaultSize);
  if (defaultSizeInSize === -1) {
    throw new Error(
      `Unable to offset size based on the unknown defaulted one: ${defaultSize}`,
    );
  }
  const resultingSizeInSize = defaultSizeInSize + sizeInRelative - 4;
  return resultingSizeInSize < 0
    ? orderedSize[0]
    : resultingSizeInSize >= orderedSize.length
      ? orderedSize[orderedSize.length - 1]
      : orderedSize[resultingSizeInSize];
}
function maxGeneratedLengthFromSizeForArbitrary(
  size,
  minLength,
  maxLength,
  specifiedMaxLength,
) {
  const {
    baseSize: defaultSize = DefaultSize,
    defaultSizeToMaxWhenMaxSpecified,
  } = readConfigureGlobal() || {};
  const definedSize =
    size !== void 0
      ? size
      : specifiedMaxLength && defaultSizeToMaxWhenMaxSpecified
        ? 'max'
        : defaultSize;
  if (definedSize === 'max') {
    return maxLength;
  }
  const finalSize = relativeSizeToSize(definedSize, defaultSize);
  return safeMathMin3(maxLengthFromMinLength(minLength, finalSize), maxLength);
}
function depthBiasFromSizeForArbitrary(depthSizeOrSize, specifiedMaxDepth) {
  if (typeof depthSizeOrSize === 'number') {
    return 1 / depthSizeOrSize;
  }
  const {
    baseSize: defaultSize = DefaultSize,
    defaultSizeToMaxWhenMaxSpecified,
  } = readConfigureGlobal() || {};
  const definedSize =
    depthSizeOrSize !== void 0
      ? depthSizeOrSize
      : specifiedMaxDepth && defaultSizeToMaxWhenMaxSpecified
        ? 'max'
        : defaultSize;
  if (definedSize === 'max') {
    return 0;
  }
  const finalSize = relativeSizeToSize(definedSize, defaultSize);
  switch (finalSize) {
    case 'xsmall':
      return 1;
    case 'small':
      return 0.5;
    case 'medium':
      return 0.25;
    case 'large':
      return 0.125;
    case 'xlarge':
      return 0.0625;
  }
}
function resolveSize(size) {
  const { baseSize: defaultSize = DefaultSize } = readConfigureGlobal() || {};
  if (size === void 0) {
    return defaultSize;
  }
  return relativeSizeToSize(size, defaultSize);
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/array.js
function array(arb, constraints = {}) {
  const size = constraints.size;
  const minLength = constraints.minLength || 0;
  const maxLengthOrUnset = constraints.maxLength;
  const depthIdentifier = constraints.depthIdentifier;
  const maxLength =
    maxLengthOrUnset !== void 0 ? maxLengthOrUnset : MaxLengthUpperBound;
  const specifiedMaxLength = maxLengthOrUnset !== void 0;
  const maxGeneratedLength = maxGeneratedLengthFromSizeForArbitrary(
    size,
    minLength,
    maxLength,
    specifiedMaxLength,
  );
  const customSlices = constraints.experimentalCustomSlices || [];
  return new ArrayArbitrary(
    arb,
    minLength,
    maxGeneratedLength,
    maxLength,
    depthIdentifier,
    void 0,
    customSlices,
  );
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/_internals/helpers/ShrinkBigInt.js
function halveBigInt(n) {
  return n / SBigInt(2);
}
function shrinkBigInt(current, target, tryTargetAsap) {
  const realGap = current - target;
  function* shrinkDecr() {
    let previous = tryTargetAsap ? void 0 : target;
    const gap = tryTargetAsap ? realGap : halveBigInt(realGap);
    for (let toremove = gap; toremove > 0; toremove = halveBigInt(toremove)) {
      const next = current - toremove;
      yield new Value(next, previous);
      previous = next;
    }
  }
  function* shrinkIncr() {
    let previous = tryTargetAsap ? void 0 : target;
    const gap = tryTargetAsap ? realGap : halveBigInt(realGap);
    for (let toremove = gap; toremove < 0; toremove = halveBigInt(toremove)) {
      const next = current - toremove;
      yield new Value(next, previous);
      previous = next;
    }
  }
  return realGap > 0 ? stream(shrinkDecr()) : stream(shrinkIncr());
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/_internals/BigIntArbitrary.js
var BigIntArbitrary = class _BigIntArbitrary extends Arbitrary {
  constructor(min, max) {
    super();
    this.min = min;
    this.max = max;
  }
  generate(mrng, biasFactor) {
    const range = this.computeGenerateRange(mrng, biasFactor);
    return new Value(mrng.nextBigInt(range.min, range.max), void 0);
  }
  computeGenerateRange(mrng, biasFactor) {
    if (biasFactor === void 0 || mrng.nextInt(1, biasFactor) !== 1) {
      return { min: this.min, max: this.max };
    }
    const ranges = biasNumericRange(this.min, this.max, bigIntLogLike);
    if (ranges.length === 1) {
      return ranges[0];
    }
    const id = mrng.nextInt(-2 * (ranges.length - 1), ranges.length - 2);
    return id < 0 ? ranges[0] : ranges[id + 1];
  }
  canShrinkWithoutContext(value) {
    return typeof value === 'bigint' && this.min <= value && value <= this.max;
  }
  shrink(current, context2) {
    if (!_BigIntArbitrary.isValidContext(current, context2)) {
      const target = this.defaultTarget();
      return shrinkBigInt(current, target, true);
    }
    if (this.isLastChanceTry(current, context2)) {
      return Stream.of(new Value(context2, void 0));
    }
    return shrinkBigInt(current, context2, false);
  }
  defaultTarget() {
    if (this.min <= 0 && this.max >= 0) {
      return SBigInt(0);
    }
    return this.min < 0 ? this.max : this.min;
  }
  isLastChanceTry(current, context2) {
    if (current > 0)
      return current === context2 + SBigInt(1) && current > this.min;
    if (current < 0)
      return current === context2 - SBigInt(1) && current < this.max;
    return false;
  }
  static isValidContext(current, context2) {
    if (context2 === void 0) {
      return false;
    }
    if (typeof context2 !== 'bigint') {
      throw new Error(`Invalid context type passed to BigIntArbitrary (#1)`);
    }
    const differentSigns =
      (current > 0 && context2 < 0) || (current < 0 && context2 > 0);
    if (context2 !== SBigInt(0) && differentSigns) {
      throw new Error(`Invalid context value passed to BigIntArbitrary (#2)`);
    }
    return true;
  }
};

// ../../../node_modules/fast-check/lib/esm/arbitrary/bigInt.js
function buildCompleteBigIntConstraints(constraints) {
  const DefaultPow = 256;
  const DefaultMin = SBigInt(-1) << SBigInt(DefaultPow - 1);
  const DefaultMax = (SBigInt(1) << SBigInt(DefaultPow - 1)) - SBigInt(1);
  const min = constraints.min;
  const max = constraints.max;
  return {
    min:
      min !== void 0
        ? min
        : DefaultMin -
          (max !== void 0 && max < SBigInt(0) ? max * max : SBigInt(0)),
    max:
      max !== void 0
        ? max
        : DefaultMax +
          (min !== void 0 && min > SBigInt(0) ? min * min : SBigInt(0)),
  };
}
function extractBigIntConstraints(args) {
  if (args[0] === void 0) {
    return {};
  }
  if (args[1] === void 0) {
    const constraints = args[0];
    return constraints;
  }
  return { min: args[0], max: args[1] };
}
function bigInt(...args) {
  const constraints = buildCompleteBigIntConstraints(
    extractBigIntConstraints(args),
  );
  if (constraints.min > constraints.max) {
    throw new Error('fc.bigInt expects max to be greater than or equal to min');
  }
  return new BigIntArbitrary(constraints.min, constraints.max);
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/bigIntN.js
function bigIntN(n) {
  if (n < 1) {
    throw new Error(
      'fc.bigIntN expects requested number of bits to be superior or equal to 1',
    );
  }
  const min = SBigInt(-1) << SBigInt(n - 1);
  const max = (SBigInt(1) << SBigInt(n - 1)) - SBigInt(1);
  return new BigIntArbitrary(min, max);
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/bigUint.js
function computeDefaultMax() {
  return (SBigInt(1) << SBigInt(256)) - SBigInt(1);
}
function bigUint(constraints) {
  const requestedMax =
    typeof constraints === 'object' ? constraints.max : constraints;
  const max = requestedMax !== void 0 ? requestedMax : computeDefaultMax();
  if (max < 0) {
    throw new Error(
      'fc.bigUint expects max to be greater than or equal to zero',
    );
  }
  return new BigIntArbitrary(SBigInt(0), max);
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/bigUintN.js
function bigUintN(n) {
  if (n < 0) {
    throw new Error(
      'fc.bigUintN expects requested number of bits to be superior or equal to 0',
    );
  }
  const min = SBigInt(0);
  const max = (SBigInt(1) << SBigInt(n)) - SBigInt(1);
  return new BigIntArbitrary(min, max);
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/boolean.js
function booleanMapper(v) {
  return v === 1;
}
function booleanUnmapper(v) {
  if (typeof v !== 'boolean') throw new Error('Unsupported input type');
  return v === true ? 1 : 0;
}
function boolean() {
  return integer({ min: 0, max: 1 })
    .map(booleanMapper, booleanUnmapper)
    .noBias();
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/_internals/ConstantArbitrary.js
var safeObjectIs2 = Object.is;
var ConstantArbitrary = class extends Arbitrary {
  constructor(values) {
    super();
    this.values = values;
  }
  generate(mrng, _biasFactor) {
    const idx =
      this.values.length === 1 ? 0 : mrng.nextInt(0, this.values.length - 1);
    const value = this.values[idx];
    if (!hasCloneMethod(value)) {
      return new Value(value, idx);
    }
    return new Value(value, idx, () => value[cloneMethod]());
  }
  canShrinkWithoutContext(value) {
    for (let idx = 0; idx !== this.values.length; ++idx) {
      if (safeObjectIs2(this.values[idx], value)) {
        return true;
      }
    }
    return false;
  }
  shrink(value, context2) {
    if (context2 === 0 || safeObjectIs2(value, this.values[0])) {
      return Stream.nil();
    }
    return Stream.of(new Value(this.values[0], 0));
  }
};

// ../../../node_modules/fast-check/lib/esm/arbitrary/constantFrom.js
function constantFrom(...values) {
  if (values.length === 0) {
    throw new Error('fc.constantFrom expects at least one parameter');
  }
  return new ConstantArbitrary(values);
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/falsy.js
function falsy(constraints) {
  if (!constraints || !constraints.withBigInt) {
    return constantFrom(false, null, void 0, 0, '', NaN);
  }
  return constantFrom(false, null, void 0, 0, '', NaN, SBigInt(0));
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/_internals/mappers/IndexToCharString.js
var indexToCharStringMapper = String.fromCodePoint;
function indexToCharStringUnmapper(c) {
  if (typeof c !== 'string') {
    throw new Error('Cannot unmap non-string');
  }
  if (c.length === 0 || c.length > 2) {
    throw new Error('Cannot unmap string with more or less than one character');
  }
  const c1 = safeCharCodeAt(c, 0);
  if (c.length === 1) {
    return c1;
  }
  const c2 = safeCharCodeAt(c, 1);
  if (c1 < 55296 || c1 > 56319 || c2 < 56320 || c2 > 57343) {
    throw new Error('Cannot unmap invalid surrogate pairs');
  }
  return c.codePointAt(0);
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/_internals/builders/CharacterArbitraryBuilder.js
function buildCharacterArbitrary(min, max, mapToCode, unmapFromCode) {
  return integer({ min, max }).map(
    (n) => indexToCharStringMapper(mapToCode(n)),
    (c) => unmapFromCode(indexToCharStringUnmapper(c)),
  );
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/_internals/mappers/IndexToPrintableIndex.js
function indexToPrintableIndexMapper(v) {
  if (v < 95) return v + 32;
  if (v <= 126) return v - 95;
  return v;
}
function indexToPrintableIndexUnmapper(v) {
  if (v >= 32 && v <= 126) return v - 32;
  if (v >= 0 && v <= 31) return v + 95;
  return v;
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/ascii.js
function ascii() {
  return buildCharacterArbitrary(
    0,
    127,
    indexToPrintableIndexMapper,
    indexToPrintableIndexUnmapper,
  );
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/base64.js
function base64Mapper(v) {
  if (v < 26) return v + 65;
  if (v < 52) return v + 97 - 26;
  if (v < 62) return v + 48 - 52;
  return v === 62 ? 43 : 47;
}
function base64Unmapper(v) {
  if (v >= 65 && v <= 90) return v - 65;
  if (v >= 97 && v <= 122) return v - 97 + 26;
  if (v >= 48 && v <= 57) return v - 48 + 52;
  return v === 43 ? 62 : v === 47 ? 63 : -1;
}
function base64() {
  return buildCharacterArbitrary(0, 63, base64Mapper, base64Unmapper);
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/char.js
function identity(v) {
  return v;
}
function char() {
  return buildCharacterArbitrary(32, 126, identity, identity);
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/char16bits.js
function char16bits() {
  return buildCharacterArbitrary(
    0,
    65535,
    indexToPrintableIndexMapper,
    indexToPrintableIndexUnmapper,
  );
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/fullUnicode.js
var gapSize = 57343 + 1 - 55296;
function unicodeMapper(v) {
  if (v < 55296) return indexToPrintableIndexMapper(v);
  return v + gapSize;
}
function unicodeUnmapper(v) {
  if (v < 55296) return indexToPrintableIndexUnmapper(v);
  if (v <= 57343) return -1;
  return v - gapSize;
}
function fullUnicode() {
  return buildCharacterArbitrary(
    0,
    1114111 - gapSize,
    unicodeMapper,
    unicodeUnmapper,
  );
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/hexa.js
function hexaMapper(v) {
  return v < 10 ? v + 48 : v + 97 - 10;
}
function hexaUnmapper(v) {
  return v < 58 ? v - 48 : v >= 97 && v < 103 ? v - 97 + 10 : -1;
}
function hexa() {
  return buildCharacterArbitrary(0, 15, hexaMapper, hexaUnmapper);
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/unicode.js
var gapSize2 = 57343 + 1 - 55296;
function unicodeMapper2(v) {
  if (v < 55296) return indexToPrintableIndexMapper(v);
  return v + gapSize2;
}
function unicodeUnmapper2(v) {
  if (v < 55296) return indexToPrintableIndexUnmapper(v);
  if (v <= 57343) return -1;
  return v - gapSize2;
}
function unicode() {
  return buildCharacterArbitrary(
    0,
    65535 - gapSize2,
    unicodeMapper2,
    unicodeUnmapper2,
  );
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/constant.js
function constant(value) {
  return new ConstantArbitrary([value]);
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/context.js
var ContextImplem = class _ContextImplem {
  constructor() {
    this.receivedLogs = [];
  }
  log(data) {
    this.receivedLogs.push(data);
  }
  size() {
    return this.receivedLogs.length;
  }
  toString() {
    return JSON.stringify({ logs: this.receivedLogs });
  }
  [cloneMethod]() {
    return new _ContextImplem();
  }
};
function context() {
  return constant(new ContextImplem());
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/_internals/mappers/TimeToDate.js
var safeNaN = Number.NaN;
var safeNumberIsNaN2 = Number.isNaN;
function timeToDateMapper(time) {
  return new SDate(time);
}
function timeToDateUnmapper(value) {
  if (!(value instanceof SDate) || value.constructor !== SDate) {
    throw new SError('Not a valid value for date unmapper');
  }
  return safeGetTime(value);
}
function timeToDateMapperWithNaN(valueForNaN) {
  return (time) => {
    return time === valueForNaN ? new SDate(safeNaN) : timeToDateMapper(time);
  };
}
function timeToDateUnmapperWithNaN(valueForNaN) {
  return (value) => {
    const time = timeToDateUnmapper(value);
    return safeNumberIsNaN2(time) ? valueForNaN : time;
  };
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/date.js
var safeNumberIsNaN3 = Number.isNaN;
function date(constraints = {}) {
  const intMin =
    constraints.min !== void 0 ? safeGetTime(constraints.min) : -864e13;
  const intMax =
    constraints.max !== void 0 ? safeGetTime(constraints.max) : 864e13;
  const noInvalidDate =
    constraints.noInvalidDate === void 0 || constraints.noInvalidDate;
  if (safeNumberIsNaN3(intMin))
    throw new Error('fc.date min must be valid instance of Date');
  if (safeNumberIsNaN3(intMax))
    throw new Error('fc.date max must be valid instance of Date');
  if (intMin > intMax)
    throw new Error('fc.date max must be greater or equal to min');
  if (noInvalidDate) {
    return integer({ min: intMin, max: intMax }).map(
      timeToDateMapper,
      timeToDateUnmapper,
    );
  }
  const valueForNaN = intMax + 1;
  return integer({ min: intMin, max: intMax + 1 }).map(
    timeToDateMapperWithNaN(valueForNaN),
    timeToDateUnmapperWithNaN(valueForNaN),
  );
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/_internals/CloneArbitrary.js
var safeSymbolIterator2 = Symbol.iterator;
var safeIsArray = Array.isArray;
var safeObjectIs3 = Object.is;
var CloneArbitrary = class _CloneArbitrary extends Arbitrary {
  constructor(arb, numValues) {
    super();
    this.arb = arb;
    this.numValues = numValues;
  }
  generate(mrng, biasFactor) {
    const items = [];
    if (this.numValues <= 0) {
      return this.wrapper(items);
    }
    for (let idx = 0; idx !== this.numValues - 1; ++idx) {
      safePush(items, this.arb.generate(mrng.clone(), biasFactor));
    }
    safePush(items, this.arb.generate(mrng, biasFactor));
    return this.wrapper(items);
  }
  canShrinkWithoutContext(value) {
    if (!safeIsArray(value) || value.length !== this.numValues) {
      return false;
    }
    if (value.length === 0) {
      return true;
    }
    for (let index = 1; index < value.length; ++index) {
      if (!safeObjectIs3(value[0], value[index])) {
        return false;
      }
    }
    return this.arb.canShrinkWithoutContext(value[0]);
  }
  shrink(value, context2) {
    if (value.length === 0) {
      return Stream.nil();
    }
    return new Stream(
      this.shrinkImpl(value, context2 !== void 0 ? context2 : []),
    ).map((v) => this.wrapper(v));
  }
  *shrinkImpl(value, contexts) {
    const its = safeMap(value, (v, idx) =>
      this.arb.shrink(v, contexts[idx])[safeSymbolIterator2](),
    );
    let cur = safeMap(its, (it) => it.next());
    while (!cur[0].done) {
      yield safeMap(cur, (c) => c.value);
      cur = safeMap(its, (it) => it.next());
    }
  }
  static makeItCloneable(vs, shrinkables) {
    vs[cloneMethod] = () => {
      const cloned = [];
      for (let idx = 0; idx !== shrinkables.length; ++idx) {
        safePush(cloned, shrinkables[idx].value);
      }
      this.makeItCloneable(cloned, shrinkables);
      return cloned;
    };
    return vs;
  }
  wrapper(items) {
    let cloneable = false;
    const vs = [];
    const contexts = [];
    for (let idx = 0; idx !== items.length; ++idx) {
      const s = items[idx];
      cloneable = cloneable || s.hasToBeCloned;
      safePush(vs, s.value);
      safePush(contexts, s.context);
    }
    if (cloneable) {
      _CloneArbitrary.makeItCloneable(vs, items);
    }
    return new Value(vs, contexts);
  }
};

// ../../../node_modules/fast-check/lib/esm/arbitrary/clone.js
function clone(arb, numValues) {
  return new CloneArbitrary(arb, numValues);
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/_internals/helpers/CustomEqualSet.js
var CustomEqualSet = class {
  constructor(isEqual) {
    this.isEqual = isEqual;
    this.data = [];
  }
  tryAdd(value) {
    for (let idx = 0; idx !== this.data.length; ++idx) {
      if (this.isEqual(this.data[idx], value)) {
        return false;
      }
    }
    safePush(this.data, value);
    return true;
  }
  size() {
    return this.data.length;
  }
  getData() {
    return this.data;
  }
};

// ../../../node_modules/fast-check/lib/esm/arbitrary/_internals/helpers/StrictlyEqualSet.js
var safeNumberIsNaN4 = Number.isNaN;
var StrictlyEqualSet = class {
  constructor(selector) {
    this.selector = selector;
    this.selectedItemsExceptNaN = new SSet();
    this.data = [];
  }
  tryAdd(value) {
    const selected = this.selector(value);
    if (safeNumberIsNaN4(selected)) {
      safePush(this.data, value);
      return true;
    }
    const sizeBefore = this.selectedItemsExceptNaN.size;
    safeAdd(this.selectedItemsExceptNaN, selected);
    if (sizeBefore !== this.selectedItemsExceptNaN.size) {
      safePush(this.data, value);
      return true;
    }
    return false;
  }
  size() {
    return this.data.length;
  }
  getData() {
    return this.data;
  }
};

// ../../../node_modules/fast-check/lib/esm/arbitrary/_internals/helpers/SameValueSet.js
var safeObjectIs4 = Object.is;
var SameValueSet = class {
  constructor(selector) {
    this.selector = selector;
    this.selectedItemsExceptMinusZero = new SSet();
    this.data = [];
    this.hasMinusZero = false;
  }
  tryAdd(value) {
    const selected = this.selector(value);
    if (safeObjectIs4(selected, -0)) {
      if (this.hasMinusZero) {
        return false;
      }
      safePush(this.data, value);
      this.hasMinusZero = true;
      return true;
    }
    const sizeBefore = this.selectedItemsExceptMinusZero.size;
    safeAdd(this.selectedItemsExceptMinusZero, selected);
    if (sizeBefore !== this.selectedItemsExceptMinusZero.size) {
      safePush(this.data, value);
      return true;
    }
    return false;
  }
  size() {
    return this.data.length;
  }
  getData() {
    return this.data;
  }
};

// ../../../node_modules/fast-check/lib/esm/arbitrary/_internals/helpers/SameValueZeroSet.js
var SameValueZeroSet = class {
  constructor(selector) {
    this.selector = selector;
    this.selectedItems = new SSet();
    this.data = [];
  }
  tryAdd(value) {
    const selected = this.selector(value);
    const sizeBefore = this.selectedItems.size;
    safeAdd(this.selectedItems, selected);
    if (sizeBefore !== this.selectedItems.size) {
      safePush(this.data, value);
      return true;
    }
    return false;
  }
  size() {
    return this.data.length;
  }
  getData() {
    return this.data;
  }
};

// ../../../node_modules/fast-check/lib/esm/arbitrary/uniqueArray.js
function buildUniqueArraySetBuilder(constraints) {
  if (typeof constraints.comparator === 'function') {
    if (constraints.selector === void 0) {
      const comparator2 = constraints.comparator;
      const isEqualForBuilder2 = (nextA, nextB) =>
        comparator2(nextA.value_, nextB.value_);
      return () => new CustomEqualSet(isEqualForBuilder2);
    }
    const comparator = constraints.comparator;
    const selector2 = constraints.selector;
    const refinedSelector2 = (next) => selector2(next.value_);
    const isEqualForBuilder = (nextA, nextB) =>
      comparator(refinedSelector2(nextA), refinedSelector2(nextB));
    return () => new CustomEqualSet(isEqualForBuilder);
  }
  const selector = constraints.selector || ((v) => v);
  const refinedSelector = (next) => selector(next.value_);
  switch (constraints.comparator) {
    case 'IsStrictlyEqual':
      return () => new StrictlyEqualSet(refinedSelector);
    case 'SameValueZero':
      return () => new SameValueZeroSet(refinedSelector);
    case 'SameValue':
    case void 0:
      return () => new SameValueSet(refinedSelector);
  }
}
function uniqueArray(arb, constraints = {}) {
  const minLength =
    constraints.minLength !== void 0 ? constraints.minLength : 0;
  const maxLength =
    constraints.maxLength !== void 0
      ? constraints.maxLength
      : MaxLengthUpperBound;
  const maxGeneratedLength = maxGeneratedLengthFromSizeForArbitrary(
    constraints.size,
    minLength,
    maxLength,
    constraints.maxLength !== void 0,
  );
  const depthIdentifier = constraints.depthIdentifier;
  const setBuilder = buildUniqueArraySetBuilder(constraints);
  const arrayArb = new ArrayArbitrary(
    arb,
    minLength,
    maxGeneratedLength,
    maxLength,
    depthIdentifier,
    setBuilder,
    [],
  );
  if (minLength === 0) return arrayArb;
  return arrayArb.filter((tab) => tab.length >= minLength);
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/_internals/mappers/KeyValuePairsToObject.js
var safeObjectCreate = Object.create;
var safeObjectDefineProperty3 = Object.defineProperty;
var safeObjectGetOwnPropertyDescriptor2 = Object.getOwnPropertyDescriptor;
var safeObjectGetPrototypeOf2 = Object.getPrototypeOf;
var safeObjectGetOwnPropertySymbols2 = Object.getOwnPropertySymbols;
var safeObjectGetOwnPropertyNames = Object.getOwnPropertyNames;
var safeObjectEntries = Object.entries;
function keyValuePairsToObjectMapper(definition) {
  const obj = definition[1] ? safeObjectCreate(null) : {};
  for (const keyValue of definition[0]) {
    safeObjectDefineProperty3(obj, keyValue[0], {
      enumerable: true,
      configurable: true,
      writable: true,
      value: keyValue[1],
    });
  }
  return obj;
}
function buildIsValidPropertyNameFilter(obj) {
  return function isValidPropertyNameFilter(key) {
    const descriptor = safeObjectGetOwnPropertyDescriptor2(obj, key);
    return (
      descriptor !== void 0 &&
      !!descriptor.configurable &&
      !!descriptor.enumerable &&
      !!descriptor.writable &&
      descriptor.get === void 0 &&
      descriptor.set === void 0
    );
  };
}
function keyValuePairsToObjectUnmapper(value) {
  if (typeof value !== 'object' || value === null) {
    throw new SError(
      'Incompatible instance received: should be a non-null object',
    );
  }
  const hasNullPrototype = safeObjectGetPrototypeOf2(value) === null;
  const hasObjectPrototype =
    'constructor' in value && value.constructor === Object;
  if (!hasNullPrototype && !hasObjectPrototype) {
    throw new SError(
      'Incompatible instance received: should be of exact type Object',
    );
  }
  if (safeObjectGetOwnPropertySymbols2(value).length > 0) {
    throw new SError('Incompatible instance received: should contain symbols');
  }
  if (
    !safeEvery(
      safeObjectGetOwnPropertyNames(value),
      buildIsValidPropertyNameFilter(value),
    )
  ) {
    throw new SError(
      'Incompatible instance received: should contain only c/e/w properties without get/set',
    );
  }
  return [safeObjectEntries(value), hasNullPrototype];
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/dictionary.js
function dictionaryKeyExtractor(entry) {
  return entry[0];
}
function dictionary(keyArb, valueArb, constraints = {}) {
  const noNullPrototype = constraints.noNullPrototype !== false;
  return tuple(
    uniqueArray(tuple(keyArb, valueArb), {
      minLength: constraints.minKeys,
      maxLength: constraints.maxKeys,
      size: constraints.size,
      selector: dictionaryKeyExtractor,
      depthIdentifier: constraints.depthIdentifier,
    }),
    noNullPrototype ? constant(false) : boolean(),
  ).map(keyValuePairsToObjectMapper, keyValuePairsToObjectUnmapper);
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/_internals/FrequencyArbitrary.js
var safePositiveInfinity2 = Number.POSITIVE_INFINITY;
var safeMaxSafeInteger = Number.MAX_SAFE_INTEGER;
var safeNumberIsInteger3 = Number.isInteger;
var safeMathFloor5 = Math.floor;
var safeMathPow = Math.pow;
var safeMathMin4 = Math.min;
var FrequencyArbitrary = class _FrequencyArbitrary extends Arbitrary {
  static from(warbs, constraints, label) {
    if (warbs.length === 0) {
      throw new Error(`${label} expects at least one weighted arbitrary`);
    }
    let totalWeight = 0;
    for (let idx = 0; idx !== warbs.length; ++idx) {
      const currentArbitrary = warbs[idx].arbitrary;
      if (currentArbitrary === void 0) {
        throw new Error(`${label} expects arbitraries to be specified`);
      }
      const currentWeight = warbs[idx].weight;
      totalWeight += currentWeight;
      if (!safeNumberIsInteger3(currentWeight)) {
        throw new Error(`${label} expects weights to be integer values`);
      }
      if (currentWeight < 0) {
        throw new Error(
          `${label} expects weights to be superior or equal to 0`,
        );
      }
    }
    if (totalWeight <= 0) {
      throw new Error(
        `${label} expects the sum of weights to be strictly superior to 0`,
      );
    }
    const sanitizedConstraints = {
      depthBias: depthBiasFromSizeForArbitrary(
        constraints.depthSize,
        constraints.maxDepth !== void 0,
      ),
      maxDepth:
        constraints.maxDepth != void 0
          ? constraints.maxDepth
          : safePositiveInfinity2,
      withCrossShrink: !!constraints.withCrossShrink,
    };
    return new _FrequencyArbitrary(
      warbs,
      sanitizedConstraints,
      getDepthContextFor(constraints.depthIdentifier),
    );
  }
  constructor(warbs, constraints, context2) {
    super();
    this.warbs = warbs;
    this.constraints = constraints;
    this.context = context2;
    let currentWeight = 0;
    this.cumulatedWeights = [];
    for (let idx = 0; idx !== warbs.length; ++idx) {
      currentWeight += warbs[idx].weight;
      safePush(this.cumulatedWeights, currentWeight);
    }
    this.totalWeight = currentWeight;
  }
  generate(mrng, biasFactor) {
    if (this.mustGenerateFirst()) {
      return this.safeGenerateForIndex(mrng, 0, biasFactor);
    }
    const selected = mrng.nextInt(
      this.computeNegDepthBenefit(),
      this.totalWeight - 1,
    );
    for (let idx = 0; idx !== this.cumulatedWeights.length; ++idx) {
      if (selected < this.cumulatedWeights[idx]) {
        return this.safeGenerateForIndex(mrng, idx, biasFactor);
      }
    }
    throw new Error(`Unable to generate from fc.frequency`);
  }
  canShrinkWithoutContext(value) {
    return this.canShrinkWithoutContextIndex(value) !== -1;
  }
  shrink(value, context2) {
    if (context2 !== void 0) {
      const safeContext = context2;
      const selectedIndex = safeContext.selectedIndex;
      const originalBias = safeContext.originalBias;
      const originalArbitrary = this.warbs[selectedIndex].arbitrary;
      const originalShrinks = originalArbitrary
        .shrink(value, safeContext.originalContext)
        .map((v) => this.mapIntoValue(selectedIndex, v, null, originalBias));
      if (safeContext.clonedMrngForFallbackFirst !== null) {
        if (safeContext.cachedGeneratedForFirst === void 0) {
          safeContext.cachedGeneratedForFirst = this.safeGenerateForIndex(
            safeContext.clonedMrngForFallbackFirst,
            0,
            originalBias,
          );
        }
        const valueFromFirst = safeContext.cachedGeneratedForFirst;
        return Stream.of(valueFromFirst).join(originalShrinks);
      }
      return originalShrinks;
    }
    const potentialSelectedIndex = this.canShrinkWithoutContextIndex(value);
    if (potentialSelectedIndex === -1) {
      return Stream.nil();
    }
    return this.defaultShrinkForFirst(potentialSelectedIndex).join(
      this.warbs[potentialSelectedIndex].arbitrary
        .shrink(value, void 0)
        .map((v) => this.mapIntoValue(potentialSelectedIndex, v, null, void 0)),
    );
  }
  defaultShrinkForFirst(selectedIndex) {
    ++this.context.depth;
    try {
      if (
        !this.mustFallbackToFirstInShrink(selectedIndex) ||
        this.warbs[0].fallbackValue === void 0
      ) {
        return Stream.nil();
      }
    } finally {
      --this.context.depth;
    }
    const rawShrinkValue = new Value(
      this.warbs[0].fallbackValue.default,
      void 0,
    );
    return Stream.of(this.mapIntoValue(0, rawShrinkValue, null, void 0));
  }
  canShrinkWithoutContextIndex(value) {
    if (this.mustGenerateFirst()) {
      return this.warbs[0].arbitrary.canShrinkWithoutContext(value) ? 0 : -1;
    }
    try {
      ++this.context.depth;
      for (let idx = 0; idx !== this.warbs.length; ++idx) {
        const warb = this.warbs[idx];
        if (
          warb.weight !== 0 &&
          warb.arbitrary.canShrinkWithoutContext(value)
        ) {
          return idx;
        }
      }
      return -1;
    } finally {
      --this.context.depth;
    }
  }
  mapIntoValue(idx, value, clonedMrngForFallbackFirst, biasFactor) {
    const context2 = {
      selectedIndex: idx,
      originalBias: biasFactor,
      originalContext: value.context,
      clonedMrngForFallbackFirst,
    };
    return new Value(value.value, context2);
  }
  safeGenerateForIndex(mrng, idx, biasFactor) {
    ++this.context.depth;
    try {
      const value = this.warbs[idx].arbitrary.generate(mrng, biasFactor);
      const clonedMrngForFallbackFirst = this.mustFallbackToFirstInShrink(idx)
        ? mrng.clone()
        : null;
      return this.mapIntoValue(
        idx,
        value,
        clonedMrngForFallbackFirst,
        biasFactor,
      );
    } finally {
      --this.context.depth;
    }
  }
  mustGenerateFirst() {
    return this.constraints.maxDepth <= this.context.depth;
  }
  mustFallbackToFirstInShrink(idx) {
    return (
      idx !== 0 &&
      this.constraints.withCrossShrink &&
      this.warbs[0].weight !== 0
    );
  }
  computeNegDepthBenefit() {
    const depthBias = this.constraints.depthBias;
    if (depthBias <= 0 || this.warbs[0].weight === 0) {
      return 0;
    }
    const depthBenefit =
      safeMathFloor5(safeMathPow(1 + depthBias, this.context.depth)) - 1;
    return (
      -safeMathMin4(this.totalWeight * depthBenefit, safeMaxSafeInteger) || 0
    );
  }
};

// ../../../node_modules/fast-check/lib/esm/arbitrary/oneof.js
function isOneOfContraints(param) {
  return (
    param != null &&
    typeof param === 'object' &&
    !('generate' in param) &&
    !('arbitrary' in param) &&
    !('weight' in param)
  );
}
function toWeightedArbitrary(maybeWeightedArbitrary) {
  if (isArbitrary(maybeWeightedArbitrary)) {
    return { arbitrary: maybeWeightedArbitrary, weight: 1 };
  }
  return maybeWeightedArbitrary;
}
function oneof(...args) {
  const constraints = args[0];
  if (isOneOfContraints(constraints)) {
    const weightedArbs2 = safeMap(safeSlice(args, 1), toWeightedArbitrary);
    return FrequencyArbitrary.from(weightedArbs2, constraints, 'fc.oneof');
  }
  const weightedArbs = safeMap(args, toWeightedArbitrary);
  return FrequencyArbitrary.from(weightedArbs, {}, 'fc.oneof');
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/nat.js
var safeNumberIsInteger4 = Number.isInteger;
function nat(arg) {
  const max =
    typeof arg === 'number'
      ? arg
      : arg && arg.max !== void 0
        ? arg.max
        : 2147483647;
  if (max < 0) {
    throw new Error('fc.nat value should be greater than or equal to 0');
  }
  if (!safeNumberIsInteger4(max)) {
    throw new Error('fc.nat maximum value should be an integer');
  }
  return new IntegerArbitrary(0, max);
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/_internals/mappers/IndexToMappedConstant.js
function indexToMappedConstantMapperFor(entries) {
  return function indexToMappedConstantMapper(choiceIndex) {
    let idx = -1;
    let numSkips = 0;
    while (choiceIndex >= numSkips) {
      numSkips += entries[++idx].num;
    }
    return entries[idx].build(choiceIndex - numSkips + entries[idx].num);
  };
}
function buildReverseMapping(entries) {
  const reverseMapping = {
    mapping: /* @__PURE__ */ new Map(),
    negativeZeroIndex: void 0,
  };
  let choiceIndex = 0;
  for (let entryIdx = 0; entryIdx !== entries.length; ++entryIdx) {
    const entry = entries[entryIdx];
    for (let idxInEntry = 0; idxInEntry !== entry.num; ++idxInEntry) {
      const value = entry.build(idxInEntry);
      if (value === 0 && 1 / value === Number.NEGATIVE_INFINITY) {
        reverseMapping.negativeZeroIndex = choiceIndex;
      } else {
        reverseMapping.mapping.set(value, choiceIndex);
      }
      ++choiceIndex;
    }
  }
  return reverseMapping;
}
function indexToMappedConstantUnmapperFor(entries) {
  let reverseMapping = null;
  return function indexToMappedConstantUnmapper(value) {
    if (reverseMapping === null) {
      reverseMapping = buildReverseMapping(entries);
    }
    const choiceIndex = Object.is(value, -0)
      ? reverseMapping.negativeZeroIndex
      : reverseMapping.mapping.get(value);
    if (choiceIndex === void 0) {
      throw new Error(
        'Unknown value encountered cannot be built using this mapToConstant',
      );
    }
    return choiceIndex;
  };
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/mapToConstant.js
function computeNumChoices(options) {
  if (options.length === 0)
    throw new Error(`fc.mapToConstant expects at least one option`);
  let numChoices = 0;
  for (let idx = 0; idx !== options.length; ++idx) {
    if (options[idx].num < 0)
      throw new Error(
        `fc.mapToConstant expects all options to have a number of entries greater or equal to zero`,
      );
    numChoices += options[idx].num;
  }
  if (numChoices === 0)
    throw new Error(
      `fc.mapToConstant expects at least one choice among options`,
    );
  return numChoices;
}
function mapToConstant(...entries) {
  const numChoices = computeNumChoices(entries);
  return nat({ max: numChoices - 1 }).map(
    indexToMappedConstantMapperFor(entries),
    indexToMappedConstantUnmapperFor(entries),
  );
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/_internals/builders/CharacterRangeArbitraryBuilder.js
var safeStringFromCharCode = String.fromCharCode;
var lowerCaseMapper = { num: 26, build: (v) => safeStringFromCharCode(v + 97) };
var upperCaseMapper = { num: 26, build: (v) => safeStringFromCharCode(v + 65) };
var numericMapper = { num: 10, build: (v) => safeStringFromCharCode(v + 48) };
function percentCharArbMapper(c) {
  const encoded = SencodeURIComponent(c);
  return c !== encoded
    ? encoded
    : `%${safeNumberToString(safeCharCodeAt(c, 0), 16)}`;
}
function percentCharArbUnmapper(value) {
  if (typeof value !== 'string') {
    throw new Error('Unsupported');
  }
  const decoded = decodeURIComponent(value);
  return decoded;
}
var percentCharArb = fullUnicode().map(
  percentCharArbMapper,
  percentCharArbUnmapper,
);
var buildLowerAlphaArbitrary = (others) =>
  mapToConstant(lowerCaseMapper, {
    num: others.length,
    build: (v) => others[v],
  });
var buildLowerAlphaNumericArbitrary = (others) =>
  mapToConstant(lowerCaseMapper, numericMapper, {
    num: others.length,
    build: (v) => others[v],
  });
var buildAlphaNumericArbitrary = (others) =>
  mapToConstant(lowerCaseMapper, upperCaseMapper, numericMapper, {
    num: others.length,
    build: (v) => others[v],
  });
var buildAlphaNumericPercentArbitrary = (others) =>
  oneof(
    { weight: 10, arbitrary: buildAlphaNumericArbitrary(others) },
    { weight: 1, arbitrary: percentCharArb },
  );

// ../../../node_modules/fast-check/lib/esm/arbitrary/option.js
function option(arb, constraints = {}) {
  const freq = constraints.freq == null ? 5 : constraints.freq;
  const nilValue = safeHasOwnProperty(constraints, 'nil')
    ? constraints.nil
    : null;
  const nilArb = constant(nilValue);
  const weightedArbs = [
    { arbitrary: nilArb, weight: 1, fallbackValue: { default: nilValue } },
    { arbitrary: arb, weight: freq },
  ];
  const frequencyConstraints = {
    withCrossShrink: true,
    depthSize: constraints.depthSize,
    maxDepth: constraints.maxDepth,
    depthIdentifier: constraints.depthIdentifier,
  };
  return FrequencyArbitrary.from(
    weightedArbs,
    frequencyConstraints,
    'fc.option',
  );
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/_internals/mappers/PatternsToString.js
function patternsToStringMapper(tab) {
  return safeJoin(tab, '');
}
function patternsToStringUnmapperFor(patternsArb, constraints) {
  return function patternsToStringUnmapper(value) {
    if (typeof value !== 'string') {
      throw new Error('Unsupported value');
    }
    const minLength =
      constraints.minLength !== void 0 ? constraints.minLength : 0;
    const maxLength =
      constraints.maxLength !== void 0
        ? constraints.maxLength
        : MaxLengthUpperBound;
    if (value.length === 0) {
      if (minLength > 0) {
        throw new Error('Unable to unmap received string');
      }
      return [];
    }
    const stack = [{ endIndexChunks: 0, nextStartIndex: 1, chunks: [] }];
    while (stack.length > 0) {
      const last = safePop(stack);
      for (let index = last.nextStartIndex; index <= value.length; ++index) {
        const chunk = safeSubstring(value, last.endIndexChunks, index);
        if (patternsArb.canShrinkWithoutContext(chunk)) {
          const newChunks = [...last.chunks, chunk];
          if (index === value.length) {
            if (newChunks.length < minLength || newChunks.length > maxLength) {
              break;
            }
            return newChunks;
          }
          safePush(stack, {
            endIndexChunks: last.endIndexChunks,
            nextStartIndex: index + 1,
            chunks: last.chunks,
          });
          safePush(stack, {
            endIndexChunks: index,
            nextStartIndex: index + 1,
            chunks: newChunks,
          });
          break;
        }
      }
    }
    throw new Error('Unable to unmap received string');
  };
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/_internals/helpers/SlicesForStringBuilder.js
var dangerousStrings = [
  '__defineGetter__',
  '__defineSetter__',
  '__lookupGetter__',
  '__lookupSetter__',
  '__proto__',
  'constructor',
  'hasOwnProperty',
  'isPrototypeOf',
  'propertyIsEnumerable',
  'toLocaleString',
  'toString',
  'valueOf',
  'apply',
  'arguments',
  'bind',
  'call',
  'caller',
  'length',
  'name',
  'prototype',
  'key',
  'ref',
];
function computeCandidateString(dangerous, charArbitrary, stringSplitter) {
  let candidate;
  try {
    candidate = stringSplitter(dangerous);
  } catch (err) {
    return void 0;
  }
  for (const entry of candidate) {
    if (!charArbitrary.canShrinkWithoutContext(entry)) {
      return void 0;
    }
  }
  return candidate;
}
function createSlicesForString(charArbitrary, stringSplitter) {
  const slicesForString = [];
  for (const dangerous of dangerousStrings) {
    const candidate = computeCandidateString(
      dangerous,
      charArbitrary,
      stringSplitter,
    );
    if (candidate !== void 0) {
      safePush(slicesForString, candidate);
    }
  }
  return slicesForString;
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/stringOf.js
var safeObjectAssign4 = Object.assign;
function stringOf(charArb, constraints = {}) {
  const unmapper = patternsToStringUnmapperFor(charArb, constraints);
  const experimentalCustomSlices = createSlicesForString(charArb, unmapper);
  const enrichedConstraints = safeObjectAssign4(
    safeObjectAssign4({}, constraints),
    {
      experimentalCustomSlices,
    },
  );
  return array(charArb, enrichedConstraints).map(
    patternsToStringMapper,
    unmapper,
  );
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/_internals/helpers/InvalidSubdomainLabelFiIter.js
function filterInvalidSubdomainLabel(subdomainLabel2) {
  if (subdomainLabel2.length > 63) {
    return false;
  }
  return (
    subdomainLabel2.length < 4 ||
    subdomainLabel2[0] !== 'x' ||
    subdomainLabel2[1] !== 'n' ||
    subdomainLabel2[2] !== '-' ||
    subdomainLabel2[3] !== '-'
  );
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/_internals/AdapterArbitrary.js
var AdaptedValue = Symbol('adapted-value');
function toAdapterValue(rawValue, adapter2) {
  const adapted = adapter2(rawValue.value_);
  if (!adapted.adapted) {
    return rawValue;
  }
  return new Value(adapted.value, AdaptedValue);
}
var AdapterArbitrary = class extends Arbitrary {
  constructor(sourceArb, adapter2) {
    super();
    this.sourceArb = sourceArb;
    this.adapter = adapter2;
    this.adaptValue = (rawValue) => toAdapterValue(rawValue, adapter2);
  }
  generate(mrng, biasFactor) {
    const rawValue = this.sourceArb.generate(mrng, biasFactor);
    return this.adaptValue(rawValue);
  }
  canShrinkWithoutContext(value) {
    return (
      this.sourceArb.canShrinkWithoutContext(value) &&
      !this.adapter(value).adapted
    );
  }
  shrink(value, context2) {
    if (context2 === AdaptedValue) {
      if (!this.sourceArb.canShrinkWithoutContext(value)) {
        return Stream.nil();
      }
      return this.sourceArb.shrink(value, void 0).map(this.adaptValue);
    }
    return this.sourceArb.shrink(value, context2).map(this.adaptValue);
  }
};
function adapter(sourceArb, adapter2) {
  return new AdapterArbitrary(sourceArb, adapter2);
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/domain.js
function toSubdomainLabelMapper([f, d]) {
  return d === null ? f : `${f}${d[0]}${d[1]}`;
}
function toSubdomainLabelUnmapper(value) {
  if (typeof value !== 'string' || value.length === 0) {
    throw new Error('Unsupported');
  }
  if (value.length === 1) {
    return [value[0], null];
  }
  return [
    value[0],
    [safeSubstring(value, 1, value.length - 1), value[value.length - 1]],
  ];
}
function subdomainLabel(size) {
  const alphaNumericArb = buildLowerAlphaNumericArbitrary([]);
  const alphaNumericHyphenArb = buildLowerAlphaNumericArbitrary(['-']);
  return tuple(
    alphaNumericArb,
    option(
      tuple(
        stringOf(alphaNumericHyphenArb, { size, maxLength: 61 }),
        alphaNumericArb,
      ),
    ),
  )
    .map(toSubdomainLabelMapper, toSubdomainLabelUnmapper)
    .filter(filterInvalidSubdomainLabel);
}
function labelsMapper(elements) {
  return `${safeJoin(elements[0], '.')}.${elements[1]}`;
}
function labelsUnmapper(value) {
  if (typeof value !== 'string') {
    throw new Error('Unsupported type');
  }
  const lastDotIndex = value.lastIndexOf('.');
  return [
    safeSplit(safeSubstring(value, 0, lastDotIndex), '.'),
    safeSubstring(value, lastDotIndex + 1),
  ];
}
function labelsAdapter(labels) {
  const [subDomains, suffix] = labels;
  let lengthNotIncludingIndex = suffix.length;
  for (let index = 0; index !== subDomains.length; ++index) {
    lengthNotIncludingIndex += 1 + subDomains[index].length;
    if (lengthNotIncludingIndex > 255) {
      return {
        adapted: true,
        value: [safeSlice(subDomains, 0, index), suffix],
      };
    }
  }
  return { adapted: false, value: labels };
}
function domain(constraints = {}) {
  const resolvedSize = resolveSize(constraints.size);
  const resolvedSizeMinusOne = relativeSizeToSize('-1', resolvedSize);
  const alphaNumericArb = buildLowerAlphaArbitrary([]);
  const publicSuffixArb = stringOf(alphaNumericArb, {
    minLength: 2,
    maxLength: 63,
    size: resolvedSizeMinusOne,
  });
  return adapter(
    tuple(
      array(subdomainLabel(resolvedSize), {
        size: resolvedSizeMinusOne,
        minLength: 1,
        maxLength: 127,
      }),
      publicSuffixArb,
    ),
    labelsAdapter,
  ).map(labelsMapper, labelsUnmapper);
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/emailAddress.js
function dotAdapter(a) {
  let currentLength = a[0].length;
  for (let index = 1; index !== a.length; ++index) {
    currentLength += 1 + a[index].length;
    if (currentLength > 64) {
      return { adapted: true, value: safeSlice(a, 0, index) };
    }
  }
  return { adapted: false, value: a };
}
function dotMapper(a) {
  return safeJoin(a, '.');
}
function dotUnmapper(value) {
  if (typeof value !== 'string') {
    throw new Error('Unsupported');
  }
  return safeSplit(value, '.');
}
function atMapper(data) {
  return `${data[0]}@${data[1]}`;
}
function atUnmapper(value) {
  if (typeof value !== 'string') {
    throw new Error('Unsupported');
  }
  return safeSplit(value, '@', 2);
}
function emailAddress(constraints = {}) {
  const others = [
    '!',
    '#',
    '$',
    '%',
    '&',
    "'",
    '*',
    '+',
    '-',
    '/',
    '=',
    '?',
    '^',
    '_',
    '`',
    '{',
    '|',
    '}',
    '~',
  ];
  const atextArb = buildLowerAlphaNumericArbitrary(others);
  const localPartArb = adapter(
    array(
      stringOf(atextArb, {
        minLength: 1,
        maxLength: 64,
        size: constraints.size,
      }),
      { minLength: 1, maxLength: 32, size: constraints.size },
    ),
    dotAdapter,
  ).map(dotMapper, dotUnmapper);
  return tuple(localPartArb, domain({ size: constraints.size })).map(
    atMapper,
    atUnmapper,
  );
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/_internals/helpers/ArrayInt64.js
var Zero64 = { sign: 1, data: [0, 0] };
var Unit64 = { sign: 1, data: [0, 1] };
function isZero64(a) {
  return a.data[0] === 0 && a.data[1] === 0;
}
function isStrictlyNegative64(a) {
  return a.sign === -1 && !isZero64(a);
}
function isStrictlyPositive64(a) {
  return a.sign === 1 && !isZero64(a);
}
function isEqual64(a, b) {
  if (a.data[0] === b.data[0] && a.data[1] === b.data[1]) {
    return a.sign === b.sign || (a.data[0] === 0 && a.data[1] === 0);
  }
  return false;
}
function isStrictlySmaller64Internal(a, b) {
  return a[0] < b[0] || (a[0] === b[0] && a[1] < b[1]);
}
function isStrictlySmaller64(a, b) {
  if (a.sign === b.sign) {
    return a.sign === 1
      ? isStrictlySmaller64Internal(a.data, b.data)
      : isStrictlySmaller64Internal(b.data, a.data);
  }
  return a.sign === -1 && (!isZero64(a) || !isZero64(b));
}
function clone64(a) {
  return { sign: a.sign, data: [a.data[0], a.data[1]] };
}
function substract64DataInternal(a, b) {
  let reminderLow = 0;
  let low = a[1] - b[1];
  if (low < 0) {
    reminderLow = 1;
    low = low >>> 0;
  }
  return [a[0] - b[0] - reminderLow, low];
}
function substract64Internal(a, b) {
  if (a.sign === 1 && b.sign === -1) {
    const low = a.data[1] + b.data[1];
    const high = a.data[0] + b.data[0] + (low > 4294967295 ? 1 : 0);
    return { sign: 1, data: [high >>> 0, low >>> 0] };
  }
  return {
    sign: 1,
    data:
      a.sign === 1
        ? substract64DataInternal(a.data, b.data)
        : substract64DataInternal(b.data, a.data),
  };
}
function substract64(arrayIntA, arrayIntB) {
  if (isStrictlySmaller64(arrayIntA, arrayIntB)) {
    const out = substract64Internal(arrayIntB, arrayIntA);
    out.sign = -1;
    return out;
  }
  return substract64Internal(arrayIntA, arrayIntB);
}
function negative64(arrayIntA) {
  return {
    sign: -arrayIntA.sign,
    data: [arrayIntA.data[0], arrayIntA.data[1]],
  };
}
function add64(arrayIntA, arrayIntB) {
  if (isZero64(arrayIntB)) {
    if (isZero64(arrayIntA)) {
      return clone64(Zero64);
    }
    return clone64(arrayIntA);
  }
  return substract64(arrayIntA, negative64(arrayIntB));
}
function halve64(a) {
  return {
    sign: a.sign,
    data: [
      Math.floor(a.data[0] / 2),
      (a.data[0] % 2 === 1 ? 2147483648 : 0) + Math.floor(a.data[1] / 2),
    ],
  };
}
function logLike64(a) {
  return {
    sign: a.sign,
    data: [
      0,
      Math.floor(Math.log(a.data[0] * 4294967296 + a.data[1]) / Math.log(2)),
    ],
  };
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/_internals/ArrayInt64Arbitrary.js
var ArrayInt64Arbitrary = class _ArrayInt64Arbitrary extends Arbitrary {
  constructor(min, max) {
    super();
    this.min = min;
    this.max = max;
    this.biasedRanges = null;
  }
  generate(mrng, biasFactor) {
    const range = this.computeGenerateRange(mrng, biasFactor);
    const uncheckedValue = mrng.nextArrayInt(range.min, range.max);
    if (uncheckedValue.data.length === 1) {
      uncheckedValue.data.unshift(0);
    }
    return new Value(uncheckedValue, void 0);
  }
  computeGenerateRange(mrng, biasFactor) {
    if (biasFactor === void 0 || mrng.nextInt(1, biasFactor) !== 1) {
      return { min: this.min, max: this.max };
    }
    const ranges = this.retrieveBiasedRanges();
    if (ranges.length === 1) {
      return ranges[0];
    }
    const id = mrng.nextInt(-2 * (ranges.length - 1), ranges.length - 2);
    return id < 0 ? ranges[0] : ranges[id + 1];
  }
  canShrinkWithoutContext(value) {
    const unsafeValue = value;
    return (
      typeof value === 'object' &&
      value !== null &&
      (unsafeValue.sign === -1 || unsafeValue.sign === 1) &&
      Array.isArray(unsafeValue.data) &&
      unsafeValue.data.length === 2 &&
      ((isStrictlySmaller64(this.min, unsafeValue) &&
        isStrictlySmaller64(unsafeValue, this.max)) ||
        isEqual64(this.min, unsafeValue) ||
        isEqual64(this.max, unsafeValue))
    );
  }
  shrinkArrayInt64(value, target, tryTargetAsap) {
    const realGap = substract64(value, target);
    function* shrinkGen() {
      let previous = tryTargetAsap ? void 0 : target;
      const gap = tryTargetAsap ? realGap : halve64(realGap);
      for (
        let toremove = gap;
        !isZero64(toremove);
        toremove = halve64(toremove)
      ) {
        const next = substract64(value, toremove);
        yield new Value(next, previous);
        previous = next;
      }
    }
    return stream(shrinkGen());
  }
  shrink(current, context2) {
    if (!_ArrayInt64Arbitrary.isValidContext(current, context2)) {
      const target = this.defaultTarget();
      return this.shrinkArrayInt64(current, target, true);
    }
    if (this.isLastChanceTry(current, context2)) {
      return Stream.of(new Value(context2, void 0));
    }
    return this.shrinkArrayInt64(current, context2, false);
  }
  defaultTarget() {
    if (!isStrictlyPositive64(this.min) && !isStrictlyNegative64(this.max)) {
      return Zero64;
    }
    return isStrictlyNegative64(this.min) ? this.max : this.min;
  }
  isLastChanceTry(current, context2) {
    if (isZero64(current)) {
      return false;
    }
    if (current.sign === 1) {
      return (
        isEqual64(current, add64(context2, Unit64)) &&
        isStrictlyPositive64(substract64(current, this.min))
      );
    } else {
      return (
        isEqual64(current, substract64(context2, Unit64)) &&
        isStrictlyNegative64(substract64(current, this.max))
      );
    }
  }
  static isValidContext(_current, context2) {
    if (context2 === void 0) {
      return false;
    }
    if (
      typeof context2 !== 'object' ||
      context2 === null ||
      !('sign' in context2) ||
      !('data' in context2)
    ) {
      throw new Error(
        `Invalid context type passed to ArrayInt64Arbitrary (#1)`,
      );
    }
    return true;
  }
  retrieveBiasedRanges() {
    if (this.biasedRanges != null) {
      return this.biasedRanges;
    }
    if (isEqual64(this.min, this.max)) {
      this.biasedRanges = [{ min: this.min, max: this.max }];
      return this.biasedRanges;
    }
    const minStrictlySmallerZero = isStrictlyNegative64(this.min);
    const maxStrictlyGreaterZero = isStrictlyPositive64(this.max);
    if (minStrictlySmallerZero && maxStrictlyGreaterZero) {
      const logMin = logLike64(this.min);
      const logMax = logLike64(this.max);
      this.biasedRanges = [
        { min: logMin, max: logMax },
        { min: substract64(this.max, logMax), max: this.max },
        { min: this.min, max: substract64(this.min, logMin) },
      ];
    } else {
      const logGap = logLike64(substract64(this.max, this.min));
      const arbCloseToMin = { min: this.min, max: add64(this.min, logGap) };
      const arbCloseToMax = {
        min: substract64(this.max, logGap),
        max: this.max,
      };
      this.biasedRanges = minStrictlySmallerZero
        ? [arbCloseToMax, arbCloseToMin]
        : [arbCloseToMin, arbCloseToMax];
    }
    return this.biasedRanges;
  }
};
function arrayInt64(min, max) {
  const arb = new ArrayInt64Arbitrary(min, max);
  return arb;
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/_internals/helpers/DoubleHelpers.js
var safeNegativeInfinity2 = Number.NEGATIVE_INFINITY;
var safePositiveInfinity3 = Number.POSITIVE_INFINITY;
var safeEpsilon = Number.EPSILON;
var INDEX_POSITIVE_INFINITY = { sign: 1, data: [2146435072, 0] };
var INDEX_NEGATIVE_INFINITY = { sign: -1, data: [2146435072, 1] };
var f64 = new Float64Array(1);
var u32 = new Uint32Array(f64.buffer, f64.byteOffset);
function bitCastDoubleToUInt64(f) {
  f64[0] = f;
  return [u32[1], u32[0]];
}
function decomposeDouble(d) {
  const { 0: hi, 1: lo } = bitCastDoubleToUInt64(d);
  const signBit = hi >>> 31;
  const exponentBits = (hi >>> 20) & 2047;
  const significandBits = (hi & 1048575) * 4294967296 + lo;
  const exponent = exponentBits === 0 ? -1022 : exponentBits - 1023;
  let significand = exponentBits === 0 ? 0 : 1;
  significand += significandBits / 2 ** 52;
  significand *= signBit === 0 ? 1 : -1;
  return { exponent, significand };
}
function positiveNumberToInt64(n) {
  return [~~(n / 4294967296), n >>> 0];
}
function indexInDoubleFromDecomp(exponent, significand) {
  if (exponent === -1022) {
    const rescaledSignificand2 = significand * 2 ** 52;
    return positiveNumberToInt64(rescaledSignificand2);
  }
  const rescaledSignificand = (significand - 1) * 2 ** 52;
  const exponentOnlyHigh = (exponent + 1023) * 2 ** 20;
  const index = positiveNumberToInt64(rescaledSignificand);
  index[0] += exponentOnlyHigh;
  return index;
}
function doubleToIndex(d) {
  if (d === safePositiveInfinity3) {
    return clone64(INDEX_POSITIVE_INFINITY);
  }
  if (d === safeNegativeInfinity2) {
    return clone64(INDEX_NEGATIVE_INFINITY);
  }
  const decomp = decomposeDouble(d);
  const exponent = decomp.exponent;
  const significand = decomp.significand;
  if (d > 0 || (d === 0 && 1 / d === safePositiveInfinity3)) {
    return { sign: 1, data: indexInDoubleFromDecomp(exponent, significand) };
  } else {
    const indexOpposite = indexInDoubleFromDecomp(exponent, -significand);
    if (indexOpposite[1] === 4294967295) {
      indexOpposite[0] += 1;
      indexOpposite[1] = 0;
    } else {
      indexOpposite[1] += 1;
    }
    return { sign: -1, data: indexOpposite };
  }
}
function indexToDouble(index) {
  if (index.sign === -1) {
    const indexOpposite = { sign: 1, data: [index.data[0], index.data[1]] };
    if (indexOpposite.data[1] === 0) {
      indexOpposite.data[0] -= 1;
      indexOpposite.data[1] = 4294967295;
    } else {
      indexOpposite.data[1] -= 1;
    }
    return -indexToDouble(indexOpposite);
  }
  if (isEqual64(index, INDEX_POSITIVE_INFINITY)) {
    return safePositiveInfinity3;
  }
  if (index.data[0] < 2097152) {
    return (index.data[0] * 4294967296 + index.data[1]) * 2 ** -1074;
  }
  const postIndexHigh = index.data[0] - 2097152;
  const exponent = -1021 + (postIndexHigh >> 20);
  const significand =
    1 + ((postIndexHigh & 1048575) * 2 ** 32 + index.data[1]) * safeEpsilon;
  return significand * 2 ** exponent;
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/double.js
var safeNumberIsNaN5 = Number.isNaN;
var safeNegativeInfinity3 = Number.NEGATIVE_INFINITY;
var safePositiveInfinity4 = Number.POSITIVE_INFINITY;
var safeMaxValue = Number.MAX_VALUE;
var safeNaN2 = Number.NaN;
function safeDoubleToIndex(d, constraintsLabel) {
  if (safeNumberIsNaN5(d)) {
    throw new Error(
      'fc.double constraints.' + constraintsLabel + ' must be a 64-bit float',
    );
  }
  return doubleToIndex(d);
}
function unmapperDoubleToIndex(value) {
  if (typeof value !== 'number') throw new Error('Unsupported type');
  return doubleToIndex(value);
}
function double(constraints = {}) {
  const {
    noDefaultInfinity = false,
    noNaN = false,
    minExcluded = false,
    maxExcluded = false,
    min = noDefaultInfinity ? -safeMaxValue : safeNegativeInfinity3,
    max = noDefaultInfinity ? safeMaxValue : safePositiveInfinity4,
  } = constraints;
  const minIndexRaw = safeDoubleToIndex(min, 'min');
  const minIndex = minExcluded ? add64(minIndexRaw, Unit64) : minIndexRaw;
  const maxIndexRaw = safeDoubleToIndex(max, 'max');
  const maxIndex = maxExcluded ? substract64(maxIndexRaw, Unit64) : maxIndexRaw;
  if (isStrictlySmaller64(maxIndex, minIndex)) {
    throw new Error(
      'fc.double constraints.min must be smaller or equal to constraints.max',
    );
  }
  if (noNaN) {
    return arrayInt64(minIndex, maxIndex).map(
      indexToDouble,
      unmapperDoubleToIndex,
    );
  }
  const positiveMaxIdx = isStrictlyPositive64(maxIndex);
  const minIndexWithNaN = positiveMaxIdx
    ? minIndex
    : substract64(minIndex, Unit64);
  const maxIndexWithNaN = positiveMaxIdx ? add64(maxIndex, Unit64) : maxIndex;
  return arrayInt64(minIndexWithNaN, maxIndexWithNaN).map(
    (index) => {
      if (
        isStrictlySmaller64(maxIndex, index) ||
        isStrictlySmaller64(index, minIndex)
      )
        return safeNaN2;
      else return indexToDouble(index);
    },
    (value) => {
      if (typeof value !== 'number') throw new Error('Unsupported type');
      if (safeNumberIsNaN5(value))
        return !isEqual64(maxIndex, maxIndexWithNaN)
          ? maxIndexWithNaN
          : minIndexWithNaN;
      return doubleToIndex(value);
    },
  );
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/_internals/helpers/FloatHelpers.js
var safeNegativeInfinity4 = Number.NEGATIVE_INFINITY;
var safePositiveInfinity5 = Number.POSITIVE_INFINITY;
var MIN_VALUE_32 = 2 ** -126 * 2 ** -23;
var MAX_VALUE_32 = 2 ** 127 * (1 + (2 ** 23 - 1) / 2 ** 23);
var EPSILON_32 = 2 ** -23;
var INDEX_POSITIVE_INFINITY2 = 2139095040;
var INDEX_NEGATIVE_INFINITY2 = -2139095041;
var f32 = new Float32Array(1);
var u322 = new Uint32Array(f32.buffer, f32.byteOffset);
function bitCastFloatToUInt32(f) {
  f32[0] = f;
  return u322[0];
}
function decomposeFloat(f) {
  const bits = bitCastFloatToUInt32(f);
  const signBit = bits >>> 31;
  const exponentBits = (bits >>> 23) & 255;
  const significandBits = bits & 8388607;
  const exponent = exponentBits === 0 ? -126 : exponentBits - 127;
  let significand = exponentBits === 0 ? 0 : 1;
  significand += significandBits / 2 ** 23;
  significand *= signBit === 0 ? 1 : -1;
  return { exponent, significand };
}
function indexInFloatFromDecomp(exponent, significand) {
  if (exponent === -126) {
    return significand * 8388608;
  }
  return (exponent + 127) * 8388608 + (significand - 1) * 8388608;
}
function floatToIndex(f) {
  if (f === safePositiveInfinity5) {
    return INDEX_POSITIVE_INFINITY2;
  }
  if (f === safeNegativeInfinity4) {
    return INDEX_NEGATIVE_INFINITY2;
  }
  const decomp = decomposeFloat(f);
  const exponent = decomp.exponent;
  const significand = decomp.significand;
  if (f > 0 || (f === 0 && 1 / f === safePositiveInfinity5)) {
    return indexInFloatFromDecomp(exponent, significand);
  } else {
    return -indexInFloatFromDecomp(exponent, -significand) - 1;
  }
}
function indexToFloat(index) {
  if (index < 0) {
    return -indexToFloat(-index - 1);
  }
  if (index === INDEX_POSITIVE_INFINITY2) {
    return safePositiveInfinity5;
  }
  if (index < 16777216) {
    return index * 2 ** -149;
  }
  const postIndex = index - 16777216;
  const exponent = -125 + (postIndex >> 23);
  const significand = 1 + (postIndex & 8388607) / 8388608;
  return significand * 2 ** exponent;
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/float.js
var safeNumberIsNaN6 = Number.isNaN;
var safeMathFround = Math.fround;
var safeNegativeInfinity5 = Number.NEGATIVE_INFINITY;
var safePositiveInfinity6 = Number.POSITIVE_INFINITY;
var safeNaN3 = Number.NaN;
function safeFloatToIndex(f, constraintsLabel) {
  const conversionTrick =
    'you can convert any double to a 32-bit float by using `Math.fround(myDouble)`';
  const errorMessage =
    'fc.float constraints.' +
    constraintsLabel +
    ' must be a 32-bit float - ' +
    conversionTrick;
  if (safeNumberIsNaN6(f) || safeMathFround(f) !== f) {
    throw new Error(errorMessage);
  }
  return floatToIndex(f);
}
function unmapperFloatToIndex(value) {
  if (typeof value !== 'number') throw new Error('Unsupported type');
  return floatToIndex(value);
}
function float(constraints = {}) {
  const {
    noDefaultInfinity = false,
    noNaN = false,
    minExcluded = false,
    maxExcluded = false,
    min = noDefaultInfinity ? -MAX_VALUE_32 : safeNegativeInfinity5,
    max = noDefaultInfinity ? MAX_VALUE_32 : safePositiveInfinity6,
  } = constraints;
  const minIndexRaw = safeFloatToIndex(min, 'min');
  const minIndex = minExcluded ? minIndexRaw + 1 : minIndexRaw;
  const maxIndexRaw = safeFloatToIndex(max, 'max');
  const maxIndex = maxExcluded ? maxIndexRaw - 1 : maxIndexRaw;
  if (minIndex > maxIndex) {
    throw new Error(
      'fc.float constraints.min must be smaller or equal to constraints.max',
    );
  }
  if (noNaN) {
    return integer({ min: minIndex, max: maxIndex }).map(
      indexToFloat,
      unmapperFloatToIndex,
    );
  }
  const minIndexWithNaN = maxIndex > 0 ? minIndex : minIndex - 1;
  const maxIndexWithNaN = maxIndex > 0 ? maxIndex + 1 : maxIndex;
  return integer({ min: minIndexWithNaN, max: maxIndexWithNaN }).map(
    (index) => {
      if (index > maxIndex || index < minIndex) return safeNaN3;
      else return indexToFloat(index);
    },
    (value) => {
      if (typeof value !== 'number') throw new Error('Unsupported type');
      if (safeNumberIsNaN6(value))
        return maxIndex !== maxIndexWithNaN ? maxIndexWithNaN : minIndexWithNaN;
      return floatToIndex(value);
    },
  );
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/_internals/helpers/TextEscaper.js
function escapeForTemplateString(originalText) {
  return originalText.replace(/([$`\\])/g, '\\$1').replace(/\r/g, '\\r');
}
function escapeForMultilineComments(originalText) {
  return originalText.replace(/\*\//g, '*\\/');
}

// ../../../node_modules/fast-check/lib/esm/utils/hash.js
var crc32Table = [
  0, 1996959894, 3993919788, 2567524794, 124634137, 1886057615, 3915621685,
  2657392035, 249268274, 2044508324, 3772115230, 2547177864, 162941995,
  2125561021, 3887607047, 2428444049, 498536548, 1789927666, 4089016648,
  2227061214, 450548861, 1843258603, 4107580753, 2211677639, 325883990,
  1684777152, 4251122042, 2321926636, 335633487, 1661365465, 4195302755,
  2366115317, 997073096, 1281953886, 3579855332, 2724688242, 1006888145,
  1258607687, 3524101629, 2768942443, 901097722, 1119000684, 3686517206,
  2898065728, 853044451, 1172266101, 3705015759, 2882616665, 651767980,
  1373503546, 3369554304, 3218104598, 565507253, 1454621731, 3485111705,
  3099436303, 671266974, 1594198024, 3322730930, 2970347812, 795835527,
  1483230225, 3244367275, 3060149565, 1994146192, 31158534, 2563907772,
  4023717930, 1907459465, 112637215, 2680153253, 3904427059, 2013776290,
  251722036, 2517215374, 3775830040, 2137656763, 141376813, 2439277719,
  3865271297, 1802195444, 476864866, 2238001368, 4066508878, 1812370925,
  453092731, 2181625025, 4111451223, 1706088902, 314042704, 2344532202,
  4240017532, 1658658271, 366619977, 2362670323, 4224994405, 1303535960,
  984961486, 2747007092, 3569037538, 1256170817, 1037604311, 2765210733,
  3554079995, 1131014506, 879679996, 2909243462, 3663771856, 1141124467,
  855842277, 2852801631, 3708648649, 1342533948, 654459306, 3188396048,
  3373015174, 1466479909, 544179635, 3110523913, 3462522015, 1591671054,
  702138776, 2966460450, 3352799412, 1504918807, 783551873, 3082640443,
  3233442989, 3988292384, 2596254646, 62317068, 1957810842, 3939845945,
  2647816111, 81470997, 1943803523, 3814918930, 2489596804, 225274430,
  2053790376, 3826175755, 2466906013, 167816743, 2097651377, 4027552580,
  2265490386, 503444072, 1762050814, 4150417245, 2154129355, 426522225,
  1852507879, 4275313526, 2312317920, 282753626, 1742555852, 4189708143,
  2394877945, 397917763, 1622183637, 3604390888, 2714866558, 953729732,
  1340076626, 3518719985, 2797360999, 1068828381, 1219638859, 3624741850,
  2936675148, 906185462, 1090812512, 3747672003, 2825379669, 829329135,
  1181335161, 3412177804, 3160834842, 628085408, 1382605366, 3423369109,
  3138078467, 570562233, 1426400815, 3317316542, 2998733608, 733239954,
  1555261956, 3268935591, 3050360625, 752459403, 1541320221, 2607071920,
  3965973030, 1969922972, 40735498, 2617837225, 3943577151, 1913087877,
  83908371, 2512341634, 3803740692, 2075208622, 213261112, 2463272603,
  3855990285, 2094854071, 198958881, 2262029012, 4057260610, 1759359992,
  534414190, 2176718541, 4139329115, 1873836001, 414664567, 2282248934,
  4279200368, 1711684554, 285281116, 2405801727, 4167216745, 1634467795,
  376229701, 2685067896, 3608007406, 1308918612, 956543938, 2808555105,
  3495958263, 1231636301, 1047427035, 2932959818, 3654703836, 1088359270,
  936918e3, 2847714899, 3736837829, 1202900863, 817233897, 3183342108,
  3401237130, 1404277552, 615818150, 3134207493, 3453421203, 1423857449,
  601450431, 3009837614, 3294710456, 1567103746, 711928724, 3020668471,
  3272380065, 1510334235, 755167117,
];
function hash(repr) {
  let crc = 4294967295;
  for (let idx = 0; idx < repr.length; ++idx) {
    const c = safeCharCodeAt(repr, idx);
    if (c < 128) {
      crc = crc32Table[(crc & 255) ^ c] ^ (crc >> 8);
    } else if (c < 2048) {
      crc = crc32Table[(crc & 255) ^ (192 | ((c >> 6) & 31))] ^ (crc >> 8);
      crc = crc32Table[(crc & 255) ^ (128 | (c & 63))] ^ (crc >> 8);
    } else if (c >= 55296 && c < 57344) {
      const cNext = safeCharCodeAt(repr, ++idx);
      if (c >= 56320 || cNext < 56320 || cNext > 57343 || Number.isNaN(cNext)) {
        idx -= 1;
        crc = crc32Table[(crc & 255) ^ 239] ^ (crc >> 8);
        crc = crc32Table[(crc & 255) ^ 191] ^ (crc >> 8);
        crc = crc32Table[(crc & 255) ^ 189] ^ (crc >> 8);
      } else {
        const c1 = (c & 1023) + 64;
        const c2 = cNext & 1023;
        crc = crc32Table[(crc & 255) ^ (240 | ((c1 >> 8) & 7))] ^ (crc >> 8);
        crc = crc32Table[(crc & 255) ^ (128 | ((c1 >> 2) & 63))] ^ (crc >> 8);
        crc =
          crc32Table[(crc & 255) ^ (128 | ((c2 >> 6) & 15) | ((c1 & 3) << 4))] ^
          (crc >> 8);
        crc = crc32Table[(crc & 255) ^ (128 | (c2 & 63))] ^ (crc >> 8);
      }
    } else {
      crc = crc32Table[(crc & 255) ^ (224 | ((c >> 12) & 15))] ^ (crc >> 8);
      crc = crc32Table[(crc & 255) ^ (128 | ((c >> 6) & 63))] ^ (crc >> 8);
      crc = crc32Table[(crc & 255) ^ (128 | (c & 63))] ^ (crc >> 8);
    }
  }
  return (crc | 0) + 2147483648;
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/_internals/builders/CompareFunctionArbitraryBuilder.js
var safeObjectAssign5 = Object.assign;
var safeObjectKeys2 = Object.keys;
function buildCompareFunctionArbitrary(cmp) {
  return tuple(
    integer().noShrink(),
    integer({ min: 1, max: 4294967295 }).noShrink(),
  ).map(([seed, hashEnvSize]) => {
    const producer = () => {
      const recorded = {};
      const f = (a, b) => {
        const reprA = stringify(a);
        const reprB = stringify(b);
        const hA = hash(`${seed}${reprA}`) % hashEnvSize;
        const hB = hash(`${seed}${reprB}`) % hashEnvSize;
        const val = cmp(hA, hB);
        recorded[`[${reprA},${reprB}]`] = val;
        return val;
      };
      return safeObjectAssign5(f, {
        toString: () => {
          const seenValues = safeObjectKeys2(recorded)
            .sort()
            .map((k) => `${k} => ${stringify(recorded[k])}`)
            .map((line) => `/* ${escapeForMultilineComments(line)} */`);
          return `function(a, b) {
  // With hash and stringify coming from fast-check${
    seenValues.length !== 0
      ? `
  ${safeJoin(seenValues, '\n  ')}`
      : ''
  }
  const cmp = ${cmp};
  const hA = hash('${seed}' + stringify(a)) % ${hashEnvSize};
  const hB = hash('${seed}' + stringify(b)) % ${hashEnvSize};
  return cmp(hA, hB);
}`;
        },
        [cloneMethod]: producer,
      });
    };
    return producer();
  });
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/compareBooleanFunc.js
var safeObjectAssign6 = Object.assign;
function compareBooleanFunc() {
  return buildCompareFunctionArbitrary(
    safeObjectAssign6((hA, hB) => hA < hB, {
      toString() {
        return '(hA, hB) => hA < hB';
      },
    }),
  );
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/compareFunc.js
var safeObjectAssign7 = Object.assign;
function compareFunc() {
  return buildCompareFunctionArbitrary(
    safeObjectAssign7((hA, hB) => hA - hB, {
      toString() {
        return '(hA, hB) => hA - hB';
      },
    }),
  );
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/func.js
var safeObjectDefineProperties = Object.defineProperties;
var safeObjectKeys3 = Object.keys;
function func(arb) {
  return tuple(array(arb, { minLength: 1 }), integer().noShrink()).map(
    ([outs, seed]) => {
      const producer = () => {
        const recorded = {};
        const f = (...args) => {
          const repr = stringify(args);
          const val = outs[hash(`${seed}${repr}`) % outs.length];
          recorded[repr] = val;
          return hasCloneMethod(val) ? val[cloneMethod]() : val;
        };
        function prettyPrint2(stringifiedOuts) {
          const seenValues = safeMap(
            safeMap(
              safeSort(safeObjectKeys3(recorded)),
              (k) => `${k} => ${stringify(recorded[k])}`,
            ),
            (line) => `/* ${escapeForMultilineComments(line)} */`,
          );
          return `function(...args) {
  // With hash and stringify coming from fast-check${
    seenValues.length !== 0
      ? `
  ${seenValues.join('\n  ')}`
      : ''
  }
  const outs = ${stringifiedOuts};
  return outs[hash('${seed}' + stringify(args)) % outs.length];
}`;
        }
        return safeObjectDefineProperties(f, {
          toString: { value: () => prettyPrint2(stringify(outs)) },
          [toStringMethod]: { value: () => prettyPrint2(stringify(outs)) },
          [asyncToStringMethod]: {
            value: async () => prettyPrint2(await asyncStringify(outs)),
          },
          [cloneMethod]: { value: producer, configurable: true },
        });
      };
      return producer();
    },
  );
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/maxSafeInteger.js
var safeMinSafeInteger = Number.MIN_SAFE_INTEGER;
var safeMaxSafeInteger2 = Number.MAX_SAFE_INTEGER;
function maxSafeInteger() {
  return new IntegerArbitrary(safeMinSafeInteger, safeMaxSafeInteger2);
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/maxSafeNat.js
var safeMaxSafeInteger3 = Number.MAX_SAFE_INTEGER;
function maxSafeNat() {
  return new IntegerArbitrary(0, safeMaxSafeInteger3);
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/_internals/mappers/NatToStringifiedNat.js
var safeNumberParseInt = Number.parseInt;
function natToStringifiedNatMapper(options) {
  const [style, v] = options;
  switch (style) {
    case 'oct':
      return `0${safeNumberToString(v, 8)}`;
    case 'hex':
      return `0x${safeNumberToString(v, 16)}`;
    case 'dec':
    default:
      return `${v}`;
  }
}
function tryParseStringifiedNat(stringValue, radix) {
  const parsedNat = safeNumberParseInt(stringValue, radix);
  if (safeNumberToString(parsedNat, radix) !== stringValue) {
    throw new Error('Invalid value');
  }
  return parsedNat;
}
function natToStringifiedNatUnmapper(value) {
  if (typeof value !== 'string') {
    throw new Error('Invalid type');
  }
  if (value.length >= 2 && value[0] === '0') {
    if (value[1] === 'x') {
      return ['hex', tryParseStringifiedNat(safeSubstring(value, 2), 16)];
    }
    return ['oct', tryParseStringifiedNat(safeSubstring(value, 1), 8)];
  }
  return ['dec', tryParseStringifiedNat(value, 10)];
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/ipV4.js
function dotJoinerMapper(data) {
  return safeJoin(data, '.');
}
function dotJoinerUnmapper(value) {
  if (typeof value !== 'string') {
    throw new Error('Invalid type');
  }
  return safeMap(safeSplit(value, '.'), (v) => tryParseStringifiedNat(v, 10));
}
function ipV4() {
  return tuple(nat(255), nat(255), nat(255), nat(255)).map(
    dotJoinerMapper,
    dotJoinerUnmapper,
  );
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/_internals/builders/StringifiedNatArbitraryBuilder.js
function buildStringifiedNatArbitrary(maxValue) {
  return tuple(constantFrom('dec', 'oct', 'hex'), nat(maxValue)).map(
    natToStringifiedNatMapper,
    natToStringifiedNatUnmapper,
  );
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/ipV4Extended.js
function dotJoinerMapper2(data) {
  return safeJoin(data, '.');
}
function dotJoinerUnmapper2(value) {
  if (typeof value !== 'string') {
    throw new Error('Invalid type');
  }
  return safeSplit(value, '.');
}
function ipV4Extended() {
  return oneof(
    tuple(
      buildStringifiedNatArbitrary(255),
      buildStringifiedNatArbitrary(255),
      buildStringifiedNatArbitrary(255),
      buildStringifiedNatArbitrary(255),
    ).map(dotJoinerMapper2, dotJoinerUnmapper2),
    tuple(
      buildStringifiedNatArbitrary(255),
      buildStringifiedNatArbitrary(255),
      buildStringifiedNatArbitrary(65535),
    ).map(dotJoinerMapper2, dotJoinerUnmapper2),
    tuple(
      buildStringifiedNatArbitrary(255),
      buildStringifiedNatArbitrary(16777215),
    ).map(dotJoinerMapper2, dotJoinerUnmapper2),
    buildStringifiedNatArbitrary(4294967295),
  );
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/_internals/mappers/CodePointsToString.js
function codePointsToStringMapper(tab) {
  return safeJoin(tab, '');
}
function codePointsToStringUnmapper(value) {
  if (typeof value !== 'string') {
    throw new Error('Cannot unmap the passed value');
  }
  return [...value];
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/hexaString.js
var safeObjectAssign8 = Object.assign;
function hexaString(constraints = {}) {
  const charArbitrary = hexa();
  const experimentalCustomSlices = createSlicesForString(
    charArbitrary,
    codePointsToStringUnmapper,
  );
  const enrichedConstraints = safeObjectAssign8(
    safeObjectAssign8({}, constraints),
    {
      experimentalCustomSlices,
    },
  );
  return array(charArbitrary, enrichedConstraints).map(
    codePointsToStringMapper,
    codePointsToStringUnmapper,
  );
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/_internals/mappers/EntitiesToIPv6.js
function readBh(value) {
  if (value.length === 0) return [];
  else return safeSplit(value, ':');
}
function extractEhAndL(value) {
  const valueSplits = safeSplit(value, ':');
  if (
    valueSplits.length >= 2 &&
    valueSplits[valueSplits.length - 1].length <= 4
  ) {
    return [
      safeSlice(valueSplits, 0, valueSplits.length - 2),
      `${valueSplits[valueSplits.length - 2]}:${valueSplits[valueSplits.length - 1]}`,
    ];
  }
  return [
    safeSlice(valueSplits, 0, valueSplits.length - 1),
    valueSplits[valueSplits.length - 1],
  ];
}
function fullySpecifiedMapper(data) {
  return `${safeJoin(data[0], ':')}:${data[1]}`;
}
function fullySpecifiedUnmapper(value) {
  if (typeof value !== 'string') throw new Error('Invalid type');
  return extractEhAndL(value);
}
function onlyTrailingMapper(data) {
  return `::${safeJoin(data[0], ':')}:${data[1]}`;
}
function onlyTrailingUnmapper(value) {
  if (typeof value !== 'string') throw new Error('Invalid type');
  if (!safeStartsWith(value, '::')) throw new Error('Invalid value');
  return extractEhAndL(safeSubstring(value, 2));
}
function multiTrailingMapper(data) {
  return `${safeJoin(data[0], ':')}::${safeJoin(data[1], ':')}:${data[2]}`;
}
function multiTrailingUnmapper(value) {
  if (typeof value !== 'string') throw new Error('Invalid type');
  const [bhString, trailingString] = safeSplit(value, '::', 2);
  const [eh, l] = extractEhAndL(trailingString);
  return [readBh(bhString), eh, l];
}
function multiTrailingMapperOne(data) {
  return multiTrailingMapper([data[0], [data[1]], data[2]]);
}
function multiTrailingUnmapperOne(value) {
  const out = multiTrailingUnmapper(value);
  return [out[0], safeJoin(out[1], ':'), out[2]];
}
function singleTrailingMapper(data) {
  return `${safeJoin(data[0], ':')}::${data[1]}`;
}
function singleTrailingUnmapper(value) {
  if (typeof value !== 'string') throw new Error('Invalid type');
  const [bhString, trailing] = safeSplit(value, '::', 2);
  return [readBh(bhString), trailing];
}
function noTrailingMapper(data) {
  return `${safeJoin(data[0], ':')}::`;
}
function noTrailingUnmapper(value) {
  if (typeof value !== 'string') throw new Error('Invalid type');
  if (!safeEndsWith(value, '::')) throw new Error('Invalid value');
  return [readBh(safeSubstring(value, 0, value.length - 2))];
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/ipV6.js
function h16sTol32Mapper([a, b]) {
  return `${a}:${b}`;
}
function h16sTol32Unmapper(value) {
  if (typeof value !== 'string') throw new Error('Invalid type');
  if (!value.includes(':')) throw new Error('Invalid value');
  return value.split(':', 2);
}
function ipV6() {
  const h16Arb = hexaString({ minLength: 1, maxLength: 4, size: 'max' });
  const ls32Arb = oneof(
    tuple(h16Arb, h16Arb).map(h16sTol32Mapper, h16sTol32Unmapper),
    ipV4(),
  );
  return oneof(
    tuple(
      array(h16Arb, { minLength: 6, maxLength: 6, size: 'max' }),
      ls32Arb,
    ).map(fullySpecifiedMapper, fullySpecifiedUnmapper),
    tuple(
      array(h16Arb, { minLength: 5, maxLength: 5, size: 'max' }),
      ls32Arb,
    ).map(onlyTrailingMapper, onlyTrailingUnmapper),
    tuple(
      array(h16Arb, { minLength: 0, maxLength: 1, size: 'max' }),
      array(h16Arb, { minLength: 4, maxLength: 4, size: 'max' }),
      ls32Arb,
    ).map(multiTrailingMapper, multiTrailingUnmapper),
    tuple(
      array(h16Arb, { minLength: 0, maxLength: 2, size: 'max' }),
      array(h16Arb, { minLength: 3, maxLength: 3, size: 'max' }),
      ls32Arb,
    ).map(multiTrailingMapper, multiTrailingUnmapper),
    tuple(
      array(h16Arb, { minLength: 0, maxLength: 3, size: 'max' }),
      array(h16Arb, { minLength: 2, maxLength: 2, size: 'max' }),
      ls32Arb,
    ).map(multiTrailingMapper, multiTrailingUnmapper),
    tuple(
      array(h16Arb, { minLength: 0, maxLength: 4, size: 'max' }),
      h16Arb,
      ls32Arb,
    ).map(multiTrailingMapperOne, multiTrailingUnmapperOne),
    tuple(
      array(h16Arb, { minLength: 0, maxLength: 5, size: 'max' }),
      ls32Arb,
    ).map(singleTrailingMapper, singleTrailingUnmapper),
    tuple(
      array(h16Arb, { minLength: 0, maxLength: 6, size: 'max' }),
      h16Arb,
    ).map(singleTrailingMapper, singleTrailingUnmapper),
    tuple(array(h16Arb, { minLength: 0, maxLength: 7, size: 'max' })).map(
      noTrailingMapper,
      noTrailingUnmapper,
    ),
  );
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/_internals/LazyArbitrary.js
var LazyArbitrary = class extends Arbitrary {
  constructor(name) {
    super();
    this.name = name;
    this.underlying = null;
  }
  generate(mrng, biasFactor) {
    if (!this.underlying) {
      throw new Error(
        `Lazy arbitrary ${JSON.stringify(this.name)} not correctly initialized`,
      );
    }
    return this.underlying.generate(mrng, biasFactor);
  }
  canShrinkWithoutContext(value) {
    if (!this.underlying) {
      throw new Error(
        `Lazy arbitrary ${JSON.stringify(this.name)} not correctly initialized`,
      );
    }
    return this.underlying.canShrinkWithoutContext(value);
  }
  shrink(value, context2) {
    if (!this.underlying) {
      throw new Error(
        `Lazy arbitrary ${JSON.stringify(this.name)} not correctly initialized`,
      );
    }
    return this.underlying.shrink(value, context2);
  }
};

// ../../../node_modules/fast-check/lib/esm/arbitrary/letrec.js
var safeObjectCreate2 = Object.create;
function letrec(builder) {
  const lazyArbs = safeObjectCreate2(null);
  const tie = (key) => {
    if (!safeHasOwnProperty(lazyArbs, key)) {
      lazyArbs[key] = new LazyArbitrary(String(key));
    }
    return lazyArbs[key];
  };
  const strictArbs = builder(tie);
  for (const key in strictArbs) {
    if (!safeHasOwnProperty(strictArbs, key)) {
      continue;
    }
    const lazyAtKey = lazyArbs[key];
    const lazyArb = lazyAtKey !== void 0 ? lazyAtKey : new LazyArbitrary(key);
    lazyArb.underlying = strictArbs[key];
    lazyArbs[key] = lazyArb;
  }
  return strictArbs;
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/_internals/mappers/WordsToLorem.js
function wordsToJoinedStringMapper(words) {
  return safeJoin(
    safeMap(words, (w) =>
      w[w.length - 1] === ',' ? safeSubstring(w, 0, w.length - 1) : w,
    ),
    ' ',
  );
}
function wordsToJoinedStringUnmapperFor(wordsArbitrary) {
  return function wordsToJoinedStringUnmapper(value) {
    if (typeof value !== 'string') {
      throw new Error('Unsupported type');
    }
    const words = [];
    for (const candidate of safeSplit(value, ' ')) {
      if (wordsArbitrary.canShrinkWithoutContext(candidate))
        safePush(words, candidate);
      else if (wordsArbitrary.canShrinkWithoutContext(candidate + ','))
        safePush(words, candidate + ',');
      else throw new Error('Unsupported word');
    }
    return words;
  };
}
function wordsToSentenceMapper(words) {
  let sentence = safeJoin(words, ' ');
  if (sentence[sentence.length - 1] === ',') {
    sentence = safeSubstring(sentence, 0, sentence.length - 1);
  }
  return safeToUpperCase(sentence[0]) + safeSubstring(sentence, 1) + '.';
}
function wordsToSentenceUnmapperFor(wordsArbitrary) {
  return function wordsToSentenceUnmapper(value) {
    if (typeof value !== 'string') {
      throw new Error('Unsupported type');
    }
    if (
      value.length < 2 ||
      value[value.length - 1] !== '.' ||
      value[value.length - 2] === ',' ||
      safeToUpperCase(safeToLowerCase(value[0])) !== value[0]
    ) {
      throw new Error('Unsupported value');
    }
    const adaptedValue =
      safeToLowerCase(value[0]) + safeSubstring(value, 1, value.length - 1);
    const words = [];
    const candidates = safeSplit(adaptedValue, ' ');
    for (let idx = 0; idx !== candidates.length; ++idx) {
      const candidate = candidates[idx];
      if (wordsArbitrary.canShrinkWithoutContext(candidate))
        safePush(words, candidate);
      else if (
        idx === candidates.length - 1 &&
        wordsArbitrary.canShrinkWithoutContext(candidate + ',')
      )
        safePush(words, candidate + ',');
      else throw new Error('Unsupported word');
    }
    return words;
  };
}
function sentencesToParagraphMapper(sentences) {
  return safeJoin(sentences, ' ');
}
function sentencesToParagraphUnmapper(value) {
  if (typeof value !== 'string') {
    throw new Error('Unsupported type');
  }
  const sentences = safeSplit(value, '. ');
  for (let idx = 0; idx < sentences.length - 1; ++idx) {
    sentences[idx] += '.';
  }
  return sentences;
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/lorem.js
var h = (v, w) => {
  return { arbitrary: constant(v), weight: w };
};
function loremWord() {
  return oneof(
    h('non', 6),
    h('adipiscing', 5),
    h('ligula', 5),
    h('enim', 5),
    h('pellentesque', 5),
    h('in', 5),
    h('augue', 5),
    h('et', 5),
    h('nulla', 5),
    h('lorem', 4),
    h('sit', 4),
    h('sed', 4),
    h('diam', 4),
    h('fermentum', 4),
    h('ut', 4),
    h('eu', 4),
    h('aliquam', 4),
    h('mauris', 4),
    h('vitae', 4),
    h('felis', 4),
    h('ipsum', 3),
    h('dolor', 3),
    h('amet,', 3),
    h('elit', 3),
    h('euismod', 3),
    h('mi', 3),
    h('orci', 3),
    h('erat', 3),
    h('praesent', 3),
    h('egestas', 3),
    h('leo', 3),
    h('vel', 3),
    h('sapien', 3),
    h('integer', 3),
    h('curabitur', 3),
    h('convallis', 3),
    h('purus', 3),
    h('risus', 2),
    h('suspendisse', 2),
    h('lectus', 2),
    h('nec,', 2),
    h('ultricies', 2),
    h('sed,', 2),
    h('cras', 2),
    h('elementum', 2),
    h('ultrices', 2),
    h('maecenas', 2),
    h('massa,', 2),
    h('varius', 2),
    h('a,', 2),
    h('semper', 2),
    h('proin', 2),
    h('nec', 2),
    h('nisl', 2),
    h('amet', 2),
    h('duis', 2),
    h('congue', 2),
    h('libero', 2),
    h('vestibulum', 2),
    h('pede', 2),
    h('blandit', 2),
    h('sodales', 2),
    h('ante', 2),
    h('nibh', 2),
    h('ac', 2),
    h('aenean', 2),
    h('massa', 2),
    h('suscipit', 2),
    h('sollicitudin', 2),
    h('fusce', 2),
    h('tempus', 2),
    h('aliquam,', 2),
    h('nunc', 2),
    h('ullamcorper', 2),
    h('rhoncus', 2),
    h('metus', 2),
    h('faucibus,', 2),
    h('justo', 2),
    h('magna', 2),
    h('at', 2),
    h('tincidunt', 2),
    h('consectetur', 1),
    h('tortor,', 1),
    h('dignissim', 1),
    h('congue,', 1),
    h('non,', 1),
    h('porttitor,', 1),
    h('nonummy', 1),
    h('molestie,', 1),
    h('est', 1),
    h('eleifend', 1),
    h('mi,', 1),
    h('arcu', 1),
    h('scelerisque', 1),
    h('vitae,', 1),
    h('consequat', 1),
    h('in,', 1),
    h('pretium', 1),
    h('volutpat', 1),
    h('pharetra', 1),
    h('tempor', 1),
    h('bibendum', 1),
    h('odio', 1),
    h('dui', 1),
    h('primis', 1),
    h('faucibus', 1),
    h('luctus', 1),
    h('posuere', 1),
    h('cubilia', 1),
    h('curae,', 1),
    h('hendrerit', 1),
    h('velit', 1),
    h('mauris,', 1),
    h('gravida', 1),
    h('ornare', 1),
    h('ut,', 1),
    h('pulvinar', 1),
    h('varius,', 1),
    h('turpis', 1),
    h('nibh,', 1),
    h('eros', 1),
    h('id', 1),
    h('aliquet', 1),
    h('quis', 1),
    h('lobortis', 1),
    h('consectetuer', 1),
    h('morbi', 1),
    h('vehicula', 1),
    h('tortor', 1),
    h('tellus,', 1),
    h('id,', 1),
    h('eu,', 1),
    h('quam', 1),
    h('feugiat,', 1),
    h('posuere,', 1),
    h('iaculis', 1),
    h('lectus,', 1),
    h('tristique', 1),
    h('mollis,', 1),
    h('nisl,', 1),
    h('vulputate', 1),
    h('sem', 1),
    h('vivamus', 1),
    h('placerat', 1),
    h('imperdiet', 1),
    h('cursus', 1),
    h('rutrum', 1),
    h('iaculis,', 1),
    h('augue,', 1),
    h('lacus', 1),
  );
}
function lorem(constraints = {}) {
  const { maxCount, mode = 'words', size } = constraints;
  if (maxCount !== void 0 && maxCount < 1) {
    throw new Error(`lorem has to produce at least one word/sentence`);
  }
  const wordArbitrary = loremWord();
  if (mode === 'sentences') {
    const sentence = array(wordArbitrary, { minLength: 1, size: 'small' }).map(
      wordsToSentenceMapper,
      wordsToSentenceUnmapperFor(wordArbitrary),
    );
    return array(sentence, { minLength: 1, maxLength: maxCount, size }).map(
      sentencesToParagraphMapper,
      sentencesToParagraphUnmapper,
    );
  } else {
    return array(wordArbitrary, {
      minLength: 1,
      maxLength: maxCount,
      size,
    }).map(
      wordsToJoinedStringMapper,
      wordsToJoinedStringUnmapperFor(wordArbitrary),
    );
  }
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/memo.js
var contextRemainingDepth = 10;
function memo(builder) {
  const previous = {};
  return (maxDepth) => {
    const n = maxDepth !== void 0 ? maxDepth : contextRemainingDepth;
    if (!safeHasOwnProperty(previous, n)) {
      const prev = contextRemainingDepth;
      contextRemainingDepth = n - 1;
      previous[n] = builder(n);
      contextRemainingDepth = prev;
    }
    return previous[n];
  };
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/_internals/helpers/ToggleFlags.js
function countToggledBits(n) {
  let count = 0;
  while (n > SBigInt(0)) {
    if (n & SBigInt(1)) ++count;
    n >>= SBigInt(1);
  }
  return count;
}
function computeNextFlags(flags, nextSize) {
  const allowedMask = (SBigInt(1) << SBigInt(nextSize)) - SBigInt(1);
  const preservedFlags = flags & allowedMask;
  let numMissingFlags = countToggledBits(flags - preservedFlags);
  let nFlags = preservedFlags;
  for (
    let mask = SBigInt(1);
    mask <= allowedMask && numMissingFlags !== 0;
    mask <<= SBigInt(1)
  ) {
    if (!(nFlags & mask)) {
      nFlags |= mask;
      --numMissingFlags;
    }
  }
  return nFlags;
}
function computeTogglePositions(chars, toggleCase) {
  const positions = [];
  for (let idx = chars.length - 1; idx !== -1; --idx) {
    if (toggleCase(chars[idx]) !== chars[idx]) safePush(positions, idx);
  }
  return positions;
}
function computeFlagsFromChars(untoggledChars, toggledChars, togglePositions) {
  let flags = SBigInt(0);
  for (
    let idx = 0, mask = SBigInt(1);
    idx !== togglePositions.length;
    ++idx, mask <<= SBigInt(1)
  ) {
    if (
      untoggledChars[togglePositions[idx]] !==
      toggledChars[togglePositions[idx]]
    ) {
      flags |= mask;
    }
  }
  return flags;
}
function applyFlagsOnChars(chars, flags, togglePositions, toggleCase) {
  for (
    let idx = 0, mask = SBigInt(1);
    idx !== togglePositions.length;
    ++idx, mask <<= SBigInt(1)
  ) {
    if (flags & mask)
      chars[togglePositions[idx]] = toggleCase(chars[togglePositions[idx]]);
  }
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/_internals/MixedCaseArbitrary.js
var MixedCaseArbitrary = class extends Arbitrary {
  constructor(stringArb, toggleCase, untoggleAll) {
    super();
    this.stringArb = stringArb;
    this.toggleCase = toggleCase;
    this.untoggleAll = untoggleAll;
  }
  buildContextFor(rawStringValue, flagsValue) {
    return {
      rawString: rawStringValue.value,
      rawStringContext: rawStringValue.context,
      flags: flagsValue.value,
      flagsContext: flagsValue.context,
    };
  }
  generate(mrng, biasFactor) {
    const rawStringValue = this.stringArb.generate(mrng, biasFactor);
    const chars = [...rawStringValue.value];
    const togglePositions = computeTogglePositions(chars, this.toggleCase);
    const flagsArb = bigUintN(togglePositions.length);
    const flagsValue = flagsArb.generate(mrng, void 0);
    applyFlagsOnChars(
      chars,
      flagsValue.value,
      togglePositions,
      this.toggleCase,
    );
    return new Value(
      safeJoin(chars, ''),
      this.buildContextFor(rawStringValue, flagsValue),
    );
  }
  canShrinkWithoutContext(value) {
    if (typeof value !== 'string') {
      return false;
    }
    return this.untoggleAll !== void 0
      ? this.stringArb.canShrinkWithoutContext(this.untoggleAll(value))
      : this.stringArb.canShrinkWithoutContext(value);
  }
  shrink(value, context2) {
    let contextSafe;
    if (context2 !== void 0) {
      contextSafe = context2;
    } else {
      if (this.untoggleAll !== void 0) {
        const untoggledValue = this.untoggleAll(value);
        const valueChars = [...value];
        const untoggledValueChars = [...untoggledValue];
        const togglePositions = computeTogglePositions(
          untoggledValueChars,
          this.toggleCase,
        );
        contextSafe = {
          rawString: untoggledValue,
          rawStringContext: void 0,
          flags: computeFlagsFromChars(
            untoggledValueChars,
            valueChars,
            togglePositions,
          ),
          flagsContext: void 0,
        };
      } else {
        contextSafe = {
          rawString: value,
          rawStringContext: void 0,
          flags: SBigInt(0),
          flagsContext: void 0,
        };
      }
    }
    const rawString = contextSafe.rawString;
    const flags = contextSafe.flags;
    return this.stringArb
      .shrink(rawString, contextSafe.rawStringContext)
      .map((nRawStringValue) => {
        const nChars = [...nRawStringValue.value];
        const nTogglePositions = computeTogglePositions(
          nChars,
          this.toggleCase,
        );
        const nFlags = computeNextFlags(flags, nTogglePositions.length);
        applyFlagsOnChars(nChars, nFlags, nTogglePositions, this.toggleCase);
        return new Value(
          safeJoin(nChars, ''),
          this.buildContextFor(nRawStringValue, new Value(nFlags, void 0)),
        );
      })
      .join(
        makeLazy(() => {
          const chars = [...rawString];
          const togglePositions = computeTogglePositions(
            chars,
            this.toggleCase,
          );
          return bigUintN(togglePositions.length)
            .shrink(flags, contextSafe.flagsContext)
            .map((nFlagsValue) => {
              const nChars = safeSlice(chars);
              applyFlagsOnChars(
                nChars,
                nFlagsValue.value,
                togglePositions,
                this.toggleCase,
              );
              return new Value(
                safeJoin(nChars, ''),
                this.buildContextFor(
                  new Value(rawString, contextSafe.rawStringContext),
                  nFlagsValue,
                ),
              );
            });
        }),
      );
  }
};

// ../../../node_modules/fast-check/lib/esm/arbitrary/mixedCase.js
function defaultToggleCase(rawChar) {
  const upper = safeToUpperCase(rawChar);
  if (upper !== rawChar) return upper;
  return safeToLowerCase(rawChar);
}
function mixedCase(stringArb, constraints) {
  if (typeof SBigInt === 'undefined') {
    throw new SError(`mixedCase requires BigInt support`);
  }
  const toggleCase =
    (constraints && constraints.toggleCase) || defaultToggleCase;
  const untoggleAll = constraints && constraints.untoggleAll;
  return new MixedCaseArbitrary(stringArb, toggleCase, untoggleAll);
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/float32Array.js
function toTypedMapper(data) {
  return SFloat32Array.from(data);
}
function fromTypedUnmapper(value) {
  if (!(value instanceof SFloat32Array)) throw new Error('Unexpected type');
  return [...value];
}
function float32Array(constraints = {}) {
  return array(float(constraints), constraints).map(
    toTypedMapper,
    fromTypedUnmapper,
  );
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/float64Array.js
function toTypedMapper2(data) {
  return SFloat64Array.from(data);
}
function fromTypedUnmapper2(value) {
  if (!(value instanceof SFloat64Array)) throw new Error('Unexpected type');
  return [...value];
}
function float64Array(constraints = {}) {
  return array(double(constraints), constraints).map(
    toTypedMapper2,
    fromTypedUnmapper2,
  );
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/_internals/builders/TypedIntArrayArbitraryBuilder.js
var __rest = function (s, e) {
  var t = {};
  for (var p in s)
    if (Object.prototype.hasOwnProperty.call(s, p) && e.indexOf(p) < 0)
      t[p] = s[p];
  if (s != null && typeof Object.getOwnPropertySymbols === 'function')
    for (var i = 0, p = Object.getOwnPropertySymbols(s); i < p.length; i++) {
      if (
        e.indexOf(p[i]) < 0 &&
        Object.prototype.propertyIsEnumerable.call(s, p[i])
      )
        t[p[i]] = s[p[i]];
    }
  return t;
};
function typedIntArrayArbitraryArbitraryBuilder(
  constraints,
  defaultMin,
  defaultMax,
  TypedArrayClass,
  arbitraryBuilder,
) {
  const generatorName = TypedArrayClass.name;
  const { min = defaultMin, max = defaultMax } = constraints,
    arrayConstraints = __rest(constraints, ['min', 'max']);
  if (min > max) {
    throw new Error(
      `Invalid range passed to ${generatorName}: min must be lower than or equal to max`,
    );
  }
  if (min < defaultMin) {
    throw new Error(
      `Invalid min value passed to ${generatorName}: min must be greater than or equal to ${defaultMin}`,
    );
  }
  if (max > defaultMax) {
    throw new Error(
      `Invalid max value passed to ${generatorName}: max must be lower than or equal to ${defaultMax}`,
    );
  }
  return array(arbitraryBuilder({ min, max }), arrayConstraints).map(
    (data) => TypedArrayClass.from(data),
    (value) => {
      if (!(value instanceof TypedArrayClass)) throw new Error('Invalid type');
      return [...value];
    },
  );
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/int16Array.js
function int16Array(constraints = {}) {
  return typedIntArrayArbitraryArbitraryBuilder(
    constraints,
    -32768,
    32767,
    SInt16Array,
    integer,
  );
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/int32Array.js
function int32Array(constraints = {}) {
  return typedIntArrayArbitraryArbitraryBuilder(
    constraints,
    -2147483648,
    2147483647,
    SInt32Array,
    integer,
  );
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/int8Array.js
function int8Array(constraints = {}) {
  return typedIntArrayArbitraryArbitraryBuilder(
    constraints,
    -128,
    127,
    SInt8Array,
    integer,
  );
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/uint16Array.js
function uint16Array(constraints = {}) {
  return typedIntArrayArbitraryArbitraryBuilder(
    constraints,
    0,
    65535,
    SUint16Array,
    integer,
  );
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/uint32Array.js
function uint32Array(constraints = {}) {
  return typedIntArrayArbitraryArbitraryBuilder(
    constraints,
    0,
    4294967295,
    SUint32Array,
    integer,
  );
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/uint8Array.js
function uint8Array(constraints = {}) {
  return typedIntArrayArbitraryArbitraryBuilder(
    constraints,
    0,
    255,
    SUint8Array,
    integer,
  );
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/uint8ClampedArray.js
function uint8ClampedArray(constraints = {}) {
  return typedIntArrayArbitraryArbitraryBuilder(
    constraints,
    0,
    255,
    SUint8ClampedArray,
    integer,
  );
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/_internals/WithShrinkFromOtherArbitrary.js
function isSafeContext(context2) {
  return context2 !== void 0;
}
function toGeneratorValue(value) {
  if (value.hasToBeCloned) {
    return new Value(
      value.value_,
      { generatorContext: value.context },
      () => value.value,
    );
  }
  return new Value(value.value_, { generatorContext: value.context });
}
function toShrinkerValue(value) {
  if (value.hasToBeCloned) {
    return new Value(
      value.value_,
      { shrinkerContext: value.context },
      () => value.value,
    );
  }
  return new Value(value.value_, { shrinkerContext: value.context });
}
var WithShrinkFromOtherArbitrary = class extends Arbitrary {
  constructor(generatorArbitrary, shrinkerArbitrary) {
    super();
    this.generatorArbitrary = generatorArbitrary;
    this.shrinkerArbitrary = shrinkerArbitrary;
  }
  generate(mrng, biasFactor) {
    return toGeneratorValue(this.generatorArbitrary.generate(mrng, biasFactor));
  }
  canShrinkWithoutContext(value) {
    return this.shrinkerArbitrary.canShrinkWithoutContext(value);
  }
  shrink(value, context2) {
    if (!isSafeContext(context2)) {
      return this.shrinkerArbitrary.shrink(value, void 0).map(toShrinkerValue);
    }
    if ('generatorContext' in context2) {
      return this.generatorArbitrary
        .shrink(value, context2.generatorContext)
        .map(toGeneratorValue);
    }
    return this.shrinkerArbitrary
      .shrink(value, context2.shrinkerContext)
      .map(toShrinkerValue);
  }
};

// ../../../node_modules/fast-check/lib/esm/arbitrary/_internals/builders/RestrictedIntegerArbitraryBuilder.js
function restrictedIntegerArbitraryBuilder(min, maxGenerated, max) {
  const generatorArbitrary = integer({ min, max: maxGenerated });
  if (maxGenerated === max) {
    return generatorArbitrary;
  }
  const shrinkerArbitrary = integer({ min, max });
  return new WithShrinkFromOtherArbitrary(
    generatorArbitrary,
    shrinkerArbitrary,
  );
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/sparseArray.js
var safeMathMin5 = Math.min;
var safeMathMax3 = Math.max;
var safeArrayIsArray3 = SArray.isArray;
var safeObjectEntries2 = Object.entries;
function extractMaxIndex(indexesAndValues) {
  let maxIndex = -1;
  for (let index = 0; index !== indexesAndValues.length; ++index) {
    maxIndex = safeMathMax3(maxIndex, indexesAndValues[index][0]);
  }
  return maxIndex;
}
function arrayFromItems(length, indexesAndValues) {
  const array2 = SArray(length);
  for (let index = 0; index !== indexesAndValues.length; ++index) {
    const it = indexesAndValues[index];
    if (it[0] < length) array2[it[0]] = it[1];
  }
  return array2;
}
function sparseArray(arb, constraints = {}) {
  const {
    size,
    minNumElements = 0,
    maxLength = MaxLengthUpperBound,
    maxNumElements = maxLength,
    noTrailingHole,
    depthIdentifier,
  } = constraints;
  const maxGeneratedNumElements = maxGeneratedLengthFromSizeForArbitrary(
    size,
    minNumElements,
    maxNumElements,
    constraints.maxNumElements !== void 0,
  );
  const maxGeneratedLength = maxGeneratedLengthFromSizeForArbitrary(
    size,
    maxGeneratedNumElements,
    maxLength,
    constraints.maxLength !== void 0,
  );
  if (minNumElements > maxLength) {
    throw new Error(
      `The minimal number of non-hole elements cannot be higher than the maximal length of the array`,
    );
  }
  if (minNumElements > maxNumElements) {
    throw new Error(
      `The minimal number of non-hole elements cannot be higher than the maximal number of non-holes`,
    );
  }
  const resultedMaxNumElements = safeMathMin5(maxNumElements, maxLength);
  const resultedSizeMaxNumElements =
    constraints.maxNumElements !== void 0 || size !== void 0 ? size : '=';
  const maxGeneratedIndexAuthorized = safeMathMax3(maxGeneratedLength - 1, 0);
  const maxIndexAuthorized = safeMathMax3(maxLength - 1, 0);
  const sparseArrayNoTrailingHole = uniqueArray(
    tuple(
      restrictedIntegerArbitraryBuilder(
        0,
        maxGeneratedIndexAuthorized,
        maxIndexAuthorized,
      ),
      arb,
    ),
    {
      size: resultedSizeMaxNumElements,
      minLength: minNumElements,
      maxLength: resultedMaxNumElements,
      selector: (item) => item[0],
      depthIdentifier,
    },
  ).map(
    (items) => {
      const lastIndex = extractMaxIndex(items);
      return arrayFromItems(lastIndex + 1, items);
    },
    (value) => {
      if (!safeArrayIsArray3(value)) {
        throw new Error('Not supported entry type');
      }
      if (
        noTrailingHole &&
        value.length !== 0 &&
        !(value.length - 1 in value)
      ) {
        throw new Error('No trailing hole');
      }
      return safeMap(safeObjectEntries2(value), (entry) => [
        Number(entry[0]),
        entry[1],
      ]);
    },
  );
  if (noTrailingHole || maxLength === minNumElements) {
    return sparseArrayNoTrailingHole;
  }
  return tuple(
    sparseArrayNoTrailingHole,
    restrictedIntegerArbitraryBuilder(
      minNumElements,
      maxGeneratedLength,
      maxLength,
    ),
  ).map(
    (data) => {
      const sparse = data[0];
      const targetLength = data[1];
      if (sparse.length >= targetLength) {
        return sparse;
      }
      const longerSparse = safeSlice(sparse);
      longerSparse.length = targetLength;
      return longerSparse;
    },
    (value) => {
      if (!safeArrayIsArray3(value)) {
        throw new Error('Not supported entry type');
      }
      return [value, value.length];
    },
  );
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/_internals/mappers/ArrayToMap.js
function arrayToMapMapper(data) {
  return new Map(data);
}
function arrayToMapUnmapper(value) {
  if (typeof value !== 'object' || value === null) {
    throw new Error(
      'Incompatible instance received: should be a non-null object',
    );
  }
  if (!('constructor' in value) || value.constructor !== Map) {
    throw new Error(
      'Incompatible instance received: should be of exact type Map',
    );
  }
  return Array.from(value);
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/_internals/mappers/ArrayToSet.js
function arrayToSetMapper(data) {
  return new Set(data);
}
function arrayToSetUnmapper(value) {
  if (typeof value !== 'object' || value === null) {
    throw new Error(
      'Incompatible instance received: should be a non-null object',
    );
  }
  if (!('constructor' in value) || value.constructor !== Set) {
    throw new Error(
      'Incompatible instance received: should be of exact type Set',
    );
  }
  return Array.from(value);
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/_internals/builders/AnyArbitraryBuilder.js
function mapOf(ka, va, maxKeys, size, depthIdentifier) {
  return uniqueArray(tuple(ka, va), {
    maxLength: maxKeys,
    size,
    comparator: 'SameValueZero',
    selector: (t) => t[0],
    depthIdentifier,
  }).map(arrayToMapMapper, arrayToMapUnmapper);
}
function dictOf(ka, va, maxKeys, size, depthIdentifier, withNullPrototype) {
  return dictionary(ka, va, {
    maxKeys,
    noNullPrototype: !withNullPrototype,
    size,
    depthIdentifier,
  });
}
function setOf(va, maxKeys, size, depthIdentifier) {
  return uniqueArray(va, {
    maxLength: maxKeys,
    size,
    comparator: 'SameValueZero',
    depthIdentifier,
  }).map(arrayToSetMapper, arrayToSetUnmapper);
}
function typedArray(constraints) {
  return oneof(
    int8Array(constraints),
    uint8Array(constraints),
    uint8ClampedArray(constraints),
    int16Array(constraints),
    uint16Array(constraints),
    int32Array(constraints),
    uint32Array(constraints),
    float32Array(constraints),
    float64Array(constraints),
  );
}
function anyArbitraryBuilder(constraints) {
  const arbitrariesForBase = constraints.values;
  const depthSize = constraints.depthSize;
  const depthIdentifier = createDepthIdentifier();
  const maxDepth = constraints.maxDepth;
  const maxKeys = constraints.maxKeys;
  const size = constraints.size;
  const baseArb = oneof(
    ...arbitrariesForBase,
    ...(constraints.withBigInt ? [bigInt()] : []),
    ...(constraints.withDate ? [date()] : []),
  );
  return letrec((tie) => ({
    anything: oneof(
      { maxDepth, depthSize, depthIdentifier },
      baseArb,
      tie('array'),
      tie('object'),
      ...(constraints.withMap ? [tie('map')] : []),
      ...(constraints.withSet ? [tie('set')] : []),
      ...(constraints.withObjectString
        ? [tie('anything').map((o) => stringify(o))]
        : []),
      ...(constraints.withTypedArray
        ? [typedArray({ maxLength: maxKeys, size })]
        : []),
      ...(constraints.withSparseArray
        ? [
            sparseArray(tie('anything'), {
              maxNumElements: maxKeys,
              size,
              depthIdentifier,
            }),
          ]
        : []),
    ),
    keys: constraints.withObjectString
      ? oneof(
          { arbitrary: constraints.key, weight: 10 },
          { arbitrary: tie('anything').map((o) => stringify(o)), weight: 1 },
        )
      : constraints.key,
    array: array(tie('anything'), {
      maxLength: maxKeys,
      size,
      depthIdentifier,
    }),
    set: setOf(tie('anything'), maxKeys, size, depthIdentifier),
    map: oneof(
      mapOf(tie('keys'), tie('anything'), maxKeys, size, depthIdentifier),
      mapOf(tie('anything'), tie('anything'), maxKeys, size, depthIdentifier),
    ),
    object: dictOf(
      tie('keys'),
      tie('anything'),
      maxKeys,
      size,
      depthIdentifier,
      constraints.withNullPrototype,
    ),
  })).anything;
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/string.js
var safeObjectAssign9 = Object.assign;
function string(constraints = {}) {
  const charArbitrary = char();
  const experimentalCustomSlices = createSlicesForString(
    charArbitrary,
    codePointsToStringUnmapper,
  );
  const enrichedConstraints = safeObjectAssign9(
    safeObjectAssign9({}, constraints),
    {
      experimentalCustomSlices,
    },
  );
  return array(charArbitrary, enrichedConstraints).map(
    codePointsToStringMapper,
    codePointsToStringUnmapper,
  );
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/_internals/mappers/UnboxedToBoxed.js
function unboxedToBoxedMapper(value) {
  switch (typeof value) {
    case 'boolean':
      return new SBoolean(value);
    case 'number':
      return new SNumber(value);
    case 'string':
      return new SString(value);
    default:
      return value;
  }
}
function unboxedToBoxedUnmapper(value) {
  if (
    typeof value !== 'object' ||
    value === null ||
    !('constructor' in value)
  ) {
    return value;
  }
  return value.constructor === SBoolean ||
    value.constructor === SNumber ||
    value.constructor === SString
    ? value.valueOf()
    : value;
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/_internals/builders/BoxedArbitraryBuilder.js
function boxedArbitraryBuilder(arb) {
  return arb.map(unboxedToBoxedMapper, unboxedToBoxedUnmapper);
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/_internals/helpers/QualifiedObjectConstraints.js
function defaultValues(constraints) {
  return [
    boolean(),
    maxSafeInteger(),
    double(),
    string(constraints),
    oneof(string(constraints), constant(null), constant(void 0)),
  ];
}
function boxArbitraries(arbs) {
  return arbs.map((arb) => boxedArbitraryBuilder(arb));
}
function boxArbitrariesIfNeeded(arbs, boxEnabled) {
  return boxEnabled ? boxArbitraries(arbs).concat(arbs) : arbs;
}
function toQualifiedObjectConstraints(settings = {}) {
  function orDefault(optionalValue, defaultValue) {
    return optionalValue !== void 0 ? optionalValue : defaultValue;
  }
  const valueConstraints = { size: settings.size };
  return {
    key: orDefault(settings.key, string(valueConstraints)),
    values: boxArbitrariesIfNeeded(
      orDefault(settings.values, defaultValues(valueConstraints)),
      orDefault(settings.withBoxedValues, false),
    ),
    depthSize: settings.depthSize,
    maxDepth: settings.maxDepth,
    maxKeys: settings.maxKeys,
    size: settings.size,
    withSet: orDefault(settings.withSet, false),
    withMap: orDefault(settings.withMap, false),
    withObjectString: orDefault(settings.withObjectString, false),
    withNullPrototype: orDefault(settings.withNullPrototype, false),
    withBigInt: orDefault(settings.withBigInt, false),
    withDate: orDefault(settings.withDate, false),
    withTypedArray: orDefault(settings.withTypedArray, false),
    withSparseArray: orDefault(settings.withSparseArray, false),
  };
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/object.js
function objectInternal(constraints) {
  return dictionary(constraints.key, anyArbitraryBuilder(constraints), {
    maxKeys: constraints.maxKeys,
    noNullPrototype: !constraints.withNullPrototype,
    size: constraints.size,
  });
}
function object(constraints) {
  return objectInternal(toQualifiedObjectConstraints(constraints));
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/_internals/helpers/JsonConstraintsBuilder.js
function jsonConstraintsBuilder(stringArbitrary, constraints) {
  const { depthSize, maxDepth } = constraints;
  const key = stringArbitrary;
  const values = [
    boolean(),
    double({ noDefaultInfinity: true, noNaN: true }),
    stringArbitrary,
    constant(null),
  ];
  return { key, values, depthSize, maxDepth };
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/anything.js
function anything(constraints) {
  return anyArbitraryBuilder(toQualifiedObjectConstraints(constraints));
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/jsonValue.js
function jsonValue(constraints = {}) {
  return anything(jsonConstraintsBuilder(string(), constraints));
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/json.js
function json(constraints = {}) {
  const arb = jsonValue(constraints);
  return arb.map(JSON.stringify);
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/unicodeString.js
var safeObjectAssign10 = Object.assign;
function unicodeString(constraints = {}) {
  const charArbitrary = unicode();
  const experimentalCustomSlices = createSlicesForString(
    charArbitrary,
    codePointsToStringUnmapper,
  );
  const enrichedConstraints = safeObjectAssign10(
    safeObjectAssign10({}, constraints),
    {
      experimentalCustomSlices,
    },
  );
  return array(charArbitrary, enrichedConstraints).map(
    codePointsToStringMapper,
    codePointsToStringUnmapper,
  );
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/unicodeJsonValue.js
function unicodeJsonValue(constraints = {}) {
  return anything(jsonConstraintsBuilder(unicodeString(), constraints));
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/unicodeJson.js
function unicodeJson(constraints = {}) {
  const arb = unicodeJsonValue(constraints);
  return arb.map(JSON.stringify);
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/_internals/helpers/EnumerableKeysExtractor.js
var safeObjectKeys4 = Object.keys;
var safeObjectGetOwnPropertySymbols3 = Object.getOwnPropertySymbols;
var safeObjectGetOwnPropertyDescriptor3 = Object.getOwnPropertyDescriptor;
function extractEnumerableKeys(instance) {
  const keys = safeObjectKeys4(instance);
  const symbols = safeObjectGetOwnPropertySymbols3(instance);
  for (let index = 0; index !== symbols.length; ++index) {
    const symbol = symbols[index];
    const descriptor = safeObjectGetOwnPropertyDescriptor3(instance, symbol);
    if (descriptor && descriptor.enumerable) {
      keys.push(symbol);
    }
  }
  return keys;
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/_internals/mappers/ValuesAndSeparateKeysToObject.js
var safeObjectCreate3 = Object.create;
var safeObjectDefineProperty4 = Object.defineProperty;
var safeObjectGetOwnPropertyDescriptor4 = Object.getOwnPropertyDescriptor;
var safeObjectGetOwnPropertyNames2 = Object.getOwnPropertyNames;
var safeObjectGetOwnPropertySymbols4 = Object.getOwnPropertySymbols;
function buildValuesAndSeparateKeysToObjectMapper(keys, noKeyValue2) {
  return function valuesAndSeparateKeysToObjectMapper(definition) {
    const obj = definition[1] ? safeObjectCreate3(null) : {};
    for (let idx = 0; idx !== keys.length; ++idx) {
      const valueWrapper = definition[0][idx];
      if (valueWrapper !== noKeyValue2) {
        safeObjectDefineProperty4(obj, keys[idx], {
          value: valueWrapper,
          configurable: true,
          enumerable: true,
          writable: true,
        });
      }
    }
    return obj;
  };
}
function buildValuesAndSeparateKeysToObjectUnmapper(keys, noKeyValue2) {
  return function valuesAndSeparateKeysToObjectUnmapper(value) {
    if (typeof value !== 'object' || value === null) {
      throw new Error(
        'Incompatible instance received: should be a non-null object',
      );
    }
    const hasNullPrototype = Object.getPrototypeOf(value) === null;
    const hasObjectPrototype =
      'constructor' in value && value.constructor === Object;
    if (!hasNullPrototype && !hasObjectPrototype) {
      throw new Error(
        'Incompatible instance received: should be of exact type Object',
      );
    }
    let extractedPropertiesCount = 0;
    const extractedValues = [];
    for (let idx = 0; idx !== keys.length; ++idx) {
      const descriptor = safeObjectGetOwnPropertyDescriptor4(value, keys[idx]);
      if (descriptor !== void 0) {
        if (
          !descriptor.configurable ||
          !descriptor.enumerable ||
          !descriptor.writable
        ) {
          throw new Error(
            'Incompatible instance received: should contain only c/e/w properties',
          );
        }
        if (descriptor.get !== void 0 || descriptor.set !== void 0) {
          throw new Error(
            'Incompatible instance received: should contain only no get/set properties',
          );
        }
        ++extractedPropertiesCount;
        safePush(extractedValues, descriptor.value);
      } else {
        safePush(extractedValues, noKeyValue2);
      }
    }
    const namePropertiesCount = safeObjectGetOwnPropertyNames2(value).length;
    const symbolPropertiesCount =
      safeObjectGetOwnPropertySymbols4(value).length;
    if (
      extractedPropertiesCount !==
      namePropertiesCount + symbolPropertiesCount
    ) {
      throw new Error(
        'Incompatible instance received: should not contain extra properties',
      );
    }
    return [extractedValues, hasNullPrototype];
  };
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/_internals/builders/PartialRecordArbitraryBuilder.js
var noKeyValue = Symbol('no-key');
function buildPartialRecordArbitrary(
  recordModel,
  requiredKeys,
  noNullPrototype,
) {
  const keys = extractEnumerableKeys(recordModel);
  const arbs = [];
  for (let index = 0; index !== keys.length; ++index) {
    const k = keys[index];
    const requiredArbitrary = recordModel[k];
    if (requiredKeys === void 0 || safeIndexOf(requiredKeys, k) !== -1) {
      safePush(arbs, requiredArbitrary);
    } else {
      safePush(arbs, option(requiredArbitrary, { nil: noKeyValue }));
    }
  }
  return tuple(
    tuple(...arbs),
    noNullPrototype ? constant(false) : boolean(),
  ).map(
    buildValuesAndSeparateKeysToObjectMapper(keys, noKeyValue),
    buildValuesAndSeparateKeysToObjectUnmapper(keys, noKeyValue),
  );
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/record.js
function record(recordModel, constraints) {
  const noNullPrototype =
    constraints === void 0 ||
    constraints.noNullPrototype === void 0 ||
    constraints.noNullPrototype;
  if (constraints == null) {
    return buildPartialRecordArbitrary(recordModel, void 0, noNullPrototype);
  }
  if ('withDeletedKeys' in constraints && 'requiredKeys' in constraints) {
    throw new Error(
      `requiredKeys and withDeletedKeys cannot be used together in fc.record`,
    );
  }
  const requireDeletedKeys =
    ('requiredKeys' in constraints && constraints.requiredKeys !== void 0) ||
    ('withDeletedKeys' in constraints && !!constraints.withDeletedKeys);
  if (!requireDeletedKeys) {
    return buildPartialRecordArbitrary(recordModel, void 0, noNullPrototype);
  }
  const requiredKeys =
    ('requiredKeys' in constraints ? constraints.requiredKeys : void 0) || [];
  for (let idx = 0; idx !== requiredKeys.length; ++idx) {
    const descriptor = Object.getOwnPropertyDescriptor(
      recordModel,
      requiredKeys[idx],
    );
    if (descriptor === void 0) {
      throw new Error(
        `requiredKeys cannot reference keys that have not been defined in recordModel`,
      );
    }
    if (!descriptor.enumerable) {
      throw new Error(
        `requiredKeys cannot reference keys that have are enumerable in recordModel`,
      );
    }
  }
  return buildPartialRecordArbitrary(
    recordModel,
    requiredKeys,
    noNullPrototype,
  );
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/_internals/StreamArbitrary.js
var safeObjectDefineProperties2 = Object.defineProperties;
function prettyPrint(seenValuesStrings) {
  return `Stream(${safeJoin(seenValuesStrings, ',')}\u2026)`;
}
var StreamArbitrary = class extends Arbitrary {
  constructor(arb) {
    super();
    this.arb = arb;
  }
  generate(mrng, biasFactor) {
    const appliedBiasFactor =
      biasFactor !== void 0 && mrng.nextInt(1, biasFactor) === 1
        ? biasFactor
        : void 0;
    const enrichedProducer = () => {
      const seenValues = [];
      const g = function* (arb, clonedMrng) {
        while (true) {
          const value = arb.generate(clonedMrng, appliedBiasFactor).value;
          safePush(seenValues, value);
          yield value;
        }
      };
      const s = new Stream(g(this.arb, mrng.clone()));
      return safeObjectDefineProperties2(s, {
        toString: { value: () => prettyPrint(seenValues.map(stringify)) },
        [toStringMethod]: {
          value: () => prettyPrint(seenValues.map(stringify)),
        },
        [asyncToStringMethod]: {
          value: async () =>
            prettyPrint(await Promise.all(seenValues.map(asyncStringify))),
        },
        [cloneMethod]: { value: enrichedProducer, enumerable: true },
      });
    };
    return new Value(enrichedProducer(), void 0);
  }
  canShrinkWithoutContext(value) {
    return false;
  }
  shrink(_value, _context) {
    return Stream.nil();
  }
};

// ../../../node_modules/fast-check/lib/esm/arbitrary/infiniteStream.js
function infiniteStream(arb) {
  return new StreamArbitrary(arb);
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/asciiString.js
var safeObjectAssign11 = Object.assign;
function asciiString(constraints = {}) {
  const charArbitrary = ascii();
  const experimentalCustomSlices = createSlicesForString(
    charArbitrary,
    codePointsToStringUnmapper,
  );
  const enrichedConstraints = safeObjectAssign11(
    safeObjectAssign11({}, constraints),
    {
      experimentalCustomSlices,
    },
  );
  return array(charArbitrary, enrichedConstraints).map(
    codePointsToStringMapper,
    codePointsToStringUnmapper,
  );
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/_internals/mappers/StringToBase64.js
function stringToBase64Mapper(s) {
  switch (s.length % 4) {
    case 0:
      return s;
    case 3:
      return `${s}=`;
    case 2:
      return `${s}==`;
    default:
      return safeSubstring(s, 1);
  }
}
function stringToBase64Unmapper(value) {
  if (typeof value !== 'string' || value.length % 4 !== 0) {
    throw new Error('Invalid string received');
  }
  const lastTrailingIndex = value.indexOf('=');
  if (lastTrailingIndex === -1) {
    return value;
  }
  const numTrailings = value.length - lastTrailingIndex;
  if (numTrailings > 2) {
    throw new Error('Cannot unmap the passed value');
  }
  return safeSubstring(value, 0, lastTrailingIndex);
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/base64String.js
function base64String(constraints = {}) {
  const {
    minLength: unscaledMinLength = 0,
    maxLength: unscaledMaxLength = MaxLengthUpperBound,
    size,
  } = constraints;
  const minLength = unscaledMinLength + 3 - ((unscaledMinLength + 3) % 4);
  const maxLength = unscaledMaxLength - (unscaledMaxLength % 4);
  const requestedSize =
    constraints.maxLength === void 0 && size === void 0 ? '=' : size;
  if (minLength > maxLength)
    throw new Error(
      'Minimal length should be inferior or equal to maximal length',
    );
  if (minLength % 4 !== 0)
    throw new Error('Minimal length of base64 strings must be a multiple of 4');
  if (maxLength % 4 !== 0)
    throw new Error('Maximal length of base64 strings must be a multiple of 4');
  const charArbitrary = base64();
  const experimentalCustomSlices = createSlicesForString(
    charArbitrary,
    codePointsToStringUnmapper,
  );
  const enrichedConstraints = {
    minLength,
    maxLength,
    size: requestedSize,
    experimentalCustomSlices,
  };
  return array(charArbitrary, enrichedConstraints)
    .map(codePointsToStringMapper, codePointsToStringUnmapper)
    .map(stringToBase64Mapper, stringToBase64Unmapper);
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/fullUnicodeString.js
var safeObjectAssign12 = Object.assign;
function fullUnicodeString(constraints = {}) {
  const charArbitrary = fullUnicode();
  const experimentalCustomSlices = createSlicesForString(
    charArbitrary,
    codePointsToStringUnmapper,
  );
  const enrichedConstraints = safeObjectAssign12(
    safeObjectAssign12({}, constraints),
    {
      experimentalCustomSlices,
    },
  );
  return array(charArbitrary, enrichedConstraints).map(
    codePointsToStringMapper,
    codePointsToStringUnmapper,
  );
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/_internals/mappers/CharsToString.js
function charsToStringMapper(tab) {
  return safeJoin(tab, '');
}
function charsToStringUnmapper(value) {
  if (typeof value !== 'string') {
    throw new Error('Cannot unmap the passed value');
  }
  return safeSplit(value, '');
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/string16bits.js
var safeObjectAssign13 = Object.assign;
function string16bits(constraints = {}) {
  const charArbitrary = char16bits();
  const experimentalCustomSlices = createSlicesForString(
    charArbitrary,
    charsToStringUnmapper,
  );
  const enrichedConstraints = safeObjectAssign13(
    safeObjectAssign13({}, constraints),
    {
      experimentalCustomSlices,
    },
  );
  return array(charArbitrary, enrichedConstraints).map(
    charsToStringMapper,
    charsToStringUnmapper,
  );
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/_internals/helpers/IsSubarrayOf.js
function isSubarrayOf(source, small) {
  const countMap = /* @__PURE__ */ new Map();
  let countMinusZero = 0;
  for (const sourceEntry of source) {
    if (Object.is(sourceEntry, -0)) {
      ++countMinusZero;
    } else {
      const oldCount = countMap.get(sourceEntry) || 0;
      countMap.set(sourceEntry, oldCount + 1);
    }
  }
  for (let index = 0; index !== small.length; ++index) {
    if (!(index in small)) {
      return false;
    }
    const smallEntry = small[index];
    if (Object.is(smallEntry, -0)) {
      if (countMinusZero === 0) return false;
      --countMinusZero;
    } else {
      const oldCount = countMap.get(smallEntry) || 0;
      if (oldCount === 0) return false;
      countMap.set(smallEntry, oldCount - 1);
    }
  }
  return true;
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/_internals/SubarrayArbitrary.js
var safeMathFloor6 = Math.floor;
var safeMathLog4 = Math.log;
var safeArrayIsArray4 = Array.isArray;
var SubarrayArbitrary = class extends Arbitrary {
  constructor(originalArray, isOrdered, minLength, maxLength) {
    super();
    this.originalArray = originalArray;
    this.isOrdered = isOrdered;
    this.minLength = minLength;
    this.maxLength = maxLength;
    if (minLength < 0 || minLength > originalArray.length)
      throw new Error(
        'fc.*{s|S}ubarrayOf expects the minimal length to be between 0 and the size of the original array',
      );
    if (maxLength < 0 || maxLength > originalArray.length)
      throw new Error(
        'fc.*{s|S}ubarrayOf expects the maximal length to be between 0 and the size of the original array',
      );
    if (minLength > maxLength)
      throw new Error(
        'fc.*{s|S}ubarrayOf expects the minimal length to be inferior or equal to the maximal length',
      );
    this.lengthArb = new IntegerArbitrary(minLength, maxLength);
    this.biasedLengthArb =
      minLength !== maxLength
        ? new IntegerArbitrary(
            minLength,
            minLength +
              safeMathFloor6(
                safeMathLog4(maxLength - minLength) / safeMathLog4(2),
              ),
          )
        : this.lengthArb;
  }
  generate(mrng, biasFactor) {
    const lengthArb =
      biasFactor !== void 0 && mrng.nextInt(1, biasFactor) === 1
        ? this.biasedLengthArb
        : this.lengthArb;
    const size = lengthArb.generate(mrng, void 0);
    const sizeValue = size.value;
    const remainingElements = safeMap(this.originalArray, (_v, idx) => idx);
    const ids = [];
    for (let index = 0; index !== sizeValue; ++index) {
      const selectedIdIndex = mrng.nextInt(0, remainingElements.length - 1);
      safePush(ids, remainingElements[selectedIdIndex]);
      safeSplice(remainingElements, selectedIdIndex, 1);
    }
    if (this.isOrdered) {
      safeSort(ids, (a, b) => a - b);
    }
    return new Value(
      safeMap(ids, (i) => this.originalArray[i]),
      size.context,
    );
  }
  canShrinkWithoutContext(value) {
    if (!safeArrayIsArray4(value)) {
      return false;
    }
    if (!this.lengthArb.canShrinkWithoutContext(value.length)) {
      return false;
    }
    return isSubarrayOf(this.originalArray, value);
  }
  shrink(value, context2) {
    if (value.length === 0) {
      return Stream.nil();
    }
    return this.lengthArb
      .shrink(value.length, context2)
      .map((newSize) => {
        return new Value(
          safeSlice(value, value.length - newSize.value),
          newSize.context,
        );
      })
      .join(
        value.length > this.minLength
          ? makeLazy(() =>
              this.shrink(safeSlice(value, 1), void 0)
                .filter(
                  (newValue) => this.minLength <= newValue.value.length + 1,
                )
                .map(
                  (newValue) =>
                    new Value([value[0], ...newValue.value], void 0),
                ),
            )
          : Stream.nil(),
      );
  }
};

// ../../../node_modules/fast-check/lib/esm/arbitrary/subarray.js
function subarray(originalArray, constraints = {}) {
  const { minLength = 0, maxLength = originalArray.length } = constraints;
  return new SubarrayArbitrary(originalArray, true, minLength, maxLength);
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/shuffledSubarray.js
function shuffledSubarray(originalArray, constraints = {}) {
  const { minLength = 0, maxLength = originalArray.length } = constraints;
  return new SubarrayArbitrary(originalArray, false, minLength, maxLength);
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/_internals/mappers/UintToBase32String.js
var encodeSymbolLookupTable = {
  10: 'A',
  11: 'B',
  12: 'C',
  13: 'D',
  14: 'E',
  15: 'F',
  16: 'G',
  17: 'H',
  18: 'J',
  19: 'K',
  20: 'M',
  21: 'N',
  22: 'P',
  23: 'Q',
  24: 'R',
  25: 'S',
  26: 'T',
  27: 'V',
  28: 'W',
  29: 'X',
  30: 'Y',
  31: 'Z',
};
var decodeSymbolLookupTable = {
  0: 0,
  1: 1,
  2: 2,
  3: 3,
  4: 4,
  5: 5,
  6: 6,
  7: 7,
  8: 8,
  9: 9,
  A: 10,
  B: 11,
  C: 12,
  D: 13,
  E: 14,
  F: 15,
  G: 16,
  H: 17,
  J: 18,
  K: 19,
  M: 20,
  N: 21,
  P: 22,
  Q: 23,
  R: 24,
  S: 25,
  T: 26,
  V: 27,
  W: 28,
  X: 29,
  Y: 30,
  Z: 31,
};
function encodeSymbol(symbol) {
  return symbol < 10 ? SString(symbol) : encodeSymbolLookupTable[symbol];
}
function pad(value, paddingLength) {
  let extraPadding = '';
  while (value.length + extraPadding.length < paddingLength) {
    extraPadding += '0';
  }
  return extraPadding + value;
}
function smallUintToBase32StringMapper(num) {
  let base32Str = '';
  for (let remaining = num; remaining !== 0; ) {
    const next = remaining >> 5;
    const current = remaining - (next << 5);
    base32Str = encodeSymbol(current) + base32Str;
    remaining = next;
  }
  return base32Str;
}
function uintToBase32StringMapper(num, paddingLength) {
  const head = ~~(num / 1073741824);
  const tail = num & 1073741823;
  return (
    pad(smallUintToBase32StringMapper(head), paddingLength - 6) +
    pad(smallUintToBase32StringMapper(tail), 6)
  );
}
function paddedUintToBase32StringMapper(paddingLength) {
  return function padded(num) {
    return uintToBase32StringMapper(num, paddingLength);
  };
}
function uintToBase32StringUnmapper(value) {
  if (typeof value !== 'string') {
    throw new SError('Unsupported type');
  }
  let accumulated = 0;
  let power = 1;
  for (let index = value.length - 1; index >= 0; --index) {
    const char2 = value[index];
    const numericForChar = decodeSymbolLookupTable[char2];
    if (numericForChar === void 0) {
      throw new SError('Unsupported type');
    }
    accumulated += numericForChar * power;
    power *= 32;
  }
  return accumulated;
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/ulid.js
var padded10Mapper = paddedUintToBase32StringMapper(10);
var padded8Mapper = paddedUintToBase32StringMapper(8);
function ulidMapper(parts) {
  return (
    padded10Mapper(parts[0]) + padded8Mapper(parts[1]) + padded8Mapper(parts[2])
  );
}
function ulidUnmapper(value) {
  if (typeof value !== 'string' || value.length !== 26) {
    throw new Error('Unsupported type');
  }
  return [
    uintToBase32StringUnmapper(value.slice(0, 10)),
    uintToBase32StringUnmapper(value.slice(10, 18)),
    uintToBase32StringUnmapper(value.slice(18)),
  ];
}
function ulid() {
  const timestampPartArbitrary = integer({ min: 0, max: 281474976710655 });
  const randomnessPartOneArbitrary = integer({ min: 0, max: 1099511627775 });
  const randomnessPartTwoArbitrary = integer({ min: 0, max: 1099511627775 });
  return tuple(
    timestampPartArbitrary,
    randomnessPartOneArbitrary,
    randomnessPartTwoArbitrary,
  ).map(ulidMapper, ulidUnmapper);
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/_internals/mappers/NumberToPaddedEight.js
function numberToPaddedEightMapper(n) {
  return safePadStart(safeNumberToString(n, 16), 8, '0');
}
function numberToPaddedEightUnmapper(value) {
  if (typeof value !== 'string') {
    throw new Error('Unsupported type');
  }
  if (value.length !== 8) {
    throw new Error('Unsupported value: invalid length');
  }
  const n = parseInt(value, 16);
  if (value !== numberToPaddedEightMapper(n)) {
    throw new Error('Unsupported value: invalid content');
  }
  return n;
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/_internals/builders/PaddedNumberArbitraryBuilder.js
function buildPaddedNumberArbitrary(min, max) {
  return integer({ min, max }).map(
    numberToPaddedEightMapper,
    numberToPaddedEightUnmapper,
  );
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/_internals/mappers/PaddedEightsToUuid.js
function paddedEightsToUuidMapper(t) {
  return `${t[0]}-${safeSubstring(t[1], 4)}-${safeSubstring(t[1], 0, 4)}-${safeSubstring(t[2], 0, 4)}-${safeSubstring(t[2], 4)}${t[3]}`;
}
var UuidRegex =
  /^([0-9a-f]{8})-([0-9a-f]{4})-([0-9a-f]{4})-([0-9a-f]{4})-([0-9a-f]{12})$/;
function paddedEightsToUuidUnmapper(value) {
  if (typeof value !== 'string') {
    throw new Error('Unsupported type');
  }
  const m = UuidRegex.exec(value);
  if (m === null) {
    throw new Error('Unsupported type');
  }
  return [
    m[1],
    m[3] + m[2],
    m[4] + safeSubstring(m[5], 0, 4),
    safeSubstring(m[5], 4),
  ];
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/uuid.js
function uuid() {
  const padded = buildPaddedNumberArbitrary(0, 4294967295);
  const secondPadded = buildPaddedNumberArbitrary(268435456, 1610612735);
  const thirdPadded = buildPaddedNumberArbitrary(2147483648, 3221225471);
  return tuple(padded, secondPadded, thirdPadded, padded).map(
    paddedEightsToUuidMapper,
    paddedEightsToUuidUnmapper,
  );
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/uuidV.js
function uuidV(versionNumber) {
  const padded = buildPaddedNumberArbitrary(0, 4294967295);
  const offsetSecond = versionNumber * 268435456;
  const secondPadded = buildPaddedNumberArbitrary(
    offsetSecond,
    offsetSecond + 268435455,
  );
  const thirdPadded = buildPaddedNumberArbitrary(2147483648, 3221225471);
  return tuple(padded, secondPadded, thirdPadded, padded).map(
    paddedEightsToUuidMapper,
    paddedEightsToUuidUnmapper,
  );
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/webAuthority.js
function hostUserInfo(size) {
  const others = [
    '-',
    '.',
    '_',
    '~',
    '!',
    '$',
    '&',
    "'",
    '(',
    ')',
    '*',
    '+',
    ',',
    ';',
    '=',
    ':',
  ];
  return stringOf(buildAlphaNumericPercentArbitrary(others), { size });
}
function userHostPortMapper([u, h2, p]) {
  return (u === null ? '' : `${u}@`) + h2 + (p === null ? '' : `:${p}`);
}
function userHostPortUnmapper(value) {
  if (typeof value !== 'string') {
    throw new Error('Unsupported');
  }
  const atPosition = value.indexOf('@');
  const user = atPosition !== -1 ? value.substring(0, atPosition) : null;
  const portRegex = /:(\d+)$/;
  const m = portRegex.exec(value);
  const port = m !== null ? Number(m[1]) : null;
  const host =
    m !== null
      ? value.substring(atPosition + 1, value.length - m[1].length - 1)
      : value.substring(atPosition + 1);
  return [user, host, port];
}
function bracketedMapper(s) {
  return `[${s}]`;
}
function bracketedUnmapper(value) {
  if (
    typeof value !== 'string' ||
    value[0] !== '[' ||
    value[value.length - 1] !== ']'
  ) {
    throw new Error('Unsupported');
  }
  return value.substring(1, value.length - 1);
}
function webAuthority(constraints) {
  const c = constraints || {};
  const size = c.size;
  const hostnameArbs = [
    domain({ size }),
    ...(c.withIPv4 === true ? [ipV4()] : []),
    ...(c.withIPv6 === true
      ? [ipV6().map(bracketedMapper, bracketedUnmapper)]
      : []),
    ...(c.withIPv4Extended === true ? [ipV4Extended()] : []),
  ];
  return tuple(
    c.withUserInfo === true ? option(hostUserInfo(size)) : constant(null),
    oneof(...hostnameArbs),
    c.withPort === true ? option(nat(65535)) : constant(null),
  ).map(userHostPortMapper, userHostPortUnmapper);
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/_internals/builders/UriQueryOrFragmentArbitraryBuilder.js
function buildUriQueryOrFragmentArbitrary(size) {
  const others = [
    '-',
    '.',
    '_',
    '~',
    '!',
    '$',
    '&',
    "'",
    '(',
    ')',
    '*',
    '+',
    ',',
    ';',
    '=',
    ':',
    '@',
    '/',
    '?',
  ];
  return stringOf(buildAlphaNumericPercentArbitrary(others), { size });
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/webFragments.js
function webFragments(constraints = {}) {
  return buildUriQueryOrFragmentArbitrary(constraints.size);
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/webSegment.js
function webSegment(constraints = {}) {
  const others = [
    '-',
    '.',
    '_',
    '~',
    '!',
    '$',
    '&',
    "'",
    '(',
    ')',
    '*',
    '+',
    ',',
    ';',
    '=',
    ':',
    '@',
  ];
  return stringOf(buildAlphaNumericPercentArbitrary(others), {
    size: constraints.size,
  });
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/_internals/mappers/SegmentsToPath.js
function segmentsToPathMapper(segments) {
  return safeJoin(
    safeMap(segments, (v) => `/${v}`),
    '',
  );
}
function segmentsToPathUnmapper(value) {
  if (typeof value !== 'string') {
    throw new Error('Incompatible value received: type');
  }
  if (value.length !== 0 && value[0] !== '/') {
    throw new Error('Incompatible value received: start');
  }
  return safeSplice(safeSplit(value, '/'), 1);
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/_internals/builders/UriPathArbitraryBuilder.js
function sqrtSize(size) {
  switch (size) {
    case 'xsmall':
      return ['xsmall', 'xsmall'];
    case 'small':
      return ['small', 'xsmall'];
    case 'medium':
      return ['small', 'small'];
    case 'large':
      return ['medium', 'small'];
    case 'xlarge':
      return ['medium', 'medium'];
  }
}
function buildUriPathArbitrary(resolvedSize) {
  const [segmentSize, numSegmentSize] = sqrtSize(resolvedSize);
  return array(webSegment({ size: segmentSize }), { size: numSegmentSize }).map(
    segmentsToPathMapper,
    segmentsToPathUnmapper,
  );
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/webPath.js
function webPath(constraints) {
  const c = constraints || {};
  const resolvedSize = resolveSize(c.size);
  return buildUriPathArbitrary(resolvedSize);
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/webQueryParameters.js
function webQueryParameters(constraints = {}) {
  return buildUriQueryOrFragmentArbitrary(constraints.size);
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/_internals/mappers/PartsToUrl.js
function partsToUrlMapper(data) {
  const [scheme, authority, path] = data;
  const query = data[3] === null ? '' : `?${data[3]}`;
  const fragments = data[4] === null ? '' : `#${data[4]}`;
  return `${scheme}://${authority}${path}${query}${fragments}`;
}
var UrlSplitRegex =
  /^([[A-Za-z][A-Za-z0-9+.-]*):\/\/([^/?#]*)([^?#]*)(\?[A-Za-z0-9\-._~!$&'()*+,;=:@/?%]*)?(#[A-Za-z0-9\-._~!$&'()*+,;=:@/?%]*)?$/;
function partsToUrlUnmapper(value) {
  if (typeof value !== 'string') {
    throw new Error('Incompatible value received: type');
  }
  const m = UrlSplitRegex.exec(value);
  if (m === null) {
    throw new Error('Incompatible value received');
  }
  const scheme = m[1];
  const authority = m[2];
  const path = m[3];
  const query = m[4];
  const fragments = m[5];
  return [
    scheme,
    authority,
    path,
    query !== void 0 ? query.substring(1) : null,
    fragments !== void 0 ? fragments.substring(1) : null,
  ];
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/webUrl.js
var safeObjectAssign14 = Object.assign;
function webUrl(constraints) {
  const c = constraints || {};
  const resolvedSize = resolveSize(c.size);
  const resolvedAuthoritySettingsSize =
    c.authoritySettings !== void 0 && c.authoritySettings.size !== void 0
      ? relativeSizeToSize(c.authoritySettings.size, resolvedSize)
      : resolvedSize;
  const resolvedAuthoritySettings = safeObjectAssign14(
    safeObjectAssign14({}, c.authoritySettings),
    {
      size: resolvedAuthoritySettingsSize,
    },
  );
  const validSchemes = c.validSchemes || ['http', 'https'];
  const schemeArb = constantFrom(...validSchemes);
  const authorityArb = webAuthority(resolvedAuthoritySettings);
  return tuple(
    schemeArb,
    authorityArb,
    webPath({ size: resolvedSize }),
    c.withQueryParameters === true
      ? option(webQueryParameters({ size: resolvedSize }))
      : constant(null),
    c.withFragments === true
      ? option(webFragments({ size: resolvedSize }))
      : constant(null),
  ).map(partsToUrlMapper, partsToUrlUnmapper);
}

// ../../../node_modules/fast-check/lib/esm/check/model/commands/CommandsIterable.js
var CommandsIterable = class _CommandsIterable {
  constructor(commands2, metadataForReplay) {
    this.commands = commands2;
    this.metadataForReplay = metadataForReplay;
  }
  [Symbol.iterator]() {
    return this.commands[Symbol.iterator]();
  }
  [cloneMethod]() {
    return new _CommandsIterable(
      this.commands.map((c) => c.clone()),
      this.metadataForReplay,
    );
  }
  toString() {
    const serializedCommands = this.commands
      .filter((c) => c.hasRan)
      .map((c) => c.toString())
      .join(',');
    const metadata = this.metadataForReplay();
    return metadata.length !== 0
      ? `${serializedCommands} /*${metadata}*/`
      : serializedCommands;
  }
};

// ../../../node_modules/fast-check/lib/esm/check/model/commands/CommandWrapper.js
var CommandWrapper = class _CommandWrapper {
  constructor(cmd) {
    this.cmd = cmd;
    this.hasRan = false;
    if (hasToStringMethod(cmd)) {
      const method = cmd[toStringMethod];
      this[toStringMethod] = function toStringMethod2() {
        return method.call(cmd);
      };
    }
    if (hasAsyncToStringMethod(cmd)) {
      const method = cmd[asyncToStringMethod];
      this[asyncToStringMethod] = function asyncToStringMethod2() {
        return method.call(cmd);
      };
    }
  }
  check(m) {
    return this.cmd.check(m);
  }
  run(m, r) {
    this.hasRan = true;
    return this.cmd.run(m, r);
  }
  clone() {
    if (hasCloneMethod(this.cmd))
      return new _CommandWrapper(this.cmd[cloneMethod]());
    return new _CommandWrapper(this.cmd);
  }
  toString() {
    return this.cmd.toString();
  }
};

// ../../../node_modules/fast-check/lib/esm/check/model/ReplayPath.js
var ReplayPath = class {
  static parse(replayPathStr) {
    const [serializedCount, serializedChanges] = replayPathStr.split(':');
    const counts = this.parseCounts(serializedCount);
    const changes = this.parseChanges(serializedChanges);
    return this.parseOccurences(counts, changes);
  }
  static stringify(replayPath) {
    const occurences = this.countOccurences(replayPath);
    const serializedCount = this.stringifyCounts(occurences);
    const serializedChanges = this.stringifyChanges(occurences);
    return `${serializedCount}:${serializedChanges}`;
  }
  static intToB64(n) {
    if (n < 26) return String.fromCharCode(n + 65);
    if (n < 52) return String.fromCharCode(n + 97 - 26);
    if (n < 62) return String.fromCharCode(n + 48 - 52);
    return String.fromCharCode(n === 62 ? 43 : 47);
  }
  static b64ToInt(c) {
    if (c >= 'a') return c.charCodeAt(0) - 97 + 26;
    if (c >= 'A') return c.charCodeAt(0) - 65;
    if (c >= '0') return c.charCodeAt(0) - 48 + 52;
    return c === '+' ? 62 : 63;
  }
  static countOccurences(replayPath) {
    return replayPath.reduce((counts, cur) => {
      if (
        counts.length === 0 ||
        counts[counts.length - 1].count === 64 ||
        counts[counts.length - 1].value !== cur
      )
        counts.push({ value: cur, count: 1 });
      else counts[counts.length - 1].count += 1;
      return counts;
    }, []);
  }
  static parseOccurences(counts, changes) {
    const replayPath = [];
    for (let idx = 0; idx !== counts.length; ++idx) {
      const count = counts[idx];
      const value = changes[idx];
      for (let num = 0; num !== count; ++num) replayPath.push(value);
    }
    return replayPath;
  }
  static stringifyChanges(occurences) {
    let serializedChanges = '';
    for (let idx = 0; idx < occurences.length; idx += 6) {
      const changesInt = occurences
        .slice(idx, idx + 6)
        .reduceRight((prev, cur) => prev * 2 + (cur.value ? 1 : 0), 0);
      serializedChanges += this.intToB64(changesInt);
    }
    return serializedChanges;
  }
  static parseChanges(serializedChanges) {
    const changesInt = serializedChanges.split('').map((c) => this.b64ToInt(c));
    const changes = [];
    for (let idx = 0; idx !== changesInt.length; ++idx) {
      let current = changesInt[idx];
      for (let n = 0; n !== 6; ++n, current >>= 1) {
        changes.push(current % 2 === 1);
      }
    }
    return changes;
  }
  static stringifyCounts(occurences) {
    return occurences.map(({ count }) => this.intToB64(count - 1)).join('');
  }
  static parseCounts(serializedCount) {
    return serializedCount.split('').map((c) => this.b64ToInt(c) + 1);
  }
};

// ../../../node_modules/fast-check/lib/esm/arbitrary/_internals/CommandsArbitrary.js
var CommandsArbitrary = class extends Arbitrary {
  constructor(
    commandArbs,
    maxGeneratedCommands,
    maxCommands,
    sourceReplayPath,
    disableReplayLog,
  ) {
    super();
    this.sourceReplayPath = sourceReplayPath;
    this.disableReplayLog = disableReplayLog;
    this.oneCommandArb = oneof(...commandArbs).map(
      (c) => new CommandWrapper(c),
    );
    this.lengthArb = restrictedIntegerArbitraryBuilder(
      0,
      maxGeneratedCommands,
      maxCommands,
    );
    this.replayPath = [];
    this.replayPathPosition = 0;
  }
  metadataForReplay() {
    return this.disableReplayLog
      ? ''
      : `replayPath=${JSON.stringify(ReplayPath.stringify(this.replayPath))}`;
  }
  buildValueFor(items, shrunkOnce) {
    const commands2 = items.map((item) => item.value_);
    const context2 = { shrunkOnce, items };
    return new Value(
      new CommandsIterable(commands2, () => this.metadataForReplay()),
      context2,
    );
  }
  generate(mrng) {
    const size = this.lengthArb.generate(mrng, void 0);
    const sizeValue = size.value;
    const items = Array(sizeValue);
    for (let idx = 0; idx !== sizeValue; ++idx) {
      const item = this.oneCommandArb.generate(mrng, void 0);
      items[idx] = item;
    }
    this.replayPathPosition = 0;
    return this.buildValueFor(items, false);
  }
  canShrinkWithoutContext(value) {
    return false;
  }
  filterOnExecution(itemsRaw) {
    const items = [];
    for (const c of itemsRaw) {
      if (c.value_.hasRan) {
        this.replayPath.push(true);
        items.push(c);
      } else this.replayPath.push(false);
    }
    return items;
  }
  filterOnReplay(itemsRaw) {
    return itemsRaw.filter((c, idx) => {
      const state = this.replayPath[this.replayPathPosition + idx];
      if (state === void 0) throw new Error(`Too short replayPath`);
      if (!state && c.value_.hasRan)
        throw new Error(`Mismatch between replayPath and real execution`);
      return state;
    });
  }
  filterForShrinkImpl(itemsRaw) {
    if (this.replayPathPosition === 0) {
      this.replayPath =
        this.sourceReplayPath !== null
          ? ReplayPath.parse(this.sourceReplayPath)
          : [];
    }
    const items =
      this.replayPathPosition < this.replayPath.length
        ? this.filterOnReplay(itemsRaw)
        : this.filterOnExecution(itemsRaw);
    this.replayPathPosition += itemsRaw.length;
    return items;
  }
  shrink(_value, context2) {
    if (context2 === void 0) {
      return Stream.nil();
    }
    const safeContext = context2;
    const shrunkOnce = safeContext.shrunkOnce;
    const itemsRaw = safeContext.items;
    const items = this.filterForShrinkImpl(itemsRaw);
    if (items.length === 0) {
      return Stream.nil();
    }
    const rootShrink = shrunkOnce
      ? Stream.nil()
      : new Stream([[]][Symbol.iterator]());
    const nextShrinks = [];
    for (let numToKeep = 0; numToKeep !== items.length; ++numToKeep) {
      nextShrinks.push(
        makeLazy(() => {
          const fixedStart = items.slice(0, numToKeep);
          return this.lengthArb
            .shrink(items.length - 1 - numToKeep, void 0)
            .map((l) =>
              fixedStart.concat(items.slice(items.length - (l.value + 1))),
            );
        }),
      );
    }
    for (let itemAt = 0; itemAt !== items.length; ++itemAt) {
      nextShrinks.push(
        makeLazy(() =>
          this.oneCommandArb
            .shrink(items[itemAt].value_, items[itemAt].context)
            .map((v) =>
              items.slice(0, itemAt).concat([v], items.slice(itemAt + 1)),
            ),
        ),
      );
    }
    return rootShrink.join(...nextShrinks).map((shrinkables) => {
      return this.buildValueFor(
        shrinkables.map((c) => new Value(c.value_.clone(), c.context)),
        true,
      );
    });
  }
};

// ../../../node_modules/fast-check/lib/esm/arbitrary/commands.js
function commands(commandArbs, constraints = {}) {
  const {
    size,
    maxCommands = MaxLengthUpperBound,
    disableReplayLog = false,
    replayPath = null,
  } = constraints;
  const specifiedMaxCommands = constraints.maxCommands !== void 0;
  const maxGeneratedCommands = maxGeneratedLengthFromSizeForArbitrary(
    size,
    0,
    maxCommands,
    specifiedMaxCommands,
  );
  return new CommandsArbitrary(
    commandArbs,
    maxGeneratedCommands,
    maxCommands,
    replayPath,
    disableReplayLog,
  );
}

// ../../../node_modules/fast-check/lib/esm/check/model/commands/ScheduledCommand.js
var ScheduledCommand = class {
  constructor(s, cmd) {
    this.s = s;
    this.cmd = cmd;
  }
  async check(m) {
    let error = null;
    let checkPassed = false;
    const status = await this.s.scheduleSequence([
      {
        label: `check@${this.cmd.toString()}`,
        builder: async () => {
          try {
            checkPassed = await Promise.resolve(this.cmd.check(m));
          } catch (err) {
            error = err;
            throw err;
          }
        },
      },
    ]).task;
    if (status.faulty) {
      throw error;
    }
    return checkPassed;
  }
  async run(m, r) {
    let error = null;
    const status = await this.s.scheduleSequence([
      {
        label: `run@${this.cmd.toString()}`,
        builder: async () => {
          try {
            await this.cmd.run(m, r);
          } catch (err) {
            error = err;
            throw err;
          }
        },
      },
    ]).task;
    if (status.faulty) {
      throw error;
    }
  }
};
var scheduleCommands = function* (s, cmds) {
  for (const cmd of cmds) {
    yield new ScheduledCommand(s, cmd);
  }
};

// ../../../node_modules/fast-check/lib/esm/check/model/ModelRunner.js
var genericModelRun = (s, cmds, initialValue, runCmd, then) => {
  return s.then((o) => {
    const { model, real } = o;
    let state = initialValue;
    for (const c of cmds) {
      state = then(state, () => {
        return runCmd(c, model, real);
      });
    }
    return state;
  });
};
var internalModelRun = (s, cmds) => {
  const then = (_p, c) => c();
  const setupProducer = {
    then: (fun) => {
      fun(s());
      return void 0;
    },
  };
  const runSync = (cmd, m, r) => {
    if (cmd.check(m)) cmd.run(m, r);
    return void 0;
  };
  return genericModelRun(setupProducer, cmds, void 0, runSync, then);
};
var isAsyncSetup = (s) => {
  return typeof s.then === 'function';
};
var internalAsyncModelRun = async (
  s,
  cmds,
  defaultPromise = Promise.resolve(),
) => {
  const then = (p, c) => p.then(c);
  const setupProducer = {
    then: (fun) => {
      const out = s();
      if (isAsyncSetup(out)) return out.then(fun);
      else return fun(out);
    },
  };
  const runAsync = async (cmd, m, r) => {
    if (await cmd.check(m)) await cmd.run(m, r);
  };
  return await genericModelRun(
    setupProducer,
    cmds,
    defaultPromise,
    runAsync,
    then,
  );
};
function modelRun(s, cmds) {
  internalModelRun(s, cmds);
}
async function asyncModelRun(s, cmds) {
  await internalAsyncModelRun(s, cmds);
}
async function scheduledModelRun(scheduler2, s, cmds) {
  const scheduledCommands = scheduleCommands(scheduler2, cmds);
  const out = internalAsyncModelRun(
    s,
    scheduledCommands,
    scheduler2.schedule(Promise.resolve(), 'startModel'),
  );
  await scheduler2.waitFor(out);
  await scheduler2.waitAll();
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/_internals/implementations/SchedulerImplem.js
var defaultSchedulerAct = (f) => f();
var SchedulerImplem = class _SchedulerImplem {
  constructor(act, taskSelector) {
    this.act = act;
    this.taskSelector = taskSelector;
    this.lastTaskId = 0;
    this.sourceTaskSelector = taskSelector.clone();
    this.scheduledTasks = [];
    this.triggeredTasks = [];
    this.scheduledWatchers = [];
  }
  static buildLog(reportItem) {
    return `[task\${${reportItem.taskId}}] ${reportItem.label.length !== 0 ? `${reportItem.schedulingType}::${reportItem.label}` : reportItem.schedulingType} ${reportItem.status}${reportItem.outputValue !== void 0 ? ` with value ${escapeForTemplateString(reportItem.outputValue)}` : ''}`;
  }
  log(schedulingType, taskId, label, metadata, status, data) {
    this.triggeredTasks.push({
      status,
      schedulingType,
      taskId,
      label,
      metadata,
      outputValue: data !== void 0 ? stringify(data) : void 0,
    });
  }
  scheduleInternal(
    schedulingType,
    label,
    task,
    metadata,
    customAct,
    thenTaskToBeAwaited,
  ) {
    let trigger = null;
    const taskId = ++this.lastTaskId;
    const scheduledPromise = new Promise((resolve, reject) => {
      trigger = () => {
        (thenTaskToBeAwaited
          ? task.then(() => thenTaskToBeAwaited())
          : task
        ).then(
          (data) => {
            this.log(schedulingType, taskId, label, metadata, 'resolved', data);
            return resolve(data);
          },
          (err) => {
            this.log(schedulingType, taskId, label, metadata, 'rejected', err);
            return reject(err);
          },
        );
      };
    });
    this.scheduledTasks.push({
      original: task,
      scheduled: scheduledPromise,
      trigger,
      schedulingType,
      taskId,
      label,
      metadata,
      customAct,
    });
    if (this.scheduledWatchers.length !== 0) {
      this.scheduledWatchers[0]();
    }
    return scheduledPromise;
  }
  schedule(task, label, metadata, customAct) {
    return this.scheduleInternal(
      'promise',
      label || '',
      task,
      metadata,
      customAct || defaultSchedulerAct,
    );
  }
  scheduleFunction(asyncFunction, customAct) {
    return (...args) =>
      this.scheduleInternal(
        'function',
        `${asyncFunction.name}(${args.map(stringify).join(',')})`,
        asyncFunction(...args),
        void 0,
        customAct || defaultSchedulerAct,
      );
  }
  scheduleSequence(sequenceBuilders, customAct) {
    const status = { done: false, faulty: false };
    const dummyResolvedPromise = { then: (f) => f() };
    let resolveSequenceTask = () => {};
    const sequenceTask = new Promise(
      (resolve) => (resolveSequenceTask = resolve),
    );
    sequenceBuilders
      .reduce((previouslyScheduled, item) => {
        const [builder, label, metadata] =
          typeof item === 'function'
            ? [item, item.name, void 0]
            : [item.builder, item.label, item.metadata];
        return previouslyScheduled.then(() => {
          const scheduled = this.scheduleInternal(
            'sequence',
            label,
            dummyResolvedPromise,
            metadata,
            customAct || defaultSchedulerAct,
            () => builder(),
          );
          scheduled.catch(() => {
            status.faulty = true;
            resolveSequenceTask();
          });
          return scheduled;
        });
      }, dummyResolvedPromise)
      .then(
        () => {
          status.done = true;
          resolveSequenceTask();
        },
        () => {},
      );
    return Object.assign(status, {
      task: Promise.resolve(sequenceTask).then(() => {
        return { done: status.done, faulty: status.faulty };
      }),
    });
  }
  count() {
    return this.scheduledTasks.length;
  }
  internalWaitOne() {
    if (this.scheduledTasks.length === 0) {
      throw new Error('No task scheduled');
    }
    const taskIndex = this.taskSelector.nextTaskIndex(this.scheduledTasks);
    const [scheduledTask] = this.scheduledTasks.splice(taskIndex, 1);
    return scheduledTask.customAct(async () => {
      scheduledTask.trigger();
      try {
        await scheduledTask.scheduled;
      } catch (_err) {}
    });
  }
  async waitOne(customAct) {
    const waitAct = customAct || defaultSchedulerAct;
    await this.act(() => waitAct(async () => await this.internalWaitOne()));
  }
  async waitAll(customAct) {
    while (this.scheduledTasks.length > 0) {
      await this.waitOne(customAct);
    }
  }
  async waitFor(unscheduledTask, customAct) {
    let taskResolved = false;
    let awaiterPromise = null;
    const awaiter = async () => {
      while (!taskResolved && this.scheduledTasks.length > 0) {
        await this.waitOne(customAct);
      }
      awaiterPromise = null;
    };
    const handleNotified = () => {
      if (awaiterPromise !== null) {
        return;
      }
      awaiterPromise = Promise.resolve().then(awaiter);
    };
    const clearAndReplaceWatcher = () => {
      const handleNotifiedIndex =
        this.scheduledWatchers.indexOf(handleNotified);
      if (handleNotifiedIndex !== -1) {
        this.scheduledWatchers.splice(handleNotifiedIndex, 1);
      }
      if (handleNotifiedIndex === 0 && this.scheduledWatchers.length !== 0) {
        this.scheduledWatchers[0]();
      }
    };
    const rewrappedTask = unscheduledTask.then(
      (ret) => {
        taskResolved = true;
        if (awaiterPromise === null) {
          clearAndReplaceWatcher();
          return ret;
        }
        return awaiterPromise.then(() => {
          clearAndReplaceWatcher();
          return ret;
        });
      },
      (err) => {
        taskResolved = true;
        if (awaiterPromise === null) {
          clearAndReplaceWatcher();
          throw err;
        }
        return awaiterPromise.then(() => {
          clearAndReplaceWatcher();
          throw err;
        });
      },
    );
    if (this.scheduledTasks.length > 0 && this.scheduledWatchers.length === 0) {
      handleNotified();
    }
    this.scheduledWatchers.push(handleNotified);
    return rewrappedTask;
  }
  report() {
    return [
      ...this.triggeredTasks,
      ...this.scheduledTasks.map((t) => ({
        status: 'pending',
        schedulingType: t.schedulingType,
        taskId: t.taskId,
        label: t.label,
        metadata: t.metadata,
      })),
    ];
  }
  toString() {
    return (
      'schedulerFor()`\n' +
      this.report()
        .map(_SchedulerImplem.buildLog)
        .map((log) => `-> ${log}`)
        .join('\n') +
      '`'
    );
  }
  [cloneMethod]() {
    return new _SchedulerImplem(this.act, this.sourceTaskSelector);
  }
};

// ../../../node_modules/fast-check/lib/esm/arbitrary/_internals/helpers/BuildSchedulerFor.js
function buildNextTaskIndex(ordering) {
  let numTasks = 0;
  return {
    clone: () => buildNextTaskIndex(ordering),
    nextTaskIndex: (scheduledTasks) => {
      if (ordering.length <= numTasks) {
        throw new Error(
          `Invalid schedulerFor defined: too many tasks have been scheduled`,
        );
      }
      const taskIndex = scheduledTasks.findIndex(
        (t) => t.taskId === ordering[numTasks],
      );
      if (taskIndex === -1) {
        throw new Error(
          `Invalid schedulerFor defined: unable to find next task`,
        );
      }
      ++numTasks;
      return taskIndex;
    },
  };
}
function buildSchedulerFor(act, ordering) {
  return new SchedulerImplem(act, buildNextTaskIndex(ordering));
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/_internals/SchedulerArbitrary.js
function buildNextTaskIndex2(mrng) {
  const clonedMrng = mrng.clone();
  return {
    clone: () => buildNextTaskIndex2(clonedMrng),
    nextTaskIndex: (scheduledTasks) => {
      return mrng.nextInt(0, scheduledTasks.length - 1);
    },
  };
}
var SchedulerArbitrary = class extends Arbitrary {
  constructor(act) {
    super();
    this.act = act;
  }
  generate(mrng, _biasFactor) {
    return new Value(
      new SchedulerImplem(this.act, buildNextTaskIndex2(mrng.clone())),
      void 0,
    );
  }
  canShrinkWithoutContext(value) {
    return false;
  }
  shrink(_value, _context) {
    return Stream.nil();
  }
};

// ../../../node_modules/fast-check/lib/esm/arbitrary/scheduler.js
function scheduler(constraints) {
  const { act = (f) => f() } = constraints || {};
  return new SchedulerArbitrary(act);
}
function schedulerFor(customOrderingOrConstraints, constraintsOrUndefined) {
  const { act = (f) => f() } = Array.isArray(customOrderingOrConstraints)
    ? constraintsOrUndefined || {}
    : customOrderingOrConstraints || {};
  if (Array.isArray(customOrderingOrConstraints)) {
    return buildSchedulerFor(act, customOrderingOrConstraints);
  }
  return function (_strs, ...ordering) {
    return buildSchedulerFor(act, ordering);
  };
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/bigInt64Array.js
function bigInt64Array(constraints = {}) {
  return typedIntArrayArbitraryArbitraryBuilder(
    constraints,
    SBigInt('-9223372036854775808'),
    SBigInt('9223372036854775807'),
    SBigInt64Array,
    bigInt,
  );
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/bigUint64Array.js
function bigUint64Array(constraints = {}) {
  return typedIntArrayArbitraryArbitraryBuilder(
    constraints,
    SBigInt(0),
    SBigInt('18446744073709551615'),
    SBigUint64Array,
    bigInt,
  );
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/_internals/helpers/SanitizeRegexAst.js
function raiseUnsupportedASTNode(astNode) {
  return new Error(`Unsupported AST node! Received: ${stringify(astNode)}`);
}
function addMissingDotStarTraversalAddMissing(astNode, isFirst, isLast) {
  if (!isFirst && !isLast) {
    return astNode;
  }
  const traversalResults = { hasStart: false, hasEnd: false };
  const revampedNode = addMissingDotStarTraversal(
    astNode,
    isFirst,
    isLast,
    traversalResults,
  );
  const missingStart = isFirst && !traversalResults.hasStart;
  const missingEnd = isLast && !traversalResults.hasEnd;
  if (!missingStart && !missingEnd) {
    return revampedNode;
  }
  const expressions = [];
  if (missingStart) {
    expressions.push({ type: 'Assertion', kind: '^' });
    expressions.push({
      type: 'Repetition',
      quantifier: { type: 'Quantifier', kind: '*', greedy: true },
      expression: {
        type: 'Char',
        kind: 'meta',
        symbol: '.',
        value: '.',
        codePoint: Number.NaN,
      },
    });
  }
  expressions.push(revampedNode);
  if (missingEnd) {
    expressions.push({
      type: 'Repetition',
      quantifier: { type: 'Quantifier', kind: '*', greedy: true },
      expression: {
        type: 'Char',
        kind: 'meta',
        symbol: '.',
        value: '.',
        codePoint: Number.NaN,
      },
    });
    expressions.push({ type: 'Assertion', kind: '$' });
  }
  return {
    type: 'Group',
    capturing: false,
    expression: { type: 'Alternative', expressions },
  };
}
function addMissingDotStarTraversal(
  astNode,
  isFirst,
  isLast,
  traversalResults,
) {
  switch (astNode.type) {
    case 'Char':
      return astNode;
    case 'Repetition':
      return astNode;
    case 'Quantifier':
      throw new Error(
        `Wrongly defined AST tree, Quantifier nodes not supposed to be scanned!`,
      );
    case 'Alternative':
      traversalResults.hasStart = true;
      traversalResults.hasEnd = true;
      return Object.assign(Object.assign({}, astNode), {
        expressions: astNode.expressions.map((node, index) =>
          addMissingDotStarTraversalAddMissing(
            node,
            isFirst && index === 0,
            isLast && index === astNode.expressions.length - 1,
          ),
        ),
      });
    case 'CharacterClass':
      return astNode;
    case 'ClassRange':
      return astNode;
    case 'Group': {
      return Object.assign(Object.assign({}, astNode), {
        expression: addMissingDotStarTraversal(
          astNode.expression,
          isFirst,
          isLast,
          traversalResults,
        ),
      });
    }
    case 'Disjunction': {
      traversalResults.hasStart = true;
      traversalResults.hasEnd = true;
      return Object.assign(Object.assign({}, astNode), {
        left:
          astNode.left !== null
            ? addMissingDotStarTraversalAddMissing(
                astNode.left,
                isFirst,
                isLast,
              )
            : null,
        right:
          astNode.right !== null
            ? addMissingDotStarTraversalAddMissing(
                astNode.right,
                isFirst,
                isLast,
              )
            : null,
      });
    }
    case 'Assertion': {
      if (astNode.kind === '^' || astNode.kind === 'Lookahead') {
        traversalResults.hasStart = true;
        return astNode;
      } else if (astNode.kind === '$' || astNode.kind === 'Lookbehind') {
        traversalResults.hasEnd = true;
        return astNode;
      } else {
        throw new Error(
          `Assertions of kind ${astNode.kind} not implemented yet!`,
        );
      }
    }
    case 'Backreference':
      return astNode;
    default:
      throw raiseUnsupportedASTNode(astNode);
  }
}
function addMissingDotStar(astNode) {
  return addMissingDotStarTraversalAddMissing(astNode, true, true);
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/_internals/helpers/ReadRegex.js
function charSizeAt(text, pos) {
  return text[pos] >= '\uD800' &&
    text[pos] <= '\uDBFF' &&
    text[pos + 1] >= '\uDC00' &&
    text[pos + 1] <= '\uDFFF'
    ? 2
    : 1;
}
function isHexaDigit(char2) {
  return (
    (char2 >= '0' && char2 <= '9') ||
    (char2 >= 'a' && char2 <= 'f') ||
    (char2 >= 'A' && char2 <= 'F')
  );
}
function isDigit(char2) {
  return char2 >= '0' && char2 <= '9';
}
function squaredBracketBlockContentEndFrom(text, from) {
  for (let index = from; index !== text.length; ++index) {
    const char2 = text[index];
    if (char2 === '\\') {
      index += 1;
    } else if (char2 === ']') {
      return index;
    }
  }
  throw new Error(`Missing closing ']'`);
}
function parenthesisBlockContentEndFrom(text, from) {
  let numExtraOpened = 0;
  for (let index = from; index !== text.length; ++index) {
    const char2 = text[index];
    if (char2 === '\\') {
      index += 1;
    } else if (char2 === ')') {
      if (numExtraOpened === 0) {
        return index;
      }
      numExtraOpened -= 1;
    } else if (char2 === '[') {
      index = squaredBracketBlockContentEndFrom(text, index);
    } else if (char2 === '(') {
      numExtraOpened += 1;
    }
  }
  throw new Error(`Missing closing ')'`);
}
function curlyBracketBlockContentEndFrom(text, from) {
  let foundComma = false;
  for (let index = from; index !== text.length; ++index) {
    const char2 = text[index];
    if (isDigit(char2)) {
    } else if (from === index) {
      return -1;
    } else if (char2 === ',') {
      if (foundComma) {
        return -1;
      }
      foundComma = true;
    } else if (char2 === '}') {
      return index;
    } else {
      return -1;
    }
  }
  return -1;
}
var TokenizerBlockMode;
(function (TokenizerBlockMode2) {
  TokenizerBlockMode2[(TokenizerBlockMode2['Full'] = 0)] = 'Full';
  TokenizerBlockMode2[(TokenizerBlockMode2['Character'] = 1)] = 'Character';
})(TokenizerBlockMode || (TokenizerBlockMode = {}));
function blockEndFrom(text, from, unicodeMode, mode) {
  switch (text[from]) {
    case '[': {
      if (mode === TokenizerBlockMode.Character) {
        return from + 1;
      }
      return squaredBracketBlockContentEndFrom(text, from + 1) + 1;
    }
    case '{': {
      if (mode === TokenizerBlockMode.Character) {
        return from + 1;
      }
      const foundEnd = curlyBracketBlockContentEndFrom(text, from + 1);
      if (foundEnd === -1) {
        return from + 1;
      }
      return foundEnd + 1;
    }
    case '(': {
      if (mode === TokenizerBlockMode.Character) {
        return from + 1;
      }
      return parenthesisBlockContentEndFrom(text, from + 1) + 1;
    }
    case ']':
    case '}':
    case ')':
      return from + 1;
    case '\\': {
      const next1 = text[from + 1];
      switch (next1) {
        case 'x':
          if (isHexaDigit(text[from + 2]) && isHexaDigit(text[from + 3])) {
            return from + 4;
          }
          throw new Error(
            `Unexpected token '${text.substring(from, from + 4)}' found`,
          );
        case 'u':
          if (text[from + 2] === '{') {
            if (!unicodeMode) {
              return from + 2;
            }
            if (text[from + 4] === '}') {
              if (isHexaDigit(text[from + 3])) {
                return from + 5;
              }
              throw new Error(
                `Unexpected token '${text.substring(from, from + 5)}' found`,
              );
            }
            if (text[from + 5] === '}') {
              if (isHexaDigit(text[from + 3]) && isHexaDigit(text[from + 4])) {
                return from + 6;
              }
              throw new Error(
                `Unexpected token '${text.substring(from, from + 6)}' found`,
              );
            }
            if (text[from + 6] === '}') {
              if (
                isHexaDigit(text[from + 3]) &&
                isHexaDigit(text[from + 4]) &&
                isHexaDigit(text[from + 5])
              ) {
                return from + 7;
              }
              throw new Error(
                `Unexpected token '${text.substring(from, from + 7)}' found`,
              );
            }
            if (text[from + 7] === '}') {
              if (
                isHexaDigit(text[from + 3]) &&
                isHexaDigit(text[from + 4]) &&
                isHexaDigit(text[from + 5]) &&
                isHexaDigit(text[from + 6])
              ) {
                return from + 8;
              }
              throw new Error(
                `Unexpected token '${text.substring(from, from + 8)}' found`,
              );
            }
            if (
              text[from + 8] === '}' &&
              isHexaDigit(text[from + 3]) &&
              isHexaDigit(text[from + 4]) &&
              isHexaDigit(text[from + 5]) &&
              isHexaDigit(text[from + 6]) &&
              isHexaDigit(text[from + 7])
            ) {
              return from + 9;
            }
            throw new Error(
              `Unexpected token '${text.substring(from, from + 9)}' found`,
            );
          }
          if (
            isHexaDigit(text[from + 2]) &&
            isHexaDigit(text[from + 3]) &&
            isHexaDigit(text[from + 4]) &&
            isHexaDigit(text[from + 5])
          ) {
            return from + 6;
          }
          throw new Error(
            `Unexpected token '${text.substring(from, from + 6)}' found`,
          );
        case 'p':
        case 'P': {
          if (!unicodeMode) {
            return from + 2;
          }
          let subIndex = from + 2;
          for (
            ;
            subIndex < text.length && text[subIndex] !== '}';
            subIndex += text[subIndex] === '\\' ? 2 : 1
          ) {}
          if (text[subIndex] !== '}') {
            throw new Error(`Invalid \\P definition`);
          }
          return subIndex + 1;
        }
        case 'k': {
          let subIndex = from + 2;
          for (
            ;
            subIndex < text.length && text[subIndex] !== '>';
            ++subIndex
          ) {}
          if (text[subIndex] !== '>') {
            if (!unicodeMode) {
              return from + 2;
            }
            throw new Error(`Invalid \\k definition`);
          }
          return subIndex + 1;
        }
        default: {
          if (isDigit(next1)) {
            const maxIndex = unicodeMode
              ? text.length
              : Math.min(from + 4, text.length);
            let subIndex = from + 2;
            for (
              ;
              subIndex < maxIndex && isDigit(text[subIndex]);
              ++subIndex
            ) {}
            return subIndex;
          }
          const charSize = unicodeMode ? charSizeAt(text, from + 1) : 1;
          return from + charSize + 1;
        }
      }
    }
    default: {
      const charSize = unicodeMode ? charSizeAt(text, from) : 1;
      return from + charSize;
    }
  }
}
function readFrom(text, from, unicodeMode, mode) {
  const to = blockEndFrom(text, from, unicodeMode, mode);
  return text.substring(from, to);
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/_internals/helpers/TokenizeRegex.js
var safeStringFromCodePoint = String.fromCodePoint;
function safePop2(tokens) {
  const previous = tokens.pop();
  if (previous === void 0) {
    throw new Error(
      'Unable to extract token preceeding the currently parsed one',
    );
  }
  return previous;
}
function isDigit2(char2) {
  return char2 >= '0' && char2 <= '9';
}
function simpleChar(char2, escaped) {
  return {
    type: 'Char',
    kind: 'simple',
    symbol: char2,
    value: char2,
    codePoint: char2.codePointAt(0) || -1,
    escaped,
  };
}
function metaEscapedChar(block, symbol) {
  return {
    type: 'Char',
    kind: 'meta',
    symbol,
    value: block,
    codePoint: symbol.codePointAt(0) || -1,
  };
}
function toSingleToken(tokens, allowEmpty) {
  if (tokens.length > 1) {
    return {
      type: 'Alternative',
      expressions: tokens,
    };
  }
  if (!allowEmpty && tokens.length === 0) {
    throw new Error(`Unsupported no token`);
  }
  return tokens[0];
}
function blockToCharToken(block) {
  if (block[0] === '\\') {
    const next = block[1];
    switch (next) {
      case 'x': {
        const allDigits = block.substring(2);
        const codePoint = Number.parseInt(allDigits, 16);
        const symbol = safeStringFromCodePoint(codePoint);
        return { type: 'Char', kind: 'hex', symbol, value: block, codePoint };
      }
      case 'u': {
        if (block === '\\u') {
          return simpleChar('u', true);
        }
        const allDigits =
          block[2] === '{'
            ? block.substring(3, block.length - 1)
            : block.substring(2);
        const codePoint = Number.parseInt(allDigits, 16);
        const symbol = safeStringFromCodePoint(codePoint);
        return {
          type: 'Char',
          kind: 'unicode',
          symbol,
          value: block,
          codePoint,
        };
      }
      case '0': {
        return metaEscapedChar(block, '\0');
      }
      case 'n': {
        return metaEscapedChar(block, '\n');
      }
      case 'f': {
        return metaEscapedChar(block, '\f');
      }
      case 'r': {
        return metaEscapedChar(block, '\r');
      }
      case 't': {
        return metaEscapedChar(block, '	');
      }
      case 'v': {
        return metaEscapedChar(block, '\v');
      }
      case 'w':
      case 'W':
      case 'd':
      case 'D':
      case 's':
      case 'S':
      case 'b':
      case 'B': {
        return {
          type: 'Char',
          kind: 'meta',
          symbol: void 0,
          value: block,
          codePoint: Number.NaN,
        };
      }
      default: {
        if (isDigit2(next)) {
          const allDigits = block.substring(1);
          const codePoint = Number(allDigits);
          const symbol = safeStringFromCodePoint(codePoint);
          return {
            type: 'Char',
            kind: 'decimal',
            symbol,
            value: block,
            codePoint,
          };
        }
        if (block.length > 2 && (next === 'p' || next === 'P')) {
          throw new Error(`UnicodeProperty not implemented yet!`);
        }
        const char2 = block.substring(1);
        return simpleChar(char2, true);
      }
    }
  }
  return simpleChar(block);
}
function pushTokens(tokens, regexSource, unicodeMode, groups) {
  let disjunctions = null;
  for (
    let index = 0,
      block = readFrom(
        regexSource,
        index,
        unicodeMode,
        TokenizerBlockMode.Full,
      );
    index !== regexSource.length;
    index += block.length,
      block = readFrom(regexSource, index, unicodeMode, TokenizerBlockMode.Full)
  ) {
    const firstInBlock = block[0];
    switch (firstInBlock) {
      case '|': {
        if (disjunctions === null) {
          disjunctions = [];
        }
        disjunctions.push(toSingleToken(tokens.splice(0), true) || null);
        break;
      }
      case '.': {
        tokens.push({
          type: 'Char',
          kind: 'meta',
          symbol: block,
          value: block,
          codePoint: Number.NaN,
        });
        break;
      }
      case '*':
      case '+': {
        const previous = safePop2(tokens);
        tokens.push({
          type: 'Repetition',
          expression: previous,
          quantifier: { type: 'Quantifier', kind: firstInBlock, greedy: true },
        });
        break;
      }
      case '?': {
        const previous = safePop2(tokens);
        if (previous.type === 'Repetition') {
          previous.quantifier.greedy = false;
          tokens.push(previous);
        } else {
          tokens.push({
            type: 'Repetition',
            expression: previous,
            quantifier: {
              type: 'Quantifier',
              kind: firstInBlock,
              greedy: true,
            },
          });
        }
        break;
      }
      case '{': {
        if (block === '{') {
          tokens.push(simpleChar(block));
          break;
        }
        const previous = safePop2(tokens);
        const quantifierText = block.substring(1, block.length - 1);
        const quantifierTokens = quantifierText.split(',');
        const from = Number(quantifierTokens[0]);
        const to =
          quantifierTokens.length === 1
            ? from
            : quantifierTokens[1].length !== 0
              ? Number(quantifierTokens[1])
              : void 0;
        tokens.push({
          type: 'Repetition',
          expression: previous,
          quantifier: {
            type: 'Quantifier',
            kind: 'Range',
            greedy: true,
            from,
            to,
          },
        });
        break;
      }
      case '[': {
        const blockContent = block.substring(1, block.length - 1);
        const subTokens = [];
        let negative = void 0;
        let previousWasSimpleDash = false;
        for (
          let subIndex = 0,
            subBlock = readFrom(
              blockContent,
              subIndex,
              unicodeMode,
              TokenizerBlockMode.Character,
            );
          subIndex !== blockContent.length;
          subIndex += subBlock.length,
            subBlock = readFrom(
              blockContent,
              subIndex,
              unicodeMode,
              TokenizerBlockMode.Character,
            )
        ) {
          if (subIndex === 0 && subBlock === '^') {
            negative = true;
            continue;
          }
          const newToken = blockToCharToken(subBlock);
          if (subBlock === '-') {
            subTokens.push(newToken);
            previousWasSimpleDash = true;
          } else {
            const operand1Token =
              subTokens.length >= 2 ? subTokens[subTokens.length - 2] : void 0;
            if (
              previousWasSimpleDash &&
              operand1Token !== void 0 &&
              operand1Token.type === 'Char'
            ) {
              subTokens.pop();
              subTokens.pop();
              subTokens.push({
                type: 'ClassRange',
                from: operand1Token,
                to: newToken,
              });
            } else {
              subTokens.push(newToken);
            }
            previousWasSimpleDash = false;
          }
        }
        tokens.push({
          type: 'CharacterClass',
          expressions: subTokens,
          negative,
        });
        break;
      }
      case '(': {
        const blockContent = block.substring(1, block.length - 1);
        const subTokens = [];
        if (blockContent[0] === '?') {
          if (blockContent[1] === ':') {
            pushTokens(
              subTokens,
              blockContent.substring(2),
              unicodeMode,
              groups,
            );
            tokens.push({
              type: 'Group',
              capturing: false,
              expression: toSingleToken(subTokens),
            });
          } else if (blockContent[1] === '=' || blockContent[1] === '!') {
            pushTokens(
              subTokens,
              blockContent.substring(2),
              unicodeMode,
              groups,
            );
            tokens.push({
              type: 'Assertion',
              kind: 'Lookahead',
              negative: blockContent[1] === '!' ? true : void 0,
              assertion: toSingleToken(subTokens),
            });
          } else if (
            blockContent[1] === '<' &&
            (blockContent[2] === '=' || blockContent[2] === '!')
          ) {
            pushTokens(
              subTokens,
              blockContent.substring(3),
              unicodeMode,
              groups,
            );
            tokens.push({
              type: 'Assertion',
              kind: 'Lookbehind',
              negative: blockContent[2] === '!' ? true : void 0,
              assertion: toSingleToken(subTokens),
            });
          } else {
            const chunks = blockContent.split('>');
            if (chunks.length < 2 || chunks[0][1] !== '<') {
              throw new Error(
                `Unsupported regex content found at ${JSON.stringify(block)}`,
              );
            }
            const groupIndex = ++groups.lastIndex;
            const nameRaw = chunks[0].substring(2);
            groups.named.set(nameRaw, groupIndex);
            pushTokens(
              subTokens,
              chunks.slice(1).join('>'),
              unicodeMode,
              groups,
            );
            tokens.push({
              type: 'Group',
              capturing: true,
              nameRaw,
              name: nameRaw,
              number: groupIndex,
              expression: toSingleToken(subTokens),
            });
          }
        } else {
          const groupIndex = ++groups.lastIndex;
          pushTokens(subTokens, blockContent, unicodeMode, groups);
          tokens.push({
            type: 'Group',
            capturing: true,
            number: groupIndex,
            expression: toSingleToken(subTokens),
          });
        }
        break;
      }
      default: {
        if (block === '^') {
          tokens.push({ type: 'Assertion', kind: block });
        } else if (block === '$') {
          tokens.push({ type: 'Assertion', kind: block });
        } else if (block[0] === '\\' && isDigit2(block[1])) {
          const reference = Number(block.substring(1));
          if (unicodeMode || reference <= groups.lastIndex) {
            tokens.push({
              type: 'Backreference',
              kind: 'number',
              number: reference,
              reference,
            });
          } else {
            tokens.push(blockToCharToken(block));
          }
        } else if (
          block[0] === '\\' &&
          block[1] === 'k' &&
          block.length !== 2
        ) {
          const referenceRaw = block.substring(3, block.length - 1);
          tokens.push({
            type: 'Backreference',
            kind: 'name',
            number: groups.named.get(referenceRaw) || 0,
            referenceRaw,
            reference: referenceRaw,
          });
        } else {
          tokens.push(blockToCharToken(block));
        }
        break;
      }
    }
  }
  if (disjunctions !== null) {
    disjunctions.push(toSingleToken(tokens.splice(0), true) || null);
    let currentDisjunction = {
      type: 'Disjunction',
      left: disjunctions[0],
      right: disjunctions[1],
    };
    for (let index = 2; index < disjunctions.length; ++index) {
      currentDisjunction = {
        type: 'Disjunction',
        left: currentDisjunction,
        right: disjunctions[index],
      };
    }
    tokens.push(currentDisjunction);
  }
}
function tokenizeRegex(regex) {
  const unicodeMode = safeIndexOf([...regex.flags], 'u') !== -1;
  const regexSource = regex.source;
  const tokens = [];
  pushTokens(tokens, regexSource, unicodeMode, {
    lastIndex: 0,
    named: /* @__PURE__ */ new Map(),
  });
  return toSingleToken(tokens);
}

// ../../../node_modules/fast-check/lib/esm/arbitrary/stringMatching.js
var safeStringFromCodePoint2 = String.fromCodePoint;
var wordChars = [
  ...'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_',
];
var digitChars = [...'0123456789'];
var spaceChars = [...' 	\r\n\v\f'];
var newLineChars = [...'\r\n'];
var terminatorChars = [...''];
var newLineAndTerminatorChars = [...newLineChars, ...terminatorChars];
var defaultChar = char();
function raiseUnsupportedASTNode2(astNode) {
  return new SError(`Unsupported AST node! Received: ${stringify(astNode)}`);
}
function toMatchingArbitrary(astNode, constraints, flags) {
  switch (astNode.type) {
    case 'Char': {
      if (astNode.kind === 'meta') {
        switch (astNode.value) {
          case '\\w': {
            return constantFrom(...wordChars);
          }
          case '\\W': {
            return defaultChar.filter((c) => safeIndexOf(wordChars, c) === -1);
          }
          case '\\d': {
            return constantFrom(...digitChars);
          }
          case '\\D': {
            return defaultChar.filter((c) => safeIndexOf(digitChars, c) === -1);
          }
          case '\\s': {
            return constantFrom(...spaceChars);
          }
          case '\\S': {
            return defaultChar.filter((c) => safeIndexOf(spaceChars, c) === -1);
          }
          case '\\b':
          case '\\B': {
            throw new SError(
              `Meta character ${astNode.value} not implemented yet!`,
            );
          }
          case '.': {
            const forbiddenChars = flags.dotAll
              ? terminatorChars
              : newLineAndTerminatorChars;
            return defaultChar.filter(
              (c) => safeIndexOf(forbiddenChars, c) === -1,
            );
          }
        }
      }
      if (astNode.symbol === void 0) {
        throw new SError(
          `Unexpected undefined symbol received for non-meta Char! Received: ${stringify(astNode)}`,
        );
      }
      return constant(astNode.symbol);
    }
    case 'Repetition': {
      const node = toMatchingArbitrary(astNode.expression, constraints, flags);
      switch (astNode.quantifier.kind) {
        case '*': {
          return stringOf(node, constraints);
        }
        case '+': {
          return stringOf(
            node,
            Object.assign(Object.assign({}, constraints), { minLength: 1 }),
          );
        }
        case '?': {
          return stringOf(
            node,
            Object.assign(Object.assign({}, constraints), {
              minLength: 0,
              maxLength: 1,
            }),
          );
        }
        case 'Range': {
          return stringOf(
            node,
            Object.assign(Object.assign({}, constraints), {
              minLength: astNode.quantifier.from,
              maxLength: astNode.quantifier.to,
            }),
          );
        }
        default: {
          throw raiseUnsupportedASTNode2(astNode.quantifier);
        }
      }
    }
    case 'Quantifier': {
      throw new SError(
        `Wrongly defined AST tree, Quantifier nodes not supposed to be scanned!`,
      );
    }
    case 'Alternative': {
      return tuple(
        ...safeMap(astNode.expressions, (n) =>
          toMatchingArbitrary(n, constraints, flags),
        ),
      ).map((vs) => safeJoin(vs, ''));
    }
    case 'CharacterClass':
      if (astNode.negative) {
        const childrenArbitraries = safeMap(astNode.expressions, (n) =>
          toMatchingArbitrary(n, constraints, flags),
        );
        return defaultChar.filter((c) =>
          safeEvery(
            childrenArbitraries,
            (arb) => !arb.canShrinkWithoutContext(c),
          ),
        );
      }
      return oneof(
        ...safeMap(astNode.expressions, (n) =>
          toMatchingArbitrary(n, constraints, flags),
        ),
      );
    case 'ClassRange': {
      const min = astNode.from.codePoint;
      const max = astNode.to.codePoint;
      return integer({ min, max }).map(
        (n) => safeStringFromCodePoint2(n),
        (c) => {
          if (typeof c !== 'string') throw new SError('Invalid type');
          if ([...c].length !== 1) throw new SError('Invalid length');
          return c.codePointAt(0);
        },
      );
    }
    case 'Group': {
      return toMatchingArbitrary(astNode.expression, constraints, flags);
    }
    case 'Disjunction': {
      const left =
        astNode.left !== null
          ? toMatchingArbitrary(astNode.left, constraints, flags)
          : constant('');
      const right =
        astNode.right !== null
          ? toMatchingArbitrary(astNode.right, constraints, flags)
          : constant('');
      return oneof(left, right);
    }
    case 'Assertion': {
      if (astNode.kind === '^' || astNode.kind === '$') {
        if (flags.multiline) {
          if (astNode.kind === '^') {
            return oneof(
              constant(''),
              tuple(stringOf(defaultChar), constantFrom(...newLineChars)).map(
                (t) => `${t[0]}${t[1]}`,
                (value) => {
                  if (typeof value !== 'string' || value.length === 0)
                    throw new SError('Invalid type');
                  return [
                    value.substring(0, value.length - 1),
                    value[value.length - 1],
                  ];
                },
              ),
            );
          } else {
            return oneof(
              constant(''),
              tuple(constantFrom(...newLineChars), stringOf(defaultChar)).map(
                (t) => `${t[0]}${t[1]}`,
                (value) => {
                  if (typeof value !== 'string' || value.length === 0)
                    throw new SError('Invalid type');
                  return [value[0], value.substring(1)];
                },
              ),
            );
          }
        }
        return constant('');
      }
      throw new SError(
        `Assertions of kind ${astNode.kind} not implemented yet!`,
      );
    }
    case 'Backreference': {
      throw new SError(`Backreference nodes not implemented yet!`);
    }
    default: {
      throw raiseUnsupportedASTNode2(astNode);
    }
  }
}
function stringMatching(regex, constraints = {}) {
  for (const flag of regex.flags) {
    if (
      flag !== 'd' &&
      flag !== 'g' &&
      flag !== 'm' &&
      flag !== 's' &&
      flag !== 'u'
    ) {
      throw new SError(
        `Unable to use "stringMatching" against a regex using the flag ${flag}`,
      );
    }
  }
  const sanitizedConstraints = { size: constraints.size };
  const flags = { multiline: regex.multiline, dotAll: regex.dotAll };
  const regexRootToken = addMissingDotStar(tokenizeRegex(regex));
  return toMatchingArbitrary(regexRootToken, sanitizedConstraints, flags);
}

// ../../../node_modules/fast-check/lib/esm/fast-check-default.js
var __type2 = 'module';
var __version2 = '3.17.2';
var __commitHash2 = 'a377b81e6e8362ad7324cf65b75bc5e93d12af64';

// ../../../node_modules/fast-check/lib/esm/fast-check.js
var fast_check_default = fast_check_default_exports;
export {
  Arbitrary,
  ExecutionStatus,
  PreconditionFailure,
  Random,
  Stream,
  Value,
  VerbosityLevel,
  __commitHash2 as __commitHash,
  __type2 as __type,
  __version2 as __version,
  anything,
  array,
  ascii,
  asciiString,
  assert,
  asyncDefaultReportMessage,
  asyncModelRun,
  asyncProperty,
  asyncStringify,
  asyncToStringMethod,
  base64,
  base64String,
  bigInt,
  bigInt64Array,
  bigIntN,
  bigUint,
  bigUint64Array,
  bigUintN,
  boolean,
  char,
  char16bits,
  check,
  clone,
  cloneIfNeeded,
  cloneMethod,
  commands,
  compareBooleanFunc,
  compareFunc,
  configureGlobal,
  constant,
  constantFrom,
  context,
  createDepthIdentifier,
  date,
  fast_check_default as default,
  defaultReportMessage,
  dictionary,
  domain,
  double,
  emailAddress,
  falsy,
  float,
  float32Array,
  float64Array,
  fullUnicode,
  fullUnicodeString,
  func,
  gen,
  getDepthContextFor,
  hasAsyncToStringMethod,
  hasCloneMethod,
  hasToStringMethod,
  hash,
  hexa,
  hexaString,
  infiniteStream,
  int16Array,
  int32Array,
  int8Array,
  integer,
  ipV4,
  ipV4Extended,
  ipV6,
  json,
  jsonValue,
  letrec,
  lorem,
  mapToConstant,
  maxSafeInteger,
  maxSafeNat,
  memo,
  mixedCase,
  modelRun,
  nat,
  object,
  oneof,
  option,
  pre,
  property,
  readConfigureGlobal,
  record,
  resetConfigureGlobal,
  sample,
  scheduledModelRun,
  scheduler,
  schedulerFor,
  shuffledSubarray,
  sparseArray,
  statistics,
  stream,
  string,
  string16bits,
  stringMatching,
  stringOf,
  stringify,
  subarray,
  toStringMethod,
  tuple,
  uint16Array,
  uint32Array,
  uint8Array,
  uint8ClampedArray,
  ulid,
  unicode,
  unicodeJson,
  unicodeJsonValue,
  unicodeString,
  uniqueArray,
  uuid,
  uuidV,
  webAuthority,
  webFragments,
  webPath,
  webQueryParameters,
  webSegment,
  webUrl,
};
