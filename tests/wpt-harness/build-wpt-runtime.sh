#!/usr/bin/env bash

set -euo pipefail

script_dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"

inputs=(
  "${script_dir}/pre-harness.js"
  "${script_dir}/wpt/resources/testharness.js"
  "${script_dir}/post-harness.js"
)

cat "${inputs[@]}" | \
  wizer --allow-wasi --dir=. -r _start=wizer.resume -o wpt-runtime.wasm js-compute-runtime.wasm
