#!/usr/bin/env bash
cd "$(dirname "$0")" || exit 1
HOST_API=$(realpath host-api) cmake -B build-debug -DCMAKE_BUILD_TYPE=Debug -DENABLE_BUILTIN_WEB_FETCH=0 -DENABLE_BUILTIN_WEB_FETCH_FETCH_EVENT=0
cmake --build build-debug --parallel 10
mv build-debug/starling.wasm/starling.wasm ../../
