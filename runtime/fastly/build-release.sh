#!/usr/bin/env bash
cd "$(dirname "$0")" || exit 1
HOST_API=$(realpath host-api) cmake -B build-release -DCMAKE_BUILD_TYPE=Release -DENABLE_BUILTIN_WEB_FETCH=0 -DENABLE_BUILTIN_WEB_FETCH_FETCH_EVENT=0
cmake --build build-release
mv build-release/starling.wasm/starling.wasm ../../
