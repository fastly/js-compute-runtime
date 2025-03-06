#!/usr/bin/env bash
cd "$(dirname "$0")" || exit 1
RUNTIME_VERSION=$(npm pkg get version --json --prefix=../../ | jq -r)
HOST_API=$(realpath host-api) cmake -B build-release -DCMAKE_BUILD_TYPE=Release -DENABLE_BUILTIN_WEB_FETCH=0 -DENABLE_BUILTIN_WEB_FETCH_FETCH_EVENT=0 -DRUNTIME_VERSION="\"$RUNTIME_VERSION\"" -DENABLE_JS_DEBUGGER=OFF
cmake --build build-release
mv build-release/starling-raw.wasm/starling-raw.wasm ../../fastly.wasm
