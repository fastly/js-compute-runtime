#!/usr/bin/env bash
cd "$(dirname "$0")" || exit 1
RUNTIME_VERSION=$(npm pkg get version --json --prefix=../../ | jq -r)
HOST_API=$(realpath host-api) cmake -B build-debug -DCMAKE_BUILD_TYPE=Debug -DENABLE_BUILTIN_WEB_FETCH=0 -DENABLE_BUILTIN_WEB_FETCH_FETCH_EVENT=0 -DRUNTIME_VERSION="\"$RUNTIME_VERSION-debug\""
cmake --build build-debug --parallel 10
if [ "$1" != "--keep-debug-info" ]; then
  wasm-tools strip build-debug/starling.wasm/starling.wasm -d ".debug_(info|loc|ranges|abbrev|line|str)" -o ../../starling.debug.wasm
else
  cp build-debug/starling.wasm/starling.wasm ../../starling.debug.wasm
fi
