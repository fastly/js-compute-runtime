#!/usr/bin/env bash
cd "$(dirname "$0")" || exit 1
RUNTIME_VERSION=$(npm pkg get version --json --prefix=../../ | jq -r)
HOST_API=$(realpath host-api) cmake -B build-release-weval -DCMAKE_BUILD_TYPE=Release -DENABLE_BUILTIN_WEB_FETCH=0 -DENABLE_BUILTIN_WEB_FETCH_FETCH_EVENT=0 -DRUNTIME_VERSION="\"$RUNTIME_VERSION\"" -DWEVAL=ON
cmake --build build-release-weval --parallel 8
mv build-release-weval/starling-raw.wasm/starling-raw.wasm ../../fastly-weval.wasm
mv build-release-weval/starling-raw.wasm/starling-ics.wevalcache ../../fastly-ics.wevalcache
