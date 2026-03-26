#!/usr/bin/env bash

set -euo pipefail
set -x

cd "$(dirname "$0")" || exit 1

# Parse command line arguments
KEEP_DEBUG_INFO=0
GC_FREQUENCY=""

while [[ $# -gt 0 ]]; do
  case $1 in
    --keep-debug-info)
      KEEP_DEBUG_INFO=1
      shift
      ;;
    --gc-frequency)
      if [[ -n "${2:-}" ]]; then
        GC_FREQUENCY="-DFASTLY_GC_FREQUENCY=$2"
        shift 2
      else
        echo "Error: --gc-frequency requires a value"
        exit 1
      fi
      ;;
    *)
      echo "Unknown option: $1"
      exit 1
      ;;
  esac
done

RUNTIME_VERSION=$(npm pkg get version --json --prefix=../../ | jq -r)
HOST_API=$(realpath host-api) cmake -B build-debug -DCMAKE_BUILD_TYPE=Debug -DENABLE_BUILTIN_WEB_FETCH=0 -DENABLE_BUILTIN_WEB_FETCH_FETCH_EVENT=0 -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DRUNTIME_VERSION="\"$RUNTIME_VERSION-debug\"" -DENABLE_JS_DEBUGGER=OFF "$GC_FREQUENCY"
cmake --build build-debug --parallel 10
if [ "$KEEP_DEBUG_INFO" -eq 0 ]; then
  wasm-tools strip build-debug/starling-raw.wasm/starling-raw.wasm -d ".debug_(info|loc|ranges|abbrev|line|str)" -o ../../fastly.debug.wasm
else
  cp build-debug/starling-raw.wasm/starling-raw.wasm ../../fastly.debug.wasm
fi
