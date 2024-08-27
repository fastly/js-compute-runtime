# Changelog

## [3.21.1](https://github.com/fastly/js-compute-runtime/compare/v3.21.0...v3.21.1) (2024-08-27)


### Bug Fixes

* missing publish file, parallel publish build ([#912](https://github.com/fastly/js-compute-runtime/issues/912)) ([91ae54c](https://github.com/fastly/js-compute-runtime/commit/91ae54cbbd92d83fa9c1896df006b1008a5f8291))

## [3.21.0](https://github.com/fastly/js-compute-runtime/compare/v3.20.0...v3.21.0) (2024-08-27)


### Features

* ship --debug-build CLI flag as public ([#907](https://github.com/fastly/js-compute-runtime/issues/907)) ([2728141](https://github.com/fastly/js-compute-runtime/commit/27281413db682f5e2e87928937456bc1b8345dd7))
* support getSetCookie on new StarlingMonkey headers implementation ([#844](https://github.com/fastly/js-compute-runtime/issues/844)) ([c102521](https://github.com/fastly/js-compute-runtime/commit/c1025210a591fc99fae9c3b921504a6189552a74))

## 3.20.0 (2024-08-08)

### Added

* Add new CLI name of `js-compute` which matches the published package name `@fastly/js-compute` ([#869](https://github.com/fastly/js-compute-runtime/issues/869)) ([60d1d20](https://github.com/fastly/js-compute-runtime/commit/60d1d2067d846aa15a76820e666004cf56d1df99))

### Fixed

* core-cache headers case ([#889](https://github.com/fastly/js-compute-runtime/issues/889)) ([3f2db5c](https://github.com/fastly/js-compute-runtime/commit/3f2db5c466151efddf1731c6be080c2a2875a43d))
* ensure we throw an error if FastlyBody.prototype.read is called with a value which is not coercible to a finite positive integer ([#877](https://github.com/fastly/js-compute-runtime/issues/877)) ([1633e02](https://github.com/fastly/js-compute-runtime/commit/1633e025d92be3a1f8b0616685b48e27dc913841))
* perf: Use wasm-opt -O3 when making a release build ([#870](https://github.com/fastly/js-compute-runtime/issues/870)) ([dd91fa5](https://github.com/fastly/js-compute-runtime/commit/dd91fa506b74487b70dc5bec510e89de95e1c569))
* When constructing an EdgeRateLimiter, retrieve the PenaltyBox instance's name using PenaltyBox::get\_name ([#866](https://github.com/fastly/js-compute-runtime/issues/866)) ([9222f1d](https://github.com/fastly/js-compute-runtime/commit/9222f1d16a9c17b080be575affffbb83c461dd81))

### Changed

* only time the fetch event when debug logging is enabled ([#873](https://github.com/fastly/js-compute-runtime/issues/873)) ([e4ddf8a](https://github.com/fastly/js-compute-runtime/commit/e4ddf8ac3c78bea753e8d9418715d1e703e7e7bc))
* re-order the http methods so the most often requested is first and the least requested is last ([#874](https://github.com/fastly/js-compute-runtime/issues/874)) ([6af7626](https://github.com/fastly/js-compute-runtime/commit/6af7626085af62a14520f14f69a0e64a515fd5ef))
* Use MOZ\_ASSERT instead of MOZ\_RELEASE\_ASSERT as these methods are already guarded correctly where they are being called ([#876](https://github.com/fastly/js-compute-runtime/issues/876)) ([f089616](https://github.com/fastly/js-compute-runtime/commit/f089616e8febc783cc96363f5ce65fc6f1acafb1))

## 3.19.0 (2024-07-29)

### Added

* Add FetchEvent.server object which contains information about the server which received the incoming HTTP request from the client. ([#855](https://github.com/fastly/js-compute-runtime/issues/855)) ([538ed9c](https://github.com/fastly/js-compute-runtime/commit/538ed9c436105caf4bf906355fcf110752870b2b))
* use StarlingMonkey by default, --disable-starlingmonkey flag ([#861](https://github.com/fastly/js-compute-runtime/issues/861)) ([475cdf9](https://github.com/fastly/js-compute-runtime/commit/475cdf910d5690d74ff96dccd14dfefc209ea944))

### Fixed

* Correct Class name for the ClientInfo class ([#854](https://github.com/fastly/js-compute-runtime/issues/854)) ([efb5694](https://github.com/fastly/js-compute-runtime/commit/efb569475a285bdeb2dcc1346d718eb26be4fed9))
* correct spelling in CLI error message ([#849](https://github.com/fastly/js-compute-runtime/issues/849)) ([38b558c](https://github.com/fastly/js-compute-runtime/commit/38b558c3ad7c6871243823a207485ba9cc6282dd))

## 3.18.1 (2024-07-18)

### Fixed

* add type definitions of Performance APIs ([#841](https://github.com/fastly/js-compute-runtime/issues/841)) ([fd95aae](https://github.com/fastly/js-compute-runtime/commit/fd95aaecc5a264860845740a3b60d4a7aa75c578))

## 3.18.0 (2024-07-16)

### Added

* support for Response.prototype.ip and port via get\_addr\_dest\_ip & get\_addr\_dest\_port ([#817](https://github.com/fastly/js-compute-runtime/issues/817)) ([391b3d8](https://github.com/fastly/js-compute-runtime/commit/391b3d8386defe5e1b4ce3c1fb70347a9f0802ca))
* Update to SpiderMonkey v127.0.2 ([#826](https://github.com/fastly/js-compute-runtime/issues/826)) ([5341f67](https://github.com/fastly/js-compute-runtime/commit/5341f674fe0da5ac5057d3415f59ac807d2f96f7))

## 3.17.3 (2024-07-16)

### Fixed

* Remove accidentally commited debug messages which write to stderr ([#838](https://github.com/fastly/js-compute-runtime/issues/838)) ([040ea8b](https://github.com/fastly/js-compute-runtime/commit/040ea8be352884c3536ac7e786663b4596617e6e))

## 3.17.2 (2024-07-13)

### Fixed

* add documentation for the sdkVersion property ([29361ad](https://github.com/fastly/js-compute-runtime/commit/29361ad65965a7a36cb2ced434af7898844dcab7))
* correct the documentation for the fastly:logger module ([#834](https://github.com/fastly/js-compute-runtime/issues/834)) ([2790cb9](https://github.com/fastly/js-compute-runtime/commit/2790cb93f5f298b021f46b9030d0e6e795003a51))

## 3.17.1 (2024-07-11)

### Fixed

* documentation site build ([#831](https://github.com/fastly/js-compute-runtime/issues/831)) ([110f1ff](https://github.com/fastly/js-compute-runtime/commit/110f1ffb4e1b80772b9f9541a58456bedfd4ddec))

## 3.17.0 (2024-07-11)

### Added

* Include in the wasm metadata whether we are using StarlingMonkey and/or PBL ([#828](https://github.com/fastly/js-compute-runtime/issues/828)) ([00b971b](https://github.com/fastly/js-compute-runtime/commit/00b971b857e9c35a08e6fc5c0a84fb3c3bc7984e))

### Fixed

* keep sdkVersion property always up-to-date with the correct version of the SDK ([#829](https://github.com/fastly/js-compute-runtime/issues/829)) ([ae42634](https://github.com/fastly/js-compute-runtime/commit/ae4263418a52dfb62fe240584314948072ff30e7))

## 3.16.2 (2024-07-10)

### Fixed

* use same rust version that StarlingMonkey uses so that we can publish ([#823](https://github.com/fastly/js-compute-runtime/issues/823)) ([f0d9ab0](https://github.com/fastly/js-compute-runtime/commit/f0d9ab07ed5a5470322a3b457cc91e308e3e289f))

## 3.16.1 (2024-07-09)

### Fixed

* CLI to allow commands/args in spawnSync() to contain whitespace ([#821](https://github.com/fastly/js-compute-runtime/issues/821)) ([68d77fb](https://github.com/fastly/js-compute-runtime/commit/68d77fbff9f695f53b30ee63d53b41fd8db87424))
* debug build & tests ([#818](https://github.com/fastly/js-compute-runtime/issues/818)) ([3d9a8da](https://github.com/fastly/js-compute-runtime/commit/3d9a8da2898fffa6b43d005eba3342df4ff67036))

## 3.16.0 (2024-06-21)

### Added

* add out-of-memory callback with stderr log ([#805](https://github.com/fastly/js-compute-runtime/issues/805)) ([a1bd16c](https://github.com/fastly/js-compute-runtime/commit/a1bd16c06924ea1748846d2b8159b9b2939ae61d))
* Allow early logger initialization ([#804](https://github.com/fastly/js-compute-runtime/issues/804)) ([514d014](https://github.com/fastly/js-compute-runtime/commit/514d0145ba88de3a7114e957457a7f9570e17019))

### Fixed

* Fix string formatting for limit-exceeded errors ([#802](https://github.com/fastly/js-compute-runtime/issues/802)) ([56f5214](https://github.com/fastly/js-compute-runtime/commit/56f5214ad9f431845f6b06cb92e0b98169ceffbe))
* Fix uses of cabi\_realloc that were discarding their results ([#811](https://github.com/fastly/js-compute-runtime/issues/811)) ([4e16641](https://github.com/fastly/js-compute-runtime/commit/4e16641ef4e159c4a11b500ac861b8fa8d9ff5d3))

## 3.15.0 (2024-06-03)

### Added

* dynamic backends clientCertificate with SecretStore fromBytes, rawbytes ([#796](https://github.com/fastly/js-compute-runtime/issues/796)) ([7d2b7b7](https://github.com/fastly/js-compute-runtime/commit/7d2b7b781ed808d9bcf1fe9584aa31f788b980a2))
* support default timeout configurations for dynamic backends ([#792](https://github.com/fastly/js-compute-runtime/issues/792)) ([4dfa8d7](https://github.com/fastly/js-compute-runtime/commit/4dfa8d76aeb4364ed5267ed22de0b9a891781589))

## 3.14.2 (2024-05-21)

### Fixed

* changelog formatting ([1473a87](https://github.com/fastly/js-compute-runtime/commit/1473a87092c6c02e37378897eb0a4042da2f90c8))
* revert changelog heading changes ([#784](https://github.com/fastly/js-compute-runtime/issues/784)) ([59195b6](https://github.com/fastly/js-compute-runtime/commit/59195b6aec1353adc6f6ad8322f7414d90adc518))

## 3.14.1 (2024-05-17)

### Fixed

* fix documentation build ([#781](https://github.com/fastly/js-compute-runtime/issues/781)) ([864864e](https://github.com/fastly/js-compute-runtime/commit/864864e05ca3cf286f049d2c692401e708008052))

## 3.14.0 (2024-05-16)

### Added

* fastly.sdkVersion implementation ([#776](https://github.com/fastly/js-compute-runtime/issues/776)) ([3eb5a8f](https://github.com/fastly/js-compute-runtime/commit/3eb5a8ff9aaad279dc17deee1c2e8760fea28a49))

### Fixed

* support cacheKey in Request init ([#770](https://github.com/fastly/js-compute-runtime/issues/770)) ([b64b22e](https://github.com/fastly/js-compute-runtime/commit/b64b22e988d8e3ca20c42c13f6cb89be871a5d61))

## 3.13.1 (2024-04-12)

### Fixed

* remove debugging message which got commited ([4219a0a](https://github.com/fastly/js-compute-runtime/commit/4219a0ac87d68d9a9fc57aaea43994a867f5dd0e))

## 3.13.0 (2024-04-11)

### Added

* Add KVStore.prototype.delete method ([578d858](https://github.com/fastly/js-compute-runtime/commit/578d858b6678c27116ead213f58d2f4fe80f1355))

### Changed

* Update to SpiderMonkey 124.0.2 ([e32632e](https://github.com/fastly/js-compute-runtime/commit/e32632e16ba822770dd9b0637185f7266a7952e2))
  This release includes:
  \- An optimization for functions that only use `arguments.length` to avoid allocating the `arguments` object.
  \- An optimization for `Object.HasOwn` which for small numbers of atoms just unrolls the loop.

### Fixed

* Correct type definition for the global BackendConfiguration type - there is no `checkCertificate` field ([62fd0ea](https://github.com/fastly/js-compute-runtime/commit/62fd0ea36e6aefd4a3cb281a09716a901f111485))
* Improve our console.log output for functions ([9a97fc1](https://github.com/fastly/js-compute-runtime/commit/9a97fc1352926ecad8377d72eca1e18e28aa2173))
* Refactor our async task implementation to be a generic AsyncTask class instead of separate implementations for each async operation ([68dfec7](https://github.com/fastly/js-compute-runtime/commit/68dfec75a0c9c583dc4be39a17cbbf9b70ff8b40))

## 3.12.1 (2024-04-05)

### Changed

* declare support for npm 10 ([#747](https://github.com/fastly/js-compute-runtime/issues/747)) ([1365ee9](https://github.com/fastly/js-compute-runtime/commit/1365ee9b1aa4e830677c840ea43df55bbf19d660))

## 3.12.0 (2024-03-28)

### Changed

* update to SpiderMonkey 123.0.1 ([#744](https://github.com/fastly/js-compute-runtime/issues/744)) ([32bf617](https://github.com/fastly/js-compute-runtime/commit/32bf61707f1133d4a2656913d726d66523398fb1))
  This update brings with it the below changes:
  * Performance improvements for `JSON.stringify()`
  * An optimisation for `Object.keys()` to take advantage of cached for-in iterators if available.
  * Optimisations for `Object.assign()`
  * RegExp `v` flag support
  * Improved JSON parsing to help avoid garbage collection time when parsing very large files
  * The `String.prototype.isWellFormed()` and `String.prototype.toWellFormed()` methods respectively can be used to check if a string contains well-formed Unicode text (i.e. contains no lone surrogates) and sanitise an ill-formed string to well-formed Unicode text.
  * The `Object.groupBy()` and `Map.groupBy()` static methods for grouping the elements of an iterable are now supported
  * `Date.parse()` now accepts several additional date formats:
    * Year > 9999 for `YYYY-MMM-DD` format (e.g. `19999-Jan-01`)
    * `MMM-DD-YYYY` (e.g. `Jan-01-1970`)
    * Milliseconds for non-ISO date formats (e.g. `Jan 1 1970 10:00:00.050`)
    * Day of week at the beginning of formats which were being rejected, such as:
      * `Wed, 1970-01-01`
      * `Wed, 1970-Jan-01`
    * The day of week does not need to be correct, or a day of week at all; for example, `foo 1970-01-01` works.
    * Numeric dashed dates which do not meet the formal ISO standard are now accepted, including:
      * `"01-12-1999"` (month first)
      * `"1999-1-5"` (single-digit month or day)
      * `"10000-01-12"` (year > 9999)
      * `"99-01-05"` or `"01-05-99"` (2-digit year, year must be >31 if it comes first)
      * `"1999-01-05 10:00:00"` (space between date and time).
        These dates will be parsed with behavior typical of other non-ISO dates, such as local time zone and month rollover (April 31 rolls over to May 1 since April 31 doesn’t exist).
    * Requirements for characters directly following numbers have been loosened to accept new formats, including:
      * `"DDMonYYYY"`
      * `"Mon.DD.YYYY"`
      * `"DD.Mon.YYYY"`
      * `"YYYY.MM.DD"`
      * `"Mon DD YYYY hh:mmXm"` (`am`/`pm` directly following time)
    * Timezone `'Z'` is now accepted for non-ISO formats (e.g. `Jan 1 1970 10:00Z`)
  * Other `Date.parse()` fixes:
    * `YYYY-M-DD` and `YYYY-MM-D` are no longer assumed GMT as an ISO date `YYYY-MM-DD` would be.
    * Milliseconds for all formats are truncated after 3 digits, rather than being rounded.
  * The `Promise.withResolvers()` static method is now supported. This exposes the `resolve` and `reject` callback functions in the same scope as the returned `Promise`, allowing code that resolves or rejects the promise to be defined after its construction.
  * The `ArrayBuffer.prototype.transfer()` and `ArrayBuffer.prototype.transferToFixedLength()` methods can now be used to transfer ownership of memory from one ArrayBuffer to another. After transfer, the original buffer is detached from its original memory and hence unusable; the state can be checked using `ArrayBuffer.prototype.detached`.

## 3.11.0 (2024-03-14)

### Added

* add new flag --experimental-enable-top-level-await ([#742](https://github.com/fastly/js-compute-runtime/issues/742)) ([437a20d](https://github.com/fastly/js-compute-runtime/commit/437a20d5f970c00d382673dbf223149b5b20ed37))

### Fixed

* correct type definition of Headers.prototype.values() to indicate it returns an IterableIterator\<string> ([#740](https://github.com/fastly/js-compute-runtime/issues/740)) ([8959e79](https://github.com/fastly/js-compute-runtime/commit/8959e79a9a7856b0ecc74b33264042c54ac8f867))

## 3.10.0 (2024-03-09)

### Added

* add fastly:device module which allows applications to detect a device based on a user-agent ([#738](https://github.com/fastly/js-compute-runtime/issues/738)) ([5274fd5](https://github.com/fastly/js-compute-runtime/commit/5274fd5280d80b276e6f13d4acbdefc435af6c57))

### Fixed

* correct title for the CoreCache.transactionLookup documentation page ([9892d90](https://github.com/fastly/js-compute-runtime/commit/9892d9074d9a1bd25b9b5db28c12a940f2aac028))

## 3.9.1 (2024-03-04)

### Fixed

* ensure we associate correct memory for the user\_metadata attached to a cache item ([#734](https://github.com/fastly/js-compute-runtime/issues/734)) ([550c4f5](https://github.com/fastly/js-compute-runtime/commit/550c4f5502e710f0b7cf11d0132270bcc91e7235))

## 3.9.0 (2024-03-02)

### Added

* Add a EdgeRateLimiter JavaScript Class which enables edge-rate-limiting by utilising a RateCounter and a PenaltyBox instance ([#732](https://github.com/fastly/js-compute-runtime/issues/732)) ([4e81fc7](https://github.com/fastly/js-compute-runtime/commit/4e81fc7dbec33a5a90743e389642e0ced5294ff1))
* Add a PenaltyBox JavaScript Class which can be used standalone for adding and checking if an entry is in the penalty-box ([#731](https://github.com/fastly/js-compute-runtime/issues/731)) ([bfe1e15](https://github.com/fastly/js-compute-runtime/commit/bfe1e15460cb2aa0da3cfa356fbf23d38f5af5ba))
* Add a RateCounter JavaScript Class which can be used standalone for counting and rate calculations ([#730](https://github.com/fastly/js-compute-runtime/issues/730)) ([0f6036f](https://github.com/fastly/js-compute-runtime/commit/0f6036f02f497345df01dbce731eb502fd406d27))
* implement and expose JavaScript classes for Fastly's Compute Core Cache feature set ([#566](https://github.com/fastly/js-compute-runtime/issues/566)) ([94f4038](https://github.com/fastly/js-compute-runtime/commit/94f4038df7ca2bfd8beef964865eb7f900b1bc04))

### Fixed

* disable the portable-baseline-tier for async functions for now ([#733](https://github.com/fastly/js-compute-runtime/issues/733)) ([4928243](https://github.com/fastly/js-compute-runtime/commit/4928243a380adfb6073a909e41ab7eb4c0d569b4))

## 3.8.3 (2024-02-21)

### Fixed

* do not use colon character in types for windows support ([#726](https://github.com/fastly/js-compute-runtime/issues/726)) ([25bf1a2](https://github.com/fastly/js-compute-runtime/commit/25bf1a2bb40528bf02e0773e6bc624560a12869a))

## 3.8.2 (2024-01-25)

### Fixed

* ensure we honor first-byte-timeout and between-bytes-timeout for dynamically registered backends ([#719](https://github.com/fastly/js-compute-runtime/issues/719)) ([2851507](https://github.com/fastly/js-compute-runtime/commit/2851507f9ca00a3f272a13c174a2906163f95c40))
* If request does not have a static backend defined, return `undefined` for the Request.prototype.backend getter ([#722](https://github.com/fastly/js-compute-runtime/issues/722)) ([251c037](https://github.com/fastly/js-compute-runtime/commit/251c037f424ace09e87ec0a47d7579d7b90626a1))

## 3.8.1 (2024-01-17)

### Fixed

* parse latin-1 encoded field values correctly ([#715](https://github.com/fastly/js-compute-runtime/issues/715)) ([9ebb524](https://github.com/fastly/js-compute-runtime/commit/9ebb524d4eef97ba71ae19ee1c2b1e61f3fd391c))

## 3.8.0 (2024-01-11)

### Added

* Add `manualFramingHeaders` on `RequestInit` and `ResponseInit`, and add `Request.prototype.setManualFramingHeaders` and `Response.prototype.setManualFramingHeaders` ([#705](https://github.com/fastly/js-compute-runtime/pull/705))
* Add `Request.prototype.backend` getter to return the name of the backend assigned to the request ([9c750e5](https://github.com/fastly/js-compute-runtime/commit/9c750e5697bb02676762225e4fdc7589d23e13d9))
* Allow URL as input on fetch() on TypeScript typings for compat with Node.js ([#707](https://github.com/fastly/js-compute-runtime/issues/707)) ([4f39943](https://github.com/fastly/js-compute-runtime/commit/4f399434c0959e902df03262dfceefdc16592afe))

## 3.7.3 (2023-11-02)

### Fixed

* Make the underlying KVStore.prototype.get implementation be async ([a6a5035](https://github.com/fastly/js-compute-runtime/commit/a6a5035fc932be0e47c7c737bd9060d27c18ab05))

## 3.7.2 (2023-10-25)

### Fixed

* Make Response.redirect headers be immutable ([3527eaf](https://github.com/fastly/js-compute-runtime/commit/3527eaf62266a3cf7ea8ea4020bb5980bb7fa615))
* Return correct error type (TypeError or RangeError instead of Error) in Request and Response methods ([4ea7de7](https://github.com/fastly/js-compute-runtime/commit/4ea7de71301d841fdc99f45a3251f85c61710fd6))

## 3.7.1 (2023-10-24)

### Added

* Add type defintions for the recently added Backend methods ([#698](https://github.com/fastly/js-compute-runtime/issues/698)) ([24f1ba7](https://github.com/fastly/js-compute-runtime/commit/24f1ba70e68f35205104eaf583c29d4af9b5039c))

## 3.7.0 (2023-10-14)

### Added

This release of `@fastly/js-compute` includes 4 new methods to the Backend class, which enable the Fastly Service to retrieve information about any backend, this is particularly useful for checking if the backend is “healthy”. ([#523](https://github.com/fastly/js-compute-runtime/issues/523)) ([08f816a](https://github.com/fastly/js-compute-runtime/commit/08f816ae4465316a2316467338e0d33ffbd20e7a))

The new methods are:

* Backend.exists(name) - Check whether a backend with the given name exists for the Fastly Service.
* Backend.fromName(name) - Check whether a backend with the given name exists for the Fastly Service and if it does, then returns an instance of Backend for the given name.
* Backend.health(name) - Returns the health of the backend with the given name.
* Backend.prototype.toName() - Return the name for the Backend instance.

### Fixed

* bring back support for build-time env vars ([#691](https://github.com/fastly/js-compute-runtime/issues/691)) ([c044ac4](https://github.com/fastly/js-compute-runtime/commit/c044ac4bbbd5629bfc879b7593a0bfa9c5e3cfcb))
* raise an error during wizening for async functions given to addEventListener ([#689](https://github.com/fastly/js-compute-runtime/issues/689)) ([e6747a2](https://github.com/fastly/js-compute-runtime/commit/e6747a28d70d71bc71da77c9b6e44848b95ea387))

## 3.6.2 (2023-10-05)

### Fixed

* improve fetch error messages ([58ddb20](https://github.com/fastly/js-compute-runtime/commit/58ddb2012f9bff5ad59fb6420bfa31051109a108))

## 3.6.1 (2023-09-27)

### Fixed

* ensure we throw an error when trying to base64 decode \_ via `atob` ([1b2b2f9](https://github.com/fastly/js-compute-runtime/commit/1b2b2f9d807780cf03964a30801644c8bc3b698b))

## 3.6.0 (2023-09-22)

### Added

* add support for ECDSA keys to be used with SubtleCrypto.prototype.sign and SubtleCrypto.prototype.verify ([#667](https://github.com/fastly/js-compute-runtime/issues/667)) ([51bb170](https://github.com/fastly/js-compute-runtime/commit/51bb1703fb81fddac24b152fc7b1e0f32f976de5))

## 3.5.0 (2023-09-19)

### Added

* implement the "fastly" condition ([#660](https://github.com/fastly/js-compute-runtime/issues/660)) ([db7db46](https://github.com/fastly/js-compute-runtime/commit/db7db46266b022afffd351421f56d5d5f7b98a53))

JavaScript dependencies can now target our JavaScript runtime for Fastly Compute explicitly via the recently added “fastly” WinterCG Runtime Key
This release updates our internal bundling system to include this functionality
Note: If you are using a custom bundling system for your JavaScript projects and want this functionality, you will need to update/configure your bundling system

## 3.4.0 (2023-09-13)

### Added

* add ability to import ECDSA JWK keys via crypto.subtle.importKey ([#639](https://github.com/fastly/js-compute-runtime/issues/639)) ([c16b001](https://github.com/fastly/js-compute-runtime/commit/c16b001bddc2dc122c26837023ab9c53664adf8a))

## 3.3.5 (2023-09-11)

### Changed

* use new host\_api implementation for transactional lookups and inserts ([#651](https://github.com/fastly/js-compute-runtime/issues/651)) ([8c29246](https://github.com/fastly/js-compute-runtime/commit/8c292466e1fef61673ad3d46b747a6c54ed71ddb))

## 3.3.4 (2023-09-07)

### Fixed

* Fix SimpleCache API by reverting host\_api implementation of the underlying cache apis ([4340375](https://github.com/fastly/js-compute-runtime/commit/4340375409be382c2faec657615c187d99d1fc7e))

## 3.3.3 (2023-09-05)

### Fixed

* remove unused lines of code from docs for SimpleCache/get.mdx ([51fd4af](https://github.com/fastly/js-compute-runtime/commit/51fd4af94f72dd9ae112a967ef05bc67d02f202c))
* update to latest version of gecko-dev which fixes a bug with the default ecma262 sorting algorithm ([#643](https://github.com/fastly/js-compute-runtime/issues/643)) ([64323e3](https://github.com/fastly/js-compute-runtime/commit/64323e344bc61d4cc52e34710ab7ae208d56e321))

## 3.3.2 (2023-08-31)

### Added

* Add documentation for Request.prototype.clone() ([9d12321](https://github.com/fastly/js-compute-runtime/commit/9d12321bf3da019f6383389098625ca1314d9fb8))

## 3.3.1 (2023-08-24)

### Changed

* update to spidermonkey which includes async resume support when using pbl ([#634](https://github.com/fastly/js-compute-runtime/issues/634)) ([1dea60f](https://github.com/fastly/js-compute-runtime/commit/1dea60f79fc07828785b12fd8a5bf13b3602f88b))

## 3.3.0 (2023-08-22)

### Added

* Add option to enable PBL. ([#628](https://github.com/fastly/js-compute-runtime/issues/628)) ([6ecda6e](https://github.com/fastly/js-compute-runtime/commit/6ecda6e89971f178f623e242d8dd6a8fd25ab63f))

## 3.2.1 (2023-08-16)

### Fixed

* Add documentation and type definitions for the new event.client.\* fields ([#625](https://github.com/fastly/js-compute-runtime/issues/625)) ([a6f557b](https://github.com/fastly/js-compute-runtime/commit/a6f557ba1b03035869e4c4fb3d9679fb3e28fd1f))

## 3.2.0 (2023-08-10)

### Added

* add ability to automatically decompress gzip responses returned from `fetch` ([#497](https://github.com/fastly/js-compute-runtime/issues/497)) ([e08d060](https://github.com/fastly/js-compute-runtime/commit/e08d060535160b8c934f60f37d8f4a71f412f0c4))

### Changed

* use spidermonkey version 115 ([4a4716d](https://github.com/fastly/js-compute-runtime/commit/4a4716d99fa1e263eae9cf5d7fcc96999519c7fe))
* reduce memory usage by caching client getters when they are first called ([87ee0cb](https://github.com/fastly/js-compute-runtime/commit/87ee0cb54edab82c0b2f6b986458d2552a8dbcba))
* update to latest url crate which passes some more wpt url tests ([f0a42fd](https://github.com/fastly/js-compute-runtime/commit/f0a42fd07821190e1ebf66c95762cb8e26b69e8b))

## 3.1.1 (2023-07-14)

### Fixed

* Request.prototype.clone - Do not create a body on the new request if the request instance being cloned does not contain a body ([5debe80](https://github.com/fastly/js-compute-runtime/commit/5debe806a4a40e0d3b07bdd6b71489aa7d739cff))

## 3.1.0 (2023-07-12)

### Added

* Add ability to disable connection-pooling behavior for Dynamic Backends ([#574](https://github.com/fastly/js-compute-runtime/issues/574)) ([718bea8](https://github.com/fastly/js-compute-runtime/commit/718bea8e2b950bc00c43187e479a7a7de41eaa70))

### Changed

* Deprecate SimpleCache.set and recommend SimpleCache.getOrSet as the alternative ([bff1bf5](https://github.com/fastly/js-compute-runtime/commit/bff1bf587c7de6012c617745b059dea24e6299ad))

## 3.0.0 (2023-07-08)

### Changed

*⚠ BREAKING CHANGE*

* Rename SimpleCache.delete to SimpleCache.purge and require purge options to be supplied as the second parameter

We are renaming because "purge" is already a well-known and documented concept for removing content from Fastly's cache.

The new addition of a second argument allows the caller to decide what scope to purge the content from, currently they can choose to purge from all of Fastly ("global") or from the POP that contains the currently executing instance ("pop"). We do not provide a default option right now, in the future we may provide a default option, if we discover a common pattern is being used.

Here is an example of migrating an application using SimpleCache.delete to SimpleCache.purge with the same behaviour:

```diff
/// <reference types="@fastly/js-compute" />

import { SimpleCache } from 'fastly:cache';

addEventListener('fetch', event => event.respondWith(app(event)));

async function app(event) {
  const url = new URL(event.request.url);
  const path = url.pathname;
  if (url.searchParams.has('delete')) {
-    SimpleCache.delete(path);
+    SimpleCache.purge(path, { scope: "global" });
    return new Response(page, { status: 204 });
  }

  let page = SimpleCache.getOrSet(path, async () => {
    return {
      value: await render(path),
      // Store the page in the cache for 1 minute.
      ttl: 60
    }
  });
  return new Response(page, {
    headers: {
      'content-type': 'text/plain;charset=UTF-8'
    }
  });
}

async function render(path) {
  // expensive/slow function which constructs and returns the contents for a given path
  await new Promise(resolve => setTimeout(resolve, 10_000));
  return path;
}


```

### Added

* add event.client.tlsCipherOpensslName ([49b0c99](https://github.com/fastly/js-compute-runtime/commit/49b0c99523147998304dc559b836bcc79008e8b0))
* add event.client.tlsClientCertificate ([cf93b62](https://github.com/fastly/js-compute-runtime/commit/cf93b6226b01ca653688571ed0db27e0f6d39bc2))
* add event.client.tlsClientHello ([3d87cb2](https://github.com/fastly/js-compute-runtime/commit/3d87cb28a670735441a0d8c6d16291867c8f2244))
* add event.client.tlsJA3MD5 ([2ecf4af](https://github.com/fastly/js-compute-runtime/commit/2ecf4afcc503e60a1aa972c88d47149b22dbf70c))
* add event.client.tlsProtocol ([4c91142](https://github.com/fastly/js-compute-runtime/commit/4c9114213343d4dea2a1ac2955980e19540a4463))
* Rename SimpleCache.delete to SimpleCache.purge and require purge options to be supplied as the second parameter ([20113c1](https://github.com/fastly/js-compute-runtime/commit/20113c1df6ad57a98c5b8c27b06d67117d2029ef))

## 2.5.0 (2023-07-05)

### Added

* add DOMException class ([58b8086](https://github.com/fastly/js-compute-runtime/commit/58b8086edce2d93928743aec462843df369d458b))
* Add support for HMAC within SubtleCrypto implementation ([96ac02d](https://github.com/fastly/js-compute-runtime/commit/96ac02d1e62b6d34f73a18ba3be30266a4b0f27e))

### Changed

* update types for SubtleCrypto to show we support a subset of importKey/sign/verify ([#568](https://github.com/fastly/js-compute-runtime/issues/568)) ([329b733](https://github.com/fastly/js-compute-runtime/commit/329b733e77d4bcb2b341eb1e1b36a5d6a7c999cc))

## 2.4.0 (2023-06-22)

### Changed

* Update to SpiderMonkey version 114.0.1 ([#563](https://github.com/fastly/js-compute-runtime/issues/563)) ([03e2254](https://github.com/fastly/js-compute-runtime/commit/03e22542cd439990ad530eb1958a12ce8ab85120))

## 2.3.0 (2023-06-12)

### Added

* implement web performance api ([ddfe11e](https://github.com/fastly/js-compute-runtime/commit/ddfe11ec92a48495edd920e48ffad3d20e69c159))

## 2.2.1 (2023-06-09)

### Fixed

* only apply our pipeTo/pipeThrough optimisations to TransformStreams who have no transformers (IdentityStreams). ([#556](https://github.com/fastly/js-compute-runtime/issues/556)) ([a88616c](https://github.com/fastly/js-compute-runtime/commit/a88616c7a5aa4e13d3f1eeef259ba7480416f3f0))

## 2.2.0 (2023-06-08)

### Added

* Implement SimpleCache.getOrSet method ([a1f4517](https://github.com/fastly/js-compute-runtime/commit/a1f4517e5e377354254ee2a635f97a562c87e13c))

## 2.1.0 (2023-06-02)

### Added

* Implement a SimpleCache Class ([#548](https://github.com/fastly/js-compute-runtime/issues/548)) ([865382d](https://github.com/fastly/js-compute-runtime/commit/865382df3a74832abce1f0d40e3627d8339b4aeb))

## 2.0.2 (2023-06-01)

### Fixed

* add fastly:secret-store types ([3805238](https://github.com/fastly/js-compute-runtime/commit/38052381331999d00b6f2cc878ae41c51068ff94))

* update to the latest wizer which brings support for prebuilt linux s390x and aarch64 wizer binaries ([69484c2](https://github.com/fastly/js-compute-runtime/commit/69484c25465a2674513f83f8c9674e1857e01cb9))

## 2.0.1 (2023-05-24)

### Fixed

* When using implicit backends with https protocol, use the hostname for the sni hostname value to match `fetch` behaviour in browsers and other runtimes ([84fb6a2](https://github.com/fastly/js-compute-runtime/commit/84fb6a2fa57408fb13e9319da91d6de3533f1e3c))

## 2.0.0 (2023-05-15)

### Changed

* Object Store renamed to KV Store ([#476](https://github.com/fastly/js-compute-runtime/issues/476))

We have renamed the `ObjectStore` class to `KVStore`, and the module name from `fastly:object-store` to `fastly:kv-store`.

You will need to update your code to use the new class name and module name.

Below is the change that would need to be made for the imported module name:

```diff
- import { ObjectStore } from 'fastly:object-store';
+ import { KVStore } from 'fastly:kv-store';
```

And this is the change that would need to be made for constructing an instance of the class:

```diff
- const store = new ObjectStore('my-store');
+ const store = new KVStore('my-store');
```

Here is a full example of migrating an application from ObjectStore to KVStore:

```diff
/// <reference types="@fastly/js-compute" />

- import { ObjectStore } from 'fastly:object-store';
+ import { KVStore } from 'fastly:kv-store';

async function app(event) {
-   const files = new ObjectStore('files');
+   const files = new KVStore('files');

  await files.put('hello', 'world')

  const entry = await files.get('hello')

  return new Response(await entry.text())
}

addEventListener("fetch", (event) => event.respondWith(app(event)))
```

## 1.13.0 (2023-05-11)

### Added

* Implement all the web console methods ([#522](https://github.com/fastly/js-compute-runtime/issues/522)) ([a12a1d3](https://github.com/fastly/js-compute-runtime/commit/a12a1d35f0b68c549d802ea2df87eb5bd5a1cd31))

## 1.12.0 (2023-05-11)

### Added

* Implement Fanout for JS SDK ([5198884](https://github.com/fastly/js-compute-runtime/commit/5198884d35c616785399d1702efa2454f9303421))

## 1.11.2 (2023-04-27)

### Fixed

* Add TypeScript definitions for Response.redirect() and Response.json() ([#512](https://github.com/fastly/js-compute-runtime/issues/512)) ([ebe429f](https://github.com/fastly/js-compute-runtime/commit/ebe429fc895f8da837e47393ebc35fe6dec5159a))

## 1.11.1 (2023-04-26)

### Fixed

* **TextDecoder:** add (nearly) full support for TextDecoder and TextEncoder ([#501](https://github.com/fastly/js-compute-runtime/issues/501)) ([a4c312e](https://github.com/fastly/js-compute-runtime/commit/a4c312e62284147da73d82323ac095670d41cdf3))

## 1.11.0 (2023-04-25)

### Added

* implement Response.json static method ([#499](https://github.com/fastly/js-compute-runtime/issues/499)) ([780067d](https://github.com/fastly/js-compute-runtime/commit/780067d429dbd90bd529f42169c2c1af6c139bb7))

## 1.10.1 (2023-04-24)

### Fixed

* Fix for `ReferenceError: pattern is not defined` ([#506](https://github.com/fastly/js-compute-runtime/issues/506)) ([107c9be](https://github.com/fastly/js-compute-runtime/commit/107c9be4c0b0c41c4d630ba556a10b697a1508f4))

## 1.10.0 (2023-04-21)

### Added

* Add MD5 support into crypto.subtle.digest ([9c8efab](https://github.com/fastly/js-compute-runtime/commit/9c8efabc89c20e5e20f8ef429b555c1d85fe0db1))
* implement Response.redirect static method and Response.prototype.redirected getter ([1623d74](https://github.com/fastly/js-compute-runtime/commit/1623d740405dcaaa5a8c946981c6840ab611c36a))

## 1.9.0 (2023-04-15)

### Added

* Implement subset of crypto.subtle.importKey which can import a JSONWebKey using RSASSA-PKCS1-v1\_5 ([b66bf50](https://github.com/fastly/js-compute-runtime/commit/b66bf506a9bf25cf251f7c58a34ba2e1a0e68c5d))
* Implement subset of crypto.subtle.sign which can sign data with a JSONWebKey using RSASSA-PKCS1-v1\_5 ([800fb66](https://github.com/fastly/js-compute-runtime/commit/800fb666aca957a62d79dbf4fefa35aad8212de5))
* Implement subset of crypto.subtle.verify which can verify a signature with a JSONWebKey using RSASSA-PKCS1-v1\_5 ([077adfd](https://github.com/fastly/js-compute-runtime/commit/077adfd16f870564e945d14e4caf0c21762c64f1))

### Fixed

* free `buf` if an error has occured ([bfa84cc](https://github.com/fastly/js-compute-runtime/commit/bfa84cc4fa22c1d2ea860cad597dd25878a24e20))

## 1.8.1 (2023-04-12)

### Fixed

* Mark NodeJS 19 and 20 as supported ([#492](https://github.com/fastly/js-compute-runtime/issues/492)) ([27b3428](https://github.com/fastly/js-compute-runtime/commit/27b34289988b6ef55ea3ce703b878dbd1da68d7a))

## 1.8.0 (2023-04-12)

### Added

* Add high-resolution timing function "fastly.now()" behind feature flag "--enable-experimental-high-resolution-time-methods" ([f090838](https://github.com/fastly/js-compute-runtime/commit/f0908384d48d0bc2e5c29083e8a20bed041d47ed))

### Changed

* replace tree-sitter with acorn + magic string ([08a0695](https://github.com/fastly/js-compute-runtime/commit/08a0695a00088fe51c289ea783a771b4f3b993f8))

## 1.7.1 (2023-04-11)

### Fixed

* Lower the supported NodeJS version from 18 or greater to only 18 ([5cc1cd6](https://github.com/fastly/js-compute-runtime/commit/5cc1cd6e5bfb8926944457e81c045682b0a37e4c))
* When converting a URL to a string, do not add a `?` if there are no query string parameters ([73cdc27](https://github.com/fastly/js-compute-runtime/commit/73cdc279fa8c038a012c050000960577dda21280))

## 1.7.0 (2023-04-11)

### Added

* BYOB streams, basic usage, *pending WPT* ([ab97e75](https://github.com/fastly/js-compute-runtime/commit/ab97e75e3b595911432327b35fcf4716675a0dd0))
* Implement subset of crypto.subtle.importKey which can import a JSONWebKey using RSASSA-PKCS1-v1\_5 ([#472](https://github.com/fastly/js-compute-runtime/issues/472)) ([110e7f4](https://github.com/fastly/js-compute-runtime/commit/110e7f42c1a86c4b4b722ea4b6780bb68f7f4523))

## 1.6.0 (2023-03-28)

### Added

* Implement JS CryptoKey Interface ([adb31f7](https://github.com/fastly/js-compute-runtime/commit/adb31f7197acf869af1852c0656847e4ab240089))

## 1.5.2 (2023-03-23)

### Fixed

* Add documentation for FetchEvent, FetchEvent.prototype.respondWith, and FetchEvent.prototype.waitUntil ([78e6d92](https://github.com/fastly/js-compute-runtime/commit/78e6d925d1ec6cdedd4f2678997e333aba9ebae6))
* fix typo in geolocation example ([f53a06e](https://github.com/fastly/js-compute-runtime/commit/f53a06ecb46c5ad1f91806c1c13ce6215a254192))

## 1.5.1 (2023-03-10)

### Fixed

* handle fallthrough of regex parser bugs ([#447](https://github.com/fastly/js-compute-runtime/issues/447)) ([8f38980](https://github.com/fastly/js-compute-runtime/commit/8f389805d6a88e476f0281df974cb971d7e78896))

## 1.5.0 (2023-03-10)

### Added

* support unicode patterns via precompilation ([87a0dce](https://github.com/fastly/js-compute-runtime/commit/87a0dce62115cfd6d665f1d2aa617cf53a8b6b01))

## 1.4.2 (2023-03-09)

### Fixed

* console logging support improvements ([#434](https://github.com/fastly/js-compute-runtime/issues/434)) ([7a74d76](https://github.com/fastly/js-compute-runtime/commit/7a74d76ed1d03c1c588caf664f471eab226c10a6))

## 1.4.1 (2023-03-01)

### Changed

* modular builtin separation ([#426](https://github.com/fastly/js-compute-runtime/issues/426)) ([c5933ea](https://github.com/fastly/js-compute-runtime/commit/c5933ea2599c0f0952d7314ecbbe93faa8ec9acb))

## 1.4.0 (2023-02-27)

### Added

* implement fastly:secret-store package ([cde22e3](https://github.com/fastly/js-compute-runtime/commit/cde22e3fa232b50e96222301ba40dda5b424bb60))

### Changed

* Bump to spidermonkey 110, and viceroy 0.3.5 ([#420](https://github.com/fastly/js-compute-runtime/issues/420)) ([e17cdfd](https://github.com/fastly/js-compute-runtime/commit/e17cdfda1878fe23a7f331fb20d33c52d580003b))

## 1.3.4 (2023-02-09)

### Changed

* add custom error message when making a request to a backend which does not exist ([#412](https://github.com/fastly/js-compute-runtime/issues/412)) ([486aed1](https://github.com/fastly/js-compute-runtime/commit/486aed1415151a2bba40b736c14555c692bd095a))

## 1.3.3 (2023-02-08)

### Changed

* Remove error codes from external error messaging as these codes are not documented anywhere and subject to change ([8f8f0ef](https://github.com/fastly/js-compute-runtime/commit/8f8f0eff871597b8453fac08b6b114ee5c188ef6))

## 1.3.2 (2023-01-30)

### Changed

* allow a downstream response to contain lots of headers with the same name without crashing ([ba1f0e6](https://github.com/fastly/js-compute-runtime/commit/ba1f0e6699bd0f218fa581b9aad0fdda89a674fc))

## 1.3.1 (2023-01-26)

### Changed

* ensure CacheOverride bitflags are the same value as defined in c-at-e ([#386](https://github.com/fastly/js-compute-runtime/issues/386)) ([8a1c215](https://github.com/fastly/js-compute-runtime/commit/8a1c2158505e8ed1ebb424fc97866da155601d1f))

## 1.3.0 (2023-01-24)

### Added

* implement SubtleCrypto.prototype.digest method ([#372](https://github.com/fastly/js-compute-runtime/issues/372)) ([bbe1754](https://github.com/fastly/js-compute-runtime/commit/bbe1754f0a8018f2124b9a5859a35fde5c4cbb97))

## 1.2.0 (2023-01-17)

### Added

* implement Request.prototype.clone ([3f3a671](https://github.com/fastly/js-compute-runtime/commit/3f3a67199c27ea4500fa861a993163e5d376aafd))

## 1.1.0 (2023-01-06)

### Added

* add crypto.randomUUID function ([2c32b42](https://github.com/fastly/js-compute-runtime/commit/2c32b42d29a1cd2de961a0cef175b96eaab4ae7d))

### Changed

* check that setTimeout/setInterval handler is an object before casting to an object ([62476f5](https://github.com/fastly/js-compute-runtime/commit/62476f5324425c4f4a12ebf4f8ceddb093b753de))
* ensure retrieving the property definitions of ObjectStoreEntry.prototype.body and ObjectStoreEntry.bodyUsed do not cause panics by ensuring we have a valid entry in their Slots ([311b84c](https://github.com/fastly/js-compute-runtime/commit/311b84c80cbc99cf534ed43f4499a291716068fd))
* error message is latin1, we need to use JS\_ReportErrorLatin1 to convert the message from latin1 to UTF8CharsZ, otherwise a panic occurs ([f1a22a4](https://github.com/fastly/js-compute-runtime/commit/f1a22a42c75aea99f47f5f6b44920275735c91e1))

## 1.0.1 (2022-12-16)

### Changed

* do not free the method\_str.ptr as we still require the memory ([17c5049](https://github.com/fastly/js-compute-runtime/commit/17c50492d6247e746daeb65ab1b7fdeeaec0ae91)), closes [#352](https://github.com/fastly/js-compute-runtime/issues/352)

## 1.0.0 (2022-12-14)

### Added

* implement validation for backend cipher definitions ([157be64](https://github.com/fastly/js-compute-runtime/commit/157be64e84956d24259003331cb51a8c5acec040))

## 0.7.0 (2022-12-10)

### Added

* compute runtime component build ([#326](https://github.com/fastly/js-compute-runtime/issues/326)) ([197504c](https://github.com/fastly/js-compute-runtime/commit/197504c4192e019264011d732a7009786a7a38d0))

#### BREAKING CHANGES

* compute runtime component build ([#326](https://github.com/fastly/js-compute-runtime/issues/326))

### Changed

* Limit to node 16/17/18 as some dependencies do not work on node19 yet ([0d48f77](https://github.com/fastly/js-compute-runtime/commit/0d48f77467fc0c85c837c36b2e3991a2f6b35bcf))

## 0.6.0 (2022-12-09)

### Added

* Disable JS iterator helpers as the feature is at Stage 3 and we should only enable by default Stage 4 features ([c90c145](https://github.com/fastly/js-compute-runtime/commit/c90c14570a0375692da62eb11811e01babe28de8))

### Changed

* Throw TypeErrors in config-store if supplied with invalid parameters or the config-store does not exist ([6b70180](https://github.com/fastly/js-compute-runtime/commit/6b70180560b0c28bbc009af49fa7b25bd890d4a2))

### Removed

* Disable JS iterator helpers as the feature is at Stage 3 and we should only enable by default Stage 4 features

## 0.5.15 (2022-12-08)

### Added

* add `allowDynamicBackends` function to `fastly:experimental` module ([83a003e](https://github.com/fastly/js-compute-runtime/commit/83a003e17307c01876751686620a6a1effbfaa99))
* upgrade from SpiderMonkey 96 to SpiderMonkey 107 ([#330](https://github.com/fastly/js-compute-runtime/pull/330))

## 0.5.14 (2022-12-07)

### Changed

* when appending headers, if the set-cookie header is set then make sure that each cookie value is sent as a separate set-cookie header to the host ([f6cf559](https://github.com/fastly/js-compute-runtime/commit/f6cf5597ec646717534b59a1002b6a6364a81065))

## 0.5.13 (2022-12-02)

### Changed

* implement validation for Dictionary names and keys ([c0b0822](https://github.com/fastly/js-compute-runtime/commit/c0b082245d9585d8c3cdbc83c6f8ebf1844e8741))
* fix: When streaming a response to the host, do not close the response body if an error occurs ([8402ecf](https://github.com/fastly/js-compute-runtime/commit/8402ecf93c91bee66217c401a5cc5954e2e71de6))

## 0.5.12 (2022-11-30)

### Added

* add fastly:experimental module which contains all our experimental functions such as includeBytes and enableDebugLogging ([5c6a5d7](https://github.com/fastly/js-compute-runtime/commit/5c6a5d7cf13274f4752fa398d9bc92de658004b8))

## 0.5.11 (2022-11-30)

### Changed

* update nodejs supported versions to 16 - 19 and npm supported version to only 8 ([5ec70b9](https://github.com/fastly/js-compute-runtime/commit/5ec70b95b0d4d3677a522120c9ae5f9a2cea4db6))

## 0.5.10 (2022-11-30)

### Changed

* ensure custom cache keys are uppercased ([f37920d](https://github.com/fastly/js-compute-runtime/commit/f37920d01f5fb9a172ae82a1d6191159be59f561)), closes [#318](https://github.com/fastly/js-compute-runtime/issues/318)

## 0.5.9 (2022-11-29)

### Added

* add fastly:cache-override module ([f433464](https://github.com/fastly/js-compute-runtime/commit/f433464928e70a8f38ecb4dd293cb2ce40098c34))
* add geo ip lookup function to fastly:geolocation ([24601e5](https://github.com/fastly/js-compute-runtime/commit/24601e5738816ce1597f80d054d312c1a95e4398))
* Add Logger constructor to "fastly:logger" module ([b4818a2](https://github.com/fastly/js-compute-runtime/commit/b4818a2623caaab0fe568c35f7636d0d3d9e8bc7))
* expose fastly loggers via fastly:logger module ([2d0bcfe](https://github.com/fastly/js-compute-runtime/commit/2d0bcfe4f4e0fd855f589205eee4316d829fd28c))
* expose the fastly features via 'fastly:' namespaced modules ([c06cd16](https://github.com/fastly/js-compute-runtime/commit/c06cd1677cd96b383284ea6ab6dbcbbc4f6dfcf4))
* move env function into fastly:env ([327b344](https://github.com/fastly/js-compute-runtime/commit/327b344dc943a53ca4a74aeb16207f02cd6d0b3c))

### Changed

* Add types for setTimeout, clearTimeout, setInterval, clearInterval ([c1ed00c](https://github.com/fastly/js-compute-runtime/commit/c1ed00c8933bc45c9ba8dc84e515d31167596aa6))

## 0.5.8 (2022-11-28)

### Changed

* Allow process.execPath to contain whitespace ([caefe51](https://github.com/fastly/js-compute-runtime/commit/caefe512413675f10a7f1e6501249b3ebe7f5d21))

## 0.5.7 (2022-11-24)

### Changed

* add missing shebang and executable bit to the binary file ([3f0cd69](https://github.com/fastly/js-compute-runtime/commit/3f0cd69e3ec39633f747f0346ae3eda5eb3f3685))

## 0.5.6 (2022-11-24)

### Added

* implement setTimeout, setInterval, clearTimeout, and clearInterval ([128bca9](https://github.com/fastly/js-compute-runtime/commit/128bca901c9ad4b6d6c1084bf13c5c474ef63a41))

## 0.5.5 (2022-11-23)

### Added

* implement Request.prototype.setCacheKey ([457eabe](https://github.com/fastly/js-compute-runtime/commit/457eabe392f44eb296ce593bcabebffb68c57371))
* implement support in Response.json/text/arrayBuffer methods for guest provided streams ([50cdc44](https://github.com/fastly/js-compute-runtime/commit/50cdc443d38e53f029fbcc1ad19ee56b5849dff0))

### Changed

* respond with 500 Internal Server Error when an unhandled error has occured and no response has already been sent to the client ([e5982d8](https://github.com/fastly/js-compute-runtime/commit/e5982d879223a8e5940717ab74c9f01a64b35ce2))

## 0.5.4 (2022-09-28)

### Added

* Add ConfigStore class ([#270](https://github.com/fastly/js-compute-runtime/pull/270))
* Add Dynamic Backends support ([#250](https://github.com/fastly/js-compute-runtime/issues/250))
* Improved performance when constructing a ObjectStore instance ([#272](https://github.com/fastly/js-compute-runtime/pull/272)

#### Dynamic Backend support

Note: This feature is disabled by default for Fastly Services. Please contact [Fastly Support](https://support.fastly.com/hc/en-us/requests/new?ticket_form_id=360000269711) to request the feature be enabled on the Fastly Services which require Dynamic Backends.

This feature makes it possible to use the standard `fetch` within JavaScript applications.

Dynamic Backends is a new feature which enables JavaScript applications to dynamically create new backend server definitions without having to deploy a new version of their Fastly Service. These backends function exactly the same as existing backends, and can be configured in all the same ways as existing backends can via the Fastly Service configuration.

By default, Dynamic Backends are disabled within a JavaScript application as it can be a potential avenue for third-party JavaScript code to send requests, potentially including sensitive/secret data, off to destinations that the JavaScript project was not intending, which could be a security issue. To enable Dynamic Backends the application will need to set `fastly.allowDynamicBackends` is to `true`.

There are two ways to make use of Dynamic Backends within JavaScript projects:

The first way is by omitting the `backend` property definition on the Request instance. The JavaScript Runtime will then create a Dynamic Backend definition using default configuration options. This approach is useful for JavaScript applications as it means that a standard `fetch` call will now be possible, which means libraries that use the standard `fetch` will begin to work for applications deployed to Fastly.

Below is as an example JavaScript application using the default Dynamic Backend option:

```js
// Enable dynamic backends -- warning, this is potentially dangerous as third-party dependencies could make requests to their own backends, potentially including your sensitive/secret data
fastly.allowDynamicBackends = true;

// For any request, return the fastly homepage -- without defining a backend!
addEventListener("fetch", event => {
  event.respondWith(fetch('https://www.fastly.com/'));
});
```

The second way is by creating a new Dynamic Backend using the new `Backend` class. This approach is useful for JavaScript applications that want to have full control over the configuration of the new backend defintion, such as only allowing TLS 1.3 and disallowing older versions of TLS for requests made to the new backend.

E.G.

```js
// Enable dynamic backends -- warning, this is potentially dangerous as third-party dependencies could make requests to their own backends, potentially including your sensitive/secret data
fastly.allowDynamicBackends = true;

// For any request, return the fastly homepage -- without defining a backend!
addEventListener("fetch", event => {
  // We are not defining all the possible fields here, the ones which are not defined will use their default value instead.
  const backend = new Backend({
    name: 'fastly',
    target: 'fastly.com',
    hostOverride: "www.fastly.com",
    sslMinVersion: 1.3,
    sslMaxVersion: 1.3,
    sniHostname: "www.fastly.com",
  });
  event.respondWith(fetch('https://www.fastly.com/', {
    backend // Here we are configuring this request to use the newly defined backend from above.
  }));
});
```

#### Config-store support and Dictionary deprecated

We have renamed the `Dictionary` class to `ConfigStore`, the old name `Dictionary` still exists but is now deprecated. We recommend replacing `Dictionary` with `ConfigStore` in your code to avoid having to migrate in the future when `Dictionary` is fully removed.

Below is an example application using the `ConfigStore` class:

```js
async function app(event) {
  const store = new ConfigStore('example')
  
  // Retrieve the contents of the 'hello' key
  const hello = await store.get('hello')
  
  return new Response(hello)
}

addEventListener("fetch", event => {
  event.respondWith(app(event))
})
```

## 0.5.3 (2022-09-16)

### Security

* [CVE-2022-39218](https://github.com/fastly/js-compute-runtime/security/advisories/GHSA-cmr8-5w4c-44v8):
  Fixed `Math.random` and `crypto.getRandomValues` methods to always use sufficiently random values. The previous versions would use a PRNG (pseudorandom number generator) which we would seed with a random value however due to our use of [Wizer](https://github.com/bytecodealliance/wizer), the initial value to seed the PRNG was baked-in to the final WebAssembly module meaning the sequence of numbers generated was predictable for that specific WebAssembly module. The new implementations of both `Math.random` and `crypto.getRandomValues` do not use a PRNG and instead pull random values from WASI (WebAssembly System Interface) libc’s `random_get` function, which is always a sufficiently random value.

  An attacker with access to the same WebAssembly module that calls the affected methods could use the fixed seed to predict random numbers generated by these functions. This information could be used to bypass cryptographic security controls, for example to disclose sensitive data encrypted by functions that use these generators.

  Developers should update affected modules after applying this patch. Any secrets generated using affected versions should be rotated. Any sensitive ciphertext generated using affected versions should be considered unsafe, e.g. and be deleted or re-generated.

### Fixed

* Updated the Typescript definitions for the `console` methods to indicate that they now accept any number of objects. ([#258](https://github.com/fastly/js-compute-runtime/pull/258))

* Store the Object-Store key string into a native object to avoid it becoming garbage collected before being used within `ObjectStore.prototype.get` or `ObjectStore.prototype.put` (([381242](https://github.com/fastly/js-compute-runtime/commit/3812425a955e52c2fd7229e762ef3e691cb78745))

## 0.5.2 (2022-09-02)

### Fixed

* Explicitly declare void as the return type for functions which return nothing - this allows our package to work with typescript's `strict:true` option ([#253](https://github.com/fastly/js-compute-runtime/pull/253))

* Declare ambient types for our npm package instead of exports as we do not yet export anything from the package ([#252](https://github.com/fastly/js-compute-runtime/pull/252))

## 0.5.1 (2022-08-31)

### Fixed

* Removed `type: "module"` from the @fastly/js-compute package.json file as the package still uses `require`

## 0.5.0 (2022-08-30)

### Added

* Implemented ObjectStore and ObjectStoreEntry classes for interacting with Fastly ObjectStore ([#110](https://github.com/fastly/js-compute-runtime/issues/110))
* add btoa and atob native implementations ([#227](https://github.com/fastly/js-compute-runtime/issues/227)) ([8b8c31f](https://github.com/fastly/js-compute-runtime/commit/8b8c31fa9ad70337b1060a3242b8e3495ae47df3))

#### Object-store support

This release adds support for Fastly [Object-store](https://developer.fastly.com/reference/api/services/resources/kv-store/), which is globally consistent key-value storage accessible across the Fastly Network. This makes it possible for your Fastly Compute application to read and write from Object-stores.

We've added two classes, `ObjectStore`, and `ObjectStoreEntry`. `ObjectStore` is used to interact with a particular Object-store and `ObjectStoreEntry` is a particular value within an Object-store. We've made `ObjectStoreEntry` have a similar API as `Response` to make it simpler to read and write from Object-stores. I.e. `ObjectStoreEntry` has a `body` property which is a `ReadableStream` and has `arrayBuffer`/`json`/`text` methods - just like `Response`.

The way to use these classes is best shown with an example:

```js
async function app(event) {
  // Create a connection the the Object-store named 'example-store'
  const store = new ObjectStore('example-store')
  
  // Create or update the 'hello' key with the contents 'world'
  await store.put('hello', 'world')
  
  // Retrieve the contents of the 'hello' key
  // Note: Object-stores are eventually consistent, this means that the updated contents associated may not be available to read from all
  // Fastly edge locations immediately and some edge locations may continue returning the previous contents associated with the key.
  const hello = await store.get('hello')
  
  // Read the contents of the `hello` key into a string
  const hellotext = await hello.text()
  return new Response(hellotext)
}

addEventListener("fetch", event => {
  event.respondWith(app(event))
})
```

#### Added `btoa` and `atob` global functions

These two functions enable you to encode to ([btoa](https://developer.mozilla.org/en-US/docs/Web/API/btoa)) and decode from ([atob](https://developer.mozilla.org/en-US/docs/Web/API/atob)) Base64 strings. They follow the same specification as the `atob` and `btoa` functions that exist in web-browsers.

```js
addEventListener("fetch", event => {
  event.respondWith(new Response(atob(btoa('hello from fastly'))))
})
```

#### Improved Console Support

Previously our console methods only supported a single argument and would convert the argument to a string via `String(argument)`, this unfortunately made it difficult to log out complex objects such as Request objects or similar.

We've updated our console methods and they now support any number of arguments. As well as supporting any number of arguments, we've also changed the implementation to have better support for logging out complex objects.

This is a before and after example of what happens when logging a Request with our console methods.

Before:

```js
const request = new Request('https://www.fastly.com', {body:'I am the body', method: 'POST'});
console.log(request); // outputs `[object Object]`.
```

After:

```js
const request = new Request('https://www.fastly.com', {body:'I am the body', method: 'POST'});
console.log(request); // outputs `Request: {method: POST, url: https://www.fastly.com/, version: 2, headers: {}, body: null, bodyUsed: false}`.
```

### Changed

* Improved console output for all types ([#204](https://github.com/fastly/js-compute-runtime/issues/204))

## 0.4.0 (2022-07-28)

### Added

* Implement the DecompressionStream builtin [`#160`](https://github.com/fastly/js-compute-runtime/pull/160)
* Improve performace of Regular Expression literals via precompilation [`#146`](https://github.com/fastly/js-compute-runtime/pull/146)

### Fixed

* Calling `tee` on the client request no longer causes the application to hang [`#156`](https://github.com/fastly/js-compute-runtime/pull/156)

## 0.3.0 (2022-06-29)

### Added

* Implement the CompressionStream builtin
  [#84](https://github.com/fastly/js-compute-runtime/pull/84)

### Changed

* Removed the requirement for a fastly.toml file to be present when using js-compute-runtimes CLI to compile a WASM file
* **Breaking change:** Removed --skip-pkg argument from js-compute-runtime's CLI
  [#108](https://github.com/fastly/js-compute-runtime/pull/108)
* **Breaking change:** Removed `console.trace` method

### Fixed

* Fix the response error message text
* Throw an error if constructors are called as plain functions
* Fix the behavior of `console.debug`
* Allow builtin classes to be extended

## 0.2.5 (2022-04-20)

### Changed

* Updated the js-compute-runtime to 0.2.5 : Increased max uri length to 8k, and properly forwards http headers to upstream requests even if the headers aren't ever read from

## 0.2.4 (2022-02-09)

### Changed

* Support streaming upstream request bodies

## 0.2.2 (2022-02-03)

### Added

* Add full support for TransformStreams
* Support directly piping Request/Response bodies to other Requests/Responses instead of manually copying every chunk
* Add support for the `queueMicrotask` global function
* Add support for the `structuredClone` global function
* Add support for the `location` global object as an instance of `WorkerLocation`
* Support BigUint64Array and BigInt64Array in crypto.getRandomValues
* Enable class static blocks syntax
* Returned the exit code from the JS Compute Runtime, by passing it up through our CLI

### Changed

* Increase max supported header size from 4096 bytes to 69000 bytes
* Update to SpiderMonkey 96 beta

### Fixed

* Avoid waiting for async tasks that weren't passed to `FetchEvent#waitUntil`
* Significantly improve spec-compliance of Request and Response builtins

## 0.2.1 (2021-11-10)

### Added

* Updated the js-compute-runtime to `0.2.2` (Which includes fixes to geoip, a way to get environment variables, improves debugging of exceptions in the request handler, and other updates)
* Added the `Env` namespace for accessing Fastly Compute environment variables.

## 0.2.0 (2021-08-31)

### Added

* Implement the WHATWG URL and URLSearchParam classes
* Enable private class fields and methods, plus ergonomic brand checks
* Breaking change: Implement FetchEvent#waitUntil
* Mark builtins that rely on hostcalls as request-handler-only
* Add support for including binary files, and for using typed arrays as Response bodies
* Improve handling of hostcall errors

### Fixed

* Breaking change: Make FetchEvent handling more spec-compliant
* Normalize HTTP method names when constructing Requests
* Don't trap when trying to delete a non-existent header
* Properly support `base` argument in `URL` constructor

## 0.1.0 (2021-07-28)

### Added

* Initial Release
* Includes TypeScript type definitions for Fastly Compute flavored ServiceWorkers APIs
* Also includes the `js-compute-runtime` CLI for bundling JavaScript applications
